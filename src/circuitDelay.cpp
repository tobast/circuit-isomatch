#include "circuitDelay.h"
#include "dotPrint.h"

#include <cassert>
#include <string>
using namespace std;

CircuitDelay::CircuitDelay(WireId* from, WireId* to) {
    from->connect(this);
    to->connect(this);
}

CircuitDelay::sig_t CircuitDelay::computeSignature(int level) {
    (&level); // UNUSED
    assert(false); // TODO implement
}

void CircuitDelay::toDot(std::basic_ostream<char>& out, int indent) {
    const string thisCirc = string("delay_") + to_string(id());

    dotPrint::indent(out, indent)
        << thisCirc << " "
        << "[shape=triangle, rotate=90]" << '\n';

    dotPrint::inWire(out, thisCirc, wireInput->uniqueName(),
            "headport=w");
    dotPrint::indent(out, indent);
    dotPrint::outWire(out, thisCirc, wireOutput->uniqueName(),
            "headport=e");
}

