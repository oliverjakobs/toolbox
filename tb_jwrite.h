/*
tb_jwrite v1.0 - a *really* simple JSON writer
-----------------------------------------------------------------------------------------

A collection of functions to generate JSON semi-automatically.
The idea is to simplify writing native C values into a JSON string and to provide some 
error trapping to ensure that the result is valid JSON.

Example:
    
    tb_jwrite_control jwc;

    tb_jwrite_open(&jwc, buffer, buflen, TB_JWRITE_OBJECT, 1);
    tb_jwrite_obj_string(&jwc, "key", "value");
    tb_jwrite_obj_int(&jwc, "int", 1);
    tb_jwrite_obj_array(&jwc, "anArray");
        tb_jwrite_array_int(&jwc, 0);
        tb_jwrite_array_int(&jwc, 1);
        tb_jwrite_array_int(&jwc, 2);
    tb_jwrite_end(&jwc);
    tb_jwrite_error err = tb_jwrite_close(&jwc);

Results in:
    {
        "key": "value",
        "int": 1,
        "anArray": [ 0, 1, 2 ]
    }

Note that tb_jwrite handles string quoting and getting commas in the right place.

If the sequence of calls is incorrect

for example:
    tb_jwrite_open(buffer, buflen, TB_JWRITE_OBJECT, 1);
    tb_jwrite_obj_string("key", "value");
    tb_jwrite_array_int(0);
    ...

then the error code returned from tb_jwrite_close() would indicate that you attempted to
put an array element into an object (instead of a key:value pair)

To locate the error, the supplied buffer has the JSON created upto the error point
and a call to jwErrorPos() would return the function call at which the error occurred
 - in this case 3, the 3rd function call "tb_jwrite_array_int(0)" is not correct at this point.

After an error, all following tb_jwrite calls are skipped internally so the error code is 
for the first error detected.

*/


#ifndef TB_JWRITE_H
#define TB_JWRITE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdint.h>

#define TB_JWRITE_STACK_DEPTH   32   /* max nesting depth of objects/arrays */
#define TB_JWRITE_TMP_BUF_SIZE  32   /* max size of the buffer used for ftoa and itoa */

/* Output string style settings for tb_jwrite_open() */
typedef enum
{
    TB_JWRITE_COMPACT,
    TB_JWRITE_INLINE,
    TB_JWRITE_NEWLINE
} tb_jwrite_style;

/* ----------------------| error codes |-------------------------- */
typedef enum
{
    TB_JWRITE_OK,
    TB_JWRITE_FILE_ERROR,
    TB_JWRITE_NOT_ARRAY,    /* tried to write Array value into Object */
    TB_JWRITE_NOT_OBJECT,   /* tried to write Object key/value into Array */
    TB_JWRITE_STACK_FULL,   /* array/object nesting > TB_JWRITE_STACK_DEPTH */
    TB_JWRITE_STACK_EMPTY,  /* stack underflow error (too many 'end's) */
    TB_JWRITE_NEST_ERROR,   /* nesting error, not all objects closed when tb_jwrite_close() called */
    TB_JWRITE_WRITE_ERROR
} tb_jwrite_error;

typedef enum
{
    TB_JWRITE_OBJECT,
    TB_JWRITE_ARRAY
} tb_jwrite_node_type;

typedef struct
{
    tb_jwrite_node_type type;
    int element;
} tb_jwrite_node;

typedef struct
{
    FILE* file;
    char tmp_buf[TB_JWRITE_TMP_BUF_SIZE];           /* local buffer for int/double convertions */
    tb_jwrite_error error;                          /* error code */
    int call;                                       /* call on which error occurred */
    tb_jwrite_node nodes[TB_JWRITE_STACK_DEPTH];    /* stack of array/object nodes */
    int stack_pos;
    tb_jwrite_style style;
    int float_prec;
} tb_jwrite_control;

/*
 * Initialises tb_jwrite_control with the application supplied taget path
 * root_type is the base JSON type: TB_JWRITE_OBJECT or TB_JWRITE_ARRAY
 * style controls 'prettifying' the output: TB_JWRITE_PRETTY or TB_JWRITE_COMPACT
 * returns TB_JWRITE_OK on success or error code
 */ 
tb_jwrite_error tb_jwrite_open(tb_jwrite_control* jwc, const char* target, tb_jwrite_node_type root_type, tb_jwrite_style style);

/*
 * Closes the tb_jwrite_control opened by tb_jwrite_open()
 * returns TB_JWRITE_OK on success or error code
 */
