#include "mikal_type.h"
#include "stdlib.h"
#include "gc.h"

gc_buffer gcbuf;

URet init_gcbuffer(int size){
    URet ret;
    if(size > MAX_GC_RECORDS){
        ret.val = 0;
        ret.error_code = E_OUTOFBOUND;
        return ret;
    }

    gcbuf.mikal_gc = (mikal_t**)calloc(1, sizeof(mikal_t*) * size);
    gcbuf.freed = (mikal_t**)calloc(1, sizeof(mikal_t*) * size);
    gcbuf.next_pt = 0;
    gcbuf.freed_next = 0;
    gcbuf.max_mikal = size;

    ret.val = 0;
    ret.error_code = GOOD;
    return ret;
}

URet add_gc_mikal(mikal_t *addr){
    URet ret;
    if(gcbuf.next_pt >= gcbuf.max_mikal){
        ret.val = 0;
        ret.error_code = E_NOSPACE_LEFT;
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
        gcbuf.freed[gcbuf.freed_next] = gcbuf.mikal_gc[i];
        gcbuf.freed_next++;
        destroy_mikal(gcbuf.mikal_gc[i]);
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
