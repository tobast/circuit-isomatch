#include "subcircMatch.h"
#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include <list>
#include <stdexcept>
#include <algorithm>

#include "circuitGroup.h"
#include "dyn_bitset.h"
#include "debug.h"

using namespace std;

#ifdef DEBUG_FIND
class ImplementationBug: public std::runtime_error {
    public:
        ImplementationBug(const char* what) : runtime_error(what) {};
};
#endif

namespace {

typedef std::vector<DynBitset> PermMatrix;

struct Vertice {
    enum Type {
        VertWire, VertCirc
    } type;

    union {
        WireId* wire;
        CircuitTree* circ;
    };
};

struct VerticeMapping {
    unordered_map<WireId*, size_t> wireId;
    map<CircuitTree*, size_t> circId;
    vector<Vertice> vertices;
};

struct FullMapping {
    VerticeMapping haystack, needle;
};

/// Recursively finds `needle` in `haystack` filling `results`
void findIn(vector<MatchResult>& results,
        CircuitGroup* needle, CircuitGroup* haystack);

// ========================================================================

sign_t localSign(CircuitTree* circ) {
    return circ->sign(0);
}

class WireFit {
    public:
        /** Add a connection on this wire from a circuit of signature
         * `inSig`, from its `pin`-th pin, which is an input of the circuit
         * iff `in` is true. */
        void connected(sign_t inSig, bool /* in */, int /* pin */) {
            ++conns[ConnType(inSig, false, 0)];
            // FIXME ^ use actual values
        }

        /** Check whether this wire has the required connections to act as
         * `role` */
        bool fitFor(WireId* role) {
            try {
                return fitness.at(role);
            } catch(const std::out_of_range&) {
                map<ConnType, int> usedConns;

                for(auto circ = role->adjacent_begin();
                        circ != role->adjacent_end(); ++circ)
                {
                    ConnType cConn(localSign(*circ), false, 0);
                    // FIXME ^ use actual values

                    ++usedConns[cConn];
                    if(usedConns[cConn] > conns[cConn]) {
                        fitness[role] = false;
                        return false;
                    }
                }
                fitness[role] = true;
                return true;
            }
        }

    private:
        struct ConnType {
            ConnType(sign_t inSig, bool in, int pin) :
                inSig(inSig), in(in), pin(pin) {}
            bool operator==(const ConnType& e) {
                return inSig == e.inSig && in == e.in && pin == e.in;
            }
            bool operator<(const ConnType& e) const {
                return
                    (inSig < e.inSig)
                    || (inSig == e.inSig && in < e.in)
                    || (inSig == e.inSig && in == e.in && pin < e.in);
            }

            sign_t inSig;
            bool in;
            int pin;
        };

