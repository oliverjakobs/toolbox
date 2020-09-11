#include "tb_jwrite.h"

#include <stddef.h>
#include <string.h>

static char* _tb_jwrite_itoa(char* buffer, int32_t value);
static void _tb_jwrite_ftoa(char* buffer, float value, int precision);

static void _tb_jwrite_put_ch(tb_jwrite_control* jwc, char c)
{
    if (fprintf(jwc->file, "%c", c) < 0)
        jwc->error = TB_JWRITE_WRITE_ERROR;
}

/* put string enclosed in quotes */
static void _tb_jwrite_put_str(tb_jwrite_control* jwc, const char* str)
{
    if (fprintf(jwc->file, "\"%s\"", str) < 0)
        jwc->error = TB_JWRITE_WRITE_ERROR;
}

/* put raw string */
static void _tb_jwrite_put_raw(tb_jwrite_control* jwc, const char* str)
{
    if (fprintf(jwc->file, "%s", str) < 0)
        jwc->error = TB_JWRITE_WRITE_ERROR;
}

static void _tb_jwrite_style(tb_jwrite_control* jwc)
{
    if (jwc->style == TB_JWRITE_NEWLINE)
    {
        _tb_jwrite_put_ch(jwc, '\n');
        for (int i = 0; i < jwc->stack_pos + 1; i++)
            _tb_jwrite_put_raw(jwc, "    ");
    }
    else if (jwc->style == TB_JWRITE_INLINE)
    {
        _tb_jwrite_put_ch(jwc, ' ');
    }
}

/* Push / Pop node stack */
static void _tb_jwrite_push(tb_jwrite_control* jwc, tb_jwrite_node_type node_type)
{
    if ((jwc->stack_pos + 1) >= TB_JWRITE_STACK_DEPTH)
    {
        jwc->error = TB_JWRITE_STACK_FULL; /* array/object nesting > TB_JWRITE_STACK_DEPTH */
    }
    else
    {
        jwc->nodes[++jwc->stack_pos].type = node_type;
        jwc->nodes[jwc->stack_pos].element = 0;
    }
}

static tb_jwrite_node_type _tb_jwrite_pop(tb_jwrite_control* jwc)
{
    tb_jwrite_node_type node = jwc->nodes[jwc->stack_pos].type;
    if (jwc->stack_pos == 0)
        jwc->error = TB_JWRITE_STACK_EMPTY; /* stack underflow error (too many 'end's) */
    else
        jwc->stack_pos--;
    return node;
}

tb_jwrite_error tb_jwrite_open(tb_jwrite_control* jwc, const char* target, tb_jwrite_node_type root_type, tb_jwrite_style style)
{
    jwc->file = fopen(target, "w");

    if (!jwc->file) return TB_JWRITE_FILE_ERROR;

    jwc->nodes[0].type = root_type;
    jwc->nodes[0].element = 0;
    jwc->stack_pos = 0;
    jwc->error = TB_JWRITE_OK;
    jwc->call = 1;
    jwc->style = style;
    jwc->float_prec = 6;

    _tb_jwrite_put_ch(jwc, (root_type == TB_JWRITE_OBJECT) ? '{' : '[');

    return TB_JWRITE_OK;
}

tb_jwrite_error tb_jwrite_close(tb_jwrite_control* jwc)
{
    if (jwc->error == TB_JWRITE_OK)
    {
        if (jwc->stack_pos == 0)
        {
            tb_jwrite_node_type node = jwc->nodes[0].type;
            if (jwc->style == TB_JWRITE_NEWLINE)
                _tb_jwrite_put_ch(jwc, '\n');

            _tb_jwrite_put_ch(jwc, (node == TB_JWRITE_OBJECT) ? '}' : ']');
        }
        else
        {
            /* nesting error, not all objects closed when tb_jwrite_close() called */
            jwc->error = TB_JWRITE_NEST_ERROR;
        }
    }
    fclose(jwc->file);
    return jwc->error;
}

void tb_jwrite_set_style(tb_jwrite_control* jwc, tb_jwrite_style style)
{
    jwc->style = style;
    jwc->call++;
}

void tb_jwrite_set_float_prec(tb_jwrite_control* jwc, int prec)
{
    jwc->float_prec = prec;
    jwc->call++;
}

tb_jwrite_error tb_jwrite_end(tb_jwrite_control* jwc)
{
    if (jwc->error == TB_JWRITE_OK)
    {
        int last_element = jwc->nodes[jwc->stack_pos].element;
        tb_jwrite_node_type node = _tb_jwrite_pop(jwc);
        if (last_element > 0)
            _tb_jwrite_style(jwc);
        _tb_jwrite_put_ch(jwc, (node == TB_JWRITE_OBJECT) ? '}' : ']');
    }
    return jwc->error;
}


/* ----------------------| object |------------------------------- */

/*
 * common object function:
 *  - checks error
 *  - checks current node is OBJECT
 *  - adds comma if required
 *  - adds "key" :
 */
