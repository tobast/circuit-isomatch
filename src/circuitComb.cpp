#include "circuitComb.h"

#include <cassert>

CircuitComb::CircuitComb()
{}

void CircuitComb::addInput(WireId* input) {
    gateInputs.push_back(input);
    input->connect(this);
}

void CircuitComb::addOutput(const ExpressionBase& expr, WireId* wire) {
    gateOutputs.push_back(wire);
    wire->connect(this);
    gateExprs.push_back(expr);
}

CircuitComb::sig_t CircuitComb::computeSignature(int level) {
    (&level); // UNUSED
    assert(false); // TODO implement
}

