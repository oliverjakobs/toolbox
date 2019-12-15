#ifndef JSON_WRITE_INCLUDE_H
#define JSON_WRITE_INCLUDE_H
//
// A *really* simple JSON writer in C
// - a collection of functions to generate JSON semi-automatically
//
// The idea is to simplify writing native C values into a JSON string and
// to provide some error trapping to ensure that the result is valid JSON.
//
// Example:
//  json_write_control_t jwc;
//
//  json_write_open(&jwc, buffer, buflen, JW_OBJECT, 1);    // open root node as object
//  json_write_obj_string(&jwc, "key", "value");
//  json_write_obj_int(&jwc, "int", 1);
//  json_write_obj_array(&jwc, "anArray");
//      json_write_array_int(&jwc, 0);
//      json_write_array_int(&jwc, 1);
//      json_write_array_int(&jwc, 2);
//  json_write_end(&jwc);
//  json_write_error err = json_write_close(&jwc);          // close root object
//
// results in:
//
//  {
//      "key": "value",
//      "int": 1,
//      "anArray": [
//          0,
//          1,
//          2
//      ]
//  }
//
// Note that json_write handles string quoting and getting commas in the right place.
// If the sequence of calls is incorrect
// e.g.
//      json_write_open(buffer, buflen, JW_OBJECT, 1);
//      json_write_obj_string("key", "value");
//          json_write_array_int(0);
//      ...
//
// then the error code returned from jwClose() would indicate that you attempted to
// put an array element into an object (instead of a key:value pair)
// To locate the error, the supplied buffer has the JSON created upto the error point
// and a call to jwErrorPos() would return the function call at which the error occurred
// - in this case 3, the 3rd function call "json_write_array_int(0)" is not correct at this point.
//
// The root JSON type can be JSON_WRITE_OBJECT or JSON_WRITE_ARRAY.
//
// For more information on each function, see the prototypes below.
//

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
// ----| Version |--------------------------------------------------------------
// -----------------------------------------------------------------------------

#define JSON_WRITE_VERSION_MAJOR    1
#define JSON_WRITE_VERSION_MINOR    0

#define JSON_WRITE_STACK_DEPTH 32   // max nesting depth of objects/arrays

typedef enum
{
    JSON_WRITE_COMPACT, // output string control for json_write_open()
    JSON_WRITE_PRETTY   // pretty adds \n and indentation
} json_write_style;

// Error Codes
// -----------
typedef enum
{
    JSON_WRITE_OK,
    JSON_WRITE_BUF_FULL,    // output buffer full
    JSON_WRITE_NOT_ARRAY,   // tried to write Array value into Object
    JSON_WRITE_NOT_OBJECT,  // tried to write Object key/value into Array
    JSON_WRITE_STACK_FULL,  // array/object nesting > JSON_WRITE_STACK_DEPTH
    JSON_WRITE_STACK_EMPTY, // stack underflow error (too many 'end's)
    JSON_WRITE_NEST_ERROR   // nesting error, not all objects closed when json_write_close() called
} json_write_error;

typedef enum
{
    JSON_WRITE_OBJECT,
    JSON_WRITE_ARRAY
} json_write_node_type;

typedef struct
{
    json_write_node_type type;
    int element;
} json_write_node_t;

typedef struct
{
    char* buffer;           // pointer to application's buffer
    unsigned int buflen;    // length of buffer
    char* writepos;         // current write position in buffer
    char tmpbuf[32];        // local buffer for int/double convertions
    json_write_error error; // error code
    int call;               // call on which error occurred
    json_write_node_t nodes[JSON_WRITE_STACK_DEPTH]; // stack of array/object nodes
    int stackpos;
    json_write_style style;
} json_write_control_t;


// json_write_open
// - initialises json_write with the application supplied 'buffer' of length 'buflen'
//   in operation, the buffer will always contain a valid '\0'-terminated string
// - json_write will not overrun the buffer (it returns an "output buffer full" error)
// - rootType is the base JSON type: JSON_WRITE_OBJECT or JSON_WRITE_ARRAY
// - style controls 'prettifying' the output: JSON_WRITE_PRETTY or JSON_WRITE_COMPACT
void json_write_open(json_write_control_t* jwc, char* buffer, unsigned int buflen, json_write_node_type root_type, json_write_style style);

// json_write_close
// - closes the element opened by json_write_open()
// - returns error code (if everything was fine: JSON_WRITE_OK)
// - after an error, all following json_write calls are skipped internally
//   so the error code is for the first error detected
json_write_error json_write_close(json_write_control_t* jwc);

