#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/tb_array.h"

typedef struct
{
    int id;
} element;

void* element_alloc(const void* src)
{
    printf("Allocating: %d\n", ((element*)src)->id);
    size_t size = sizeof(element);
    void* block = malloc(size);

    if (block)
    {
        memcpy(block, src, size);
    }

    return block;
}

void element_free(void* block)
{
    printf("Freeing: %d\n", ((element*)block)->id);
    free(block);
}

void print_array(tb_array* arr)
{
    printf("Array: ");
    for (int i = 0; i < arr->used; ++i)
    {
        element* e = tb_array_get(arr, i);

        if (e) printf("%d, ", e->id);
    }
    printf("\n");
}

int main()
{
    tb_array arr;
    tb_array_alloc(&arr, 10, sizeof(element), 1.2f);

    printf("\nStart\n");
    printf("-------------------------------------\n");

    int input[] = { 7, 8, 5, 2, 6, 9, 1, 3, 4 };

    printf("Push:\n");
    for (int i = 0; i < 9; ++i)
    {
        element e;
        e.id = input[i];
        tb_array_push(&arr, &e);
    }

    print_array(&arr);

    printf("-------------------------------------\n");

    printf("Remove:\n");

    tb_array_remove(&arr, 3);
    tb_array_remove(&arr, 8);
    tb_array_remove(&arr, 10);

    print_array(&arr);
    printf("-------------------------------------\n");

    tb_array_free(&arr);
    printf("\nDone\n");

    return 0;
}