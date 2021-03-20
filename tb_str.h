#ifndef TB_STRING_H
#define TB_STRING_H

#include <string.h>

char tb_tolower(unsigned char c);

int tb_strcasecmp(const char* str1, const char* str2);
int tb_strncasecmp(const char* str1, const char* str2, size_t max_count);

size_t tb_strlcpy(char* dst, const char* src, size_t size);

char* tb_strdup(const char* src);
char* tb_strndup(const char* src, size_t max_len);

char* tb_strsep(char** str_ptr, const char* sep);

#endif /* !TB_STRING_H */

/*
 * -----------------------------------------------------------------------------
 * ----| IMPLEMENTATION |-------------------------------------------------------
 * -----------------------------------------------------------------------------
 */

#ifdef TB_STR_IMPLEMENTATION


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


#endif /* !TB_STR_IMPLEMENTATION */

/*
MIT License

Copyright (c) 2020 oliverjakobs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/