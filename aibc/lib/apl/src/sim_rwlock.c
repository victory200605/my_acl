#include "apl/synch.h"
#include "apl/time.h"
#include <assert.h>

/* ---------------------------------------------------------------------- */
#define APL_RWLOCK_MAGIC        APL_INT_C(0x4f8e7d6c)

/* ---------------------------------------------------------------------- */

static inline
apl_int_t sim_rwlock_rdlock(
    struct apl_rwlock_t*    aps_rwl)
{
    apl_int_t   li_ret = 0;
    apl_int_t   li_ret2;

    assert(APL_RWLOCK_MAGIC == aps_rwl->mi_magic);

    li_ret2 = pthread_mutex_lock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    ++(aps_rwl->mu_rdwait);

    while (APL_UINT_MAX == aps_rwl->mu_running  /* a writer is running */
        || aps_rwl->mu_wrwait > APL_UINT_C(0))  /* writers are waitting */
    { 
        li_ret2 = pthread_cond_wait(&aps_rwl->ms_rdcond, &aps_rwl->ms_mtx);
        assert(0 == li_ret2);
    }

    if (APL_UINT_MAX - 1 == aps_rwl->mu_running)
    {
        li_ret = APL_EAGAIN;
    }
    else
    {
        ++(aps_rwl->mu_running);
        li_ret = 0;
    }

    --(aps_rwl->mu_rdwait);

    li_ret2 = pthread_mutex_unlock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

static inline
apl_int_t sim_rwlock_tryrdlock(
    struct apl_rwlock_t*    aps_rwl)
{   
    apl_int_t   li_ret;
    apl_int_t   li_ret2;

    assert(APL_RWLOCK_MAGIC == aps_rwl->mi_magic);

    li_ret2 = pthread_mutex_lock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    if ((APL_UINT_MAX == aps_rwl->mu_running /* a writer is running */
        || aps_rwl->mu_wrwait > APL_UINT_C(0))) /* writers are waitting */
    { 
        li_ret = APL_EBUSY;
    }
    else if (APL_UINT_MAX - 1 == aps_rwl->mu_running)
    {
        li_ret = APL_EAGAIN;
    }
    else
    {
        ++(aps_rwl->mu_running);
        li_ret = 0;
    }

    li_ret2 = pthread_mutex_unlock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

static inline
apl_int_t sim_rwlock_timedrdlock(
    struct apl_rwlock_t*    aps_rwl,
    struct timespec*        aps_timeout)
{
    apl_int_t   li_ret;
    apl_int_t   li_ret2;

    assert(APL_RWLOCK_MAGIC == aps_rwl->mi_magic);

    li_ret2 = pthread_mutex_lock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    ++(aps_rwl->mu_rdwait);

    li_ret = 0;

    while (APL_ETIMEDOUT != li_ret                  /* not timedout */
        &&(APL_UINT_MAX == aps_rwl->mu_running      /* a writer is running */
        || aps_rwl->mu_wrwait > APL_UINT_C(0))) /* writers are waitting */
    { 
        li_ret = pthread_cond_timedwait(&aps_rwl->ms_rdcond, &aps_rwl->ms_mtx, aps_timeout);
        assert(APL_ETIMEDOUT == li_ret || 0 == li_ret);
    }

    if (APL_UINT_MAX == aps_rwl->mu_running || aps_rwl->mu_wrwait > APL_UINT_C(0))
    {
        li_ret = APL_ETIMEDOUT;
    }
    else if (APL_UINT_MAX - 1 == aps_rwl->mu_running)
    {
        li_ret = APL_EAGAIN;
    }
    else
    {
        ++(aps_rwl->mu_running);
        li_ret = 0;
    }

    --(aps_rwl->mu_rdwait);

    li_ret2 = pthread_mutex_unlock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

static inline
apl_int_t sim_rwlock_wrlock(
    struct apl_rwlock_t*    aps_rwl)
{
    apl_int_t   li_ret;
    apl_int_t   li_ret2;

    assert(APL_RWLOCK_MAGIC == aps_rwl->mi_magic);

    li_ret2 = pthread_mutex_lock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    ++(aps_rwl->mu_wrwait);

    while (aps_rwl->mu_running > APL_UINT_C(0)) /* writer or readers are running */
    { 
        li_ret2 = pthread_cond_wait(&aps_rwl->ms_wrcond, &aps_rwl->ms_mtx);
        assert(0 == li_ret2);
    }

    aps_rwl->mu_running = APL_UINT_MAX;
    li_ret = APL_UINT_C(0);

    --(aps_rwl->mu_wrwait);

    li_ret2 = pthread_mutex_unlock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

static inline
apl_int_t sim_rwlock_trywrlock(
    struct apl_rwlock_t*    aps_rwl)
{
    apl_int_t   li_ret;
    apl_int_t   li_ret2;

    assert(APL_RWLOCK_MAGIC == aps_rwl->mi_magic);

    li_ret2 = pthread_mutex_lock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    if (aps_rwl->mu_running > APL_UINT_C(0)) /* writer or readers are running */
    { 
        li_ret = APL_EBUSY;
    }
    else
    {
        aps_rwl->mu_running = APL_UINT_MAX;
        li_ret = APL_UINT_C(0);
    }

    li_ret2 = pthread_mutex_unlock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

static inline
apl_int_t sim_rwlock_timedwrlock(
    struct apl_rwlock_t*    aps_rwl,
    struct timespec*        aps_timeout)
{
    apl_int_t   li_ret;
    apl_int_t   li_ret2;

    assert(APL_RWLOCK_MAGIC == aps_rwl->mi_magic);

