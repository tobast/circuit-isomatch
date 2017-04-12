#pragma once

#include "circuitTree.h"

class CircuitTristate : public CircuitTree {
    protected:
        // ========= I/O ITERATOR =============================================
        class InnerConstIoIter : public CircuitTree::InnerConstIoIter {
                WireId* ptr;
                const CircuitTristate* circ;
            public:
                InnerConstIoIter(const CircuitTristate* circ, WireId* wire)
                    : ptr(wire), circ(circ) {}
                InnerConstIoIter(const InnerConstIoIter& it)
                    : ptr(it.ptr), circ(it.circ) {}
                virtual void operator++();
                virtual const WireId* operator*() { return ptr; }
                virtual InnerConstIoIter* clone() const {
                    return new InnerConstIoIter(*this);
                }
            private:
                virtual bool equal(const InnerConstIoIter& oth) const {
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

        CircuitTristate(WireId* from, WireId* to, WireId* enable);

        CircType circType() const { return CIRC_TRI; }

        /** Gets the input wire. */
        const WireId* input() const { return wireInput; }

        /** Gets the output wire. */
        const WireId* output() const { return wireOutput; }

        /** Gets the enable wire. */
        const WireId* enable() const { return wireEnable; }

        void toDot(std::basic_ostream<char>& out, int indent=0);

    protected:
        sig_t computeSignature(int level);

    private:
        WireId *wireInput, *wireOutput, *wireEnable;
};

