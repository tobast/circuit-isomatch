#include "circuitGroup.h"
#include "dotPrint.h"
#include "signatureConstants.h"
#include <cassert>
#include <cstdint>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>

#include "debug.h"

using namespace std;

IOPin::IOPin(WireId* formal,
        WireId* actual,
        CircuitGroup* group) :
    _formal(formal), _actual(actual), _group(group)
{}

IOPin::IOPin(std::string formalName, WireId* actual, CircuitGroup* group) :
    _formal(NULL), _formalName(formalName), _actual(actual), _group(group)
{}

void IOPin::connect(WireId* formal) {
    if(_formal != NULL)
        throw IOPin::AlreadyConnected();
    _formal = formal;
    link();
}

void IOPin::link() {
    _formal->connect(this, _actual);
}

void CircuitGroup::InnerIoIter::operator++() {
    innerIncr();
    nextValid(); // Forward to the next valid element.
}

void CircuitGroup::InnerIoIter::nextValid() {
    while(ptr != circ->grpOutputs.end() && (*ptr)->formal() == nullptr)
        innerIncr();
}

void CircuitGroup::InnerIoIter::innerIncr() {
    if(ptr == circ->grpOutputs.end())
        return;
    ++ptr;
    if(ptr == circ->grpInputs.end())
        ptr = circ->grpOutputs.begin();
}

CircuitGroup::CircuitGroup(const std::string& name) :
    CircuitTree(), name_(name)
{
    wireManager_ = new WireManager();
}

CircuitGroup::CircuitGroup(const std::string& name, WireManager* manager) :
    CircuitTree(), name_(name), wireManager_(manager)
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
            if(inp->formal() == nullptr)
                inp->connect(wireManager_->wire(inp->formalName()));
        }
        for(auto out : grp->getOutputs()) {
            if(out->formal() == nullptr)
                out->connect(wireManager_->wire(out->formalName()));
        }
    }
    grpChildren.push_back(child);
}

void CircuitGroup::addInput(const IOPin& pin) {
    failIfFrozen();
    IOPin* nPin = new IOPin(pin);
    if(nPin->_formal != nullptr)
        nPin->link();
    grpInputs.push_back(nPin);
}

void CircuitGroup::addInput(const std::string& formal, WireId* actual) {
    addInput(IOPin(formal, actual, this));
}

