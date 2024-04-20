#ifndef __READER_H__ 
#define __READER_H__


enum token_type{
    TOKEN_REGULAR,
    TOKEN_NUMBER,
    TOKEN_STRING,
    L_PAREN,
    R_PAREN,
    TOKEN_SPECIAL,
    TOKEN_OPERATOR,
    TOKEN_UNDEFINED
};

struct Token{
    char tok[128];
    enum token_type type;
};

#include "types.h"

struct Reader{
    struct Token *token_list;
    int max_token;
    int readpt;
};

struct AST_Node{
    struct Token token;
    struct AST_Node *ops[64];
    struct Gen_type_t *gen_val;
    int isleaf;
    struct env_t *env;
};

void AST_destroy(struct AST_Node *root);
struct AST_Node *line_reader(char *line);
void pr_str(struct AST_Node *root);
struct Token *peek_operator(struct AST_Node *root);
int AST_Node_isleaf(struct AST_Node *ptr);
int is_arith_op(char *str);

#endif
