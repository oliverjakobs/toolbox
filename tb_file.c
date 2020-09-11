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