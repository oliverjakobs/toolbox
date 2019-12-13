// json.cpp 
// Version 1v6
//
// json - an in-place JSON element reader
// =======================================
//
// Instead of parsing JSON into some structure, this maintains the input JSON as unaltered text
// and allows queries to be made on it directly.
//
// e.g. with the simple JSON:
//		{
//			"astring":"This is a string",
//			"anumber":42,
//			"myarray":[ "one", 2, {"description":"element 3"}, null ],
//			"yesno":true,
//			"HowMany":"1234",
//			"foo":null
//		}
//
// calling:
//		json_read(json, "{'myarray'[0", &elem);
//
// would return:
//		elem.data_type = JREAD_STRING;
//		elem.elements = 1
//		elem.bytelen = 3
//		elem.value -> "one"
//
// or you could call the helper functions:
//		jRead_string( json, "{'astring'", destString, MAXLEN );
//		jRead_int( json, "{'anumber'", &myint );
//		jRead_string( json, "{'myarray'[3", destString, MAXLEN );
//		etc.
//
// Note that the helper functions do type coersion and always return a value
// (on error an empty string is returned or value of zero etc.)
//
// The query string simply defines the route to the required data item
// as an arbitary list of object or array specifiers:
//		object element=			"{'keyname'"
//		array element=			"[INDEX"
//
// The json_read() function fills a json_element_t structure to describe the located element
// this can be used to locate any element, not just terminal values
// e.g.
//		json_read( json, "{'myarray'", &jElem );
//
// in this case jElem would contain:
//		jElem.dataType= JSON_ARRAY
//		jElem.elements= 4
//		jElem.bytelen= 46
//		jElem.pValue -> [ "one", 2, {"descripton":"element 3"}, null ] ...
//
// allowing json_read to be called again on the array:
// e.g.
//		json_read( jElem.pValue, "[3", &jElem );		// get 4th element - the null value
//
//		.oO!	see main.c runExamples() for a whole bunch of examples		!Oo.
//             -------------------------------------------------------
//
// Note that jRead never modifies the source JSON and does not allocate any memory.
// i.e. elements are returned as pointer and length into the source text.
//
// Functions
// =========
// Main JSON reader:
//		int		json_read( char * JsonSource, char *query, json_element_t &pResult );
//
// Extended function using query parameters for indexing:
//		int		json_read( char * JsonSource, char *query, json_element_t &pResult, int *queryParams );
//
// Function to step thru JSON arrays instead of indexing:
//      char* json_array_step(char *json_array, json_element_t* result);
//
// Optional Helper functions:
//		long	jRead_long( char *pJson, char *pQuery );
//		int		jRead_int( char *pJson, char *pQuery );
//		double	jRead_double( char *pJson, char *pQuery );
//		int		jRead_string( char *pJson, char *pQuery, char *pDest, int destlen );
//
// Optional String output Functions
//		char *	jReadTypeToString( int dataType );		// string describes dataType
//		char *	jReadErrorToString( int error );		// string descibes error code
//
// *NEW* in 1v2
// - "{NUMBER" returns the "key" value at that index within an object
// - jReadParam() adds queryParams which can be used as indexes into arrays (or into 
//   objects to return key values) by specifying '*' in the query string
//   e.g. jReadParam( pJson, "[*", &result, &index ) 
// *NEW in 1v4
// - fixed a couple of error return values
// - added #define JREAD_DOUBLE_QUOTE_IN_QUERY
// *NEW* in 1v5  (11mar2015)
// - fixed null ptr if '[*' used when null param passed
// *NEW* in 1v6  (24sep2016)
// - fixed handling of empty arrays and objects
//
// TonyWilk, 24sep2016 
// mail at tonywilk . co .uk
//
// License: "Free as in You Owe Me a Beer"
// - actually, since some people really worry about licenses, you are free to apply
//   whatever licence you want.
//
// Note: jRead_atol() and jRead_atof() are modified from original routines
//       fast_atol() and fast_atof() 09-May-2009 Tom Van Baak (tvb) www.LeapSecond.com
//
//       You may want to replace the use of jRead_atol() and jRead_atof() in helper functions
//       of your own. Especially note that my atof does not handle exponents.
//
//
#include <stdio.h>

#include "json.h"