tb_jwrite_error tb_jwrite_close(tb_jwrite_control* jwc);

void tb_jwrite_set_style(tb_jwrite_control* jwc, tb_jwrite_style style);
void tb_jwrite_set_float_prec(tb_jwrite_control* jwc, int prec);

/* 
 * Object insertion functions
 * insert "key":"value" pairs into an object
 */
void tb_jwrite_string(tb_jwrite_control* jwc, const char* key, const char* value);
void tb_jwrite_int(tb_jwrite_control* jwc, const char* key, int32_t value);
void tb_jwrite_float(tb_jwrite_control* jwc, const char* key, float value);
void tb_jwrite_null(tb_jwrite_control* jwc, const char* key);
/* put another object into the current object */
void tb_jwrite_object(tb_jwrite_control* jwc, const char* key);
void tb_jwrite_array(tb_jwrite_control* jwc, const char* key);
/* Write the JSON value as the contents of rawtext (enclosing quotes are not added) */
void tb_jwrite_raw(tb_jwrite_control* jwc, const char* key, const char* rawtext);

/* 
 * Array insertion functions
 * insert "value" elements into an array
 */
void tb_jwrite_array_string(tb_jwrite_control* jwc, const char* value);
void tb_jwrite_array_int(tb_jwrite_control* jwc, int32_t value);
void tb_jwrite_array_float(tb_jwrite_control* jwc, float value);
void tb_jwrite_array_null(tb_jwrite_control* jwc);
void tb_jwrite_array_object(tb_jwrite_control* jwc);
/* put another array into the current array */
void tb_jwrite_array_array(tb_jwrite_control* jwc);
/* Write the JSON value as the contents of rawtext (enclosing quotes are not added) */
void tb_jwrite_array_raw(tb_jwrite_control* jwc, const char* rawtext);

/* defines the end of an object or array definition */
tb_jwrite_error tb_jwrite_end(tb_jwrite_control* jwc);

/* 
 * If tb_jwrite_close returned an error, this function returns the number of the jwrite function call
 * which caused that error.
 */
int tb_jwrite_error_pos(tb_jwrite_control* jwc);

/* Returns '\0'-termianted string describing the error code */
char* tb_jwrite_error_string(tb_jwrite_error err);

#ifdef __cplusplus
}
#endif

#endif /* !TB_JWRITE_H */
/*
 * -----------------------------------------------------------------------------
 * ----| IMPLEMENTATION |-------------------------------------------------------
 * -----------------------------------------------------------------------------
 */

#ifdef TB_JWRITE_IMPLEMENTATION

#include "tb_jwrite.h"

#include <stddef.h>
#include <string.h>

static char* _tb_jwrite_itoa(char* buffer, int32_t value);
static void _tb_jwrite_ftoa(char* buffer, float value, int precision);

static void _tb_jwrite_put_ch(tb_jwrite_control* jwc, char c)
{
    if (fprintf(jwc->file, "%c", c) < 0)
        jwc->error = TB_JWRITE_WRITE_ERROR;
}

/* put string enclosed in quotes */
static void _tb_jwrite_put_str(tb_jwrite_control* jwc, const char* str)
{
    if (fprintf(jwc->file, "\"%s\"", str) < 0)
        jwc->error = TB_JWRITE_WRITE_ERROR;
}

/* put raw string */
static void _tb_jwrite_put_raw(tb_jwrite_control* jwc, const char* str)
{
    if (fprintf(jwc->file, "%s", str) < 0)
        jwc->error = TB_JWRITE_WRITE_ERROR;
}

static void _tb_jwrite_style(tb_jwrite_control* jwc)
{
    if (jwc->style == TB_JWRITE_NEWLINE)
    {
        _tb_jwrite_put_ch(jwc, '\n');
        for (int i = 0; i < jwc->stack_pos + 1; i++)
            _tb_jwrite_put_raw(jwc, "    ");
    }
    else if (jwc->style == TB_JWRITE_INLINE)
    {
        _tb_jwrite_put_ch(jwc, ' ');
    }
}

/* Push / Pop node stack */
static void _tb_jwrite_push(tb_jwrite_control* jwc, tb_jwrite_node_type node_type)
{
    if ((jwc->stack_pos + 1) >= TB_JWRITE_STACK_DEPTH)
    {
        jwc->error = TB_JWRITE_STACK_FULL; /* array/object nesting > TB_JWRITE_STACK_DEPTH */
    }
    else
    {
        jwc->nodes[++jwc->stack_pos].type = node_type;
        jwc->nodes[jwc->stack_pos].element = 0;
    }
}

