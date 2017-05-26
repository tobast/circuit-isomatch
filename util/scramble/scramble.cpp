#include <unordered_map>
#include <string>
#include <random>
#include <algorithm>
#include <stdexcept>

#include "scramble.h"
using namespace std;

namespace {
    typedef unordered_map<string, string> NameMap;

    ExpressionVar* scrambleExpressionVar(const ExpressionVar* expr);
    ExpressionConst* scrambleExpressionConst(const ExpressionConst* expr);
    ExpressionBinOp* scrambleExpressionBinOp(const ExpressionBinOp* expr);
    ExpressionUnOp* scrambleExpressionUnOp(const ExpressionUnOp* expr);
    ExpressionUnOpCst* scrambleExpressionUnOpCst(
            const ExpressionUnOpCst* expr);
    ExpressionSlice* scrambleExpressionSlice(const ExpressionSlice* expr);
    ExpressionMerge* scrambleExpressionMerge(const ExpressionMerge* expr);
    ExpressionBase* scrambleExpression(const ExpressionBase* expr);

    CircuitGroup* scrambleGroup(CircuitGroup* orig, WireManager* manager,
            NameMap& nameMap);
    CircuitComb* scrambleComb(CircuitComb* orig, WireManager* manager,
            NameMap& nameMap);
    CircuitDelay* scrambleDelay(CircuitDelay* orig, WireManager* manager,
            NameMap& nameMap);
    CircuitTristate* scrambleTristate(CircuitTristate* orig,
            WireManager* manager, NameMap& nameMap);
    CircuitAssert* scrambleAssert(CircuitAssert* orig, WireManager* manager,
            NameMap& nameMap);
    CircuitTree* recScramble(CircuitTree* orig, WireManager* manager,
            NameMap& nameMap);

    // =================================================================

    /** Returns a random string that can be used as a valid wire name */
    string randName(size_t length=12) {
        static default_random_engine generator;
        static uniform_int_distribution<int> distribution(0, 26*2+10-1);

        string out;
        for(size_t pos = 0; pos < length; ++pos) {
            int selector = distribution(generator);
            char randCh = (selector < 26*2) ?
                ('A' + selector) :
                ('0' + (selector - 26*2));
            out += randCh;
        }
        return out;
    }

    /** Returns the `WireId*` corresponding to the given wire name, wrt. the
     * given `WireManager` and `NameMap`. Creates a random name mapping if the
     * given name was not previously mapped. */
    WireId* wire(const string& name, WireManager* manager, NameMap& nameMap) {
        const auto mappedName = nameMap.find(name);
        WireId* mapped = nullptr;;
        if(mappedName == nameMap.end()) {
            string nName = randName() + string("_") + name;
            nameMap[nName] = nName;
            mapped = manager->wire(nName);
        }
        else
            mapped = manager->wire(mappedName->second);

        return mapped;
    }

    WireId* wire(const WireId* cur, WireManager* manager, NameMap& nameMap) {
        return wire(cur->name(), manager, nameMap);
    }

    ExpressionVar* scrambleExpressionVar(const ExpressionVar* expr) {
        return new ExpressionVar(expr->id);
    }

    ExpressionConst* scrambleExpressionConst(const ExpressionConst* expr) {
        return new ExpressionConst(expr->val);
    }

    ExpressionBinOp* scrambleExpressionBinOp(const ExpressionBinOp* expr) {
        return new ExpressionBinOp(
                scrambleExpression(expr->left),
                scrambleExpression(expr->right),
                expr->op);
    }

    ExpressionUnOp* scrambleExpressionUnOp(const ExpressionUnOp* expr) {
        return new ExpressionUnOp(
                scrambleExpression(expr->expr),
                expr->op);
    }

    ExpressionUnOpCst* scrambleExpressionUnOpCst(
            const ExpressionUnOpCst* expr)
    {
        return new ExpressionUnOpCst(
                scrambleExpression(expr->expr),
                expr->val,
                expr->op);
    }

    ExpressionSlice* scrambleExpressionSlice(const ExpressionSlice* expr) {
        return new ExpressionSlice(
                scrambleExpression(expr->expr),
                expr->beg,
                expr->end);
    }

    ExpressionMerge* scrambleExpressionMerge(const ExpressionMerge* expr) {
        return new ExpressionMerge(
                scrambleExpression(expr->left),
                scrambleExpression(expr->right));
    }

    ExpressionBase* scrambleExpression(const ExpressionBase* expr) {
        switch(expr->type) {
            case expr::ExprVar:
                return scrambleExpressionVar(
                        dynamic_cast<const ExpressionVar*>(expr));
            case expr::ExprConst:
                return scrambleExpressionConst(
                        dynamic_cast<const ExpressionConst*>(expr));
            case expr::ExprBinOp:
                return scrambleExpressionBinOp(
                        dynamic_cast<const ExpressionBinOp*>(expr));
            case expr::ExprUnOp:
                return scrambleExpressionUnOp(
                        dynamic_cast<const ExpressionUnOp*>(expr));
            case expr::ExprUnOpCst:
                return scrambleExpressionUnOpCst(
                        dynamic_cast<const ExpressionUnOpCst*>(expr));
            case expr::ExprSlice:
                return scrambleExpressionSlice(
                        dynamic_cast<const ExpressionSlice*>(expr));
            case expr::ExprMerge:
                return scrambleExpressionMerge(
                        dynamic_cast<const ExpressionMerge*>(expr));
        }

        throw domain_error("Encountered unexpected expression type");
    }