// Object insertion functions
// - used to insert "key":"value" pairs into an object
void json_write_object_string(json_write_control_t* jwc, char* key, char* value);
void json_write_object_int(json_write_control_t* jwc, char* key, int value);
void json_write_object_double(json_write_control_t* jwc, char* key, double value);
void json_write_object_bool(json_write_control_t* jwc, char* key, int oneOrZero);
void json_write_object_null(json_write_control_t* jwc, char* key);
void json_write_object_object(json_write_control_t* jwc, char* key);
void json_write_object_array(json_write_control_t* jwc, char* key);

// Array insertion functions
// - used to insert "value" elements into an array
void json_write_array_string(json_write_control_t* jwc, char* value);
void json_write_array_int(json_write_control_t* jwc, int value);
void json_write_array_double(json_write_control_t* jwc, double value);
void json_write_array_bool(json_write_control_t* jwc, int oneOrZero);
void json_write_array_null(json_write_control_t* jwc);
void json_write_array_object(json_write_control_t* jwc);
void json_write_array_array(json_write_control_t* jwc);

// json_write_end
// - defines the end of an Object or Array definition
json_write_error json_write_end(json_write_control_t* jwc);

// these 'raw' routines write the JSON value as the contents of rawtext
// i.e. enclosing quotes are not added
// - use if your app. supplies its own value->string functions
void json_write_object_raw(json_write_control_t* jwc, char* key, char* rawtext);
void json_write_array_raw(json_write_control_t* jwc, char* rawtext);

// json_write_error_pos
// - if json_write_close returned an error, this function returns the number of the jWrite function call
//   which caused that error.
int json_write_error_pos(json_write_control_t* jwc);

// Returns '\0'-termianted string describing the error (as returned by json_write_close())
char* json_write_error_string(json_write_error err);

#ifdef __cplusplus
}
#endif

//
//
////   end header file   /////////////////////////////////////////////////////
#endif // JSON_WRITE_INCLUDE_H

//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
// Implementation
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------

#ifdef JSON_WRITE_IMPLEMENTATION

#include <stddef.h>
#include <stdio.h>
#include <string.h> // memset()

//#include <stdint.h> // definintion of uint32_t, int32_t
typedef unsigned int uint32_t;
typedef int int32_t;

static void _jw_modp_itoa10(int32_t value, char* str);
static void _jw_modp_dtoa2(double value, char* str, int prec);

//------------------------------------------
// Internal functions
static void _jw_put_ch(json_write_control_t* jwc, char c)
{
    if ((unsigned int)(jwc->writepos - jwc->buffer) >= jwc->buflen)
    {
        jwc->error = JSON_WRITE_BUF_FULL;
    }
    else
    {
        *jwc->writepos++ = c;
    }
}

// put string enclosed in quotes
static void _jw_put_str(json_write_control_t* jwc, char* str)
{
    _jw_put_ch(jwc, '\"');

    while (*str != '\0')
        _jw_put_ch(jwc, *str++);

    _jw_put_ch(jwc, '\"');
}

// put raw string
static void _jw_put_raw(json_write_control_t* jwc, char* str)
{
    while (*str != '\0')
        _jw_put_ch(jwc, *str++);
}

static void _jw_pretty(json_write_control_t* jwc)
{
    if (jwc->style == JSON_WRITE_PRETTY)
    {
        _jw_put_ch(jwc, '\n');
        for (int i = 0; i < jwc->stackpos + 1; i++)
            _jw_put_raw(jwc, "    ");
    }
}

// Push / Pop node stack
static void _jw_push(json_write_control_t* jwc, json_write_node_type node_type)
{
    if ((jwc->stackpos + 1) >= JSON_WRITE_STACK_DEPTH)
    {
        jwc->error = JSON_WRITE_STACK_FULL; // array/object nesting > JSON_WRITE_STACK_DEPTH
    }
    else
    {
        jwc->nodes[++jwc->stackpos].type = node_type;
        jwc->nodes[jwc->stackpos].element = 0;
    }
}

static json_write_node_type _jw_pop(json_write_control_t* jwc)
{
    json_write_node_type retval= jwc->nodes[jwc->stackpos].type;
    if (jwc->stackpos == 0)
        jwc->error = JSON_WRITE_STACK_EMPTY; // stack underflow error (too many 'end's)
    else
        jwc->stackpos--;
    return retval;
}

