#include "apl/synch.h"
#include "apl/time.h"

/* ---------------------------------------------------------------------- */

apl_int_t apl_sema_init(
    struct apl_sema_t*  aps_sema,
    apl_int_t           ai_value)
{
    return (apl_int_t) sem_init(&(aps_sema->ms_sema), 0, (unsigned)ai_value);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sema_destroy(
    struct apl_sema_t*  aps_sema)
{
    return (apl_int_t) sem_destroy(&(aps_sema->ms_sema));
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sema_post(
    struct apl_sema_t*  aps_sema)
{
    return (apl_int_t) sem_post(&(aps_sema->ms_sema));
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sema_wait(
    struct apl_sema_t*  aps_sema,
    apl_time_t          ai64_timeout)
{
    if (ai64_timeout < APL_INT64_C(0))
    {
        return (apl_int_t) sem_wait(&(aps_sema->ms_sema));
    }
    else if (APL_INT64_C(0) == ai64_timeout)
    {
        return (apl_int_t) sem_trywait(&(aps_sema->ms_sema));
    }
    else
    {
        struct timespec ls_ts;
        apl_int_t li_ret;

        li_ret = (apl_int_t) clock_gettime(CLOCK_REALTIME, &ls_ts);
        assert(0 == li_ret);

        APL_TIME_TO_TIMESPEC(&ls_ts, APL_TIME_FROM_TIMESPEC(&ls_ts) + ai64_timeout);

        return (apl_int_t) sem_timedwait(&(aps_sema->ms_sema), &ls_ts);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sema_wait_abs(
    struct apl_sema_t*  aps_sema,
    apl_time_t          ai64_abstime)
{
    if (ai64_abstime < APL_INT64_C(0))
    {
        return (apl_int_t) sem_wait(&(aps_sema->ms_sema));
    }
    else if (APL_INT64_C(0) == ai64_abstime)
    {
        return (apl_int_t) sem_trywait(&(aps_sema->ms_sema));
    }
    else
    {
        struct timespec ls_ts;

        APL_TIME_TO_TIMESPEC(&ls_ts, ai64_abstime);

        return (apl_int_t) sem_timedwait(&(aps_sema->ms_sema), &ls_ts);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sema_getvalue(
    struct apl_sema_t*  aps_sema,
    apl_int_t*          api_value)
{
    int         li_value;
    apl_int_t   li_ret;
    
    li_ret = (apl_int_t) sem_getvalue(&(aps_sema->ms_sema), &li_value);

    if (0 == li_ret)
    {
        *api_value = (apl_int_t) li_value;
    }

    return li_ret;
}

/* ---------------------------------------------------------------------- */

