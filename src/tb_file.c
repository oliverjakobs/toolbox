#include "tb_file.h"

tb_file_error tb_file_read_chunk(FILE* file, char** dataptr, size_t* sizeptr, size_t chunk_size)
{
    char* data = NULL;
    char* temp;

    size_t max_size = 0;
    size_t size = 0;

    /* None of the parameters can be NULL. */
    if (!(file && dataptr && sizeptr))
        return TB_FILE_INVALID;

    /* A read error already occurred? */
    if (ferror(file)) return TB_FILE_READ_ERROR;

    while (1)
    {
        if (size + chunk_size + 1 > max_size) 
        {
            max_size = size + chunk_size + 1;

            /* Overflow check. Some ANSI C compilers may optimize this away, though. */
            if (max_size <= size)
            {
                free(data);
                return TB_FILE_MEMORY_ERROR;
            }

            temp = realloc(data, max_size);
            if (!temp)
            {
                free(data);
                return TB_FILE_MEMORY_ERROR;
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
        return TB_FILE_READ_ERROR;
    }

    temp = realloc(data, size + 1);
    if (!temp)
    {
        free(data);
        return TB_FILE_MEMORY_ERROR;
    }

    data = temp;
    data[size] = '\0';

    *dataptr = data;
    *sizeptr = size;

    return TB_FILE_OK;
}

tb_file_error tb_file_read_buffer(FILE* file, char* buffer, size_t size)
{
    if (fread(buffer, 1, size, file) != size)
        return TB_FILE_READ_ERROR;

    buffer[size] = '\0';

    return TB_FILE_OK;
}

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

    while (size = fread(buffer, 1, TB_FILE_COPY_BUFFER_SIZE, src))
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
    case TB_FILE_OK:            return "TB_FILE_OK";
    case TB_FILE_INVALID:       return "TB_FILE_INVALID";
    case TB_FILE_OPEN_ERROR:    return "TB_FILE_OPEN_ERROR";
    case TB_FILE_READ_ERROR:    return "TB_FILE_READ_ERROR";
    case TB_FILE_WRITE_ERROR:   return "TB_FILE_WRITE_ERROR";
    case TB_FILE_MEMORY_ERROR:  return "TB_FILE_MEMORY_ERROR";
    default:                    return "UNKOWN_ERROR";
    }
};