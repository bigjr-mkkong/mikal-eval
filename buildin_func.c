#include <stdio.h>
#include <stdarg.h>
#include "mikal_type.h"
#include "eval.h"
#include "gc.h"
#include "helpers.h"

URet add_mikal(mikal_t **args, ...){
    URet ret;
    long sum;
    int i;
    mikal_t *adder, *ans;

    adder = args[0];
    sum = 0;
    for(i=0; adder; i++, adder = args[i]){
        if(!valid_mikal(adder) || adder->type != MT_INTEGER){
            ret.error_code = E_INVAL_TYPE;
            ret.ret_union.val = 0;
            return ret;
        }
        sum = sum + adder->mk_data.integer;
    }

    ans = URet_val(make_integer(sum) ,mikal_t*);
    ret.ret_union.addr = ans;
    ret.error_code = GOOD;
    return ret;
}

URet sub_mikal(mikal_t **args, ...){
    URet ret;
    mikal_t *suber, *ans;
    long result;
    int i;

    for(i=0; args[i]; i++){
        suber = args[i];
        if(!valid_mikal(suber) || suber->type != MT_INTEGER){
            ret.error_code = E_INVAL_TYPE;
            ret.ret_union.val = 0;
            return ret;
        }
        if(i == 0)
            result = args[i]->mk_data.integer;
        else
            result -= suber->mk_data.integer;
    }

    ans = URet_val(make_integer(result) ,mikal_t*);
    ret.ret_union.addr = ans;
    ret.error_code = GOOD;
    return ret;
}

URet mul_mikal(mikal_t **args, ...){
    URet ret;
    long sum;
    mikal_t *multer, *ans;
    int i;

    multer = args[0];
    sum = 1;
    for(i=0; multer; i++, multer = args[i]){
        if(!valid_mikal(multer) || multer->type != MT_INTEGER){
            ret.error_code = E_INVAL_TYPE;
            ret.ret_union.val = 0;
            return ret;
        }
        sum *= multer->mk_data.integer;
    }

    ans = URet_val(make_integer(sum) ,mikal_t*);
    ret.ret_union.addr = ans;
    ret.error_code = GOOD;
    return ret;
}

