#include "circuitComb.h"
#include "signatureConstants.h"
#include "dotPrint.h"

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
        exprsSum += expr->sign();

    return signatureConstants::opcst_leaftype(
            ((circType() << 16)
             | (gateInputs.size() << 8)
             | (gateOutputs.size()))
            + exprsSum);
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

void CircuitComb::serialize_body(std::basic_ostream<char>& out) {
    char delim=' ';
    out << "\"inp\":[";
    for(auto elt: gateInputs) {
        out << delim << "\"" << elt->name() << "\"";
        delim = ',';
    }

    out << "],\"out\":[";
    delim=' ';
    for(auto elt: gateOutputs) {
        out << delim << "\"" << elt->name() << "\"";
        delim = ',';
    }

    out << "],\"exprs\":[";
    delim=' ';
    for(auto expr: gateExprs) {
        out << delim;
        expr->serialize(out);
        delim = ',';
    }
    out << "]";
}
