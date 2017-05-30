#include "circuitComb.h"
#include "signatureConstants.h"
#include "dotPrint.h"
#include "debug.h"

#include <cassert>
using namespace std;

CircuitComb::InnerIoIter::InnerIoIter(
        const CircuitComb* circ, CircuitComb::InnerIoIter::LowIter lowIter)
    : ptr(lowIter), circ(circ)
{
    if(ptr == circ->gateInputs.end())
        ptr = circ->gateOutputs.begin();
}

void CircuitComb::InnerIoIter::operator++() {
    ++ptr;
    if(ptr == circ->gateInputs.end())
        ptr = circ->gateOutputs.begin();
}

CircuitComb::CircuitComb()
{}

CircuitComb::~CircuitComb() {
    for(auto expr : gateExprs)
        delete expr;
}

void CircuitComb::addInput(WireId* input) {
    gateInputs.push_back(input);
    input->connect(this);
}

void CircuitComb::addOutput(ExpressionBase* expr, WireId* wire) {
    gateOutputs.push_back(wire);
    wire->connect(this);
    gateExprs.push_back(expr);
}

sig_t CircuitComb::innerSignature() const {
    sig_t exprsSum = 0;
    for(auto expr : gateExprs)
        exprsSum ^= expr->sign();

    return signatureConstants::opcst_leaftype(
            ((circType() << 16)
             | (gateInputs.size() << 8)
             | (gateOutputs.size()))
            ^ exprsSum);
}

bool CircuitComb::innerEqual(CircuitTree* othTree) {
    const CircuitComb* oth = dynamic_cast<const CircuitComb*>(othTree);
    if(gateInputs.size() != oth->gateInputs.size()
        || gateOutputs.size() != oth->gateOutputs.size()
        || gateExprs.size() != oth->gateExprs.size())
    {
        EQ_DEBUG("Comb: mismatched sizes\n");
        return false;
    }
    for(size_t inp=0; inp < gateExprs.size(); ++inp) {
        if(!gateExprs[inp]->equals(*oth->gateExprs[inp])) {
            EQ_DEBUG("Comb: mismatched expressions (%s - %s)\n",
                    gateOutputs[inp]->uniqueName().c_str(),
                    oth->gateOutputs[inp]->uniqueName().c_str());
            return false;
        }
    }
    return true;
}

void CircuitComb::toDot(std::basic_ostream<char>& out, int indent) {
    const string thisCirc = string("delay_") + to_string(id());

    dotPrint::indent(out, indent)
        << thisCirc << " "
        << "[shape=octagon, label=\"comb\"]" << '\n';

    for(auto inp : inputs()) {
        dotPrint::indent(out, indent);
        dotPrint::inWire(out, thisCirc, inp->uniqueName(),
                "");
    }
    for(auto outp : outputs()) {
        dotPrint::indent(out, indent);
        dotPrint::outWire(out, thisCirc, outp->uniqueName(),
                "");
    }
}
