#pragma once
#include <string>
#include <vector>

#include "circuitTree.h"

class CircuitGroup : public CircuitTree {
    public:
        struct IOPin {
            WireId formal;
            WireId actual;
        };

        CircuitGroup(const std::string& name);

        bool isLeaf() const { return false; }
        sig_t sign(int level=2);

        /**
         * Freezes the circuit as `CircuitTree::freeze` does, but also freezes
         * all the children of this group.
         */
        void freeze();

        /**
         * Adds `child` as a child of this group.
         * Requires the group to be unfrozen.
         */
        void addChild(CircuitTree* child);

        /**
         * Adds `pin` as input pin of this group.
         * Requires the group to be unfrozen.
         */
        void addInput(const IOPin& pin);

        /**
         * Adds `pin` as output pin of this group.
         * Requires the group to be unfrozen.
         */
        void addOutput(const IOPin& pin);

        /**
         * Group's subcircuits, mutable.
         * Requires the group to be unfrozen.
         */
        std::vector<CircuitTree*>& getChildren();
        /**
         * Group's subcircuits
         */
        const std::vector<CircuitTree*>& getChildren() const;

        /** Group's inputs, mutable.
         * Requires the group to be unfrozen.
         */
        std::vector<IOPin>& getInputs();
        /** Group's inputs */
        const std::vector<IOPin>& getInputs() const;

        /** Group's outputs, mutable.
         * Requires the group to be unfrozen.
         */
        std::vector<IOPin>& getOutputs();
        /** Group's outputs */
        const std::vector<IOPin>& getOutputs() const;

    private:
        std::string name;
        std::vector<sig_t> memoSig;

        std::vector<CircuitTree*> grpChildren;
        std::vector<IOPin> grpInputs, grpOutputs;
};

