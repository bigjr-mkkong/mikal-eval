#include "stdio.h"
#include "stdlib.h"
#include "mikal_type.h"
#include "reader.h"
#include "readline/readline.h"
#include "env.h"
#include "eval.h"

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

    print_mikal(URet_val(eval_result, mikal_t*));

    printf("\n");
    return;
}


static void init_meta_env(struct env_t *env){
    mikal_t *plus_op = URet_val(make_symbol("+"), mikal_t*);
    mikal_t *minus_op = URet_val(make_symbol("-"), mikal_t*);
    mikal_t *mul_op = URet_val(make_symbol("*"), mikal_t*);
    mikal_t *div_op = URet_val(make_symbol("/"), mikal_t*);
    
    mikal_t *add_func = URet_val(make_func(add_mikal, OP_ARITH), mikal_t*);
    mikal_t *sub_func = URet_val(make_func(sub_mikal, OP_ARITH), mikal_t*);
    mikal_t *mul_func = URet_val(make_func(mul_mikal, OP_ARITH), mikal_t*);
    mikal_t *div_func = URet_val(make_func(div_mikal, OP_ARITH), mikal_t*);

    add_env_entry(env, plus_op, add_func);
    add_env_entry(env, minus_op, sub_func);
    add_env_entry(env, mul_op, mul_func);
    add_env_entry(env, div_op, div_func);

    env->fa_env = env;

    return;
}

int main(void){

    char prompt[128];
    struct AST_Node *ast;

    URet eval_result;


    struct env_t *meta_env = URet_val(init_env(), struct env_t*);
    
    init_meta_env(meta_env);
    
    snprintf(prompt, sizeof(prompt), "user> ");
    while(1){
        ast = READ(prompt);

        if(ast == NULL){
            break;
        }
        eval_result = EVAL(ast, meta_env);
        PRINT(eval_result);
        AST_destroy(ast);
    }
    destroy_env(meta_env);
}
