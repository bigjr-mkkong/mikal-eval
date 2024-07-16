//#define SINGTEST_MIKAL_TYPE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <malloc.h>
#include "mikal_type.h"
#include "env.h"
#include "gc.h"

URet copy_mikal(mikal_t *src);
extern gc_buffer gcbuf;
/*
 * build-in operations:
 * + - * /
 * def let set
 * car cdr cons
 * lambda
 */
int valid_mikal(mikal_t *addr){
    if(!addr || addr->magic != MIKAL_MAGIC){
        return 0;
    }else{
        return 1;
    }
}

URet make_integer(long long x){
    URet retval;

    mikal_t *ret = (mikal_t*)malloc(sizeof(mikal_t));
    ret->integer = x;
    ret->type = MT_INTEGER;
    ret->magic = MIKAL_MAGIC;
    ret->refcnt = 1;

    retval.addr = ret;
    retval.error_code = GOOD;
    return retval;
}

URet make_symbol(char *sym_name){
    URet retval;

    if(!sym_name){
        retval.val = 0;
        retval.error_code = E_INVAL_ADDR;
        return retval;
    }

    int name_len = strlen(sym_name);
    if(name_len <= 0){
        retval.val = 0;
        retval.error_code = E_EMPTY;
        return retval;
    }
    
    mikal_t *ret = (mikal_t*)malloc(sizeof(mikal_t));
    ret->sym = (char*)calloc(1, ROUND_UP(sizeof(name_len)));
    strcpy(ret->sym, sym_name);
    ret->type = MT_SYMBOL;
    ret->magic = MIKAL_MAGIC;
    ret->refcnt = 1;

    retval.addr = ret;
    retval.error_code = GOOD;
    return retval;
}

URet make_string(char *str_name){
    URet retval;

    if(!str_name){
        retval.val = 0;
        retval.error_code = E_INVAL_ADDR;
        return retval;
    }

    int name_len = strlen(str_name);
    if(name_len <= 0){
        retval.val = 0;
        retval.error_code = E_EMPTY;
        return retval;
    }
    
    mikal_t *ret = (mikal_t*)malloc(sizeof(mikal_t));
    ret->str = (char*)calloc(1, ROUND_UP(sizeof(name_len + 8)));
    
    if(name_len >= 2 && str_name[0] == '\"' && str_name[name_len-1] == '\"'){
        strcpy(ret->str, str_name);
    }else{
        ret->str[0] = '\"';
        strcpy(ret->str + 1, str_name);
        ret->str[name_len + 1] = '\"';
    }

    ret->type = MT_STRING;
    ret->magic = MIKAL_MAGIC;
    ret->refcnt = 1;

    retval.addr = ret;
    retval.error_code = GOOD;
    return retval;
}

URet make_cons(mikal_t *car, mikal_t *cdr){
    URet retval;
    if(!valid_mikal(car)){
        retval.val = 0;
        retval.error_code = E_INVAL_TYPE;
        return retval;
    }

    if(!valid_mikal(cdr)){
        retval.val = 0;
        retval.error_code = E_INVAL_TYPE;
        return retval;
    }

    mikal_t *ret = (mikal_t*)malloc(sizeof(mikal_t));
    ret->car = car;
    ret->cdr = cdr;
    ret->type = MT_CONS;
    ret->magic = MIKAL_MAGIC;
    ret->refcnt = 1;

    car->refcnt++;
    cdr->refcnt++;

    retval.addr = ret;
    retval.error_code = GOOD;
    return retval;
}

URet make_ast(struct AST_Node *ast){
    URet retval;

    if(!ast){
        retval.val = 0;
        retval.error_code = E_INVAL_ADDR;
        return retval;
    }

    mikal_t *ret = (mikal_t*)malloc(sizeof(mikal_t));
    ret->ast = ast;
    ret->type = MT_AST;
    ret->magic = MIKAL_MAGIC;
    ret->refcnt = 1;

    retval.addr = ret;
    retval.error_code = GOOD;
    return retval;
}

URet make_func(mikal_func func, enum mikal_op_type type, enum func_return return_type){
    URet retval;

    if(!func){
        retval.val = 0;
        retval.error_code = E_INVAL_ADDR;
        return retval;
    }

    mikal_t *ret = (mikal_t*)malloc(sizeof(mikal_t));
    ret->func = func;
    ret->type = MT_FUNC;
    ret->op_type = type;
    ret->ret_type = return_type;
    ret->magic = MIKAL_MAGIC;
    ret->refcnt++;

    retval.addr = ret;
    retval.error_code = GOOD;
    return retval;
}

