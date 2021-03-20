#ifndef TB_MEM_H
#define TB_MEM_H

#include <stdlib.h>

typedef void* (*tb_alloc)   (size_t size);
typedef void* (*tb_realloc) (void* block, size_t old_size, size_t new_size);
typedef void  (*tb_free)    (void* block, size_t size);

typedef struct
{
    tb_alloc    alloc;
    tb_realloc  realloc;
    tb_free     free;
} tb_allocator;

void* tb_mem_alloc(tb_allocator* allocator, size_t size);
void* tb_mem_realloc(tb_allocator* allocator, void* block, size_t size);
void  tb_mem_free(tb_allocator* allocator, void* block);

void* tb_mem_dup(tb_allocator* allocator, const void* src, size_t size);

#endif /* !TB_MEM_H */
