#ifndef __READER_H__ 
#define __READER_H__


enum token_type{
    TOKEN_REGULAR,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_UNDEFINED
};

struct Token{
    char tok[128];
    enum token_type type;
};

#include "mikal_type.h"

struct Reader{
    struct Token *token_list;
    int max_token;
    int readpt;
};

#define MAX_CHILD       64

struct AST_Node{
    struct Token token;
    struct AST_Node *ops[MAX_CHILD];
};

void AST_destroy(struct AST_Node *root);
struct AST_Node *line_reader(char *line);
void print_ast(struct AST_Node *root);

#endif
