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
------------------------------------------------------------------------------
This software is available under the MIT License.
------------------------------------------------------------------------------
Copyright (c) 2020 Oliver Jakobs

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
------------------------------------------------------------------------------
*/