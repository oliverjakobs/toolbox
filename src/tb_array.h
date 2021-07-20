#ifndef TB_ARRAY_H
#define TB_ARRAY_H

#include <stdlib.h>

#define TB_ARRAY_HDR_ELEM	size_t
#define TB_ARRAY_HDR_SIZE	2 * sizeof(TB_ARRAY_HDR_ELEM)

#define tb_array__hdr(b) ((size_t*)(void*)(b) - 2)
#define tb_array__cap(b) tb_array__hdr(b)[0]
#define tb_array__len(b) tb_array__hdr(b)[1]

#define tb_array_len(b) ((b) ? tb_array__len(b) : 0)
#define tb_array_cap(b) ((b) ? tb_array__cap(b) : 0)

#define tb_array_resize(b, n)   (*((void**)&(b)) = tb_array__resize((b), (n), sizeof(*(b))))
#define tb_array_reserve(b, n)  (*((void**)&(b)) = tb_array__reserve((b), (n), sizeof(*(b))))
#define tb_array_grow(b, n)     (*((void**)&(b)) = tb_array__grow((b), (n), sizeof(*(b))))

#define tb_array_push(b, v) (tb_array_grow((b), 1), (b)[tb_array__len(b)++] = (v))
#define tb_array_free(b)    (tb_array_resize(b, 0))

#define tb_array_pack(b)    (tb_array_resize((b), tb_array_len(b)))
#define tb_array_clear(b)   ((b) ? tb_array__len(b) = 0 : 0)

#define tb_array_last(b)    ((b) + tb_array_len(b))
#define tb_array_sizeof(b)  (tb_array_len(b) * sizeof(*(b)))

void* tb_array__resize(void* buf, size_t new_cap, size_t elem_size);
void* tb_array__reserve(void* buf, size_t reserve, size_t elem_size);
void* tb_array__grow(void* buf, size_t increment, size_t elem_size);

#endif /* !TB_ARRAY_H */
