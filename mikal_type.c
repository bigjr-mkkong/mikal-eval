//#define SINGTEST_MIKAL_TYPE

#include "mikal_type.h"
#include "stdlib.h"
#include "string.h"
#include "env.h"
#include "stdio.h"
#include "errno.h"
#include "assert.h"

URet copy_mikal(mikal_t *src);

/*
 * build-in operations:
 * + - * /
 * def let set
 * car cdr cons
 * lambda
 */
int valid_mikal(mikal_t *addr){
    if(!addr || addr->magic != MIKAL_MAGIC)
        return 0;
    else
        return 1;
}

URet make_integer(long long x){
    URet retval;

    mikal_t *ret = (mikal_t*)malloc(sizeof(mikal_t));
    ret->integer = x;
    ret->type = MT_INTEGER;
    ret->magic = MIKAL_MAGIC;

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
    ret->sym = (char*)malloc(sizeof(name_len));
    strcpy(ret->sym, sym_name);
    ret->type = MT_SYMBOL;
    ret->magic = MIKAL_MAGIC;

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
    ret->str = (char*)malloc(sizeof(name_len + 8));
    
    if(name_len >= 2 && str_name[0] == '\"' && str_name[name_len-1] == '\"'){
        strcpy(ret->str, str_name);
    }else{
        ret->str[0] = '\"';
        strcpy(ret->str + 1, str_name);
        ret->str[name_len + 1] = '\"';
    }

    ret->type = MT_STRING;
    ret->magic = MIKAL_MAGIC;

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

    retval.addr = ret;
    retval.error_code = GOOD;
    return retval;
}

URet make_func(mikal_func func, enum mikal_op_type type){
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
    ret->magic = MIKAL_MAGIC;

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
    mik_clos->type = MT_CLOSURE;

    int cpidx;
    for(int cpidx=0; cpidx<MAX_PROCARGS; cpidx++){
        call_ret = copy_mikal(args[cpidx]);
        if(URet_state(call_ret) != GOOD){
            ret = call_ret;
            goto make_closure_Failed;
        }

        clos->args[cpidx] = URet_val(call_ret, mikal_t *);
    }

    clos->root = copy_ast(root);
    clos->env = env;

    ret.addr = mik_clos;
    ret.error_code = GOOD;

    return ret;

make_closure_Failed:
    for(int i=0; i<cpidx; i++)
        destroy_mikal(clos->args[i]);
    free(clos);
    free(mik_clos);
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

        case MT_CONS:
            fprintf(stdout, "(");
            print_mikal(target->car);
            fprintf(stdout, ".");
            print_mikal(target->cdr);
            fprintf(stdout, ")");
            break;

        case MT_FUNC:
            ret.error_code = E_INVAL_TYPE;
            break;

        case MT_AST:
            ret.error_code = E_CASE_UNIMPL;
            break;

        default:
            ret.error_code = E_CASE_UNIMPL;
            break; 
    }

    return ret;
}

URet destroy_mikal(mikal_t *target){
    URet retval;
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

        case MT_SYMBOL:
            free(target->sym);
            free(target);
            break;

        case MT_STRING:
            free(target->str);
            free(target);
            break;

        case MT_CONS:
            destroy_mikal(target->car);
            destroy_mikal(target->cdr);
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
            break;

        case MT_AST:
            retval.error_code = E_CASE_UNIMPL;
            break;

        default:
            retval.error_code = E_CASE_UNIMPL;
            break;
    }
    
    return retval;;
}

int mikal_cmp(mikal_t *val1, mikal_t *val2){
    int cmp_result;
    if(val1->type != val2->type){
        cmp_result = -1;
        return cmp_result;
    }else{
        switch(val1->type){
            case MT_INTEGER:
                cmp_result = (val1->integer == val2->integer);
                break;

            case MT_SYMBOL:
                cmp_result = !(strcmp(val1->sym, val2->sym));
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
    URet ret;
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
            dst->str = (char*)malloc(len);
            strcpy(dst->str, src->str);
            break;

        case MT_SYMBOL:
            len = strlen(src->sym);
            dst = malloc(sizeof(mikal_t));
            memcpy(dst, src, sizeof(mikal_t));
            dst->str = (char*)malloc(len);
            strcpy(dst->sym, src->sym);
            break; 

        case MT_INTEGER:
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


#ifdef SINGLE_FILE_TEST

int main(void){
    char num[] = "00001234a";
    URet uret_val = str2ll(num);
    long long conv_val = URet_val(uret_val, long long);

    assert(URet_state(uret_val) == GOOD);
    printf("\n%d\n", conv_val);
}

#endif

