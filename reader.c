#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include "reader.h"
#include "env.h"

#define MAX_SYM     512

//#define READER_TEST

static char *parens[] = {"{", "}", "(", ")", "[", "]"};
static char special_char[] = {'[', ']', '{', '}', '(', ')', '\'', ' ', '`', ';', '\t'};
static int is_special_char(int ch){
    for(int i=0; i<sizeof(special_char); i++){
        if(ch == special_char[i]){
            return 1;
        }
    }
    return 0;
}

static int get_token_len(char *st){
    for(int i=0; i<sizeof(special_char); i++){
        if(*st == special_char[i]){
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

struct Reader *tokenize(char *line){
    int line_len = strlen(line);

    if(line_len <= 0)
        return NULL;
    
    struct Reader *token_reader = calloc(1, sizeof(struct Reader)); 
    token_reader->token_list = (struct Token*)calloc(512, sizeof(struct Token));
    struct Token *token_list = token_reader->token_list;
    int wrtpt = 0;
    
    int next = 0;
    int token_len = 0;
    for(; line[next] != '\0';){
        if(line[next] == ' ' || line[next] == ',' || line[next] == '\t'){
            next += 1;
            continue;
        }
        if(next < (line_len-1) && (line[next] == '~' && line[next+1] == '@')){
            memcpy((token_list[wrtpt].tok), &(line[next]), 2);
            
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
                token_list[wrtpt].type = TOKEN_RPAREN;
            }else if(strcmp((token_list[wrtpt].tok), "(") == 0){
                token_list[wrtpt].type = TOKEN_LPAREN;
            }else{
                token_list[wrtpt].type = TOKEN_REGULAR;
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


struct AST_Node *AST_Node_create(struct Token *tok, struct AST_Node **ops, int isleaf){
    struct AST_Node *new_node = (struct AST_Node*)calloc(1, sizeof(struct AST_Node));

    for(int i=0; i<64; i++)
        new_node->ops[i] = NULL;

    if(tok != NULL)
        memcpy(&(new_node->token), tok, sizeof(struct AST_Node));
    
    memcpy(new_node->ops, ops, sizeof(void*) * 64);
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

        if(tok->type != TOKEN_LPAREN && tok->type !=TOKEN_RPAREN){
            subexp_paren_cnt = 0;
            ops[ops_pt] = AST_create(tk_reader, idx, idx);
        }else if(tok->type == TOKEN_LPAREN){
            paren_match = 0;
            subexp_start = idx;
            for(; idx<=end; idx++){
                tok = &(tk_list[idx]);

                if(tok->type == TOKEN_LPAREN){
                    subexp_paren_cnt++;
                }else if(tok->type == TOKEN_RPAREN){
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

void print_ast(struct AST_Node *root){
    if(!root) return;

    if(root->ops[0] == NULL){
        printf("%s", root->token.tok);
        return;
    }
    
    struct AST_Node *pt = root->ops[0];

    printf("(");
    for(int i=0; i<64 && pt; i++, pt = root->ops[i]){
        print_ast(pt);
        printf(" ");
    }
    printf(")");


}

struct AST_Node *copy_ast(struct AST_Node *root){
    struct AST_Node *new_node = (struct AST_Node*)calloc(1, sizeof(struct AST_Node));
    memcpy(new_node, root, sizeof(struct AST_Node));
    for(int i=0; i<MAX_CHILD; i++){
        if(root->ops[i]){
            new_node->ops[i] = copy_ast(root->ops[i]);
        }
    }

    return new_node;
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
    char str[] = "(+ 2 3 (+ 3 4))";
    struct Reader *tk_reader = tokenize(str);

    struct AST_Node *AST_root = AST_create(tk_reader, 0, tk_reader->max_token - 1);

    print_ast(AST_root);
}

#endif
