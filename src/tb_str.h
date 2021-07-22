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