static tb_jwrite_node_type _tb_jwrite_pop(tb_jwrite_control* jwc)
{
    tb_jwrite_node_type node = jwc->nodes[jwc->stack_pos].type;
    if (jwc->stack_pos == 0)
        jwc->error = TB_JWRITE_STACK_EMPTY; /* stack underflow error (too many 'end's) */
    else
        jwc->stack_pos--;
    return node;
}

tb_jwrite_error tb_jwrite_open(tb_jwrite_control* jwc, const char* target, tb_jwrite_node_type root_type, tb_jwrite_style style)
{
    jwc->file = fopen(target, "w");

    if (!jwc->file) return TB_JWRITE_FILE_ERROR;

    jwc->nodes[0].type = root_type;
    jwc->nodes[0].element = 0;
    jwc->stack_pos = 0;
    jwc->error = TB_JWRITE_OK;
    jwc->call = 1;
    jwc->style = style;
    jwc->float_prec = 6;

    _tb_jwrite_put_ch(jwc, (root_type == TB_JWRITE_OBJECT) ? '{' : '[');

    return TB_JWRITE_OK;
}

tb_jwrite_error tb_jwrite_close(tb_jwrite_control* jwc)
{
    if (jwc->error == TB_JWRITE_OK)
    {
        if (jwc->stack_pos == 0)
        {
            tb_jwrite_node_type node = jwc->nodes[0].type;
            if (jwc->style == TB_JWRITE_NEWLINE)
                _tb_jwrite_put_ch(jwc, '\n');

            _tb_jwrite_put_ch(jwc, (node == TB_JWRITE_OBJECT) ? '}' : ']');
        }
        else
        {
            /* nesting error, not all objects closed when tb_jwrite_close() called */
            jwc->error = TB_JWRITE_NEST_ERROR;
        }
    }
    fclose(jwc->file);
    return jwc->error;
}

void tb_jwrite_set_style(tb_jwrite_control* jwc, tb_jwrite_style style)
{
    jwc->style = style;
    jwc->call++;
}

void tb_jwrite_set_float_prec(tb_jwrite_control* jwc, int prec)
{
    jwc->float_prec = prec;
    jwc->call++;
}

tb_jwrite_error tb_jwrite_end(tb_jwrite_control* jwc)
{
    if (jwc->error == TB_JWRITE_OK)
    {
        int last_element = jwc->nodes[jwc->stack_pos].element;
        tb_jwrite_node_type node = _tb_jwrite_pop(jwc);
        if (last_element > 0)
            _tb_jwrite_style(jwc);
        _tb_jwrite_put_ch(jwc, (node == TB_JWRITE_OBJECT) ? '}' : ']');
    }
    return jwc->error;
}


/* ----------------------| object |------------------------------- */

/*
 * common object function:
 *  - checks error
 *  - checks current node is OBJECT
 *  - adds comma if required
 *  - adds "key" :
 */
static tb_jwrite_error _tb_jwrite_object(tb_jwrite_control* jwc, const char* key)
{
    if (jwc->error == TB_JWRITE_OK)
    {
        jwc->call++;
        if (jwc->nodes[jwc->stack_pos].type != TB_JWRITE_OBJECT)
            jwc->error = TB_JWRITE_NOT_OBJECT; /* tried to write object key/value into array */
        else if (jwc->nodes[jwc->stack_pos].element++ > 0)
            _tb_jwrite_put_ch(jwc, ',');
        _tb_jwrite_style(jwc);
        _tb_jwrite_put_str(jwc, key);
        _tb_jwrite_put_ch(jwc, ':');
        if (jwc->style == TB_JWRITE_NEWLINE)
            _tb_jwrite_put_ch(jwc, ' ');
    }
    return jwc->error;
}

void tb_jwrite_raw(tb_jwrite_control* jwc, const char* key, const char* rawtext)
{
    if (_tb_jwrite_object(jwc, key) == TB_JWRITE_OK)
        _tb_jwrite_put_raw(jwc, rawtext);
}

void tb_jwrite_string(tb_jwrite_control* jwc, const char* key, const char* value)
{
    if (_tb_jwrite_object(jwc, key) == TB_JWRITE_OK)
        _tb_jwrite_put_str(jwc, value);
}

