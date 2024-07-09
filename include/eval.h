#ifndef __EVAL_H__
#define __EVAL_H__

#include "mikal_type.h"
#include "env.h"

URet eval(struct AST_Node *root, struct env_t *env);

#endif
