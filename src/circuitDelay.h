#pragma once

#include "circuitTree.h"

class CircuitDelay : public CircuitTree {
    public:
        CircuitDelay(WireId* from, WireId* to);

        CircType circType() const { return CIRC_DELAY; }

        sig_t sign(int level=2);

        /** Gets the input wire. */
        const WireId* input() const { return wireInput; }

        /** Gets the output wire. */
        const WireId* output() const { return wireOutput; }

    private:
        WireId *wireInput, *wireOutput;
        std::vector<sig_t> memoSig;
};

