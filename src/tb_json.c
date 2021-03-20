#include "tb_json.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// By default we use single quote in query strings so it's a lot easier
// to type in code i.e.  "{'key'" instead of "{\"key\""
#ifdef TB_JSON_DOUBLE_QUOTE_IN_QUERY
#define TB_JSON_QUERY_QUOTE    '\"'
#else
#define TB_JSON_QUERY_QUOTE    '\''
#endif

//--------------------------------------------------------------------
// Internal Functions
//--------------------------------------------------------------------

static char* _tb_json_skip_whitespace(char* sp)
{
    while ((*sp != '\0') && (*sp <= ' '))
        sp++;
    return sp;
};

// _tb_json_find_token
//  find start of a token
//
// returns:
//  pointer to start of next token or element
//  type via token_type
static char* _tb_json_find_token(char* sp, tb_json_type* token_type)
{
    char c;
    sp = _tb_json_skip_whitespace(sp);
    c = *sp;
    if (c == '\0') *token_type = TB_JSON_EOL;
    else if ((c == '"') || (c == TB_JSON_QUERY_QUOTE))  *token_type = TB_JSON_STRING;
    else if ((c >= '0') && (c <= '9'))                  *token_type = TB_JSON_NUMBER;
    else if ((c == 't') || (c == 'f'))                  *token_type = TB_JSON_BOOL;
    else if (c == '-') *token_type = TB_JSON_NUMBER;
    else if (c == '{') *token_type = TB_JSON_OBJECT;
    else if (c == '[') *token_type = TB_JSON_ARRAY;
    else if (c == '}') *token_type = TB_JSON_EOBJECT;
    else if (c == ']') *token_type = TB_JSON_EARRAY;
    else if (c == 'n') *token_type = TB_JSON_NULL;
    else if (c == ':') *token_type = TB_JSON_COLON;
    else if (c == ',') *token_type = TB_JSON_COMMA;
    else if (c == '*') *token_type = TB_JSON_QPARAM;
    else *token_type = TB_JSON_ERROR;
    return sp;
};

// _tb_json_get_string
//  assumes next element is "string" which may include "\" sequences
//  returns pointer to -------------^
//  element contains result (TB_JSON_STRING, length, pointer to string)
//  pass quote = '"' for Json, quote = '\'' for query scanning
//
// returns:
//  pointer into json after the string (char after the " terminator)
//   element contains pointer and length of string (or data_type = JSON_ERROR)
static char* _tb_json_get_string(char* json, tb_json_element* element, char quote)
{
    short skip_ch;
    element->data_type = TB_JSON_ERROR;
    element->elements = 1;
    element->bytelen = 0;
    json = _tb_json_skip_whitespace(json);
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
                element->data_type = TB_JSON_STRING;
                json++;
                break;
            }
            element->bytelen++;
            json++;
        };
    };
    return json;
};

// _tb_json_text_len
//  used to identify length of element text
//  terminators: ' ' , } ]
//
// returns:
//  no. of chars from json upto a terminator
static int _tb_json_text_len(char* json)
{
    int len = 0;
    while ((*json > ' ') && (*json != ',') && (*json != '}') && (*json != ']'))
    {
        len++;
        json++;
    }
    return len;
}

