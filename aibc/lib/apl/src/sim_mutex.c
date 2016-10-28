#include "apl/synch.h"
#include "apl/time.h"
#include "apl/errno.h"
#include "apl/thread.h"
#include <assert.h>

/* ---------------------------------------------------------------------- */
#define APL_MUTEX_MAGIC         (APL_INT_C(0x1a2b3c4d))

/* ---------------------------------------------------------------------- */

static inline
apl_int_t sim_mutex_lock(
    struct apl_mutex_t*     aps_mtx)
{
    apl_int_t   li_ret = 0;
    apl_int_t   li_ret2;

    li_ret2 = pthread_mutex_lock(&aps_mtx->ms_mtx2);
    assert(0 == li_ret2);

    while ((li_ret = pthread_mutex_trylock(&aps_mtx->ms_mtx)) == APL_EBUSY)
    {
        li_ret = pthread_cond_wait(
            &aps_mtx->ms_cond2, 
            &aps_mtx->ms_mtx2);

        if (0 != li_ret)
        {
            break;
        }
    }

    li_ret2 = pthread_mutex_unlock(&aps_mtx->ms_mtx2); 
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

static inline
apl_int_t sim_mutex_trylock(
    struct apl_mutex_t*     aps_mtx)
{
    apl_int_t   li_ret;
    apl_int_t   li_ret2;

    li_ret2 = pthread_mutex_lock(&aps_mtx->ms_mtx2);
    assert(0 == li_ret2);

    li_ret = pthread_mutex_trylock(&aps_mtx->ms_mtx);

    li_ret2 = pthread_mutex_unlock(&aps_mtx->ms_mtx2); 
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

static inline
apl_int_t sim_mutex_timedlock(
    struct apl_mutex_t*     aps_mtx,
    struct timespec*        aps_timeout)
{
    apl_int_t   li_ret = 0;
    apl_int_t   li_ret2;

    li_ret2 = pthread_mutex_lock(&aps_mtx->ms_mtx2);
    assert(0 == li_ret2);

    while ((li_ret = pthread_mutex_trylock(&aps_mtx->ms_mtx)) == APL_EBUSY)
    {
        li_ret = pthread_cond_timedwait(
            &aps_mtx->ms_cond2, 
            &aps_mtx->ms_mtx2, 
            aps_timeout);

        if (0 != li_ret)
        {
            break;
        }
    }

    li_ret2 = pthread_mutex_unlock(&aps_mtx->ms_mtx2); 
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mutex_init(
    struct apl_mutex_t*     aps_mtx)
{
    apl_int_t   li_ret2;

    li_ret2 = pthread_mutex_init(&aps_mtx->ms_mtx, APL_NULL);
    assert(0 == li_ret2);

    li_ret2 = pthread_mutex_init(&aps_mtx->ms_mtx2, APL_NULL);
    assert(0 == li_ret2);

    li_ret2 = pthread_cond_init(&aps_mtx->ms_cond2, APL_NULL);
    assert(0 == li_ret2);

    aps_mtx->mi_magic   = APL_MUTEX_MAGIC;

    return 0;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mutex_destroy(
    struct apl_mutex_t*     aps_mtx)
{
    apl_int_t   li_ret2;

    assert(APL_MUTEX_MAGIC == aps_mtx->mi_magic);

    aps_mtx->mi_magic = 0;

    li_ret2 = pthread_cond_destroy(&aps_mtx->ms_cond2);
    assert(0 == li_ret2);

    li_ret2 = pthread_mutex_destroy(&aps_mtx->ms_mtx2);
    assert(0 == li_ret2);

    li_ret2 = pthread_mutex_destroy(&aps_mtx->ms_mtx);
    assert(0 == li_ret2);

    return 0;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mutex_lock(
    struct apl_mutex_t*     aps_mtx,
    apl_time_t              ai64_timeout)
{
    apl_int_t   li_ret;

    assert(APL_MUTEX_MAGIC == aps_mtx->mi_magic);

    if (ai64_timeout < APL_INT64_C(0))
    {
        li_ret = sim_mutex_lock(aps_mtx);
    }
    else if (APL_INT64_C(0) == ai64_timeout)
    {
        li_ret = sim_mutex_trylock(aps_mtx);
    }
    else
    {
        struct timespec ls_ts;

        APL_TIME_TO_TIMESPEC(
            &ls_ts, 
            apl_clock_gettime() + ai64_timeout);

        li_ret = sim_mutex_timedlock(aps_mtx, &ls_ts);
    }

    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mutex_unlock(
    struct apl_mutex_t* aps_mtx)
{
    apl_int_t   li_ret = 0;
    apl_int_t   li_ret2;

    assert(APL_MUTEX_MAGIC == aps_mtx->mi_magic);

    li_ret2 = pthread_mutex_lock(&(aps_mtx->ms_mtx2)); 
    assert(0 == li_ret2);

    li_ret = pthread_mutex_unlock(&aps_mtx->ms_mtx);

    li_ret2 = pthread_cond_signal(&aps_mtx->ms_cond2);
    assert(0 == li_ret2);

    li_ret2 = pthread_mutex_unlock(&aps_mtx->ms_mtx2); 
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

