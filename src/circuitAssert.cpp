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
    return signatureConstants::opcst_leaftype(
            ((circType() << 16) | (gateInputs.size() << 8))
            + gateExpr->sign());
}

bool CircuitAssert::innerEqual(const CircuitTree* othTree) const {
    const CircuitAssert* oth = dynamic_cast<const CircuitAssert*>(othTree);
    return gateInputs.size() == oth->gateInputs.size()
        && gateExpr->equals(*oth->gateExpr);
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

