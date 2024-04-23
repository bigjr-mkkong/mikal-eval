#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "reader.h"
#include "env.h"
#include <sys/param.h>

#define MAX_SYM     512

//#define READER_TEST

char single_symtab[] = {'[', ']', '{', '}', '(', ')', '\'', '`', '~', '^', '@'};

char special_char[] = {'[', ']', '{', '}', '(', ')', '\'', ' ', '\"', '`', ';'};


int is_special_char(int ch){
    for(int i=0; i<sizeof(special_char); i++){
        if(ch == special_char[i]){
            return 1;
        }
    }
    return 0;
}

static int is_integer(char *num){
    int len = strlen(num);
    for(int i=0; i<len; i++){
        if(num[i] < '0' || num[i] > '9')
            return 0;
    }
    return 1;
}
/*
int is_arith_op(char *str){
    int oplist_len = sizeof(arith_op)/sizeof(char*);

    for(int i=0; i<oplist_len; i++){
        if(strcmp(arith_op[i], str) == 0)
            return 1;
    }
    return 0;
}
*/
static int get_token_len(char *st){
    for(int i=0; i<sizeof(single_symtab); i++){
        if(*st == single_symtab[i]){
            return 1;
        }
    }
    int endpt = 1;
    if(*st == '\"'){
        while((st[endpt] != '\"') || ((st[endpt] == '\"') && (st[endpt-1] == '\\'))){
            endpt++;
            if(st[endpt] == 0){
                fprintf(stderr, "String not close\n");
                return -1;
            }
        }
        return endpt + 1;
    }else{
        for(; *(st+endpt); endpt++){
            if(is_special_char(st[endpt])){
                break;
            }

            if(st[endpt] == ','){
                break;
            }
        }
    }

    return endpt;
}
/**
 *  This function needs to have a more elegant implementation
 * 
 **/
enum token_type get_token_type(struct Token *token){
    char *tok_str = token->tok;
    int len = strlen(tok_str);
   
    if(tok_str[0] == '\"' && tok_str[len-1] == '\"')
        return TOKEN_STRING;

    if(tok_str[0] == '0' && len > 1){
        fprintf(stderr, "Number shouldn't contain leading zeros\n");
        return TOKEN_UNDEFINED;
    }

    enum operation_type op_type = which_op(tok_str);
    if(op_type != OP_UNDEFINED){
         return TOKEN_OPERATOR;
    }

    for(int i=0; i<len; i++){
        if(\
            is_integer(tok_str) || \
            (tok_str[0] == '-' && is_integer(&(tok_str[1]))) \
          )
            return TOKEN_NUMBER;
    }
    return TOKEN_REGULAR;
}

struct Reader *tokenize(char *line){
    int line_len = strlen(line);
    
    struct Reader *token_reader = calloc(1, sizeof(struct Reader)); 
    token_reader->token_list = (struct Token*)calloc(512, sizeof(struct Token));
    struct Token *token_list = token_reader->token_list;
    int wrtpt = 0;
    
    int next = 0;
    int token_len = 0;
    for(; line[next] != '\0';){
        if(line[next] == ' ' || line[next] == ','){
            next += 1;
            continue;
        }

        if(next < (line_len-1) && (line[next] == '~' && line[next+1] == '@')){
            memcpy((token_list[wrtpt].tok), &(line[next]), 2);
            token_list[wrtpt].type = TOKEN_SPECIAL;
            
            token_len = 2;
            next += token_len;
            wrtpt += 1;
        }else{
            token_len = get_token_len(&(line[next]));
            if(token_len < 0){
                free(token_reader->token_list);
                free(token_reader);
                return NULL;
            }
            memcpy((token_list[wrtpt].tok), &(line[next]), token_len);
            if(strcmp((token_list[wrtpt].tok), ")") == 0){
                token_list[wrtpt].type = R_PAREN;
            }else if(strcmp((token_list[wrtpt].tok), "(") == 0){
                token_list[wrtpt].type = L_PAREN;
            }else{
                token_list[wrtpt].type = get_token_type(&(token_list[wrtpt]));
                if(token_list[wrtpt].type == OP_UNDEFINED){
                    free(token_reader->token_list);
                    free(token_reader);
                    return NULL;
                }
            }
            next += token_len;
            wrtpt += 1;
        }
    }
    token_reader->token_list = token_list;
    token_reader->max_token = wrtpt;
    token_reader->readpt = 0;

    return token_reader;
}

static void destroy_reader(struct Reader *tk_reader){
    free(tk_reader->token_list);
    free(tk_reader);
    return;
}


int AST_Node_isleaf(struct AST_Node *ptr){
    return ptr->isleaf;
}

struct AST_Node *AST_Node_create(struct Token *tok, struct AST_Node **ops, int isleaf){
    struct AST_Node *new_node = (struct AST_Node*)calloc(1, sizeof(struct AST_Node));

