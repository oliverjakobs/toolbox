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