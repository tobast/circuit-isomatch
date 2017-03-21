#include "circuitAssert.h"

#include <cassert>

using namespace std;

CircuitAssert::CircuitAssert(const std::string& name,
        const ExpressionBase& expr) :
    name(name), gateExpr(expr)
{}

void CircuitAssert::addInput(WireId* wire) {
    failIfFrozen();
    gateInputs.push_back(wire);
}

CircuitAssert::sig_t CircuitAssert::computeSignature(int level) {
    (&level); // UNUSED
    assert(false); //TODO implement
}

