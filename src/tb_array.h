#ifndef TB_ARRAY_H
#define TB_ARRAY_H

#include <stdlib.h>

#define TB_ARRAY_HDR_ELEM	size_t
#define TB_ARRAY_HDR_SIZE	2 * sizeof(TB_ARRAY_HDR_ELEM)

#define tb_array__max(a, b) ((a) >= (b) ? (a) : (b))

#define tb_array__hdr(b) ((size_t*)(void*)(b) - 2)
#define tb_array__cap(b) tb_array__hdr(b)[0]
#define tb_array__len(b) tb_array__hdr(b)[1]

#define tb_array_len(b) ((b) ? tb_array__len(b) : 0)
#define tb_array_cap(b) ((b) ? tb_array__cap(b) : 0)

#define tb_array__growth(b)     ((b) ? (2 * tb_array__cap(b)) : 1)
#define tb_array__make_space(b) (*((void**)&(b)) = tb_array__resize((b), sizeof(*(b)), tb_array__growth(b), 0))

#define tb_array_resize(b, n)   (*((void**)&(b)) = tb_array__resize((b), sizeof(*(b)), (n), 1))
#define tb_array_reserve(b, n)  (*((void**)&(b)) = tb_array__resize((b), sizeof(*(b)), (n), 0))
#define tb_array_free(b)        ((b) ? (free(tb_array__hdr(b)), (b) = NULL) : 0);

#define tb_array_push(b, v)     (tb_array__make_space(b), (b)[tb_array__len(b)++] = (v))

#define tb_array_pack(b)    (tb_array_resize((b), tb_array_len(b)))
#define tb_array_clear(b)   ((b) ? tb_array__len(b) = 0 : 0)

#define tb_array_last(b)    ((b) + tb_array_len(b))
#define tb_array_sizeof(b)  (tb_array_len(b) * sizeof(*(b)))

void* tb_array__resize(void* buf, size_t elem_size, size_t new_cap, int shrink);

/* extra functionality */
#define tb_array_insert(b, v, cmp)  (*((void**)&(b)) = tb_array__insert((b), sizeof(*(b)), (v), (cmp)))
#define tb_array_remove(b, i)       (*((void**)&(b)) = tb_array__remove((b), sizeof(*(b)), (i)))

void* tb_array__insert(void* buf, size_t elem_size, void* value, int (*cmp)(const void*,const void*));
void* tb_array__remove(void* buf, size_t elem_size, size_t index);

#endif /* !TB_ARRAY_H */
