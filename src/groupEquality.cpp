#include "groupEquality.h"
#include "debug.h"
#include <algorithm>
#include <map>

using namespace std;

namespace groupEquality {
    Permutation::Permutation(const SigSplit& split) {
        for(const auto& category : split) {
            perms.push_back(vector<int>(category.size()));
            mkIdentity(perms.back());
        }
        nextChange = lastIter();
    }

    const Permutation::PermElem& Permutation::operator[](sign_t index) const {
        return perms.at(index);
    }

    bool Permutation::next() {
        while(!next_permutation(nextChange->begin(),
                    nextChange->end()))
        {
            if(nextChange == perms.begin())
                return false; // Last permutation
            --nextChange;
        }
        nextChange = lastIter();
        return true;
    }

    void Permutation::mkIdentity(vector<int>& vect) {
        int n=0;
        generate(vect.begin(), vect.end(), [&n]{ return n++; });
    }
    Permutation::PermStruct::iterator Permutation::lastIter() {
        return --(perms.end());
    }

    int factorial(int k) {
        static vector<int> memoized {1, 1};
        for(int nextVal = (int)memoized.size(); nextVal <= k; ++nextVal)
            memoized.push_back(memoized.back() * nextVal);
        return memoized[k];
    }

    sign_t wireSignature(WireId* wire, int accuracy) {
        sign_t outSig = 0;
        for(auto circ = wire->adjacent_begin();
                circ != wire->adjacent_end();
                ++circ)
        {
            if(accuracy < 0)
                outSig ^= (*circ)->sign();
            else
                outSig ^= (*circ)->sign(accuracy);
        }

        return outSig;
    }

    void splitOnSig(const vector<CircuitTree*> circuits,
            SigSplit& splitted,
            std::vector<sign_t>& signatures,
            int maxPermutations,
            int accuracy)
    {
        splitted.clear();
        signatures.clear();

        map<sign_t, set<CircuitTree*> > wipSplit;

        for(auto circ : circuits) {
            sign_t sig = accuracy >= 0 ? circ->sign(accuracy) : circ->sign();
            wipSplit[sig].insert(circ);

            if(maxPermutations >= 0
                    && factorial(wipSplit[sig].size()) > maxPermutations)
            {
                // Keep it simple: the threshold will be low anyway, and we do
                // not want to divide anything here, it must be fast.
                throw TooManyPermutations();
            }
        }

        splitted.reserve(wipSplit.size());
        signatures.reserve(wipSplit.size());
        for(const auto& assoc : wipSplit) { // Order is deterministic
            signatures.push_back(assoc.first);
            splitted.push_back(vector<CircuitTree*>());
            splitted.back().reserve(assoc.second.size());
            for(const auto& circ : assoc.second)
                splitted.back().push_back(circ);
        }
    }

    bool equalSizes(
            const SigSplit& fst,
            const SigSplit& snd)
    {
        if(fst.size() != snd.size())
            return false;

        for(size_t pos = 0; pos < fst.size(); ++pos) {
            if(fst[pos].size() != snd[pos].size())
                return false;
        }
        return true;
    }

    bool equalWithPermutation(
            const SigSplit& leftSplit, const SigSplit& rightSplit,
            const Permutation& perm)
    {
        // NOTE: here, we assume that keys(leftSplit) == keys(rightSplit)

        // Check sub-equality
        for(size_t pos = 0 ; pos < leftSplit.size(); ++pos) {
            const vector<int>& curPerm = perm[pos];
            for(size_t circId = 0; circId < leftSplit[pos].size(); ++circId) {
                if(!leftSplit[pos][circId]->equals(
                            rightSplit[pos][curPerm[circId]]))
                {
                    EQ_DEBUG("Not sub-equal (types %d, %d)\n",
                            leftSplit[pos][circId]->circType(),
                            rightSplit[pos][curPerm[circId]]->circType());
                    return false;
                }
            }
        }

        // Check graph structure
        unordered_map<WireId*, WireId*> lrWireMap;
        /* This map ^ maps a left-hand WireId to a right-hand one. We want this
         * map to be a bijection in order to have a graph isomorphism, so we
         * simply have to walk through our nodes mapping, map wires in this map
         * (and fail in case of conflict). This ensures the functionnal
         * property (`f(x)` is unique). Then, checking that each `WireId*` has
         * at most one occurrence in the right-hand side of the map ensures
         * injectivity. Surjectivity is then guaranteed by the structure of the
         * circuit itself and the pigeon-hole principle.
         */
        for(size_t pos = 0; pos < leftSplit.size(); ++pos) {
            const vector<int>& curPerm = perm[pos];
            for(size_t circId = 0; circId < leftSplit[pos].size(); ++circId) {
                CircuitTree *left = leftSplit[pos][circId],
                    *right = rightSplit[pos][curPerm[circId]];
                CircuitTree::IoIter lWire = left->io_begin(),
                    rWire = right->io_begin();
                for(; lWire != left->io_end() /* rWire checked after */ ;
                        ++lWire, ++rWire)
                {
                    if(rWire == right->io_end()) { // Prematurate end
                        EQ_DEBUG("Bad wire count (<- %s)\n",
                                (*lWire)->name().c_str());
                        return false;
                    }
                    if(lrWireMap.find(*lWire) != lrWireMap.end()
                            && *(lrWireMap[*lWire]) != **rWire)
                    {
                        EQ_DEBUG("Wire conflict %s -> {%s - %s}\n",
                                (*lWire)->uniqueName().c_str(),
                                lrWireMap.find(*lWire)->second->uniqueName().c_str(),
                                (*rWire)->uniqueName().c_str());
                        return false; // Wire conflict
                    }

                    lrWireMap[*lWire] = *rWire;
                }
                if(rWire != right->io_end()) { // Bad wire count
                    EQ_DEBUG("Bad wire count (-> %s)\n",
                            (*rWire)->name().c_str());
                    return false;
                }
            }
        }
        unordered_set<WireId*> rightSide;
        for(const auto& assoc : lrWireMap) {
            if(rightSide.find(assoc.second) != rightSide.end()) {
                EQ_DEBUG("Non injective\n");
                return false; // Non-injective
            }
            rightSide.insert(assoc.second);
        }

        return true;
    }
}
