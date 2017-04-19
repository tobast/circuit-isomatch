#include "circuitComb.h"
#include "signatureConstants.h"
#include "dotPrint.h"

#include <cassert>
using namespace std;

void CircuitComb::InnerIoIter::operator++() {
    ++ptr;
    if(ptr == circ->gateInputs.end())
        ptr = circ->gateOutputs.begin();
}

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

sig_t CircuitComb::innerSignature() const {
    assert(false); // TODO implement
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

