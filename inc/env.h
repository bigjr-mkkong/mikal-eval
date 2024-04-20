#ifndef __ENV_H__
#define __ENV_H__

#include "reader.h"
#include "types.h"

enum env_val_type{
    ENV_FUNC,
    ENV_GENVAL,
};

struct env_entry{
    char name[32];
    union env_value{
        struct Gen_type_t *(*func)(struct Gen_type_t **);
        struct Gen_type_t *gen_val; 
    }value;

    enum env_val_type type;
};

struct env_t{
    struct env_entry *symmap[64];
    int next;
    struct env_t *fa_env;
};

struct env_t *init_env(void);
void add_env_function(struct env_t *env, char *name, struct Gen_type_t *(*func)(struct Gen_type_t **));
void add_env_integer(struct env_t *env, char *name, struct Gen_type_t *val);
void remove_env_entry(struct env_t *env, int idx);
struct env_entry *lookup_env(struct env_t *env, char *name);
struct env_entry *lookup_single_env(struct env_t *env,  char *name);
void destroy_env(struct env_t *env);
int is_global_env(struct env_t *env);
enum env_op_type which_envop(char *str);


#endif
