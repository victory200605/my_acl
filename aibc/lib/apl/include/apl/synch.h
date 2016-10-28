/**
 * @file synch.h
 * @author  $Author: fzz $
 * @date    $Date: 2012/08/09 10:30:31 $
 * @version $Revision: 1.17 $
 */
#ifndef APL_SYNCH_H
#define APL_SYNCH_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include <pthread.h>
#include <semaphore.h>

/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

/**
 * mutex
 */
struct apl_mutex_t
{
/** @cond NEVER */
    pthread_mutex_t     ms_mtx;
/** @endcond */
};


struct apl_cond_t
{
/** @cond NEVER */
    pthread_cond_t      ms_cond;
/** @endcond */
};


struct apl_rwlock_t
{
/** @cond NEVER */
#if defined(HAVE_PTHREAD_RWLOCK_TIMEDRDLOCK) && defined(HAVE_PTHREAD_RWLOCK_TIMEDWRLOCK)
    pthread_rwlock_t    ms_rwl;
#else
    pthread_mutex_t     ms_mtx;
    pthread_cond_t      ms_rdcond; 
    pthread_cond_t      ms_wrcond;
    apl_int_t           mi_magic;
    apl_size_t          mu_running;
    apl_size_t          mu_rdwait;
    apl_size_t          mu_wrwait;
#endif
/** @endcond */
};


struct apl_sema_t
{
/** @cond NEVER */
#if defined(HAVE_SEM_TIMEDWAIT)
    sem_t               ms_sema;
#else
    pthread_mutex_t     ms_mtx;
    pthread_cond_t      ms_cond;  
    apl_int_t           mi_magic;
    apl_int_t           mi_value;
#endif
/** @endcond */
};


struct apl_spin_t
{
/** @cond NEVER */
#if defined(HAVE_PTHREAD_SPIN_LOCK)
    pthread_spinlock_t  ms_spin;
#else
    pthread_mutex_t     ms_mtx;
    apl_int_t           mi_magic;
#endif
/** @endcond */
};


/**
 * Initialize a mutex.
 *
 * @param[in,out]  aps_mtx the mutex to be initialized
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error
 */ 
apl_int_t apl_mutex_init(
    struct apl_mutex_t* aps_mtx);


/**
 * Destory a mutex.
 *
 * @param[in,out]    aps_mtx    the mutex to be destroyed
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_mutex_destroy(
    struct apl_mutex_t* aps_mtx);


/**
 * Lock a mutex.
 *
 * @param[in,out]   aps_mtx         the mutex to be locked
 * 
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_mutex_lock(struct apl_mutex_t* aps_mtx);


/**
 * Unlock a mutex.
 *
 * @param[in,out]    aps_mtx    the mutex to be unlocked
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */  
apl_int_t apl_mutex_unlock(
    struct apl_mutex_t* aps_mtx);



/**
 * Initialize condition variables.
 *
 * @param[in,out]    aps_cond   the condition variable to be initialized
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_cond_init(
    struct apl_cond_t*  aps_cond);


/**
 * Destroy condition variables.
 *
 * @param[in,out]    aps_cond   the condition variable to be destroyed
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_cond_destroy(
    struct apl_cond_t*  aps_cond);


/**
 * Signal a condition,unblock at least one of the threads that are blocked on 
 * the specified condition variable aps_cond.
 *
 * @param[in,out]    aps_cond   the specified condition variable 
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_cond_signal(
    struct apl_cond_t*  aps_cond);


/**
 * Broadcast a condition,unblock all threads currently blocked on the 
 * specified condition variable aps_cond.
 *
 * @param[in,out]    aps_cond   the specified condition variable 
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_cond_broadcast(
    struct apl_cond_t*  aps_cond);


/**
 * Wait on a condition.
 *
 * @param[in,out]    aps_cond      the specified condition variable to be blocked
 * @param[in,out]    aps_mtx       the mutex
 * @param[in]        ai64_timeout  timeout(relatived time value)
 *                                  - <0 \n
 *                                    blocking forever.
 *                                  - >0 \n
 *                                    blocking with timeout
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_cond_wait(
    struct apl_cond_t*  aps_cond,
    struct apl_mutex_t* aps_mtx,
    apl_time_t          ai64_timeout);


/**
 * Wait on a condition, with a absoluted time value.
 *
 * @param[in,out]    aps_cond      the specified condition variable to be blocked
 * @param[in,out]    aps_mtx       the mutex
 * @param[in]        ai64_abstimeout  (absoluted time value)
 *                                  - <0 \n
 *                                    blocking forever.
 *                                  - >0 \n
 *                                    blocking with timeout
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_cond_wait_abs(
    struct apl_cond_t*  aps_cond,
    struct apl_mutex_t* aps_mtx,
    apl_time_t          ai64_abstime);


/**
 * Initialize a read-write lock object.
 *
 * @param[in,out]   aps_rwlock  the read-write lock object to be initialized
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_rwlock_init(
    struct apl_rwlock_t*    aps_rwlock);


/**
 * Destroy a read-write lock object.
 *
 * @param[in,out]   aps_rwlock  the read-write lock object to be destroyed
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_rwlock_destroy(
    struct apl_rwlock_t*    aps_rwlock);


/**
 * Lock a read-write lock object for reading.
 *
 * @param[in,out]    aps_rwlock   the read-write lock object
 * @param[in]        ai64_timeout timeout(<0:rdlock,=0:tryrdlock,>0:timedrdlock)
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */  
apl_int_t apl_rwlock_rdlock(
    struct apl_rwlock_t*    aps_rwlock,
    apl_time_t              ai64_timeout);


