#include "wireManager.h"
using namespace std;

WireManager::WireManager()
{}

WireId* WireManager::fresh(const std::string& name) {
    if(wireByName.find(name) == wireByName.end())
        throw AlreadyDefined(name.c_str());
    wireById.push_back(WireId(wireById.size(), name, this));
    wireByName[name] = &wireById.back();
    return &(wireById.back());
}

WireId* WireManager::freshInsulated(const std::string& name) {
    wireById.push_back(WireId(wireById.size(), name, this));
    return &(wireById.back());
}

bool WireManager::hasWire(const std::string& name) {
    return wireByName.find(name) != wireByName.end();
}

bool WireManager::hasWire(size_t id) {
    return wireById.size() > id; // `id` is unsigned
}

WireId* WireManager::wire(const std::string& name, bool dontCreate) {
    if(!hasWire(name)) {
        if(dontCreate)
            throw NotDefined(name.c_str());
        return fresh(name);
    }
    else
        return wireByName[name];
}

WireId* WireManager::wire(size_t id) {
    if(!hasWire(id))
        throw NotDefined("[id]");
    return &wireById[id];
}

