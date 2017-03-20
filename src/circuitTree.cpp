#include "circuitTree.h"

using namespace std;

size_t CircuitTree::nextCircuitId = 0;

CircuitTree::CircuitTree() : frozen(false), circuitId(nextCircuitId) {
    nextCircuitId++;
}

void CircuitTree::failIfFrozen() const {
    if(frozen)
        throw Frozen();
}

void CircuitTree::failIfNotFrozen() const {
    if(!frozen)
        throw NotFrozen();
}

