/**
 * Implement the C api
 **/

#include "isomatch.h"
#include "../isomatch.h"

#include <exception>
#include <type_traits>

/******************/
/* Internal state */
/******************/

static isom_rc lastError = ISOM_RC_OK;

/***************************/
/* Static helper functions */
/***************************/

class IsomError : public std::exception {
    public:
        IsomError(isom_rc error) : _error(error) {}
        const char* what() const noexcept { return isom_strerror(_error); }
        inline isom_rc error() const { return _error; }

    private:
        isom_rc _error;
};

static isom_rc handleError(const IsomError& err) {
    lastError = err.error();
    return lastError;
}

static CircuitTree* circuitOfHandle(circuit_handle handle) {
    if(handle == nullptr)
        throw IsomError(ISOM_RC_NULLPTR);

    CircuitTree* out = static_cast<CircuitTree*>(handle);
    return out;
}

template <typename Circ>
static Circ* circuitOfHandle(circuit_handle handle) {
    static_assert(std::is_base_of<CircuitTree, Circ>::value,
            "Template parameter must derive from CircuitTree");
    CircuitTree* circ = circuitOfHandle(handle);
    Circ* out = dynamic_cast<Circ*>(circ);
    if(out == nullptr)
        throw IsomError(ISOM_RC_DOMAIN);
    return out;
}

static ExpressionBase* exprOfHandle(expr_handle handle) {
    if(handle == nullptr)
        throw IsomError(ISOM_RC_NULLPTR);

    ExpressionBase* out = static_cast<ExpressionBase*>(handle);
    return out;
}

static WireId* wireOfHandle(wire_handle wire, CircuitGroup* context) {
    if(context == nullptr)
        throw IsomError(ISOM_RC_NO_PARENT);
    return context->wireManager()->wire(wire);
}

/** Sets the given `parent` as the gate's parent */
static void setParent(circuit_handle parent, CircuitTree* self) {
    CircuitGroup* par = circuitOfHandle<CircuitGroup>(parent);
    par->addChild(self);
}

/**********************/
/* API implementation */
/**********************/

isom_rc isom_last_error() {
    return lastError;
}

const char* isom_strerror(isom_rc err_code) {
    switch(err_code) {
        case ISOM_RC_OK:
            return "no error";
        case ISOM_RC_NULLPTR:
            return "bad handle (null pointer)";
        case ISOM_RC_DOMAIN:
            return "bad handle (cannot cast to right pointer type)";
        case ISOM_RC_NO_PARENT:
            return "this circuit handle has no parent group (see "
                "`build_group_add_child`) and is required to have one in this "
                "context. This applies eg. when you try to access a wire.";
        case ISOM_RC_ERROR:
        default:
            return "generic error, please submit a bug report";
    }
}

// === Generic

int freeze_circuit(circuit_handle circuit) {
    try {
        circuitOfHandle(circuit)->freeze();
        return ISOM_RC_OK;
    } catch(const IsomError& e) {
        return handleError(e);
    }
}

int free_circuit(circuit_handle circuit) {
    try {
        delete circuitOfHandle(circuit);
        return ISOM_RC_OK;
    } catch(const IsomError& e) {
        return handleError(e);
    }
}

// === Assert

circuit_handle build_assert(circuit_handle parent,
        const char* name,
        expr_handle expr)
{
    try {
        CircuitAssert* out = new CircuitAssert(name, exprOfHandle(expr));
        setParent(parent, out);
        return out;
    } catch(const IsomError& e) {
        handleError(e);
        return nullptr;
    }
}

int build_assert_add_input(circuit_handle self, wire_handle wire) {
    try {
        CircuitAssert* asser = circuitOfHandle<CircuitAssert>(self);
        asser->addInput(wireOfHandle(wire, asser->ancestor()));
        return ISOM_RC_OK;
    } catch(const IsomError& e) {
        return handleError(e);
    }
}

// === Comb

circuit_handle build_comb(circuit_handle parent) {
    try {
        CircuitComb* out = new CircuitComb();
        setParent(parent, out);
        return out;
    } catch(const IsomError& e) {
        handleError(e);
        return nullptr;
    }
}

int build_comb_add_input(circuit_handle self, wire_handle wire) {
    try {
        CircuitComb* circ = circuitOfHandle<CircuitComb>(self);
        circ->addInput(wireOfHandle(wire, circ->ancestor()));
        return ISOM_RC_OK;
    } catch(const IsomError& e) {
        return handleError(e);
    }
}