/**
 * Lock a read-write lock object for writing.
 *
 * @param[in,out]   aps_rwlock      the read-write lock object
 * @param[in]       ai64_timeout    timeout
 *                                  - <0 \n
 *                                    blocking forever.
 *                                  - ==0 \n
 *                                    nonblocking lock
 *                                  - >0 \n
 *                                    blocking with timeout
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_rwlock_wrlock(
    struct apl_rwlock_t*    aps_rwlock,
    apl_time_t              ai64_timeout);


/**
 * Unlock a read-write lock object.
 *
 * @param[in,out]   aps_rwlock  the read-write lock object to be unlocked
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_rwlock_unlock(
    struct apl_rwlock_t*    aps_rwlock);



/**
 * Initialize an unnamed semaphore.
 *
 * @param[in,out]   aps_sema    the unnamed semaphore to be initialized
 * @param[in]       ai_value    the value of the initialized semaphore
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_sema_init(
    struct apl_sema_t*  aps_sema,
    apl_int_t           ai_value);


/**
 * Destroy an unnamed semaphore.
 *
 * @param[in,out]   aps_sema    the unnamed semaphore to be destroyed
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_sema_destroy(
    struct apl_sema_t*  aps_sema);


/**
 * Unlock a semaphore.
 *
 * @param[in,out]    aps_sema   the semaphore to be unlocked
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */
apl_int_t apl_sema_post(
    struct apl_sema_t*  aps_sema);


/**
 * Lock a semaphore.
 *
 * @param[in,out]   aps_sema        the semaphore to be locked
 * @param[in]       ai64_timeout    timeout
 *                                  - <0 \n
 *                                    blocking forever.
 *                                  - ==0 \n
 *                                    nonblocking lock
 *                                  - >0 \n
 *                                    blocking with timeout
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_sema_wait(
    struct apl_sema_t*  aps_sema,
    apl_time_t          ai64_timeout);

/**
 * Lock a semaphore.
 *
 * @param[in,out]   aps_sema        the semaphore to be locked
 * @param[in]       ai64_abstime    absoluted timeout value 
 *                                  - <0 \n
 *                                    blocking forever.
 *                                  - ==0 \n
 *                                    nonblocking lock
 *                                  - >0 \n
 *                                    blocking with timeout
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_sema_wait_abs(
    struct apl_sema_t*  aps_sema,
    apl_time_t          ai64_abstime);


/**
 * Get the value of a semaphore.
 *
 * @param[in,out]   aps_sema    the specified semaphore
 * @param[out]      api_value   the value of the specified semaphore
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_sema_getvalue(
    struct apl_sema_t*  aps_sema,
    apl_int_t*          api_value);

/**
 * Initialize a spin lock object.
 *
 * @param[in,out]   aps_spin    the spin lock object to be initialized
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_spin_init(
    struct apl_spin_t*  aps_spin);


/**
 * Destroy a spin lock object.
 *
 * @param[in,out]   aps_spin    the spin lock object to be destroyed
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_spin_destroy(
    struct apl_spin_t*  aps_spin);


/**
 * Lock a spin lock object.
 * 
 * @param[in,out]   aps_spin    the spin lock object to be locked
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_spin_lock(
    struct apl_spin_t*  aps_spin);


/**
 * Unlock a spin lock object.
 *
 * @param[in,out]   aps_spin    the spin lock object to be unlock
 *
 * @retval 0    success.
 * @retval !=0  failed, an error number shall be returned to indicate the 
 *              error.
 */ 
apl_int_t apl_spin_unlock(
    struct apl_spin_t*  aps_spin);


/* ------------------------------ */  APL_DECLARE_END  /* ------------------------------ */

#endif /* APL_SYNCH_H */

