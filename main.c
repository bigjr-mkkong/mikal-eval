#include "stdio.h"
#include "stdlib.h"
#include "mikal_type.h"
#include "reader.h"
#include "readline/readline.h"
#include "env.h"

struct AST_Node *READ(char *prompt){
    char *user_in;
    user_in = readline(prompt);

    if(user_in == NULL){
        return NULL;
    }

    struct AST_Node *AST_root = line_reader(user_in);
    return AST_root;
}

struct AST_Node *EVAL(struct AST_Node *root, struct env_t *env){
    if(root == NULL)
        return NULL;
    else
        return root;
}

void PRINT(struct AST_Node *root){
    print_ast(root);
    printf("\n");
    return;
}

int main(void){

    char prompt[128];
    struct AST_Node *ast, *exp;

    snprintf(prompt, sizeof(prompt), "user> ");

    struct env_t *meta_env = URet_val(init_env(), struct env_t*);
    
    while(1){
        ast = READ(prompt);

        if(ast == NULL){
            break;
        }
        exp = EVAL(ast, meta_env);
        PRINT(exp);
        AST_destroy(ast);
    }
    destroy_env(meta_env);
}