URet make_closure(mikal_t *args[], struct AST_Node *root, struct env_t *env){
    URet ret, call_ret;
    int argidx;
    for(int argidx=0; argidx<MAX_PROCARGS && args[argidx]; argidx++){
        if(!valid_mikal(args[argidx])){
            ret.val = 0;
            ret.error_code = E_INVAL_TYPE;
            return ret;
        }
    }

    mikal_t *mik_clos = malloc(sizeof(mikal_t));
    Closure *clos = (Closure*)malloc(sizeof(Closure));

    mik_clos->clos = clos;
    mik_clos->magic = MIKAL_MAGIC;
    mik_clos->type = MT_CLOSURE;
    mik_clos->op_type = OP_CLOSURE;
    mik_clos->refcnt = 1;

    memset(clos->args, 0, MAX_PROCARGS * sizeof(mikal_t *));
    int cpidx = 0;
    for(cpidx=0; cpidx<MAX_PROCARGS && args[cpidx]; cpidx++){
        call_ret = copy_mikal(args[cpidx]);
        if(URet_state(call_ret) != GOOD){
            ret = call_ret;
            goto make_closure_Failed;
        }

        clos->args[cpidx] = URet_val(call_ret, mikal_t *);
    }

    clos->root = copy_ast(root);
    clos->env = env;
    clos->env->ref_cnt++;

    ret.addr = mik_clos;
    ret.error_code = GOOD;

    return ret;

make_closure_Failed:
    for(int i=0; i<cpidx; i++)
        destroy_mikal(clos->args[i]);
    free(clos);
    free(mik_clos);
}

URet make_bool(enum mt_bool val){
    URet retval;

    mikal_t *ret = (mikal_t*)malloc(sizeof(mikal_t));
    ret->boolval = val;
    ret->type = MT_BOOL;
    ret->magic = MIKAL_MAGIC;
    ret->refcnt = 1;

    retval.addr = ret;
    retval.error_code = GOOD;
    return retval;
}

