#include "circuitGroup.h"
#include "dotPrint.h"
#include "signatureConstants.h"
#include <cassert>
#include <cstdint>

using namespace std;

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
    if(_formal != NULL)
        throw IOPin::AlreadyConnected();
    _formal = formal;
    formal->connect(this, _actual);
}

void CircuitGroup::InnerIoIter::operator++() {
    ++ptr;
    if(ptr == circ->grpInputs.end())
        ptr = circ->grpOutputs.begin();
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
    for(auto child: grpChildren)
        delete child;
    for(auto pin: grpInputs)
        delete pin;
    for(auto pin: grpOutputs)
        delete pin;
    delete wireManager_;
}

void CircuitGroup::freeze() {
    for(auto child: grpChildren)
        child->freeze();
    CircuitTree::freeze();

    computeIoSigs();
}

void CircuitGroup::addChild(CircuitTree* child) {
    failIfFrozen();

    child->ancestor_ = this; // CircuitGroup is friend of CircuitTree
    if(child->circType() == CIRC_GROUP) {
        CircuitGroup* grp = static_cast<CircuitGroup*>(child);
        for(auto inp : grp->getInputs()) {
            inp->connect(wireManager_->wire(inp->formalName()));
        }
        for(auto out : grp->getOutputs()) {
            out->connect(wireManager_->wire(out->formalName()));
        }
    }
    grpChildren.push_back(child);
}

void CircuitGroup::addInput(const IOPin& pin) {
    failIfFrozen();
    grpInputs.push_back(new IOPin(pin));
}

void CircuitGroup::addInput(const std::string& formal, WireId* actual) {
    addInput(IOPin(formal, actual, this));
}

void CircuitGroup::addOutput(const IOPin& pin) {
    failIfFrozen();
    grpOutputs.push_back(new IOPin(pin));
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

std::vector<IOPin*>& CircuitGroup::getInputs() {
    failIfFrozen();
    return grpInputs;
}
const std::vector<IOPin*>& CircuitGroup::getInputs() const {
    return grpInputs;
}

std::vector<IOPin*>& CircuitGroup::getOutputs() {
    failIfFrozen();
    return grpOutputs;
}
const std::vector<IOPin*>& CircuitGroup::getOutputs() const {
    return grpOutputs;
}

void CircuitGroup::toDot(std::basic_ostream<char>& out, int indent) {
    const string thisCirc = string("group_") + name + to_string(id());

    if(ancestor() == NULL) {
        // Root group
        dotPrint::indent(out, indent)
            << "digraph " << thisCirc << " {\n";
    }
    else {
        dotPrint::indent(out, indent)
            << "subgraph cluster_" << thisCirc << " {\n";
    }
    indent += 2;
    dotPrint::indent(out, indent)
        << "graph[style=filled, splines=curved, label=\"" << name << "\"]\n";

    // Wires
    for(auto wire : wireManager()->wires()) {
        dotPrint::indent(out, indent)
            << wire->uniqueName()
            << " [shape=plain, label="
            << wire->uniqueName()
            << "]"
            << '\n';
    }

    // IO pins
    for(auto inPin : grpInputs) {
        if(inPin->formal() != NULL) {
            dotPrint::indent(out, indent)
                << inPin->actual()->uniqueName()
                << " -> "
                << inPin->formal()->uniqueName()
                << " [arrowhead=none]"
                << '\n';
        }
    }
    for(auto outPin : grpOutputs) {
        if(outPin->formal() != NULL)
            dotPrint::indent(out, indent)
                << outPin->actual()->uniqueName()
                << " -> "
                << outPin->formal()->uniqueName()
                << " [arrowhead=none]"
                << '\n';
    }

    // Children
    for(auto child : grpChildren)
        child->toDot(out, indent);

    indent -= 2;
    dotPrint::indent(out, indent)
        << "}\n";
}

CircuitTree::sig_t CircuitGroup::ioSigOf(WireId* wire) const {
    failIfNotFrozen();
    try {
        return ioSigs_.at(*wire);
    }
    catch(const std::out_of_range& e) {
        return 0;
    }
}

CircuitGroup::sig_t CircuitGroup::computeSignature(int level) {
    (&level); // UNUSED
    assert(false); // TODO implement
}

/** Computes (base ** exp) % mod through quick exponentiation */
static uint64_t expmod(uint64_t base, uint64_t exp, uint64_t mod) {
    if(exp <= 1)
        return (exp == 1) ? base : 1;
    uint64_t out = expmod((base * base) % mod, exp / 2, mod);
    return (mod & 1) ? ((out * base) % mod) : out;
}

/** Computes the I/O signature of a single wire, given the I/O pins it is
 * connected to. */
static CircuitTree::sig_t ioSigOfSet(const unordered_set<size_t>& set) {
    static const uint32_t pinMod = signatureConstants::pinIdMod;
    auto valSig = [](size_t val) { return expmod(2, val, pinMod); };

    CircuitTree::sig_t out = 0;
    for(auto& val : set)
        out = (out + valSig(val)) % pinMod;
    return out;
}

void CircuitGroup::computeIoSigs() {
    unordered_map<WireId, unordered_set<size_t> > inpPinsForWire;
    unordered_map<WireId, unordered_set<size_t> > outPinsForWire;

    for(size_t inpId = 0; inpId < grpInputs.size(); ++inpId) {
        IOPin* pin = grpInputs[inpId];
        auto& wireSet = inpPinsForWire[*(pin->actual())];
        wireSet.insert(inpId);
    }
    for(size_t outId = 0; outId < grpOutputs.size(); ++outId) {
        IOPin* pin = grpInputs[outId];
        auto& wireSet = outPinsForWire[*(pin->actual())];
        wireSet.insert(outId);
    }

    for(auto& entry : inpPinsForWire)
        ioSigs_[entry.first] = ioSigOfSet(entry.second);
    for(auto& entry : outPinsForWire)
        ioSigs_[entry.first] += (ioSigOfSet(entry.second) << 32);
    // FIXME ough to mix up a bit the two parts.
}
