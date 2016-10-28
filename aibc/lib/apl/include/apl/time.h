/**
 * @file time.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.11 $
 */ 
#ifndef APL_TIME_H
#define APL_TIME_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include <time.h>

/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */


#define apl_tm_t    tm



/** 
 * define nano second
 */
#define APL_TIME_NSEC       (APL_INT64_C(1))

/**
 * define a millisecond that used nano second
 */
#define APL_TIME_USEC       (APL_TIME_NSEC * APL_INT64_C(1000))

/**
 * define a microsecond that used nano second
 */
#define APL_TIME_MSEC       (APL_TIME_USEC * APL_INT64_C(1000))

/**
 * define a second that used nano second
 */
#define APL_TIME_SEC        (APL_TIME_MSEC * APL_INT64_C(1000))

/**
 * define a minute that used nano second
 *
 * @def APL_TIME_MIN
 */
#define APL_TIME_MIN        (APL_TIME_SEC * APL_INT64_C(60))

/**
 * define a hour that used nano second
 *
 * @def APL_TIME_HOUR
 */
#define APL_TIME_HOUR       (APL_TIME_MIN * APL_INT64_C(60))

/**
 * define a day that used nano second
 *
 * @def APL_TIME_DAY
 */
#define APL_TIME_DAY        (APL_TIME_HOUR * APL_INT64_C(24))

/**
 * define a week that used nano second
 *
 * @def APL_TIME_WEEK
 */
#define APL_TIME_WEEK       (APL_TIME_DAY * APL_INT64_C(7))



/**
 * this macro shall convert the time represented by timeval structure to the value count by nano second.
 *
 * @param[in]    tv     the timeval structure.
 *
 * @return     the time value count by nano second.
 */
#define APL_TIME_FROM_TIMEVAL(tv)   (APL_TIME_SEC * (tv)->tv_sec + APL_TIME_NSEC * (tv)->tv_usec)

/**
 * This macro shall convert the time represented by nano second to the timeval structure
 *
 * @param[out]    tv   the timeval structure.
 *
 * @param[in]     t    the time value count by nano second.
 */
#define APL_TIME_TO_TIMEVAL(tv, t)  \
    do{ \
        apl_time_t  li_t = (t); \
        (tv)->tv_sec = li_t / APL_TIME_SEC; \
        (tv)->tv_usec = (li_t % APL_TIME_SEC) / APL_TIME_USEC; \
    }while(0)


/**
 * This macro shall convert the time represented by timespec structure to the value count by nano second.
 * 
 * @param[in]    ts    the timespec structure
 *
 * @return      the time value count by nano second.
 */
#define APL_TIME_FROM_TIMESPEC(ts)  (APL_TIME_SEC * (ts)->tv_sec + APL_TIME_NSEC * (ts)->tv_nsec)

/**
 * This macro shall convert the time represented by nano second to the timespec structure
 *
 * @param[out]    ts     the timespec structure.
 *
 * @param[in]     t      the time valude count by nano second.
 */
#define APL_TIME_TO_TIMESPEC(ts, t)  \
    do{ \
        apl_time_t  li_t = (t); \
        (ts)->tv_sec = li_t / APL_TIME_SEC; \
        (ts)->tv_nsec = (li_t % APL_TIME_SEC) / APL_TIME_NSEC; \
    }while(0)


/**
 * Get current time in nanoseconds(precision:sec).
 *
 * @return current time.
 */ 
apl_time_t apl_time(void);

/**
 * Get current time in nanoseconds(precision:usec).
 *
 * @return current time.
 */ 
apl_time_t apl_gettimeofday(void);

/**
 * Get current time in nanoseconds(precision:nsec).
 *
 * @return current time. 
 */ 
apl_time_t apl_clock_gettime(void);


/**
 * Sleep until timeout.
 *
 * @param[in]    ai64_timeout  timeout
 * 
 * @retval >=0  success.
 * @retval -1   failure.
 */ 
apl_int_t apl_sleep(
    apl_time_t      ai64_timeout);