// By default we use single quote in query strings so it's a lot easier
// to type in code i.e.  "{'key'" instead of "{\"key\""
//
#ifdef JREAD_DOUBLE_QUOTE_IN_QUERY
#define QUERY_QUOTE	'\"'
#else
#define QUERY_QUOTE '\''
#endif

//------------------------------------------------------
// Internal Functions

//=======================================================

char* json_skip_whitespace(char* sp)
{
    while((*sp != '\0') && (*sp <= ' '))
        sp++;
    return sp;
};


// Find start of a token
// - returns pointer to start of next token or element
//   returns type via token_type
//
char* json_find_token(char* sp, int* token_type)
{
    char c;
    sp = json_skip_whitespace(sp);
    c = *sp;
    if( c == '\0' )	*token_type= JREAD_EOL;
    else if((c == '"') || (c == QUERY_QUOTE))*token_type= JREAD_STRING;
    else if((c >= '0') && (c <= '9')) *token_type= JREAD_NUMBER;
    else if( c == '-') *token_type= JREAD_NUMBER;
    else if( c == '{') *token_type= JREAD_OBJECT;
    else if( c == '[') *token_type= JREAD_ARRAY;
    else if( c == '}') *token_type= JREAD_EOBJECT; 
    else if( c == ']') *token_type= JREAD_EARRAY;
    else if((c == 't') || (c == 'f')) *token_type= JREAD_BOOL;
    else if( c == 'n') *token_type= JREAD_NULL;
    else if( c == ':') *token_type= JREAD_COLON;
    else if( c == ',') *token_type= JREAD_COMMA;
    else if( c == '*') *token_type= JREAD_QPARAM;
    else *token_type= JREAD_ERROR;
    return sp;
};

