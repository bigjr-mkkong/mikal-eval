#include "reader.h"
#include "env.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "gc.h"
URet eval(struct AST_Node *root, struct env_t *env);

int is_leafnode(struct AST_Node *node){
    for(int i=0; i<MAX_CHILD; i++){
        if(node->ops[i]){
            return 0;
        }
    }

    return 1;
}


URet apply(mikal_t *op, struct AST_Node *root, struct env_t *env){
    URet ret, call_ret;
    int eval_idx = 1;
    int free_idx = 0;
    mikal_t *subexp[MAX_CHILD];

    memset(subexp, 0, sizeof(mikal_t*) * MAX_CHILD);

    switch(op->type){
        case MT_FUNC:
            if(op->op_type == OP_ARITH || op->op_type == OP_CONS){
                while((eval_idx < MAX_CHILD) && root->ops[eval_idx]){
                    ret = eval(root->ops[eval_idx], env);
                    if(URet_state(ret) != GOOD)
                        goto apply_Failed;
                    
                    subexp[eval_idx-1] = URet_val(ret, mikal_t*);
                    eval_idx++;
                }

                call_ret = op->func(subexp);
                if(URet_state(call_ret) != GOOD)
                    goto apply_Failed;

                call_ret.addr = URet_val(call_ret, mikal_t *);
                call_ret.error_code = GOOD;

                for(; subexp[free_idx] && free_idx < MAX_CHILD; free_idx++)
                    add_gc_mikal(subexp[free_idx]);
            }else if(op->op_type == OP_LAMBDA){
                //assemble args string into mikal symbol
                struct AST_Node *arg_node = root->ops[1];
                for(int argidx = 0; argidx < MAX_PROCARGS && arg_node->ops[argidx]; argidx++){
                    ret = eval(arg_node->ops[argidx], env);
                    if(URet_state(ret) != GOOD)
                        goto apply_Failed;

                    subexp[argidx] = URet_val(ret, mikal_t*);
                }
                call_ret = op->func(subexp, root->ops[2], env);
                if(URet_state(call_ret) != GOOD)
                    goto apply_Failed;
                for(; subexp[free_idx] && free_idx < MAX_CHILD; free_idx++)
                    add_gc_mikal(subexp[free_idx]);
            }
            add_gc_mikal(op);
            break;
        case MT_CLOSURE:
            struct env_t *new_env = URet_val(init_env(), struct env_t*);
            new_env->fa_env = env;
            struct closure *clos = op->clos;
            int val_idx = 1;
            int sym_idx = 0;
            for(; val_idx<MAX_PROCARGS && root->ops[val_idx]; val_idx++){
                ret = eval(root->ops[val_idx], env);
                if(URet_state(ret) != GOOD)
                    goto apply_Failed;

                subexp[val_idx-1] = URet_val(ret, mikal_t*);
            }

            for(; sym_idx<MAX_PROCARGS && clos->args[sym_idx]; sym_idx++){
                add_env_entry(new_env, clos->args[sym_idx], subexp[sym_idx]);
            }
            
            call_ret = eval(clos->root, new_env);
            
            destroy_env(new_env);
            break;

        default:
           call_ret.val = 0;
           call_ret.error_code = E_CASE_UNIMPL;
           break;
    }

    return call_ret;

apply_Failed:
    call_ret.val = 0;
    call_ret.error_code = E_FAILED;
    return call_ret;

}

URet self_eval(char *tokstr, struct env_t *env){
    URet ret, call_ret;
    enum mikal_types m_type;
    mikal_t *val;

    m_type = which_mktype(tokstr, env);
    if(m_type == MT_NONE){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }
    switch (m_type){
        case MT_INTEGER:
            call_ret = str2ll(tokstr);
            if(URet_state(call_ret) != GOOD) 
                goto selfeval_Failed;
            
            call_ret = make_integer(URet_val(call_ret, long long));
            if(URet_state(call_ret) != GOOD)
                goto selfeval_Failed;

            ret.val = URet_val(call_ret, long long);
            ret.error_code = GOOD;
            break;
 
        case MT_STRING:
            call_ret = make_string(tokstr);
            if(URet_state(call_ret) != GOOD)
                goto selfeval_Failed;

            ret = call_ret;
            break;

        case MT_SYMBOL:
            mikal_t *val;
            call_ret = lookup_env(env, tokstr);
            if(URet_state(call_ret) == GOOD){
                val = URet_val(call_ret, struct env_entry*)->value;

                call_ret = copy_mikal(val);
                if(URet_state(call_ret) != GOOD)
                    goto selfeval_Failed;

                ret.addr = URet_val(call_ret, mikal_t*);
                ret.error_code = GOOD;
                return ret;
            }else if(URet_state(call_ret) == E_NOTFOUND){
                call_ret = make_symbol(tokstr);
                if(URet_state(call_ret) != GOOD){
                    free(val);
                    goto selfeval_Failed;
                }

                ret = call_ret;
            }else{
                goto selfeval_Failed;
            }
            break; 

        case MT_UNBOND_SYM:
            ret.addr = URet_val(make_symbol(tokstr), void *);
            ret.error_code = GOOD;
            break;

        default:
            ret.val = 0;
            ret.error_code = E_CASE_UNIMPL;
            break;
    }

    return ret;

selfeval_Failed:
    ret = call_ret;
    return ret;
}

URet eval(struct AST_Node *root, struct env_t *env){
    URet ret;
    mikal_t *operation;

    if(is_leafnode(root)){
        char *tokstr = root->token.tok;
        ret = self_eval(tokstr, env);
        return ret;
    }

    ret = eval(root->ops[0], env);
    if(URet_state(ret) != GOOD)
        goto eval_Failed;

    operation = URet_val(ret, mikal_t*);

    if(operation->type != MT_FUNC && operation->type != MT_CLOSURE){
        ret.val = 0;
        ret.error_code = E_FAILED;
        goto eval_Failed;
    }
    
    int eval_idx = 1;
    if(operation->op_type != OP_ENV){
        ret = apply(operation, root, env);
        if(URet_state(ret) != GOOD)
            goto eval_Failed;
    }else{
        printf("%s is not an arithmetic operation\n", operation->op);
    }

    return ret;
eval_Failed:
    return ret;

}
