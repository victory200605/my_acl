#include "apl/str.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

/* ---------------------------------------------------------------------- */

apl_size_t apl_strlen(
    char const* apc_str)
{
    return (apl_size_t) strlen(apc_str);
}

/* ---------------------------------------------------------------------- */

char* apl_strncpy(
    char*       apc_dest,
    char const* apc_src,
    apl_size_t  au_destlen)
{
    char* lpc_cur = apc_dest;

    while (au_destlen > 0 && lpc_cur - apc_dest < au_destlen - 1 && '\0' != *apc_src)
    {
        *lpc_cur++ = *apc_src++;
    }

    if (au_destlen > 0)
    {
        *lpc_cur = '\0';
    }

    return apc_dest;
}

/* ---------------------------------------------------------------------- */

char* apl_strncat(
    char*       apc_dest,
    char const* apc_src,
    apl_size_t  au_destlen)
{
    apl_size_t  li_len = apl_strlen(apc_dest);

    if (au_destlen < li_len+2)
        return apc_dest;

    char *lpc_cur = apc_dest + li_len;

    while (lpc_cur < (apc_dest + au_destlen -1) && '\0' != *apc_src)
    {
        *lpc_cur++ = *apc_src++;
    }

    *lpc_cur = '\0';

    return apc_dest;
}

/* ---------------------------------------------------------------------- */

char* apl_strchr(
    char const* apc_str,
    apl_uint8_t au8_chr)
{
    return (char*) strchr(apc_str, (int)au8_chr);
}

/* ---------------------------------------------------------------------- */

char* apl_strrchr(
    char const* apc_str,
    apl_uint8_t au8_chr)
{
    return (char*) strrchr(apc_str, (int)au8_chr);
}

/* ---------------------------------------------------------------------- */

char* apl_strstr(
    char const* apc_str,
    char const* apc_substr)
{
    return (char*) strstr(apc_str, apc_substr);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_strcmp(
    char const* apc_s1,
    char const* apc_s2)
{
    return (apl_int_t) strcmp(apc_s1, apc_s2);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_strncmp(
    char const* apc_s1,
    char const* apc_s2,
    apl_size_t  au_len)
{
    return (apl_int_t) strncmp(apc_s1, apc_s2, (size_t)au_len);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_strcasecmp(
    char const* apc_s1,
    char const* apc_s2)
{
    return (apl_int_t) strcasecmp(apc_s1, apc_s2);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_strncasecmp(
    char const* apc_s1,
    char const* apc_s2,
    apl_size_t  au_len)
{
    return (apl_int_t) strncasecmp(apc_s1, apc_s2, (size_t)au_len);
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_snprintf(
    char*       apc_buf,
    apl_size_t  au_buflen,
    char const* apc_fmt,
    ...) 
{
    va_list     ls_valist;
    apl_ssize_t li_ret;

    va_start(ls_valist, apc_fmt);

    li_ret = apl_vsnprintf(apc_buf, (size_t)au_buflen, apc_fmt, ls_valist);

    va_end(ls_valist);

    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_vsnprintf(
    char *      apc_buf,
    apl_size_t  au_buflen,
    char const* apc_fmt,
    va_list     as_valist)
{
    apl_int_t li_ret;

    //return -1 instead of core dump
    if (APL_NULL == apc_buf && au_buflen != 0) 
        return -1;

#if defined(APL_OS_HPUX) || defined(APL_OS_SOLARIS)
    if (au_buflen <= 1)
    {
        char lac_tmp[2];
        li_ret = vsnprintf(lac_tmp, sizeof(lac_tmp), apc_fmt, as_valist);

        if (APL_NULL != apc_buf && 1 == au_buflen)
        {
            apc_buf[0] = '\0';
        }

        return li_ret;
    }
#endif
    
    li_ret = vsnprintf(apc_buf, (size_t)au_buflen, apc_fmt, as_valist);

    return li_ret;
}

/* ---------------------------------------------------------------------- */

char* apl_strdup(
    char const* apc_str)
{
    return (char*) strdup(apc_str);
}

/* ---------------------------------------------------------------------- */

