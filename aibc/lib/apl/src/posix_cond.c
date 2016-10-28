#include "apl/synch.h"
#include "apl/time.h"
#include <assert.h>

/* ---------------------------------------------------------------------- */

apl_int_t apl_cond_init(
    struct apl_cond_t*  aps_cond)
{
    apl_int_t li_ret;
    
    li_ret = (apl_int_t) pthread_cond_init(&(aps_cond->ms_cond), NULL);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_cond_destroy(
    struct apl_cond_t*  aps_cond)
{
    apl_int_t li_ret;

    li_ret = (apl_int_t) pthread_cond_destroy(&(aps_cond->ms_cond));

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_cond_signal(
    struct apl_cond_t*  aps_cond)
{
    apl_int_t li_ret;
    
    li_ret = (apl_int_t) pthread_cond_signal(&(aps_cond->ms_cond));

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_cond_broadcast(
    struct apl_cond_t*  aps_cond)
{
    apl_int_t li_ret;
    
    li_ret = (apl_int_t) pthread_cond_broadcast(&(aps_cond->ms_cond));

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_cond_wait(
    struct apl_cond_t*  aps_cond,
    struct apl_mutex_t* aps_mtx,
    apl_time_t          ai64_timeout)
{
    apl_int_t   li_ret;

    if (ai64_timeout < APL_INT64_C(0))
    {
        li_ret = (apl_int_t) pthread_cond_wait(&(aps_cond->ms_cond), &(aps_mtx->ms_mtx));
    }
    else 
    {
        struct timespec ls_ts;

        li_ret = clock_gettime(CLOCK_REALTIME, &ls_ts);
        assert(0 == li_ret);

        APL_TIME_TO_TIMESPEC(&ls_ts, APL_TIME_FROM_TIMESPEC(&ls_ts) + ai64_timeout); 

        li_ret = (apl_int_t) pthread_cond_timedwait(&(aps_cond->ms_cond), &(aps_mtx->ms_mtx), &ls_ts);
    }

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_cond_wait_abs(
    struct apl_cond_t*  aps_cond,
    struct apl_mutex_t* aps_mtx,
    apl_time_t          ai64_abstime)
{
    apl_int_t   li_ret;

    if (ai64_abstime < APL_INT64_C(0))
    {
        li_ret = (apl_int_t) pthread_cond_wait(&(aps_cond->ms_cond), &(aps_mtx->ms_mtx));
    }
    else 
    {
        struct timespec ls_ts;

        APL_TIME_TO_TIMESPEC(&ls_ts, ai64_abstime); 

        li_ret = (apl_int_t) pthread_cond_timedwait(&(aps_cond->ms_cond), &(aps_mtx->ms_mtx), &ls_ts);
    }

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */
