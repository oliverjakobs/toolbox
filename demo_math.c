#include <stdio.h>

#include "tb_math.h"

void print_vec2(vec2 v)
{
    printf("[ %f  %f ]\n", v.x, v.y);
}

void print_vec3(vec3 v)
{
    printf("[ %f  %f  %f ]\n", v.x, v.y, v.z);
}

void print_vec4(vec4 v)
{
    printf("[ %f  %f  %f  %f ]\n", v.x, v.y, v.z, v.w);
}

void print_mat4(mat4 m)
{
    for (int i = 0; i < 4; i++)
    {
        printf("[  ");
        for (int j = 0; j < 4; j++)
        {
            printf("%f  ", m.values[i][j]);
        }
        printf("]\n");
    }
}

int main()
{
    vec2 a = { 1.0f, 2.0f };
    vec2 b = { 3.0f, 5.0f };

    vec4 v = { a.x, a.y, 0.0f, 0.0f };

    //print_vec2(add_vec2(a, b));

    print_vec2(a);
    print_vec4(v);

    mat4 m = indentity();

    print_mat4(m);

    return 0;
}