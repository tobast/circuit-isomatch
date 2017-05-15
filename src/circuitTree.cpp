#include "circuitTree.h"
#include "circuitGroup.h"
#include <cassert>

using namespace std;

size_t CircuitTree::nextCircuitId = 0;

CircuitTree::CircuitTree() :
        frozen(false), ancestor_(NULL), circuitId(nextCircuitId)
{
    nextCircuitId++;
}

CircuitTree::~CircuitTree()
{}

sig_t CircuitTree::sign(int level) {
    failIfNotFrozen();
    if(level < (int)memoSig.size() && memoSig[level] != 0)
        return memoSig[level];

    sig_t signature = computeSignature(level);
    while((int)memoSig.size() <= level) // Create [level] cell
        memoSig.push_back(0);
    memoSig[level] = signature;
    return signature;
}

bool CircuitTree::equals(const CircuitTree* oth) const {
    failIfNotFrozen();
    if(circType() != oth->circType())
        return false;
    return innerEqual(oth);
}

sig_t CircuitTree::computeSignature(int level) {
    // Depends only on the gate's type and contents.
    sig_t inner = innerSignature();

    if(level <= 0)
        return inner;

    // inpSig is the sum of the signatures of order `level - 1` of all
    // directly input-adjacent gates.
    sig_t inpSig = 0;
    for(auto wire = inp_begin(); wire != inp_end(); ++wire) {
        for(auto circ = (*wire)->adjacent_begin();
                circ != (*wire)->adjacent_end(); ++circ)
        {
            inpSig += (*circ)->sign(level-1);
        }
    }

    // Idem with output-adjacent gates
    sig_t outSig = 0;
    for(auto wire = out_begin(); wire != out_end(); ++wire) {
        for(auto circ = (*wire)->adjacent_begin();
                circ != (*wire)->adjacent_end(); ++circ)
        {
            outSig += (*circ)->sign(level-1);
        }
    }

    // Sum of the IO signatures of the connected wires. For more details on
    // what's an IO signature, see `CircuitGroup::ioSigOf`'s docstring.
    sig_t ioSig = 0;
    if(ancestor_ != nullptr) {
        for(auto wire = io_begin(); wire != io_end(); ++wire)
            ioSig += ancestor_->ioSigOf(*wire);
    }

    return inner + ioSig + inpSig - outSig;
}

void CircuitTree::failIfFrozen() const {
    if(frozen)
        throw Frozen();
}

void CircuitTree::failIfNotFrozen() const {
    if(!frozen)
        throw NotFrozen();
}