    li_ret2 = pthread_mutex_lock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    ++(aps_rwl->mu_wrwait);

    li_ret = 0;

    while (APL_ETIMEDOUT != li_ret               /* not timedout */
        && aps_rwl->mu_running > APL_UINT_C(0)) /* writer or readers are running */
    { 
        li_ret = pthread_cond_timedwait(&aps_rwl->ms_wrcond, &aps_rwl->ms_mtx, aps_timeout);
        assert(APL_ETIMEDOUT == li_ret || 0 == li_ret);
    }

    if (APL_UINT_C(0) == aps_rwl->mu_running)
    {
        aps_rwl->mu_running = APL_UINT_MAX;
        li_ret = APL_UINT_C(0);
    }
    else
    {
        li_ret = APL_ETIMEDOUT;
    }

    --(aps_rwl->mu_wrwait);

    li_ret2 = pthread_mutex_unlock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_rwlock_init(
    struct apl_rwlock_t*    aps_rwl)
{
    apl_int_t   li_ret;

    li_ret = pthread_mutex_init(&aps_rwl->ms_mtx, APL_NULL);
    assert(0 == li_ret);

    li_ret = pthread_cond_init(&aps_rwl->ms_rdcond, APL_NULL);
    assert(0 == li_ret);

    li_ret = pthread_cond_init(&aps_rwl->ms_wrcond, APL_NULL);
    assert(0 == li_ret);

    aps_rwl->mi_magic   = APL_RWLOCK_MAGIC;
    aps_rwl->mu_running = APL_UINT_C(0);
    aps_rwl->mu_rdwait  = APL_UINT_C(0);
    aps_rwl->mu_wrwait  = APL_UINT_C(0);

    return 0;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_rwlock_destroy(
    struct apl_rwlock_t*    aps_rwl)
{
    apl_int_t   li_ret;

    assert(APL_RWLOCK_MAGIC == aps_rwl->mi_magic);

    li_ret = pthread_mutex_destroy(&aps_rwl->ms_mtx);
    assert(0 == li_ret);

    li_ret = pthread_cond_destroy(&aps_rwl->ms_rdcond);
    assert(0 == li_ret);

    li_ret = pthread_cond_destroy(&aps_rwl->ms_wrcond);
    assert(0 == li_ret);

    aps_rwl->mi_magic   = 0;

    return 0;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_rwlock_rdlock(
    struct apl_rwlock_t*    aps_rwl,
    apl_time_t              ai64_timeout)
{
    apl_int_t   li_ret;
    
    assert(APL_RWLOCK_MAGIC == aps_rwl->mi_magic);

    if (ai64_timeout < 0)
    {
        li_ret = sim_rwlock_rdlock(aps_rwl);
    }
    else if (0 == ai64_timeout)
    {
        li_ret = sim_rwlock_tryrdlock(aps_rwl);
    }
    else
    {
        struct timespec ls_ts;

        li_ret = clock_gettime(CLOCK_REALTIME, &ls_ts);
        assert(0 == li_ret);

        APL_TIME_TO_TIMESPEC(&ls_ts, APL_TIME_FROM_TIMESPEC(&ls_ts) + ai64_timeout);

        li_ret = sim_rwlock_timedrdlock(aps_rwl, &ls_ts);
    }

    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_rwlock_wrlock(
    struct apl_rwlock_t*    aps_rwl,
    apl_time_t              ai64_timeout)
{
    apl_int_t   li_ret;
    
    assert(APL_RWLOCK_MAGIC == aps_rwl->mi_magic);

    if (ai64_timeout < 0)
    {
        li_ret = sim_rwlock_wrlock(aps_rwl);
    }
    else if (0 == ai64_timeout)
    {
        li_ret = sim_rwlock_trywrlock(aps_rwl);
    }
    else
    {
        struct timespec ls_ts;

        li_ret = clock_gettime(CLOCK_REALTIME, &ls_ts);
        assert(0 == li_ret);

        APL_TIME_TO_TIMESPEC(&ls_ts, APL_TIME_FROM_TIMESPEC(&ls_ts) + ai64_timeout);

        li_ret = sim_rwlock_timedwrlock(aps_rwl, &ls_ts);
    }

    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_rwlock_unlock(
    struct apl_rwlock_t*    aps_rwl)
{
    apl_int_t   li_ret;
    apl_int_t   li_ret2;

    assert(APL_RWLOCK_MAGIC == aps_rwl->mi_magic);

    li_ret2 = pthread_mutex_lock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    if (APL_UINT_C(0) == aps_rwl->mu_running)
    {
        li_ret = APL_EPERM;
    }
    else /* need unlock */
    { 
        if (APL_UINT_MAX == aps_rwl->mu_running)
        { /* writer unlock */
            aps_rwl->mu_running = APL_UINT_C(0);
        }
        else
        { /* reader unlock */
            --(aps_rwl->mu_running);
        }

        if (APL_UINT_C(0) == aps_rwl->mu_running)
        {
            if (aps_rwl->mu_wrwait > APL_UINT_C(0)) /* writer first */
            {
                li_ret2 = pthread_cond_signal(&aps_rwl->ms_wrcond);
                assert(0 == li_ret2);
            }
            else if (aps_rwl->mu_rdwait > APL_UINT_C(0))
            { 
                /* broadcast, not just signal :) */
                li_ret2 = pthread_cond_broadcast(&aps_rwl->ms_rdcond);
                assert(0 == li_ret2);
            }
        }

        li_ret = 0;
    }

    li_ret2 = pthread_mutex_unlock(&aps_rwl->ms_mtx);
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

