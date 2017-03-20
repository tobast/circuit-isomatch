#pragma once

#include "circuitTree.h"

class CircuitTristate : public CircuitTree {
    public:
        CircuitTristate(WireId* from, WireId* to, WireId* enable);

        CircType circType() const { return CIRC_TRI; }

        sig_t sign(int level=2);

        /** Gets the input wire. */
        const WireId* input() const { return wireInput; }

        /** Gets the output wire. */
        const WireId* output() const { return wireOutput; }

        /** Gets the enable wire. */
        const WireId* enable() const { return wireEnable; }

    private:
        WireId *wireInput, *wireOutput, *wireEnable;
        std::vector<sig_t> memoSig;
};