void CircuitGroup::addOutput(const IOPin& pin) {
    failIfFrozen();
    IOPin* nPin = new IOPin(pin);
    if(nPin->_formal != nullptr)
        nPin->link();
    grpOutputs.push_back(nPin);
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
    const string thisCirc = string("group_") + name_ + to_string(id());

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
        << "graph[style=filled, splines=curved, label=\"" << name_ << "\"]\n";

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

sig_t CircuitGroup::ioSigOf(WireId* wire) const {
    failIfNotFrozen();
    try {
        return ioSigs_.at(wire);
    }
    catch(const std::out_of_range& e) {
        return 0;
    }
}

sig_t CircuitGroup::innerSignature() const {
    sig_t subsigs = 0;
    for(auto sub : grpChildren)
        subsigs += sub->sign();
    return signatureConstants::opcst_leaftype(
            ((circType() << 16)
             | (grpInputs.size() << 8)
             | (grpOutputs.size()))
            + subsigs);
}

namespace groupEquality {
    class TooManyPermutations : public std::exception {};
    class Permutation;

    typedef vector<vector<CircuitTree*> > SigSplit;
    typedef unordered_map<sig_t, set<CircuitTree*> > SigSplitMapped;

    /// Computes k!
    int factorial(int k);

    /// Computes the signature of a wire with given accuracy
    sig_t wireSignature(WireId* wire, int accuracy = -1);

    /** Splits a set of circuits into sets of circuits with the same signatures
     * @param circuits The wires to consider
     * @param splitted A reference to the vector that will be filled
     * @param signatures Will be filled with the list of signatures of each
     *        chunk from `splitted`
     * @param maxPermutations Stop if the number of permutations exceeds this
     *        parameter, and raise TooManyPermutations
     * @param accuracy Level of accuracy of the signature function, or -1 for
     *        the default value
     */
    void splitOnSig(const vector<CircuitTree*> circuits,
            SigSplit& splitted,
            std::vector<sig_t>& signatures,
            int maxPermutations = -1,
            int accuracy = -1);

    /** Checks that both `fst` and `snd` have the same keys, and sets of equal
     * sizes for each key. */
    bool equalSizes(
            const SigSplit& fst,
            const SigSplit& snd);

    bool equalWithPermutation(
            const SigSplit& leftSplit, const SigSplit& rightSplit,
            const Permutation& perm);

    // ======

    class Permutation {
        public:
            typedef vector<int> PermElem;

            Permutation(const SigSplit& split) {
                for(const auto& category : split) {
                    perms.push_back(vector<int>(category.size()));
                    mkIdentity(perms.back());
                }
                nextChange = lastIter();
            }

            const PermElem& operator[](sig_t index) const {
                return perms.at(index);
            }

            /// Next "meta-permutation"
            bool next() {
                while(!next_permutation(nextChange->begin(),
                            nextChange->end()))
                {
                    if(nextChange == perms.begin())
                        return false; // Last permutation
                    --nextChange;
                }
                nextChange = lastIter();
                return true;
            }

        private:
            typedef vector<PermElem> PermStruct;

            inline void mkIdentity(vector<int>& vect) {
                int n=0;
                generate(vect.begin(), vect.end(), [&n]{ return n++; });
            }
            inline PermStruct::iterator lastIter() {
                return --(perms.end());
            }

            PermStruct perms;
            PermStruct::iterator nextChange;
    };

    int factorial(int k) {
        static vector<int> memoized {1, 1};
        for(int nextVal = (int)memoized.size(); nextVal <= k; ++nextVal)
            memoized.push_back(memoized.back() * nextVal);
        return memoized[k];
    }

    sig_t wireSignature(WireId* wire, int accuracy) {
        sig_t outSig = 0;
        for(auto circ = wire->adjacent_begin();
                circ != wire->adjacent_end();
                ++circ)
        {
            if(accuracy < 0)
                outSig ^= (*circ)->sign();
            else
                outSig ^= (*circ)->sign(accuracy);
        }

        return outSig;
    }

    void splitOnSig(const vector<CircuitTree*> circuits,
            SigSplit& splitted,
            std::vector<sig_t>& signatures,
            int maxPermutations,
            int accuracy)
    {
        splitted.clear();
        signatures.clear();

        map<sig_t, set<CircuitTree*> > wipSplit;

        for(auto circ : circuits) {
            sig_t sig = accuracy >= 0 ? circ->sign(accuracy) : circ->sign();
            wipSplit[sig].insert(circ);

            if(maxPermutations >= 0
                    && factorial(wipSplit[sig].size()) > maxPermutations)
            {
                // Keep it simple: the threshold will be low anyway, and we do
                // not want to divide anything here, it must be fast.
                throw TooManyPermutations();
            }
        }

        splitted.reserve(wipSplit.size());
        signatures.reserve(wipSplit.size());
        for(const auto& assoc : wipSplit) { // Order is deterministic
            signatures.push_back(assoc.first);
            splitted.push_back(vector<CircuitTree*>());
            splitted.back().reserve(assoc.second.size());
            for(const auto& circ : assoc.second)
                splitted.back().push_back(circ);
        }
    }

    bool equalSizes(
            const SigSplit& fst,
            const SigSplit& snd)
    {
        if(fst.size() != snd.size())
            return false;

        for(size_t pos = 0; pos < fst.size(); ++pos) {
            if(fst[pos].size() != snd[pos].size())
                return false;
        }
        return true;
    }

    bool equalWithPermutation(
            const SigSplit& leftSplit, const SigSplit& rightSplit,
            const Permutation& perm)
    {
        // NOTE: here, we assume that keys(leftSplit) == keys(rightSplit)

        // Check sub-equality
        for(size_t pos = 0 ; pos < leftSplit.size(); ++pos) {
            const vector<int>& curPerm = perm[pos];
            for(size_t circId = 0; circId < leftSplit[pos].size(); ++circId) {
                if(!leftSplit[pos][circId]->equals(
                            rightSplit[pos][curPerm[circId]]))
                {
                    EQ_DEBUG("Not sub-equal (types %d, %d)\n",
                            leftSplit[pos][circId]->circType(),
                            rightSplit[pos][curPerm[circId]]->circType());
                    return false;
                }
            }
        }

        // Check graph structure
        unordered_map<WireId*, WireId*> lrWireMap;
        /* This map ^ maps a left-hand WireId to a right-hand one. We want this
         * map to be a bijection in order to have a graph isomorphism, so we
         * simply have to walk through our nodes mapping, map wires in this map
         * (and fail in case of conflict). This ensures the functionnal
         * property (`f(x)` is unique). Then, checking that each `WireId*` has
         * at most one occurrence in the right-hand side of the map ensures
         * injectivity. Surjectivity is then guaranteed by the structure of the
         * circuit itself and the pigeon-hole principle.
         */
        for(size_t pos = 0; pos < leftSplit.size(); ++pos) {
            const vector<int>& curPerm = perm[pos];
            for(size_t circId = 0; circId < leftSplit[pos].size(); ++circId) {
                CircuitTree *left = leftSplit[pos][circId],
                    *right = rightSplit[pos][curPerm[circId]];
                CircuitTree::IoIter lWire = left->io_begin(),
                    rWire = right->io_begin();
                for(; lWire != left->io_end() /* rWire checked after */ ;
                        ++lWire, ++rWire)
                {
                    if(rWire == right->io_end()) { // Prematurate end
                        EQ_DEBUG("Bad wire count (<- %s)\n",
                                (*lWire)->name().c_str());
                        return false;
                    }
                    if(lrWireMap.find(*lWire) != lrWireMap.end()
                            && *(lrWireMap[*lWire]) != **rWire)
                    {
                        EQ_DEBUG("Wire conflict %s -> {%s - %s}\n",
                                (*lWire)->uniqueName().c_str(),
                                lrWireMap.find(*lWire)->second->uniqueName().c_str(),
                                (*rWire)->uniqueName().c_str());
                        return false; // Wire conflict
                    }

                    lrWireMap[*lWire] = *rWire;
                }
                if(rWire != right->io_end()) { // Bad wire count
                    EQ_DEBUG("Bad wire count (-> %s)\n",
                            (*rWire)->name().c_str());
                    return false;
                }
            }
        }
        unordered_set<WireId*> rightSide;
        for(const auto& assoc : lrWireMap) {
            if(rightSide.find(assoc.second) != rightSide.end()) {
                EQ_DEBUG("Non injective\n");
                return false; // Non-injective
            }
            rightSide.insert(assoc.second);
        }

        return true;
    }
}

bool CircuitGroup::innerEqual(CircuitTree* othTree) {
    CircuitGroup* oth = dynamic_cast<CircuitGroup*>(othTree);

    // FIXME obscure constants
    const int BASE_PRECISION = 2,
              MAX_PRECISION = 15,
              MAX_PERMUTATIONS = 4;

    EQ_DEBUG("\t> Entering %s (has WM %lu) <\n",
            name().c_str(), wireManager_->id());

    groupEquality::SigSplit sigSplit[2];

    for(int precision = BASE_PRECISION;
            precision <= MAX_PRECISION;
            ++precision)
    {
        int maxPermutations = (precision == MAX_PRECISION) ?
            -1 : MAX_PERMUTATIONS;

        vector<sig_t> leftSig, rightSig;
        try {
            // use the const version of `getChildren`
            const vector<CircuitTree*>& lChildren =
                static_cast<const CircuitGroup*>(this)->getChildren();
            const vector<CircuitTree*>& rChildren =
                static_cast<const CircuitGroup*>(oth)->getChildren();

            groupEquality::splitOnSig(lChildren, sigSplit[0], leftSig,
                    maxPermutations, precision);
            groupEquality::splitOnSig(rChildren, sigSplit[1], rightSig,
                    maxPermutations, precision);
        } catch(const groupEquality::TooManyPermutations&) {
            continue;
        }

        if(!groupEquality::equalSizes(sigSplit[0], sigSplit[1])) {
            EQ_DEBUG(">> Mismatched signature sets' sizes\n");
            return false;
        }
        if(leftSig != rightSig) {
            EQ_DEBUG(">> Mismatched signature sets\n");
            return false;
        }

        // TODO split again on adjacent wires' signatures

        if(maxPermutations >= 0) {
            // Check if we have too many permutations -- for real this time
            int perms = 1;
            for(const auto& assoc : sigSplit[0])
                perms *= groupEquality::factorial(assoc.size());
            if(perms > maxPermutations)
                continue; // Increase the precision
            else {
                EQ_DEBUG("Trying %d permutations (prec. %d)\n",
                        perms, precision);
            }
        }

        // Now try all the remaining permutations.
        // TODO some clever heuristic to enumerate the permutations in a clever
        // order?

        const groupEquality::SigSplit& leftSplit = sigSplit[0];;
        const groupEquality::SigSplit& rightSplit = sigSplit[1];;
        // Since `sigSplit` is not mutated from now on, we can safely rely on
        // its ordering

        groupEquality::Permutation perm(leftSplit);
        do {
            if(groupEquality::equalWithPermutation(
                        leftSplit, rightSplit, perm))
            {
                EQ_DEBUG(">> Permutation (%s) OK\n", name().c_str());
                return true;
            }
        } while(perm.next());
        EQ_DEBUG(">> No permutation worked :c (%s)\n", name().c_str());
        return false;
    }

    throw std::runtime_error("Reached end of CircuitGroup::innerEqual, please "
            "submit a bugreport");
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
static sig_t ioSigOfSet(const unordered_set<size_t>& set) {
    static const uint32_t pinMod = signatureConstants::pinIdMod;
    auto valSig = [](size_t val) { return expmod(2, val, pinMod); };

    sig_t out = 0;
    for(auto& val : set)
        out = (out + valSig(val)) % pinMod;
    return out;
}

void CircuitGroup::computeIoSigs() {
    unordered_map<WireId*, unordered_set<size_t> > inpPinsForWire;
    unordered_map<WireId*, unordered_set<size_t> > outPinsForWire;

    for(size_t inpId = 0; inpId < grpInputs.size(); ++inpId) {
        IOPin* pin = grpInputs[inpId];
        auto& wireSet = inpPinsForWire[pin->actual()];
        wireSet.insert(inpId);
    }
    for(size_t outId = 0; outId < grpOutputs.size(); ++outId) {
        IOPin* pin = grpOutputs[outId];
        auto& wireSet = outPinsForWire[pin->actual()];
        wireSet.insert(outId);
    }

    for(auto& entry : inpPinsForWire)
        ioSigs_[entry.first] = ioSigOfSet(entry.second);
    for(auto& entry : outPinsForWire)
        ioSigs_[entry.first] += (ioSigOfSet(entry.second) << 32);
    // FIXME ough to mix up a bit the two parts.
}
