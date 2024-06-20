#include "mikal_type.h"
#include "stdarg.h"

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
