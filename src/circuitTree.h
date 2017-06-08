#pragma once
#include <exception>
#include <ostream>
#include <iterator>
#include <typeinfo>

#include "signatureConstants.h"
#include "wireId.h"

class CircuitTree {
    protected:
        /** Inner `ConstIoIter`, to be reimplemented in derived classes. */
        class InnerIoIter :
            public std::iterator<std::forward_iterator_tag, WireId*>
        {
            public:
                virtual ~InnerIoIter() {};
                virtual void operator++() {};
                bool operator==(InnerIoIter& oth) {
                    return typeid(*this) == typeid(oth) && equal(oth);
                }
                bool operator!=(InnerIoIter& oth) {
                    return !(operator==(oth));
                }
                virtual WireId* operator*() { return nullptr; }
                virtual InnerIoIter* clone() const {
                    return new InnerIoIter(*this);
                }
            protected:
                /** Checks for equality with its parameter. It can be assumed
                 * that this parameter is of the same type as `*this`, even in
                 * subclassed iterators.
                 */
                virtual bool equal(const InnerIoIter&) const {
                    return true;
                }
        };

    public:
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
        class IoIter {
            public:
                IoIter() : inner(nullptr) {}
                IoIter(InnerIoIter* ptr) : inner(ptr) {}
                ~IoIter() { delete inner; }

                IoIter(const IoIter& oth) :
                    inner(oth.inner->clone()) {}
                IoIter& operator=(const IoIter& oth) {
                    delete inner;
                    inner = oth.inner->clone();
                    return *this;
                }

                IoIter& operator++() {
                    ++(*inner);
                    return *this;
                }
                IoIter operator++(int) {
                    IoIter out(*this);
                    operator++();
                    return out;
                }
                WireId* operator*() const { return *(*inner); }

                bool operator==(const IoIter& oth) const {
                    return inner == oth.inner || (*inner) == (*oth.inner);
                }

                bool operator!=(const IoIter& oth) const {
                    return !(operator==(oth));
                }

            private:
                InnerIoIter* inner;
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
        sign_t sign(int level=2);

        /**
         * Checks whether this circuit is formally equal to its argument, wrt.
         * permutations, names, etc. This does not take into account the gate's
         * I/O, but only its internal structure.
         */
        bool equals(CircuitTree* oth);

        /**
         * Freezes the circuit forever: any function modifying its structure
         * will fail with a Frozen exception.
         * This is required before computing any memoized result dependning on
         * the circuit's structure.
         */
        virtual void freeze() { frozen = true; }

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
        virtual IoIter inp_begin() const = 0;

        /** Get an iterator to the end of input wires */
        IoIter inp_end() const { return out_begin(); }

        /** Get an iterator to the first output wire */
        virtual IoIter out_begin() const = 0;

        /** Get an iterator to the end of output wires */
        virtual IoIter out_end() const = 0;

        /** Get an iterator to the first I/O wire */
        IoIter io_begin() const { return inp_begin(); }

        /** Get an iterator to the end of output wires */
        IoIter io_end() const { return out_end(); }

        /** Generates a Dot representation of the circuit, primarily intended
         * for debugging. */
        virtual void toDot(std::basic_ostream<char>& out, int indent=0) = 0;

    protected:
        /** Computes the actual signature of the circuit when it was not
         * previously memoized.
         * You should call `sign` when overriding this function and needing a
         * lower-level signature of a block. */
        virtual sign_t computeSignature(int level);

        /** Computes the inner signature of a gate. This should be
         * reimplemented for every gate type. */
        virtual sign_t innerSignature() const = 0;

        /** Computes the actual equality of two gates, assumed of the same type
         */
        virtual bool innerEqual(CircuitTree* othTree) = 0;

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
        std::vector<sign_t> memoSig;

        /** Group this circuit belongs to. This is automatically set. */
        CircuitGroup* ancestor_;

    private:
        static size_t nextCircuitId;
        size_t circuitId;

    friend CircuitGroup;
};

