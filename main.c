#include "stdio.h"
#include "stdlib.h"
#include "mikal_type.h"
#include "reader.h"
#include "readline/readline.h"
#include "env.h"
#include "eval.h"
#include "buildin_func.h"
#include "gc.h"

extern gc_buffer gcbuf;

struct AST_Node *READ(char *prompt){
    char *user_in;
    user_in = readline(prompt);

    if(user_in == NULL){
        return NULL;
    }

    struct AST_Node *AST_root = line_reader(user_in);
    return AST_root;
}


URet EVAL(struct AST_Node *root, struct env_t *env){
    URet ret = eval(root, env);
    
    return ret;
}

void PRINT(URet eval_result){
    if(URet_state(eval_result) != GOOD){
        fprintf(stderr, "Failed to evaluate\n");
    }

    mikal_t *eval_val = URet_val(eval_result, mikal_t*);

    print_mikal(eval_val);

    add_gc_mikal(eval_val);

    printf("\n");
    return;
}


static void init_meta_env(struct env_t *env){
    mikal_t *plus_op = URet_val(make_symbol("+"), mikal_t*);
    mikal_t *minus_op = URet_val(make_symbol("-"), mikal_t*);
    mikal_t *mul_op = URet_val(make_symbol("*"), mikal_t*);
    mikal_t *div_op = URet_val(make_symbol("/"), mikal_t*);
    mikal_t *lambda_op = URet_val(make_symbol("lambda"), mikal_t*);
    mikal_t *cons_op = URet_val(make_symbol("cons"), mikal_t*);
    mikal_t *car_op = URet_val(make_symbol("car"), mikal_t*);
    mikal_t *cdr_op = URet_val(make_symbol("cdr"), mikal_t*);
    mikal_t *def_op = URet_val(make_symbol("def"), mikal_t*);
    mikal_t *set_op = URet_val(make_symbol("set"), mikal_t*);
    mikal_t *let_op = URet_val(make_symbol("let"), mikal_t*);
    mikal_t *beq_op = URet_val(make_symbol("="), mikal_t*);
    mikal_t *if_op = URet_val(make_symbol("if"), mikal_t*);
    
    mikal_t *add_func = URet_val(make_func(add_mikal, OP_ARITH, RETURN_VAL), mikal_t*);
    mikal_t *sub_func = URet_val(make_func(sub_mikal, OP_ARITH, RETURN_VAL), mikal_t*);
    mikal_t *mul_func = URet_val(make_func(mul_mikal, OP_ARITH, RETURN_VAL), mikal_t*);
    mikal_t *div_func = URet_val(make_func(div_mikal, OP_ARITH, RETURN_VAL), mikal_t*);
    mikal_t *lambda_func = URet_val(make_func(make_clos_mikal, OP_LAMBDA, RETURN_VAL), mikal_t*);
    mikal_t *cons_func = URet_val(make_func(cons_mikal, OP_CONS, RETURN_VAL), mikal_t*);
    mikal_t *car_func = URet_val(make_func(car_mikal, OP_CONS, RETURN_REF), mikal_t*);
    mikal_t *cdr_func = URet_val(make_func(cdr_mikal, OP_CONS, RETURN_REF), mikal_t*);
    mikal_t *def_func = URet_val(make_func(def_mikal, OP_DEF, RETURN_REF), mikal_t*);
    mikal_t *set_func = URet_val(make_func(set_mikal, OP_SET, RETURN_REF), mikal_t*);
    mikal_t *let_func = URet_val(make_func(let_mikal, OP_LET, RETURN_REF), mikal_t*);
    mikal_t *beq_func = URet_val(make_func(beq_mikal, OP_BOOL, RETURN_VAL), mikal_t*);
    mikal_t *if_func = URet_val(make_func(if_mikal, OP_IF, RETURN_VAL), mikal_t*);

    add_env_entry(env, plus_op, add_func);
    add_env_entry(env, minus_op, sub_func);
    add_env_entry(env, mul_op, mul_func);
    add_env_entry(env, div_op, div_func);
    add_env_entry(env, lambda_op, lambda_func);
    add_env_entry(env, car_op, car_func);
    add_env_entry(env, cdr_op, cdr_func);
    add_env_entry(env, cons_op, cons_func);
    add_env_entry(env, def_op, def_func);
    add_env_entry(env, set_op, set_func);
    add_env_entry(env, let_op, let_func);
    add_env_entry(env, beq_op, beq_func);
    add_env_entry(env, if_op, if_func);
    
    destroy_mikal(plus_op);
    destroy_mikal(minus_op);
    destroy_mikal(mul_op);
    destroy_mikal(div_op);
    destroy_mikal(lambda_op);
    destroy_mikal(cons_op);
    destroy_mikal(car_op);
    destroy_mikal(cdr_op);
    destroy_mikal(def_op);
    destroy_mikal(set_op);
    destroy_mikal(let_op);
    destroy_mikal(beq_op);
    destroy_mikal(if_op);
    
    destroy_mikal(add_func);
    destroy_mikal(sub_func);
    destroy_mikal(mul_func);
    destroy_mikal(div_func);
    destroy_mikal(lambda_func);
    destroy_mikal(cons_func);
    destroy_mikal(car_func);
    destroy_mikal(cdr_func);
    destroy_mikal(def_func);
    destroy_mikal(set_func);
    destroy_mikal(let_func);
    destroy_mikal(beq_func);
    destroy_mikal(if_func);

    env->fa_env = env;

    return;
}

int main(void){

    char prompt[128];
    struct AST_Node *ast;

    URet eval_result, call_ret;


    struct env_t *meta_env = URet_val(init_env(), struct env_t*);
    
    init_meta_env(meta_env);

    init_gcbuffer(64);
    snprintf(prompt, sizeof(prompt), "user> ");
    while(1){
        ast = READ(prompt);

        if(ast == NULL){
            break;
        }
        eval_result = EVAL(ast, meta_env);
        PRINT(eval_result);
        AST_destroy(ast);

        gc_clear_all();
    }
    destroy_env(meta_env);
}