static tb_jwrite_error _tb_jwrite_object(tb_jwrite_control* jwc, const char* key)
{
    if (jwc->error == TB_JWRITE_OK)
    {
        jwc->call++;
        if (jwc->nodes[jwc->stack_pos].type != TB_JWRITE_OBJECT)
            jwc->error = TB_JWRITE_NOT_OBJECT; /* tried to write object key/value into array */
        else if (jwc->nodes[jwc->stack_pos].element++ > 0)
            _tb_jwrite_put_ch(jwc, ',');
        _tb_jwrite_style(jwc);
        _tb_jwrite_put_str(jwc, key);
        _tb_jwrite_put_ch(jwc, ':');
        if (jwc->style == TB_JWRITE_NEWLINE)
            _tb_jwrite_put_ch(jwc, ' ');
    }
    return jwc->error;
}

void tb_jwrite_raw(tb_jwrite_control* jwc, const char* key, const char* rawtext)
{
    if (_tb_jwrite_object(jwc, key) == TB_JWRITE_OK)
        _tb_jwrite_put_raw(jwc, rawtext);
}

void tb_jwrite_string(tb_jwrite_control* jwc, const char* key, const char* value)
{
    if (_tb_jwrite_object(jwc, key) == TB_JWRITE_OK)
        _tb_jwrite_put_str(jwc, value);
}

void tb_jwrite_int(tb_jwrite_control* jwc, const char* key, int32_t value)
{
    _tb_jwrite_itoa(jwc->tmp_buf, value);
    tb_jwrite_raw(jwc, key, jwc->tmp_buf);
}

void tb_jwrite_float(tb_jwrite_control* jwc, const char* key, float value)
{
    _tb_jwrite_ftoa(jwc->tmp_buf, value, jwc->float_prec);
    tb_jwrite_raw(jwc, key, jwc->tmp_buf);
}

void tb_jwrite_null(tb_jwrite_control* jwc, const char* key)
{
    tb_jwrite_raw(jwc, key, "null");
}

void tb_jwrite_object(tb_jwrite_control* jwc, const char* key)
{
    if (_tb_jwrite_object(jwc, key) == TB_JWRITE_OK)
    {
        _tb_jwrite_put_ch(jwc, '{');
        _tb_jwrite_push(jwc, TB_JWRITE_OBJECT);
    }
}

void tb_jwrite_array(tb_jwrite_control* jwc, const char* key)
{
    if (_tb_jwrite_object(jwc, key) == TB_JWRITE_OK)
    {
        _tb_jwrite_put_ch(jwc, '[');
        _tb_jwrite_push(jwc, TB_JWRITE_ARRAY);
    }
}

/* ----------------------| array |-------------------------------- */

/*
 * common array function
 *  - checks error
 *  - checks current node is ARRAY
 *  - adds comma if required
 */
tb_jwrite_error _tb_jwrite_array(tb_jwrite_control* jwc)
{
    if (jwc->error == TB_JWRITE_OK)
    {
        jwc->call++;
        if (jwc->nodes[jwc->stack_pos].type != TB_JWRITE_ARRAY)
            jwc->error = TB_JWRITE_NOT_ARRAY; /* tried to write array value into Object */
        else if (jwc->nodes[jwc->stack_pos].element++ > 0)
            _tb_jwrite_put_ch(jwc, ',');
        _tb_jwrite_style(jwc);
    }
    return jwc->error;
}

void tb_jwrite_array_raw(tb_jwrite_control* jwc, const char* rawtext)
{
    if (_tb_jwrite_array(jwc) == TB_JWRITE_OK)
        _tb_jwrite_put_raw(jwc, rawtext);
}

void tb_jwrite_array_string(tb_jwrite_control* jwc, const char* value)
{
    if (_tb_jwrite_array(jwc) == TB_JWRITE_OK)
        _tb_jwrite_put_str(jwc, value);
}

void tb_jwrite_array_int(tb_jwrite_control* jwc, int32_t value)
{
    _tb_jwrite_itoa(jwc->tmp_buf, value);
    tb_jwrite_array_raw(jwc, jwc->tmp_buf);
}

void tb_jwrite_array_float(tb_jwrite_control* jwc, float value)
{
    _tb_jwrite_ftoa(jwc->tmp_buf, value, jwc->float_prec);
    tb_jwrite_array_raw(jwc, jwc->tmp_buf);
}

void tb_jwrite_array_null(tb_jwrite_control* jwc)
{
    tb_jwrite_array_raw(jwc, "null");
}

void tb_jwrite_array_object(tb_jwrite_control* jwc)
{
    if (_tb_jwrite_array(jwc) == TB_JWRITE_OK)
    {
        _tb_jwrite_put_ch(jwc, '{');
        _tb_jwrite_push(jwc, TB_JWRITE_OBJECT);
    }
}

void tb_jwrite_array_array(tb_jwrite_control* jwc)
{
    if (_tb_jwrite_array(jwc) == TB_JWRITE_OK)
    {
        _tb_jwrite_put_ch(jwc, '[');
        _tb_jwrite_push(jwc, TB_JWRITE_ARRAY);
    }
}

int tb_jwrite_error_pos(tb_jwrite_control* jwc)
{
    return jwc->call;
}

