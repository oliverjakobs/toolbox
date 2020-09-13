#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <stdint.h>

void tb_swap(void** a, void** b);
void tb_swap_i(int* a, int* b);
void tb_swap_i32(int32_t* a, int32_t* b);
void tb_swap_i64(int64_t* a, int64_t* b);
void tb_swap_f(float* a, float* b);
void tb_swap_str(char** a, char** b);

inline int tb_max_i(int a, int b) { return a > b ? a : b; }
inline int32_t tb_max_i32(int32_t a, int32_t b) { return a > b ? a : b; }
inline int64_t tb_max_i64(int64_t a, int64_t b) { return a > b ? a : b; }
inline float tb_max_f(float a, float b) { return a > b ? a : b; }

inline int tb_min_i(int a, int b) { return a < b ? a : b; }
inline int32_t tb_min_i32(int32_t a, int32_t b) { return a < b ? a : b; }
inline int64_t tb_min_i64(int64_t a, int64_t b) { return a < b ? a : b; }
inline float tb_min_f(float a, float b) { return a < b ? a : b; }

inline uint8_t tb_between_i(int start, int end, int value) { return (value >= start) && (value <= end); }
inline uint8_t tb_between_i32(int start, int end, int value) { return (value >= start) && (value <= end); }
inline uint8_t tb_between_i64(int start, int end, int value) { return (value >= start) && (value <= end); }
inline uint8_t tb_between_f(int start, int end, int value) { return (value >= start) && (value <= end); }

#endif /* !TOOLBOX_H */

/*
 * -----------------------------------------------------------------------------
 * ----| IMPLEMENTATION |-------------------------------------------------------
 * -----------------------------------------------------------------------------
 */

#ifdef TOOLBOX_IMPLEMENTATION

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


#endif /* !TOOLBOX_IMPLEMENTATION */

/*
MIT License

Copyright (c) 2020 oliverjakobs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/