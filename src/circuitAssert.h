#pragma once

#include <string>

#include "circuitTree.h"
#include "gateExpression.h"

class CircuitAssert : public CircuitTree {
    public:
        CircuitAssert(const std::string& name, const ExpressionBase& expr);

        CircType circType() const { return CIRC_ASSERT; }

        /** Adds `wire` as the next input for this gate.
         * Requires the gate to be unfrozen.
         */
        void addInput(WireId* wire);

        /** Gate's inputs */
        const std::vector<WireId*>& inputs() const { return gateInputs; }

        /** Gate's expression */
        const ExpressionBase& expression() const { return gateExpr; }

        void toDot(std::basic_ostream<char>& out, int indent=0);

    protected:
        sig_t computeSignature(int level);

    private:
        std::string name;
        std::vector<WireId*> gateInputs;
        ExpressionBase gateExpr;
};

