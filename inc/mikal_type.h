#ifndef __MIKAL_TYPE_H__
#define __MIKAL_TYPE_H__
#define MIKAL_MAGIC   0xc0ffee

enum mikal_types{
    MT_INTEGER,
    MT_SYMBOL,
    MT_OPERATOR,
    MT_STRING,      
    MT_CONS,        //list cons
    MT_AST,
    MT_FUNC,
    MT_NONE
};

enum mikal_op_type{
    OP_ENV,
    OP_CONS,
    OP_ARITH,
    OP_BRANCH,
    OP_LAMBDA,
    OP_UNDEF
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


typedef struct mikal_t{
    long long magic;
    union{
        long long integer;
        char *sym;
        char *str;
        char *op;
        URet (*func)(struct mikal_t**);
    };

    struct mikal_t *car;
    struct mikal_t *cdr;
    struct AST_Node *ast;

    enum mikal_types type;
    enum mikal_op_type op_type;
}mikal_t;

typedef URet (*mikal_func)(mikal_t**);

#define URet_val(x, type)    ((type)((x).val))
#define URet_state(x)       ((x).error_code)

#include "env.h"

int valid_mikal(mikal_t *addr);
URet make_integer(long long x);
URet make_symbol(char *sym_name);
URet make_operator(char *op_name);
URet make_string(char *str_name);
URet make_cons(mikal_t *car, mikal_t *cdr);
URet make_ast(struct AST_Node *ast);
URet make_func(mikal_func func, enum mikal_op_type type);

URet print_mikal(mikal_t *target);
URet destroy_mikal(mikal_t *target);

int mikal_cmp(mikal_t *val1, mikal_t *val2);

enum mikal_op_type which_op(char *str, struct env_t *env);
enum mikal_types which_mktype(char *str, struct env_t *env);

URet add_mikal(mikal_t **args);
URet sub_mikal(mikal_t **args);
URet mul_mikal(mikal_t **args);
URet div_mikal(mikal_t **args);

URet str2ll(char *str);

#endif
