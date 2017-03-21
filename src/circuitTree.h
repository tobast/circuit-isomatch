#pragma once
#include <exception>

#include "wireId.h"

class CircuitTree {
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

        CircuitTree();

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

    protected:
        /** Computes the actual signature of the circuit when it was not
         * previously memoized.
         * Computing the signature of level `n` requires the signature of level
         * `n-1`, this function is thus expected to call `sign` whenever
         * `level > 0`. */
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

    private:
        static size_t nextCircuitId;
        size_t circuitId;
};

