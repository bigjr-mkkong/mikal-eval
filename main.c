#include "stdio.h"
#include "stdlib.h"
#include "types.h"
#include "readline/readline.h"
#include "reader.h"
#include "env.h"
/*
 * TODO:
 * make "let" work
 */

struct Gen_type_t *mikal_add(struct Gen_type_t **list_add){
    struct Gen_type_t *adder = NULL;
    long sum = 0;
    for(int i=0; list_add[i] != NULL; i++){
        adder = list_add[i];
        if(adder->type != TYPE_INTEGER){
            fprintf(stderr, "Failed to add: number is in incorrect type\n");
            return NULL;
        }
        sum += adder->value.integer;
    }

    struct Gen_type_t *result;
    result = make_integer(sum);

    return result;
}

struct Gen_type_t *mikal_sub(struct Gen_type_t **list_sub){
    struct Gen_type_t *sub1 = list_sub[0];
    struct Gen_type_t *sub2 = list_sub[1];

    if(sub1->type != TYPE_INTEGER){
        fprintf(stderr, "Failed to subtract: number 1 is in incorrect type\n");
        return NULL;
    }
    if(sub2->type != TYPE_INTEGER){
        fprintf(stderr, "Failed to subtract: number 2 is in incorrect type\n");
        return NULL;
    }

    struct Gen_type_t *result;
    result = make_integer(sub1->value.integer - sub2->value.integer);

    return result;
}

struct Gen_type_t *mikal_mul(struct Gen_type_t **list_mul){
    struct Gen_type_t *mul_num = NULL;
    long mul_result = 1;
    for(int i=0; list_mul[i] != NULL; i++){
        mul_num = list_mul[i];
        if(mul_num->type != TYPE_INTEGER){
            fprintf(stderr, "Failed to mul: number is in incorrect type\n");
            return NULL;
        }
        mul_result *= mul_num->value.integer;
    }

    struct Gen_type_t *result;
    result = make_integer(mul_result);

    return result;
}

struct Gen_type_t *mikal_div(struct Gen_type_t **list_div){
    struct Gen_type_t *div1 = list_div[0];
    struct Gen_type_t *div2 = list_div[1];

    if(div1->type != TYPE_INTEGER){
        fprintf(stderr, "Failed to divid: number 1 is in incorrect type\n");
        return NULL;
    }
    if(div2->type != TYPE_INTEGER){
        fprintf(stderr, "Failed to divid: number 2 is in incorrect type\n");
        return NULL;
    }

    if(div2->value.integer == 0){
        fprintf(stderr, "Failed to divid: number 2 is zero\n");
        return NULL;
    }

    struct Gen_type_t *result;
    result = make_integer(div1->value.integer / div2->value.integer);

    return result;
}

struct env_t *env_apply(enum env_op_type op, struct Gen_type_t **name, struct Gen_type_t **value, struct env_t *env){
    struct env_entry *target;
    char *sym_name;
    if(op == ENVOP_DEFINE){
        for(int i=0; name[i] && value[i]; i++){
            sym_name = name[i]->value.symbol;
            target = lookup_single_env(env, sym_name);
            if(target == NULL){
                add_env_integer(env, sym_name, value[i]);
            }else{
                memcpy(target->value.gen_val, value[i], sizeof(struct Gen_type_t));
            }
        }
    }else if(op == ENVOP_LET){
        struct env_t *newenv = init_env();
        newenv->fa_env = env;
        for(int i=0; name[i] && value[i]; i++){
            sym_name = name[i]->value.symbol;
            add_env_integer(newenv, sym_name, value[i]);
        }
        return newenv;
    }else if(op == ENVOP_SET){
        for(int i=0; name[i] && value[i]; i++){
            sym_name = name[i]->value.symbol;
            target = lookup_env(env, sym_name);
       
            if(target == NULL){
                fprintf(stderr, "Failed to set: symbol not found\n");
                return NULL;
            }else{
                memcpy(target->value.gen_val, value[i], sizeof(struct Gen_type_t));
            }
        }
    }else if(op == ENVOP_UNDEFINED){
        fprintf(stderr, "env_apply: operation undefined\n");
        return NULL;
    }else{
        fprintf(stderr, "env_apply: should not be here\n");
        return NULL;
    }
}

struct Gen_type_t *apply(struct Gen_type_t *op, struct Gen_type_t **args, struct env_t *env){
    struct env_entry *ent;
    struct Gen_type_t *ret = NULL;

    switch (op->type){
        case TYPE_OPERATOR:
            ent = lookup_env(env, op->value.op);
            if(ent == NULL){
                fprintf(stderr, "operator %s undefined\n", op->value.op);
            }
            ret = ent->value.func(args);
            break;

        default:
            fprintf(stderr, "operator type unsuppoerted by apply function\n");
            break;
    }

    return ret; 
}

