#pragma once

/**
 * @brief C API header file for the Isomatch library
 * @file c_api/isomatch.h
 *
 * General purpose notes on this API:
 *
 * - Every `free`-like specific function is recursive, where possible; eg.
 *   freeing a hierarchy group will free the underlying circuits, freeing a
 *   comb gate will free its expressions, ...
 * - Every function returning a pointer will return `NULL` if an error
 *   occurred.
 **/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* Type declarations                                                         */
/*****************************************************************************/
typedef uint64_t sig_t;         ///< Type of a circuit signature
typedef void* circuit_handle;   ///< Value representing a circuit
typedef void* expr_handle;      ///< Value representing an expression
typedef const char* wire_handle;    ///< A wire name

/*****************************************************************************/
/* Error handling                                                            */
/*****************************************************************************/

/***********************************/
/* Return values (aka error codes) */
/***********************************/

/// Return codes of the `int`-returning functions in this API
typedef enum isom_rc {
    ISOM_RC_OK      = 0, ///< Everything went fine
    ISOM_RC_NULLPTR = 1, ///< One of the parameters was a null pointer
    ISOM_RC_DOMAIN  = 2, ///< One of the parameters has the wrong pointer type
    ISOM_RC_NO_PARENT = 3, ///< A circuit outside of a group was used, while
                           ///< it needed to be in a group
    ISOM_RC_BADHEX = 4,    ///< Non hexadecimal string was supplied
    ISOM_RC_ERROR   = 255, ///< An undefined error occurred
} isom_rc;
//TODO complete this list

/*****************************/
/* Error reporting functions */
/*****************************/

/// Return the error code of the last error that occurred
isom_rc isom_last_error();

/** Return a human-friendly string describing the given error.
 * Note that you don't have to `free` the returned string. */
const char* isom_strerror(isom_rc err_code);

/*****************************************************************************/
/* Enumerations                                                              */
/*****************************************************************************/

/***************/
/* Expressions */
/***************/

/** Operator for `ExprBinOp` */
enum isom_expr_binop {
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
enum isom_expr_unop {
    UNot,           ///< Unary bitwise not
};

/** Operator for `ExprUnOpCst` */
enum isom_expr_unop_cst {
    UCLsr,          ///< Logical shift right of fixed shift
    UCLsl,          ///< Logical shift left of fixed shift
    UCAsr,          ///< Arithmetic shift right of fixed shift
};

/*****************************************************************************/
/* Circuit construction                                                      */
/*****************************************************************************/

/*********************/
/* Generic functions */
/*********************/

/** Freeze a circuit, allowing to compute signatures and matchings
 * @return 0 on success, > 0 on failure
 */
int freeze_circuit(circuit_handle circuit);

/** Free the given previously created circuit
 * @return 0 on success, > 0 on failure
 */
int free_circuit(circuit_handle circuit);

/****************/
/* Assert gates */
/****************/

/// Build an assert gate
circuit_handle build_assert(circuit_handle parent,
        const char* name,
        expr_handle expr);

/** Add an input wire to a given gate (order matters!)
 * @return 0 on success, > 0 on failure
 */
int build_assert_add_input(circuit_handle self, wire_handle wire);

/**************/
/* Comb gates */
/**************/

/// Build a comb gate, which will need to be edited with the functions below
circuit_handle build_comb(circuit_handle parent);

/** Add an input wire to a given combinator gate
 * @return 0 on success, > 0 on failure
 */
int build_comb_add_input(circuit_handle self, wire_handle wire);

/** Add an output wire, alongside with an expression, to a given comb gate
 * @return 0 on success, > 0 on failure
 */
int build_comb_add_output(circuit_handle self,
        wire_handle wire,
        expr_handle expr);

/***************/
/* Delay gates */
/***************/

/// Build a delay gate
circuit_handle build_delay(circuit_handle parent,
        wire_handle input,
        wire_handle output);

/*************************/
/* Hierarchy group gates */
/*************************/

/** Build a hierarchy group gate, which will need to be edited with the
 * functions below.
 * A group does not necessarily belong to another group, thus there is no
 * `parent` argument, and you must call `build_group_add_child` yourself. */
circuit_handle build_group(const char* name);

/** Add the given circuit as a child of the given group.
 * @return 0 on success, > 0 on failure
 */
int build_group_add_child(circuit_handle self, circuit_handle child);

/** Add an input pin to the given circuit group.
 * @param self  The group to work on
 * @param actual The name of the pin when accessed from inside the group
 * @param formal The name of the pin when accessed from outside the group
 * @return 0 on success, > 0 on failure
 */
int build_group_add_input(circuit_handle self,
        wire_handle actual,
        wire_handle formal);

/** Add an output pin to the given circuit group.
 * @param self The group to work on
 * @param actual The name of the pin when accessed from inside the group
 * @param formal The name of the pin when accessed from outside the group
 * @return 0 on success, > 0 on failure
 */
int build_group_add_output(circuit_handle self,
        wire_handle actual,
        wire_handle formal);

/******************/
/* Tristate gates */
/******************/

/// Build a tristate gate
circuit_handle build_tristate(circuit_handle parent,
        wire_handle from,
        wire_handle to,
        wire_handle enable);

/***************/
/* Expressions */
/***************/

/// Build a constant expression node
expr_handle build_expr_const(unsigned val);

/** Build a long constant expression node
 * @param value Hexadecimal string [0-9a-fA-F]+
 */
expr_handle build_expr_longconst(const char* value); // TODO

/// Build a variable expression node referring to the `n`th input of the gate
expr_handle build_expr_var(int input_pin);

/// Build a binary operator expression node
expr_handle build_expr_binop(enum isom_expr_binop op,
        expr_handle left,
        expr_handle right);

/// Build a unary operator expression node
expr_handle build_expr_unop(enum isom_expr_unop op, expr_handle expr);

/// Build a unary operator with constant parameter expression node
expr_handle build_expr_unop_cst(enum isom_expr_unop_cst op,
        int param,
        expr_handle expr);

/// Build a slice expression node (beginning inclusive, end exclusive)
expr_handle build_expr_slice(expr_handle expr, unsigned beg, unsigned end);

/// Build a merge expression node
expr_handle build_expr_merge(expr_handle left, expr_handle right);

/// Free the given previously created expression
int free_expression(expr_handle expr);

/*****************************************************************************/
/* Signature                                                                 */
/*****************************************************************************/

/** Computes a signature for the given circuit handle */
sig_t sign(circuit_handle circuit);

/** Computes a signature for the given circuit handle with a precision level of
 * `precision_level` */
sig_t sign_with_precision(circuit_handle circuit, unsigned precision_level);

/*****************************************************************************/
/* Circuit matching                                                          */
/*****************************************************************************/

// TODO TO BE IMPLEMENTED

#ifdef __cplusplus
}
#endif
