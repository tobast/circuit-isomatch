#pragma once
#include <string>
#include <vector>

#include "wireId.h"
#include "circuitTree.h"

class IOPin {
    public:
        IOPin(WireId* formal, WireId* actual, CircuitGroup* group);

        WireId* formal() const { return _formal; }
        WireId* actual() const { return _actual; }
        CircuitGroup* group() const { return _group; }
    private:
        WireId* _formal;
        WireId* _actual;
        CircuitGroup* _group;
};


class CircuitGroup : public CircuitTree {
    public:
        CircuitGroup(const std::string& name);

        CircType circType() const { return CIRC_GROUP; }
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

