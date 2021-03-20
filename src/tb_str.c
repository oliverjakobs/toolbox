#include "tb_str.h"

#include <stdlib.h>

char tb_tolower(unsigned char c)
{
    return (c >= 'A' && c <= 'Z') ? 'a' + (c - 'A') : c;
}

int tb_strcasecmp(const char* str1, const char* str2)
{
    const unsigned char* us1 = (const unsigned char*)str1;
    const unsigned char* us2 = (const unsigned char*)str2;

    while (tb_tolower(*us1) == tb_tolower(*us2++))
        if (*us1++ == '\0') return 0;

    return (tb_tolower(*us1) - tb_tolower(*--us2));
}

int tb_strncasecmp(const char* str1, const char* str2, size_t max_count)
{
	if (max_count != 0)
	{
		const unsigned char* us1 = (const unsigned char*)str1;
		const unsigned char* us2 = (const unsigned char*)str2;

		do
		{
			if (tb_tolower(*us1) != tb_tolower(*us2++))
				return tb_tolower(*us1) - tb_tolower(*--us2);

			if (*us1++ == '\0') break;
		} while (--max_count != 0);
	}
	return 0;
}

size_t tb_strlcpy(char* dst, const char* src, size_t size)
{
	size_t ret = strlen(src);

	if (size)
	{
		size_t len = (ret >= size) ? size - 1 : ret;
		memcpy(dst, src, len);
		dst[len] = '\0';
	}
	return ret;
}

char* tb_strdup(const char* src)
{
    size_t len = strlen(src) + 1;
    char* dst = malloc(len);

    return dst ? memcpy(dst, src, len) : NULL;
}

char* tb_strndup(const char* src, size_t max_len)
{
	return NULL;
}

char* tb_strsep(char** str_ptr, const char* sep)
{
	char* begin = *str_ptr;
	if (!begin) return NULL;

	char* end = begin + strcspn(begin, sep);
	if (*end) *end++ = '\0';
	*str_ptr = end;
	return begin;
}
