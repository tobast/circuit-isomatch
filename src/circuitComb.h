#pragma once

#include "circuitTree.h"
#include "gateExpression.h"

class CircuitComb : public CircuitTree {
    protected:
        // ========= I/O ITERATOR =============================================
        class InnerIoIter : public CircuitTree::InnerIoIter {
                typedef std::vector<WireId*>::const_iterator LowIter;
                LowIter ptr;
                const CircuitComb* circ;
            public:
                InnerIoIter(const CircuitComb* circ, LowIter lowIter)
                    : ptr(lowIter), circ(circ) {}
                InnerIoIter(const InnerIoIter& it)
                    : ptr(it.ptr) {}
                virtual void operator++();
                virtual WireId* operator*() { return *ptr; }
                virtual InnerIoIter* clone() const {
                    return new InnerIoIter(*this);
                }
            protected:
                virtual bool equal(const InnerIoIter& oth) const {
                    return circ == oth.circ && ptr == oth.ptr;
                }
        };

    public:
        IoIter inp_begin() const {
            return IoIter(
                    new InnerIoIter(this, gateInputs.begin())
                    );
        }
        IoIter out_begin() const {
            return IoIter(
                    new InnerIoIter(this, gateOutputs.begin())
                    );
        }
        IoIter io_end() const {
            return IoIter(
                    new InnerIoIter(this, gateOutputs.end())
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
        virtual sig_t innerSignature() const;

    private:
        std::vector<WireId*> gateInputs, gateOutputs;
        std::vector<ExpressionBase> gateExprs;
};

