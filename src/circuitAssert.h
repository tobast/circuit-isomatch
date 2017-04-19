#pragma once

#include <string>

#include "circuitTree.h"
#include "gateExpression.h"

class CircuitAssert : public CircuitTree {
    protected:
        // ========= I/O ITERATOR =============================================
        class InnerIoIter : public CircuitTree::InnerIoIter {
                typedef std::vector<WireId*>::const_iterator LowIter;
                LowIter ptr;
            public:
                InnerIoIter(LowIter lowIter) : ptr(lowIter) {}
                InnerIoIter(const InnerIoIter& it)
                    : ptr(it.ptr) {}
                virtual void operator++();
                virtual WireId* operator*() { return *ptr; }
                virtual InnerIoIter* clone() const {
                    return new InnerIoIter(*this);
                }

            protected:
                virtual bool equal(const InnerIoIter& oth) const {
                    return ptr == oth.ptr;
                }
        };

    public:
        IoIter inp_begin() const {
            return IoIter(
                    new InnerIoIter(gateInputs.begin())
                    );
        }
        IoIter out_begin() const {
            return IoIter(
                    new InnerIoIter(gateInputs.end())
                    );
        }
        IoIter io_end() const {
            return out_begin();
        }
        // ========= END I/O ITERATOR =========================================

        CircuitAssert(const std::string& name, ExpressionBase* expr);

        /** Deletes the gate's expression */
        virtual ~CircuitAssert();

        CircType circType() const { return CIRC_ASSERT; }

        /** Adds `wire` as the next input for this gate.
         * Requires the gate to be unfrozen.
         */
        void addInput(WireId* wire);

        /** Gate's inputs */
        const std::vector<WireId*>& inputs() const { return gateInputs; }

        /** Gate's expression */
        const ExpressionBase* expression() const { return gateExpr; }

        void toDot(std::basic_ostream<char>& out, int indent=0);

    protected:
        virtual sig_t innerSignature() const;

    private:
        std::string name;
        std::vector<WireId*> gateInputs;
        ExpressionBase* gateExpr;
};

