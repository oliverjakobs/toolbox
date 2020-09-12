#include <stdio.h>

#include "../tb_jwrite.h"

void tb_jwrite_object_example(const char* path)
{
    printf("A JSON object example at (%s)\n", path);

    tb_jwrite_control jwc;
	tb_jwrite_open(&jwc, path, TB_JWRITE_OBJECT, TB_JWRITE_NEWLINE);
	tb_jwrite_set_float_prec(&jwc, 3);

    tb_jwrite_string(&jwc, "key", "value");
    tb_jwrite_int(&jwc, "int", 1);
    tb_jwrite_float(&jwc, "i2", 154.65);
    tb_jwrite_float(&jwc, "i3", 1.562);
    tb_jwrite_float(&jwc, "i4", 0.0);
    tb_jwrite_float(&jwc, "i5", 2896.6);
    tb_jwrite_float(&jwc, "i6", -8.546);
    tb_jwrite_float(&jwc, "i7", 23.5);
    tb_jwrite_float(&jwc, "i8", 444444.44);
    tb_jwrite_float(&jwc, "i9", 52.6);
    tb_jwrite_float(&jwc, "i10", -98451);
    tb_jwrite_float(&jwc, "float", 1.234);
    tb_jwrite_null(&jwc, "nullThing");
    tb_jwrite_array(&jwc, "EmptyArray");
    tb_jwrite_end(&jwc);
    tb_jwrite_array(&jwc, "anArray");
        tb_jwrite_array_string(&jwc, "array one");
        tb_jwrite_array_int(&jwc, 2);
        tb_jwrite_array_float(&jwc, 1234.567);
        tb_jwrite_array_null(&jwc);
        tb_jwrite_array_int(&jwc, 0);
        tb_jwrite_array_object(&jwc);
            tb_jwrite_string(&jwc, "obj3_one", "one");
            tb_jwrite_string(&jwc, "obj3_two", "two");
        tb_jwrite_end(&jwc);
        tb_jwrite_array_array(&jwc);
            tb_jwrite_array_int(&jwc, 0);
            tb_jwrite_array_int(&jwc, 1);
            tb_jwrite_array_int(&jwc, 2);
        tb_jwrite_end(&jwc);
    tb_jwrite_end(&jwc);

    tb_jwrite_object(&jwc, "EmptyObject");
    tb_jwrite_end(&jwc);

    tb_jwrite_object(&jwc, "anObject");
        tb_jwrite_string(&jwc, "msg","object in object");
        tb_jwrite_string(&jwc, "msg2","object in object 2nd entry");
    tb_jwrite_end(&jwc);
    tb_jwrite_string( &jwc, "ObjEntry", "This is the last one" );

    tb_jwrite_error err = tb_jwrite_close(&jwc);

    if(err != TB_JWRITE_OK)
        printf("Error: %s at function call %d\n", tb_jwrite_error_string(err), tb_jwrite_error_pos(&jwc));
}

void tb_jwrite_array_example(const char* path)
{
    printf("A JSON array example at (%s)\n", path);

    tb_jwrite_control jwc;
	tb_jwrite_open(&jwc, path, TB_JWRITE_ARRAY, TB_JWRITE_NEWLINE);
	tb_jwrite_set_float_prec(&jwc, 2);

    tb_jwrite_array_string(&jwc, "String value");
    tb_jwrite_array_int(&jwc, 1234);
    tb_jwrite_array_float(&jwc, 567.89012);
    tb_jwrite_array_int(&jwc, 1);
    tb_jwrite_array_null(&jwc);
    tb_jwrite_array_object(&jwc);
    tb_jwrite_end(&jwc);
    tb_jwrite_array_object(&jwc);
        tb_jwrite_string(&jwc, "key", "value");
        tb_jwrite_string(&jwc, "key2", "value2");
    tb_jwrite_end(&jwc);
    tb_jwrite_array_array(&jwc);
        tb_jwrite_array_string(&jwc, "Array in array");
        tb_jwrite_array_string(&jwc, "the end");
    tb_jwrite_end(&jwc);

    tb_jwrite_error err = tb_jwrite_close(&jwc);

    if(err != TB_JWRITE_OK)
        printf("Error: %s at function call %d\n", tb_jwrite_error_string(err), tb_jwrite_error_pos(&jwc));
}

void tb_jwrite_error_example(const char* path)
{
    printf("A JSON error example at (%s)\n", path);

    tb_jwrite_control jwc;
    tb_jwrite_open(&jwc, path, TB_JWRITE_ARRAY, TB_JWRITE_NEWLINE); /* 1 */
    tb_jwrite_set_float_prec(&jwc, 2);                              /* 2 */

    tb_jwrite_array_string(&jwc, "String value");                   /* 3 */
    tb_jwrite_array_int(&jwc, 1234);                                /* 4 */
    tb_jwrite_array_float(&jwc, 567.89012);                         /* 5 */
    tb_jwrite_array_int(&jwc, 1);                                   /* 6 */
    tb_jwrite_array_null(&jwc);                                     /* 7 */
    tb_jwrite_array_object(&jwc);                                   /* 8 */
    tb_jwrite_array_object(&jwc);                                   /* 9  <-- this is where the error is */
        tb_jwrite_string(&jwc, "key", "value");                     /* 10 */
        tb_jwrite_string(&jwc, "key2", "value2");                   /* 11 */
    tb_jwrite_end(&jwc);                                            /* 12 */
    tb_jwrite_array_array(&jwc);                                    /* 13 */
        tb_jwrite_array_string(&jwc, "Array in array");             /* 14 */
        tb_jwrite_array_string(&jwc, "the end");                    /* 15 */
    tb_jwrite_end(&jwc);                                            /* 16 */

    tb_jwrite_error err = tb_jwrite_close(&jwc);                    /* 17 */

    if(err != TB_JWRITE_OK)
        printf("Error: %s at function call %d\n", tb_jwrite_error_string(err), tb_jwrite_error_pos(&jwc));
}

int main(int argc, char* argv[])
{
    printf("---| tb_jwrite |---\n\n");

    tb_jwrite_object_example("res/jwrite_object.json");
    tb_jwrite_array_example("res/jwrite_array.json");
    tb_jwrite_error_example("res/jwrite_error.json");

    return 0;
}