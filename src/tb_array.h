#ifndef TB_ARRAY_H
#define TB_ARRAY_H

#include <stdlib.h>

#define TB_ARRAY_HDR_ELEM	size_t
#define TB_ARRAY_HDR_SIZE	2 * sizeof(TB_ARRAY_HDR_ELEM)

#define tb_array__max(a, b) ((a) >= (b) ? (a) : (b))

#define tb_array__hdr(a) ((size_t*)(void*)(a) - 2)
#define tb_array__cap(a) tb_array__hdr(a)[0]
#define tb_array__len(a) tb_array__hdr(a)[1]

#define tb_array_len(a) ((a) ? tb_array__len(a) : 0)
#define tb_array_cap(a) ((a) ? tb_array__cap(a) : 0)

#define tb_array__growth(a)     ((a) && (tb_array__len(a) >= tb_array__cap(a)) ? (2 * tb_array__cap(a)) : 0)
#define tb_array__make_space(a) (*((void**)&(a)) = tb_array__resize((a), sizeof(*(a)), tb_array__growth(a), 0))

#define tb_array_resize(a, n)   (*((void**)&(a)) = tb_array__resize((a), sizeof(*(a)), (n), 1))
#define tb_array_reserve(a, n)  (*((void**)&(a)) = tb_array__resize((a), sizeof(*(a)), (n), 0))
#define tb_array_free(a)        ((a) ? (free(tb_array__hdr(a)), (a) = NULL) : 0);

#define tb_array_push(a, v)     (tb_array__make_space(a), (a)[tb_array__len(a)++] = (v))

#define tb_array_pack(a)    (tb_array_resize((a), tb_array_len(a)))
#define tb_array_clear(a)   ((a) ? tb_array__len(a) = 0 : 0)

#define tb_array_last(a)    ((a) + tb_array_len(a))
#define tb_array_sizeof(a)  (tb_array_len(a) * sizeof(*(a)))

void* tb_array__resize(void* arr, size_t elem_size, size_t new_cap, int shrink);

/* extra functionality */
#define tb_array_insert(a, v, cmp)  (*((void**)&(a)) = tb_array__insert((a), sizeof(*(a)), (v), (cmp)))
#define tb_array_remove(a, i)       (*((void**)&(a)) = tb_array__remove((a), sizeof(*(a)), (i)))

void* tb_array__insert(void* arr, size_t elem_size, void* value, int (*cmp)(const void*,const void*));
void* tb_array__remove(void* arr, size_t elem_size, size_t index);

#endif /* !TB_ARRAY_H */
