#include "subcircMatch.h"
#include <map>
#include <set>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>

#include "circuitGroup.h"
#include "debug.h"

using namespace std;

namespace {
    /// Recursively finds `needle` in `haystack` filling `results`
    void findIn(vector<MatchResult>& results,
            CircuitGroup* needle, CircuitGroup* haystack);

    /// Yields every possible output layout for a wire to continue the walk
    class WireOutPermutation {
        public:
            typedef CircuitTree Val;
            WireOutPermutation(
                    const unordered_map<sign_t, vector<Val*> >& sigMatches,
                    const unordered_map<sign_t, int>& occursOfSig);
            bool next();
            Val* get(sign_t sig, int occur) const;

        private:
            const unordered_map<sign_t, vector<Val*> >& sigMatches;
            const unordered_map<sign_t, int>& occursOfSig;
            map<sign_t, vector<int> > perms;
    };

    bool walkMatchesNode(
            vector<MatchResult>& results,
            CircuitGroup* fullNeedle,
            CircuitTree* match,
            CircuitTree* needleMatch,
            set<CircuitTree*>& alreadyImplied,
            map<CircuitTree*, set<CircuitTree*> >& singleMatches,
            map<CircuitTree*, CircuitTree*>& nodeMap,
            unordered_map<WireId*, WireId*>& edgeMap);

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

    class FoundResult : public std::exception {
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

    WireOutPermutation::WireOutPermutation(
            const unordered_map<sign_t, vector<Val*> >& sigMatches,
            const unordered_map<sign_t, int>& occursOfSig) :
        sigMatches(sigMatches), occursOfSig(occursOfSig)
    {
        for(const auto& occur: occursOfSig) {
            vector<int> permVal(sigMatches.at(occur.first).size(), -1);
            if(occur.second > (int)permVal.size()) {
                FIND_DEBUG("@ Expecting < %d, has %ld\n",
                        occur.second, permVal.size());
                throw std::out_of_range("Not enough possible outputs");
            }
            for(int pos = 0; pos < occur.second; ++pos)
                permVal[pos] = pos;
            perms[occur.first] = permVal;
        }
    }

    bool WireOutPermutation::next() {
        for(auto& perm: perms) {
            if(next_permutation(perm.second.begin(), perm.second.end()))
                return true;
        }
        return false; // Reached end of permutations
    }

    // NOTE: lets a `vector<>.at` exception escape on invalid `sig`.
    WireOutPermutation::Val* WireOutPermutation::get(sign_t sig, int occur)
        const
    {
        // A bit slower that what it could be, but much simpler that way
        int elemPos = 0;
        for(const auto& elt: perms.at(sig)) {
            if(elt == occur)
                break;
            ++elemPos;
        }

        return sigMatches.at(sig).at(elemPos);
    }

    // FIXME at the moment this is **slightly** too exponential.
    // The DFS-like walk runs into the same configuration multiple times in
    // different orders.

    /// Walks the `match` graph for a WireId edge
    void walkMatchesEdge(
            vector<MatchResult>& results,
            CircuitGroup* fullNeedle,
            WireId* match,
            WireId* needleMatch,
            set<CircuitTree*>& alreadyImplied,
            map<CircuitTree*, set<CircuitTree*> >& singleMatches,
            map<CircuitTree*, CircuitTree*>& nodeMap,
            unordered_map<WireId*, WireId*>& edgeMap)
    {
        // We do not have to check `edgeMap` here, it was taken care of in
        // `walkMatchesNode`.

        // Collect the `CircuitTree`s that are not connected yet in the match
        vector<CircuitTree*> missing;
        unordered_map<sign_t, int> occursOfSig;
        map<CircuitTree*, int> occurId;
        for(auto needleCirc = needleMatch->adjacent_begin();
                needleCirc != needleMatch->adjacent_end();
                ++needleCirc)
        {
            if(nodeMap.find(*needleCirc) == nodeMap.end()) {
                sign_t signature = localSign(*needleCirc);
                missing.push_back(*needleCirc);
                occurId[*needleCirc] = occursOfSig[signature]; // initially 0
                ++occursOfSig[signature];
            }
        }

        // Build up lists of possible matches for each missing signature
        unordered_map<sign_t, vector<CircuitTree*> > sigMatches;
        for(auto circ = match->adjacent_begin();
                circ != match->adjacent_end();
                ++circ)
        {
            sign_t signature = localSign(*circ);
            if(occursOfSig.find(signature) != occursOfSig.end())
                sigMatches[signature].push_back(*circ);
        }

        // Tries every possible permutation
        WireOutPermutation perm(sigMatches, occursOfSig);
        do {
            FIND_DEBUG("   > Trying a permutation\n");
            for(const auto& circ: missing)
                nodeMap[circ] = perm.get(localSign(circ), occurId[circ]);

            for(const auto& circ: missing) {
                CircuitTree* matched = nodeMap[circ];
                nodeMap.erase(circ); // It will be put back again
                if(!walkMatchesNode(results, fullNeedle,
                            matched, circ,
                            alreadyImplied, singleMatches,
                            nodeMap, edgeMap))
                {
                    break;
                }
            }

            for(const auto& circ: missing)
                nodeMap.erase(circ);
        } while(perm.next());
    }

