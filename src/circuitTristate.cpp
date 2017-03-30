#include "circuitTristate.h"
#include "dotPrint.h"

#include <string>
#include <cassert>
using namespace std;

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

