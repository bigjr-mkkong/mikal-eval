//#define SINGLE_FILE_TEST

#include "./inc/mikal_type.h"
#include "stdlib.h"
#include "string.h"
#include "env.h"
#include "stdio.h"

enum mikal_op_type which_op(char *str);

static char *arith_op[] = {"+", "-", "*", "/"};
static char *env_op[] = {"def!", "let!", "set!"};
static char *list_op[] = {"car", "cdr", "cons"};
static char *br_op[] = {"if"};
static char *lambda_op[] = {"lambda"};

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

URet make_operator(char *op_name){
    URet retval;

    if(!op_name){
        retval.val = 0;
        retval.error_code = E_INVAL_ADDR;
        return retval;
    }

    int name_len = strlen(op_name);
    if(name_len <= 0){
        retval.val = 0;
        retval.error_code = E_EMPTY;
        return retval;
    }
    
    mikal_t *ret = (mikal_t*)malloc(sizeof(mikal_t));
    ret->op = (char*)malloc(sizeof(name_len));
    strcpy(ret->op, op_name);
    ret->type = MT_OPERATOR;
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

URet make_func(mikal_func func){
    URet retval;

    if(!func){
        retval.val = 0;
        retval.error_code = E_INVAL_ADDR;
        return retval;
    }

    mikal_t *ret = (mikal_t*)malloc(sizeof(mikal_t));
    ret->func = func;
    ret->type = MT_FUNC;
    ret->magic = MIKAL_MAGIC;

    retval.addr = ret;
    retval.error_code = GOOD;
    return retval;
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

        case MT_OPERATOR:
            fprintf(stdout, "%s", target->op);
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

        case MT_OPERATOR:
            free(target->op);
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
    int cmp_result = 0;
    if(val1->type != val2->type){
        return cmp_result;
    }else{
        switch(val1->type){
            case MT_INTEGER:
                cmp_result = (val1->integer == val2->integer);
                break;

            case MT_SYMBOL:
                cmp_result = !(strcmp(val1->sym, val2->sym));
                break;

            case MT_OPERATOR:
                cmp_result = (val1->op_type == val2->op_type) && (!strcmp(val1->op, val2->op));
                break;
                
        }
    }
}

enum mikal_op_type which_op(char *str){
    int lstlen_arithop = sizeof(arith_op) / sizeof(char*);
    int lstlen_envop = sizeof(env_op) / sizeof(char*);
    int lstlen_listop = sizeof(list_op) / sizeof(char*);
    int lstlen_brop = sizeof(br_op) / sizeof(char*);
    int lstlen_lamop = sizeof(lambda_op) / sizeof(char*);

    enum mikal_op_type op_type = OP_UNDEF;
    if(str == NULL)
        return op_type;

    for(int i=0; i<lstlen_arithop; i++){
        if(strcmp(str, arith_op[i]) == 0){
            op_type = OP_ARITH;
            return op_type;
        }
    }

    for(int i=0; i<lstlen_envop; i++){
        if(strcmp(str, env_op[i]) == 0){
            op_type = OP_ENV;
            return op_type;
        }
    }

    for(int i=0; i<lstlen_listop; i++){
        if(strcmp(str, list_op[i]) == 0){
            op_type = OP_CONS;
            return op_type;
        }
    }

    for(int i=0; i<lstlen_brop; i++){
        if(strcmp(str, br_op[i]) == 0){
            op_type = OP_BRANCH;
            return op_type;
        }
    }

    for(int i=0; i<lstlen_lamop; i++){
        if(strcmp(str, lambda_op[i]) == 0){
            op_type = OP_LAMBDA;
            return op_type;
        }
    }


    return op_type;
}


int is_regchar(char x){
    return (x>='a' && x <= 'z') || (x>='A' && x <= 'Z');
}

/* enum mikal_types which_mktype(char *str, struct env_t *env){ */
/*     int len = strlen(str); */

/*     if(len <= 0) return MT_NONE; */

/*     if(str[0] == '\"' && str[len-1] == '\"' && len >= 2){ */
/*         return MT_STRING; */
/*     } */

/*     URet env_q = lookup_env(env, str); */
/*     if(URet_state(env_q) == GOOD && \ */
/*             (URet_val(env_q, struct env_entry*)->value.type == MT_FUNC)){ */
/*         return MT_OPERATOR; */
/*     } */

/*     int is_int = 1; */
/*     for(int i=0; i<len; i++){ */
/*         is_int &= (str[i] >=0 && str[i] <= 9); */
/*     } */
    
/*     if(is_int){ */
/*         return MT_INTEGER; */
/*     }else{ */
/*         if(!is_regchar(str[0])) */
/*             return MT_NONE; */
/*         else */
/*             return MT_SYMBOL; */
/*     } */
/* } */

URet add_mikal(mikal_t **args){
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

URet sub_mikal(mikal_t **args){
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

URet mul_mikal(mikal_t **args){
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

URet div_mikal(mikal_t **args){
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

#ifdef SINGLE_FILE_TEST

int main(void){
    
}

#endif