URet copy_clos(mikal_t *src){
    URet ret, call_ret;
    if(!src || !valid_mikal(src)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    mikal_t *new_mikal = (mikal_t*)malloc(sizeof(mikal_t));
    new_mikal->magic = MIKAL_MAGIC;
    new_mikal->type = MT_CLOSURE;
    new_mikal->op_type = OP_CLOSURE;
    new_mikal->refcnt = 1;

    new_mikal->clos = (Closure*)malloc(sizeof(Closure));
    memset(new_mikal->clos, 0, sizeof(Closure));
    Closure *new_clos = new_mikal->clos;
    Closure *old_clos = src->clos;

    new_mikal->clos = new_clos;
    
    new_clos->root = copy_ast(old_clos->root);
    new_clos->env = old_clos->env;
    new_clos->env->ref_cnt++;
    
    ret.addr = new_mikal;
    ret.error_code = GOOD;

    for(int cpidx = 0; cpidx < MAX_PROCARGS; cpidx++){
        if(!old_clos->args[cpidx]){
            break;
        }

        call_ret = copy_mikal(old_clos->args[cpidx]);
        if(URet_state(call_ret) != GOOD){
            ret = call_ret;
            goto copy_clos_Failed;
        }

        new_clos->args[cpidx] = URet_val(call_ret, mikal_t *);
    }

    return ret;

copy_clos_Failed:
    return call_ret;

}

void print_cons(mikal_t *node){
    if(!node) return;
    if(valid_mikal(node) && node->type != MT_CONS){
        print_mikal(node);
        return;
    }

    printf("(");
    print_cons(node->car);
    printf(" . ");
    print_cons(node->cdr);
    printf(")");

    return;
}

void destroy_cons(mikal_t *node){
    if(!node) return;
    if(valid_mikal(node) && node->type != MT_CONS){
        destroy_mikal(node);
        return;
    }

    destroy_cons(node->car);
    destroy_cons(node->cdr);

    free(node);

    return;
}

URet copy_cons(mikal_t *node){
    URet call_ret, ret;
    if(!node){
        call_ret.val = 0;
        call_ret.error_code = E_INVAL_ADDR;
        return call_ret;
    }
    if(valid_mikal(node) && node->type != MT_CONS){
        return copy_mikal(node);
    }

    mikal_t *copied_car, *copied_cdr, *new_cons;
    call_ret = copy_cons(node->car);
    if(URet_state(call_ret) != GOOD)
        goto copy_cons_failed;

    copied_car = URet_val(call_ret, mikal_t*);

    call_ret = copy_cons(node->cdr);
    if(URet_state(call_ret) != GOOD)
        goto copy_cons_failed;

    copied_cdr = URet_val(call_ret, mikal_t*);

    call_ret = make_cons(copied_car, copied_cdr);
    if(URet_state(call_ret) != GOOD)
        goto copy_cons_failed;

    new_cons = URet_val(call_ret, mikal_t*);
    ret.addr = new_cons;
    ret.error_code = GOOD;

    return ret;

copy_cons_failed:
    return call_ret;

}

URet print_mikal(mikal_t *target){
    URet ret;
    if(!valid_mikal(target)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    ret.val = 0;
    ret.error_code = GOOD;

    switch (target->type){
        case MT_INTEGER:
            fprintf(stdout, "%lld", target->integer);
            break;

        case MT_SYMBOL:
            fprintf(stdout, "%s", target->sym);
            break;

        case MT_STRING:
            fprintf(stdout, "%s", target->str);
            break;

        case MT_FUNC:
            ret.error_code = E_INVAL_TYPE;
            break;

        case MT_AST:
            ret.error_code = E_CASE_UNIMPL;
            break;

        case MT_CLOSURE:
            fprintf(stdout, "This is a clusure\n");
            break;

        case MT_CONS:
            print_cons(target);
            break;

        case MT_BOOL:
            fprintf(stdout, "%s\n", (target->boolval == BOOL_FALSE)? \
                        "FALSE":"TRUE");
            break;

        default:
            ret.error_code = E_CASE_UNIMPL;
            break; 
    }

    return ret;
}

URet destroy_mikal(mikal_t *target){
    URet retval;
    if(target == NULL){
        retval.val = 0;
        retval.error_code = GOOD;
        return retval;
    }

    if(!valid_mikal(target)){
        retval.val = 0;
        retval.error_code = E_INVAL_TYPE;
        return retval;
    }

    retval.val = 0;
    retval.error_code = GOOD;
    switch(target->type){
        case MT_INTEGER:
            free(target);
            break;

        case MT_BOOL:
            free(target);
            break;

        case MT_SYMBOL:
            free(target->sym);
            free(target);
            break;

        case MT_STRING:
            free(target->str);
            free(target);
            break;

        case MT_FUNC:
            free(target);
            break;

        case MT_CLOSURE:
            Closure *tmp_clos;
            tmp_clos = target->clos;
            for(int i=0; i<MAX_PROCARGS && tmp_clos->args[i]; i++){
                destroy_mikal(tmp_clos->args[i]);
            }

            AST_destroy(tmp_clos->root);
            tmp_clos->env->ref_cnt--;
            if(tmp_clos->env->ref_cnt <= 1){
                destroy_env(tmp_clos->env);
            }
            
            free(target->clos);
            free(target);
            break;

        case MT_AST:
            retval.error_code = E_CASE_UNIMPL;
            break;

        case MT_CONS:
            destroy_cons(target);
            break;

        default:
            retval.error_code = E_CASE_UNIMPL;
            break;
    }
    
    return retval;
}

/*
 * Currently only support integer and string
 */
int mikal_cmp(mikal_t *val1, mikal_t *val2, struct env_t *env){
    int cmp_result;
    URet call_ret;
    mikal_t *tmp1, *tmp2;
    if(val1->type != val2->type){
        cmp_result = 0;
        return cmp_result;
    }else{
        switch(val1->type){
            case MT_INTEGER:
                cmp_result = (val1->integer == val2->integer);
                break;

            case MT_SYMBOL:
                if(!env){
                    cmp_result = 0;
                    break;
                }else{
                    call_ret = lookup_env(env, val1->sym);
                    struct env_entry *ent1 = URet_val(call_ret, struct env_entry*);
                    tmp1 = ent1->value;

                    call_ret = lookup_env(env, val2->sym);
                    struct env_entry *ent2 = URet_val(call_ret, struct env_entry*);
                    tmp2 = ent2->value;

                    cmp_result = mikal_cmp(tmp1, tmp2, env);
                    break;
                }

            case MT_STRING:
                cmp_result = !(strcmp(val1->str, val2->str));
                break;

            default:
                cmp_result = -1;
                break;
        }
    }

    return cmp_result;
}

int is_regchar(char x){
    return (x>='a' && x <= 'z') || (x>='A' && x <= 'Z');
}

URet str2ll(char *str){
    URet ret;
    int len = strlen(str);
    char *endpt = (char*)(str + len);
    char *refpt = endpt;
    
    long long num = strtoll(str, &endpt, 10);
    
    if(errno == ERANGE){
        ret.val = 0;
        ret.error_code = E_OUTOFBOUND;
    }else if(endpt != refpt){
        ret.val = 0;
        ret.error_code = E_FAILED;
    }else{
        ret.val = num;
        ret.error_code = GOOD;
    }

    return ret;
}

#ifndef SINGTEST_MIKAL_TYPE
enum mikal_types which_mktype(char *str, struct env_t *env){
    int len = strlen(str);

    if(len <= 0) return MT_NONE;

    if(str[0] == '\"' && str[len-1] == '\"' && len >= 2){
        return MT_STRING;
    }

    URet env_q = lookup_env(env, str);
    if(URet_state(env_q) == GOOD){
        return MT_SYMBOL;
    }

    int is_int = 1;
    for(int i=0; i<len; i++){
        is_int &= (str[i] >= '0' && str[i] <= '9');
    }
    
    if(is_int){
        return MT_INTEGER;
    }else{
        if(!is_regchar(str[0]))
            return MT_NONE;
        else
            return MT_UNBOND_SYM;
    }
}

#endif


URet copy_mikal(mikal_t *src){
    URet ret, call_ret;
    if(!valid_mikal(src)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    mikal_t *dst;
    int len;

    switch(src->type){
        case MT_STRING:
            len = strlen(src->str);
            dst = malloc(sizeof(mikal_t));
            memcpy(dst, src, sizeof(mikal_t));
            dst->str = (char*)calloc(1, ROUND_UP(len));
            strcpy(dst->str, src->str);
            break;

        case MT_SYMBOL:
            len = strlen(src->sym);
            dst = malloc(sizeof(mikal_t));
            memcpy(dst, src, sizeof(mikal_t));
            dst->str = (char*)calloc(1, ROUND_UP(len));
            strcpy(dst->sym, src->sym);
            break; 

        case MT_INTEGER:
            dst = malloc(sizeof(mikal_t));
            memcpy(dst, src, sizeof(mikal_t));
            break;

        case MT_FUNC:
            dst = malloc(sizeof(mikal_t));
            memcpy(dst, src, sizeof(mikal_t));
            break;

        case MT_CONS:
            call_ret = copy_cons(src);
            dst = URet_val(call_ret, mikal_t*);
            break;

        case MT_CLOSURE:
            call_ret = copy_clos(src);
            dst = URet_val(call_ret, mikal_t*);
            break;

        case MT_BOOL:
            dst = malloc(sizeof(mikal_t));
            memcpy(dst, src, sizeof(mikal_t));
            break;

        default:
            ret.addr = 0;
            ret.error_code = E_CASE_UNIMPL;
            return ret;
    }

    ret.addr = dst;
    ret.error_code = GOOD;

    return ret;
}

URet move_mikal(mikal_t *dst, mikal_t *src){
    URet ret, call_ret;
    if(!valid_mikal(src)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    if(malloc_usable_size(dst) < sizeof(mikal_t)){
        ret.val = 0;
        ret.error_code = E_OUTOFBOUND;
        return ret;
    }
    call_ret = copy_mikal(src);
    if(URet_state(call_ret) != GOOD)
        return call_ret;
        
    mikal_t *copied = URet_val(call_ret, mikal_t*);
    memcpy(dst, copied, sizeof(mikal_t));
    free(copied);

    ret.error_code = GOOD;
    ret.val = 0;
    return ret;

}


#ifdef SINGTEST_MIKAL_TYPE

int main(void){
    URet call_ret;
    mikal_t *a, *b, *cons, *cons2, *full_cons;
    call_ret = make_integer(3);
    a = URet_val(call_ret, mikal_t*);

    call_ret = make_integer(4);
    b = URet_val(call_ret, mikal_t*);

    call_ret = make_cons(a, b);
    cons = URet_val(call_ret, mikal_t*);

    call_ret = copy_mikal(cons);
    cons2 = URet_val(call_ret, mikal_t*);

    call_ret = make_cons(cons, cons2);
    full_cons = URet_val(call_ret, mikal_t*);

    print_mikal(full_cons);
    destroy_mikal(full_cons);
}

#endif

