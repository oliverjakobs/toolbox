/*
shtool v1.0 - Command line tool to create a single-header libraries from one header and 
one source file.
-----------------------------------------------------------------------------------------

Location of header and source file, location for the resulting file, license and separator 
between header and source file are stored in config.txt.

!IMPORTANT! 
    #include "header.h" needs to be the first line of the source file

Usage:
    shtool <filename> <impl_define>

    <filename>:     base name for header, source and single-header file
    <impl_define>:  should be LIBRARYNAME_IMPLEMENTATION. Used to control creating
                    the implementation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* comment_start = "/*";
const char* comment_end = "*/";

const char* define_start = "#ifdef ";
const char* define_end = "#endif ";

size_t str_filter(char* buffer, char c)
{
    int pos = 0;
    for (size_t i = 0; buffer[i] != '\0'; ++i)
    {
        if (buffer[i] != c)
            buffer[pos++] = buffer[i];
    }
    buffer[pos] = '\0';

    return pos;
}

char* get_config(const char* buffer, const char* id, size_t* size)
{
    char* start = strstr(buffer, id) + strlen(id) + 1;
    char* end = strchr(start, '[');

    if (!end) end = strchr(start, '\0');

    if (size)
        *size = end - start - 1;

    return start;
}

char* make_path(const char* folder, const char* file, const char* ext)
{
    size_t folder_len = strcspn(folder, "\n");
    size_t file_len = strlen(file);
    size_t path_len = folder_len + file_len + strlen(ext);
    char* path = malloc(path_len + 1);

    if (!path) return NULL;
    
    path[path_len] = '\0';

    strncpy(path, folder, folder_len);
    strncpy(path + folder_len, file, file_len);
    strcpy(path + folder_len + file_len, ext);

    return path;
}

char* read_file(const char* path, size_t* size_ptr)
{
    FILE* file = fopen(path, "rb");
    
    if (!file)
    {
        printf("[ERROR] Failed to open file (%s)\n", path);
        return NULL;
    }
    
    /* find file size */
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char* buffer = malloc(size + 1);

    if (!buffer)
    {
        printf("[ERROR] Failed to allocate memory for %s\n", path);
        return NULL;
    }

    buffer[size] = '\0';

    size_t result = fread(buffer, size, 1, file);
    fclose(file);
    
    if (result != 1)
    {
        printf("[ERROR] Failed to read file (%s)\n", path);
        return NULL;
    }
    
    *size_ptr = str_filter(buffer, '\r');
    return buffer;
}


char* header;
char* source;

char* config_buffer;

void cleanup()
{
    if (header) free(header);
    if (source) free(source);

    if (config_buffer) free(config_buffer);
}

int main(int argc, char** argv)
{
    /* get args */
    if(argc != 3)
    {
        /* wrong input */
        printf("usage:\n");
        printf("  shtool <filename> <impl_define>\n");
        return 1;
    };

    char* filename = argv[1];
    char* impl_def = argv[2];

    /* read config */
    size_t config_size;
    config_buffer = read_file("config.txt", &config_size);

    if (!config_buffer) return 1;

    printf("[OUT] Reading config\n");

    size_t license_size;
    char* license = get_config(config_buffer, "[license]", &license_size);

    size_t separator_size;
    char* separator = get_config(config_buffer, "[separator]", &separator_size);

    char* src = get_config(config_buffer, "[src]", NULL);

    /* get header */
    char* h_path = make_path(src, filename, ".h");

    printf("[OUT] Reading header file from %s\n", h_path);

    size_t header_size;
    header = read_file(h_path, &header_size);

    free(h_path);

    if (!header)
    {
        cleanup();
        return 1;
    }

    /* get source */
    char* c_path = make_path(src, filename, ".c");

    printf("[OUT] Reading source file from %s\n", c_path);

    size_t source_size;
    source = read_file(c_path, &source_size);

    free(c_path);

    /* skip first line of source file to ignore include of header file */
    size_t source_offset = strcspn(source, "\n");
    source_size -= source_offset;
    
    if (!source)
    {
        cleanup();
        return 1;
    }

    /* write to target */
    char* target = get_config(config_buffer, "[target]", NULL);
    char* target_path = make_path(target, filename, ".h");
    
    printf("[OUT] Writing to %s\n", target_path);

    FILE* out = fopen(target_path, "wb");

    free(target_path);
    
    if (out)
    {
        fwrite(header, header_size, 1, out);
        fwrite("\n", 1, 1, out);
        fwrite(separator, separator_size, 1, out);
        fwrite("\n", 1, 1, out);
        fwrite(define_start, strlen(define_start), 1, out);
        fwrite(impl_def, strlen(impl_def), 1, out);
        fwrite("\n", 1, 1, out);
        fwrite(source + source_offset, source_size, 1, out);
        fwrite("\n\n", 2, 1, out);
        fwrite(define_end, strlen(define_end), 1, out);
        fwrite(comment_start, strlen(comment_start), 1, out);
        fwrite(" !", 2, 1, out);
        fwrite(impl_def, strlen(impl_def), 1, out);
        fwrite(" ", 1, 1, out);
        fwrite(comment_end, strlen(comment_end), 1, out);
        fwrite("\n\n", 2, 1, out);
        fwrite(comment_start, strlen(comment_start), 1, out);
        fwrite("\n", 1, 1, out);
        fwrite(license, license_size, 1, out);
        fwrite(comment_end, strlen(comment_end), 1, out);
        
        printf("[OUT] Done.\n");
    }
    else
    {
        printf("[ERROR] Failed to open file (%s)\n", target_path);
    }

    fclose(out);

    cleanup();

    return 0;
}