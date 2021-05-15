#include "../src/tb_array.h"

#include <stdio.h>

typedef struct
{
    int id;
} Element;

int main()
{
    Element* array = NULL;

    Element e;

    for (int i = 0; i < 10; ++i)
    {
        e.id = i;
        tb_array_push(array, e);
    }


    for (int i = 0; i < tb_array_len(array); ++i)
    {
        printf("ID: %d\n", array[i].id);
    }

    tb_array_free(array);

    return 0;
}