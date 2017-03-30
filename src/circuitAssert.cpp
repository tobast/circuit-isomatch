#include "circuitAssert.h"
#include "dotPrint.h"

#include <cassert>
#include <string>

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

void CircuitAssert::toDot(std::basic_ostream<char>& out, int indent) {
    const string thisCirc = string("assert_") + to_string(id());

    dotPrint::indent(out, indent)
        << thisCirc << " "
        << " [label=\"assert\"]" << '\n';

    for(auto input : inputs()) {
        dotPrint::indent(out, indent);
        dotPrint::inWire(out, thisCirc, input->uniqueName());
    }
}