struct Gen_type_t *eval(struct AST_Node *root, struct env_t *env){
    root->env = env;
    if(AST_Node_isleaf(root)){
        if(root->token.type == TOKEN_REGULAR){
            struct env_entry *bd_entry = lookup_env(env, root->token.tok);
            if(bd_entry == NULL){
                fprintf(stderr, "Symbol didn't bind with any value\n");
                return NULL;
            }
            
            struct Gen_type_t *selfeval;
            switch (bd_entry->type){
                case ENV_FUNC:
                    fprintf(stderr, "Currently not support evaluate a regular symbol into a function\n");
                    selfeval = NULL;
                    break;
                case ENV_GENVAL:
                    selfeval = make_integer(0xdeadbeef);
                    memcpy(selfeval, bd_entry->value.gen_val, sizeof(struct Gen_type_t));
                    break;
                default:
                    fprintf(stderr, "eval: should not be here\n");
                    selfeval = NULL;
                    break;
            }
            root->gen_val = selfeval;
        }else{
            root->gen_val = token2gen(&(root->token));
        }
        return root->gen_val;
    }else{
        struct Gen_type_t *args[64];
        struct Gen_type_t *eval_result;
        memset(args, 0, sizeof(void*) * 64);
       
        struct Gen_type_t *name[32];
        struct Gen_type_t *value[32];
        memset(name, 0, sizeof(void*) * 32);
        memset(value, 0, sizeof(void*) * 32);
        enum operation_type op_type = which_op(root->ops[0]->token.tok);
        if(op_type == OP_ENV){
            //apply ops on env operation
            enum env_op_type envop = which_envop(root->ops[0]->token.tok);
            switch (envop){
                case ENVOP_DEFINE:
                    name[0] = token2gen(&(root->ops[1]->token));
                    value[0] = eval(root->ops[2], env);
                    env_apply(envop, name, value, env);
                    destroy_gentype(name[0]);
                    break;
                case ENVOP_LET:
                    struct AST_Node *let_pairs = root->ops[1];
                    struct AST_Node *local_exp = root->ops[2];
                    struct Token *name_tok;
                    for(int i=0; let_pairs->ops[i]; i++){
                        name_tok = &(let_pairs->ops[i]->ops[0]->token);
                        name[i] = token2gen(name_tok);
                        value[i] = eval(let_pairs->ops[i]->ops[1], env);
                    }
                    env = env_apply(envop, name, value, env);
                    
                    let_pairs->isleaf = 1;

                    for(int i=0; name[i]; i++){
                        destroy_gentype(name[i]);
                    }

                    for(int i=0; i<64; i++){
                        if(local_exp->ops[i] != NULL){
                            args[i] = eval(local_exp->ops[i], env);
                            if(args[i] == NULL)
                                break;
                        }
                    }
                    break;
                case ENVOP_SET:
                    name[0] = token2gen(&(root->ops[1]->token));
                    value[0] = eval(root->ops[2], env);
                    env_apply(envop, name, value, env);
                    destroy_gentype(name[0]);
                    break;
                case ENVOP_UNDEFINED:
                    fprintf(stderr, "environment operation undefined\n");
                    break;
            }

        }else if(op_type == OP_LIST){
            //apply ops on list operation
        }else{
            //keep evaluate when it is UNDEFINED or ARITH
            for(int i=0; i<64; i++){
                if(root->ops[i] != NULL){
                    args[i] = eval(root->ops[i], env);
                    if(args[i] == NULL)
                    return NULL;
                }
            }
        }
        
        
        int root_isleaf = 1;

        if(!args[0] || args[0]->type != TYPE_OPERATOR){// if current node is unevaluable
            eval_result = make_list(args);
            root_isleaf = 0;
        }else{
            eval_result = apply(args[0], &(args[1]), env);
            if(eval_result == NULL){
                return NULL;
            }
        }
        
        for(int i=0; i<64; i++){
            if(root->ops[i] != NULL)
                root_isleaf &= root->ops[i]->isleaf;
        }

        root->gen_val = eval_result;
        root->isleaf = root_isleaf;
        return eval_result;
    }
    fprintf(stderr, "eval(): should not be there :(\n");
    return NULL;
}

struct AST_Node *READ(char *prompt){
    char *user_in;
    user_in = readline(prompt);

    if(user_in == NULL){
        return NULL;
    }

    struct AST_Node *AST_root = line_reader(user_in);
    return AST_root;
}

struct AST_Node *EVAL(struct AST_Node *root, struct env_t *env){
    struct Gen_type_t *eval_result = eval(root, env);
    if(eval_result == NULL)
        return NULL;
    else
        return root;
}

void PRINT(struct AST_Node *root){
    pr_str(root);
    printf("\n");
    return;
}

int main(void){

    char prompt[128];
    struct AST_Node *ast, *exp;

    snprintf(prompt, sizeof(prompt), "user> ");

    struct env_t *meta_env = init_env();
    meta_env->fa_env = meta_env;// mark it as global environment
    add_env_function(meta_env, "+", &mikal_add);
    add_env_function(meta_env, "-", &mikal_sub);
    add_env_function(meta_env, "*", &mikal_mul);
    add_env_function(meta_env, "/", &mikal_div);

    while(1){
        ast = READ(prompt);

        if(ast == NULL){
            break;
        }
        exp = EVAL(ast, meta_env);
        PRINT(exp);
        AST_destroy(ast);
    }
    destroy_env(meta_env);
}
