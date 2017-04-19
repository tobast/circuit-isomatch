#include "circuitAssert.h"
#include "signatureConstants.h"
#include "dotPrint.h"

#include <cassert>
#include <string>

using namespace std;

void CircuitAssert::InnerIoIter::operator++() {
    ++ptr;
}

CircuitAssert::CircuitAssert(const std::string& name,
        ExpressionBase* expr) :
    name(name), gateExpr(expr)
{}

CircuitAssert::~CircuitAssert() {
    delete gateExpr;
}

void CircuitAssert::addInput(WireId* wire) {
    failIfFrozen();
    gateInputs.push_back(wire);
}

sig_t CircuitAssert::innerSignature() const {
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

