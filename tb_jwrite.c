#include "tb_jwrite.h"

#include <stddef.h>
#include <string.h>

static void _tb_jwrite_modp_itoa10(int32_t value, char* str);
static void _tb_jwrite_modp_ftoa2(float value, char* str, int prec);


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
        for (int i = 0; i < jwc->stackpos + 1; i++)
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
    if ((jwc->stackpos + 1) >= TB_JWRITE_STACK_DEPTH)
    {
        jwc->error = TB_JWRITE_STACK_FULL; /* array/object nesting > TB_JWRITE_STACK_DEPTH */
    }
    else
    {
        jwc->nodes[++jwc->stackpos].type = node_type;
        jwc->nodes[jwc->stackpos].element = 0;
    }
}

static tb_jwrite_node_type _tb_jwrite_pop(tb_jwrite_control* jwc)
{
    tb_jwrite_node_type node = jwc->nodes[jwc->stackpos].type;
    if (jwc->stackpos == 0)
        jwc->error = TB_JWRITE_STACK_EMPTY; /* stack underflow error (too many 'end's) */
    else
        jwc->stackpos--;
    return node;
}

//------------------------------------------
// tb_jwrite_open
// - open writing of JSON starting with rootType = TB_JWRITE_OBJECT or TB_JWRITE_ARRAY
// - initialise with user string buffer of length buflen
// - isPretty=TB_JWRITE_PRETTY adds \n and spaces to prettify output (else TB_JWRITE_COMPACT)
void tb_jwrite_open(tb_jwrite_control* jwc, const char* target, tb_jwrite_node_type root_type, tb_jwrite_style style)
{
    jwc->file = fopen(target, "w");

    jwc->nodes[0].type = root_type;
    jwc->nodes[0].element = 0;
    jwc->stackpos = 0;
    jwc->error = TB_JWRITE_OK;
    jwc->call = 1;
    jwc->style = style;
    jwc->float_prec = 6;

    _tb_jwrite_put_ch(jwc, (root_type == TB_JWRITE_OBJECT) ? '{' : '[');
}

