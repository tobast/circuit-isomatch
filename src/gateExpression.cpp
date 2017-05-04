#include "gateExpression.h"
#include <exception>

using namespace signatureConstants;

class UnimplementedOperator : public std::exception {
    const char* what() {
        return "Reached the end of a supposedly comprehensive switch. "
            "An operator is unimplemented in `gateExpression.cpp`.";
    }
};

static bool isCommutative(ExpressionBinOperator op) {
    switch(op) {
        case BAnd:
        case BOr:
        case BXor:
        case BAdd:
        case BMul:
            return true;

        case BSub:
        case BDiv:
        case BMod:
        case BLsr:
        case BLsl:
        case BAsr:
            return false;
    }
    throw UnimplementedOperator();
}

static const OperConstants& cstOf(ExpressionBinOperator op) {
    switch(op) {
        case BAnd:
            return opcst_and;
        case BOr:
            return opcst_or;
        case BXor:
            return opcst_xor;
        case BAdd:
            return opcst_add;
        case BSub:
            return opcst_sub;
        case BMul:
            return opcst_mul;
        case BDiv:
            return opcst_div;
        case BMod:
            return opcst_mod;
        case BLsr:
            return opcst_lsr;
        case BLsl:
            return opcst_lsl;
        case BAsr:
            return opcst_asr;
    }
    throw UnimplementedOperator();
}

static const OperConstants& cstOf(ExpressionUnOperator op) {
    switch(op) {
        case UNot:
            return opcst_not;
    }
    throw UnimplementedOperator();
}

static const OperConstants& cstOf(ExpressionUnOperatorCst op) {
    switch(op) {
        case UCLsr:
            return opcst_un_lsr;
        case UCLsl:
            return opcst_un_lsl;
        case UCAsr:
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

void ExpressionBase::serialize(std::basic_ostream<char>& out) const {
    out << "{\"type\":"
        << (int)type
        << ",";

    serialize_body(out);

    out << "}";
}

void ExpressionConst::serialize_body(std::basic_ostream<char>& out) const {
    out << "\"val\":" << val;
}

void ExpressionVar::serialize_body(std::basic_ostream<char>& out) const {
    out << "\"id\":" << id;
}

void ExpressionBinOp::serialize_body(std::basic_ostream<char>& out) const {
    out << "\"op\":"
        << op
        << ",\"left\":";
    left->serialize(out);
    out << ",\"right\":";
    right->serialize(out);
}

void ExpressionUnOp::serialize_body(std::basic_ostream<char>& out) const {
    out << "\"op\":"
        << op
        << ",\"expr\":";
    expr->serialize(out);
}

void ExpressionUnOpCst::serialize_body(std::basic_ostream<char>& out) const {
    out << "\"op\":"
        << op
        << ",\"val\":"
        << val
        << ",\"expr\":";
    expr->serialize(out);
}

void ExpressionSlice::serialize_body(std::basic_ostream<char>& out) const {
    out << "\"beg\":"
        << beg
        << ",\"end\":"
        << end
        << ",\"expr\":";
    expr->serialize(out);
}

void ExpressionMerge::serialize_body(std::basic_ostream<char>& out) const {
    out << "\"left\":";
    left->serialize(out);
    out << ".\"right\":";
    right->serialize(out);
}
