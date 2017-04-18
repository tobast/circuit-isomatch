#include "circuitTristate.h"
#include "dotPrint.h"

#include <string>
#include <cassert>
using namespace std;

void CircuitTristate::InnerIoIter::operator++() {
    if(ptr == circ->wireInput)
        ptr = circ->wireEnable;
    else if(ptr == circ->wireEnable)
        ptr = circ->wireOutput;
    else if(ptr == circ->wireOutput)
        ptr = NULL;
}

CircuitTristate::CircuitTristate(WireId* from, WireId* to, WireId* enable) :
    wireInput(from), wireOutput(to), wireEnable(enable)
{
    from->connect(this);
    to->connect(this);
    enable->connect(this);
}

CircuitTristate::sig_t CircuitTristate::computeSignature(int level) {
    (&level); // UNUSED
    assert(false); // TODO implement
}

void CircuitTristate::toDot(std::basic_ostream<char>& out, int indent) {
    const string thisCirc = string("tri_") + to_string(id());

    dotPrint::indent(out, indent)
        << thisCirc << " "
        << "[shape=triangle, rotate=90]" << '\n';

    dotPrint::inWire(out, thisCirc, wireInput->uniqueName(),
            "headport=w");
    dotPrint::indent(out, indent);
    dotPrint::inWire(out, thisCirc, wireEnable->uniqueName(),
            "headport=n");
    dotPrint::indent(out, indent);
    dotPrint::outWire(out, thisCirc, wireOutput->uniqueName(),
            "headport=e");
}

