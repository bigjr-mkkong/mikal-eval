#include "env.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

//#define ENV_TEST

static int alloc_env_slot(struct env_t *env){
    if(env->next > 64){
        fprintf(stderr, "Envioronment is full :(\n");
    }

    int retval = env->next;
    env->symmap[env->next] = (struct env_entry*)malloc(sizeof(struct env_entry));
    memset(env->symmap[env->next], 0, sizeof(struct env_entry));
    env->next++;
    return retval;
}

static void free_env_slot(struct env_t *env, int idx){
    if(idx >= 64){
        fprintf(stderr, "Failed to free env slot: idx Out of Bound\n");
        return;
    }

    if(env->symmap[idx] == NULL){
        fprintf(stderr, "Failed to free env slot: double free\n");
        return;
    }

    if(env->symmap[idx]->type == ENV_GENVAL){
        destroy_gentype(env->symmap[idx]->value.gen_val);
    }
    free(env->symmap[idx]);
    env->symmap[idx] = 0;
    return ;
}

struct env_t *init_env(void){
    struct env_t *ret_env = (struct env_t*)malloc(sizeof(struct env_t));

    ret_env->fa_env = 0;
    return ret_env;
}

void destroy_env(struct env_t *env){
    if(env->ref_cnt > 0){
        env->ref_cnt--;
        return;
    }
    for(int i=0; i<env->next; i++){
        free_env_slot(env, i);
    }
    free(env);
}

void add_env_function(struct env_t *env, char *name, struct Gen_type_t *(*func)(struct Gen_type_t **)){
    
    int alloc_pt = alloc_env_slot(env);
    strcpy(env->symmap[alloc_pt]->name, name);
    env->symmap[alloc_pt]->value.func = func;
    env->symmap[alloc_pt]->type = ENV_FUNC;

    return;
}

void add_env_integer(struct env_t *env, char *name, struct Gen_type_t *val){
    int alloc_pt = alloc_env_slot(env);
    strcpy(env->symmap[alloc_pt]->name, name);

    env->symmap[alloc_pt]->value.gen_val = make_integer(0xdeadbeef);
    memcpy(env->symmap[alloc_pt]->value.gen_val, val, sizeof(struct Gen_type_t));

    env->symmap[alloc_pt]->type = ENV_GENVAL;
    
    return;
}

void remove_env_entry(struct env_t *env, int idx){
    free_env_slot(env, idx);
    return;
}

struct env_entry *lookup_env(struct env_t *env, char *name){
    struct env_entry *env_ent;

    int find = 0;
    for(int i=0; i<64; i++){
        env_ent = env->symmap[i];
        if(env_ent == NULL){
            continue;
        }
        if(strcmp(name, env_ent->name) == 0){
            find = 1;
            break;
        }
    }

    if(!find && is_global_env(env)){
        return NULL;
    }else if(!find){
        return lookup_env(env->fa_env, name);
    }else{
        return env_ent;
    }
}

struct env_entry *lookup_single_env(struct env_t *env,  char *name){
    struct env_entry *env_ent;

    int find = 0;
    for(int i=0; i<64; i++){
        env_ent = env->symmap[i];
        if(env_ent == NULL){
            continue;
        }
        if(strcmp(name, env_ent->name) == 0){
            find = 1;
            break;
        }
    }

    if(!find){
        return NULL;
    }else{
        return env_ent;
    }
}

int is_global_env(struct env_t *env){
    return env->fa_env == env;
}

#ifdef ENV_TEST

int main(void){
    struct env_t *meta_env = init_env();
    add_env_entry(meta_env, "+", &add);
    add_env_entry(meta_env, "-", &sub);

    struct env_entry *target = lookup_env(meta_env, "-");

    struct Gen_type_t *a = make_integer(10);
    struct Gen_type_t *b = make_integer(14);
    struct Gen_type_t *ans = target->func(a, b);

    printf("%d\n", ans->value.integer);
}

#endif
