#include "apl/synch.h"
#include "apl/time.h"

/* ---------------------------------------------------------------------- */

apl_int_t apl_rwlock_init(
    struct apl_rwlock_t*    aps_rwl)
{
    apl_int_t li_ret;
    
    li_ret = (apl_int_t) pthread_rwlock_init(&(aps_rwl->ms_rwl), APL_NULL);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_rwlock_destroy(
    struct apl_rwlock_t*    aps_rwl)
{
    apl_int_t li_ret;
    
    li_ret = (apl_int_t) pthread_rwlock_destroy(&(aps_rwl->ms_rwl));

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_rwlock_rdlock(
    struct apl_rwlock_t*    aps_rwl,
    apl_time_t              ai64_timeout)
{
    apl_int_t   li_ret;

    if (ai64_timeout < APL_INT64_C(0))
    {
        li_ret = (apl_int_t) pthread_rwlock_rdlock(&(aps_rwl->ms_rwl));
    }
    else if (APL_INT64_C(0) == ai64_timeout)
    {
        li_ret = (apl_int_t) pthread_rwlock_tryrdlock(&(aps_rwl->ms_rwl));
    }
    else
    {
        struct timespec ls_ts;

        li_ret = (apl_int_t) clock_gettime(CLOCK_REALTIME, &ls_ts);
        assert(0 == li_ret);

        APL_TIME_TO_TIMESPEC(&ls_ts, APL_TIME_FROM_TIMESPEC(&ls_ts) + ai64_timeout);

        li_ret = (apl_int_t) pthread_rwlock_timedrdlock(&(aps_rwl->ms_rwl), &ls_ts);
    }

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_rwlock_wrlock(
    struct apl_rwlock_t*    aps_rwl,
    apl_time_t              ai64_timeout)
{
    apl_int_t   li_ret;

    if (ai64_timeout < APL_INT64_C(0))
    {
        li_ret = (apl_int_t) pthread_rwlock_wrlock(&(aps_rwl->ms_rwl));
    }
    else if (APL_INT64_C(0) == ai64_timeout)
    {
        li_ret = (apl_int_t) pthread_rwlock_trywrlock(&(aps_rwl->ms_rwl));
    }
    else
    {
        struct timespec ls_ts;

        li_ret = (apl_int_t) clock_gettime(CLOCK_REALTIME, &ls_ts);
        assert(0 == li_ret);

        APL_TIME_TO_TIMESPEC(&ls_ts, APL_TIME_FROM_TIMESPEC(&ls_ts) + ai64_timeout);

        li_ret = (apl_int_t) pthread_rwlock_timedwrlock(&(aps_rwl->ms_rwl), &ls_ts);
    }

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_rwlock_unlock(
    struct apl_rwlock_t*    aps_rwl)
{
    apl_int_t li_ret;
    
    li_ret = (apl_int_t) pthread_rwlock_unlock(&(aps_rwl->ms_rwl));

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

