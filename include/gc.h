#ifndef __MIKAL_GC_H__
#define __MIKAL_GC_H__

#define MAX_GC_RECORDS      256

typedef struct gc_buffer{
    mikal_t **mikal_gc;
    mikal_t **freed;
    int next_pt, max_mikal;
    int freed_next;
}gc_buffer;

URet init_gcbuffer(int size);
URet add_gc_mikal(mikal_t *addr);
void gc_clear_all();
void destroy_gcbuffer();
#endif
