#pragma once

#include "circuitTree.h"

class CircuitDelay : public CircuitTree {
    protected:
        // ========= I/O ITERATOR =============================================
        class InnerConstIoIter : public CircuitTree::InnerConstIoIter {
                WireId* ptr;
                const CircuitDelay* circ;
            public:
                InnerConstIoIter(const CircuitDelay* circ, WireId* wire)
                    : ptr(wire), circ(circ) {}
                InnerConstIoIter(const InnerConstIoIter& it)
                    : ptr(it.ptr), circ(it.circ) {}
                virtual void operator++();
                virtual const WireId* operator*() { return ptr; }
                virtual InnerConstIoIter* clone() const {
                    return new InnerConstIoIter(*this);
                }
            protected:
                bool equal(const InnerConstIoIter& oth) const {
                    return ptr == oth.ptr && circ == oth.circ;
                }
        };

    public:
        ConstIoIter inp_begin() const {
            return ConstIoIter(
                    new InnerConstIoIter(this, wireInput)
                    );
        }
        ConstIoIter out_begin() const {
            return ConstIoIter(
                    new InnerConstIoIter(this, wireOutput)
                    );
        }
        ConstIoIter io_end() const {
            return ConstIoIter(
                    new InnerConstIoIter(this, NULL)
                    );
        }
        // ========= END I/O ITERATOR =========================================

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

