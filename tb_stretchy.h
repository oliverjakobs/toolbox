#ifndef TB_STRETCHY_H
#define TB_STRETCHY_H

#include <stdlib.h>

#define tb_stretchy__hdr(b) ((size_t*)(void*)(b) - 2)
#define tb_stretchy__cap(b) tb_stretchy__hdr(b)[0]
#define tb_stretchy__len(b) tb_stretchy__hdr(b)[1]

#define tb_stretchy__grow(b, n) ((b) = tb_stretchy__realloc((b), tb_stretchy_size(b) + (n), sizeof(*(b))))

#define tb_stretchy__need_space(b, n) ((b) == NULL || tb_stretchy__len(b) + (n) >= tb_stretchy__cap(b))
#define tb_stretchy__make_space(b, n) (tb_stretchy__need_space(b, (n)) ? tb_stretchy__grow(b, n) : 0)

#define tb_stretchy__push_checked(b, v) ((b) ? (b)[tb_stretchy__len(b)++] = (v) : 0)

#define tb_stretchy_push(b, v) (tb_stretchy__make_space(b, 1), tb_stretchy__push_checked(b, v))
#define tb_stretchy_size(b) ((b) ? tb_stretchy__len(b) : 0)
#define tb_stretchy_free(b) ((b) ? free(tb_stretchy__hdr(b)), 0 : 0);
#define tb_stretchy_last(b) ((b)[tb_stretchy__len(b) - 1])

static void* tb_stretchy__realloc(const void* buf, size_t new_len, size_t elem_size)
{
	size_t c = buf ? 2 * tb_stretchy__cap(buf) : 0;
	size_t new_cap = c > new_len ? c : new_len;
	size_t new_size = 2 * sizeof(size_t) + new_cap * elem_size;

	size_t* hdr = realloc(buf ? tb_stretchy__hdr(buf) : NULL, new_size);

	if (!hdr) return NULL; /* out of memory */

	hdr[0] = new_cap;
	if (!buf) hdr[1] = 0;

	return hdr + 2;
}

#endif // !TB_STRETCHY_H