/**
 * Sleep until timeup.
 *
 * @param[in]   ai64_abstime  the final time in nanoseconds
 * 
 * @retval >=0  success.
 * @retval -1   failure.
 */ 
apl_int_t apl_sleep_until(
    apl_time_t      ai64_abstime);


/**
 * Convert broken-down time into time since the Epoch.
 *
 * @param[in]    aps_tm   the broken-down time
 *
 * @retval >= 0 success, the specified time since the Epoch 
 * @retval -1   failure.
 */ 
apl_time_t apl_mktime(
    struct apl_tm_t const* aps_tm);


/**
 * Convert a time value to a broken-down local time.
 *
 * @param[in]    ai64_time  the time in nanoseconds since the Epoch
 * @param[out]   aps_tm     the structure to store broken-down time 
 *
 * @retval aps_tm   success, return a pointer to the structure pointed to by 
 *                  the argument aps_tm.
 * @retval APL_NULL failure, and set errno to indicate the error.
 */
struct apl_tm_t* apl_localtime(
    apl_time_t          ai64_time, 
    struct apl_tm_t*    aps_tm);


/**
 * Convert a time value to a broken-down UTC(Coordinated Universal Time) time.
 *
 * @param[in]    ai64_time  the time in nanoseconds since the Epoch
 * @param[out]   aps_tm     the pointer to the structure to store 
 *                          broken-down UTC time
 *
 * @retval aps_tm   success.
 * @retval APL_NULL failure, and set errno to indicate the error
 */ 
struct apl_tm_t* apl_gmtime(
    apl_time_t          ai64_time, 
    struct apl_tm_t*    aps_tm);


/**
 * Convert date and time to a string.
 *
 * @param[in]    aps_tm      the pointer to the structure of tm
 * @param[out]   apc_buf     the output buffer to store asc time
 * @param[in]    au_buflen   the output buffer size
 *
 * @return Return the date and time sting.
 */ 
char*   apl_asctime(
    struct apl_tm_t const* aps_tm, 
    char* apc_buf, 
    apl_size_t au_buflen);


/**
 * Convert a time value to a date and time string.
 *
 * @param[in]    ai64_time   the time in nanoseconds since the Epoch
 * @param[out]   apc_buf     the buffer to store date and time string
 * @param[in]    au_buflen   the output buffer size
 *
 * @return Return the date and time sting.
 */ 
char* apl_ctime(
    apl_time_t      ai64_time, 
    char*           apc_buf, 
    apl_size_t      au_buflen);


/**
 * Convert date and time to a string.
 *
 * This function shall place bytes into the array pointed to by apv_buf as 
 * controlled by the string pointed to by apc_fmt. 
 *
 * @param[out]  apv_buf     the buffer to store date and time string
 * @param[in]   au_buflen   the output buffer size
 * @param[in]   apc_fmt     the format of output date and time string
 * @param[in]   aps_tm      the broken-down time structure
 *
 * @retval >0   success, return the number of bytes placed into the array 
 *              pointed to by apv_buf, not including the terminating null 
 *              byte.
 * @retval 0    failure.
 */ 
apl_size_t  apl_strftime(
    char*                   apv_buf, 
    apl_size_t              au_buflen, 
    char const*             apc_fmt, 
    struct apl_tm_t const*  aps_tm);


/**
 * Date and time conversion.
 *
 * This function shall convert the character string pointed to by apc_buf to 
 * values which are stored in the tm structure pointed to by aps_tm, using 
 * the format specified by apc_fmt.
 *
 * @param[in]   apc_buf the date and time sting
 * @param[in]   apc_fmt the date and time string format
 * @param[out]  aps_tm  the structure to store the value
 * 
 * @return  Return a pointer to the character following the last character 
 *          parsed. Otherwise, a null pointer shall be returned.
 */ 
char* apl_strptime(
    char const*         apc_buf,
    char const*         apc_fmt,
    struct apl_tm_t*    aps_tm);



/* ------------------------------ */  APL_DECLARE_END  /* ------------------------------ */

#endif /* APL_TIME_H */

