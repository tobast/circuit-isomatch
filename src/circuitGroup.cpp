#include "circuitGroup.h"
#include <cassert>

CircuitGroup::CircuitGroup(const std::string& name) :
    CircuitTree(), name(name)
{}

CircuitGroup::sig_t CircuitGroup::sign(int level) {
    if(!frozen)
        throw NotFrozen();
    if(level < (int)memoSig.size())
        return memoSig[level];

    assert(false); //TODO implement
    return 0;
}

void CircuitGroup::freeze() {
    for(auto child: grpChildren)
        child->freeze();
    CircuitTree::freeze();
}

void CircuitGroup::addChild(CircuitTree* child) {
    failIfFrozen();
    grpChildren.push_back(child);
}

void CircuitGroup::addInput(const CircuitGroup::IOPin& pin) {
    failIfFrozen();
    grpInputs.push_back(pin);
}

void CircuitGroup::addOutput(const CircuitGroup::IOPin& pin) {
    failIfFrozen();
    grpOutputs.push_back(pin);
}

std::vector<CircuitTree*>& CircuitGroup::getChildren() {
    failIfFrozen();
    return grpChildren;
}
const std::vector<CircuitTree*>& CircuitGroup::getChildren() const {
    return grpChildren;
}

std::vector<CircuitGroup::IOPin>& CircuitGroup::getInputs() {
    failIfFrozen();
    return grpInputs;
}
const std::vector<CircuitGroup::IOPin>& CircuitGroup::getInputs() const {
    return grpInputs;
}

std::vector<CircuitGroup::IOPin>& CircuitGroup::getOutputs() {
    failIfFrozen();
    return grpOutputs;
}
const std::vector<CircuitGroup::IOPin>& CircuitGroup::getOutputs() const {
    return grpOutputs;
}

