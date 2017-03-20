#include "wireId.h"
#include "circuitTree.h"
#include "circuitGroup.h"

#include <unordered_set>
using namespace std;

WireId::WireId(size_t id, const std::string& name) : id(id), name(name)
{}

void WireId::connect(CircuitTree* circ) {
    _connected.push_back(circ);
}

void WireId::connect(const PinConnection& pin) {
    _connectedPins.push_back(pin);
}

void WireId::connect(IOPin* pin, WireId* other) {
    connect(PinConnection(pin, other));
}

const std::vector<CircuitTree*>& WireId::connectedCirc() const {
    return _connected;
}

const std::vector<WireId::PinConnection>& WireId::connectedPins() const {
    return _connectedPins;
}

std::vector<CircuitTree*> WireId::connected() const {
    unordered_set<CircuitTree*> outSet;
    unordered_set<WireId> seenWires;
    walkConnected(outSet, seenWires, this);

    vector<CircuitTree*> out;
    for(auto circ : outSet)
        out.push_back(circ);
    return out;
}

void WireId::walkConnected(std::unordered_set<CircuitTree*>& curConnected,
        std::unordered_set<WireId>& seenWires,
        const WireId* curWire) const
{
    if(seenWires.find(*curWire) != seenWires.end())
        return;
    seenWires.insert(*curWire);

    for(auto circ : curWire->connectedCirc())
        curConnected.insert(circ);

    for(auto pin : curWire->connectedPins())
        walkConnected(curConnected, seenWires, pin.other);
}
