#ifndef JSON_INCLUDE_H
#define JSON_INCLUDE_H

// json - an in-place JSON element reader
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
//  json_read(json, "{'myarray'[0", &elem);
//
// would return:
//  elem.data_type = JREAD_STRING;
//  elem.elements = 1
//  elem.bytelen = 3
//  elem.value -> "one"
//
// or you could call the helper functions:
//  json_string(json, "{'astring'", dest_string, NULL);
//  json_int(json, "{'anumber'", &myint);
//  json_string(json, "{'myarray'[3", dest_string, NULL);
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
// The json_read() function fills a json_element_t structure to describe the located element
// this can be used to locate any element, not just terminal values
// e.g.
//  json_read(json, "{'myarray'", &elem);
//
// in this case elem would contain:
//  elem.dataType = JSON_ARRAY
//  elem.elements = 4
//  elem.bytelen = 46
//  elem.value -> [ "one", 2, {"descripton":"element 3"}, null ] ...
//
// allowing json_read to be called again on the array:
// e.g.
//  json_read(elem.value, "[3", &elem);    // get 4th element - the null value
//
// -------------------------------------------------------
//
// Note that this reader never modifies the source JSON and does not allocate any memory.
// i.e. elements are returned as pointer and length into the source text.
//
// Functions
// =========
// Main JSON reader:
//      int json_read(char* json_source, char* query, json_element_t* result);
//
// Extended function using query parameters for indexing:
//      int json_read_param(char* json_source, char* query, json_element_t* pResult, int* query_params);
//
// Function to step thru JSON arrays instead of indexing:
//      char* json_array_step(char* json_array, json_element_t* result);
//
// Helper functions:
//      long    json_long(char* json, char* query, int* query_params);
//      int     json_int(char* json, char* query, int* query_params);
//      double  json_double(char* json, char* query, int* query_params);
//      int     json_string(char* json, char* query, char* dest, int destlen, int* query_params);
//
// String output Functions
//      char* json_type_to_string(int dataType);    // string describes dataType
//      char* json_error_to_string(int error);      // string descibes error code
//
// Note: json_atol() and json_atof() are modified from original routines
//       fast_atol() and fast_atof() 09-May-2009 Tom Van Baak (tvb) www.LeapSecond.com
//
//       You may want to replace the use of json_atol() and json_atof() in helper functions
//       of your own. Especially note that my atof does not handle exponents.
//

// uncomment this if you really want to use double quotes in query strings instead of '
//#define JSON_DOUBLE_QUOTE_IN_QUERY

