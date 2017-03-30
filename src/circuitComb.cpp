#include "circuitComb.h"
#include "dotPrint.h"

#include <cassert>
using namespace std;

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

