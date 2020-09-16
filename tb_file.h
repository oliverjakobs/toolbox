/*
tb_jwrite v0.7 - Utilities for files
-----------------------------------------------------------------------------------------
*/

#ifndef TB_FILE_H
#define TB_FILE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TB_FILE_COPY_BUFFER_SIZE    4096

/* ----------------------| error codes |-------------------------- */
typedef enum
{
    TB_FILE_OK = 0,
    TB_FILE_OPEN_ERROR,
    TB_FILE_READ_ERROR,
    TB_FILE_WRITE_ERROR,
    TB_FILE_MEMORY_ERROR
} tb_file_error;

/*
 * Reads an '\0'-terminated string from the file specified by path.
 */
char* tb_file_read(const char* path, const char* mode, tb_file_error* err);

/*
 * Writes an '\0'-terminated string to the file specified by path.
 */
tb_file_error tb_file_write(const char* path, const char* mode, const char* data);

/*
 * Copies a file from src_path to dst_path without dynamic memory allocations.
 */
tb_file_error tb_file_copy(const char* src_path, const char* dst_path);

/*
 * Returns the size of the file 
 */
size_t tb_file_get_size(FILE* file);

/*
 * Returns an human readable description for a tb_file_error.
 */
const char* tb_file_error_to_string(tb_file_error error);

#ifdef __cplusplus
}
#endif

#endif /* !TB_FILE_INCLUDE_H */
/*
 * -----------------------------------------------------------------------------
 * ----| IMPLEMENTATION |-------------------------------------------------------
 * -----------------------------------------------------------------------------
 */

#ifdef TB_FILE_IMPLEMENTATION


char* tb_file_read(const char* path, const char* mode, tb_file_error* err)
{
    FILE* file = fopen(path, mode);

    if (!file)
    {
        if (err) *err = TB_FILE_OPEN_ERROR;
        return NULL;
    }

    if (err) *err = TB_FILE_OK;

    size_t size = tb_file_get_size(file);
    char* buffer = calloc(size + 1, 1);

    if (!buffer)
    {
        if (err) *err = TB_FILE_MEMORY_ERROR;
        fclose(file);
        return NULL;
    }

    if (fread(buffer, 1, size, file) != size)
    {
        free(buffer);
        fclose(file);
        if (err) *err = TB_FILE_READ_ERROR;

        return NULL;
    }
    fclose(file);

    return buffer;
}

tb_file_error tb_file_write(const char* path, const char* mode, const char* data)
{
    FILE* file = fopen(path, mode);

    if (!file) return TB_FILE_OPEN_ERROR;

    size_t size = strlen(data);
    if (fwrite(data, 1, size, file) != size)
        return TB_FILE_WRITE_ERROR;

    return TB_FILE_OK;
}

tb_file_error tb_file_copy(const char* src_path, const char* dst_path)
{
    char buffer[TB_FILE_COPY_BUFFER_SIZE];
    size_t size;

    FILE* src = fopen(src_path, "rb");
    FILE* dst = fopen(dst_path, "wb");

    if (!(src && dst))
    {
        fclose(src);
        fclose(dst);
        return TB_FILE_OPEN_ERROR;
    }

    while ((size = fread(buffer, 1, TB_FILE_COPY_BUFFER_SIZE, src)))
        fwrite(buffer, 1, TB_FILE_COPY_BUFFER_SIZE, dst);

    fclose(src);
    fclose(dst);
    return TB_FILE_OK;
}

size_t tb_file_get_size(FILE* file)
{
    size_t size = 0;
    size_t reset = ftell(file);

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, reset, SEEK_SET);

    return size;
}

const char* tb_file_error_to_string(tb_file_error error)
{
    switch (error)
    {
    case TB_FILE_OK:            return "Ok";
    case TB_FILE_OPEN_ERROR:    return "Failed to open file";
    case TB_FILE_READ_ERROR:    return "Failed to read file";
    case TB_FILE_WRITE_ERROR:   return "Failed to write to file";
    case TB_FILE_MEMORY_ERROR:  return "Failed to allocate memory";
    default:                    return "Unkown error";
    }
};

#endif /* !TB_FILE_IMPLEMENTATION */

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