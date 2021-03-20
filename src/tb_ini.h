#ifndef TB_INI_H
#define TB_INI_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    TB_INI_OK,
    TB_INI_BAD_NAME,
    TB_INI_BAD_SECTION,
    TB_INI_BAD_PROPERTY,
    TB_INI_BAD_NUMBER,
    TB_INI_UNKOWN_VALUE,
    TB_INI_UNTERMINATED_STR,
    TB_INI_MEM_ERROR,
    TB_INI_UNKOWN_ERROR
} tb_ini_error;

typedef enum
{
    TB_INI_ERROR,
    TB_INI_SECTION,
    TB_INI_STRING,
    TB_INI_INT,
    TB_INI_BOOL
} tb_ini_type;

/* in place query functions */
typedef struct
{
    char* name;
    size_t name_len;    /* bytelen of name */
    tb_ini_type type;
    char* start;
    size_t len;         /* bytelen of value_str or num of properites in section if type == TB_INI_SECTION */
    tb_ini_error error;
} tb_ini_element;

char* tb_ini_query(char* ini, char* query, tb_ini_element* element);
char* tb_ini_query_section(char* section, char* query, tb_ini_element* element);

int32_t tb_ini_query_int(char* ini, char* query, int32_t def);
uint8_t tb_ini_query_bool(char* ini, char* query, uint8_t def);
size_t tb_ini_query_string(char* ini, char* query, char* dst, size_t dst_len);

int32_t tb_ini_to_int(const tb_ini_element* element, int32_t def);
uint8_t tb_ini_to_bool(const tb_ini_element* element, uint8_t def);
size_t tb_ini_to_string(const tb_ini_element* element, char* dst, size_t dst_len);
size_t tb_ini_get_name(const tb_ini_element* element, char* dst, size_t dst_len);


/*  */
char* tb_ini_atoi(char* p, int32_t* result);

void tb_ini_print_element(tb_ini_element* element);

const char* tb_ini_get_type_name(tb_ini_type type);
const char* tb_ini_get_error_desc(tb_ini_error error);

#endif /* !TB_INI_H */
