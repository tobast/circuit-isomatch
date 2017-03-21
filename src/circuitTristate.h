#pragma once

#include "circuitTree.h"

class CircuitTristate : public CircuitTree {
    public:
        CircuitTristate(WireId* from, WireId* to, WireId* enable);

        CircType circType() const { return CIRC_TRI; }

        /** Gets the input wire. */
        const WireId* input() const { return wireInput; }

        /** Gets the output wire. */
        const WireId* output() const { return wireOutput; }

        /** Gets the enable wire. */
        const WireId* enable() const { return wireEnable; }

    protected:
        sig_t computeSignature(int level);

    private:
        WireId *wireInput, *wireOutput, *wireEnable;
};

