#pragma once
#include <exception>
#include <ostream>
#include <iterator>
#include <typeinfo>

#include "wireId.h"

class CircuitTree {
    protected:
        /** Inner `ConstIoIter`, to be reimplemented in derived classes. */
        class InnerConstIoIter :
            public std::iterator<std::forward_iterator_tag, WireId*>
        {
            public:
                virtual ~InnerConstIoIter() {};
                virtual void operator++() {};
                bool operator==(InnerConstIoIter& oth) {
                    return typeid(*this) == typeid(oth) && equal(oth);
                }
                bool operator!=(InnerConstIoIter& oth) {
                    return !(operator==(oth));
                }
                virtual const WireId* operator*() { return nullptr; }
                virtual InnerConstIoIter* clone() const {
                    return new InnerConstIoIter(*this);
                }
            protected:
                /** Checks for equality with its parameter. It can be assumed
                 * that this parameter is of the same type as `*this`, even in
                 * subclassed iterators.
                 */
                virtual bool equal(const InnerConstIoIter&) const {
                    return true;
                }
        };

    public:
        typedef unsigned long long sig_t;

        class Frozen : public std::exception {
            const char* what() {
                return "This circuit is frozen and cannot be altered.";
            }
        };

        class NotFrozen : public std::exception {
            const char* what() {
                return "This circuit must be frozen before memoizing results.";
            }
        };

        enum CircType {
            CIRC_GROUP,
            CIRC_COMB,
            CIRC_DELAY,
            CIRC_TRI,
            CIRC_ASSERT,
        };

        /** Iterator over the WireIds of the diverse circuit gates */
        class ConstIoIter {
            public:
                ConstIoIter() : inner(nullptr) {}
                ConstIoIter(InnerConstIoIter* ptr) : inner(ptr) {}
                ~ConstIoIter() { delete inner; }

                ConstIoIter(const ConstIoIter& oth) :
                    inner(oth.inner->clone()) {}
                ConstIoIter& operator=(const ConstIoIter& oth) {
                    delete inner;
                    inner = oth.inner->clone();
                    return *this;
                }

                ConstIoIter& operator++() {
                    ++(*inner);
                    return *this;
                }
                ConstIoIter operator++(int) {
                    ConstIoIter out(*this);
                    operator++();
                    return out;
                }
                const WireId* operator*() const { return *(*inner); }

            private:
                InnerConstIoIter* inner;
        };

        CircuitTree();
        virtual ~CircuitTree();

        /**
         * Returns the tree element's type
         */
        virtual CircType circType() const = 0;

        /**
         * Computes the signature of the circuit. Memoized function, it will
         * only be costy on the first run.
         * The circuit must be frozen.
         *
         * @param level Defines the signature level used. Lower means cheaper,
         * but also less precise.
         */
         sig_t sign(int level=2);

        /**
         * Freezes the circuit forever: any function modifying its structure
         * will fail with a Frozen exception.
         * This is required before computing any memoized result dependning on
         * the circuit's structure.
         */
        void freeze() { frozen = true; }

        /**
         * O(1) comparaison using IDs
         */
        bool operator==(const CircuitTree& oth) const {
            return circuitId == oth.circuitId;
        }

        /** Get the parent group of this circuit. This might be `NULL` if the
         * group has no parent (yet). */
        CircuitGroup* ancestor() { return ancestor_; }

        /** Get this circuit's id */
        size_t id() const { return circuitId; }

        /** Get an iterator to the first input wire */
        virtual ConstIoIter inp_begin() const = 0;

        /** Get an iterator to the end of input wires */
        ConstIoIter inp_end() const { return out_begin(); }

        /** Get an iterator to the first output wire */
        virtual ConstIoIter out_begin() const = 0;

        /** Get an iterator to the end of output wires */
        virtual ConstIoIter out_end() const = 0;

        /** Get an iterator to the first I/O wire */
        ConstIoIter io_begin() const { return inp_begin(); }

        /** Get an iterator to the end of output wires */
        ConstIoIter io_end() const { return out_end(); }

        /** Generates a Dot representation of the circuit, primarily intended
         * for debugging. */
        virtual void toDot(std::basic_ostream<char>& out, int indent=0) = 0;

    protected:
        /** Computes the actual signature of the circuit when it was not
         * previously memoized.
         * You should call `sign` when overriding this function and needing a
         * lower-level signature of a block. */
        virtual sig_t computeSignature(int level) = 0;

        /**
         * Checks whether the circuit is frozen, and fails with `Frozen` if it
         * is.
         */
        void failIfFrozen() const;

        /**
         * Checks whether the circuit is frozen, and fails with `NotFrozen` if
         * it is not.
         */
        void failIfNotFrozen() const;

        bool frozen;
        std::vector<sig_t> memoSig;

        /** Group this circuit belongs to. This is automatically set. */
        CircuitGroup* ancestor_;

    private:
        static size_t nextCircuitId;
        size_t circuitId;

    friend CircuitGroup; // FIXME? To allow changing `ancestor_`
};