// _tb_json_count_object
//  used when query ends at an object, we want to return the object length
//  on entry json -> "{... "
//  used to skip unwanted values which are objects
//  keyIndex normally passed as -1 unless we're looking for the nth "key" value
//  in which case keyIndex is the index of the key we want
static char* _tb_json_count_object(char* json, tb_json_element* result, int key_index)
{
    tb_json_element element;
    tb_json_type token;
    char* sp;
    result->data_type = TB_JSON_OBJECT;
    result->error = TB_JSON_OK;
    result->elements = 0;
    result->value = json;
    sp = _tb_json_find_token(json + 1, &token); // check for empty object
    if (token == TB_JSON_EOBJECT)
    {
        json = sp + 1;
    }
    else
    {
        while (1)
        {
            json = _tb_json_get_string(++json, &element, '\"');
            if (element.data_type != TB_JSON_STRING)
            {
                result->error = TB_JSON_EXPECTED_KEY; // Expected "key"
                break;
            }
            if (result->elements == key_index) // if passed keyIndex
            {
                *result = element; // we return "key" at this index
                result->data_type = TB_JSON_KEY;
                return json;
            }
            json = _tb_json_find_token(json, &token);
            if (token != TB_JSON_COLON)
            {
                result->error = TB_JSON_EXPECTED_COLON; // Expected ":"
                break;
            }
            json = tb_json_read(++json, &element, "");
            if (result->error) break;
            json = _tb_json_find_token(json, &token);
            result->elements++;
            if (token == TB_JSON_EOBJECT)
            {
                json++;
                break;
            }
            if (token != TB_JSON_COMMA)
            {
                result->error = TB_JSON_EXPECTED_COMMA_OBJECT; // Expected "," in object
                break;
            }
        }
    }
    if (key_index >= 0)
    {
        // we wanted a "key" value - that we didn't find
        result->data_type = TB_JSON_ERROR;
        result->error = TB_JSON_BAD_INDEX_OBJECT; // Object key not found (bad index)
    }
    else
    {
        result->bytelen = (int)(json - (char*)result->value);
    }
    return json;
}

// _tb_json_count_array
//  used when query ends at an array, we want to return the array length
//  on entry json -> "[... "
//  used to skip unwanted values which are arrays
static char* _tb_json_count_array(char* json, tb_json_element* result)
{
    tb_json_type token;
    char* sp = _tb_json_find_token(json + 1, &token); // check for empty array
    result->data_type = TB_JSON_ARRAY;
    result->error = TB_JSON_OK;
    result->elements = 0;
    result->value = json;

    if (token == TB_JSON_EARRAY)
    {
        json = sp + 1;
    }
    else
    {
        tb_json_element element;
        while (1)
        {
            json = tb_json_read(++json, &element, ""); // array value
            if (result->error) break;
            json = _tb_json_find_token(json, &token); // , or ]
            result->elements++;
            if (token == TB_JSON_EARRAY)
            {
                json++;
                break;
            }
            if (token != TB_JSON_COMMA)
            {
                result->error = TB_JSON_EXPECTED_COMMA_ARRAY;
                break;
            }
        }
    }

    result->bytelen = (int)(json - (char*)result->value);
    return json;
}

//--------------------------------------------------------------------
// JSON reader function
//--------------------------------------------------------------------

// tb_json_read
//  reads a complete JSON <value>
//  matches query against json, results in result
//
// returns:
//  pointer into json
//
// Note: is recursive
char* tb_json_read(char* json, tb_json_element* result, char* query)
{
    return tb_json_read_param(json, result, query, NULL);
}

char* tb_json_read_format(char* json, tb_json_element* result, const char* query_fmt, ...)
{
    va_list args;
    va_start(args, query_fmt);
    size_t query_size = vsnprintf(NULL, 0, query_fmt, args);
    char* query = malloc(query_size + 1);
    vsnprintf(query, query_size + 1, query_fmt, args);
    va_end(args);

    json = tb_json_read_param(json, result, query, NULL);

    free(query);

    return json;
}

