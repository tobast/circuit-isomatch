#pragma once

#include "circuitTree.h"
#include "gateExpression.h"

class CircuitComb : public CircuitTree {
    protected:
        // ========= I/O ITERATOR =============================================
        class InnerConstIoIter : public CircuitTree::InnerConstIoIter {
                typedef std::vector<WireId*>::const_iterator LowIter;
                LowIter ptr;
                const CircuitComb* circ;
            public:
                InnerConstIoIter(const CircuitComb* circ, LowIter lowIter)
                    : ptr(lowIter), circ(circ) {}
                InnerConstIoIter(const InnerConstIoIter& it)
                    : ptr(it.ptr) {}
                virtual void operator++();
                virtual const WireId* operator*() { return *ptr; }
                virtual InnerConstIoIter* clone() const {
                    return new InnerConstIoIter(*this);
                }
            protected:
                virtual bool equal(const InnerConstIoIter& oth) const {
                    return circ == oth.circ && ptr == oth.ptr;
                }
        };

    public:
        ConstIoIter inp_begin() const {
            return ConstIoIter(
                    new InnerConstIoIter(this, gateInputs.begin())
                    );
        }
        ConstIoIter out_begin() const {
            return ConstIoIter(
                    new InnerConstIoIter(this, gateOutputs.begin())
                    );
        }
        ConstIoIter io_end() const {
            return ConstIoIter(
                    new InnerConstIoIter(this, gateOutputs.end())
                    );
        }
        // ========= END I/O ITERATOR =========================================

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

        void toDot(std::ostream& out, int indent=0);

    protected:
        sig_t computeSignature(int level);

    private:
        std::vector<WireId*> gateInputs, gateOutputs;
        std::vector<ExpressionBase> gateExprs;
};

