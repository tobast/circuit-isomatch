#include "circuitTree.h"
#include <cassert>

using namespace std;

size_t CircuitTree::nextCircuitId = 0;

CircuitTree::CircuitTree() : frozen(false), circuitId(nextCircuitId) {
    nextCircuitId++;
}

CircuitTree::sig_t CircuitTree::sign(int level) {
    failIfNotFrozen();
    if(level < (int)memoSig.size())
        return memoSig[level];

    sig_t signature = computeSignature(level);
    assert((int)memoSig.size() == level);
    memoSig.push_back(signature);
    return signature;
}

void CircuitTree::failIfFrozen() const {
    if(frozen)
        throw Frozen();
}

void CircuitTree::failIfNotFrozen() const {
    if(!frozen)
        throw NotFrozen();
}