//------------------------------------------
// json_write_open
// - open writing of JSON starting with rootType = JW_OBJECT or JW_ARRAY
// - initialise with user string buffer of length buflen
// - isPretty=JW_PRETTY adds \n and spaces to prettify output (else JW_COMPACT)
void json_write_open(json_write_control_t *jwc, char* buffer, unsigned int buflen, json_write_node_type root_type, json_write_style style)
{
    memset(buffer, 0, buflen); // zap the whole destination buffer
    jwc->buffer = buffer;
    jwc->buflen = buflen;
    jwc->writepos = buffer;
    jwc->nodes[0].type = root_type;
    jwc->nodes[0].element = 0;
    jwc->stackpos = 0;
    jwc->error = JSON_WRITE_OK;
    jwc->call = 1;
    jwc->style = style;
    _jw_put_ch(jwc, (root_type == JSON_WRITE_OBJECT) ? '{' : '[');
}

//------------------------------------------
// json_write_close
// - closes the root JSON object started by jwOpen()
// - returns error code
json_write_error json_write_close(json_write_control_t *jwc )
{
    if (jwc->error == JSON_WRITE_OK)
    {
        if (jwc->stackpos == 0)
        {
            json_write_node_type node = jwc->nodes[0].type;
            if (jwc->style == JSON_WRITE_PRETTY)
                _jw_put_ch(jwc, '\n');

            _jw_put_ch(jwc, (node == JSON_WRITE_OBJECT) ? '}' : ']');
        }
        else
        {
            jwc->error = JSON_WRITE_NEST_ERROR; // nesting error, not all objects closed when json_write_close() called
        }
    }
    return jwc->error;
}

//------------------------------------------
// End the current array/object
json_write_error json_write_end(json_write_control_t* jwc)
{
    if (jwc->error == JSON_WRITE_OK)
    {
        int last_element = jwc->nodes[jwc->stackpos].element;
        json_write_node_type node = _jw_pop(jwc);
        if (last_element > 0)
            _jw_pretty(jwc);
        _jw_put_ch(jwc, (node == JSON_WRITE_OBJECT) ? '}' : ']');
    }
    return jwc->error;
}

//------------------------------------------
// Object insert functions
//

// *common Object function*
// - checks error
// - checks current node is OBJECT
// - adds comma if reqd
// - adds "key" :
json_write_error _json_write_object(json_write_control_t* jwc, char* key)
{
    if (jwc->error == JSON_WRITE_OK)
    {
        jwc->call++;
        if (jwc->nodes[jwc->stackpos].type != JSON_WRITE_OBJECT)
            jwc->error = JSON_WRITE_NOT_OBJECT; // tried to write Object key/value into Array
        else if (jwc->nodes[jwc->stackpos].element++ > 0)
            _jw_put_ch(jwc, ',');
        _jw_pretty(jwc);
        _jw_put_str(jwc, key );
        _jw_put_ch(jwc, ':');
        if (jwc->style == JSON_WRITE_PRETTY)
            _jw_put_ch(jwc, ' ');
    }
    return jwc->error;
}

// put raw string to object (i.e. contents of rawtext without quotes)
void json_write_object_raw(json_write_control_t* jwc, char* key, char* rawtext)
{
    if (_json_write_object(jwc, key) == JSON_WRITE_OK)
        _jw_put_raw(jwc, rawtext);
}

// put "quoted" string to object
void json_write_object_string(json_write_control_t* jwc, char* key, char* value)
{
    if (_json_write_object(jwc, key) == JSON_WRITE_OK)
        _jw_put_str(jwc, value);
}

void json_write_object_int(json_write_control_t* jwc, char* key, int value)
{
    _jw_modp_itoa10(value, jwc->tmpbuf);
    json_write_object_raw(jwc, key, jwc->tmpbuf);
}

void json_write_object_double(json_write_control_t* jwc, char* key, double value)
{
    _jw_modp_dtoa2(value, jwc->tmpbuf, 6);
    json_write_object_raw(jwc, key, jwc->tmpbuf);
}

void json_write_object_bool(json_write_control_t* jwc, char* key, int oneOrZero)
{
    json_write_object_raw(jwc, key, (oneOrZero) ? "true" : "false");
}

void json_write_object_null(json_write_control_t* jwc, char* key)
{
    json_write_object_raw(jwc, key, "null");
}

