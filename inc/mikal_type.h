#ifndef __MIKAL_TYPE_H__
#define __MIKAL_TYPE_H__
#define MIKAL_MAGIC   0xc0ffee

#define ROUND_UP(x) (((x) + 8) & (~7))

enum mikal_types{
    MT_INTEGER,
    MT_SYMBOL,
    MT_STRING,      
    MT_CONS,        //list cons
    MT_AST,
    MT_FUNC,
    MT_CLOSURE,
    MT_UNBOND_SYM,
    MT_NONE
};

enum mikal_op_type{
    OP_UNDEF,
    OP_CONS,
    OP_ARITH,
    OP_BRANCH,
    OP_LAMBDA,
    OP_CLOSURE,
    OP_DEF,
    OP_LET,
    OP_SET
};

enum func_return{
    RETURN_VAL,
    RETURN_REF,
};

#include "reader.h"

enum error_cases{
    GOOD,
    E_EMPTY,
    E_INVAL_ADDR,
    E_INVAL_TYPE,
    E_CASE_UNIMPL,
    E_ARITH_ERROR,
    E_NOSPACE_LEFT,
    E_OUTOFBOUND,
    E_DOUBLE_FREE,
    E_NOTFOUND,
    E_FAILED,
    E_UNDEF
};

typedef struct uret{
    union{
        long long val;
        void *addr;
    };
    enum error_cases error_code;
}URet;

#define MAX_PROCARGS    16

struct mikal_t;

typedef struct closure{
    struct mikal_t *args[MAX_PROCARGS]; //args should have a copy of arg symbols
    struct AST_Node *root; //root should have a copy of procedure AST
    struct env_t *env; //env should NOT have a copy
}Closure;


typedef struct mikal_t{
    long long magic;
    union{
        long long integer;
        char *sym;
        char *str;
        char *op;
        URet (*func)(struct mikal_t**, ...);
    };

    struct mikal_t *car;
    struct mikal_t *cdr;

    struct AST_Node *ast;

    struct closure *clos;

    enum mikal_types type;
    enum mikal_op_type op_type;
    enum func_return ret_type;
    int refcnt;
}mikal_t;

typedef URet (*mikal_func)(mikal_t**, ...);

#define URet_val(x, type)    ((type)((x).val))
#define URet_state(x)       ((x).error_code)

#include "env.h"

int valid_mikal(mikal_t *addr);
URet make_integer(long long x);
URet make_symbol(char *sym_name);
//URet make_operator(char *op_name);
URet make_string(char *str_name);
URet make_cons(mikal_t *car, mikal_t *cdr);
URet make_ast(struct AST_Node *ast);
URet make_func(mikal_func func, enum mikal_op_type type, enum func_return return_type);
URet make_closure(mikal_t *args[], struct AST_Node *root, struct env_t *env);

URet print_mikal(mikal_t *target);
URet destroy_mikal(mikal_t *target);
URet move_mikal(mikal_t *dst, mikal_t *src);
URet copy_mikal(mikal_t *src);

int mikal_cmp(mikal_t *val1, mikal_t *val2);

enum mikal_op_type which_op(char *str, struct env_t *env);
enum mikal_types which_mktype(char *str, struct env_t *env);

URet str2ll(char *str);

#endif