    /** Fills `perm` with a random permutation of [0..`perm.size()`-1] */
    void randPermutation(vector<int>& perm) {
        for(size_t pos=0; pos < perm.size(); ++pos)
            perm[pos] = pos;
        random_shuffle(perm.begin(), perm.end());
    }

    CircuitGroup* scrambleGroup(CircuitGroup* orig, WireManager* manager,
            NameMap& nameMap)
    {
        CircuitGroup* outGrp = new CircuitGroup(orig->name());
        WireManager* innerManager = outGrp->wireManager();
        NameMap innerNameMap;

        // Recreate all the wires in a randomized order, to randomize IDs
        {
            const vector<WireId*>& oldWires = orig->wireManager()->wires();
            vector<int> walkPerm(oldWires.size());
            randPermutation(walkPerm);

            for(const auto& curId : walkPerm)
                wire(oldWires[curId]->name(), innerManager, innerNameMap);
        }

        for(const auto& inp : orig->getInputs()) {
            if(manager == nullptr || inp->formal() == nullptr)
                outGrp->addInput(inp->formalName(),
                        wire(inp->actual(), innerManager, innerNameMap));
            else
                outGrp->addInput(IOPin(
                        wire(inp->formal(), manager, nameMap),
                        wire(inp->actual(), innerManager, innerNameMap),
                        outGrp));
        }
        for(const auto& out : orig->getOutputs()) {
            if(manager == nullptr || out->formal() == nullptr)
                outGrp->addOutput(out->formalName(),
                        wire(out->actual(), innerManager, innerNameMap));
            else
                outGrp->addOutput(IOPin(
                        wire(out->formal(), manager, nameMap),
                        wire(out->actual(), innerManager, innerNameMap),
                        outGrp));
        }

        // Recreate all the children in a randomized order
        {
            const vector<CircuitTree*>& oldChildren =
                dynamic_cast<const CircuitGroup*>(orig)->getChildren();
            vector<int> walkPerm(oldChildren.size());
            randPermutation(walkPerm);

            for(const auto& curId : walkPerm) {
                CircuitTree* child = oldChildren[curId];
                outGrp->addChild(
                        recScramble(child, innerManager, innerNameMap));
            }
        }

        return outGrp;
    }

    CircuitComb* scrambleComb(CircuitComb* orig, WireManager* manager,
            NameMap& nameMap)
    {
        CircuitComb* outGate = new CircuitComb();
        for(const auto& inp : orig->inputs())
            outGate->addInput(wire(inp->name(), manager, nameMap));
        for(size_t outPos = 0;
                outPos < orig->outputs().size() &&
                outPos < orig->expressions().size();
                ++outPos)
        {
            outGate->addOutput(
                    scrambleExpression(orig->expressions()[outPos]),
                    wire(orig->outputs()[outPos]->name(), manager, nameMap));
        }

        return outGate;
    }

    CircuitDelay* scrambleDelay(CircuitDelay* orig, WireManager* manager,
            NameMap& nameMap)
    {
        return new CircuitDelay(
                wire(orig->input()->name(), manager, nameMap),
                wire(orig->output()->name(), manager, nameMap));
    }

    CircuitTristate* scrambleTristate(CircuitTristate* orig,
            WireManager* manager, NameMap& nameMap)
    {
        return new CircuitTristate(
                wire(orig->input()->name(), manager, nameMap),
                wire(orig->output()->name(), manager, nameMap),
                wire(orig->enable()->name(), manager, nameMap));
    }

    CircuitAssert* scrambleAssert(CircuitAssert* orig, WireManager* manager,
            NameMap& nameMap)
    {
        CircuitAssert* out = new CircuitAssert(orig->name(),
                scrambleExpression(orig->expression()));
        for(const auto& input : orig->inputs())
            out->addInput(wire(input->name(), manager, nameMap));

        return out;
    }

    /** Recursively descends on `CircuitTree`s, copying and scrambling them
     * on-the-go. The returned `CircuitTree` will be of the same sub-type as
     * `orig`.
     * If `orig` is a `CircuitGroup`, `manager` may be `nullptr`, in which case
     * the `formal` wires will be left null. */
    CircuitTree* recScramble(CircuitTree* orig, WireManager* manager,
            NameMap& nameMap)
    {
        switch(orig->circType()) {
            case CircuitTree::CIRC_GROUP:
                return scrambleGroup(
                        dynamic_cast<CircuitGroup*>(orig),
                        manager,
                        nameMap);
            case CircuitTree::CIRC_COMB:
                return scrambleComb(
                        dynamic_cast<CircuitComb*>(orig),
                        manager,
                        nameMap);
            case CircuitTree::CIRC_DELAY:
                return scrambleDelay(
                        dynamic_cast<CircuitDelay*>(orig),
                        manager,
                        nameMap);
            case CircuitTree::CIRC_TRI:
                return scrambleTristate(
                        dynamic_cast<CircuitTristate*>(orig),
                        manager,
                        nameMap);
            case CircuitTree::CIRC_ASSERT:
                return scrambleAssert(
                        dynamic_cast<CircuitAssert*>(orig),
                        manager,
                        nameMap);
        }

        throw domain_error("Encountered unexpected gate type");
    }

}

CircuitGroup* scrambleCircuit(CircuitGroup* original) {
    NameMap nameMap;
    return dynamic_cast<CircuitGroup*>(
            recScramble(static_cast<CircuitTree*>(original), nullptr,
                nameMap));
}
