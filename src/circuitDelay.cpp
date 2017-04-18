#include "circuitDelay.h"
#include "signatureConstants.h"
#include "dotPrint.h"

#include <cassert>
#include <string>
using namespace std;


void CircuitDelay::InnerIoIter::operator++() {
    if(ptr == circ->wireInput)
        ptr = circ->wireOutput;
    if(ptr == circ->wireOutput)
        ptr = NULL;
}

CircuitDelay::CircuitDelay(WireId* from, WireId* to) :
        wireInput(from), wireOutput(to)
{
    assert(from != NULL && to != NULL);
    from->connect(this);
    to->connect(this);
}

CircuitDelay::sig_t CircuitDelay::innerSignature() const {
    return signatureConstants::opcst_leaftype(
            (circType() << 16) + (1 << 8) + 1);
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

