/*
tb_json v1.0 - an in-place JSON element reader
-----------------------------------------------------------------------------------------

Instead of parsing JSON into some structure, this maintains the input JSON as unaltered text
and allows queries to be made on it directly.

Note that this reader never modifies the source JSON and does not allocate any memory.
i.e. elements are returned as pointer and length into the source text.

USAGE:

with the simple JSON:
    {
        "astring":"This is a string",
        "anumber":42,
        "myarray":[ "one", 2, {"description":"element 3"}, null ],
        "yesno":true,
        "HowMany":"1234",
        "foo":null
    }

calling: 
    tb_json_read(json, "{'myarray'[0", &elem);
    
would return:
    elem.data_type = TB_JSON_STRING;
    elem.elements = 1
    elem.bytelen = 3
    elem.value -> "one"

or you could call the helper functions:
    tb_json_string(json, "{'astring'", dest_string, NULL);
    tb_json_int(json, "{'anumber'", &myint);
    tb_json_string(json, "{'myarray'[3", dest_string, NULL);
    etc.
Notes:
    - helper functions do type coersion and always return a value
        (on error an empty string is returned or the default_value etc.)
    - by default, pass NULL for queryParams unless you are using '*' in the query for indexing

The query string simply defines the route to the required data item
as an arbitary list of object or array specifiers:
    object element =    "{'keyname'"
    array element =     "[INDEX"

The tb_json_read() function fills a json_element structure to describe the located element
this can be used to locate any element, not just terminal values

After calling 'tb_json_read(json, "{'myarray'", &elem)' elem would contain in this case:
    elem.dataType = TB_JSON_ARRAY
    elem.elements = 4
    elem.bytelen = 46
    elem.value -> [ "one", 2, {"descripton":"element 3"}, null ] ...

allowing tb_json_read to be called again on the array:
    e.g. 'tb_json_read(elem.value, "[3", &elem)' to get the 4th element - the null value

tb_json_array_step:
    With JSON like:   "{ ...  "arrayInObject":[ elem1,elem2,... ], ... }"

    json = tb_json_read(json, "{'arrayInObject'", &the_array);
    if(the_array.data_type == TB_JSON_ARRAY)
    {
        char* array= (char*)the_array.value;
        tb_json_element array_element;
        int index;
        for(index = 0; index < the_array.elements; index++)
        {
            array = tb_json_array_step(array, &array_element);
            ...

*/

#ifndef TB_JSON_H
#define TB_JSON_H

/* uncomment this if you really want to use double quotes in query strings instead of ' */
/* #define TB_JSON_DOUBLE_QUOTE_IN_QUERY */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>

/* ----------------------| types |-------------------------------- */
typedef enum
{
    /* return types: */
    TB_JSON_ERROR,      /* general error, eof etc. */
    TB_JSON_OBJECT,     /* "{" */
    TB_JSON_ARRAY,      /* "[" */
    TB_JSON_STRING,     /* "string" */
    TB_JSON_NUMBER,     /* number (may be -ve) int or float */
    TB_JSON_BOOL,       /* true or false */
    TB_JSON_NULL,       /* null */
    TB_JSON_KEY,        /* object "key" */
    /* internal values: */
    TB_JSON_COLON,      /* ":" */
    TB_JSON_EOL,        /* end of input string (ptr at '\0') */
    TB_JSON_COMMA,      /* "," */
    TB_JSON_EOBJECT,    /* "}" */
    TB_JSON_EARRAY,     /* "]" */
    TB_JSON_QPARAM      /* "*" query string parameter */
} tb_json_type;

/* ----------------------| error codes |-------------------------- */
typedef enum
{
    TB_JSON_OK,
    TB_JSON_QUERY_MISMATCH,
    TB_JSON_READ_ERROR,
    TB_JSON_EXPECTED_KEY,
    TB_JSON_EXPECTED_COLON,
    TB_JSON_KEY_NOT_FOUND,
    TB_JSON_EXPECTED_COMMA_OBJECT,
    TB_JSON_TERMINAL_BEFORE_END,
    TB_JSON_UNEXPECTED_CHARACTER,
    TB_JSON_EXPECTED_COMMA_ARRAY,
    TB_JSON_BAD_INDEX_ARRAY,
    TB_JSON_BAD_INDEX_OBJECT,
    TB_JSON_BAD_OBJECT_KEY,
    TB_JSON_END_OF_ARRAY,
    TB_JSON_END_OF_OBJECT
} tb_json_error;

