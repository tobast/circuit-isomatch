#include "wireId.h"
#include "circuitTree.h"
#include "circuitGroup.h"

#include <unordered_set>
#include <sstream>
using namespace std;

WireId::WireId(size_t id, const std::string& name, WireManager* manager) :
    end(new Inner), isEndpoint(true), ufDepth(0)
{
    inner()->id = id;
    inner()->name = name;
    inner()->manager = manager;
}

WireId::~WireId() {
    if(isEndpoint)
        delete end;
}

bool WireId::operator==(WireId& oth) {
    return inner()->manager->id() == oth.inner()->manager->id()
        && inner()->id == oth.inner()->id;
}

bool WireId::operator==(const WireId& oth) const {
    return inner()->manager->id() == oth.inner()->manager->id()
        && inner()->id == oth.inner()->id;
}

bool WireId::operator<(WireId& oth) {
    return (inner()->manager->id() < oth.inner()->manager->id())
        || (inner()->manager->id() == oth.inner()->manager->id()
                && inner()->id < oth.inner()->id);
}

bool WireId::operator<(const WireId& oth) const {
    return (inner()->manager->id() < oth.inner()->manager->id())
        || (inner()->manager->id() == oth.inner()->manager->id()
                && inner()->id < oth.inner()->id);
}

void WireId::connect(CircuitTree* circ) {
    inner()->connected.push_back(circ);
}

void WireId::connect(const PinConnection& pin) {
    inner()->connectedPins.push_back(pin);
}

void WireId::connect(IOPin* pin, WireId* other) {
    connect(PinConnection(pin, other));
}

const std::vector<CircuitTree*>& WireId::connectedCirc() {
    return inner()->connected;
}

const std::vector<WireId::PinConnection>& WireId::connectedPins() {
    return inner()->connectedPins;
}

std::vector<CircuitTree*> WireId::connected() {
    unordered_set<CircuitTree*> outSet;
    unordered_set<WireId> seenWires;
    walkConnected(outSet, seenWires, this);

    vector<CircuitTree*> out;
    for(auto circ : outSet)
        out.push_back(circ);
    return out;
}

std::string WireId::uniqueName() {
    ostringstream stream;
    stream << name() << '_' << inner()->manager->id() << '_' << inner()->id;
    return stream.str();
}

void WireId::walkConnected(std::unordered_set<CircuitTree*>& curConnected,
        std::unordered_set<WireId>& seenWires,
        WireId* curWire)
{
    if(seenWires.find(*curWire) != seenWires.end())
        return;
    seenWires.insert(*curWire);

    for(auto circ : curWire->connectedCirc())
        curConnected.insert(circ);

    for(auto pin : curWire->connectedPins())
        walkConnected(curConnected, seenWires, pin.other);
}

void WireId::merge(WireId* other) {
    WireId *kept = this->ufRoot();
    WireId *merged = other->ufRoot();
    if(merged->ufDepth > kept->ufDepth) {
        WireId* swap = kept;
        kept = merged;
        merged = swap;
    }

    delete merged->inner();
    merged->isEndpoint = false;
    merged->chain = kept;
    kept->ufDepth = max(merged->ufDepth + 1, (int)kept->ufDepth);
}

WireId* WireId::ufRoot() {
    if(isEndpoint)
        return this;
    chain = chain->ufRoot();
    return chain;
}

const WireId::Inner* WireId::inner() const {
    if(isEndpoint)
        return end;
    return chain->inner();
}
