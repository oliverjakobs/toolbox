#include "tb_algorithm.h"

void tb_swap(void** a, void** b)
{
    void* t = *a;
    *a = *b;
    *b = t;
}

void tb_swapi(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

void tb_swapi32(int32_t* a, int32_t* b)
{
    int32_t t = *a;
    *a = *b;
    *b = t;
}

void tb_swapi64(int64_t* a, int64_t* b)
{
    int64_t t = *a;
    *a = *b;
    *b = t;
}

void tb_swapf(float* a, float* b)
{
    float t = *a;
    *a = *b;
    *b = t;
}

void tb_swapd(double* a, double* b)
{
    double t = *a;
    *a = *b;
    *b = t;
}

void tb_swapstr(char** a, char** b)
{
    char* t = *a;
    *a = *b;
    *b = t;
}

int tb_clamp(int value, int min, int max)
{
    const int t = value < min ? min : value;
    return t > max ? max : t;
}

int32_t tb_clamp32(int32_t value, int32_t min, int32_t max)
{
    const int32_t t = value < min ? min : value;
    return t > max ? max : t;
}

int64_t tb_clamp64(int64_t value, int64_t min, int64_t max)
{
    const int64_t t = value < min ? min : value;
    return t > max ? max : t;
}

float tb_clampf(float value, float min, float max)
{
    const float t = value < min ? min : value;
    return t > max ? max : t;
}

double tb_clampd(double value, double min, double max)
{
    const double t = value < min ? min : value;
    return t > max ? max : t;
}

uint8_t tb_set_bit(uint8_t value, uint8_t position)     { return value | (1 << position); }
uint8_t tb_clear_bit(uint8_t value, uint8_t position)  { return value & ~(1 << position); }
uint8_t tb_flip_bit(uint8_t value, uint8_t position)   { return value ^ (1 << position); }
uint8_t tb_get_bit(uint8_t value, uint8_t position)    { return (value & (1 << position)) > 0; }

uint8_t tb_count_bits(uint8_t value)
{
    uint8_t count = 0;
    while (value)
    {
        value &= (value - 1);
        count++;
    }
    return count;
}