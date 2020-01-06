#include "tiny_math.h"

#include <math.h>

// --------------------------------------------------
// vec2
// --------------------------------------------------

vec2 add_vec2(vec2 a, vec2 b)
{
    a.x += b.x;
    a.y += b.y;

    return a;
}

vec2 sub_vec2(vec2 a, vec2 b)
{
    a.x -= b.x;
    a.y -= b.y;

    return a;
}

vec2 mul_vec2(vec2 v, float s)
{
    v.x *= s;
    v.y *= s;

    return v;
}

float dot_vec2(vec2 a, vec2 b)
{
    return (a.x * b.x) + (a.y * b.y);
}

float length_vec2(vec2 v)
{
    return sqrtf(length_squared_vec2(v));
}

float length_squared_vec2(vec2 v)
{
    return (v.x * v.x) + (v.y * v.y);
}

vec2 normalize_vec2(vec2 v)
{
    float w = length_vec2(v);
    v.x /= w;
    v.y /= w;

    return v;
}

// --------------------------------------------------
// vec3
// --------------------------------------------------

vec3 add_vec3(vec3 a, vec3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;

    return a;
}

vec3 sub_vec3(vec3 a, vec3 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;

    return a;
}

vec3 mul_vec3(vec3 v, float s)
{
    v.x *= s;
    v.y *= s;
    v.z *= s;

    return v;
}

float dot_vec3(vec3 a, vec3 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

vec3 cross_vec3(vec3 a, vec3 b)
{
    vec3 c;

    c.x = (a.y * b.z) - (a.z * b.y);
    c.y = (a.z * b.x) - (a.x * b.z);
    c.z = (a.x * b.y) - (a.y * b.x);

    return c;
}

float length_vec3(vec3 v)
{
    return sqrtf(length_squared_vec3(v));
}

float length_squared_vec3(vec3 v)
{
    return (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
}

vec3 normalize_vec3(vec3 v)
{
    float w = length_vec3(v);
    v.x /= w;
    v.y /= w;
    v.z /= w;

    return v;
}

// --------------------------------------------------
// mat4
// --------------------------------------------------

mat4 indentity()
{
    mat4 m;

    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            m.values[r][c] = r == c ? 1.0f : 0.0f;
        }
    }

    return m;
}

mat4 translate(const mat4 m, vec3 v);
mat4 scale(const mat4 m, vec3 v);

float* value_ptr(const mat4);