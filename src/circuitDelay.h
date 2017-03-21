#pragma once

#include "circuitTree.h"

class CircuitDelay : public CircuitTree {
    public:
        CircuitDelay(WireId* from, WireId* to);

        CircType circType() const { return CIRC_DELAY; }

        /** Gets the input wire. */
        const WireId* input() const { return wireInput; }

        /** Gets the output wire. */
        const WireId* output() const { return wireOutput; }

    protected:
        sig_t computeSignature(int level);

    private:
        WireId *wireInput, *wireOutput;
};