void tb_jwrite_int(tb_jwrite_control* jwc, const char* key, int32_t value)
{
    _tb_jwrite_itoa(jwc->tmp_buf, value);
    tb_jwrite_raw(jwc, key, jwc->tmp_buf);
}

void tb_jwrite_float(tb_jwrite_control* jwc, const char* key, float value)
{
    _tb_jwrite_ftoa(jwc->tmp_buf, value, jwc->float_prec);
    tb_jwrite_raw(jwc, key, jwc->tmp_buf);
}

void tb_jwrite_null(tb_jwrite_control* jwc, const char* key)
{
    tb_jwrite_raw(jwc, key, "null");
}

void tb_jwrite_object(tb_jwrite_control* jwc, const char* key)
{
    if (_tb_jwrite_object(jwc, key) == TB_JWRITE_OK)
    {
        _tb_jwrite_put_ch(jwc, '{');
        _tb_jwrite_push(jwc, TB_JWRITE_OBJECT);
    }
}

void tb_jwrite_array(tb_jwrite_control* jwc, const char* key)
{
    if (_tb_jwrite_object(jwc, key) == TB_JWRITE_OK)
    {
        _tb_jwrite_put_ch(jwc, '[');
        _tb_jwrite_push(jwc, TB_JWRITE_ARRAY);
    }
}

/* ----------------------| array |-------------------------------- */

/*
 * common array function
 *  - checks error
 *  - checks current node is ARRAY
 *  - adds comma if required
 */
tb_jwrite_error _tb_jwrite_array(tb_jwrite_control* jwc)
{
    if (jwc->error == TB_JWRITE_OK)
    {
        jwc->call++;
        if (jwc->nodes[jwc->stack_pos].type != TB_JWRITE_ARRAY)
            jwc->error = TB_JWRITE_NOT_ARRAY; /* tried to write array value into Object */
        else if (jwc->nodes[jwc->stack_pos].element++ > 0)
            _tb_jwrite_put_ch(jwc, ',');
        _tb_jwrite_style(jwc);
    }
    return jwc->error;
}

void tb_jwrite_array_raw(tb_jwrite_control* jwc, const char* rawtext)
{
    if (_tb_jwrite_array(jwc) == TB_JWRITE_OK)
        _tb_jwrite_put_raw(jwc, rawtext);
}

void tb_jwrite_array_string(tb_jwrite_control* jwc, const char* value)
{
    if (_tb_jwrite_array(jwc) == TB_JWRITE_OK)
        _tb_jwrite_put_str(jwc, value);
}

void tb_jwrite_array_int(tb_jwrite_control* jwc, int32_t value)
{
    _tb_jwrite_itoa(jwc->tmp_buf, value);
    tb_jwrite_array_raw(jwc, jwc->tmp_buf);
}

void tb_jwrite_array_float(tb_jwrite_control* jwc, float value)
{
    _tb_jwrite_ftoa(jwc->tmp_buf, value, jwc->float_prec);
    tb_jwrite_array_raw(jwc, jwc->tmp_buf);
}

void tb_jwrite_array_null(tb_jwrite_control* jwc)
{
    tb_jwrite_array_raw(jwc, "null");
}

void tb_jwrite_array_object(tb_jwrite_control* jwc)
{
    if (_tb_jwrite_array(jwc) == TB_JWRITE_OK)
    {
        _tb_jwrite_put_ch(jwc, '{');
        _tb_jwrite_push(jwc, TB_JWRITE_OBJECT);
    }
}

void tb_jwrite_array_array(tb_jwrite_control* jwc)
{
    if (_tb_jwrite_array(jwc) == TB_JWRITE_OK)
    {
        _tb_jwrite_put_ch(jwc, '[');
        _tb_jwrite_push(jwc, TB_JWRITE_ARRAY);
    }
}

int tb_jwrite_error_pos(tb_jwrite_control* jwc)
{
    return jwc->call;
}

char* tb_jwrite_error_string(tb_jwrite_error err)
{
    switch (err)
    {
    case TB_JWRITE_OK:          return "OK";
    case TB_JWRITE_NOT_ARRAY:   return "tried to write Array value into Object";
    case TB_JWRITE_NOT_OBJECT:  return "tried to write Object key/value into Array";
    case TB_JWRITE_STACK_FULL:  return "array/object nesting > TB_JWRITE_STACK_DEPTH";
    case TB_JWRITE_STACK_EMPTY: return "stack underflow error (too many 'end's)";
    case TB_JWRITE_NEST_ERROR:  return "nesting error, not all objects closed when tb_jwrite_close() called";
    case TB_JWRITE_WRITE_ERROR: return "failed to write to file";
    default:                    return "Unknown error";
    }
}

