#include <stdio.h>
#include <stdarg.h>
#include "unistd.h"
#include "mikal_type.h"
#include "eval.h"
#include "gc.h"

extern struct env_t *meta_env;

URet add_mikal(mikal_t **args, ...){
    URet ret;
    long long sum = 0;
    mikal_t *adder = args[0];
    for(int i=0; adder; i++, adder = args[i]){
        if(!valid_mikal(adder) || adder->type != MT_INTEGER){
            ret.error_code = E_INVAL_TYPE;
            ret.val = 0;
            return ret;
        }
        sum += adder->integer;
    }

    mikal_t *ans = URet_val(make_integer(sum) ,mikal_t*);
    ret.addr = ans;
    ret.error_code = GOOD;
    return ret;
}

URet sub_mikal(mikal_t **args, ...){
    URet ret;
    mikal_t *suber;
    long long result;
    for(int i=0; args[i]; i++){
        suber = args[i];
        if(!valid_mikal(suber) || suber->type != MT_INTEGER){
            ret.error_code = E_INVAL_TYPE;
            ret.val = 0;
            return ret;
        }
        if(i == 0)
            result = args[i]->integer;
        else
            result -= suber->integer;
    }

    mikal_t *ans = URet_val(make_integer(result) ,mikal_t*);
    ret.addr = ans;
    ret.error_code = GOOD;
    return ret;
}

URet mul_mikal(mikal_t **args, ...){
    URet ret;
    long long sum = 1;
    mikal_t *multer = args[0];
    for(int i=0; multer; i++, multer = args[i]){
        if(!valid_mikal(multer) || multer->type != MT_INTEGER){
            ret.error_code = E_INVAL_TYPE;
            ret.val = 0;
            return ret;
        }
        sum *= multer->integer;
    }

    mikal_t *ans = URet_val(make_integer(sum) ,mikal_t*);
    ret.addr = ans;
    ret.error_code = GOOD;
    return ret;
}

