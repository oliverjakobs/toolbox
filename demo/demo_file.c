#include "../src/tb_file.h"


int main()
{
    char* filename_write = "res/write.txt";
    char* filename_read = "res/test.txt";

    char* data;

    tb_file_error error;
    data = tb_file_read(filename_read, "rb", &error);
    if (!data)
    {
        printf("Failed to read file: %s (%s)\n", filename_read, tb_file_error_to_string(error));
        return 1;
    }

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