#ifdef __cplusplus
extern "C"
{
#endif

//--------------------------------------------------------------------
// enums
//--------------------------------------------------------------------

typedef enum
{
    // return types:
    JSON_ERROR,     // general error, eof etc.
    JSON_OBJECT,    // "{"
    JSON_ARRAY,     // "["
    JSON_STRING,    // "string" 
    JSON_NUMBER,    // number (may be -ve) int or float
    JSON_BOOL,      // true or false
    JSON_NULL,      // null
    JSON_KEY,       // object "key"
    // internal values:
    JSON_COLON,     // ":"
    JSON_EOL,       // end of input string (ptr at '\0')
    JSON_COMMA,     // ","
    JSON_EOBJECT,   // "}"
    JSON_EARRAY,    // "]"
    JSON_QPARAM     // "*" query string parameter
} json_type;

typedef enum
{
    JSON_OK,
    JSON_QUERY_MISMATCH,
    JSON_READ_ERROR,
    JSON_EXPECTED_KEY,
    JSON_EXPECTED_COLON,
    JSON_KEY_NOT_FOUND,
    JSON_EXPECTED_COMMA_OBJECT,
    JSON_TERMINAL_BEFORE_END,
    JSON_UNEXPECTED_CHARACTER,
    JSON_EXPECTED_COMMA_ARRAY,
    JSON_BAD_INDEX_ARRAY,
    JSON_BAD_INDEX_OBJECT,
    JSON_BAD_OBJECT_KEY,
    JSON_END_OF_ARRAY,
    JSON_END_OF_OBJECT
} json_error;

//--------------------------------------------------------------------
// json_element_t
// - structure to return JSON elements
// - error = 0 for valid returns
//
// *NOTES*
//    the returned value pointer points into the passed JSON
//    string returns are not '\0' terminated.
//    bytelen specifies the length of the returned data pointed to by pValue
typedef struct
{
	json_type data_type;    // type of the element
	int elements;           // number of elements (e.g. elements in array or object)
	int bytelen;            // byte length of element (e.g. length of string, array text "[ ... ]" etc.)
	void* value;            // pointer to value string in JSON text
	json_error error;       // error value if data_type == JSON_ERROR
} json_element_t;

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
//    json_read(json, "{'key'", &result);
// returns with: 
//    result.data_type = JREAD_STRING, result.value->'value', result.bytelen = 5
char* json_read(char* json, char* query, json_element_t* result);

// version of json_read which allows one or more queryParam integers to be substituted
// for array or object indexes marked by a '*' in the query
//
// e.g. json_read_param(json, "[*", &result_element, &array_index);
//
// *!* CAUTION *!*
// You can supply an array of integers which are indexed for each '*' in query
// however, horrid things will happen if you don't supply enough parameters
char* json_read_param(char* json, char* query, json_element_t* result, int* query_params);

// Array Stepping function
// assumes pJsonArray is JSON source of an array "[ ... ]"
// returns next element of the array in pResult
// returns pointer to end of element, to be passed to next call of jReadArrayStep()
// if end of array is encountered, pResult->error = 13 "End of array found"
//
// e.g.
//   With JSON like:   "{ ...  "arrayInObject":[ elem1,elem2,... ], ... }"
//
//   json = json_read(json, "{'arrayInObject'", &theArray);
//   if(theArray.data_type == JREAD_ARRAY)
//   {
//       char* array= (char*)theArray.value;
//       json_element_t arrayElement;
//       int index;
//       for(index = 0; index < theArray.elements; index++)
//       {
//           array = json_array_step(array, &arrayElement);
//           ...
//
// Note: this significantly speeds up traversing arrays.
char* json_array_step(char* json_array, json_element_t* result);

//--------------------------------------------------------------------
// Helper Functions
//--------------------------------------------------------------------

long    json_long(char* json, char* query, int* query_params);
int     json_int(char* json, char* query, int* query_params);
double  json_double(char* json, char* query, int* query_params);
int     json_string(char* json, char* query, char *dest, int destlen, int* query_params);

char* json_atoi(char* p, unsigned int* result); // string to unsigned int
char* json_atol(char* p, long* result);         // string to signed long
char* json_atof(char* p, double* result);       // string to double (does not do exponents)

int json_strcmp(json_element_t* j1, json_element_t* j2); // compare STRING elements
char* json_strcpy(char* dest_buffer, int dest_length, json_element_t* element); // copy element to '\0'-terminated buffer

//--------------------------------------------------------------------
// String output Functions
//--------------------------------------------------------------------

char* json_type_to_string(json_type data_type); // string describes data_type
char* json_error_to_string(json_error error);   // string descibes error code

#ifdef __cplusplus
}
#endif

//
//
////   end header file   /////////////////////////////////////////////////////
#endif // JSON_INCLUDE_H

//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
// Implementation
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------

#ifdef JSON_IMPLEMENTATION

#include <stdio.h>

// By default we use single quote in query strings so it's a lot easier
// to type in code i.e.  "{'key'" instead of "{\"key\""
#ifdef JSON_DOUBLE_QUOTE_IN_QUERY
#define JSON_QUERY_QUOTE    '\"'
#else
#define JSON_QUERY_QUOTE    '\''
#endif

//--------------------------------------------------------------------
// Internal Functions
//--------------------------------------------------------------------

static char* _json_skip_whitespace(char* sp)
{
    while ((*sp != '\0') && (*sp <= ' '))
        sp++;
    return sp;
};

// json_find_token
//  find start of a token
//
// returns:
//  pointer to start of next token or element
//  type via token_type
static char* _json_find_token(char* sp, json_type* token_type)
{
    char c;
    sp = _json_skip_whitespace(sp);
    c = *sp;
    if ( c == '\0' ) *token_type = JSON_EOL;
    else if ((c == '"') || (c == JSON_QUERY_QUOTE)) *token_type = JSON_STRING;
    else if ((c >= '0') && (c <= '9'))              *token_type = JSON_NUMBER;
    else if ((c == 't') || (c == 'f'))              *token_type = JSON_BOOL;
    else if (c == '-') *token_type = JSON_NUMBER;
    else if (c == '{') *token_type = JSON_OBJECT;
    else if (c == '[') *token_type = JSON_ARRAY;
    else if (c == '}') *token_type = JSON_EOBJECT; 
    else if (c == ']') *token_type = JSON_EARRAY;
    else if (c == 'n') *token_type = JSON_NULL;
    else if (c == ':') *token_type = JSON_COLON;
    else if (c == ',') *token_type = JSON_COMMA;
    else if (c == '*') *token_type = JSON_QPARAM;
    else *token_type = JSON_ERROR;
    return sp;
};