//------------------------------------------
// tb_jwrite_close
// - closes the root JSON object started by jwOpen()
// - returns error code
tb_jwrite_error tb_jwrite_close(tb_jwrite_control* jwc)
{
    if (jwc->error == TB_JWRITE_OK)
    {
        if (jwc->stackpos == 0)
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
}

void tb_jwrite_set_float_prec(tb_jwrite_control* jwc, int prec)
{
    jwc->float_prec = prec;
}

//------------------------------------------
// End the current array/object
tb_jwrite_error tb_jwrite_end(tb_jwrite_control* jwc)
{
    if (jwc->error == TB_JWRITE_OK)
    {
        int last_element = jwc->nodes[jwc->stackpos].element;
        tb_jwrite_node_type node = _tb_jwrite_pop(jwc);
        if (last_element > 0)
            _tb_jwrite_style(jwc);
        _tb_jwrite_put_ch(jwc, (node == TB_JWRITE_OBJECT) ? '}' : ']');
    }
    return jwc->error;
}

//------------------------------------------
// Object insert functions
//

// *common Object function*
// - checks error
// - checks current node is OBJECT
// - adds comma if reqd
// - adds "key" :
static tb_jwrite_error _tb_jwrite_object(tb_jwrite_control* jwc, const char* key)
{
    if (jwc->error == TB_JWRITE_OK)
    {
        jwc->call++;
        if (jwc->nodes[jwc->stackpos].type != TB_JWRITE_OBJECT)
            jwc->error = TB_JWRITE_NOT_OBJECT; // tried to write Object key/value into Array
        else if (jwc->nodes[jwc->stackpos].element++ > 0)
            _tb_jwrite_put_ch(jwc, ',');
        _tb_jwrite_style(jwc);
        _tb_jwrite_put_str(jwc, key);
        _tb_jwrite_put_ch(jwc, ':');
        if (jwc->style == TB_JWRITE_NEWLINE)
            _tb_jwrite_put_ch(jwc, ' ');
    }
    return jwc->error;
}

// put raw string to object (i.e. contents of rawtext without quotes)
void tb_jwrite_object_raw(tb_jwrite_control* jwc, const char* key, const char* rawtext)
{
    if (_tb_jwrite_object(jwc, key) == TB_JWRITE_OK)
        _tb_jwrite_put_raw(jwc, rawtext);
}

// put "quoted" string to object
void tb_jwrite_string(tb_jwrite_control* jwc, const char* key, const char* value)
{
    if (_tb_jwrite_object(jwc, key) == TB_JWRITE_OK)
        _tb_jwrite_put_str(jwc, value);
}

void tb_jwrite_int(tb_jwrite_control* jwc, const char* key, int32_t value)
{
    _tb_jwrite_modp_itoa10(value, jwc->tmpbuf);
    tb_jwrite_object_raw(jwc, key, jwc->tmpbuf);
}

void tb_jwrite_float(tb_jwrite_control* jwc, const char* key, float value)
{
    _tb_jwrite_modp_ftoa2(value, jwc->tmpbuf, jwc->float_prec);
    tb_jwrite_object_raw(jwc, key, jwc->tmpbuf);
}

void tb_jwrite_null(tb_jwrite_control* jwc, const char* key)
{
    tb_jwrite_object_raw(jwc, key, "null");
}

// put Object in Object
void tb_jwrite_object(tb_jwrite_control* jwc, const char* key)
{
    if (_tb_jwrite_object(jwc, key) == TB_JWRITE_OK)
    {
        _tb_jwrite_put_ch(jwc, '{');
        _tb_jwrite_push(jwc, TB_JWRITE_OBJECT);
    }
}

// put Array in Object
void tb_jwrite_array(tb_jwrite_control* jwc, const char* key)
{
    if (_tb_jwrite_object(jwc, key) == TB_JWRITE_OK)
    {
        _tb_jwrite_put_ch(jwc, '[');
        _tb_jwrite_push(jwc, TB_JWRITE_ARRAY);
    }
}

//------------------------------------------
// Array insert functions
//

// *common Array function*
// - checks error
// - checks current node is ARRAY
// - adds comma if reqd
tb_jwrite_error _tb_jwrite_array(tb_jwrite_control* jwc)
{
    if (jwc->error == TB_JWRITE_OK)
    {
        jwc->call++;
        if (jwc->nodes[jwc->stackpos].type != TB_JWRITE_ARRAY)
            jwc->error = TB_JWRITE_NOT_ARRAY; // tried to write array value into Object
        else if (jwc->nodes[jwc->stackpos].element++ > 0)
            _tb_jwrite_put_ch(jwc, ',');
        _tb_jwrite_style(jwc);
    }
    return jwc->error;
}

// put raw string to array (i.e. contents of rawtext without quotes)
//
void tb_jwrite_array_raw(tb_jwrite_control* jwc, const char* rawtext)
{
    if (_tb_jwrite_array(jwc) == TB_JWRITE_OK)
        _tb_jwrite_put_raw(jwc, rawtext);
}

// put "quoted" string to array
//
void tb_jwrite_array_string(tb_jwrite_control* jwc, const char* value)
{
    if (_tb_jwrite_array(jwc) == TB_JWRITE_OK)
        _tb_jwrite_put_str(jwc, value);
}

void tb_jwrite_array_int(tb_jwrite_control* jwc, int32_t value)
{
    _tb_jwrite_modp_itoa10(value, jwc->tmpbuf);
    tb_jwrite_array_raw(jwc, jwc->tmpbuf);
}

void tb_jwrite_array_float(tb_jwrite_control* jwc, float value)
{
    _tb_jwrite_modp_ftoa2(value, jwc->tmpbuf, jwc->float_prec);
    tb_jwrite_array_raw(jwc, jwc->tmpbuf);
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

//------------------------------------------
// tb_jwrite_error_pos
// - Returns position of error: the nth call to a TB_JWRITE function
//
int tb_jwrite_error_pos(tb_jwrite_control* jwc)
{
    return jwc->call;
}

//------------------------------------------
// tb_jwrite_error_string
// - returns string describing error code
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

//=================================================================
//
// modp value-to-string functions
// - modified for C89
//
// We use these functions as they are a lot faster than sprintf()
//
// Origin of these routines:
/*
 * <pre>
 * Copyright &copy; 2007, Nick Galbreath -- nickg [at] modp [dot] com
 * All rights reserved.
 * http://code.google.com/p/stringencoders/
 * Released under the bsd license.
 * </pre>
 */

static void _tb_jwrite_strreverse(char* begin, char* end)
{
    char aux;
    while (end > begin)
        aux = *end, * end-- = *begin, * begin++ = aux;
}

/* \brief convert an signed integer to char buffer
 *
 * \param[in] value
 * \param[out] buf the output buffer.  Should be 16 chars or more.
 */
void _tb_jwrite_modp_itoa10(int32_t value, char* str)
{
    char* wstr = str;
    // Take care of sign
    unsigned int uvalue = (value < 0) ? -value : value;
    // Conversion. Number is reversed.
    do *wstr++ = (char)(48 + (uvalue % 10)); while (uvalue /= 10);
    if (value < 0) *wstr++ = '-';
    *wstr = '\0';

    // Reverse string
    _tb_jwrite_strreverse(str, wstr - 1);
}

/*
 * Powers of 10
 * 10^0 to 10^9
 */
static const double pow10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000,
                               10000000, 100000000, 1000000000 };

/* \brief convert a floating point number to char buffer with a
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
void _tb_jwrite_modp_ftoa2(float value, char* str, int prec)
{
    /* if input is larger than thres_max, revert to exponential */
    const double thres_max = (double)(0x7FFFFFFF);
    int count;
    double diff = 0.0;
    char* wstr = str;
    int neg = 0;
    int whole;
    double tmp;
    uint32_t frac;

    /* 
     * Hacky test for NaN
     * under -fast-math this won't work, but then you also won't
     * have correct nan values anyways.  The alternative is
     * to link with libmath (bad) or hack IEEE double bits (bad)
     */
    if (!(value == value))
    {
        str[0] = 'n'; str[1] = 'a'; str[2] = 'n'; str[3] = '\0';
        return;
    }

    if (prec < 0)
        prec = 0;
    else if (prec > 9) /* precision of >= 10 can lead to overflow errors */
        prec = 9;

    /* 
     * we'll work in positive values and deal with the
     * negative sign issue later 
     */
    if (value < 0) {
        neg = 1;
        value = -value;
    }

    whole = (int)value;
    tmp = ((double)value - whole) * pow10[prec];
    frac = (uint32_t)(tmp);
    diff = tmp - frac;

    if (diff > 0.5)
    {
        ++frac;
        /* handle rollover, e.g.  case 0.99 with prec 1 is 1.0  */
        if (frac >= pow10[prec])
        {
            frac = 0;
            ++whole;
        }
    }
    else if (diff == 0.5 && ((frac == 0) || (frac & 1)))
    {
        /* if halfway, round up if odd, OR if last digit is 0.  That last part is strange */
        ++frac;
    }

    /*
     * for very large numbers switch back to native sprintf for exponentials.
     * anyone want to write code to replace this?
     * normal printf behavior is to print EVERY whole number digit
     * which can be 100s of characters overflowing your buffers == bad
     */
    if (value > thres_max)
    {
        sprintf(str, "%e", neg ? -value : value);
        return;
    }

    if (prec == 0)
    {
        diff = (double)value - whole;
        if (diff > 0.5)
        {
            /* greater than 0.5, round up, e.g. 1.6 -> 2 */
            ++whole;
        }
        else if (diff == 0.5 && (whole & 1))
        {
            /* exactly 0.5 and ODD, then round up */
            /* 1.5 -> 2, but 2.5 -> 2 */
            ++whole;
        }

        /* vvvvvvvvvvvvvvvvvvv  Diff from modp_dto2 */
    }
    else if (frac)
    {
        count = prec;
        /*
         * now do fractional part, as an unsigned number
         * we know it is not 0 but we can have leading zeros, these
         * should be removed
         */
        while (!(frac % 10))
        {
            --count;
            frac /= 10;
        }
        /* ^^^^^^^^^^^^^^^^^^^  Diff from modp_dto2 */

        /* now do fractional part, as an unsigned number */
        do {
            --count;
            *wstr++ = (char)(48 + (frac % 10));
        } while (frac /= 10);
        /* add extra 0s */
        while (count-- > 0) *wstr++ = '0';
        /* add decimal */
        *wstr++ = '.';
    }

    /*
     * do whole part
     * Take care of sign
     * Conversion. Number is reversed.
     */
    do *wstr++ = (char)(48 + (whole % 10)); while (whole /= 10);

    if (neg) *wstr++ = '-';

    *wstr = '\0';
    _tb_jwrite_strreverse(str, wstr - 1);
}