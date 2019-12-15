#include <stdio.h>

#include "json_write.h"

//-------------------------------------------------
// Command-line interface
// usage:
//  jWrite			prints example tests
//

void json_write_object_example()
{
    char buffer[1024];
    unsigned int buflen = 1024;
    json_write_control_t jwc;

    printf("A JSON object example:\n\n" );

    // Example JSON object
    json_write_open(&jwc, buffer, buflen, JSON_WRITE_OBJECT, 1);

    json_write_object_string(&jwc, "key", "value");
    json_write_object_int(&jwc, "int", 1);
    json_write_object_double(&jwc, "double", 1.234);
    json_write_object_null(&jwc, "nullThing");
    json_write_object_bool(&jwc, "bool", 1);
    json_write_object_array(&jwc, "EmptyArray"); // empty array
    json_write_end(&jwc);
    json_write_object_array(&jwc, "anArray");
        json_write_array_string(&jwc, "array one");
        json_write_array_int(&jwc, 2);
        json_write_array_double(&jwc, 1234.567);
        json_write_array_null(&jwc);
        json_write_array_bool(&jwc, 0);
        json_write_array_object(&jwc);
            json_write_object_string(&jwc, "obj3_one", "one");
            json_write_object_string(&jwc, "obj3_two", "two");
        json_write_end(&jwc);
        json_write_array_array(&jwc);
            json_write_array_int(&jwc, 0);
            json_write_array_int(&jwc, 1);
            json_write_array_int(&jwc, 2);
        json_write_end(&jwc);
    json_write_end(&jwc);

    json_write_object_object(&jwc, "EmptyObject");
    json_write_end(&jwc);

    json_write_object_object(&jwc, "anObject");
        json_write_object_string(&jwc, "msg","object in object");
        json_write_object_string(&jwc, "msg2","object in object 2nd entry");
    json_write_end(&jwc);
    json_write_object_string( &jwc, "ObjEntry", "This is the last one" );

    json_write_error err = json_write_close(&jwc);

    printf(buffer);

    if(err != JSON_WRITE_OK)
        printf("Error: %s at function call %d\n", json_write_error_string(err), json_write_error_pos(&jwc));
}

void json_write_array_example()
{
    char buffer[1024];
    unsigned int buflen = 1024;
    json_write_control_t jwc;

    printf("\n\nA JSON array example:\n\n" );

    json_write_open(&jwc, buffer, buflen, JSON_WRITE_ARRAY, 1);
    json_write_array_string(&jwc, "String value");
    json_write_array_int(&jwc, 1234);
    json_write_array_double(&jwc, 567.89012);
    json_write_array_bool(&jwc, 1);
    json_write_array_null(&jwc);
    json_write_array_object(&jwc);
    // empty object
    json_write_end(&jwc);
    json_write_array_object(&jwc);
        json_write_object_string(&jwc, "key", "value");
        json_write_object_string(&jwc, "key2", "value2");
    json_write_end(&jwc);
    json_write_array_array(&jwc); // array in array
        json_write_array_string(&jwc, "Array in array");
        json_write_array_string(&jwc, "the end");
    json_write_end(&jwc);

    json_write_error err = json_write_close(&jwc);

    printf(buffer);

    if(err != JSON_WRITE_OK)
        printf("Error: %s at function call %d\n", json_write_error_string(err), json_write_error_pos(&jwc));
}

void json_write_error_example()
{
    char buffer[1024];
    unsigned int buflen = 1024;
    json_write_control_t jwc;

    printf("\n\nA JSON error example:\n\n" );

    json_write_open(&jwc, buffer, buflen, JSON_WRITE_ARRAY, 1); // 1
    json_write_array_string(&jwc, "String value");              // 2
    json_write_array_int(&jwc, 1234);                           // 3
    json_write_array_double(&jwc, 567.89012);                   // 4
    json_write_array_bool(&jwc, 1);                             // 5
    json_write_array_null(&jwc);                                // 6
    json_write_array_object(&jwc);                              // 7
    json_write_array_object(&jwc);                              // 8  <-- this is where the error is
        json_write_object_string(&jwc, "key", "value");         // 9
        json_write_object_string(&jwc, "key2", "value2");       // 10
    json_write_end(&jwc);                                       // 11 
    json_write_array_array(&jwc);    // array in array          // 12
        json_write_array_string(&jwc, "Array in array");        // 13
        json_write_array_string(&jwc, "the end");               // 14
    json_write_end(&jwc);                                       // 15

    json_write_error err = json_write_close(&jwc);              // 16

    printf(buffer);

    if(err != JSON_WRITE_OK)
        printf("Error: %s at function call %d\n", json_write_error_string(err), json_write_error_pos(&jwc));
}

int main(int argc, char* argv[])
{
    printf("---| jWrite |---\n\n");

    json_write_object_example();
    json_write_array_example();
    json_write_error_example();

    return 0;
}