    for(int i=0; i<64; i++)
        new_node->ops[i] = NULL;

    if(tok != NULL)
        memcpy(&(new_node->token), tok, sizeof(struct AST_Node));
    
    memcpy(new_node->ops, ops, sizeof(void*) * 64);
    new_node->isleaf = isleaf;
    return new_node;
}

void AST_destroy(struct AST_Node *root){
    if(root == NULL){
        return;
    }
    for(int i=0; i<64; i++){
        if(root->ops[i] != NULL){
            AST_destroy(root->ops[i]);
        }
    }

    if(root->env && !is_global_env(root->env)){
        destroy_env(root->env);
    }

    if(root->gen_val != NULL)
        destroy_gentype(root->gen_val);
    free(root);
    return;
}

struct AST_Node *AST_create(struct Reader *tk_reader, int begin, int end){
    struct Token *tk_list = tk_reader->token_list;
    
    struct AST_Node *ops[64];
    memset(ops, 0, sizeof(void*) * 64);
    
    if(begin == end){
        return AST_Node_create(&(tk_list[begin]), ops, 1);
    }

    int idx = begin;

    int subexp_start = 0;
    int subexp_paren_cnt = 0;

    int ops_pt = 0;  

    int paren_match;

    for(;idx < end; ops_pt++){
        idx++;
        struct Token *tok = &(tk_list[idx]);     

        if(tok->type != L_PAREN && tok->type !=R_PAREN){
            subexp_paren_cnt = 0;
            ops[ops_pt] = AST_create(tk_reader, idx, idx);
        }else if(tok->type == L_PAREN){
            paren_match = 0;
            subexp_start = idx;
            for(; idx<=end; idx++){
                tok = &(tk_list[idx]);

                if(tok->type == L_PAREN){
                    subexp_paren_cnt++;
                }else if(tok->type == R_PAREN){
                    subexp_paren_cnt--;
                }

                if(subexp_paren_cnt == 0){
                    paren_match = 1;
                    ops[ops_pt] = AST_create(tk_reader, subexp_start, idx);
                    break;
                }
            }

            if(paren_match == 0){
                fprintf(stderr, "epression not close\n");
                return NULL;
            }
        }else{
            continue;
        }
        
        subexp_paren_cnt = 0;
    }
     
   return AST_Node_create(NULL, ops, 0);
}

void print_pateval_list(struct Gen_type_t *lst){
    struct Token *tok;
    if(lst->type != TYPE_PATEVAL_LIST){
        tok = gen2token(lst);
        printf("%s", tok->tok);
        free(tok);
        return; 
    }

    printf("(");
    for(int i=0; lst->value.pateval_list[i]; i++){
        print_pateval_list(lst->value.pateval_list[i]);
        if(lst->value.pateval_list[i+1] != NULL)
            printf(" ");
    }
    printf(")");

    return;
}

void print_pair_list(struct Gen_type_t *root){
    struct Token *tok;
    if(root->type != TYPE_PAIR){
        tok = gen2token(root);
        printf("%s", tok->tok);
        free(tok);
        return;
    }

    printf("(");
    print_pair_list(root->pair.car);
    printf(" . ");
    print_pair_list(root->pair.cdr);
    printf(")");
    return;
}

void pr_str(struct AST_Node *pos){
    struct Token *tok;
    if(pos == NULL) return;

    if(AST_Node_isleaf(pos)){ 
        tok = gen2token(pos->gen_val);
        memcpy(&(pos->token), tok, sizeof(struct Token));
        free(tok);
        printf("%s", pos->token.tok);
        return;
    }else if(pos->gen_val->type == TYPE_PATEVAL_LIST){
        print_pateval_list(pos->gen_val);
        return;
    }else if(pos->gen_val->type == TYPE_PAIR){
        print_pair_list(pos->gen_val);
        return;
    }

    printf("(");
    for(int i=0; i<63; i++){
        if(pos->ops[i] != NULL){
            pr_str(pos->ops[i]);
        }

        if(pos->ops[i+1] != NULL){
            printf(" ");
        }
    }
    printf(")");

    return;
}

struct AST_Node *line_reader(char *line){
    struct Reader *tk_reader = tokenize(line);
    if(tk_reader == NULL)
        return NULL;

    struct AST_Node *AST_root = AST_create(tk_reader, 0, MAX(0, tk_reader->max_token-1));
    
    destroy_reader(tk_reader);
    free(line);
    return AST_root;
}

#ifdef READER_TEST

int main(void){
    char str[] = "+";
    struct Reader *tk_reader = tokenize(str);

    struct AST_Node *AST_root = AST_create(tk_reader, 0, tk_reader->max_token - 1);

    pr_str(AST_root, AST_root);
}

#endif
