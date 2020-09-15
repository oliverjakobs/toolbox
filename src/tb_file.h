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

#define TB_FILE_COPY_BUFFER_SIZE    4096

/* ----------------------| error codes |-------------------------- */
typedef enum
{
    TB_FILE_OK = 0,
    TB_FILE_INVALID,        /* Invalid parameters */
    TB_FILE_OPEN_ERROR,     /* Stream error */
    TB_FILE_READ_ERROR,
    TB_FILE_WRITE_ERROR,
    TB_FILE_MEMORY_ERROR
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


tb_file_error tb_file_read_buffer(FILE* file, char* buffer, size_t size);

/*
 * Reads an '\0'-terminated string from the file specified by path.
 */
char* tb_file_read(const char* path, const char* mode, tb_file_error* err);

/*
 * Writes an '\0'-terminated string to the file specified by path.
 */
tb_file_error tb_file_write(const char* path, const char* mode, const char* data);

/*
 * Copies a file from src_path to dst_path.
 */
tb_file_error tb_file_copy(const char* src_path, const char* dst_path);

/*
 * Returns the size of the file 
 */
size_t tb_file_get_size(FILE* file);

const char* tb_file_error_to_string(tb_file_error error);

#ifdef __cplusplus
}
#endif

#endif /* !TB_FILE_INCLUDE_H */