char* tb_json_read_param(char* json, tb_json_element* result, char* query, int* query_params)
{
    tb_json_type token_q, token_j;
    int bytelen;
    unsigned int index, count;
    tb_json_element element_q, element_j;

    json = _tb_json_find_token(json, &token_j);
    query = _tb_json_find_token(query ? query : "", &token_q);

    result->data_type = token_j;
    result->bytelen = result->elements = 0;
    result->error = TB_JSON_OK;
    result->value = json;

    if ((token_q != TB_JSON_EOL) && (token_q != token_j))
    {
        result->error = TB_JSON_QUERY_MISMATCH; // JSON does not match Query
        return json;
    }

    switch (token_j)
    {
    case TB_JSON_ERROR: // general error, eof etc.
        result->error = TB_JSON_READ_ERROR;
        break;

    case TB_JSON_OBJECT: // "{"
        if (token_q == TB_JSON_EOL)
            return _tb_json_count_object(json, result, -1); // return length of object 

        query = _tb_json_find_token(++query, &token_q); // "('key'...", "{NUMBER", "{*" or EOL
        if (token_q != TB_JSON_STRING)
        {
            index = 0;
            switch (token_q)
            {
            case TB_JSON_NUMBER:
                query = tb_json_atoi(query, &index); // index value
                break;
            case TB_JSON_QPARAM:
                query++;
                index = (query_params != NULL) ? *query_params++ : 0; // substitute parameter
                break;
            default:
                result->error = TB_JSON_BAD_OBJECT_KEY;
                return json;
            }
            return _tb_json_count_object(json, result, index);
        }
        query = _tb_json_get_string(query, &element_q, TB_JSON_QUERY_QUOTE); // element_q = query 'key'

        // read <key> : <value> , ... } loop until key matched
        while (1)
        {
            json = _tb_json_get_string(++json, &element_j, '\"');
            if (element_j.data_type != TB_JSON_STRING)
            {
                result->error = TB_JSON_EXPECTED_KEY;
                break;
            }
            json = _tb_json_find_token(json, &token_j);
            if (token_j != TB_JSON_COLON)
            {
                result->error = TB_JSON_EXPECTED_COLON;
                break;
            }
            // compare object keys
            if (tb_json_strcmp(&element_q, &element_j) == 0)
            {
                // found object key
                return tb_json_read_param(++json, result, query, query_params);
            }
            // no key match... skip this value
            json = tb_json_read(++json, result, "");
            json = _tb_json_find_token(json, &token_j);
            if (token_j == TB_JSON_EOBJECT)
            {
                result->error = TB_JSON_KEY_NOT_FOUND;
                break;
            }
            if (token_j != TB_JSON_COMMA)
            {
                result->error = TB_JSON_EXPECTED_COMMA_OBJECT;
                break;
            }
        }
        break;
    case TB_JSON_ARRAY: // "[NUMBER" or "[*"
        // read index, skip values until index
        if (token_q == TB_JSON_EOL)
            return _tb_json_count_array(json, result); // return length of object 

        index = 0;
        query = _tb_json_find_token(++query, &token_q); // "[NUMBER" or "[*"
        if (token_q == TB_JSON_NUMBER)
        {
            query = tb_json_atoi(query, &index); // get array index
        }
        else if (token_q == TB_JSON_QPARAM)
        {
            query++;
            index = (query_params != NULL) ? *query_params++ : 0; // substitute parameter
        }

        count = 0;
        while (1)
        {
            if (count == index)
                return tb_json_read_param(++json, result, query, query_params); // return value at index

            // not this index... skip this value
            json = tb_json_read(++json, &element_j, "");
            if (result->error)
                break;
            count++;
            json = _tb_json_find_token(json, &token_j); // , or ]
            if (token_j == TB_JSON_EARRAY)
            {
                result->error = TB_JSON_BAD_INDEX_ARRAY;
                break;
            }
            if (token_j != TB_JSON_COMMA)
            {
                result->error = TB_JSON_EXPECTED_COMMA_ARRAY;
                break;
            }
        }
        break;
    case TB_JSON_STRING: // "string" 
        json = _tb_json_get_string(json, result, '\"');
        break;
    case TB_JSON_NUMBER: // number (may be -ve) int or float
    case TB_JSON_BOOL: // true or false
    case TB_JSON_NULL: // null
        bytelen = _tb_json_text_len(json);
        result->data_type = token_j;
        result->bytelen = bytelen;
        result->value = json;
        result->elements = 1;
        json += bytelen;
        break;
    default:
        result->error = TB_JSON_UNEXPECTED_CHARACTER;
    }
    // We get here on a 'terminal value'
    // - make sure the query string is empty also
    query = _tb_json_find_token(query, &token_q);
    if (!result->error && (token_q != TB_JSON_EOL))
        result->error = TB_JSON_TERMINAL_BEFORE_END;
    if (result->error)
    {
        result->data_type = TB_JSON_ERROR;
        result->elements = result->bytelen = 0;
        result->value = json; // return pointer into JSON at error point
    }
    return json;
}