/* Utility function to reverse a string  */
static void _tb_jwrite_strreverse(char* str, size_t length) 
{
    char temp;
    for (size_t start = 0; start < length; start++, length--)
    {
        temp = str[start];
        str[start] = str[length];
        str[length] = temp;
    }
} 

/*
 * convert an signed integer to char buffer 
 * make sure buf is big enough
 */
char* _tb_jwrite_itoa(char* buf, int32_t value)
{
    size_t i = 0;

    /* handle the sign */
    uint32_t uvalue = (value < 0) ? -value : value;

    /* Process individual digits */
    do buf[i++] = (char)(48 + (uvalue % 10)); while (uvalue /= 10);
  
    if (value < 0) buf[i++] = '-';
    buf[i] = '\0';
  
    /* Reverse the string */ 
    _tb_jwrite_strreverse(buf, i - 1);

    return buf;
}

/*
 * Taken from: https://github.com/client9/stringencoders
 * Released under the MIT license.
 */

/* Powers of 10 - * 10^0 to 10^9 */
static const double pow10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000,
                               10000000, 100000000, 1000000000 };

/*
 * \brief convert a floating point number to char buffer with a
 *         variable-precision format, and no trailing zeros
 *
 * This is similar to "%.[0-9]f" in the printf style, except it will
 * NOT include trailing zeros after the decimal point.  This type
 * of format oddly does not exists with printf.
 *
 * If the input value is greater than 1<<31, then the output format
 * will be switched exponential format.
 *
 * \param[in] value
 * \param[out] buf  The allocated output buffer.  Should be 32 chars or more.
 * \param[in] precision  Number of digits to the right of the decimal point.
 *    Can only be 0-9.
 */
void _tb_jwrite_ftoa(char* buf, float value, int precision)
{
    /* 
     * Hacky test for NaN
     * under -fast-math this won't work, but then you also won't
     * have correct nan values anyways.  The alternative is
     * to link with libmath (bad) or hack IEEE double bits (bad)
     */
    if (!(value == value))
        strcpy(buf,"nan");

    /* if input is larger than thres_max, revert to exponential */
    const double thres_max = (double)(0x7FFFFFFF);

    double diff = 0.0;
    char* wstr = buf;

    if (precision < 0)
        precision = 0;
    else if (precision > 9)
        precision = 9; /* precision of >= 10 can lead to overflow errors */

    /* we'll work in positive values and deal with the negative sign issue later */
    int neg = 0;
    if (value < 0)
    {
        neg = 1;
        value = -value;
    }

    int whole = (int)value;
    double tmp = ((double)value - whole) * pow10[precision];
    uint32_t frac = (uint32_t)(tmp);
    diff = tmp - frac;

    if ((diff > 0.5)
        /* if halfway, round up if odd, OR if last digit is 0. That last part is strange */
        || (diff == 0.5 && precision > 0 && (frac & 1)) 
        || (diff == 0.5 && precision == 0 && (whole & 1)))
    {
        ++frac;
        /* handle rollover, e.g.  case 0.99 with prec 1 is 1.0  */
        if (frac >= pow10[precision])
        {
            frac = 0;
            ++whole;
        }
    }

    /* 
     * for very large numbers switch back to native sprintf for exponentials.
     * normal printf behavior is to print EVERY whole number digit
     * which can be 100s of characters overflowing your buffers == bad
     */
    if (value > thres_max)
    {
        sprintf(buf, "%e", neg ? -value : value);
        return;
    }

    int has_decimal = 0;
    int count = precision;

    /* Remove ending zeros */
    if (precision > 0)
    {
        while (count > 0 && ((frac % 10) == 0))
        {
            count--;
            frac /= 10;
        }
    }

    while (count > 0)
    {
        --count;
        *wstr++ = (char)(48 + (frac % 10));
        frac /= 10;
        has_decimal = 1;
    }

    if (frac > 0) ++whole;

    /* add decimal */
    if (has_decimal) *wstr++ = '.';

    /* do whole part, take care of sign conversion. Number is reversed. */
    do *wstr++ = (char)(48 + (whole % 10)); while (whole /= 10);

    if (neg) *wstr++ = '-';
    *wstr = '\0';

    _tb_jwrite_strreverse(buf, (wstr - buf) - 1);
}

#endif /* !TB_JWRITE_IMPLEMENTATION */

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