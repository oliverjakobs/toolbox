#include "tb_ini.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <stdio.h>

char* tb_ini_skip_whitespaces(char* cursor)
{
    while ((*cursor != '\0') && isspace(*cursor)) cursor++;
    return cursor;
}

size_t tb_ini_strncpy(char* dst, char* src, size_t len, size_t max_len)
{
    if (len >= max_len) len = max_len - 1;
    strncpy(dst, src, len);
    dst[len] = '\0';

    return len;
}

void tb_ini_make_element(tb_ini_element* element, tb_ini_type type, char* start, size_t len)
{
    element->type = type;
    element->start = start;
    element->len = len;
    element->error = TB_INI_OK;
}

void tb_ini_make_error(tb_ini_element* element, tb_ini_error error, char* pos)
{
    element->type = TB_INI_ERROR;
    element->start = pos;
    element->len = 0;
    element->error = error;
}

size_t tb_ini_count_properties(char* section)
{
    size_t count = 0;
    while (*section != '\0' && *section != '[')
    {
        uint8_t not_blank = 0;
        while (*section != '\0' && *section != '\n')
        {
            if (!isspace(*section)) not_blank = 1;

            if (not_blank)  section = strchr(section, '\n');
            else            section++;
        }
        count += not_blank;
        if (*section != '\0') section++;
    }
    return count;
}

void tb_ini_make_section(tb_ini_element* element, char* start)
{
    element->type = TB_INI_SECTION;
    element->start = start;
    element->len = tb_ini_count_properties(start);
    element->error = TB_INI_OK;
}

char* tb_ini_read_value(char* ini, tb_ini_element* element)
{
    char* start = ini;
    switch (*ini++)
    {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        while (*ini != '\0' && isdigit(*ini)) ini++;
        if (isspace(*ini))
            tb_ini_make_element(element, TB_INI_INT, start, ini - start);
        else
            tb_ini_make_error(element, TB_INI_BAD_NUMBER, start);
        break;
    case 't': case 'f':
        while (*ini != '\0' && !isspace(*ini)) ini++;
        if ((strncmp(start, "true", 4) == 0) || (strncmp(start, "false", 5) == 0))
            tb_ini_make_element(element, TB_INI_BOOL, start, ini - start);
        else
            tb_ini_make_error(element, TB_INI_UNKOWN_VALUE, start);
        break;
    case '\"':
        while (*ini != '\0' && *ini != '\n' && *ini != '\"') ini++;
        
        if (*ini++ == '\"')
            tb_ini_make_element(element, TB_INI_STRING, start, ini - start);
        else
            tb_ini_make_error(element, TB_INI_UNTERMINATED_STR, start);
        break;
    default:
        tb_ini_make_error(element, TB_INI_UNKOWN_ERROR, start);
        break;
    }

    return ini;
}

char* tb_ini_read_name(char* ini, tb_ini_element* element)
{
    if (!isalpha(*ini))
    {
        tb_ini_make_error(element, TB_INI_BAD_NAME, ini);
        return ini;
    }

    element->name = ini++;
    while (*ini != '\0' && isalnum(*ini)) ini++;

    if (*ini == '\0')
        tb_ini_make_error(element, TB_INI_BAD_NAME, ini);
    else
        element->name_len = ini - element->name;

    return ini;
}

char* tb_ini_read_element(char* ini, tb_ini_element* element)
{
    ini = tb_ini_skip_whitespaces(ini);

    ini = tb_ini_read_name(ini, element);
    if (element->type == TB_INI_ERROR) return ini;

    ini = tb_ini_skip_whitespaces(ini);

    if (*ini++ == '=')
        return tb_ini_read_value(tb_ini_skip_whitespaces(ini), element);

    tb_ini_make_error(element, TB_INI_BAD_PROPERTY, ini);
    return ini;
}

char* tb_ini_query_section(char* section, char* query, tb_ini_element* element)
{
    size_t query_len = strlen(query);
    section = tb_ini_skip_whitespaces(section);

    if (query_len == 0)
        return tb_ini_read_element(section, element);

    while (*section != '\0' && *section != '[')
    {
        /* compare key */
        if (strncmp(section, query, query_len) == 0)
            return tb_ini_read_element(section, element);

        /* skip to next property */
        section = strchr(section, '\n');
        section = tb_ini_skip_whitespaces(section);
    }
    return NULL;
}