// tb_json_array_step
//  reads one value from an array
//  assumes json_array points at the start of an array or array element
char* tb_json_array_step(char* json_array, tb_json_element* result)
{
    tb_json_type token;
    json_array = _tb_json_find_token(json_array, &token);
    switch (token)
    {
    case TB_JSON_ARRAY:     // start of array
    case TB_JSON_COMMA:     // element separator
        return tb_json_read(++json_array, result, "");
    case TB_JSON_EARRAY:    // end of array
        result->error = TB_JSON_END_OF_ARRAY;
        break;
    default: // some other error
        result->error = TB_JSON_EXPECTED_COMMA_ARRAY;
        break;
    }
    result->data_type = TB_JSON_ERROR;
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

// tb_json_long
// - reads signed long value from JSON 
// - returns number from NUMBER or STRING elements (if possible)
//   returns 1 or 0 from BOOL elements
//   otherwise returns 0
long tb_json_long(char* json, char* query, int* query_params, long default_value)
{
    tb_json_element elem;
    long result;
    tb_json_read_param(json, &elem, query, query_params);
    if ((elem.data_type == TB_JSON_ERROR) || (elem.data_type == TB_JSON_NULL))
        return default_value;
    if (elem.data_type == TB_JSON_BOOL)
        return *(elem.value) == 't' ? 1 : 0;

    tb_json_atol(elem.value, &result);
    return result;
}

int tb_json_int(char* json, char* query, int* query_params, int default_value)
{
    return (int)tb_json_long(json, query, query_params, default_value);
}

// tb_json_float
// - returns float from JSON
// - returns number from NUMBER or STRING elements
//   otherwise returns 0.0f
float tb_json_float(char* json, char* query, int* query_params, float default_value)
{
    tb_json_element element;
    tb_json_read_param(json, &element, query, query_params);
    if (element.data_type == TB_JSON_ERROR)
        return default_value;

    float result;
    tb_json_atof((char*)element.value, &result);
    return result;
}

// tb_json_string
// Copy string to dest and '\0'-terminate it (upto dest_len total bytes)
// returns: character length of string (excluding '\0' terminator)
//
// Note: any element can be returned as a string
int tb_json_string(char* json, char* query, char* dest, int destlen, int* query_params)
{
    tb_json_element element;

    *dest = '\0';
    tb_json_read_param(json, &element, query, query_params);
    if (element.data_type == TB_JSON_ERROR)
        return 0;

    for (int i = 0; (i < element.bytelen) && (i < destlen - 1); i++)
        *dest++ = ((char*)element.value)[i];
    *dest = '\0';
    return element.bytelen;
}

int tb_json_parse(char* json, char* query, int* query_params, tb_json_parse_func parse)
{
    tb_json_element element;
    tb_json_read_param(json, &element, query, query_params);

    return parse(element.value, element.bytelen);
}

// read unsigned int from string
char* tb_json_atoi(char* p, unsigned int* result)
{
    unsigned int x = 0;
    while (*p >= '0' && *p <= '9')
    {
        x = (x * 10) + (*p - '0');
        ++p;
    }
    *result = x;
    return p;
}

// read long int from string
char* tb_json_atol(char* p, long* result)
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
        x = (x * 10) + (*p - '0');
        ++p;
    }
    if (neg)
    {
        x = -x;
    }
    *result = x;
    return p;
}

// read double from string
// *CAUTION* does not handle exponents
char* tb_json_atof(char* p, float* result)
{
#define VALID_DIGIT(c) ((c) >= '0' && (c) <= '9')

    float sign, value;

    // Get sign, if any.
    sign = 1.0f;
    if (*p == '-')
    {
        sign = -1.0f;
        p += 1;

    }
    else if (*p == '+')
    {
        p += 1;
    }

    // Get digits before decimal point or exponent, if any.
    for (value = 0.0f; VALID_DIGIT(*p); p += 1)
    {
        value = value * 10.0f + (float)(*p - '0');
    }

    // Get digits after decimal point, if any.
    if (*p == '.')
    {
        float pow10 = 10.0f;
        p += 1;
        while (VALID_DIGIT(*p))
        {
            value += (float)(*p - '0') / pow10;
            pow10 *= 10.0f;
            p += 1;
        }
    }
    *result = sign * value;

#undef VALID_DIGIT
    return p;
}