int build_comb_add_output(circuit_handle self,
        wire_handle wire,
        expr_handle expr)
{
    try {
        CircuitComb* circ = circuitOfHandle<CircuitComb>(self);
        circ->addOutput(exprOfHandle(expr),
                wireOfHandle(wire, circ->ancestor()));
        return ISOM_RC_OK;
    } catch(const IsomError& e) {
        return handleError(e);
    }
}

// === Delay

circuit_handle build_delay(circuit_handle parent,
        wire_handle input,
        wire_handle output)
{
    try {
        CircuitGroup* par = circuitOfHandle<CircuitGroup>(parent);
        CircuitDelay* delay = new CircuitDelay(
                wireOfHandle(input, par),
                wireOfHandle(output, par));
        setParent(parent, delay);
        return delay;
    } catch(const IsomError& e) {
        handleError(e);
        return nullptr;
    }
}

// === Group

circuit_handle build_group(const char* name) {
    try {
        CircuitGroup* out = new CircuitGroup(name);
        return out;
    } catch(const IsomError& e) {
        handleError(e);
        return nullptr;
    }
}

int build_group_add_child(circuit_handle self, circuit_handle child) {
    try {
        CircuitGroup* group = circuitOfHandle<CircuitGroup>(self);
        group->addChild(circuitOfHandle(child));
        return ISOM_RC_OK;
    } catch(const IsomError& e) {
        return handleError(e);
    }
}

int build_group_add_input(circuit_handle self,
        wire_handle actual,
        wire_handle formal)
{
    try {
        CircuitGroup* group = circuitOfHandle<CircuitGroup>(self);
        if(group->ancestor() != nullptr) {
            group->addInput(IOPin(
                        wireOfHandle(formal, group->ancestor()),
                        wireOfHandle(actual, group),
                        group));
        }
        else
            group->addInput(formal, wireOfHandle(actual, group));

        return ISOM_RC_OK;
    } catch(const IsomError& e) {
        return handleError(e);
    }
}

int build_group_add_output(circuit_handle self,
        wire_handle actual,
        wire_handle formal)
{
    try {
        CircuitGroup* group = circuitOfHandle<CircuitGroup>(self);
        if(group->ancestor() != nullptr) {
            group->addOutput(IOPin(
                        wireOfHandle(formal, group->ancestor()),
                        wireOfHandle(actual, group),
                        group));
        }
        else
            group->addOutput(formal, wireOfHandle(actual, group));

        return ISOM_RC_OK;
    } catch(const IsomError& e) {
        return handleError(e);
    }
}

// === Tristate

circuit_handle build_tristate(circuit_handle parent,
        wire_handle from,
        wire_handle to,
        wire_handle enable)
{
    try {
        CircuitGroup* par = circuitOfHandle<CircuitGroup>(parent);
        CircuitTristate* out = new CircuitTristate(
                wireOfHandle(from, par),
                wireOfHandle(to, par),
                wireOfHandle(enable, par));
        setParent(parent, out);
        return out;
    } catch(const IsomError& e) {
        handleError(e);
        return nullptr;
    }
}

// === Expressions

expr_handle build_expr_const(unsigned val) {
    return new ExpressionConst(val);
}

expr_handle build_expr_var(int input_pin) {
    return new ExpressionVar(input_pin);
}

expr_handle build_expr_binop(enum isom_expr_binop op,
        expr_handle left,
        expr_handle right)
{
    return new ExpressionBinOp(
            exprOfHandle(left),
            exprOfHandle(right),
            (expr::ExpressionBinOperator)op);
}

expr_handle build_expr_unop(enum isom_expr_unop op, expr_handle expr) {
    return new ExpressionUnOp(
            exprOfHandle(expr),
            (expr::ExpressionUnOperator)op);
}

expr_handle build_expr_unop_cst(enum isom_expr_unop_cst op,
        int param,
        expr_handle expr)
{
    return new ExpressionUnOpCst(
            exprOfHandle(expr),
            param,
            (expr::ExpressionUnOperatorCst)op);
}

expr_handle build_expr_slice(expr_handle expr, unsigned beg, unsigned end) {
    return new ExpressionSlice(exprOfHandle(expr), beg, end);
}

expr_handle build_expr_merge(expr_handle left, expr_handle right) {
    return new ExpressionMerge(exprOfHandle(left), exprOfHandle(right));
}

void free_expression(expr_handle expr) {
    delete exprOfHandle(expr);
}
