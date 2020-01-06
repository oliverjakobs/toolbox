#ifndef TINY_MATH_H
#define TINY_MATH_H

// --------------------------------------------------
// vec2
// --------------------------------------------------

typedef struct
{
    float x, y;
} vec2;

vec2 add_vec2(vec2 a, vec2 b);
vec2 sub_vec2(vec2 a, vec2 b);

vec2 mul_vec2(vec2 v, float s);

float dot_vec2(vec2 a, vec2 b);

float length_vec2(vec2 v);
float length_squared_vec2(vec2 v);

vec2 normalize_vec2(vec2 v);

// --------------------------------------------------
// vec3
// --------------------------------------------------

typedef struct
{
    float x, y, z;
} vec3;

vec3 add_vec3(vec3 a, vec3 b);
vec3 sub_vec3(vec3 a, vec3 b);

vec3 mul_vec3(vec3 v, float s);

float dot_vec3(vec3 a, vec3 b);
vec3 cross_vec3(vec3 a, vec3 b);

float length_vec3(vec3 v);
float length_squared_vec3(vec3 v);

vec3 normalize_vec3(vec3 v);

// --------------------------------------------------
// vec4
// --------------------------------------------------

typedef struct
{
    float x, y, z, w;
} vec4;

vec4 add_vec4(vec4 a, vec4 b);
vec4 sub_vec4(vec4 a, vec4 b);

vec4 mul_vec4(vec4 v, float s);

// --------------------------------------------------
// mat4
// --------------------------------------------------

typedef struct
{
    float values[4][4];
} mat4;

mat4 indentity();

mat4 translate(const mat4 m, vec3 v);
mat4 scale(const mat4 m, vec3 v);

float* value_ptr(const mat4);

// --------------------------------------------------
// 2d primitives
// --------------------------------------------------
typedef struct
{
    vec2 start;
    vec2 end;
} line;

typedef struct
{
    vec2 position;
    vec2 dimension;
} rect;

#endif