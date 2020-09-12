#include "../tb_json.h"

#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>


//-------------------------------------------------
// Do a query and print the results
void test_query(char* json, char* query)
{
    tb_json_element element;
    tb_json_read(json, &element, query);
    printf("Query: \"%s\"\n", query);
    printf("return: %d = %s\n", element.error, tb_json_error_to_string(element.error));
    printf(" dataType = %s\n", tb_json_type_to_string(element.data_type));
    printf(" elements = %d\n", element.elements);
    printf(" bytelen  = %d\n", element.bytelen);
    printf(" value    = %*.*s\n\n", element.bytelen, element.bytelen, element.value);
}

//=================================================================
// Examples
// - whole bunch of JSON examples
void run_examples()
{
    char str[128];
    tb_json_element array_element;

    char* example_json=
        "{" 
        "  \"astring\": \"This is a string\",\n"
        "  \"number1\": 42,\n"
        "  \"number2\":  -123.45,\n"
        "  \"anObject\":{\"one\":1,\"two\":{\"obj2.1\":21,\"obj2.2\":22},\"three\":333},\n"
        "  \"anArray\":[0, \"one\", {\"two.0\":20,\"two.1\":21}, 3, [4,44,444]],\n"
        "  \"isnull\":null,\n"
        "  \"emptyArray\":[],\n"
        "  \"emptyObject\":{  },\n"
        "  \"yes\": true,\n"
        "  \"no\":  false\n"
        "}\n";

    test_query(example_json, "");
    test_query(example_json, "[1");
    test_query(example_json, "{'astring'");
    test_query(example_json, "{'number1'");
    test_query(example_json, "{'number2'");
    test_query(example_json, "{'anObject'");
    test_query(example_json, "{'anArray'");
    test_query(example_json, "{'isnull'");
    test_query(example_json, "{'yes'");
    test_query(example_json, "{'no'");
    test_query(example_json, "{'missing'");
    test_query(example_json, "{'anObject'{'two'");
    test_query(example_json, "{'anObject' {'two' {'obj2.2'");
    test_query(example_json, "{'anObject'{'three'");
    test_query(example_json, "{'anArray' [1");
    test_query(example_json, "{'anArray' [2 {'two.1'");
    test_query(example_json, "{'anArray' [4 [2");
    test_query(example_json, "{'anArray' [999");

    printf("Empty array or object...\n");
    test_query(example_json, "{'emptyArray'");
    test_query(example_json, "{'emptyObject'");

    printf("Return the key at a given index in an object...\n");
    test_query(example_json, "{3");
    test_query(example_json, "{'anObject' {1");
    test_query(example_json, "{999");

    // examples of helper functions
    long l =    tb_json_long(example_json, "{'number1'", NULL, 0);     // 42
    int i =     tb_json_int(example_json, "{'yes'", NULL, 0);          // 1    (BOOL example)
    float f =   tb_json_float(example_json, "{'number2'", NULL, 0.0f);    // -123.45
    tb_json_string(example_json, "{'astring'", str, 16, NULL);     // "This is a strin\0" (buffer too short example)

    printf("Helper Functions...\n");
    printf("  \"number1\"= %ld\n", l);
    printf("  \"yes\"    = %d\n", i);
    printf("  \"number2\"= %f\n", f);
    printf("  \"astring\"= \"%s\"\n", str);

    // Example of cascading queries
    printf("\nQueries on sub-elements and use of query parameters...\n");

    // locate "anArray"...
    tb_json_read(example_json, &array_element, "{'anArray'");
    printf("  \"anArray\": = %*.*s\n\n", array_element.bytelen, array_element.bytelen, array_element.value);

    // do queries within "anArray"...
    for(i = 0; i < array_element.elements; i++)
    {
        // index the array using queryParam
        tb_json_string((char*)array_element.value, "[*", str, 128, &i); 
        printf("  anArray[%d] = %s\n", i, str);
    }

    // example using a parameter array
    {
        int params[2] = { 2, 1 };
        tb_json_string((char*)array_element.value, "[*{*", str, 128, params);
        printf("\n  anArray[%d] objectKey[%d] = \"%s\"\n", params[0], params[1], str);
    }
}

//=================================================================
//
// Helper functions to read a JSON file into a malloc'd buffer with '\0' terminator
//
typedef struct
{
    unsigned long length;   // length in bytes
    unsigned char* data;    // malloc'd data, free with freeFileBuffer()
} file_buffer_t;

#define FILE_BUFFER_MAXLEN 1024*1024

unsigned long file_buffer_read(char* filename, file_buffer_t* buffer, unsigned long maxlen);
void file_buffer_free(file_buffer_t* buffer);

// file_buffer_read
// - reads file into a malloc'd buffer with appended '\0' terminator
// - limits malloc() to maxlen bytes
// - if file size > maxlen then the function fails (returns 0)
//
// returns: length of file data (excluding '\0' terminator)
//          0=empty/failed
unsigned long file_buffer_read(char* filename, file_buffer_t* buffer, unsigned long maxlen)
{
    FILE *file;

    if ((file = fopen(filename, "rb")) == NULL)
    {
        printf("Can't open file: %s\n", filename);
        return 0;
    }
    // find file size and allocate buffer for JSON file
    fseek(file, 0L, SEEK_END);
    buffer->length = ftell(file);
    if (buffer->length >= maxlen)
    {
        fclose(file);
        return 0;
    }
    // rewind and read file
    fseek(file, 0L, SEEK_SET);
    buffer->data = (unsigned char*)malloc(buffer->length + 1);
    memset(buffer->data, 0, buffer->length + 1); // +1 guarantees trailing \0

    int i = fread(buffer->data, buffer->length, 1, file);
    fclose(file);
    if (i != 1)
    {
        file_buffer_free(buffer);
        return 0;
    }
    return buffer->length;
}

// file_buffer_free
// - free's buffer space and zeros it
void file_buffer_free(file_buffer_t* buffer)
{
    if (buffer->data != NULL)
        free(buffer->data);
    buffer->data = 0;
    buffer->length = 0;
}

//====================================================================================
// Functions for command-ine interface
//

void print_help()
{
    printf("json - an in-place json element reader\n");
    printf("usage:\n");
    printf("  json t        runs built-in test examples\n");
    printf("  json <filename> \"query String\"\n" );
    printf("e.g.\n");
    printf("  json example.json \"{'astring'\"\n");
};

//-------------------------------------------------
// Command-line interface
// usage:
//  JSON ?			prints help text
//  JSON t			runs test examples
//
//	JSON jsonfile "query string"
//  - reads jsonfile and executes "query string"
//
int main(int argc, char * argv[])
{
    if(argc == 2)
    {
        switch(*argv[1])
        {
        case '?':   print_help(); break;
        case 't':   run_examples(); break;
        }
        return 0;
    }

    if(argc != 3)
    {
        print_help();
        return 1;
    };

    file_buffer_t json;
    if(file_buffer_read(argv[1], &json, FILE_BUFFER_MAXLEN) == 0)
    {
        printf("Can't open file: %s\n", argv[1] );
        return 1;
    }

    // perform query on JSON file
    test_query((char*)json.data, argv[2]);

    file_buffer_free(&json);
    return 0;
}