/*
 * tb_json_element
 * - structure to return JSON elements
 * - error = TB_JSON_OK for valid returns
 *
 * NOTE:
 *  the returned value pointer points into the passed JSON
 *  string returns are not '\0' terminated.
 *  bytelen specifies the length of the returned data pointed to by value
 */
typedef struct
{
	tb_json_type data_type; /* type of the element */
	int elements;           /* number of elements (e.g. elements in array or object) */
	int bytelen;            /* byte length of element (e.g. length of string, array text "[ ... ]" etc.) */
	void* value;            /* pointer to value string in JSON text */
	tb_json_error error;    /* error value if data_type == TB_JSON_ERROR */
} tb_json_element;

/* ----------------------| JSON reader functions |---------------- */

/*
 * reads a '\0'-terminated JSON text string from json
 * traverses the JSON according to the query string
 * returns the result value in result
 * returns: pointer into json after the queried value
 * Note: tb_json_read is recursive
 */
char* tb_json_read(char* json, tb_json_element* result, char* query);

/* 
 * version of tb_json_read with printf like formatting
 */ 
char* tb_json_read_format(char* json, tb_json_element* result, const char* query_fmt, ...);

/*
 * version of tb_json_read which allows one or more query_param integers to be substituted
 * for array or object indexes marked by a '*' in the query
 *  e.g. tb_json_read_param(json, "[*", &result_element, &array_index);
 *
 * CAUTION:
 *  You can supply an array of integers which are indexed for each '*' in query
 *  however, horrid things will happen if you don't supply enough parameters
 */
char* tb_json_read_param(char* json, tb_json_element* result, char* query, int* query_params);

/*
 * Array Stepping function
 * assumes json_array points at the start of an array or array element
 * returns next element of the array in result
 * returns pointer to end of element, to be passed to next call of tb_json_array_step()
 * if end of array is encountered, result->error = 13 "End of array found"
 * 
 * Note: this significantly speeds up traversing arrays.
 */
char* tb_json_array_step(char* json_array, tb_json_element* result);

/* ----------------------| Helper functions |--------------------- */

/*
 * reads signed long value from JSON 
 * returns number from NUMBER or STRING elements (if possible)
 * returns 1 or 0 from BOOL elements
 * returns default_value on error
 */
long tb_json_long(char* json, char* query, int* query_params, long default_value);

/*
 * tb_json_long castet to int
 */
int tb_json_int(char* json, char* query, int* query_params, int default_value);

/*
 * returns float from JSON
 * returns number from NUMBER or STRING elements
 * returns default_value on error
 */
float tb_json_float(char* json, char* query, int* query_params, float default_value);

/*
 * Copy string to dest and '\0'-terminate it (upto dest_len total bytes)
 * returns: character length of string (excluding '\0' terminator)
 *
 * Note: any element can be returned as a string
 */
int tb_json_string(char* json, char* query, char *dest, int destlen, int* query_params);

/*
 * tb_json_atol() and tb_json_atof() are modified from original routines
 * fast_atol() and fast_atof() 09-May-2009 Tom Van Baak (tvb) www.LeapSecond.com
 *
 * You may want to replace the use of tb_json_atol() and tb_json_atof() in helper functions
 * of your own. Especially note that my atof does not handle exponents.
 */
char* tb_json_atoi(char* p, unsigned int* result);  /* read unsigned int from string */
char* tb_json_atol(char* p, long* result);          /* read signed long from string */
char* tb_json_atof(char* p, float* result);         /* string to float (does not handle exponents) */

/* 
 * compare STRING elements
 * returns: 0 if they are identical strings, else 1
 */
int tb_json_strcmp(tb_json_element* a, tb_json_element* b);

/*
 * read element into destination buffer and add '\0' terminator
 * always copies element irrespective of data_type (unless it's an error)
 * dest_buffer is always '\0'-terminated (even on zero lenght returns)
 * returns pointer to dest_buffer
 */
char* tb_json_strcpy(char* dest_buffer, int dest_length, tb_json_element* element);


/* prints the value of an element with printf */
void tb_json_print_element(tb_json_element element);

/* ----------------------| String output functions |-------------- */
char* tb_json_type_to_string(tb_json_type data_type);   /* string describes data_type */
char* tb_json_error_to_string(tb_json_error error);     /* string descibes error code */

#ifdef __cplusplus
}
#endif

#endif /* !TB_JSON_H */

/*
------------------------------------------------------------------------------
This software is available under the MIT License.
------------------------------------------------------------------------------
Copyright (c) 2020 Oliver Jakobs

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