// json_get_string
//  assumes next element is "string" which may include "\" sequences
//  returns pointer to -------------^
//  element contains result (JSON_STRING, length, pointer to string)
//  pass quote = '"' for Json, quote = '\'' for query scanning
//
// returns:
//  pointer into json after the string (char after the " terminator)
//   element contains pointer and length of string (or data_type = JSON_ERROR)
static char* _json_get_string(char* json, json_element_t* element, char quote)
{
    short skip_ch;
    element->data_type = JSON_ERROR;
    element->elements = 1;
    element->bytelen = 0;
    json = _json_skip_whitespace(json);
    if (*json == quote)
    {
        json++;
        element->value = json; // -> start of actual string
        element->bytelen = 0;
        skip_ch = 0;
        while (*json != '\0')
        {
            if (skip_ch)
                skip_ch = 0;
            else if (*json == '\\') // "\" sequence
                skip_ch = 1;
            else if (*json == quote)
            {
                element->data_type = JSON_STRING;
                json++;
                break;
            }
            element->bytelen++;
            json++;
        };
    };
    return json;
};

// json_text_len
//  used to identify length of element text
//  terminators: ' ' , } ]
//
// returns:
//  no. of chars from json upto a terminator
static int _json_text_len(char* json)
{
    int len = 0;
    while ((*json >  ' ') && (*json != ',') && (*json != '}') && (*json != ']'))
    {
        len++;
        json++;
    }
    return len;
}

// json_count_object
//  used when query ends at an object, we want to return the object length
//  on entry json -> "{... "
//  used to skip unwanted values which are objects
//  keyIndex normally passed as -1 unless we're looking for the nth "key" value
//  in which case keyIndex is the index of the key we want
static char* _json_count_object(char* json, json_element_t* result, int key_index)
{
    json_element_t element;
    json_type token;
    char* sp;
    result->data_type = JSON_OBJECT;
    result->error = JSON_OK;
    result->elements = 0;
    result->value = json;
    sp = _json_find_token(json + 1, &token); // check for empty object
    if (token == JSON_EOBJECT)
    {
        json = sp + 1;
    }
    else
    {
        while (1)
        {
            json = _json_get_string(++json, &element, '\"');
            if (element.data_type != JSON_STRING)
            {
                result->error = JSON_EXPECTED_KEY; // Expected "key"
                break;
            }
            if (result->elements == key_index) // if passed keyIndex
            {
                *result = element; // we return "key" at this index
                result->data_type = JSON_KEY;
                return json;
            }
            json = _json_find_token(json, &token);
            if (token != JSON_COLON)
            {
                result->error = JSON_EXPECTED_COLON; // Expected ":"
                break;
            }
            json = json_read(++json, "", &element);
            if (result->error) break;
            json = _json_find_token(json, &token);
            result->elements++;
            if (token == JSON_EOBJECT)
            {
                json++;
                break;
            }
            if (token != JSON_COMMA)
            {
                result->error = JSON_EXPECTED_COMMA_OBJECT; // Expected "," in object
                break;
            }
        }
    }
    if (key_index >= 0)
    {
        // we wanted a "key" value - that we didn't find
        result->data_type = JSON_ERROR;
        result->error = JSON_BAD_INDEX_OBJECT; // Object key not found (bad index)
    }
    else
    {
        result->bytelen = json - (char*)result->value;
    }
    return json;
}

