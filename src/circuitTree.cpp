#include "circuitTree.h"

using namespace std;

size_t CircuitTree::nextCircuitId = 0;

CircuitTree::CircuitTree() : circuitId(nextCircuitId), frozen(false) {
    nextCircuitId++;
}

void CircuitTree::failIfFrozen() const {
    if(frozen)
        throw Frozen();
}

