#ifndef __ENV_H__
#define __ENV_H__

#include "reader.h"
#include "mikal_type.h"

struct env_entry{
    mikal_t symbol;
    mikal_t value;
};

struct env_t{
    struct env_entry *symmap[64];
    int next;
    struct env_t *fa_env;
    int ref_cnt;
};

URet init_env(void);
void add_env_function(struct env_t *env, char *name, mikal_t *(*func)(mikal_t **));
void add_env_integer(struct env_t *env, char *name, mikal_t *val);
void remove_env_entry(struct env_t *env, int idx);
URet lookup_env(struct env_t *env, char *name);
URet lookup_single_env(struct env_t *env,  char *name);
void destroy_env(struct env_t *env);
int is_global_env(struct env_t *env);


#endif
