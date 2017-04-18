#pragma once

#include "circuitTree.h"

class CircuitTristate : public CircuitTree {
    protected:
        // ========= I/O ITERATOR =============================================
        class InnerIoIter : public CircuitTree::InnerIoIter {
                WireId* ptr;
                const CircuitTristate* circ;
            public:
                InnerIoIter(const CircuitTristate* circ, WireId* wire)
                    : ptr(wire), circ(circ) {}
                InnerIoIter(const InnerIoIter& it)
                    : ptr(it.ptr), circ(it.circ) {}
                virtual void operator++();
                virtual WireId* operator*() { return ptr; }
                virtual InnerIoIter* clone() const {
                    return new InnerIoIter(*this);
                }
            private:
                virtual bool equal(const InnerIoIter& oth) const {
                    return ptr == oth.ptr && circ == oth.circ;
                }
        };

    public:
        IoIter inp_begin() const {
            return IoIter(
                    new InnerIoIter(this, wireInput)
                    );
        }
        IoIter out_begin() const {
            return IoIter(
                    new InnerIoIter(this, wireOutput)
                    );
        }
        IoIter io_end() const {
            return IoIter(
                    new InnerIoIter(this, NULL)
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
        virtual sig_t innerSignature() const;

    private:
        WireId *wireInput, *wireOutput, *wireEnable;
};

