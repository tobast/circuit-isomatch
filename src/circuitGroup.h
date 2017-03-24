#pragma once
#include <string>
#include <vector>
#include <exception>

#include "wireId.h"
#include "circuitTree.h"

/** Input/output pin for a `CircuitGroup` */
class IOPin {
    public:
        class AlreadyConnected : std::exception {};

        IOPin(WireId* formal, WireId* actual, CircuitGroup* group);

        /** Partially connect this pin, leave its outter part exposed for later
         * `connect`.
         */
        IOPin(std::string formalName, WireId* actual, CircuitGroup* group);

        /** Connects the outter part of the pin to the given wire.
         * @throw AlreadyConnected if the pin is already connected.
         */
        void connect(WireId* formal);

        WireId* formal() const { return _formal; }
        std::string formalName() const { return _formalName; }
        WireId* actual() const { return _actual; }
        CircuitGroup* group() const { return _group; }

    private:
        WireId* _formal;
        std::string _formalName; /// Used if formal is not yet connected
        WireId* _actual;
        CircuitGroup* _group;
};


class CircuitGroup : public CircuitTree {
    public:
        CircuitGroup(const std::string& name);

        CircType circType() const { return CIRC_GROUP; }

        /**
         * Freezes the circuit as `CircuitTree::freeze` does, but also freezes
         * all the children of this group.
         */
        void freeze();

        /**
         * Adds `child` as a child of this group. All external pins of `child`
         * are disconnected, and reconnected to this group's corresponding
         * wires during the process.
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

    protected:
        sig_t computeSignature(int level);

    private:
        std::string name;

        std::vector<CircuitTree*> grpChildren;
        std::vector<IOPin> grpInputs, grpOutputs;
};