// json_count_array
//  used when query ends at an array, we want to return the array length
//  on entry json -> "[... "
//  used to skip unwanted values which are arrays
static char* _json_count_array(char* json, json_element_t* result)
{
    json_type token;
    char* sp = _json_find_token(json + 1, &token); // check for empty array
    result->data_type = JSON_ARRAY;
    result->error = JSON_OK;
    result->elements = 0;
    result->value = json;

    if (token == JSON_EARRAY)
    {
        json = sp + 1;
    }
    else
    {
        json_element_t element;
        while (1)
        {
            json = json_read(++json, "", &element); // array value
            if(result->error) break;
            json = _json_find_token(json, &token); // , or ]
            result->elements++;
            if (token == JSON_EARRAY)
            {
                json++;
                break;
            }
            if (token != JSON_COMMA)
            {
                result->error = JSON_EXPECTED_COMMA_ARRAY;
                break;
            }
        }
    }

    result->bytelen = json - (char*)result->value;
    return json;
}

//--------------------------------------------------------------------
// JSON reader function
//--------------------------------------------------------------------

// json_read
//  reads a complete JSON <value>
//  matches query against json, results in result
//
// returns:
//  pointer into json
//
// Note: is recursive
char* json_read(char* json, char* query, json_element_t* result)
{
    return json_read_param(json, query, result, NULL);
}

char* json_read_param(char* json, char* query, json_element_t* result, int* query_params)
{
    json_type token_q, token_j;
    int bytelen;
    unsigned int index, count;
    json_element_t element_q, element_j;

    json = _json_find_token(json, &token_j);
    query = _json_find_token(query, &token_q);

    result->data_type = token_j;
    result->bytelen = result->elements = 0;
    result->error = JSON_OK;
    result->value = json;

    if ((token_q != JSON_EOL) && (token_q != token_j))
    {
        result->error= JSON_QUERY_MISMATCH; // JSON does not match Query
        return json;
    }

    switch (token_j)
    {
    case JSON_ERROR: // general error, eof etc.
        result->error= JSON_READ_ERROR;
        break;

    case JSON_OBJECT: // "{"
        if (token_q == JSON_EOL)
            return _json_count_object(json, result, -1); // return length of object 

        query = _json_find_token(++query, &token_q); // "('key'...", "{NUMBER", "{*" or EOL
        if (token_q != JSON_STRING)
        {
            index = 0;
            switch (token_q)
            {
            case JSON_NUMBER:
                query = json_atoi((char*)query, &index); // index value
                break;
            case JSON_QPARAM:
                query++;
                index = (query_params != NULL) ? *query_params++ : 0; // substitute parameter
                break;
            default:
                result->error = JSON_BAD_OBJECT_KEY;
                return json;
            }
            return _json_count_object(json, result, index);
        }

        query = _json_get_string(query, &element_q, JSON_QUERY_QUOTE); // element_q = query 'key'

        // read <key> : <value> , ... }
        // loop 'til key matched
        while (1)
        {
            json = _json_get_string(++json, &element_j, '\"');
            if (element_j.data_type != JSON_STRING)
            {
                result->error = JSON_EXPECTED_KEY;
                break;
            }
            json = _json_find_token(json, &token_j);
            if (token_j != JSON_COLON)
            {
                result->error = JSON_EXPECTED_COLON;
                break;
            }
            // compare object keys
            if (json_strcmp(&element_q, &element_j) == 0)
            {
                // found object key
                return json_read_param(++json, query, result, query_params);
            }
            // no key match... skip this value
            json = json_read(++json, "", result);
            json = _json_find_token(json, &token_j);
            if (token_j == JSON_EOBJECT)
            {
                result->error = JSON_KEY_NOT_FOUND;
                break;
            }
            if (token_j != JSON_COMMA)
            {
                result->error = JSON_EXPECTED_COMMA_OBJECT;
                break;
            }
        }
        break;
    case JSON_ARRAY: // "[NUMBER" or "[*"
        // read index, skip values 'til index
        if (token_q == JSON_EOL)
            return _json_count_array(json, result); // return length of object 

        index = 0;
        query = _json_find_token(++query, &token_q); // "[NUMBER" or "[*"
        if (token_q == JSON_NUMBER)
        {
            query = json_atoi(query, &index); // get array index
        }
        else if (token_q == JSON_QPARAM)
        {
            query++;
            index = (query_params != NULL) ? *query_params++ : 0; // substitute parameter
        }

        count = 0;
        while (1)
        {
            if (count == index)
                return json_read_param(++json, query, result, query_params); // return value at index

            // not this index... skip this value
            json = json_read(++json, "", &element_j);
            if (result->error)
                break;
            count++;
            json = _json_find_token(json, &token_j); // , or ]
            if (token_j == JSON_EARRAY)
            {
                result->error = JSON_BAD_INDEX_ARRAY;
                break;
            }
            if (token_j != JSON_COMMA)
            {
                result->error = JSON_EXPECTED_COMMA_ARRAY;
                break;
            }
        }
        break;
    case JSON_STRING: // "string" 
        json = _json_get_string(json, result, '\"');
        break;
    case JSON_NUMBER: // number (may be -ve) int or float
    case JSON_BOOL: // true or false
    case JSON_NULL: // null
        bytelen = _json_text_len(json);
        result->data_type = token_j;
        result->bytelen = bytelen;
        result->value = json;
        result->elements = 1;
        json += bytelen;
        break;
    default:
        result->error = JSON_UNEXPECTED_CHARACTER;
    }
    // We get here on a 'terminal value'
    // - make sure the query string is empty also
    query = _json_find_token(query, &token_q);
    if (!result->error && (token_q != JSON_EOL))
        result->error = JSON_TERMINAL_BEFORE_END;
    if (result->error)
    {
        result->data_type = JSON_ERROR;
        result->elements = result->bytelen = 0;
        result->value = json; // return pointer into JSON at error point
    }
    return json;
}

