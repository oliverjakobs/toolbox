#include <stdio.h>

#include "tb_jwrite.h"

//-------------------------------------------------
// Command-line interface
// usage:
//  jWrite  prints example tests
//

void tb_jwrite_object_example()
{
    char buffer[1024];
    unsigned int buflen = 1024;
    tb_jwrite_control jwc;

    printf("A JSON object example:\n\n" );

    // Example JSON object
    tb_jwrite_open(&jwc, buffer, buflen, TB_JWRITE_OBJECT, 1);

    tb_jwrite_object_string(&jwc, "key", "value");
    tb_jwrite_object_int(&jwc, "int", 1);
    tb_jwrite_object_double(&jwc, "double", 1.234);
    tb_jwrite_object_null(&jwc, "nullThing");
    tb_jwrite_object_bool(&jwc, "bool", 1);
    tb_jwrite_object_array(&jwc, "EmptyArray"); // empty array
    tb_jwrite_end(&jwc);
    tb_jwrite_object_array(&jwc, "anArray");
        tb_jwrite_array_string(&jwc, "array one");
        tb_jwrite_array_int(&jwc, 2);
        tb_jwrite_array_double(&jwc, 1234.567);
        tb_jwrite_array_null(&jwc);
        tb_jwrite_array_bool(&jwc, 0);
        tb_jwrite_array_object(&jwc);
            tb_jwrite_object_string(&jwc, "obj3_one", "one");
            tb_jwrite_object_string(&jwc, "obj3_two", "two");
        tb_jwrite_end(&jwc);
        tb_jwrite_array_array(&jwc);
            tb_jwrite_array_int(&jwc, 0);
            tb_jwrite_array_int(&jwc, 1);
            tb_jwrite_array_int(&jwc, 2);
        tb_jwrite_end(&jwc);
    tb_jwrite_end(&jwc);

    tb_jwrite_object_object(&jwc, "EmptyObject");
    tb_jwrite_end(&jwc);

    tb_jwrite_object_object(&jwc, "anObject");
        tb_jwrite_object_string(&jwc, "msg","object in object");
        tb_jwrite_object_string(&jwc, "msg2","object in object 2nd entry");
    tb_jwrite_end(&jwc);
    tb_jwrite_object_string( &jwc, "ObjEntry", "This is the last one" );

    tb_jwrite_error err = tb_jwrite_close(&jwc);

    printf(buffer);

    if(err != TB_JWRITE_OK)
        printf("Error: %s at function call %d\n", tb_jwrite_error_string(err), tb_jwrite_error_pos(&jwc));
}

void tb_jwrite_array_example()
{
    char buffer[1024];
    unsigned int buflen = 1024;
    tb_jwrite_control jwc;

    printf("\n\nA JSON array example:\n\n" );

    tb_jwrite_open(&jwc, buffer, buflen, TB_JWRITE_ARRAY, 1);
    tb_jwrite_array_string(&jwc, "String value");
    tb_jwrite_array_int(&jwc, 1234);
    tb_jwrite_array_double(&jwc, 567.89012);
    tb_jwrite_array_bool(&jwc, 1);
    tb_jwrite_array_null(&jwc);
    tb_jwrite_array_object(&jwc);
    tb_jwrite_end(&jwc);
    tb_jwrite_array_object(&jwc);
        tb_jwrite_object_string(&jwc, "key", "value");
        tb_jwrite_object_string(&jwc, "key2", "value2");
    tb_jwrite_end(&jwc);
    tb_jwrite_array_array(&jwc); // array in array
        tb_jwrite_array_string(&jwc, "Array in array");
        tb_jwrite_array_string(&jwc, "the end");
    tb_jwrite_end(&jwc);

    tb_jwrite_error err = tb_jwrite_close(&jwc);

    printf(buffer);

    if(err != TB_JWRITE_OK)
        printf("Error: %s at function call %d\n", tb_jwrite_error_string(err), tb_jwrite_error_pos(&jwc));
}

void tb_jwrite_error_example()
{
    char buffer[1024];
    unsigned int buflen = 1024;
    tb_jwrite_control jwc;

    printf("\n\nA JSON error example:\n\n" );

    tb_jwrite_open(&jwc, buffer, buflen, TB_JWRITE_ARRAY, 1);   // 1
    tb_jwrite_array_string(&jwc, "String value");               // 2
    tb_jwrite_array_int(&jwc, 1234);                            // 3
    tb_jwrite_array_double(&jwc, 567.89012);                    // 4
    tb_jwrite_array_bool(&jwc, 1);                              // 5
    tb_jwrite_array_null(&jwc);                                 // 6
    tb_jwrite_array_object(&jwc);                               // 7
    tb_jwrite_array_object(&jwc);                               // 8  <-- this is where the error is
        tb_jwrite_object_string(&jwc, "key", "value");          // 9
        tb_jwrite_object_string(&jwc, "key2", "value2");        // 10
    tb_jwrite_end(&jwc);                                        // 11 
    tb_jwrite_array_array(&jwc);    // array in array           // 12
        tb_jwrite_array_string(&jwc, "Array in array");         // 13
        tb_jwrite_array_string(&jwc, "the end");                // 14
    tb_jwrite_end(&jwc);                                        // 15

    tb_jwrite_error err = tb_jwrite_close(&jwc);                // 16

    printf(buffer);

    if(err != TB_JWRITE_OK)
        printf("Error: %s at function call %d\n", tb_jwrite_error_string(err), tb_jwrite_error_pos(&jwc));
}

int main(int argc, char* argv[])
{
    printf("---| tb_jwrite |---\n\n");

    tb_jwrite_object_example();
    tb_jwrite_array_example();
    tb_jwrite_error_example();

    return 0;
}