#ifndef __BUILDIN_FUNC_H__
#define __BUILDIN_FUNC_H__

URet add_mikal(mikal_t **args, ...);
URet sub_mikal(mikal_t **args, ...);
URet mul_mikal(mikal_t **args, ...);
URet div_mikal(mikal_t **args, ...);
URet make_clos_mikal(mikal_t **args, ...);

URet car_mikal(mikal_t **args, ...);
URet cdr_mikal(mikal_t **args, ...);
URet cons_mikal(mikal_t **args, ...);

URet def_mikal(mikal_t **args, ...);
URet set_mikal(mikal_t **args, ...);
URet let_mikal(mikal_t **args, ...);

URet beq_mikal(mikal_t **args, ...);
URet blt_mikal(mikal_t **args, ...);
URet bgt_mikal(mikal_t **args, ...);
URet if_mikal(mikal_t **args, ...);
#endif
