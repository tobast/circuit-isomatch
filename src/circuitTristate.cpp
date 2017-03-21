#include "circuitTristate.h"
#include <cassert>

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