// json_array_step
//  reads one value from an array
//  assumes pJsonArray points at the start of an array or array element
char* json_array_step(char* json_array, json_element_t* result)
{
    json_type token;
    json_array = _json_find_token(json_array, &token);
    switch (token)
    {
    case JSON_ARRAY: // start of array
    case JSON_COMMA: // element separator
        return json_read(++json_array, "", result);
    case JSON_EARRAY: // end of array
        result->error = JSON_END_OF_ARRAY;
        break;
    default: // some other error
        result->error = JSON_EXPECTED_COMMA_ARRAY;
        break;
    }
    result->data_type = JSON_ERROR;
    return json_array;
}

//--------------------------------------------------------------------
// helper functions
//--------------------------------------------------------------------

// - simple routines to extract values from JSON
// - does coercion of types where possible
// - always returns a value (e.g. 0 or "" on error)
//
// Note: by default, pass NULL for queryParams
//       unless you are using '*' in the query for indexing

// json_long
// - reads signed long value from JSON 
// - returns number from NUMBER or STRING elements (if possible)
//   returns 1 or 0 from BOOL elements
//   otherwise returns 0
long json_long(char* json, char* query, int* query_params)
{
    json_element_t elem;
    long result;
    json_read_param(json, query, &elem, query_params);
    if ((elem.data_type == JSON_ERROR) || (elem.data_type == JSON_NULL))
        return 0;
    if (elem.data_type == JSON_BOOL)
        return *((char *)elem.value)=='t' ? 1 : 0;

    json_atol((char *)elem.value, &result);
    return result;
}

int json_int(char* json, char* query, int* query_params)
{
    return (int)json_long(json, query, query_params);
}

// json_double
// - returns double from JSON
// - returns number from NUMBER or STRING elements
//   otherwise returns 0.0
double json_double(char* json, char* query, int* query_params)
{
    json_element_t element;
    json_read_param(json, query, &element, query_params);
    if (element.data_type == JSON_ERROR)
        return 0.0;
    
    double result;
    json_atof((char*)element.value, &result);
    return result;
}

// json_string
// Copy string to dest and '\0'-terminate it (upto dest_len total bytes)
// returns: character length of string (excluding '\0' terminator)
//
// Note: any element can be returned as a string
int json_string(char* json, char* query, char* dest, int destlen, int* query_params)
{
    json_element_t element;

    *dest = '\0';
    json_read_param(json, query, &element, query_params);
    if (element.data_type == JSON_ERROR)
        return 0;

    for (int i = 0; (i < element.bytelen) && (i < destlen - 1); i++)
        *dest++ = ((char*)element.value)[i];
    *dest = '\0';
    return element.bytelen;
}

// read unsigned int from string
char* json_atoi(char* p, unsigned int* result)
{
    unsigned int x = 0;
    while (*p >= '0' && *p <= '9')
    {
        x = (x*10) + (*p - '0');
        ++p;
    }
    *result = x;
    return p;
}

// read long int from string
char* json_atol(char* p, long* result)
{
    long x = 0;
    int neg = 0;
    if (*p == '-')
    {
        neg = 1;
        ++p;
    }
    while (*p >= '0' && *p <= '9')
    {
        x = (x*10) + (*p - '0');
        ++p;
    }
    if (neg)
    {
        x = -x;
    }
    *result= x;
    return p;
}

