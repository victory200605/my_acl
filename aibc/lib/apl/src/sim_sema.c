#include "apl/synch.h"
#include "apl/limits.h"
#include "apl/time.h"

/* ---------------------------------------------------------------------- */

#define APL_SEMA_MAGIC          (APL_INT_C(0x12345678)) 

/* ---------------------------------------------------------------------- */

static inline
apl_int_t sim_sema_wait(
    struct apl_sema_t*  aps_sema)
{
    apl_int_t   li_ret = 0;
    apl_int_t   li_ret2;

    li_ret2 = pthread_mutex_lock(&aps_sema->ms_mtx);
    assert(0 == li_ret2);

    while (aps_sema->mi_value <= 0 && 0 == li_ret)
    {
        li_ret = pthread_cond_wait(&aps_sema->ms_cond, &aps_sema->ms_mtx);
    }

    if (aps_sema->mi_value > 0)
    {
        --(aps_sema->mi_value);
        li_ret = 0;
    }

    li_ret2 = pthread_mutex_unlock(&aps_sema->ms_mtx);
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

static inline
apl_int_t sim_sema_trywait(
    struct apl_sema_t*  aps_sema)
{
    apl_int_t   li_ret = 0;
    apl_int_t   li_ret2;

    li_ret2 = pthread_mutex_lock(&aps_sema->ms_mtx);
    assert(0 == li_ret2);

    if (aps_sema->mi_value <= 0)
    {
        li_ret = APL_EBUSY;
    }
    else
    {
        --(aps_sema->mi_value);
    }

    li_ret2 = pthread_mutex_unlock(&aps_sema->ms_mtx);
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

static inline
apl_int_t sim_sema_timedwait(
    struct apl_sema_t*  aps_sema,
    struct timespec*    aps_timeout)
{
    apl_int_t   li_ret = 0;
    apl_int_t   li_ret2;

    li_ret2 = pthread_mutex_lock(&aps_sema->ms_mtx);
    assert(0 == li_ret2);

    while (aps_sema->mi_value <= 0 && 0 == li_ret)
    {
        li_ret = pthread_cond_timedwait(&aps_sema->ms_cond, &aps_sema->ms_mtx, aps_timeout);
    }

    if (aps_sema->mi_value > 0)
    {
        --(aps_sema->mi_value);
        li_ret = 0;
    }
    else if (0 == li_ret)
    {
        li_ret = APL_EINTR;
    }

    li_ret2 = pthread_mutex_unlock(&aps_sema->ms_mtx);
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sema_init(
    struct apl_sema_t*  aps_sema,
    apl_int_t           ai_value)
{
    apl_int_t   li_ret2;

    li_ret2 = pthread_mutex_init(&aps_sema->ms_mtx, APL_NULL);
    assert(0 == li_ret2);

    li_ret2 = pthread_cond_init(&aps_sema->ms_cond, APL_NULL);
    assert(0 == li_ret2);

    aps_sema->mi_magic      = APL_SEMA_MAGIC;
    aps_sema->mi_value      = ai_value;

    return 0;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sema_destroy(
    struct apl_sema_t*  aps_sema)
{
    apl_int_t   li_ret2;

    assert(APL_SEMA_MAGIC == aps_sema->mi_magic);

    li_ret2 = pthread_mutex_destroy(&aps_sema->ms_mtx);
    assert(0 == li_ret2);

    li_ret2 = pthread_cond_destroy(&aps_sema->ms_cond);
    assert(0 == li_ret2);

    aps_sema->mi_magic = 0;

    return 0;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sema_post(
    struct apl_sema_t*  aps_sema)
{
    apl_int_t   li_ret = 0;
    apl_int_t   li_ret2;

    assert(APL_SEMA_MAGIC == aps_sema->mi_magic);

    li_ret2 = pthread_mutex_lock(&aps_sema->ms_mtx);
    assert(0 == li_ret2);

    if (aps_sema->mi_value >= APL_SEM_VALUE_MAX)
    {
        li_ret = APL_EAGAIN;
    }
    else
    {
        ++(aps_sema->mi_value);
    }

    li_ret2 = pthread_cond_signal(&aps_sema->ms_cond); 
    assert(0 == li_ret2);

    li_ret2 = pthread_mutex_unlock(&aps_sema->ms_mtx);
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sema_wait(
    struct apl_sema_t*  aps_sema,
    apl_time_t          ai64_timeout)
{
    apl_int_t   li_ret;

    assert(APL_SEMA_MAGIC == aps_sema->mi_magic);

    if (ai64_timeout < APL_INT64_C(0))
    {
        li_ret = sim_sema_wait(aps_sema);
    }
    else if (APL_INT64_C(0) == ai64_timeout)
    {
        li_ret = sim_sema_trywait(aps_sema);
    }
    else 
    { 
        struct timespec ls_ts;
        li_ret = (apl_int_t) clock_gettime(CLOCK_REALTIME, &ls_ts);
        assert(0 == li_ret);

        APL_TIME_TO_TIMESPEC(&ls_ts, APL_TIME_FROM_TIMESPEC(&ls_ts) + ai64_timeout);

        li_ret = sim_sema_timedwait(aps_sema, &ls_ts);
    }

    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sema_wait_abs(
    struct apl_sema_t*  aps_sema,
    apl_time_t          ai64_abstime)
{
    apl_int_t   li_ret;

    assert(APL_SEMA_MAGIC == aps_sema->mi_magic);

    if (ai64_abstime < APL_INT64_C(0))
    {
        li_ret = sim_sema_wait(aps_sema);
    }
    else if (APL_INT64_C(0) == ai64_abstime)
    {
        li_ret = sim_sema_trywait(aps_sema);
    }
    else 
    {
        struct timespec ls_ts;
        li_ret = (apl_int_t) clock_gettime(CLOCK_REALTIME, &ls_ts);
        assert(0 == li_ret);

        li_ret = sim_sema_timedwait(aps_sema, &ls_ts);
    }

    return li_ret;
}

/* ---------------------------------------------------------------------- */


apl_int_t apl_sema_getvalue(
    struct apl_sema_t*  aps_sema,
    apl_int_t*          api_value)
{
    apl_int_t   li_ret2;

    assert(APL_SEMA_MAGIC == aps_sema->mi_magic);

    li_ret2 = pthread_mutex_lock(&aps_sema->ms_mtx);
    assert(0 == li_ret2);

    if (api_value)
    {
        *api_value = aps_sema->mi_value;
    }

    li_ret2 = pthread_mutex_unlock(&aps_sema->ms_mtx);
    assert(0 == li_ret2);

    return 0;
}

/* ---------------------------------------------------------------------- */