// put Object in Object
void json_write_object_object(json_write_control_t* jwc, char* key)
{
    if (_json_write_object(jwc, key) == JSON_WRITE_OK)
    {
        _jw_put_ch(jwc, '{');
        _jw_push(jwc, JSON_WRITE_OBJECT);
    }
}

// put Array in Object
void json_write_object_array(json_write_control_t* jwc, char* key)
{
    if (_json_write_object(jwc, key) == JSON_WRITE_OK)
    {
        _jw_put_ch(jwc, '[');
        _jw_push(jwc, JSON_WRITE_ARRAY);
    }
}

//------------------------------------------
// Array insert functions
//

// *common Array function*
// - checks error
// - checks current node is ARRAY
// - adds comma if reqd
json_write_error _json_write_array(json_write_control_t* jwc)
{
    if (jwc->error == JSON_WRITE_OK)
    {
        jwc->call++;
        if (jwc->nodes[jwc->stackpos].type != JSON_WRITE_ARRAY)
            jwc->error = JSON_WRITE_NOT_ARRAY; // tried to write array value into Object
        else if (jwc->nodes[jwc->stackpos].element++ > 0)
            _jw_put_ch(jwc, ',');
        _jw_pretty(jwc);
    }
    return jwc->error;
}

// put raw string to array (i.e. contents of rawtext without quotes)
//
void json_write_array_raw(json_write_control_t* jwc, char* rawtext)
{
    if (_json_write_array(jwc) == JSON_WRITE_OK)
        _jw_put_raw(jwc, rawtext);
}

// put "quoted" string to array
//
void json_write_array_string(json_write_control_t* jwc, char* value)
{
    if (_json_write_array(jwc) == JSON_WRITE_OK)
        _jw_put_str(jwc, value);
}

void json_write_array_int(json_write_control_t* jwc, int value)
{
    _jw_modp_itoa10(value, jwc->tmpbuf);
    json_write_array_raw(jwc, jwc->tmpbuf);
}

void json_write_array_double(json_write_control_t* jwc, double value)
{
    _jw_modp_dtoa2(value, jwc->tmpbuf, 6);
    json_write_array_raw(jwc, jwc->tmpbuf);
}

void json_write_array_bool(json_write_control_t* jwc, int oneOrZero)
{
    json_write_array_raw(jwc, (oneOrZero) ? "true" : "false");
}

void json_write_array_null(json_write_control_t* jwc)
{
    json_write_array_raw(jwc, "null");
}

void json_write_array_object(json_write_control_t* jwc)
{
    if (_json_write_array(jwc) == JSON_WRITE_OK)
    {
        _jw_put_ch(jwc, '{');
        _jw_push(jwc, JSON_WRITE_OBJECT);
    }
}

void json_write_array_array(json_write_control_t* jwc)
{
    if (_json_write_array(jwc) == JSON_WRITE_OK)
    {
        _jw_put_ch(jwc, '[');
        _jw_push(jwc, JSON_WRITE_ARRAY);
    }
}

//------------------------------------------
// json_write_error_pos
// - Returns position of error: the nth call to a JSON_WRITE function
//
int json_write_error_pos(json_write_control_t* jwc)
{
    return jwc->call;
}

//------------------------------------------
// json_write_error_string
// - returns string describing error code
char* json_write_error_string(json_write_error err)
{
    switch(err)
    {
    case JSON_WRITE_OK:         return "OK"; 
    case JSON_WRITE_BUF_FULL:   return "output buffer full";
    case JSON_WRITE_NOT_ARRAY:  return "tried to write Array value into Object";
    case JSON_WRITE_NOT_OBJECT: return "tried to write Object key/value into Array";
    case JSON_WRITE_STACK_FULL: return "array/object nesting > JSON_WRITE_STACK_DEPTH";
    case JSON_WRITE_STACK_EMPTY:return "stack underflow error (too many 'end's)";
    case JSON_WRITE_NEST_ERROR: return "nesting error, not all objects closed when json_write_close() called";
    default:                    return "Unknown error";
    }
}

//=================================================================
//
// modp value-to-string functions
// - modified for C89
//
// We use these functions as they are a lot faster than sprintf()
//
// Origin of these routines:
/*
 * <pre>
 * Copyright &copy; 2007, Nick Galbreath -- nickg [at] modp [dot] com
 * All rights reserved.
 * http://code.google.com/p/stringencoders/
 * Released under the bsd license.
 * </pre>
 */

static void _jw_strreverse(char* begin, char* end)
{
    char aux;
    while (end > begin)
        aux = *end, *end-- = *begin, *begin++ = aux;
}