// read double from string
// *CAUTION* does not handle exponents
char* json_atof(char* p, double* result)
{
#define VALID_DIGIT(c) ((c) >= '0' && (c) <= '9')

    double sign, value;

    // Get sign, if any.
    sign = 1.0;
    if (*p == '-')
    {
        sign = -1.0;
        p += 1;

    } 
    else if (*p == '+')
    {
        p += 1;
    }

    // Get digits before decimal point or exponent, if any.
    for (value = 0.0; VALID_DIGIT(*p); p += 1)
    {
        value = value * 10.0 + (*p - '0');
    }

    // Get digits after decimal point, if any.
    if (*p == '.')
    {
        double pow10 = 10.0;
        p += 1;
        while (VALID_DIGIT(*p))
        {
            value += (*p - '0') / pow10;
            pow10 *= 10.0;
            p += 1;
        }
    }
    *result= sign * value;

#undef VALID_DIGIT
    return p;
}

// compare two json elements
// returns: 0 if they are identical strings, else 1
int json_strcmp(json_element_t* j1, json_element_t* j2)
{
    int i;
    if ((j1->data_type != JSON_STRING) || 
        (j2->data_type != JSON_STRING) ||
        (j1->bytelen != j2->bytelen ))
        return 1;

    for (i = 0; i < j1->bytelen; i++)
        if (((char *)(j1->value))[i] != ((char*)(j2->value))[i])
            return 1;
    return 0; 
}

// read element into destination buffer and add '\0' terminator
// always copies element irrespective of data_type (unless it's an error)
// dest_buffer is always '\0'-terminated (even on zero lenght returns)
// returns pointer to dest_buffer
char* json_strcpy(char* dest_buffer, int dest_length, json_element_t* element)
{
    int len = element->bytelen;
    char* dest = dest_buffer;
    char* src = (char*)element->value;
    if (element->error == 0)
    {
        if(len >= dest_length)
            len = dest_length;
        for(int i = 0; i < dest_length; i++)
            *dest++ = *src++;
    }
    *dest= '\0';
    return dest_buffer;
}

//--------------------------------------------------------------------
// String output Functions
//--------------------------------------------------------------------

char* json_type_to_string(json_type data_type)
{
    switch (data_type)
    {
    case JSON_ERROR:    return "Error";
    case JSON_OBJECT:   return "Object";
    case JSON_ARRAY:    return "Array";
    case JSON_STRING:   return "String";
    case JSON_NUMBER:   return "Number";
    case JSON_BOOL:     return "Bool";
    case JSON_NULL:     return "null";
    case JSON_KEY:      return "Object key";
    case JSON_COLON:    return "colon";
    case JSON_EOL:      return "eol";
    case JSON_COMMA:    return "comma";
    case JSON_EOBJECT:  return "}";
    case JSON_EARRAY:   return "]";
    case JSON_QPARAM:   return "* parameter";
    default:            return "Unkown type";
    }
};

char* json_error_to_string(json_error error)
{
    switch (error)
    {
    case JSON_OK:                   return "Ok";
    case JSON_QUERY_MISMATCH:       return "JSON does not match Query";
    case JSON_READ_ERROR:           return "Error reading JSON value";
    case JSON_EXPECTED_KEY:         return "Expected \"key\"";
    case JSON_EXPECTED_COLON:       return "Expected ':'";
    case JSON_KEY_NOT_FOUND:        return "Object key not found";
    case JSON_EXPECTED_COMMA_OBJECT:return "Expected ',' in object";
    case JSON_TERMINAL_BEFORE_END:  return "Terminal value found before end of query";
    case JSON_UNEXPECTED_CHARACTER: return "Unexpected character";
    case JSON_EXPECTED_COMMA_ARRAY: return "Expected ',' in array";
    case JSON_BAD_INDEX_ARRAY:      return "Array element not found (bad index)";
    case JSON_BAD_INDEX_OBJECT:     return "Object key not found (bad index)";
    case JSON_BAD_OBJECT_KEY:       return "Bad object key";
    case JSON_END_OF_ARRAY:         return "End of array found";
    case JSON_END_OF_OBJECT:        return "End of object found";
    default:                        return "Unknown error";
    }
};

#endif // JSON_IMPLEMENTATION

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