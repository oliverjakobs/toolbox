#include "tb_file.h"

#include <stdlib.h>
#include <string.h>

char* tb_file_read(const char* path, const char* mode)
{
    return tb_file_read_alloc(path, mode, malloc, free);
}

char* tb_file_read_alloc(const char* path, const char* mode, void* (*mallocf)(size_t), void (*freef)(void*))
{
    char* buffer = NULL;
    FILE* stream = fopen(path, mode);

    if (stream)
    {
        buffer = tb_file_readf_alloc(stream, mallocf, freef);
        fclose(stream);
    }

    return buffer;
}

char* tb_file_readf(FILE* const stream)
{
    return tb_file_readf_alloc(stream, malloc, free);
}

char* tb_file_readf_alloc(FILE* const stream, void* (*mallocf)(size_t), void(*freef)(void*))
{
    if (!stream) return NULL;

    /* find file size */
    fseek(stream, 0, SEEK_END);
    size_t size = ftell(stream);
    rewind(stream);

    char* buffer = mallocf(size + 1);
    if (buffer)
    {
        if (fread(buffer, size, 1, stream) != 1)
        {
            freef(buffer);
            fclose(stream);
            return NULL;
        }

        buffer[size] = '\0'; /* zero terminate buffer */
    }

    return buffer;
}

size_t tb_file_read_buf(const char* path, const char* mode, char* buf, size_t max_len)
{
    FILE* stream = fopen(path, mode);
    if (!stream) return 0;

    size_t size = fread(buf, 1, max_len - 1, stream);

    buf[size] = '\0'; /* zero terminate buffer */

    fclose(stream);
    return size + 1;
}

size_t tb_file_copy(const char* src_path, const char* dst_path)
{
    char buffer[TB_FILE_COPY_BUFFER_SIZE];

    FILE* src = fopen(src_path, "rb");
    FILE* dst = fopen(dst_path, "wb");

    size_t read, wrote = 0;
    if (src && dst)
    {
        while ((read = fread(buffer, 1, TB_FILE_COPY_BUFFER_SIZE, src)))
            wrote += fwrite(buffer, 1, read, dst);
    }

    if (src) fclose(src);
    if (dst) fclose(dst);
    return wrote;
}

size_t tb_path_join(char* dst, size_t size, const char* path1, const char* path2)
{
    if (!path1 || !path2) return 0;

    size_t len = strlen(path1);

    if (len > 0)
    {
        strncpy(dst, path1, size);

        if (path1[len - 1] != TB_PATH_SEPARATOR && path2[0] != '\0')
            dst[len++] = TB_PATH_SEPARATOR;
    }

    strncpy(dst + len, path2, size - len);
    len += strlen(path2);

    return len < size ? len : size;
}