/** \brief convert an signed integer to char buffer
 *
 * \param[in] value
 * \param[out] buf the output buffer.  Should be 16 chars or more.
 */
void _jw_modp_itoa10(int32_t value, char* str)
{
    char* wstr=str;
    // Take care of sign
    unsigned int uvalue = (value < 0) ? -value : value;
    // Conversion. Number is reversed.
    do *wstr++ = (char)(48 + (uvalue % 10)); while(uvalue /= 10);
    if (value < 0) *wstr++ = '-';
    *wstr='\0';

    // Reverse string
    _jw_strreverse(str,wstr-1);
}

/**
 * Powers of 10
 * 10^0 to 10^9
 */
static const double pow10[] = {1, 10, 100, 1000, 10000, 100000, 1000000,
                               10000000, 100000000, 1000000000};

/** \brief convert a floating point number to char buffer with a
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
void _jw_modp_dtoa2(double value, char* str, int prec)
{
    /* if input is larger than thres_max, revert to exponential */
    const double thres_max = (double)(0x7FFFFFFF);
    int count;
    double diff = 0.0;
    char* wstr = str;
    int neg= 0;
    int whole;
    double tmp;
    uint32_t frac;

    /* Hacky test for NaN
     * under -fast-math this won't work, but then you also won't
     * have correct nan values anyways.  The alternative is
     * to link with libmath (bad) or hack IEEE double bits (bad)
     */
    if (! (value == value)) {
        str[0] = 'n'; str[1] = 'a'; str[2] = 'n'; str[3] = '\0';
        return;
    }

    if (prec < 0) {
        prec = 0;
    } else if (prec > 9) {
        /* precision of >= 10 can lead to overflow errors */
        prec = 9;
    }

    /* we'll work in positive values and deal with the
       negative sign issue later */
    if (value < 0) {
        neg = 1;
        value = -value;
    }

    whole = (int) value;
    tmp = (value - whole) * pow10[prec];
    frac = (uint32_t)(tmp);
    diff = tmp - frac;

    if (diff > 0.5) {
        ++frac;
        /* handle rollover, e.g.  case 0.99 with prec 1 is 1.0  */
        if (frac >= pow10[prec]) {
            frac = 0;
            ++whole;
        }
    } else if (diff == 0.5 && ((frac == 0) || (frac & 1))) {
        /* if halfway, round up if odd, OR
           if last digit is 0.  That last part is strange */
        ++frac;
    }

    /* for very large numbers switch back to native sprintf for exponentials.
       anyone want to write code to replace this? */
    /*
      normal printf behavior is to print EVERY whole number digit
      which can be 100s of characters overflowing your buffers == bad
    */
    if (value > thres_max) {
        sprintf(str, "%e", neg ? -value : value);
        return;
    }

    if (prec == 0) {
        diff = value - whole;
        if (diff > 0.5) {
            /* greater than 0.5, round up, e.g. 1.6 -> 2 */
            ++whole;
        } else if (diff == 0.5 && (whole & 1)) {
            /* exactly 0.5 and ODD, then round up */
            /* 1.5 -> 2, but 2.5 -> 2 */
            ++whole;
        }

        //vvvvvvvvvvvvvvvvvvv  Diff from modp_dto2
    } else if (frac) {
        count = prec;
        // now do fractional part, as an unsigned number
        // we know it is not 0 but we can have leading zeros, these
        // should be removed
        while (!(frac % 10)) {
            --count;
            frac /= 10;
        }
        //^^^^^^^^^^^^^^^^^^^  Diff from modp_dto2

        // now do fractional part, as an unsigned number
        do {
            --count;
            *wstr++ = (char)(48 + (frac % 10));
        } while (frac /= 10);
        // add extra 0s
        while (count-- > 0) *wstr++ = '0';
        // add decimal
        *wstr++ = '.';
    }

    // do whole part
    // Take care of sign
    // Conversion. Number is reversed.
    do *wstr++ = (char)(48 + (whole % 10)); while (whole /= 10);
    if (neg) {
        *wstr++ = '-';
    }
    *wstr='\0';
    _jw_strreverse(str, wstr-1);
}

#endif // JSON_WRITE_IMPLEMENTATION

/*
------------------------------------------------------------------------------
This software is available under the MIT License.
------------------------------------------------------------------------------
Copyright (c) 2019 Oliver Jakobs

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
------------------------------------------------------------------------------
*/