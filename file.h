#ifndef FILE_INCLUDE_H
#define FILE_INCLUDE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


#define  FILE_OK         0  // Success
#define  FILE_INVALID   -1  // Invalid parameters
#define  FILE_ERROR     -2  // Stream error
#define  FILE_OVERFLOW  -3  // Too much input
#define  FILE_OOM       -4  // Out of memory

/*
Read the input in chunks of size chunk_size, dynamically reallocating the 
buffer as needed. Only using realloc(), fread(), ferror(), and free()

If successful (return FILE_OK):
    (*dataptr) points to a dynamically allocated buffer, with
    (*sizeptr) bytes read from the file.
    The buffer is allocated for one extra char, which is '\0',
    and automatically appended after the data.

Taken from: https://stackoverflow.com/a/44894946
*/
int read_chunk(FILE* file, char** dataptr, size_t* sizeptr, size_t chunk_size);

/*
reads file into a malloc'd buffer with appended '\0' terminator
limits malloc() to max_size bytes
*/
int read_buffer(FILE* file, char** dataptr, size_t* sizeptr, size_t max_size);

#ifdef __cplusplus
}
#endif

#endif // FILE_INCLUDE_H

// -----------------------------------------------------------------------------
// ----| IMPLEMENTATION |-------------------------------------------------------
// -----------------------------------------------------------------------------

#ifdef FILE_IMPLEMENTATION

// ----| read_chunk |-----------------------------------------------------------

int read_chunk(FILE* file, char** dataptr, size_t* sizeptr, size_t chunk_size)
{
    char* data = NULL;
    char* temp;

    size_t max_size = 0;
    size_t size = 0;

    // None of the parameters can be NULL.
    if (file == NULL || dataptr == NULL || sizeptr == NULL)
        return FILE_INVALID;

    // A read error already occurred?
    if (ferror(file))
        return FILE_ERROR;

    while (1)
    {
        if (size + chunk_size + 1 > max_size) 
        {
            max_size = size + chunk_size + 1;

            // Overflow check. Some ANSI C compilers
            // may optimize this away, though.
            if (max_size <= size)
            {
                free(data);
                return FILE_OVERFLOW;
            }

            temp = realloc(data, max_size);
            if (temp == NULL) 
            {
                free(data);
                return FILE_OOM;
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
        return FILE_ERROR;
    }

    temp = realloc(data, size + 1);
    if (temp == NULL) 
    {
        free(data);
        return FILE_OOM;
    }

    data = temp;
    data[size] = '\0';

    *dataptr = data;
    *sizeptr = size;

    return FILE_OK;
}

// ----| file_buffer |----------------------------------------------------------

int read_buffer(FILE* file, char** dataptr, size_t* sizeptr, size_t max_size)
{
    char* data = NULL;
    size_t size = 0;

    // find file size and allocate buffer for file
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    if (size >= max_size)
    {
        return FILE_OVERFLOW;
    }

    // rewind and read file
    fseek(file, 0, SEEK_SET);
    data = (char*)malloc(size + 1);
    memset(data, 0, size + 1); // +1 guarantees trailing \0

    if (fread(data, size, 1, file) != 1)
    {
        free(data);
        return FILE_ERROR;
    }
    
    *dataptr = data;
    *sizeptr = size;

    return FILE_OK;
}

#endif // FILE_IMPLEMENTATION

/*
------------------------------------------------------------------------------
This software is available under the MIT License.
------------------------------------------------------------------------------
Copyright (c) 2019 Oliver Jakobs

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