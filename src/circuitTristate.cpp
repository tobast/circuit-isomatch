#include "circuitTristate.h"
#include <cassert>

CircuitTristate::CircuitTristate(WireId* from, WireId* to, WireId* enable) :
    wireInput(from), wireOutput(to), wireEnable(enable)
{
    from->connect(this);
    to->connect(this);
    enable->connect(this);
}

CircuitTristate::sig_t CircuitTristate::sign(int level) {
    failIfNotFrozen();
    if(level < (int)memoSig.size())
        return memoSig[level];

    assert(false); // TODO implement
}

