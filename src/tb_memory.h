#ifndef TB_MEMORY_H
#define TB_MEMORY_H

#include <stdlib.h>

#define TB_MEM_ALLOC(size) tb_memroy_allocate(size, __FILE__, __LINE__)

void* tb_memroy_allocate(size_t size, const char* file, int line);

#endif /* !TB_MEMORY_H */