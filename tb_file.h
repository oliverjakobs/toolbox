/*
tb_jwrite v0.6 - Utilities for files
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

/* ----------------------| error codes |-------------------------- */
typedef enum
{
    TB_FILE_OK = 0,     /* Success */
    TB_FILE_INVALID,    /* Invalid parameters */
    TB_FILE_ERROR,      /* Stream error */
    TB_FILE_OVERFLOW,   /* Too much input */
    TB_FILE_OOM         /* Out of memory */
} tb_file_error;

/*
 * Read the input in chunks of size chunk_size, dynamically reallocating the 
 * buffer as needed. Only using realloc(), fread(), ferror(), and free()
 *
 * If successful (return FILE_OK):
 *  (*dataptr) points to a dynamically allocated buffer, with
 *  (*sizeptr) bytes read from the file.
 *  The buffer is allocated for one extra char, which is '\0',
 *  and automatically appended after the data.
 */
tb_file_error tb_file_read_chunk(FILE* file, char** dataptr, size_t* sizeptr, size_t chunk_size);

/*
 * reads file into a malloc'd buffer with appended '\0' terminator
 * limits malloc() to max_size bytes
 */
tb_file_error tb_file_read_buffer(FILE* file, char** dataptr, size_t* sizeptr, size_t max_size);

tb_file_error tb_file_write(FILE* file, char* data, size_t size);

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

#include "tb_file.h"

tb_file_error tb_file_read_chunk(FILE* file, char** dataptr, size_t* sizeptr, size_t chunk_size)
{
    char* data = NULL;
    char* temp;

    size_t max_size = 0;
    size_t size = 0;

    /* None of the parameters can be NULL. */
    if (file == NULL || dataptr == NULL || sizeptr == NULL)
        return TB_FILE_INVALID;

    /* A read error already occurred? */
    if (ferror(file))
        return TB_FILE_ERROR;

    while (1)
    {
        if (size + chunk_size + 1 > max_size) 
        {
            max_size = size + chunk_size + 1;

            /* Overflow check. Some ANSI C compilers may optimize this away, though. */
            if (max_size <= size)
            {
                free(data);
                return TB_FILE_OVERFLOW;
            }

            temp = realloc(data, max_size);
            if (temp == NULL) 
            {
                free(data);
                return TB_FILE_OOM;
            }
            data = temp;
        }

        size_t elem = fread(data + size, 1, chunk_size, file);
        if (elem == 0)
            break;

        size += elem;
    }

    if (ferror(file)) 
    {
        free(data);
        return TB_FILE_ERROR;
    }

    temp = realloc(data, size + 1);
    if (temp == NULL) 
    {
        free(data);
        return TB_FILE_OOM;
    }

    data = temp;
    data[size] = '\0';

    *dataptr = data;
    *sizeptr = size;

    return TB_FILE_OK;
}

tb_file_error tb_file_read_buffer(FILE* file, char** dataptr, size_t* sizeptr, size_t max_size)
{
    char* data = NULL;
    size_t size = 0;

    /* find file size */
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    if (size >= max_size)
    {
        return TB_FILE_OVERFLOW;
    }

    data = (char*)malloc(size + 1);
    memset(data, 0, size + 1); /* +1 guarantees trailing \0 */

    if (fread(data, size, 1, file) != 1)
    {
        free(data);
        return TB_FILE_ERROR;
    }
    
    *dataptr = data;
    *sizeptr = size;

    return TB_FILE_OK;
}

tb_file_error tb_file_write(FILE* file, char* data, size_t size)
{
    if (fwrite(data, size, 1, file) != 1)
    {
        return TB_FILE_ERROR;
    }

    return TB_FILE_OK;
}

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