char* tb_jwrite_error_string(tb_jwrite_error err)
{
    switch (err)
    {
    case TB_JWRITE_OK:          return "OK";
    case TB_JWRITE_NOT_ARRAY:   return "tried to write Array value into Object";
    case TB_JWRITE_NOT_OBJECT:  return "tried to write Object key/value into Array";
    case TB_JWRITE_STACK_FULL:  return "array/object nesting > TB_JWRITE_STACK_DEPTH";
    case TB_JWRITE_STACK_EMPTY: return "stack underflow error (too many 'end's)";
    case TB_JWRITE_NEST_ERROR:  return "nesting error, not all objects closed when tb_jwrite_close() called";
    case TB_JWRITE_WRITE_ERROR: return "failed to write to file";
    default:                    return "Unknown error";
    }
}

/* Utility function to reverse a string  */
static void _tb_jwrite_strreverse(char* str, size_t length) 
{
    char temp;
    for (size_t start = 0; start < length; start++, length--)
    {
        temp = str[start];
        str[start] = str[length];
        str[length] = temp;
    }
} 

/*
 * convert an signed integer to char buffer 
 * make sure buf is big enough
 */
char* _tb_jwrite_itoa(char* buf, int32_t value)
{
    size_t i = 0;

    /* handle the sign */
    uint32_t uvalue = (value < 0) ? -value : value;

    /* Process individual digits */
    do buf[i++] = (char)(48 + (uvalue % 10)); while (uvalue /= 10);
  
    if (value < 0) buf[i++] = '-';
    buf[i] = '\0';
  
    /* Reverse the string */ 
    _tb_jwrite_strreverse(buf, i - 1);

    return buf;
}

/*
 * Taken from: https://github.com/client9/stringencoders
 * Released under the MIT license.
 */

/* Powers of 10 - * 10^0 to 10^9 */
static const double pow10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000,
                               10000000, 100000000, 1000000000 };

/*
 * \brief convert a floating point number to char buffer with a
 *         variable-precision format, and no trailing zeros
 *
 * This is similar to "%.[0-9]f" in the printf style, except it will
 * NOT include trailing zeros after the decimal point.  This type
 * of format oddly does not exists with printf.
 *
 * If the input value is greater than 1<<31, then the output format
 * will be switched exponential format.
 *
 * \param[in] value
 * \param[out] buf  The allocated output buffer.  Should be 32 chars or more.
 * \param[in] precision  Number of digits to the right of the decimal point.
 *    Can only be 0-9.
 */
void _tb_jwrite_ftoa(char* buf, float value, int precision)
{
    /* 
     * Hacky test for NaN
     * under -fast-math this won't work, but then you also won't
     * have correct nan values anyways.  The alternative is
     * to link with libmath (bad) or hack IEEE double bits (bad)
     */
    if (!(value == value))
        strcpy(buf,"nan");

    /* if input is larger than thres_max, revert to exponential */
    const double thres_max = (double)(0x7FFFFFFF);

    double diff = 0.0;
    char* wstr = buf;

    if (precision < 0)
        precision = 0;
    else if (precision > 9)
        precision = 9; /* precision of >= 10 can lead to overflow errors */

    /* we'll work in positive values and deal with the negative sign issue later */
    int neg = 0;
    if (value < 0)
    {
        neg = 1;
        value = -value;
    }

    int whole = (int)value;
    double tmp = ((double)value - whole) * pow10[precision];
    uint32_t frac = (uint32_t)(tmp);
    diff = tmp - frac;

    if ((diff > 0.5)
        /* if halfway, round up if odd, OR if last digit is 0. That last part is strange */
        || (diff == 0.5 && precision > 0 && (frac & 1)) 
        || (diff == 0.5 && precision == 0 && (whole & 1)))
    {
        ++frac;
        /* handle rollover, e.g.  case 0.99 with prec 1 is 1.0  */
        if (frac >= pow10[precision])
        {
            frac = 0;
            ++whole;
        }
    }

    /* 
     * for very large numbers switch back to native sprintf for exponentials.
     * normal printf behavior is to print EVERY whole number digit
     * which can be 100s of characters overflowing your buffers == bad
     */
    if (value > thres_max)
    {
        sprintf(buf, "%e", neg ? -value : value);
        return;
    }

    int has_decimal = 0;
    int count = precision;

    /* Remove ending zeros */
    if (precision > 0)
    {
        while (count > 0 && ((frac % 10) == 0))
        {
            count--;
            frac /= 10;
        }
    }

    while (count > 0)
    {
        --count;
        *wstr++ = (char)(48 + (frac % 10));
        frac /= 10;
        has_decimal = 1;
    }

    if (frac > 0) ++whole;

    /* add decimal */
    if (has_decimal) *wstr++ = '.';

    /* do whole part, take care of sign conversion. Number is reversed. */
    do *wstr++ = (char)(48 + (whole % 10)); while (whole /= 10);

    if (neg) *wstr++ = '-';
    *wstr = '\0';

    _tb_jwrite_strreverse(buf, (wstr - buf) - 1);
}