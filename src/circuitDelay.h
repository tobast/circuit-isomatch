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

        void toDot(std::basic_ostream<char>& out, int indent=0);

    protected:
        sig_t computeSignature(int level);

    private:
        WireId *wireInput, *wireOutput;
};