// json_get_string
// - assumes next element is "string" which may include "\" sequences
// - returns pointer to -------------^
// - element contains result (JREAD_STRING, length, pointer to string)
// - pass quote = '"' for Json, quote = '\'' for query scanning
//
// returns: pointer into json after the string (char after the " terminator)
//			element contains pointer and length of string (or data_type=JREAD_ERROR)
//
char* json_get_string(char* json, json_element_t* element, char quote)
{
    short skip_ch;
    element->data_type = JREAD_ERROR;
    element->elements = 1;
    element->bytelen = 0;
    json= json_skip_whitespace(json);
    if(*json == quote)
    {
        json++;
        element->value = json; // -> start of actual string
        element->bytelen = 0;
        skip_ch = 0;
        while(*json != '\0')
        {
            if(skip_ch)
                skip_ch = 0;
            else if(*json == '\\') // "\" sequence
                skip_ch = 1;
            else if(*json == quote)
            {
                element->data_type = JREAD_STRING;
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
// - used to identify length of element text
// - returns no. of chars from json upto a terminator
// - terminators: ' ' , } ]
//
int json_text_len(char* json)
{
    int len = 0;
    while((*json >  ' ') && // any ctrl char incl '\0'
            (*json != ',') &&
            (*json != '}') &&
            (*json != ']'))
    {
        len++;
        json++;
    }
    return len;
}

// json_count_object
// - used when query ends at an object, we want to return the object length
// - on entry pJson -> "{... "
// - used to skip unwanted values which are objects
// - keyIndex normally passed as -1 unless we're looking for the nth "key" value
//   in which case keyIndex is the index of the key we want
//
char* json_count_object(char* json, json_element_t* result, int keyIndex)
{
    json_element_t element;
    int token;
    char* sp;
    result->data_type = JREAD_OBJECT;
    result->error = 0;
    result->elements = 0;
    result->value = json;
    sp = json_find_token(json + 1, &token); // check for empty object
    if(token == JREAD_EOBJECT)
    {
        json = sp + 1;
    }
    else
    {
        while(1)
        {
            json = json_get_string(++json, &element, '\"');
            if(element.data_type != JREAD_STRING)
            {
                result->error = 3; // Expected "key"
                break;
            }
            if(result->elements == keyIndex) // if passed keyIndex
            {
                *result = element; // we return "key" at this index
                result->data_type = JREAD_KEY;
                return json;
            }
            json = json_find_token(json, &token);
            if(token != JREAD_COLON)
            {
                result->error = 4; // Expected ":"
                break;
            }
            json = json_read(++json, "", &element);
            if(result->error)
                break;
            json = json_find_token(json, &token);
            result->elements++;
            if(token == JREAD_EOBJECT)
            {
                json++;
                break;
            }
            if(token != JREAD_COMMA)
            {
                result->error = 6; // Expected "," in object
                break;
            }
        }
    }
    if(keyIndex >= 0)
    {
        // we wanted a "key" value - that we didn't find
        result->data_type = JREAD_ERROR;
        result->error = 11; // Object key not found (bad index)
    }
    else
    {
        result->bytelen = json - (char*)result->value;
    }
    return json;
}



// json_count_array
// - used when query ends at an array, we want to return the array length
// - on entry pJson -> "[... "
// - used to skip unwanted values which are arrays
//
char* json_count_array(char* json, json_element_t* result)
{
    json_element_t element;
    int token;
    char* sp;
    result->data_type = JREAD_ARRAY;
    result->error = 0;
    result->elements = 0;
    result->value = json;
    sp = json_find_token(json + 1, &token); // check for empty array
    if(token == JREAD_EARRAY)
    {
        json = sp + 1;
    }
    else
    {
        while(1)
        {
            json = json_read(++json, "", &element); // array value
            if(result->error)
                break;
            json = json_find_token(json, &token); // , or ]
            result->elements++;
            if(token == JREAD_EARRAY)
            {
                json++;
                break;
            }
            if(token != JREAD_COMMA)
            {
                result->error = 9; // Expected "," in array
                break;
            }
        }
    }
    result->bytelen = json - (char*)result->value;
    return json;
}

// json_array_step
// - reads one value from an array
// - assumes pJsonArray points at the start of an array or array element
//
char* json_array_step(char* json_array, json_element_t* result)
{
    int token;
    json_array = json_find_token(json_array, &token);
    switch(token)
    {
    case JREAD_ARRAY:       // start of array
    case JREAD_COMMA:       // element separator
        return json_read(++json_array, "", result);
    case JREAD_EARRAY:      // end of array
        result->error = 13; // End of array found
        break;
    default:                // some other error
        result->error = 9;  // expected comma in array
        break;
    }
    result->data_type = JREAD_ERROR;
    return json_array;
}


// json_read
// - reads a complete JSON <value>
// - matches pQuery against pJson, results in pResult
// returns: pointer into pJson
//
// Note: is recursive
//
char* json_read( char *pJson, char *pQuery, json_element_t *pResult )
{
    return jReadParam( pJson, pQuery, pResult, NULL );
}

char* jReadParam( char *pJson, char *pQuery, json_element_t *pResult, int *queryParams )
{
    int qTok, jTok, bytelen;
    unsigned int index, count;
    json_element_t qElement, jElement;

    pJson= json_find_token( pJson, &jTok );
    pQuery= json_find_token( pQuery, &qTok );

    pResult->data_type= jTok;
    pResult->bytelen= pResult->elements= pResult->error= 0;
    pResult->value= pJson;

    if( (qTok != JREAD_EOL) && (qTok != jTok) )
    {
        pResult->error= 1;	// JSON does not match Query
        return pJson;
    }

    switch( jTok )
    {
    case JREAD_ERROR:       // general error, eof etc.
        pResult->error= 2;  // Error reading JSON value
        break;

    case JREAD_OBJECT:      // "{"
        if( qTok == JREAD_EOL )
            return json_count_object( pJson, pResult, -1 ); // return length of object 

        pQuery= json_find_token( ++pQuery, &qTok );			// "('key'...", "{NUMBER", "{*" or EOL
        if( qTok != JREAD_STRING )
        {
            index= 0;
            switch( qTok )
            {
            case JREAD_NUMBER:
                pQuery= json_atoi((char*)pQuery, &index); // index value
                break;
            case JREAD_QPARAM:
                pQuery++;
                index= (queryParams != NULL) ? *queryParams++ : 0;	// substitute parameter
                break;
            default:
                pResult->error= 12;								// Bad Object key
                return pJson;
            }
            return json_count_object( pJson, pResult, index );
        }

        pQuery= json_get_string( pQuery, &qElement, QUERY_QUOTE );	// qElement = query 'key'
        //
        // read <key> : <value> , ... }
        // loop 'til key matched
        //
        while( 1 )
        {
            pJson= json_get_string( ++pJson, &jElement, '\"' );
            if( jElement.data_type != JREAD_STRING )
            {
                pResult->error= 3;		// Expected "key"
                break;
            }
            pJson= json_find_token( pJson, &jTok );
            if( jTok != JREAD_COLON )
            {
                pResult->error= 4;		// Expected ":"
                break;
            }
            // compare object keys
            if(json_strcmp(&qElement, &jElement) == 0)
            {
                // found object key
                return jReadParam( ++pJson, pQuery, pResult, queryParams );
            }
            // no key match... skip this value
            pJson= json_read( ++pJson, "", pResult );
            pJson= json_find_token( pJson, &jTok );
            if( jTok == JREAD_EOBJECT )
            {
                pResult->error= 5;		// Object key not found
                break;
            }
            if( jTok != JREAD_COMMA )
            {
                pResult->error= 6;		// Expected "," in object
                break;
            }
        }
        break;
    case JREAD_ARRAY:		// "[NUMBER" or "[*"
        //
        // read index, skip values 'til index
        //
        if( qTok == JREAD_EOL )
            return json_count_array( pJson, pResult );	// return length of object 

        index= 0;
        pQuery= json_find_token( ++pQuery, &qTok );		// "[NUMBER" or "[*"
        if( qTok == JREAD_NUMBER )		
        {
            pQuery= json_atoi( pQuery, &index );		// get array index	
        }else if( qTok == JREAD_QPARAM )
        {
            pQuery++;
            index= (queryParams != NULL) ? *queryParams++ : 0;	// substitute parameter
        }

        count=0;
        while( 1 )
        {
            if( count == index )
                return jReadParam( ++pJson, pQuery, pResult, queryParams );	// return value at index
            // not this index... skip this value
            pJson= json_read( ++pJson, "", &jElement );
            if( pResult->error )
                break;
            count++;				
            pJson= json_find_token( pJson, &jTok );			// , or ]
            if( jTok == JREAD_EARRAY )
            {
                pResult->error= 10;		// Array element not found (bad index)
                break;
            }
            if( jTok != JREAD_COMMA )
            {
                pResult->error= 9;		// Expected "," in array
                break;
            }
        }
        break;
    case JREAD_STRING:		// "string" 
        pJson= json_get_string( pJson, pResult, '\"' );
        break;
    case JREAD_NUMBER:		// number (may be -ve) int or float
    case JREAD_BOOL:		// true or false
    case JREAD_NULL:		// null
        bytelen= json_text_len( pJson );
        pResult->data_type= jTok;
        pResult->bytelen= bytelen;
        pResult->value= pJson;
        pResult->elements= 1;
        pJson += bytelen;
        break;
    default:
        pResult->error= 8;	// unexpected character (in pResult->dataType)
    }
    // We get here on a 'terminal value'
    // - make sure the query string is empty also
    pQuery= json_find_token( pQuery, &qTok );
    if( !pResult->error && (qTok != JREAD_EOL) )
        pResult->error = 7;	// terminal value found before end of query
    if( pResult->error )
    {
        pResult->data_type = JREAD_ERROR;
        pResult->elements = pResult->bytelen = 0;
        pResult->value = pJson;		// return pointer into JSON at error point
    }
    return pJson;
}


//--------------------------------------------------------------------
// Optional helper functions
// - simple routines to extract values from JSON
// - does coercion of types where possible
// - always returns a value (e.g. 0 or "" on error)
//
// Note: by default, pass NULL for queryParams
//       unless you are using '*' in the query for indexing
// 

// jRead_long
// - reads signed long value from JSON 
// - returns number from NUMBER or STRING elements (if possible)
//   returns 1 or 0 from BOOL elements
//   otherwise returns 0
//
long json_long(char* json, char* query, int* query_params)
{
    json_element_t elem;
    long result;
    jReadParam(json, query, &elem, query_params);
    if( (elem.data_type == JREAD_ERROR) || (elem.data_type == JREAD_NULL))
        return 0;
    if( elem.data_type == JREAD_BOOL )
        return *((char *)elem.value)=='t' ? 1 : 0;

    json_atol( (char *)elem.value, &result );
    return result;
}

int json_int(char* json, char* query, int* query_params)
{
    return (int)json_long(json, query, query_params);
}

// jRead_double
// - returns double from JSON
// - returns number from NUMBER or STRING elements
//   otherwise returns 0.0
//
double json_double(char* json, char* query, int* query_params)
{
    json_element_t elem;
    double result;
    jReadParam(json, query, &elem, query_params);
    if(elem.data_type == JREAD_ERROR)
        return 0.0;
    json_atof((char*)elem.value, &result);
    return result;
}

// jRead_string
// Copy string to pDest and '\0'-terminate it (upto destlen total bytes)
// returns: character length of string (excluding '\0' terminator)
//
// Note: any element can be returned as a string
//
int json_string(char* json, char* query, char* dest, int dest_len, int* query_params)
{
    json_element_t elem;
    int i;

    *dest= '\0';
    jReadParam(json, query, &elem, query_params);
    if(elem.data_type == JREAD_ERROR)
        return 0;

    for(i=0; (i < elem.bytelen) && (i < dest_len - 1); i++)
        *dest++ = ((char*)elem.value)[i];
    *dest= '\0';
    return elem.bytelen;
}

// compare two json elements
// returns: 0 if they are identical strings, else 1
//
int json_strcmp(json_element_t* j1, json_element_t* j2)
{
    int i;
    if( (j1->data_type != JREAD_STRING) || 
        (j2->data_type != JREAD_STRING) ||
        (j1->bytelen != j2->bytelen ) )
        return 1;

    for( i=0; i< j1->bytelen; i++ )
        if( ((char *)(j1->value))[i] != ((char *)(j2->value))[i] )
            return 1;
    return 0; 
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
    *result= x;
    return p;
}

// read long int from string
//
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
//
char* json_atof(char *p, double *result)
{
#define VALID_DIGIT(c) ((c) >= '0' && (c) <= '9')

    double sign, value;

    // Get sign, if any.
    sign = 1.0;
    if (*p == '-') {
        sign = -1.0;
        p += 1;

    } else if (*p == '+') {
        p += 1;
    }

    // Get digits before decimal point or exponent, if any.
    for (value = 0.0; VALID_DIGIT(*p); p += 1) {
        value = value * 10.0 + (*p - '0');
    }

    // Get digits after decimal point, if any.
    if (*p == '.') {
        double pow10 = 10.0;
        p += 1;
        while (VALID_DIGIT(*p)) {
            value += (*p - '0') / pow10;
            pow10 *= 10.0;
            p += 1;
        }
    }
    *result= sign * value;

#undef VALID_DIGIT
    return p;
}

// read element into destination buffer and add '\0' terminator
// - always copies element irrespective of dataType (unless it's an error)
// - destBuffer is always '\0'-terminated (even on zero lenght returns)
// - returns pointer to destBuffer
//
char* json_strcpy(char* dest_buffer, int dest_length, json_element_t* element)
{
    int i;
    int len = element->bytelen;
    char* dest = dest_buffer;
    char* src = (char*)element->value;
    if( element->error == 0 )
    {
        if(len >= dest_length)
            len = dest_length;
        for(i = 0; i < dest_length; i++)
            *dest++ = *src++;
    }
    *dest= '\0';
    return dest_buffer;
}


//-------------------------------------------------
// Optional String output Functions
//
char* jReadTypeStrings[]={
    "Error",			// 0
    "Object",			// 1
    "Array",			// 2
    "String",			// 3
    "Number",			// 4
    "Bool",				// 5
    "null",				// 6
    "Object key",		// 7
    "colon",			// 8
    "eol",				// 9
    "comma",			// 10
    "}",				// 11
    "]",				// 12
    "* parameter"		// 13
};

char* json_type_to_string(int dataType)
{
    return jReadTypeStrings[dataType];
};

char* jReadErrorStrings[]={
    "Ok",                                       // 0
    "JSON does not match Query",                // 1
    "Error reading JSON value",                 // 2
    "Expected \"key\"",                         // 3
    "Expected ':'",                             // 4
    "Object key not found",                     // 5
    "Expected ',' in object",                   // 6
    "Terminal value found before end of query", // 7
    "Unexpected character",                     // 8
    "Expected ',' in array",                    // 9
    "Array element not found (bad index)",      // 10
    "Object key not found (bad index)",			// 11
    "Bad object key",							// 12
    "End of array found",						// 13
    "End of object found"						// 14
};
char* json_error_to_string(int error)
{
    if((error >=0 ) && (error <= 14))
        return jReadErrorStrings[error];
    return "Unknown error";
};

// end of jRead.c
