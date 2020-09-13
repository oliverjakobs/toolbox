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