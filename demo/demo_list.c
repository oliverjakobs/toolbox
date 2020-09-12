#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../tb_list.h"

/* ----------------------------------------------------------- */

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

int element_cmp(const void* left, const void* right)
{
    return ((element*)left)->id - ((element*)right)->id;
}

void print_list(tb_list* list)
{
    printf("List: ");
    for (int i = 0; i < list->used; ++i)
    {
        element* e = tb_list_get(list, i);

        if (e) printf("%d, ", e->id);
    }
    printf("\n");
}

int main()
{
    tb_list list;
    tb_list_alloc(&list, 10, sizeof(element), element_cmp);

    printf("\nStart\n");
    printf("-------------------------------------\n");

    int input[] = { 7, 8, 5, 2, 6, 9, 1, 3, 4 };

    printf("Insert:\n");
    for (int i = 0; i < 9; ++i)
    {
        element e;
        e.id = input[i];
        tb_list_insert(&list, &e);
    }

    print_list(&list);

    printf("-------------------------------------\n");
    printf("Insert duplicate:\n");
    element e;
    e.id = 2;
    if (!tb_list_insert(&list, &e))
        printf("Tried to insert duplicate (%d)\n", e.id);

    print_list(&list);
    
    printf("-------------------------------------\n");
    printf("Insert and grow:\n");
    int new_input[] = { 14, 11, 10 };
    
    for (int i = 0; i < 3; ++i)
    {
        element e;
        e.id = new_input[i];
        tb_list_insert_and_grow(&list, &e, 1.2f);
    }
    print_list(&list);

    printf("-------------------------------------\n");

    printf("Remove at:\n");

    tb_list_remove_at(&list, 3);
    tb_list_remove_at(&list, 8);
    tb_list_remove_at(&list, 10);

    print_list(&list);
    printf("-------------------------------------\n");

    printf("Remove:\n");

    e.id = 4;
    tb_list_remove(&list, &e);
    e.id = 6;
    tb_list_remove(&list, &e);

    print_list(&list);
    printf("-------------------------------------\n");

    int find[] = { 4, 7, 8 };
    printf("Find:\n");
    for (int i = 0; i < 3; ++i)
    {
        element e;
        e.id = find[i];
        if (tb_list_find(&list, &e))
            printf("Found: %d\n", find[i]);
    }
    printf("-------------------------------------\n");

    printf("Find index:\n");
    for (int i = 0; i < 3; ++i)
    {
        element e;
        e.id = find[i];
        size_t index = tb_list_find_index(&list, &e);
        printf("Found: %d at %d\n", find[i], (index < list.used) ? index : -1);
    }

    tb_list_clear(&list);
    printf("-------------------------------------\n");

    tb_list_free(&list);
    printf("\nDone\n");

    return 0;
}