URet div_mikal(mikal_t **args, ...){
    URet ret;
    mikal_t *div1 = args[0];
    mikal_t *div2 = args[1];

    if(!valid_mikal(div1)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    if(!valid_mikal(div2)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    if(div2->integer == 0){
        ret.val = 0;
        ret.error_code = E_ARITH_ERROR;
        return ret;
    }
    long long result;

    result = div1->integer / div2->integer;
    
    mikal_t *ans = URet_val(make_integer(result) ,mikal_t*);
    ret.addr = ans;
    ret.error_code = GOOD;
    return ret;
}

/*
 * Required format:
 * URet make_closure(mikal_t *args[], struct AST_Node *root, struct env_t *env)
 * args is the arguments for new closure
 * root is the ast root for new 'sub-rutine'
 * env is the environment
 * all arguments does NOT need to pass as copy
 */
URet make_clos_mikal(mikal_t **args, ...){
    va_list vvar;
    va_start(vvar, args);

    struct AST_Node *root = va_arg(vvar, struct AST_Node*);
    struct env_t *env = va_arg(vvar, struct env_t*);

    va_end(vvar);

    URet ret;
    
    ret = make_closure(args, root, env);
    
    return ret;
}

URet car_mikal(mikal_t **args, ...){
    URet ret;
    mikal_t *src = args[0];
    if(!src){
        ret.val = 0;
        ret.error_code = E_INVAL_ADDR;
        return ret;
    }

    if(!valid_mikal(src)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    mikal_t *car_val = src->car;

    ret.addr = car_val;
    ret.error_code = GOOD;

    return ret;
}


URet cdr_mikal(mikal_t **args, ...){
    URet ret;
    mikal_t *src = args[0];
    if(!src){
        ret.val = 0;
        ret.error_code = E_INVAL_ADDR;
        return ret;
    }

    if(!valid_mikal(src)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    mikal_t *cdr_val = src->cdr;

    ret.addr = cdr_val;
    ret.error_code = GOOD;

    return ret;
}

URet cons_mikal(mikal_t **args, ...){
    URet call_ret;
    mikal_t *car = args[0];
    mikal_t *cdr = args[1];

    if(!car || !valid_mikal(car)){
        call_ret.val = 0;
        call_ret.error_code = GOOD;
        return call_ret;
    }
    
    if(!cdr || !valid_mikal(cdr)){
        call_ret.val = 0;
        call_ret.error_code = GOOD;
        return call_ret;
    }

    call_ret = make_cons(car, cdr);

    return call_ret;
}

//URet def_mikal(mikal_t *args, struct env_t *env)
URet def_mikal(mikal_t **args, ...){
    URet ret, call_ret;

    mikal_t *sym = args[0];
    mikal_t *val = args[1];
    mikal_t *new_sym, *new_val;
    va_list vvar;
    va_start(vvar, args);

    struct env_t *env = va_arg(vvar, struct env_t*);
    va_end(vvar);

    struct env_entry *entry;

    call_ret = lookup_env(env, sym->sym);
    if(URet_state(call_ret) == E_NOTFOUND){

        /* call_ret = copy_mikal(sym); */
        /* new_sym = URet_val(call_ret, mikal_t*); */

        /* call_ret = copy_mikal(val); */
        /* new_val = URet_val(call_ret, mikal_t*); */

        add_env_entry(env, sym, val);

    }else{
        entry = URet_val(call_ret, struct env_entry*);
        add_gc_mikal(entry->value);

        call_ret = copy_mikal(val);

        entry->value = URet_val(call_ret, mikal_t*);
    }

    ret.val = 0;
    ret.error_code = GOOD;

    return ret;
}

//URet set_mikal(mikal_t *args, struct env_t *env)
URet set_mikal(mikal_t **args, ...){
    URet ret, call_ret;

    mikal_t *sym = args[0];
    mikal_t *val = args[1];
    mikal_t *new_sym, *new_val;
    va_list vvar;
    va_start(vvar, args);

    struct env_t *env = va_arg(vvar, struct env_t*);
    va_end(vvar);

    struct env_entry *entry;

    call_ret = lookup_env(env, sym->sym);
    if(URet_state(call_ret) == E_NOTFOUND){
        ret.val = 0;
        ret.error_code = E_FAILED;
        
        return ret;
    }else{
        entry = URet_val(call_ret, struct env_entry*);

        add_gc_mikal(entry->value);
        call_ret = copy_mikal(val);

        entry->value = URet_val(call_ret, mikal_t*);
    }

    ret.val = 0;
    ret.error_code = GOOD;

    return ret;
}

URet let_mikal(mikal_t **args, ...){
    struct AST_Node *exp;
    struct env_t *env;

    va_list vvar;
    va_start(vvar, args);
    exp = va_arg(vvar, struct AST_Node*);
    env = va_arg(vvar, struct env_t*);
    va_end(vvar);

    URet ret;
    ret = eval(exp, env);

    return ret;
}

URet beq_mikal(mikal_t **args, ...){
    mikal_t *l_val = args[0];
    mikal_t *r_val = args[1];
    
    mikal_t *cmp_result;
    URet ret;

    if(!valid_mikal(l_val) || !valid_mikal(r_val)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    int tmp = mikal_cmp(l_val, r_val, 0);
    
    cmp_result = URet_val(make_bool((tmp == 0) ? BOOL_FALSE : BOOL_TRUE ), mikal_t *);

    ret.addr = cmp_result;
    ret.error_code = GOOD;

    return ret;
}

URet blt_mikal(mikal_t **args, ...){
    mikal_t *l_val = args[0];
    mikal_t *r_val = args[1];
    
    mikal_t *cmp_result;
    URet ret;

    if(!valid_mikal(l_val) || !valid_mikal(r_val)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }else if(l_val->type != MT_INTEGER || r_val->type != MT_INTEGER){
        ret.val = 0;
        ret.error_code = E_CASE_UNIMPL;
        return ret;
    }

    cmp_result = URet_val(make_bool((l_val->integer < r_val->integer) ? BOOL_TRUE : BOOL_FALSE )\
            , mikal_t *);

    ret.addr = cmp_result;
    ret.error_code = GOOD;

    return ret;
}

URet bgt_mikal(mikal_t **args, ...){
    mikal_t *l_val = args[0];
    mikal_t *r_val = args[1];
    
    mikal_t *cmp_result;
    URet ret;

    if(!valid_mikal(l_val) || !valid_mikal(r_val)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }else if(l_val->type != MT_INTEGER || r_val->type != MT_INTEGER){
        ret.val = 0;
        ret.error_code = E_CASE_UNIMPL;
        return ret;
    }

    cmp_result = URet_val(make_bool((l_val->integer > r_val->integer) ? BOOL_TRUE : BOOL_FALSE )\
            , mikal_t *);

    ret.addr = cmp_result;
    ret.error_code = GOOD;

    return ret;
}

URet if_mikal(mikal_t **args, ...){
    URet ret;
    fprintf(stdout, "placeholder function for if, I will do nothing\n");
    ret.val = 0;
    ret.error_code = GOOD;

    return ret;
}

URet remainder_mikal(mikal_t **args, ...){
    URet ret;
    
    mikal_t *arg1 = args[0];
    mikal_t *arg2 = args[1];

    if(!valid_mikal(arg1) || !valid_mikal(arg2)){
        ret.val = 0;
        ret.error_code = E_INVAL_ADDR;
        return ret;
    }

    if(arg1->type != MT_INTEGER || arg2->type != MT_INTEGER){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    if(arg2->integer == 0){
        ret.val = 0;
        ret.error_code = E_ARITH_ERROR;
        return ret;
    }

    int rem = arg1->integer % arg2->integer;

    mikal_t *rem_result = URet_val(make_integer(rem), mikal_t *);
    ret.addr = rem_result;
    ret.error_code = GOOD;

    return ret;
}

URet assert_mikal(mikal_t **args, ...){
    URet ret;

    mikal_t *ret_mikal = URet_val(make_bool(BOOL_TRUE), mikal_t *);

    mikal_t *arg1 = args[0];
    
    if(arg1->type != MT_BOOL){
        ret.addr = arg1;
        ret.error_code = GOOD;
    }else if(arg1->boolval == BOOL_FALSE){
        fprintf(stderr, "assertion failed\n");
        _exit(0);
    }else{
        ret.addr = arg1;
        ret.error_code = GOOD;
    }

    return ret;

}

URet help_mikal(mikal_t **args, ...){
    URet ret;
    struct env_entry *env_ent;

    fprintf(stdout, "Supported operations:\n");

    for(int i=0; i<64; i++){
        env_ent = meta_env->symmap[i];
        if(env_ent == NULL){
            continue;
        }

        fprintf(stdout, "%s\n", env_ent->symbol->sym);
    }

    ret.error_code = GOOD;

    return ret;
}


URet not_mikal(mikal_t **args, ...){
    URet ret;

    mikal_t *ret_mikal = URet_val(make_bool(BOOL_FALSE), mikal_t *);

    mikal_t *arg1 = args[0];
    
    if(arg1->type != MT_BOOL){
        ret.addr = ret_mikal;
        ret.error_code = GOOD;
    }else{
        if(arg1->boolval == BOOL_TRUE){
            ret_mikal->boolval = BOOL_FALSE;
        }else{
            ret_mikal->boolval = BOOL_TRUE;
        }

        ret.addr = ret_mikal;
        ret.error_code = GOOD;
    }

    return ret;

}
