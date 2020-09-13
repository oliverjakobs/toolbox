#include "toolbox.h"

void tb_swap(void** a, void** b)
{
    void* t = *a;
    *a = *b;
    *b = t;
}

void tb_swap_i(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

void tb_swap_i32(int32_t* a, int32_t* b)
{
    int32_t t = *a;
    *a = *b;
    *b = t;
}

void tb_swap_i64(int64_t* a, int64_t* b)
{
    int64_t t = *a;
    *a = *b;
    *b = t;
}

void tb_swap_f(float* a, float* b)
{
    float t = *a;
    *a = *b;
    *b = t;
}

void tb_swap_str(char** a, char** b)
{
    char* t = *a;
    *a = *b;
    *b = t;
}
