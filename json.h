// json.h
//
// see json.c for more information
//

// uncomment this if you really want to use double quotes in query strings instead of '
//#define JREAD_DOUBLE_QUOTE_IN_QUERY

//
// return dataTypes:
#define JREAD_ERROR		0		// general error, eof etc.
#define JREAD_OBJECT	1		// "{"
#define JREAD_ARRAY		2		// "["
#define JREAD_STRING	3		// "string" 
#define JREAD_NUMBER	4		// number (may be -ve) int or float
#define JREAD_BOOL		5		// true or false
#define JREAD_NULL		6		// null
#define JREAD_KEY		7		// object "key"
// internal values:
#define JREAD_COLON		8		// ":"
#define JREAD_EOL		9		// end of input string (ptr at '\0')
#define JREAD_COMMA		10		// ","
#define JREAD_EOBJECT	11		// "}"
#define JREAD_EARRAY	12		// "]"
#define JREAD_QPARAM	13		// "*" query string parameter

//------------------------------------------------------
// json_element_t
// - structure to return JSON elements
// - error=0 for valid returns
//
// *NOTES*
//    the returned pValue pointer points into the passed JSON
//    string returns are not '\0' terminated.
//    bytelen specifies the length of the returned data pointed to by pValue
//
typedef struct
{
	int data_type;		// one of JREAD_...
	int elements;		// number of elements (e.g. elements in array or object)
	int bytelen;		// byte length of element (e.g. length of string, array text "[ ... ]" etc.)
	void* value;		// pointer to value string in JSON text
	int error;			// error value if dataType == JREAD_ERROR
} json_element_t;

//------------------------------------------------------
// The JSON reader function
//
// - reads a '\0'-terminated JSON text string from json
// - traverses the JSON according to the query string
// - returns the result value in result
//
// returns: pointer into json after the queried value
//
// e.g.
//    With JSON like: "{ ..., "key":"value", ... }"
//
//    json_read(json, "{'key'", &result);
// returns with: 
//    result.data_type= JREAD_STRING, result.value->'value', result.bytelen=5
//
char* json_read( char *pJson, char *pQuery, json_element_t* pResult );

// version of json_read which allows one or more queryParam integers to be substituted
// for array or object indexes marked by a '*' in the query
//
// e.g. jReadParam( pJson, "[*", &resultElement, &arrayIndex );
//
// *!* CAUTION *!*
// You can supply an array of integers which are indexed for each '*' in pQuery
// however, horrid things will happen if you don't supply enough parameters
// 
char * jReadParam( char *pJson, char *pQuery, json_element_t* pResult, int *queryParams );

// Array Stepping function
// - assumes pJsonArray is JSON source of an array "[ ... ]"
// - returns next element of the array in pResult
// - returns pointer to end of element, to be passed to next call of jReadArrayStep()
// - if end of array is encountered, pResult->error = 13 "End of array found"
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
//       for(index=0; index < theArray.elements; index++)
//       {
//           array = json_array_step(array, &arrayElement);
//           ...
//
// Note: this significantly speeds up traversing arrays.
//
char* json_array_step(char* json_array, json_element_t* result);


//------------------------------------------------------
// Helper Functions
//
long    json_long(char* json, char* query, int* query_params);
int     json_int(char* json, char* query, int* query_params);
double  json_double(char* json, char* query, int* query_params);
int     json_string(char* json, char* query, char *dest, int dest_len, int* query_params);

//------------------------------------------------------
// String output Functions
//
char* json_type_to_string(int data_type);   // string describes data_type
char* json_error_to_string(int error);      // string descibes error code

//------------------------------------------------------
// Other json utilities which may be useful...
//
char* json_atoi(char* p, unsigned int* result);    // string to unsigned int
char* json_atol(char* p, long* result);            // string to signed long
char* json_atof(char* p, double* result);          // string to double (does not do exponents)
int json_strcmp(json_element_t* j1, json_element_t* j2); // compare STRING elements

// copy element to '\0'-terminated buffer
char* json_strcpy(char* dest_buffer, int dest_length, json_element_t* element);

// end of json.h
