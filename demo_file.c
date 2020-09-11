#include "tb_file.h"

#define FILE_BUFFER_MAX_SIZE    10485761 // 10MB
#define FILE_CHUNK_SIZE         10485761

#include <time.h>

double get_time_spent(clock_t begin, clock_t end)
{
    return (double)((end - begin) / CLOCKS_PER_SEC);
}

int main()
{
    char* filename_r = "10mb.txt";
    char* filename_w = "write.txt";
    FILE* file_r;
    FILE* file_w;

    if ((file_r = fopen(filename_r, "rb")) == NULL)
    {
        printf("Failed to open file: %s\n", filename_r);
        return 1;
    }

    if ((file_w = fopen(filename_w, "wb")) == NULL)
    {
        printf("Failed to open file: %s\n", filename_w);
        return 1;
    }

    char* data;
    size_t size;
    int status = TB_FILE_OK;

    // clock
    clock_t begin, end;

    // read_buffer
    begin = clock();

    status = tb_file_read_buffer(file_r, &data, &size, FILE_BUFFER_MAX_SIZE);
    if(status != TB_FILE_OK)
    {
        printf("Failed to read file: %s\n", filename_r);

        free(data);
        fclose(file_r);
        fclose(file_w);

        return status;
    }

    end = clock();

    printf("-----------------------------------\n");
    printf("read_buffer:\n");
    printf("-----------------------------------\n");
    printf("Read %d bytes in %d ticks.\n", size, (end - begin));
    printf("-----------------------------------\n");

    // refresh
    free(data);
    rewind(file_r);

    // read_chunk
    begin = clock();

    status = tb_file_read_chunk(file_r, &data, &size, FILE_CHUNK_SIZE);
    if (status != TB_FILE_OK)
    {
        printf("Failed to read file: %s\n", filename_r);

        free(data);
        fclose(file_r);
        fclose(file_w);

        return status;
    }

    end = clock();

    printf("-----------------------------------\n");
    printf("read_chunk:\n");
    printf("-----------------------------------\n");
    printf("Read %d bytes in %d ticks.\n", size, (end - begin));
    printf("-----------------------------------\n");

    fclose(file_r);

    /*
    // write
    status = tbf_write(file_w, data, size);
    if (status != TB_FILE_OK)
    {
        printf("Failed to write to file: %s\n", filename_w);

        free(data);
        fclose(file_w);

        return status;
    }

    printf("Successfully wrote to file: %s\n", filename_w);

    */

    fclose(file_w);
    free(data);

    return status;
}