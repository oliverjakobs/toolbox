#ifndef TB_MEM_H
#define TB_MEM_H

#include <stdlib.h>

typedef void* (*tb_mem_malloc_callback)     (size_t size);
typedef void* (*tb_mem_realloc_callback)    (void* block, size_t old_size, size_t new_size);
typedef void  (*tb_mem_free_callback)       (void* block, size_t size);

typedef struct
{
    tb_mem_malloc_callback  malloc;
    tb_mem_realloc_callback realloc;
    tb_mem_free_callback    free;
} tb_allocator;

void* tb_mem_malloc(tb_allocator* allocator, size_t size);
void* tb_mem_calloc(tb_allocator* allocator, size_t count, size_t size);
void* tb_mem_realloc(tb_allocator* allocator, void* block, size_t size);
void  tb_mem_free(tb_allocator* allocator, void* block);
void  tb_mem_constfree(tb_allocator* allocator, const void* block);

void* tb_mem_dup(tb_allocator* allocator, const void* src, size_t size);

#endif /* !TB_MEM_H */
