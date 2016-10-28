#include "apl/ctype.h"

apl_int_t apl_isalpha(apl_uint8_t au8_chr)
{
    return apl_islower(au8_chr) || apl_isupper(au8_chr); 
}

apl_int_t apl_isalnum(apl_uint8_t au8_chr)
{
    return apl_isalpha(au8_chr) || apl_isdigit(au8_chr); 
}

apl_int_t apl_isascii(apl_uint8_t au8_chr)
{
    return au8_chr <= '\x7F'; 
}

apl_int_t apl_isblank(apl_uint8_t au8_chr)
{
    return '\x08' == au8_chr || '\x20' == au8_chr; 
}

apl_int_t apl_iscntrl(apl_uint8_t au8_chr)
{
    return /* au8_chr >= '\0' && */ au8_chr <= '\x1F'; 
}

apl_int_t apl_isdigit(apl_uint8_t au8_chr)
{
    return au8_chr >= '0' && au8_chr <= '9'; 
}

apl_int_t apl_isgraph(apl_uint8_t au8_chr)
{
    return au8_chr >= '\x21' && au8_chr <= '\x7E'; 
}

apl_int_t apl_islower(apl_uint8_t au8_chr)
{
    return au8_chr >= 'a' && au8_chr <= 'z'; 
}

apl_int_t apl_isprint(apl_uint8_t au8_chr)
{
    return au8_chr >= '\x20' && au8_chr <= '\x7E'; 
}

apl_int_t apl_ispunct(apl_uint8_t au8_chr)
{
    return (/* au8_chr >= '\0' && */ au8_chr <= '\x2F' && au8_chr != '\x20') 
        || (au8_chr >= '\x3A' && au8_chr <= '\x40')
        || (au8_chr >= '\x5A' && au8_chr <= '\x60')
        || au8_chr >= '\x7B';
}

apl_int_t apl_isspace(apl_uint8_t au8_chr)
{
    return (au8_chr >= '\x09' && au8_chr <= '\x0D') || '\x20' == au8_chr;
}

apl_int_t apl_isupper(apl_uint8_t au8_chr)
{
    return au8_chr >= 'A' && au8_chr <= 'Z'; 
}

apl_int_t apl_isxdigit(apl_uint8_t au8_chr)
{
    return apl_isdigit(au8_chr) || (au8_chr >= 'a' && au8_chr <= 'f') || (au8_chr >= 'A' && au8_chr <= 'F'); 
}

apl_uint8_t apl_tolower(apl_uint8_t au8_chr)
{
    if (apl_isupper(au8_chr))
    {
        return au8_chr + ('a' - 'A');
    }
    else
    {
        return au8_chr;
    }
}

apl_uint8_t apl_toupper(apl_uint8_t au8_chr)
{
    if (apl_islower(au8_chr))
    {
        return au8_chr + ('A' - 'a');
    }
    else
    {
        return au8_chr;
    }
}

