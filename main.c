#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mikal_type.h"
#include "reader.h"
#include "env.h"
#include "eval.h"
#include "buildin_func.h"
#include "gc.h"

#ifdef LREADLINE
    #include "readline/readline.h"
#endif

extern gc_buffer gcbuf;
char prompt[128];

#ifndef LREADLINE
static char* readline(char *prompt){
    printf("%s", prompt);
    fflush(stdout);
    char *buf = (char*)malloc(256);
    memset(buf, 0, 256);
    char ch;
    int pt = 0, paren_match = 0;
    while(read(0, &ch, 1)){
        if(ch == '\n')
            if(!paren_match) break;
            else continue;

        if(ch == '(') paren_match--;
        else if(ch == ')') paren_match++;

        buf[pt] = ch;
        pt++;
    }

    if(pt == 0){
        free(buf);
        return NULL;
    }
    
    return buf;
}
#endif


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

static void add_meta_bound(struct env_t *env, char *name, mikal_func func, enum mikal_op_type op_type, enum func_return ret_type){
    mikal_t *op_mikal = URet_val(make_symbol(name), mikal_t*);

    mikal_t *func_mikal = URet_val(make_func(func, op_type, ret_type), mikal_t*);

    add_env_entry(env, op_mikal, func_mikal);

    destroy_mikal(op_mikal);
    destroy_mikal(func_mikal);
}

static void init_meta_env(struct env_t *env){
    env->ref_cnt = 2;
    add_meta_bound(env, "+", add_mikal, OP_ARITH, RETURN_VAL);
    add_meta_bound(env, "-", sub_mikal, OP_ARITH, RETURN_VAL);
    add_meta_bound(env, "*", mul_mikal, OP_ARITH, RETURN_VAL);
    add_meta_bound(env, "/", div_mikal, OP_ARITH, RETURN_VAL);
    add_meta_bound(env, "lambda", make_clos_mikal, OP_LAMBDA, RETURN_VAL);
    add_meta_bound(env, "cons", cons_mikal, OP_CONS, RETURN_VAL);
    add_meta_bound(env, "car", car_mikal, OP_CONS, RETURN_REF);
    add_meta_bound(env, "cdr", cdr_mikal, OP_CONS, RETURN_REF);
    add_meta_bound(env, "define", def_mikal, OP_DEF, RETURN_REF);
    add_meta_bound(env, "let", let_mikal, OP_LET, RETURN_REF);
    add_meta_bound(env, "set!", set_mikal, OP_SET, RETURN_REF);
    add_meta_bound(env, "=", beq_mikal, OP_BOOL, RETURN_VAL);
    add_meta_bound(env, "<", blt_mikal, OP_BOOL, RETURN_VAL);
    add_meta_bound(env, ">", bgt_mikal, OP_BOOL, RETURN_VAL);
    add_meta_bound(env, "if", if_mikal, OP_IF, RETURN_VAL);

    env->fa_env = env;

    return;
}

static void toggle_args(int argc, char *argv[]){
    char *argstr;
    int toggled = 0;
    for(int i=1; i<argc; i++){
        toggled = 1;
        argstr = argv[i];
        if(argstr[0] == '-'){
            switch (argstr[1]){
                case 'q':
                    snprintf(prompt, sizeof(prompt), "");
                    break;

                default:
                    snprintf(prompt, sizeof(prompt), "user> ");
                    break;
            }
        }else{
            snprintf(prompt, sizeof(prompt), "user> ");
            //open file
        }
        
    }
    
    if(!toggled){
        snprintf(prompt, sizeof(prompt), "user> ");
    }

    return;
}


int main(int argc, char *argv[]){

    struct AST_Node *ast;

    URet eval_result, call_ret;
    toggle_args(argc, argv);
    struct env_t *meta_env = URet_val(init_env(), struct env_t*);
    
    init_meta_env(meta_env);

    init_gcbuffer(MAX_GC_RECORDS);
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
    destroy_meta_env(meta_env);
}
