/** Defines an AST for the expressions used in gates (combinator gates,
 * assertion gates).
 */

#pragma once

/** Type of expression (used to cast to the right `struct`). */
enum ExpressionType {
    ExprVar,        /** End variable */
    ExprConst,      /** Integer constant */
    ExprBinOp,      /** Binary operator (+, AND, ...) */
    ExprUnOp,       /** Unary operator (NOT, ...) */
    ExprUnOpCst,    /** Unary operator with constant (CstLSL, ...) */
    ExprSlice,      /** Take a subword out of a word */
    ExprMerge,      /** Concatenate two words into a longer one */
};

/** Operator for `ExprBinOp` */
enum ExpressionBinOperator {
    BAnd,           /** Bitwise and */
    BOr,            /** Bitwise or */
    BXor,           /** Bitwise exclusive or */
    BAdd,           /** Addition */
    BSub,           /** Subtraction */
    BMul,           /** Multiplication */
    BDiv,           /** Division */
    BMod,           /** Modulus */
    BLsr,           /** Logical shift right */
    BLsl,           /** Logical shift left */
    BAsr,           /** Arithmetic shift right */
};

/** Operator for `ExprUnOp` */
enum ExpressionUnOperator {
    UNot,           /** Unary bitwise not */
};

/** Operator for `ExprUnOpCst` */
enum ExpressionUnOperatorCst {
    UCLsr,          /** Logical shift right of fixed shift */
    UCLsl,          /** Logical shift left of fixed shift */
    UCAsr,          /** Arithmetic shift right of fixed shift */
};


/** Base expression type, inherited by every "real" expression type */
struct ExpressionBase {
    ExpressionBase(const ExpressionType& type) : type(type) {}
    ExpressionType type;    /** Type of the expression (used for casts) */
};

/** Integer constant (`ExprConst`) */
struct ExpressionConst : ExpressionBase {
    ExpressionConst(unsigned val) : ExpressionBase(ExprConst), val(val) {}

    unsigned val;           /** Numeric value */
};

/** End variable expression (`ExprVar`) */
struct ExpressionVar : ExpressionBase {
    ExpressionVar(int id) : ExpressionBase(ExprVar), id(id) {}

    int id;                 /** Id of the input pin referred */
};

/** Binary operator expression (`ExprBinOp`) */
struct ExpressionBinOp : ExpressionBase {
    ExpressionBinOp(const ExpressionBase& left,
            const ExpressionBase& right,
            ExpressionBinOperator op) :
        ExpressionBase(ExprBinOp), left(left), right(right), op(op) {}

    ExpressionBase left, right;
    ExpressionBinOperator op;       /** Operator */
};

/** Unary operator expression (`ExprUnOp`) */
struct ExpressionUnOp : ExpressionBase {
    ExpressionUnOp(const ExpressionBase& expr, ExpressionUnOperator op) :
       ExpressionBase(ExprUnOp), expr(expr), op(op) {}

    ExpressionBase expr;            /** Sub-expression */
    ExpressionUnOperator op;        /** Operator */
};

/** Unary operator with constant (`ExprUnOpCst`) */
struct ExpressionUnOpCst : ExpressionBase {
    ExpressionUnOpCst(const ExpressionBase& expr,
            int val,
            ExpressionUnOperatorCst op) :
        ExpressionBase(ExprUnOpCst), expr(expr), val(val), op(op) {}

    ExpressionBase expr;
    int val;                        /** Constant associated */
    ExpressionUnOperatorCst op;     /** Operator */
};

/** Take a subword out of a word (`ExprSlice`) */
struct ExpressionSlice : ExpressionBase {
    ExpressionSlice(const ExpressionBase& expr, unsigned beg, unsigned end) :
        ExpressionBase(ExprSlice), expr(expr), beg(beg), end(end) {}

    ExpressionBase expr;
    unsigned beg;           /** First index (inclusive) of the subword */
    unsigned end;           /** Last index (exclusive) of the subword */
};

/** Concatenate two words (`ExprMerge`) */
struct ExpressionMerge : ExpressionBase {
    ExpressionMerge(const ExpressionBase& left, const ExpressionBase& right) :
        ExpressionBase(ExprMerge), left(left), right(right) {}

    ExpressionBase left, right;
};

