#include "../src/tb_file.h"

#define FILE_BUFFER_MAX_SIZE    10485761 // 10MB
#define FILE_CHUNK_SIZE         10485761

#include <time.h>

double get_time_spent(clock_t begin, clock_t end)
{
    return (double)((end - begin) / CLOCKS_PER_SEC);
}

int main()
{
    char* filename_big = "res/10mb.txt";
    char* filename_write = "res/write.txt";
    char* filename_read = "res/test.txt";

    char* data;
    size_t size;

    FILE* file;

    if (!(file = fopen(filename_big, "rb")))
    {
        printf("Failed to open file: %s\n", filename_big);
        return 1;
    }


    // clock
    clock_t begin, end;

    // read_buffer
    begin = clock();

    size = tb_file_get_size(file);
    data = malloc(size + 1);
    if(tb_file_read_buffer(file, data, size) != TB_FILE_OK)
    {
        printf("Failed to read file: %s\n", filename_big);

        free(data);
        fclose(file);
        return 1;
    }

    end = clock();

    printf("-----------------------------------\n");
    printf("read_buffer:\n");
    printf("-----------------------------------\n");
    printf("Read %d bytes in %d ticks.\n", size, (end - begin));
    printf("-----------------------------------\n");

    // refresh
    free(data);
    rewind(file);

    // read_chunk 
    begin = clock();

    if (tb_file_read_chunk(file, &data, &size, FILE_CHUNK_SIZE) != TB_FILE_OK)
    {
        printf("Failed to read file: %s\n", filename_big);

        free(data);
        fclose(file);
        return 1;
    }

    end = clock();

    printf("-----------------------------------\n");
    printf("read_chunk:\n");
    printf("-----------------------------------\n");
    printf("Read %d bytes in %d ticks.\n", size, (end - begin));
    printf("-----------------------------------\n");

    fclose(file);
    free(data);

    /* Test writing */
    tb_file_error error;
    data = tb_file_read(filename_read, "rb", &error);
    if (!data)
    {
        printf("Failed to read file: %s (%s)\n", filename_read, tb_file_error_to_string(error));
        return 1;
    }

    // write
    error = tb_file_write(filename_write, "wb", data);
    if (error != TB_FILE_OK)
    {
        printf("Failed to write to file: %s (%s)\n", filename_write, tb_file_error_to_string(error));

        free(data);
        return 1;
    }

    printf("Successfully wrote to file: %s\n", filename_write);

    free(data);

    return 0;
}