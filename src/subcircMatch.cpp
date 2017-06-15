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

namespace {

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
        map<CircuitTree*, CircuitTree*>& nodeMap,
        unordered_map<WireId*, WireId*>& edgeMap,
        map<CircuitTree*, set<CircuitTree*> > singleMatches)
{
    /* We have a few things to check here.
       (1) The nodes must be formally equal
       (2) The structure must be identical, that is, when iterating over
           the IOs of each pair of mapped nodes, the IOs must be the same
           wrt. the edge mapping
       (3) The nodes' and edges' map must be a bijective mapping, that is,
           we must ensure that not two keys map to the same value
    */

    FIND_DEBUG(" > Checking a potential solution…\n");

    // (1)
    for(const auto& nodeMatch: nodeMap) {
        if(!nodeMatch.first->equals(nodeMatch.second)) {
            // Remember that this was not a match, after all.
            singleMatches[nodeMatch.first].erase(
                    singleMatches[nodeMatch.first].find(nodeMatch.second));
            FIND_DEBUG("  > Not sub-equal\n");
            return false;
        }
    }

    // (2)
    for(const auto& nodeMatch: nodeMap) {
        try{
            auto needlePin = nodeMatch.first->io_begin(),
                 haystackPin = nodeMatch.second->io_begin();
            for(; needlePin != nodeMatch.first->io_end()
                    && haystackPin != nodeMatch.second->io_end() ;
                    ++needlePin, ++haystackPin)
            {
                if(*(edgeMap.at(*needlePin)) != **haystackPin) {
                    FIND_DEBUG("%s ---> %s <> %s\n",
                            (*needlePin)->name().c_str(),
                            edgeMap.at(*needlePin)->name().c_str(),
                            (*haystackPin)->name().c_str());
                    FIND_DEBUG("  > Wires inconsistency\n");
                    return false;
                }

            }

            if(needlePin != nodeMatch.first->io_end()
                    || haystackPin != nodeMatch.second->io_end())
            {
                FIND_DEBUG("  > Mismatched number of pins\n");
                return false; // mismatched pin number?!
            }
        } catch(const std::out_of_range&) {
            FIND_DEBUG("  > Unmapped pin\n");
            return false; // One of the edgeMap lookups failed
        }
    }

    // (3)
    // (3.1) -- nodes
    {
        set<CircuitTree*> seen;
        for(const auto& match: nodeMap) {
            if(seen.find(match.second) != seen.end()) {
                FIND_DEBUG("  > Nodes non-injective\n");
                return false;
            }
            seen.insert(match.second);
        }
    }
    // (3.2) -- edges
    {
        unordered_set<WireId*> seen;
        for(const auto& match: edgeMap) {
            if(seen.find(match.second) != seen.end()) {
                FIND_DEBUG("  > Wires non-injective\n");
                return false;
            }
            seen.insert(match.second);
        }
    }

    FIND_DEBUG("  > Found a match\n");

    // Everything is fine now!
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
