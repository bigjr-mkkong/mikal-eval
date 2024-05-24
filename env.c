#include "mikal_type.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

//#define ENV_TEST

/*
 * Return the new allocated index for given environemnt
 */
static URet alloc_env_slot(struct env_t *env){
    URet ret;
    if(env->next > 64){
        ret.val = 0;
        ret.error_code = E_NOSPACE_LEFT;
        return ret;
    }

    int retval = env->next;
    env->symmap[env->next] = (struct env_entry*)malloc(sizeof(mikal_t) * 2);
    memset(env->symmap[env->next], 0, sizeof(struct env_entry));
    env->next++;

    ret.val = retval;
    ret.error_code = GOOD;
    return ret;
}

static URet free_env_slot(struct env_t *env, int idx){
    URet ret;
    if(idx >= 64){
        ret.val = 0;
        ret.error_code = E_OUTOFBOUND;
        return ret;
    }

    if(env->symmap[idx] == NULL){
        ret.val = 0;
        ret.error_code = E_DOUBLE_FREE;
        return ret;
    }

    free(env->symmap[idx]);
    env->symmap[idx] = 0;

    ret.val = 0;
    ret.error_code = GOOD;
    return ret;
}

URet init_env(void){
    URet ret;
    struct env_t *ret_env = (struct env_t*)malloc(sizeof(struct env_t));
    //ret_env->symmap = (struct env_entry**)malloc(sizeof(struct env_entry*) * 64);

    ret_env->fa_env = 0;

    ret.addr = ret_env;
    ret.error_code = GOOD;
    return ret;
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

URet add_env_entry(struct env_t *env, mikal_t *symbol, mikal_t *value){
    URet ret;
    if(!valid_mikal(symbol)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }
    
    if(!valid_mikal(value)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    int alloc_pt = URet_val(alloc_env_slot(env), int);
    env->symmap[alloc_pt]->symbol = *symbol;
    env->symmap[alloc_pt]->value = *value;

    ret.val = 0;
    ret.error_code = GOOD;
    return ret;
}

void remove_env_entry(struct env_t *env, int idx){
    free_env_slot(env, idx);
    return;
}

URet lookup_env(struct env_t *env, char *name){
    URet ret;
    URet callret1, callret2, mk_tmp;
    struct env_entry *env_ent;

    ret.val = 0;
    ret.error_code = E_NOTFOUND;
    for(int i=0; i<64; i++){
        env_ent = env->symmap[i];
        if(env_ent == NULL){
            continue;
        }
        if(strcmp(name, env_ent->symbol.sym) == 0){
            ret.error_code = GOOD;
            break;
        }
    }

    if((URet_state(ret) == E_NOTFOUND) && is_global_env(env)){
        return ret;
    }else if(URet_state(ret) == E_NOTFOUND){
        return lookup_env(env->fa_env, name);
    }else if(URet_state(ret) == GOOD && env_ent->value.type == MT_SYMBOL){
        return lookup_env(env, env_ent->value.sym);
    }else{
        ret.addr = env_ent;
        ret.error_code = GOOD;
        return ret;
    }
}

URet lookup_single_env(struct env_t *env,  char *name){
    URet ret;
    struct env_entry *env_ent;

    ret.error_code = E_UNDEF;
    for(int i=0; i<64; i++){
        env_ent = env->symmap[i];
        if(env_ent == NULL){
            continue;
        }
        if(strcmp(name, env_ent->symbol.sym) == 0){
            ret.error_code = E_NOTFOUND;
            break;
        }
    }

    if(URet_state(ret) == E_NOTFOUND){
        ret.val = 0;
        return ret;
    }else{
        ret.val = (long long)env_ent;
        ret.error_code = GOOD;
        return ret;
    }
}

int is_global_env(struct env_t *env){
    return env->fa_env == env;
}

#ifdef ENV_TEST

int main(void){
    struct env_t *meta_env = init_env();
    mikal_t *plus = make_operator("+");
    add_env_entry(meta_env, "+", &add);
    add_env_entry(meta_env, "-", &sub);

    struct env_entry *target = lookup_env(meta_env, "-");

    struct Gen_type_t *a = make_integer(10);
    struct Gen_type_t *b = make_integer(14);
    struct Gen_type_t *ans = target->func(a, b);

    printf("%d\n", ans->value.integer);
}

#endif
