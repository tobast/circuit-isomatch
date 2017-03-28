#include "circuitGroup.h"
#include <cassert>

IOPin::IOPin(WireId* formal,
        WireId* actual,
        CircuitGroup* group) :
    _formal(NULL), _actual(actual), _group(group)
{
    connect(formal);
}

IOPin::IOPin(std::string formalName, WireId* actual, CircuitGroup* group) :
    _formal(NULL), _formalName(formalName), _actual(actual), _group(group)
{}

void IOPin::connect(WireId* formal) {
    if(_formal == NULL)
        throw IOPin::AlreadyConnected();
    _formal = formal;
    formal->connect(this, _actual);
}

CircuitGroup::CircuitGroup(const std::string& name) :
    CircuitTree(), name(name)
{
    wireManager_ = new WireManager();
}

CircuitGroup::CircuitGroup(const std::string& name, WireManager* manager) :
    CircuitTree(), name(name), wireManager_(manager)
{}

CircuitGroup::~CircuitGroup() {
    delete wireManager_;
}

void CircuitGroup::freeze() {
    for(auto child: grpChildren)
        child->freeze();
    CircuitTree::freeze();
}

void CircuitGroup::addChild(CircuitTree* child) {
    failIfFrozen();

    child->ancestor_ = this; // CircuitGroup is friend of CircuitTree
    if(child->circType() == CIRC_GROUP) {
        CircuitGroup* grp = static_cast<CircuitGroup*>(child);
        for(auto inp : grp->getInputs()) {
            inp.connect(wireManager_->wire(inp.formalName()));
        }
        for(auto out : grp->getOutputs()) {
            out.connect(wireManager_->wire(out.formalName()));
        }
    }
    grpChildren.push_back(child);
}

void CircuitGroup::addInput(const IOPin& pin) {
    failIfFrozen();
    grpInputs.push_back(pin);
}

void CircuitGroup::addInput(const std::string& formal, WireId* actual) {
    addInput(IOPin(formal, actual, this));
}

void CircuitGroup::addOutput(const IOPin& pin) {
    failIfFrozen();
    grpOutputs.push_back(pin);
}

void CircuitGroup::addOutput(const std::string& formal, WireId* actual) {
    addOutput(IOPin(formal, actual, this));
}

std::vector<CircuitTree*>& CircuitGroup::getChildren() {
    failIfFrozen();
    return grpChildren;
}
const std::vector<CircuitTree*>& CircuitGroup::getChildren() const {
    return grpChildren;
}

std::vector<IOPin>& CircuitGroup::getInputs() {
    failIfFrozen();
    return grpInputs;
}
const std::vector<IOPin>& CircuitGroup::getInputs() const {
    return grpInputs;
}

std::vector<IOPin>& CircuitGroup::getOutputs() {
    failIfFrozen();
    return grpOutputs;
}
const std::vector<IOPin>& CircuitGroup::getOutputs() const {
    return grpOutputs;
}

CircuitGroup::sig_t CircuitGroup::computeSignature(int level) {
    (&level); // UNUSED
    assert(false); // TODO implement
}

