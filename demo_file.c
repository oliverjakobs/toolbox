
#define FILE_IMPLEMENTATION
#include "file.h"

#define FILE_BUFFER_MAX_SIZE    1024 * 1024
#define FILE_CHUNK_SIZE         1024 * 256

int main()
{
    char* filename = "README.md";
    FILE* file;

    if ((file = fopen(filename, "rb")) == NULL)
    {
        printf("Failed to open file: %s\n", filename);
        return 1;
    }
    
    char* data;
    size_t size;
    int status = FILE_OK;

    // read_buffer
    status = read_buffer(file, &data, &size, FILE_BUFFER_MAX_SIZE);
    if(status != FILE_OK)
    {
        printf("Failed to read file: %s\n", filename);
        return status;
    }

    printf("-----------------------------------\n");
    printf("read_buffer:\n");
    printf("-----------------------------------\n");
    printf("size: %d bytes\n", size);
    printf("-----------------------------------\n");
    printf("%s", data);
    printf("-----------------------------------\n");
    printf("\n");

    // refresh
    free(data);
    rewind(file);

    // read_chunk
    status = read_chunk(file, &data, &size, FILE_CHUNK_SIZE);
    if (status != FILE_OK)
    {
        printf("Failed to read file: %s\n", filename);
        return status;
    }

    printf("-----------------------------------\n");
    printf("read_chunk:\n");
    printf("-----------------------------------\n");
    printf("size: %d bytes\n", size);
    printf("-----------------------------------\n");
    printf("%s", data);
    printf("-----------------------------------\n");
    printf("\n");

    return 0;
}