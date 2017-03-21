#include "circuitDelay.h"

#include <cassert>

CircuitDelay::CircuitDelay(WireId* from, WireId* to) {
    from->connect(this);
    to->connect(this);
}

CircuitDelay::sig_t CircuitDelay::computeSignature(int level) {
    (&level); // UNUSED
    assert(false); // TODO implement
}