URet div_mikal(mikal_t **args, ...){
    URet ret;
    mikal_t *div1;
    mikal_t *div2, *ans;
    long result;

    div1 = args[0];
    div2 = args[1];

    if(!valid_mikal(div1)){
        ret.ret_union.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    if(!valid_mikal(div2)){
        ret.ret_union.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    if(div2->mk_data.integer == 0){
        ret.ret_union.val = 0;
        ret.error_code = E_ARITH_ERROR;
        return ret;
    }

    result = div1->mk_data.integer / div2->mk_data.integer;
    
    ans = URet_val(make_integer(result) ,mikal_t*);

    ret.ret_union.addr = ans;
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
    URet ret;
    struct AST_Node *root;
    struct env_t *env;
    va_list vvar;
    va_start(vvar, args);

    root = va_arg(vvar, struct AST_Node*);
    env = va_arg(vvar, struct env_t*);

    va_end(vvar);
    
    ret = make_closure(args, root, env);
    
    return ret;
}

URet car_mikal(mikal_t **args, ...){
    URet ret;
    mikal_t *src, *car_val;

    src = args[0];

    if(!src){
        ret.ret_union.val = 0;
        ret.error_code = E_INVAL_ADDR;
        return ret;
    }

    if(!valid_mikal(src)){
        ret.ret_union.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    car_val = src->car;

    ret.ret_union.addr = car_val;
    ret.error_code = GOOD;

    return ret;
}


URet cdr_mikal(mikal_t **args, ...){
    URet ret;
    mikal_t *src, *cdr_val;

    src = args[0];
    if(!src){
        ret.ret_union.val = 0;
        ret.error_code = E_INVAL_ADDR;
        return ret;
    }

    if(!valid_mikal(src)){
        ret.ret_union.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    cdr_val = src->cdr;

    ret.ret_union.addr = cdr_val;
    ret.error_code = GOOD;

    return ret;
}

URet cons_mikal(mikal_t **args, ...){
    URet call_ret;
    mikal_t *car;
    mikal_t *cdr;

    car = args[0];
    cdr = args[1];

    if(!car || !valid_mikal(car)){
        call_ret.ret_union.val = 0;
        call_ret.error_code = GOOD;
        return call_ret;
    }
    
    if(!cdr || !valid_mikal(cdr)){
        call_ret.ret_union.val = 0;
        call_ret.error_code = GOOD;
        return call_ret;
    }

    call_ret = make_cons(car, cdr);

    return call_ret;
}

//URet def_mikal(mikal_t *args, struct env_t *env)
URet def_mikal(mikal_t **args, ...){
    URet ret, call_ret;

    mikal_t *sym;
    mikal_t *val;
    mikal_t *new_sym, *new_val;
    struct env_t *env;
    struct env_entry *entry;
    va_list vvar;

    va_start(vvar, args);
    env = va_arg(vvar, struct env_t*);
    va_end(vvar);


    sym = args[0];
    val = args[1];
    call_ret = lookup_env(env, sym->mk_data.sym);
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

    ret.ret_union.val = 0;
    ret.error_code = GOOD;

    return ret;
}

//URet set_mikal(mikal_t *args, struct env_t *env)
URet set_mikal(mikal_t **args, ...){
    URet ret, call_ret;

    mikal_t *sym;
    mikal_t *val;
    mikal_t *new_sym, *new_val;
    struct env_t *env;
    struct env_entry *entry;
    va_list vvar;

    va_start(vvar, args);
    env = va_arg(vvar, struct env_t*);
    va_end(vvar);

    sym = args[0];
    val = args[1];
    call_ret = lookup_env(env, sym->mk_data.sym);
    if(URet_state(call_ret) == E_NOTFOUND){
        ret.ret_union.val = 0;
        ret.error_code = E_FAILED;
        
        return ret;
    }else{
        entry = URet_val(call_ret, struct env_entry*);

        add_gc_mikal(entry->value);
        call_ret = copy_mikal(val);

        entry->value = URet_val(call_ret, mikal_t*);
    }

    ret.ret_union.val = 0;
    ret.error_code = GOOD;

    return ret;
}

URet let_mikal(mikal_t **args, ...){
    struct AST_Node *exp;
    struct env_t *env;
    URet ret;

    va_list vvar;
    va_start(vvar, args);
    exp = va_arg(vvar, struct AST_Node*);
    env = va_arg(vvar, struct env_t*);
    va_end(vvar);

    ret = eval(exp, env);

    return ret;
}

URet beq_mikal(mikal_t **args, ...){
    mikal_t *l_val;
    mikal_t *r_val;
    
    mikal_t *cmp_result;
    URet ret;

    int tmp;

    l_val = args[0];
    r_val = args[1];
    if(!valid_mikal(l_val) || !valid_mikal(r_val)){
        ret.ret_union.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    tmp = mikal_cmp(l_val, r_val, 0);
    
    cmp_result = URet_val(make_bool((tmp == 0) ? BOOL_FALSE : BOOL_TRUE ), mikal_t *);

    ret.ret_union.addr = cmp_result;
    ret.error_code = GOOD;

    return ret;
}

URet blt_mikal(mikal_t **args, ...){
    mikal_t *l_val;
    mikal_t *r_val;
    
    mikal_t *cmp_result;
    URet ret;

    l_val = args[0];
    r_val = args[1];

    if(!valid_mikal(l_val) || !valid_mikal(r_val)){
        ret.ret_union.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }else if(l_val->type != MT_INTEGER || r_val->type != MT_INTEGER){
        ret.ret_union.val = 0;
        ret.error_code = E_CASE_UNIMPL;
        return ret;
    }

    cmp_result = URet_val(make_bool((l_val->mk_data.integer < r_val->mk_data.integer) ? BOOL_TRUE : BOOL_FALSE )\
            , mikal_t *);

    ret.ret_union.addr = cmp_result;
    ret.error_code = GOOD;

    return ret;
}

URet bgt_mikal(mikal_t **args, ...){
    mikal_t *l_val;
    mikal_t *r_val;
    
    mikal_t *cmp_result;
    URet ret;

    l_val = args[0];
    r_val = args[1];

    if(!valid_mikal(l_val) || !valid_mikal(r_val)){
        ret.ret_union.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }else if(l_val->type != MT_INTEGER || r_val->type != MT_INTEGER){
        ret.ret_union.val = 0;
        ret.error_code = E_CASE_UNIMPL;
        return ret;
    }

    cmp_result = URet_val(make_bool((l_val->mk_data.integer > r_val->mk_data.integer) ? BOOL_TRUE : BOOL_FALSE )\
            , mikal_t *);

    ret.ret_union.addr = cmp_result;
    ret.error_code = GOOD;

    return ret;
}

URet if_mikal(mikal_t **args, ...){
    URet ret;
    printf("placeholder function for if, I will do nothing\n");
    ret.ret_union.val = 0;
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
