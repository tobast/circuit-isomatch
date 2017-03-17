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

        CircuitTree();

        /**
         * Returns whether this tree element is a leaf or a group.
         */
        virtual bool isLeaf() const = 0;

        /**
         * Computes the signature of the circuit. Memoized function, it will
         * only be costy on the first run.
         * freeze must have been called before.
         *
         * @param level Defines the signature level used. Lower means cheaper,
         * but also less precise.
         */
        virtual sig_t sign(int level=2) = 0;

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
        /**
         * Checks whether the circuit is frozen, and fails with `Frozen` if it
         * is.
         */
        void failIfFrozen() const;

        bool frozen;

    private:
        static size_t nextCircuitId;
        size_t circuitId;
};

