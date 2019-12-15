#ifndef JSON_WRITE_INCLUDE_H
#define JSON_WRITE_INCLUDE_H
//
// A *really* simple JSON writer in C  (C89)
// - a collection of functions to generate JSON semi-automatically
//
// The idea is to simplify writing native C values into a JSON string and
// to provide some error trapping to ensure that the result is valid JSON.
//
// Example:
//		jwOpen( buffer, buflen, JW_OBJECT, JW_PRETTY );		// open root node as object
//		json_write_obj_string( "key", "value" );
//		json_write_obj_int( "int", 1 );
//		json_write_obj_array( "anArray");
//			json_write_array_int( 0 );
//			json_write_array_int( 1 );
//			json_write_array_int( 2 );
//		jwEnd();
//		err= jwClose();								// close root object
//
// results in:
//
//		{
//		    "key": "value",
//		    "int": 1,
//		    "anArray": [
//		        0,
//		        1,
//		        2
//		    ]
//		}
//
// Note that jWrite handles string quoting and getting commas in the right place.
// If the sequence of calls is incorrect
// e.g.
//		jwOpen( buffer, buflen, JW_OBJECT, 1 );
//		json_write_obj_string( "key", "value" );
//			json_write_array_int( 0 );
//      ...
//
// then the error code returned from jwClose() would indicate that you attempted to
// put an array element into an object (instead of a key:value pair)
// To locate the error, the supplied buffer has the JSON created upto the error point
// and a call to jwErrorPos() would return the function call at which the error occurred
// - in this case 3, the 3rd function call "json_write_array_int(0)" is not correct at this point.
//
// The root JSON type can be JW_OBJECT or JW_ARRAY.
//
// For more information on each function, see the prototypes below.
//
//
// GLOBAL vs. Application-Supplied Control Structure
// -------------------------------------------------
// jWrite requires a jWriteControl structure to save the internal state.
// For many applications it is much simpler for this to be a global variable as
// used by the above examples.
//
// To use multiple instances of jWrite, an application has to supply unique instances
// of jWriteControl structures. 
//
// This feature is enabled by commenting out the definition of JW_GLOBAL_CONTROL_STRUCT
//
// All the jWrite functions then take an additional parameter: a ptr to the structure
// e.g.
//		json_write_control_t jwc;
//
//		jwOpen( &jwc, buffer, buflen, JW_OBJECT, 1 );
//		json_write_obj_string( &jwc, "key", "value" );
//		json_write_obj_int( &jwc, "int", 1 );
//		json_write_obj_array( &jwc, "anArray");
//			json_write_array_int( &jwc, 0 );
//			json_write_array_int( &jwc, 1 );
//			json_write_array_int( &jwc, 2 );
//		jwEnd( &jwc );
//		err= jwClose( &jwc );
//
// - which is more flexible, but a pain to type in !
//
// TonyWilk, Mar 2015
//
//

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
    JSON_WRITE_OBJECT = 1,
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
// - initialises jWrite with the application supplied 'buffer' of length 'buflen'
//   in operation, the buffer will always contain a valid '\0'-terminated string
// - jWrite will not overrun the buffer (it returns an "output buffer full" error)
// - rootType is the base JSON type: JW_OBJECT or JW_ARRAY
// - style controls 'prettifying' the output: JSON_WRITE_PRETTY or JSON_WRITE_COMPACT
void json_write_open(json_write_control_t* jwc, char* buffer, unsigned int buflen, json_write_node_type root_type, json_write_style style);

// json_write_close
// - closes the element opened by json_write_open()
// - returns error code (0 = JWRITE_OK)
// - after an error, all following jWrite calls are skipped internally
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

#endif // JSON_WRITE_INCLUDE_H