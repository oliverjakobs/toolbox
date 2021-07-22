#ifndef TB_INI_H
#define TB_INI_H

#include <stddef.h>

typedef enum
{
    TB_INI_OK,
    TB_INI_BAD_NAME,
    TB_INI_BAD_VALUE,
    TB_INI_BAD_SECTION,
    TB_INI_BAD_PROPERTY,
    TB_INI_UNKOWN_ERROR
} tb_ini_error;

typedef struct
{
    char* name;
    size_t name_len;    /* bytelen of name */
    char* start;
    size_t len;         /* bytelen of value or num of properites for sections */
    tb_ini_error error;
} tb_ini_element;

/* 
 * default query 
 * searches for section and if found calls tb_ini_query_section on it 
 * if prop is empty returns the specified section as element
 * if section is empty calls tb_ini_query_section on the current ini pos
 * returns a pointer into the ini file after the queried value
 */
char* tb_ini_query(char* ini, const char* section, const char* prop, tb_ini_element* element);

/* searches for the given property in the current section (until '[' is reached) */
char* tb_ini_query_section(char* section, const char* prop, tb_ini_element* element);

/* returns the next section in the group */
char* tb_ini_group_next(char* ini, const char* group, tb_ini_element* element);

/* returns the next property (after the ini cursor) */
char* tb_ini_property_next(char* ini, tb_ini_element* element);

/* unchecked conversion from element to different types */
int     tb_ini_element_to_bool(tb_ini_element* element);
int     tb_ini_element_to_int(tb_ini_element* element);
float   tb_ini_element_to_float(tb_ini_element* element);
size_t  tb_ini_element_to_string(tb_ini_element* element, char* dst, size_t dst_len);

/* copies the name of the element into the dst buffer (copies at most dst_len bytes)*/
size_t tb_ini_name(const tb_ini_element* element, char* dst, size_t dst_len);

/* utility functions to directly convert query to different types */
int     tb_ini_bool(char* ini, const char* section, const char* prop, int def);
int     tb_ini_int(char* ini, const char* section, const char* prop, int def);
float   tb_ini_float(char* ini, const char* section, const char* prop, float def);
size_t  tb_ini_string(char* ini, const char* section, const char* prop, char* dst, size_t dst_len);

typedef int(*tb_ini_parse_func)(const char* start, size_t len);
int     tb_ini_parse(char* ini, const char* section, const char* prop, tb_ini_parse_func parse);

/* 
 * functions to split quoted values into Comma Separated Values 
 * tb_ini_csv:      creates an element where the value starts after the quote and the len is the number of CSV
 * tb_ini_csv_step: returns next CSV or NULL if closing brace or EOF is reached
 */
char* tb_ini_csv(char* ini, const char* section, const char* prop, tb_ini_element* element);
char* tb_ini_csv_step(char* stream, tb_ini_element* element);

/* returns a string describing the error */
const char* tb_ini_get_error_desc(tb_ini_error error);

#endif /* !TB_INI_H */
