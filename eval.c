#include "reader.h"
#include "env.h"
#include "stdio.h"
#include "string.h"

int is_leafnode(struct AST_Node *node){
    for(int i=0; i<MAX_CHILD; i++){
        if(node->ops[i]){
            return 0;
        }
    }

    return 1;
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
            call_ret = lookup_env(env, tokstr);
            if(URet_state(call_ret) == GOOD){
                ret = call_ret;
            }else if(URet_state(call_ret) == E_NOTFOUND){
                call_ret = make_symbol(tokstr);
                if(URet_state(call_ret) != GOOD)
                    goto selfeval_Failed;

                ret = call_ret;
            }else{
                goto selfeval_Failed;
            }
            break; 

        case MT_OPERATOR:
            call_ret = lookup_env(env, tokstr);
            if(URet_state(call_ret) != GOOD)
                goto selfeval_Failed;

            struct env_entry *ent = URet_val(call_ret, struct env_entry*);
            ret.addr = &(ent->value);
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
    mikal_t *subexp[MAX_CHILD];
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

    if(operation->type != MT_FUNC)
        goto eval_Failed;
    
    memset(subexp, 0, sizeof(mikal_t*) * MAX_CHILD);
    int eval_idx = 1;
    if(operation->op_type == OP_ARITH){
        while((eval_idx < MAX_CHILD) && root->ops[eval_idx]){
            ret = eval(root->ops[eval_idx], env);
            if(URet_state(ret) != GOOD)
                goto eval_Failed;
            
            subexp[eval_idx-1] = URet_val(ret, mikal_t*);
            eval_idx++;
        }

        ret = operation->func(subexp);

        if(URet_state(ret) != GOOD)
            goto eval_Failed;
    }else{
        printf("%s is not an arithmetic operation\n", operation->op);
    }


    return ret;

eval_Failed:
    return ret;
}