char* tb_ini_find_section(char* ini, char* name, size_t name_len, tb_ini_element* element)
{
    if (name_len == 0) return ini;
    while (*ini != '\0')
    {
        if (*ini != '[') ini++;
        else if (strncmp(++ini, name, name_len) == 0)
        {
            element->name = ini;
            element->name_len = name_len;
            while (*ini != '\0' && *ini != ']') ini++;
            return tb_ini_skip_whitespaces(++ini);
        }
    }

    return NULL;
}

char* tb_ini_query(char* ini, char* query, tb_ini_element* element)
{
    char* s_name = query;
    char* p_name = strchr(query, '.');

    size_t s_len = p_name ? p_name++ - s_name : strlen(query);

    char* section = tb_ini_find_section(ini, s_name, s_len, element);

    if (!section)
    {
        tb_ini_make_error(element, TB_INI_BAD_SECTION, ini);
        return NULL;
    }

    if (!p_name)
    {
        tb_ini_make_section(element, section);
        return section;
    }

    return tb_ini_query_section(section, p_name, element);
}

int32_t tb_ini_query_int(char* ini, char* query, int32_t def)
{
    tb_ini_element element;
    tb_ini_query(ini, query, &element);

    return tb_ini_to_int(&element, def);
}

uint8_t tb_ini_query_bool(char* ini, char* query, uint8_t def)
{
    tb_ini_element element;
    tb_ini_query(ini, query, &element);

    return tb_ini_to_bool(&element, def);
}

size_t tb_ini_query_string(char* ini, char* query, char* dst, size_t dst_len)
{
    tb_ini_element element;
    tb_ini_query(ini, query, &element);

    return tb_ini_to_string(&element, dst, dst_len);
}

int32_t tb_ini_to_int(const tb_ini_element* element, int32_t def)
{
    if (element->type != TB_INI_INT) return def;

    int32_t result;
    tb_ini_atoi(element->start, &result);
    return result;
}

uint8_t tb_ini_to_bool(const tb_ini_element* element, uint8_t def)
{
    if (element->type != TB_INI_BOOL) return def;

    return *element->start == 't' ? 1 : 0;
}

size_t tb_ini_to_string(const tb_ini_element* element, char* dst, size_t dst_len)
{
    *dst = '\0';
    if (element->type != TB_INI_STRING) return 0;

    return tb_ini_strncpy(dst, element->start + 1, element->len - 2, dst_len);
}

size_t tb_ini_get_name(const tb_ini_element* element, char* dst, size_t dst_len)
{
    return tb_ini_strncpy(dst, element->name, element->name_len, dst_len);
}

char* tb_ini_atoi(char* p, int32_t* result)
{
    uint8_t neg = 0;
    if (*p == '-')
    {
        neg = 1;
        ++p;
    }

    int32_t x = 0;
    while (*p >= '0' && *p <= '9')
    {
        x = (x * 10) + (*p - '0');
        ++p;
    }

    *result = neg ? -x : x;
    return p;
}

void tb_ini_print_element(tb_ini_element* element)
{
    if (element->type != TB_INI_ERROR)
        printf("%.*s (%s)", (uint32_t)element->len, element->start, tb_ini_get_type_name(element->type));
    else
        printf("ERROR: %s", tb_ini_get_error_desc(element->error));
}

const char* tb_ini_get_type_name(tb_ini_type type)
{
    switch (type)
    {
    case TB_INI_SECTION: return "section";
    case TB_INI_STRING:  return "string";
    case TB_INI_INT:     return "int";
    case TB_INI_BOOL:    return "bool";
    default:             return "error";
    }
}

const char* tb_ini_get_error_desc(tb_ini_error error)
{
    switch (error)
    {
    case TB_INI_OK:                 return "no error";
    case TB_INI_BAD_NAME:           return "bad name";
    case TB_INI_BAD_SECTION:        return "bad section";
    case TB_INI_BAD_PROPERTY:       return "bad property";
    case TB_INI_BAD_NUMBER:         return "bad number";
    case TB_INI_UNKOWN_VALUE:       return "unkown value type";
    case TB_INI_UNTERMINATED_STR:   return "unterminated string";
    case TB_INI_MEM_ERROR:          return "memory error";
    default:                        return "unkown error";
    }
}