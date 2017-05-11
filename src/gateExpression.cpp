#include "gateExpression.h"
#include <exception>

using namespace signatureConstants;

class UnimplementedOperator : public std::exception {
    const char* what() {
        return "Reached the end of a supposedly comprehensive switch. "
            "An operator is unimplemented in `gateExpression.cpp`.";
    }
};

static bool isCommutative(expr::ExpressionBinOperator op) {
    switch(op) {
        case expr::BAnd:
        case expr::BOr:
        case expr::BXor:
        case expr::BAdd:
        case expr::BMul:
            return true;

        case expr::BSub:
        case expr::BDiv:
        case expr::BMod:
        case expr::BLsr:
        case expr::BLsl:
        case expr::BAsr:
            return false;
    }
    throw UnimplementedOperator();
}

static const OperConstants& cstOf(expr::ExpressionBinOperator op) {
    switch(op) {
        case expr::BAnd:
            return opcst_and;
        case expr::BOr:
            return opcst_or;
        case expr::BXor:
            return opcst_xor;
        case expr::BAdd:
            return opcst_add;
        case expr::BSub:
            return opcst_sub;
        case expr::BMul:
            return opcst_mul;
        case expr::BDiv:
            return opcst_div;
        case expr::BMod:
            return opcst_mod;
        case expr::BLsr:
            return opcst_lsr;
        case expr::BLsl:
            return opcst_lsl;
        case expr::BAsr:
            return opcst_asr;
    }
    throw UnimplementedOperator();
}

static const OperConstants& cstOf(expr::ExpressionUnOperator op) {
    switch(op) {
        case expr::UNot:
            return opcst_not;
    }
    throw UnimplementedOperator();
}

static const OperConstants& cstOf(expr::ExpressionUnOperatorCst op) {
    switch(op) {
        case expr::UCLsr:
            return opcst_un_lsr;
        case expr::UCLsl:
            return opcst_un_lsl;
        case expr::UCAsr:
            return opcst_un_asr;
    }
    throw UnimplementedOperator();
}

sig_t ExpressionConst::sign() const {
    return opcst_numconst(val);
}

sig_t ExpressionVar::sign() const {
    return opcst_wireid(id);
}

sig_t ExpressionBinOp::sign() const {
    if(isCommutative(op))
        return cstOf(op)(left->sign() + right->sign());
    return cstOf(op)(left->sign() - right->sign());
}

sig_t ExpressionUnOp::sign() const {
    return cstOf(op)(expr->sign());
}

sig_t ExpressionUnOpCst::sign() const {
    return cstOf(op)(expr->sign() - opcst_cstint(val));
}

sig_t ExpressionSlice::sign() const {
    return opcst_slice(expr->sign()
            - opcst_slicebounds(end * sliceMulInner - beg));
}

sig_t ExpressionMerge::sign() const {
    return opcst_merge(left->sign() - right->sign());
}
