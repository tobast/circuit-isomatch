/** Defines an AST for the expressions used in gates (combinator gates,
 * assertion gates).
 */

#pragma once

#include "signatureConstants.h"

/** Type of expression (used to cast to the right `struct`). */
enum ExpressionType {
    ExprVar,        ///< End variable
    ExprConst,      ///< Integer constant
    ExprBinOp,      ///< Binary operator (+, AND, ...)
    ExprUnOp,       ///< Unary operator (NOT, ...)
    ExprUnOpCst,    ///< Unary operator with constant (CstLSL, ...)
    ExprSlice,      ///< Take a subword out of a word
    ExprMerge,      ///< Concatenate two words into a longer one
};

/***********************************************************************/
/* WARNING! The following enums are duplicated (on purpose) inside the */
/* C API. Change this API as well if you are to change these enums!    */
/***********************************************************************/

/** Operator for `ExprBinOp` */
enum ExpressionBinOperator {
    BAnd,           ///< Bitwise and
    BOr,            ///< Bitwise or
    BXor,           ///< Bitwise exclusive or
    BAdd,           ///< Addition
    BSub,           ///< Subtraction
    BMul,           ///< Multiplication
    BDiv,           ///< Division
    BMod,           ///< Modulus
    BLsr,           ///< Logical shift right
    BLsl,           ///< Logical shift left
    BAsr,           ///< Arithmetic shift right
};

/** Operator for `ExprUnOp` */
enum ExpressionUnOperator {
    UNot,           ///< Unary bitwise not
};

/** Operator for `ExprUnOpCst` */
enum ExpressionUnOperatorCst {
    UCLsr,          ///< Logical shift right of fixed shift
    UCLsl,          ///< Logical shift left of fixed shift
    UCAsr,          ///< Arithmetic shift right of fixed shift
};


/** Base expression type, inherited by every "real" expression type */
struct ExpressionBase {
    ExpressionBase(const ExpressionType& type) : type(type) {}
    virtual ~ExpressionBase() {}
    ExpressionType type;    ///< Type of the expression (used for casts)

    /** Compute a signature for this expression */
    virtual sig_t sign() const = 0; // FIXME memoize?
};

/** Integer constant (`ExprConst`) */
struct ExpressionConst : ExpressionBase {
    ExpressionConst(unsigned val) : ExpressionBase(ExprConst), val(val) {}

    unsigned val;           ///< Numeric value

    virtual sig_t sign() const;
};

/** End variable expression (`ExprVar`) */
struct ExpressionVar : ExpressionBase {
    ExpressionVar(int id) : ExpressionBase(ExprVar), id(id) {}

    int id;                 ///< Id of the input pin referred

    virtual sig_t sign() const;
};

/** Binary operator expression (`ExprBinOp`) */
struct ExpressionBinOp : ExpressionBase {
    ExpressionBinOp(ExpressionBase* left,
            ExpressionBase* right,
            ExpressionBinOperator op) :
        ExpressionBase(ExprBinOp), left(left), right(right), op(op) {}
    virtual ~ExpressionBinOp() {
        delete left;
        delete right;
    }

    ExpressionBase *left, *right;
    ExpressionBinOperator op;       ///< Operator

    virtual sig_t sign() const;
};

/** Unary operator expression (`ExprUnOp`) */
struct ExpressionUnOp : ExpressionBase {
    ExpressionUnOp(ExpressionBase* expr, ExpressionUnOperator op) :
       ExpressionBase(ExprUnOp), expr(expr), op(op) {}
    virtual ~ExpressionUnOp() {
        delete expr;
    }

    ExpressionBase *expr;           ///< Sub-expression
    ExpressionUnOperator op;        ///< Operator

    virtual sig_t sign() const;
};

/** Unary operator with constant (`ExprUnOpCst`) */
struct ExpressionUnOpCst : ExpressionBase {
    ExpressionUnOpCst(ExpressionBase* expr,
            int val,
            ExpressionUnOperatorCst op) :
        ExpressionBase(ExprUnOpCst), expr(expr), val(val), op(op) {}
    virtual ~ExpressionUnOpCst() {
        delete expr;
    }

    ExpressionBase *expr;
    int val;                        ///< Constant associated
    ExpressionUnOperatorCst op;     ///< Operator

    virtual sig_t sign() const;
};

/** Take a subword out of a word (`ExprSlice`) */
struct ExpressionSlice : ExpressionBase {
    ExpressionSlice(ExpressionBase* expr, unsigned beg, unsigned end) :
        ExpressionBase(ExprSlice), expr(expr), beg(beg), end(end) {}
    virtual ~ExpressionSlice() {
        delete expr;
    }

    ExpressionBase *expr;
    unsigned beg;           ///< First index (inclusive) of the subword
    unsigned end;           ///< Last index (exclusive) of the subword

    virtual sig_t sign() const;
};

/** Concatenate two words (`ExprMerge`) */
struct ExpressionMerge : ExpressionBase {
    ExpressionMerge(ExpressionBase* left, ExpressionBase* right) :
        ExpressionBase(ExprMerge), left(left), right(right) {}
    virtual ~ExpressionMerge() {
        delete left;
        delete right;
    };

    ExpressionBase *left, *right;

    virtual sig_t sign() const;
};
