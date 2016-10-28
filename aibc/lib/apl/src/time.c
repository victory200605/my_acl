#include "apl/time.h"
#include "apl/errno.h"
#include <sys/time.h>
#include <stdio.h>
#include <assert.h>

/* ---------------------------------------------------------------------- */

apl_time_t apl_time(void)
{
    return APL_TIME_SEC * time(NULL);
}

/* ---------------------------------------------------------------------- */

apl_time_t apl_gettimeofday(void)
{
    struct timeval  ls_tv;

    int li_ret = gettimeofday(&ls_tv, APL_NULL);

    assert(0 == li_ret);

    return ls_tv.tv_sec * APL_TIME_SEC + ls_tv.tv_usec * APL_TIME_USEC;
}

/* ---------------------------------------------------------------------- */

apl_time_t apl_clock_gettime(void)
{
    struct timespec ls_ts;

    int li_ret = clock_gettime(CLOCK_REALTIME, &ls_ts);

    assert(0 == li_ret);

    return ls_ts.tv_sec * APL_TIME_SEC + ls_ts.tv_nsec * APL_TIME_NSEC;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sleep(
    apl_time_t      ai64_timeout)
{
    return apl_sleep_until(ai64_timeout + apl_clock_gettime());
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sleep_until(
    apl_time_t      ai64_abstime)
{
    apl_int_t       li_ret = 0;

    do {
        apl_time_t      li64_timeout = ai64_abstime - apl_clock_gettime();
        struct timeval  ls_tv;
        struct timespec ls_ts;

        if (li64_timeout <= 0)
        {
            break;
        }
        else if (li64_timeout < APL_TIME_SEC) 
        { /* nano_sleep() can only sleep for no more than one second */
            ls_ts.tv_sec = 0;
            ls_ts.tv_nsec = li64_timeout / APL_TIME_NSEC;

            li_ret = nanosleep(&ls_ts, NULL);
        }
        else 
        {
            ls_tv.tv_sec = li64_timeout / APL_TIME_SEC;
            ls_tv.tv_usec = (li64_timeout % APL_TIME_SEC) / APL_TIME_USEC;

            li_ret = select(1, NULL, NULL, NULL, &ls_tv);
        }
    } while (-1 == li_ret && apl_get_errno() == APL_EINTR);

    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_time_t apl_mktime(
    struct apl_tm_t const* aps_tm)
{
    time_t li_time = mktime((struct tm*)aps_tm);

    if ((time_t)-1 == li_time)
    {
        return (apl_time_t)-1;
    }

    return APL_TIME_SEC * li_time;
}

/* ---------------------------------------------------------------------- */

struct apl_tm_t* apl_localtime(
    apl_time_t          ai64_time,
    struct apl_tm_t*    aps_tm)
{
    time_t  li_time = (time_t)(ai64_time/APL_TIME_SEC);
    return (struct apl_tm_t*)localtime_r(&li_time, aps_tm);
}

/* ---------------------------------------------------------------------- */

struct apl_tm_t* apl_gmtime(
    apl_time_t          ai64_time,
    struct apl_tm_t*    aps_tm)
{
    time_t  li_time = (time_t)(ai64_time/APL_TIME_SEC);
    return (struct apl_tm_t*)gmtime_r(&li_time, aps_tm);
}

/* ---------------------------------------------------------------------- */

char*   apl_asctime(
    struct apl_tm_t const* aps_tm,
    char* apc_buf,
    apl_size_t au_buflen)
{
    static char saac_wday[7][3] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static char saac_mon[12][3] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    snprintf(apc_buf, au_buflen, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
        saac_wday[aps_tm->tm_wday],
        saac_mon[aps_tm->tm_mon],
        aps_tm->tm_mday, 
        aps_tm->tm_hour,
        aps_tm->tm_min, 
        aps_tm->tm_sec,
        1900 + aps_tm->tm_year);
   
    return apc_buf;
}

/* ---------------------------------------------------------------------- */

char* apl_ctime(
    apl_time_t      ai64_time,
    char*           apc_buf,
    apl_size_t      au_buflen)
{
    struct apl_tm_t ls_tm;

    return apl_asctime(apl_localtime(ai64_time, &ls_tm), apc_buf, au_buflen);
}

/* ---------------------------------------------------------------------- */

apl_size_t  apl_strftime(
    char*                   apv_buf,
    apl_size_t              au_buflen,
    char const*             apc_fmt,
    struct apl_tm_t const*  aps_tm)
{
    return (apl_size_t) strftime(apv_buf, au_buflen, apc_fmt, (struct tm*)aps_tm);
}

/* ---------------------------------------------------------------------- */

char* apl_strptime(
    char const*         apc_buf,
    char const*         apc_fmt,
    struct apl_tm_t*    aps_tm)
{
    return (char*) strptime(apc_buf, apc_fmt, (struct tm*)aps_tm);
}

/* ---------------------------------------------------------------------- */