        map<ConnType, int> conns;
        unordered_map<WireId*, bool> fitness;
};

bool isMatchFit(CircuitTree* match, CircuitTree* needleMatch,
        unordered_map<WireId*, WireFit>& wireFit)
{
    FIND_DEBUG(" > Checking fitness\n");
    auto wire = match->io_begin(),
    role = needleMatch->io_begin();

    for(; wire != match->io_end()
            && role != needleMatch->io_end();
            ++wire, ++role)
    {
        if(!wireFit[*wire].fitFor(*role)) {
            FIND_DEBUG("  Not fit\n");
            return false;
        }
    }
    if(wire != match->io_end()
            || role != needleMatch->io_end())
    {
        return false;
    }

    return true;
}

/** Check whether a supposed match is actually a match or not.
 * This actually also updates `singleMatches` whenever an exact match
 * proves a `singleMatch` wrong. */
bool isActualMatch(
        const PermMatrix& perm,
        const FullMapping& mapping,
        map<CircuitTree*, set<CircuitTree*> > singleMatches)
{
    /* Here, we must check that the nodes are actually equal to each other.
     * This only applies for circuits; Ullmann's algorithm ensures that wires
     * are correctly mapped when we reach this point.
     */

    FIND_DEBUG(" > Checking a potential solution…\n");

    for(size_t haystackPos = 0;
            haystackPos < mapping.haystack.vertices.size();
            ++haystackPos)
    {
        if(mapping.haystack.vertices[haystackPos].type != Vertice::VertCirc)
            continue; // We don't have to check that one

        if(!perm[haystackPos].any())
            continue; // not mapped

        int mappedId = perm[haystackPos].singleBit();
        if(mappedId < 0) {
#ifdef DEBUG_FIND
            throw ImplementationBug("Non-bijective mapping `isActualMatch`");
#else
            return false; // Bad mapping
#endif
        }

        if(mapping.needle.vertices[mappedId].type != Vertice::VertCirc) {
#ifdef DEBUG_FIND
            throw ImplementationBug("Mapping vertice to wire `isActualMatch`");
#else
            return false;
#endif
        }

        CircuitTree* needlePart = mapping.needle.vertices[mappedId].circ;
        CircuitTree* haystackPart = mapping.haystack.vertices[mappedId].circ;

        if(!needlePart->equals(haystackPart)) {
            set<CircuitTree*> singleMatchesOf = singleMatches[needlePart];
            // Remember that this was not a match, after all.
            auto singleIter = singleMatchesOf.find(haystackPart);
            if(singleIter != singleMatchesOf.end())
                singleMatchesOf.erase(singleIter);
            FIND_DEBUG("  > Not sub-equal\n");
            return false;
        }
    }

    // Everything is fine now!
    FIND_DEBUG("  > Found a match\n");
    return true;
}

/// Create a `MatchResult` based on match maps
MatchResult buildMatchResult(
        const CircuitGroup* fullNeedle,
         map<CircuitTree*, CircuitTree*>& nodeMap,
         unordered_map<WireId*, WireId*>& edgeMap)
{
    MatchResult res;
    for(const auto& needlePart: fullNeedle->getChildrenCst())
        res.parts.push_back(nodeMap[needlePart]);
    for(const auto& inp: fullNeedle->getInputs())
        res.inputs.push_back(edgeMap.at(inp->actual()));
    for(const auto& out: fullNeedle->getOutputs())
        res.outputs.push_back(edgeMap.at(out->actual()));
    return res;
}

void findIn(vector<MatchResult>& results,
        CircuitGroup* needle, CircuitGroup* haystack)
{
    // Circuits that are already part of a match result
    set<CircuitTree*> alreadyImplied;

    // Recurse in hierarchy
    for(auto& child: haystack->getChildrenCst()) {
        if(child->circType() == CircuitTree::CIRC_GROUP) {
            size_t prevMatches = results.size();
            findIn(results, needle, dynamic_cast<CircuitGroup*>(child));
            if(results.size() != prevMatches)
                alreadyImplied.insert(child);
        }
    }


    map<CircuitTree*, set<CircuitTree*> > singleMatches;

    // Fill single matches
    {
        unordered_map<sign_t, set<CircuitTree*> > signatures;
        for(auto hayPart : haystack->getChildrenCst())
            signatures[localSign(hayPart)].insert(hayPart);
        for(auto needlePart : needle->getChildrenCst()) {
            singleMatches[needlePart] = signatures[localSign(needlePart)];
        }
    }

    // Fill wire connections -- computes "fitness" for given wire roles
    unordered_map<WireId*, WireFit> wireFit;
    for(const auto& needleMatch: singleMatches) {
        sign_t cSig = localSign(needleMatch.first);
        for(const auto& match: needleMatch.second) {
            int pin = 0;
            for(auto inp = match->inp_begin(); inp != match->inp_end();
                    ++inp)
            {
                wireFit[*inp].connected(cSig, true, pin);
                ++pin;
            }

            pin = 0;
            for(auto out = match->out_begin(); out != match->out_end();
                    ++out)
            {
                wireFit[*out].connected(cSig, false, pin);
                ++pin;
            }
        }
    }

    FIND_DEBUG("=== IN %s ===\n", haystack->name().c_str());

    // Filter out the matches that are not connected as needed
    {
        auto needleMatch = singleMatches.begin();
        while(needleMatch != singleMatches.end()) {
            set<CircuitTree*> nMatches;
            for(const auto& match: needleMatch->second) {
                if(isMatchFit(match, needleMatch->first, wireFit))
                    nMatches.insert(match);
            }
            if(nMatches.size() > 0) {
                singleMatches[needleMatch->first] = nMatches;
                ++needleMatch;
            }
            else // lighten the data structure
                needleMatch = singleMatches.erase(needleMatch);
        }
    }

    // Ensure there is at least enough matches for a full `needle`
    for(const auto& child: needle->getChildrenCst())
        if(singleMatches[child].empty())
            return;

    // Ullman algorithm
    // TODO
}

}; // namespace

std::vector<MatchResult> matchSubcircuit(CircuitGroup* needle,
        CircuitGroup* haystack)
{
    vector<MatchResult> out;
    findIn(out, needle, haystack);
    return out;
}