// compare two json elements
// returns: 0 if they are identical strings, else 1
int tb_json_strcmp(const tb_json_element* a, const tb_json_element* b)
{
    if ((a->data_type != TB_JSON_STRING) || (b->data_type != TB_JSON_STRING) 
        || (a->bytelen != b->bytelen))
        return 1;

    for (int i = 0; i < a->bytelen; i++)
        if (a->value[i] != b->value[i])
            return 1;
    return 0;
}

// read element into destination buffer and add '\0' terminator
// always copies element irrespective of data_type (unless it's an error)
// dest_buffer is always '\0'-terminated (even on zero lenght returns)
// returns pointer to dest_buffer
char* tb_json_strcpy(char* dest_buffer, const tb_json_element* element)
{
    char* dest = dest_buffer;
    char* src = element->value;
    if (element->error == 0)
    {
        for (int i = 0; i < element->bytelen; i++)
            *dest++ = *src++;
    }
    *dest = '\0';
    return dest_buffer;
}

int tb_json_is_type(const tb_json_element* element, tb_json_type type)
{
    return element->error == TB_JSON_OK && element->data_type == type;
}

// prints the value of an element
void tb_json_print_element(tb_json_element element)
{
    printf("%.*s\n", element.bytelen, (char*)element.value);
}

//--------------------------------------------------------------------
// String output Functions
//--------------------------------------------------------------------

char* tb_json_type_to_string(tb_json_type data_type)
{
    switch (data_type)
    {
    case TB_JSON_ERROR:     return "Error";
    case TB_JSON_OBJECT:    return "Object";
    case TB_JSON_ARRAY:     return "Array";
    case TB_JSON_STRING:    return "String";
    case TB_JSON_NUMBER:    return "Number";
    case TB_JSON_BOOL:      return "Bool";
    case TB_JSON_NULL:      return "null";
    case TB_JSON_KEY:       return "Object key";
    case TB_JSON_COLON:     return "colon";
    case TB_JSON_EOL:       return "eol";
    case TB_JSON_COMMA:     return "comma";
    case TB_JSON_EOBJECT:   return "}";
    case TB_JSON_EARRAY:    return "]";
    case TB_JSON_QPARAM:    return "* parameter";
    default:                return "Unkown type";
    }
};

char* tb_json_error_to_string(tb_json_error error)
{
    switch (error)
    {
    case TB_JSON_OK:                    return "Ok";
    case TB_JSON_QUERY_MISMATCH:        return "JSON does not match Query";
    case TB_JSON_READ_ERROR:            return "Error reading JSON value";
    case TB_JSON_EXPECTED_KEY:          return "Expected \"key\"";
    case TB_JSON_EXPECTED_COLON:        return "Expected ':'";
    case TB_JSON_KEY_NOT_FOUND:         return "Object key not found";
    case TB_JSON_EXPECTED_COMMA_OBJECT: return "Expected ',' in object";
    case TB_JSON_TERMINAL_BEFORE_END:   return "Terminal value found before end of query";
    case TB_JSON_UNEXPECTED_CHARACTER:  return "Unexpected character";
    case TB_JSON_EXPECTED_COMMA_ARRAY:  return "Expected ',' in array";
    case TB_JSON_BAD_INDEX_ARRAY:       return "Array element not found (bad index)";
    case TB_JSON_BAD_INDEX_OBJECT:      return "Object key not found (bad index)";
    case TB_JSON_BAD_OBJECT_KEY:        return "Bad object key";
    case TB_JSON_END_OF_ARRAY:          return "End of array found";
    case TB_JSON_END_OF_OBJECT:         return "End of object found";
    default:                            return "Unknown error";
    }
};