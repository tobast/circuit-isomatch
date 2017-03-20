#include "circuitDelay.h"

#include <cassert>

CircuitDelay::CircuitDelay(WireId* from, WireId* to) {
    from->connect(this);
    to->connect(this);
}

CircuitDelay::sig_t CircuitDelay::sign(int level) {
    failIfNotFrozen();
    if(level < (int)memoSig.size())
        return memoSig[level];

    assert(false); // TODO implement
}

