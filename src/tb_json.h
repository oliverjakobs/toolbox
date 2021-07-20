// tb_json - an in-place JSON element reader
// =======================================
//
// Instead of parsing JSON into some structure, this maintains the input JSON as unaltered text
// and allows queries to be made on it directly.
//
// e.g. with the simple JSON:
//  {
//      "astring":"This is a string",
//      "anumber":42,
//      "myarray":[ "one", 2, {"description":"element 3"}, null ],
//      "yesno":true,
//      "HowMany":"1234",
//      "foo":null
//  }
//
// calling:
//  tb_json_read(json, "{'myarray'[0", &elem);
//
// would return:
//  elem.data_type = TB_JSON_STRING;
//  elem.elements = 1
//  elem.bytelen = 3
//  elem.value -> "one"
//
// or you could call the helper functions:
//  tb_json_string(json, "{'astring'", dest_string, NULL);
//  tb_json_int(json, "{'anumber'", &myint);
//  tb_json_string(json, "{'myarray'[3", dest_string, NULL);
//  etc.
//
// Note that the helper functions do type coersion and always return a value
// (on error an empty string is returned or value of zero etc.)
//
// The query string simply defines the route to the required data item
// as an arbitary list of object or array specifiers:
//      object element =    "{'keyname'"
//      array element =     "[INDEX"
//
// The tb_json_read() function fills a json_element structure to describe the located element
// this can be used to locate any element, not just terminal values
// e.g.
//  tb_json_read(json, "{'myarray'", &elem);
//
// in this case elem would contain:
//  elem.dataType = TB_JSON_ARRAY
//  elem.elements = 4
//  elem.bytelen = 46
//  elem.value -> [ "one", 2, {"descripton":"element 3"}, null ] ...
//
// allowing tb_json_read to be called again on the array:
// e.g.
//  tb_json_read(elem.value, "[3", &elem);    // get 4th element - the null value
//
// -------------------------------------------------------
//
// Note that this reader never modifies the source JSON and does not allocate any memory.
// i.e. elements are returned as pointer and length into the source text.
//
// Functions
// =========
// Main JSON reader:
//      int tb_json_read(char* json_source, char* query, json_element* result);
//
// Extended function using query parameters for indexing:
//      int tb_json_read_param(char* json_source, char* query, json_element* pResult, int* query_params);
//
// Function to step thru JSON arrays instead of indexing:
//      char* tb_json_array_step(char* json_array, json_element* result);
//
// Helper functions:
//      long    tb_json_long(char* json, char* query, int* query_params);
//      int     tb_json_int(char* json, char* query, int* query_params);
//      double  tb_json_double(char* json, char* query, int* query_params);
//      int     tb_json_string(char* json, char* query, char* dest, int destlen, int* query_params);
//
// String output Functions
//      char* tb_json_type_to_string(int dataType);    // string describes dataType
//      char* tb_json_error_to_string(int error);      // string descibes error code
//
// Note: tb_json_atol() and tb_json_atof() are modified from original routines
//       fast_atol() and fast_atof() 09-May-2009 Tom Van Baak (tvb) www.LeapSecond.com
//
//       You may want to replace the use of tb_json_atol() and tb_json_atof() in helper functions
//       of your own. Especially note that my atof does not handle exponents.
//

// uncomment this if you really want to use double quotes in query strings instead of '
//#define TB_JSON_DOUBLE_QUOTE_IN_QUERY

#ifndef TB_JSON_INCLUDE_H
#define TB_JSON_INCLUDE_H

#include <stdarg.h>

//--------------------------------------------------------------------
// enums
//--------------------------------------------------------------------

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

//--------------------------------------------------------------------
// tb_json_element
// - structure to return JSON elements
// - error = 0 (TB_JSON_OK) for valid returns
//
// *NOTES*
//    the returned value pointer points into the passed JSON
//    string returns are not '\0' terminated.
//    bytelen specifies the length of the returned data pointed to by value
typedef struct
{
	tb_json_type data_type; // type of the element
	int elements;           // number of elements (e.g. elements in array or object)
	int bytelen;            // byte length of element (e.g. length of string, array text "[ ... ]" etc.)
	char* value;            // pointer to value string in JSON text
	tb_json_error error;    // error value if data_type == TB_JSON_ERROR
} tb_json_element;

//--------------------------------------------------------------------
// JSON reader function
//--------------------------------------------------------------------

// reads a '\0'-terminated JSON text string from json
// traverses the JSON according to the query string
// returns the result value in result
//
// returns: pointer into json after the queried value
//
// e.g.
//    With JSON like: "{ ..., "key":"value", ... }"
//
//    tb_json_read(json, "{'key'", &result);
// returns with: 
//    result.data_type = TB_JSON_STRING, result.value->'value', result.bytelen = 5
char* tb_json_read(char* json, tb_json_element* result, char* query);

char* tb_json_read_format(char* json, tb_json_element* result, const char* query_fmt, ...);

// version of tb_json_read which allows one or more queryParam integers to be substituted
// for array or object indexes marked by a '*' in the query
//
// e.g. tb_json_read_param(json, "[*", &result_element, &array_index);
//
// *!* CAUTION *!*
// You can supply an array of integers which are indexed for each '*' in query
// however, horrid things will happen if you don't supply enough parameters
char* tb_json_read_param(char* json, tb_json_element* result, char* query, int* query_params);

// Array Stepping function
// assumes json_array is JSON source of an array "[ ... ]"
// returns next element of the array in result
// returns pointer to end of element, to be passed to next call of tb_json_array_step()
// if end of array is encountered, result->error = 13 "End of array found"
//
// e.g.
//   With JSON like:   "{ ...  "arrayInObject":[ elem1,elem2,... ], ... }"
//
//   json = tb_json_read(json, "{'arrayInObject'", &theArray);
//   if(theArray.data_type == TB_JSON_ARRAY)
//   {
//       char* array= (char*)theArray.value;
//       tb_json_element arrayElement;
//       for(int index = 0; index < theArray.elements; index++)
//       {
//           array = tb_json_array_step(array, &arrayElement);
//           ...
//
// Note: this significantly speeds up traversing arrays.
char* tb_json_array_step(char* json_array, tb_json_element* result);

//--------------------------------------------------------------------
// Helper Functions
//--------------------------------------------------------------------

long    tb_json_long(char* json, char* query, int* query_params, long default_value);
int     tb_json_int(char* json, char* query, int* query_params, int default_value);
float   tb_json_float(char* json, char* query, int* query_params, float default_value);
int     tb_json_string(char* json, char* query, char *dest, int destlen, int* query_params);

typedef int(*tb_json_parse_func)(const char*, size_t);
int     tb_json_parse(char* json, char* query, int* query_params, tb_json_parse_func parse);

char* tb_json_atoi(char* p, unsigned int* result);  // string to unsigned int
char* tb_json_atol(char* p, long* result);          // string to signed long
char* tb_json_atof(char* p, float* result);         // string to float (does not do exponents)

int tb_json_strcmp(const tb_json_element* a, const tb_json_element* b); // compare STRING elements
char* tb_json_strcpy(char* dest_buffer, const tb_json_element* element); // copy element to '\0'-terminated buffer

int tb_json_is_type(const tb_json_element* element, tb_json_type type);

void tb_json_print_element(tb_json_element element);

//--------------------------------------------------------------------
// String output Functions
//--------------------------------------------------------------------

char* tb_json_type_to_string(tb_json_type data_type);   // string describes data_type
char* tb_json_error_to_string(tb_json_error error);     // string descibes error code

#endif // TB_JSON_INCLUDE_H