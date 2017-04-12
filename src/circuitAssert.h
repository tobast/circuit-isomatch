#pragma once

#include <string>

#include "circuitTree.h"
#include "gateExpression.h"

class CircuitAssert : public CircuitTree {
    protected:
        // ========= I/O ITERATOR =============================================
        class InnerConstIoIter : public CircuitTree::InnerConstIoIter {
                typedef std::vector<WireId*>::const_iterator LowIter;
                LowIter ptr;
            public:
                InnerConstIoIter(LowIter lowIter) : ptr(lowIter) {}
                InnerConstIoIter(const InnerConstIoIter& it)
                    : ptr(it.ptr) {}
                virtual void operator++();
                virtual const WireId* operator*() { return *ptr; }
                virtual InnerConstIoIter* clone() const {
                    return new InnerConstIoIter(*this);
                }

            protected:
                virtual bool equal(const InnerConstIoIter& oth) const {
                    return ptr == oth.ptr;
                }
        };

    public:
        ConstIoIter inp_begin() const {
            return ConstIoIter(
                    new InnerConstIoIter(gateInputs.begin())
                    );
        }
        ConstIoIter out_begin() const {
            return ConstIoIter(
                    new InnerConstIoIter(gateInputs.end())
                    );
        }
        ConstIoIter io_end() const {
            return out_begin();
        }
        // ========= END I/O ITERATOR =========================================

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

