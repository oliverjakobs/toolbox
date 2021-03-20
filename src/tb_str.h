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
