#pragma once

#include "circuitTree.h"
#include "gateExpression.h"

class CircuitComb : public CircuitTree {
    public:
        CircuitComb();

        CircType circType() const { return CIRC_COMB; }

        /** Adds `wire` as the next input for this gate.
         * Requires the gate to be unfrozen.
         */
        void addInput(WireId* wire);

        /** Adds `expr` as the expression for the next output wire, `out`.
         * Requires the gate to be unfrozen.
         */
        void addOutput(const ExpressionBase& expr, WireId* wire);

        /** Gate's inputs */
        const std::vector<WireId*>& inputs() const { return gateInputs; }

        /** Gate's outputs */
        const std::vector<WireId*>& outputs() const { return gateOutputs; }

        /** Gate's expressions */
        const std::vector<ExpressionBase>& expressions() const {
            return gateExprs;
        }

    protected:
        sig_t computeSignature(int level);

    private:
        std::vector<WireId*> gateInputs, gateOutputs;
        std::vector<ExpressionBase> gateExprs;
};

