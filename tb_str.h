#ifndef TB_STRING_H
#define TB_STRING_H

#include <string.h>

int tb_strcasecmp(const char* str1, const char* str2);
int tb_strncasecmp(const char* str1, const char* str2, size_t max_count);

int tb_streq(const char* str1, const char* str2);
int tb_strcaseeq(const char* str1, const char* str2);

size_t tb_strlcpy(char* dst, const char* src, size_t size);

char* tb_strdup(const char* src);
char* tb_strndup(const char* src, size_t max_len);

char* tb_strsep(char** str_ptr, const char* sep);

char tb_tolower(char c);
char tb_toupper(char c);

/* Fill buf with binary representation of value. Make sure buf is big enough. */
char* tb_bitstr(char* buf, char value);

#endif /* !TB_STRING_H */

/*
 * -----------------------------------------------------------------------------
 * ----| IMPLEMENTATION |-------------------------------------------------------
 * -----------------------------------------------------------------------------
 */

#ifdef TB_STR_IMPLEMENTATION


#include <stdlib.h>

int tb_strcasecmp(const char* str1, const char* str2)
{
    while (tb_tolower(*str1) == tb_tolower(*str2++))
        if (*str1++ == '\0') return 0;

    return (tb_tolower(*str1) - tb_tolower(*--str2));
}

int tb_strncasecmp(const char* str1, const char* str2, size_t max_count)
{
    while (max_count-- != 0)
    {
        if (tb_tolower(*str1) != tb_tolower(*str2++))
            return tb_tolower(*str1) - tb_tolower(*--str2);

        if (*str1++ == '\0') break;
    }
    return 0;
}

int tb_streq(const char* str1, const char* str2)
{
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);

    return len1 == len2 ? strncmp(str1, str2, len1) == 0 : 0;
}

int tb_strcaseeq(const char* str1, const char* str2)
{
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);

    return len1 == len2 ? tb_strncasecmp(str1, str2, len1) == 0 : 0;
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

char* tb_strdup(const char* src) { return tb_strndup(src, strlen(src)); }

char* tb_strndup(const char* src, size_t max_len)
{
    char* dst = malloc(max_len + 1);
    return dst ? memcpy(dst, src, max_len + 1) : NULL;
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

char tb_tolower(char c) { return (c >= 'A' && c <= 'Z') ? 'a' + (c - 'A') : c; }
char tb_toupper(char c) { return (c >= 'a' && c <= 'z') ? 'A' + (c - 'a') : c; }

char* tb_bitstr(char* buf, char value)
{
    char bit = 0;
    char bits = sizeof(char) * 8;

    for (char i = 1 << (bits - 1); i > 0; i = i / 2)
        buf[bit++] = (value & i) ? '1' : '0'; 

    buf[bit] = '\0';

    return buf;
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