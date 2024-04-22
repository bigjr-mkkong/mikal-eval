#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "reader.h"

static char *arith_op[] = {"+", "-", "*", "/"};
static char *env_op[] = {"def!", "let!", "set!"};
static char *list_op[] = {"car", "cdr", "cons"};
static char *io_op[] = {"print"};

#include "types.h"

struct Gen_type_t *make_integer(long x){
    struct Gen_type_t *ret = (struct Gen_type_t*)malloc(sizeof(struct Gen_type_t));
    ret->value.integer = x;
    ret->type = TYPE_INTEGER;
    return ret;
}

struct Gen_type_t *make_string(char *str){
    int str_len = ROUND_UP(strlen(str));
    struct Gen_type_t *ret = (struct Gen_type_t*)malloc(sizeof(struct Gen_type_t));
    ret->value.str = (char*)malloc(str_len);
    strcpy(ret->value.str, str);
    ret->type = TYPE_STRING;
    return ret;
}

struct Gen_type_t *make_symbol(char *str){
    int str_len = ROUND_UP(strlen(str));
    struct Gen_type_t *ret = (struct Gen_type_t*)malloc(sizeof(struct Gen_type_t));
    ret->value.symbol = (char*)malloc(str_len);
    strcpy(ret->value.str, str);
    ret->type = TYPE_SYMBOL;
    return ret;
}

struct Gen_type_t *make_operator(char *x){
    int str_len = ROUND_UP(strlen(x));
    struct Gen_type_t *ret = (struct Gen_type_t*)malloc(sizeof(struct Gen_type_t));
    ret->value.op = (char*)malloc(str_len);
    strcpy(ret->value.op, x);
    ret->type = TYPE_OPERATOR;
    return ret;
}

struct Gen_type_t *make_list(struct Gen_type_t **lst){
    struct Gen_type_t *pt;

    struct Gen_type_t *ret = (struct Gen_type_t*)malloc(sizeof(struct Gen_type_t));
    ret->value.pateval_list = (struct Gen_type_t**)calloc(64, sizeof(struct Gen_type_t*));
    ret->type = TYPE_PATEVAL_LIST;
    for(int i=0; lst[i]; i++){
        ret->value.pateval_list[i] = lst[i];
    }

    return ret;
}

/* make_pair will not allocate memory space for car and cdr */
struct Gen_type_t *make_pair(struct Gen_type_t *car, struct Gen_type_t *cdr){
    struct Gen_type_t *ret = (struct Gen_type_t*)malloc(sizeof(struct Gen_type_t));
    ret->car = car;
    ret->cdr = cdr;
    ret->type = TYPE_PAIR;
    return ret;
}

void destroy_gentype(struct Gen_type_t *gentype){

    switch(gentype->type){
        case TYPE_SYMBOL:   
            free(gentype->value.symbol);
            free(gentype);
            break;
        case TYPE_STRING:
            free(gentype->value.str);
            free(gentype);
            break;
        case TYPE_OPERATOR:
            free(gentype->value.op);
            free(gentype);
            break;

        case TYPE_PATEVAL_LIST:
            struct Gen_type_t *pt;
            for(int i=0; gentype->value.pateval_list[i]; i++){
                pt = gentype->value.pateval_list[i];
                if(pt != NULL)
                    destroy_gentype(gentype->value.pateval_list[i]);
            }
            free(gentype->value.pateval_list);
            free(gentype);
            break;

        default:
            free(gentype);
            break;
    }

    return;
}

struct Gen_type_t *token2gen(struct Token *token){
    struct Gen_type_t *ret = NULL;

    switch (token->type){
        case TOKEN_NUMBER:
            ret = make_integer(atoi(token->tok));
            ret->type = TYPE_INTEGER;
            break;

        case TOKEN_STRING:
            ret = make_string(token->tok);
            ret->type = TYPE_STRING;
            break;

        case TOKEN_REGULAR:
            ret = make_symbol(token->tok);
            ret->type = TYPE_SYMBOL;
            break;

        case TOKEN_SPECIAL:
            ret = make_symbol(token->tok);
            ret->type = TYPE_SYMBOL;
            break;

        case TOKEN_OPERATOR:
            ret = make_operator(token->tok);
            ret->type = TYPE_OPERATOR;
            break;

        default:
            fprintf(stderr, "Failed to convert token to Gen type: Unsupported type of token\n");
            break;
    }

    return ret;
}

struct Token *gen2token(struct Gen_type_t *gentype){
    struct Token *tok = NULL;

    switch (gentype->type){
        case TYPE_INTEGER:
            tok = (struct Token *)malloc(sizeof(struct Token));
            snprintf(tok->tok, 128, "%d", gentype->value.integer);
            tok->type = TOKEN_NUMBER;
            break;

        case TYPE_STRING:
            tok = (struct Token *)malloc(sizeof(struct Token));
            strcpy(tok->tok, gentype->value.str);
            tok->type = TOKEN_STRING;
            break;

        case TYPE_SYMBOL:
            tok = (struct Token *)malloc(sizeof(struct Token));
            strcpy(tok->tok, gentype->value.op);
            tok->type = TOKEN_REGULAR;
            break;

        default:
            fprintf(stderr, "This type is not ready to be converted into a token\n");
            break;
    }

    return tok;
}

enum operation_type which_op(char *str){
    int lstlen_arithop = sizeof(arith_op) / sizeof(char*);
    int lstlen_envop = sizeof(env_op) / sizeof(char*);
    int lstlen_listop = sizeof(list_op) / sizeof(char*);

    enum operation_type op_type = OP_UNDEFINED;
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
            op_type = OP_LIST;
            return op_type;
        }
    }

    return op_type;
}

enum env_op_type which_envop(char *str){
    if(strcmp(str, env_op[0]) == 0)
        return ENVOP_DEFINE;
    else if(strcmp(str, env_op[1]) == 0)
        return ENVOP_LET;
    else if(strcmp(str, env_op[2]) == 0)
        return ENVOP_SET;
    else
        return ENVOP_UNDEFINED;
}

/*
int main(void){
    struct Gen_type_t *var = make_integer(4);

    printf("%d", var->value.integer);
    destroy_gentype(var);
}
*/
