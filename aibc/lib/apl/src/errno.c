#include "apl/errno.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <netdb.h>

/* ---------------------------------------------------------------------- */

apl_int_t apl_get_errno(void)
{
    return (apl_int_t) errno;
}

/* ---------------------------------------------------------------------- */

void apl_set_errno(
    apl_int_t ai_err)
{
    errno = (int) ai_err;
}

/* ---------------------------------------------------------------------- */

char const* apl_strerror(
    apl_int_t ai_err)
{
    if (ai_err & APL_H_ERRNO_MASK)
    {
#if defined(HAVE_HSTRERROR)
        return (char const*) hstrerror(ai_err & ~APL_H_ERRNO_MASK);
#else
        switch (ai_err & ~APL_H_ERRNO_MASK)
        {
            case HOST_NOT_FOUND:
                return "host not found";

            case TRY_AGAIN:
                return "try again";

            case NO_RECOVERY:
                return "no recovery";

            case NO_DATA:
                return "no data";

            default:
                return "unknown error";
        }
#endif
    }
    else if (ai_err & APL_GAI_ERRNO_MASK)
    {
        return (char const*) gai_strerror(ai_err & ~APL_GAI_ERRNO_MASK);
    }
    else
    {
        return (char const*) strerror((int) ai_err);
    }
}

/* ---------------------------------------------------------------------- */

void apl_perror(
    char const* apc_msg)
{
    apl_errprintf("%s: %s\n", apc_msg, apl_strerror(errno));
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_errprintf(
    char const* apc_fmt,
    ...)
{
    va_list     ls_ap;
    apl_ssize_t li_ret;
    apl_int_t   li_errno = apl_get_errno();

    va_start(ls_ap, apc_fmt);

    li_ret = vfprintf(stderr, apc_fmt, ls_ap);

    va_end(ls_ap);

    apl_set_errno(li_errno);

    return li_ret;
}

/* ---------------------------------------------------------------------- */