    /// Walks the `match` graph, trying to build up the possible final matches
    bool walkMatchesNode(
            vector<MatchResult>& results,
            CircuitGroup* fullNeedle,
            CircuitTree* match,
            CircuitTree* needleMatch,
            set<CircuitTree*>& alreadyImplied,
            map<CircuitTree*, set<CircuitTree*> >& singleMatches,
            map<CircuitTree*, CircuitTree*>& nodeMap,
            unordered_map<WireId*, WireId*>& edgeMap)
    {
        if(nodeMap.find(needleMatch) != nodeMap.end()) { // Already taken care of
            FIND_DEBUG("    Already mapped ←\n");
            return nodeMap.find(needleMatch)->second == match;
        }

        if(alreadyImplied.find(match) != alreadyImplied.end()) {
            FIND_DEBUG("    Implied in a match ←\n");
            return false;
        }

        if(singleMatches[needleMatch].find(match)
                == singleMatches[needleMatch].end()) {
            FIND_DEBUG("    Not a match ←\n");
            return false;
        }

        for(auto pin = match->io_begin(), needlePin = needleMatch->io_begin();
                pin != match->io_end() && needlePin != needleMatch->io_end();
                ++pin, ++needlePin)
        {
            try {
                if(*edgeMap.at(*needlePin) != **pin) {
                    FIND_DEBUG("    (%s --> %s <> %s\n",
                            (*needlePin)->name().c_str(),
                            (*pin)->name().c_str(),
                            edgeMap.at(*needlePin)->name().c_str());
                    FIND_DEBUG("    Bad structure ←\n");
                    return false;
                }
            } catch(const out_of_range&) {}
        }

        nodeMap[needleMatch] = match;
        vector<WireId*> toUnassign;
        for(auto pin = match->io_begin(), needlePin = needleMatch->io_begin();
                pin != match->io_end() && needlePin != needleMatch->io_end();
                ++pin, ++needlePin)
        {
            if(edgeMap.find(*needlePin) == edgeMap.end()) {
                toUnassign.push_back(*needlePin);
                edgeMap[*needlePin] = *pin;
            }
        }

        FIND_DEBUG("  > So far, %lu\n", nodeMap.size());
        if(nodeMap.size() == fullNeedle->getChildrenCst().size()) {
            // Reached end of recursion - we have a full-size result
            if(isActualMatch(nodeMap, edgeMap, singleMatches)) {
                results.push_back(buildMatchResult(
                            fullNeedle, nodeMap, edgeMap));
                for(const auto& node: nodeMap)
                    alreadyImplied.insert(node.second);
                throw FoundResult();
            }
        }
        else {
            for(auto pin = match->io_begin(),
                    needlePin = needleMatch->io_begin();
                    pin != match->io_end()
                    && needlePin != needleMatch->io_end();
                    ++pin, ++needlePin)
            {
                walkMatchesEdge(results, fullNeedle, *pin, *needlePin,
                        alreadyImplied, singleMatches, nodeMap, edgeMap);
            }
        }

        for(const auto& unassign: toUnassign)
            edgeMap.erase(unassign);
        nodeMap.erase(needleMatch);
        return true;
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
        bool found = false;
        for(const auto& match: singleMatches) {
            if(match.second.size() > 0) {
                found = true;
                FIND_DEBUG(" %lu", match.second.size());
            }
        }
        if(found)
            FIND_DEBUG(" sig matches\n");

        // Filter out the matches that are not connected as needed
        for(const auto& needleMatch: singleMatches) {
            set<CircuitTree*> nMatches;
            for(const auto& match: needleMatch.second) {
                if(isMatchFit(match, needleMatch.first, wireFit))
                    nMatches.insert(match);
            }
            if(nMatches.size() > 0)
                singleMatches[needleMatch.first] = nMatches;
            else // lighten the data structure
                singleMatches.erase(needleMatch.first);
        }

        // Ensure there is at least enough matches for a full `needle`
        for(const auto& child: needle->getChildrenCst())
            if(singleMatches[child].empty())
                return;

        // Pick the fewest matches
        CircuitTree* fewestMatchesNeedle = singleMatches.begin()->first;
        for(const auto& match: singleMatches) {
            if(match.second.size() < singleMatches[fewestMatchesNeedle].size())
                fewestMatchesNeedle = match.first;
        }

        // Build the various possible matches
        for(auto& match: singleMatches[fewestMatchesNeedle]) {
            map<CircuitTree*, CircuitTree*> nodeMap;
            unordered_map<WireId*, WireId*> edgeMap;

            try {
                walkMatchesNode(results, needle, match, fewestMatchesNeedle,
                        alreadyImplied, singleMatches, nodeMap, edgeMap);
            } catch(const FoundResult&) {}
        }
    }
};

std::vector<MatchResult> matchSubcircuit(CircuitGroup* needle,
        CircuitGroup* haystack)
{
    vector<MatchResult> out;
    findIn(out, needle, haystack);
    return out;
}
