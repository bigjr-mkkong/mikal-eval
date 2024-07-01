#include "mikal_type.h"
#include "stdlib.h"
#include "gc.h"
#include "stdio.h"

gc_buffer gcbuf;

URet init_gcbuffer(int size){
    URet ret;
    if(size > MAX_GC_RECORDS){
        ret.val = 0;
        ret.error_code = E_OUTOFBOUND;
        return ret;
    }

    gcbuf.mikal_gc = (mikal_t**)calloc(1, sizeof(mikal_t*) * size);
    gcbuf.next_pt = 0;
    gcbuf.max_mikal = size;

    ret.val = 0;
    ret.error_code = GOOD;
    return ret;
}

URet add_gc_mikal(mikal_t *addr){
    URet ret;
    if(!valid_mikal(addr)){
        ret.val = 0;
        ret.error_code = E_INVAL_TYPE;
        return ret;
    }

    if(gcbuf.next_pt >= gcbuf.max_mikal){
        ret.val = 0;
        ret.error_code = E_NOSPACE_LEFT;
        return ret;
    }

    /*
     * If current mikal_t* has more than 1 refcnt,
     * then it means this is a reference, and we should
     * just avoid free a reference to prevent double
     * free
     */
    if(addr->refcnt != 1){
        ret.val = 0;
        ret.error_code = E_FAILED;
        return ret;
    }

    gcbuf.mikal_gc[gcbuf.next_pt] = addr;
    gcbuf.next_pt++;

    ret.val = 0;
    ret.error_code = GOOD;

    return ret;
}

void gc_clear_all(){
    for(int i=0; i<gcbuf.next_pt; i++){
        mikal_t *target = gcbuf.mikal_gc[i];
        if(target->refcnt > 1){
            target->refcnt--;
        }else{
            destroy_mikal(target);
            gcbuf.mikal_gc[i] = NULL;
        }
    }

    gcbuf.next_pt = 0;
    gcbuf.freed_next = 0;
    return;
}

void destroy_gcbuffer(){
    gc_clear_all(gcbuf);

    free(gcbuf.mikal_gc);

    gcbuf.next_pt = 0;
    gcbuf.max_mikal = 0;

    return;
}
