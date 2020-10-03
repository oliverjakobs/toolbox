#include "tb_memory.h"

#include <stdio.h>

void* tb_memroy_allocate(size_t size, const char* file, int line)
{
    void* block = malloc(size);

    if (block)
    {
        printf("Allocated %zu bytes (%s, %d)\n", size, file, line);
    }

    return block;
}