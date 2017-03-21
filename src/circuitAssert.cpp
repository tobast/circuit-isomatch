#include "circuitAssert.h"

using namespace std;

CircuitAssert::CircuitAssert(const std::string& name,
        const ExpressionBase& expr) :
    name(name), gateExpr(expr)
{}

void CircuitAssert::addInput(WireId* wire) {
    failIfFrozen();
    gateInputs.push_back(wire);
}

