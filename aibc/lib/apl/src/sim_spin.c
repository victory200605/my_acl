#include "apl/synch.h"

/* ---------------------------------------------------------------------- */

apl_int_t apl_spin_init(
    struct apl_spin_t*  aps_spin)
{   
#if defined(HAVE_PTHREAD_MUTEXATTR_SETSPIN_NP)
    apl_int_t   li_ret2;
    pthread_mutexaddr_t ls_mtxattr;

    li_ret = pthread_mutexattr_init(&ls_mtxattr);
    assert(0 == li_ret);

    li_ret2 = pthread_mutexattr_setspin_np(&ls_mtxattr, PTHREAD_MUTEX_SPINONLY_NP);
    assert(0 == li_ret);
   
    li_ret2 = pthread_mutex_init(&aps_spin->ms_mtx, &ls_mtxattr);

    li_ret = pthread_mutexattr_destroy(&ls_mtxattr);
    assert(0 == li_ret);
#else
    apl_int_t   li_ret;
    li_ret = pthread_mutex_init(&aps_spin->ms_mtx, APL_NULL);

    if (0 != li_ret)
    {
        apl_set_errno(li_ret);
        return APL_INT_C(-1);
    }
#endif
    return 0;
}   

/* ---------------------------------------------------------------------- */

apl_int_t apl_spin_destroy(
    struct apl_spin_t*  aps_spin)
{   
    apl_int_t li_ret;

    li_ret = pthread_mutex_destroy(&aps_spin->ms_mtx);

    if (0 != li_ret)
    {
        apl_set_errno(li_ret);
        return APL_INT_C(-1);
    }

    return 0;
}   

/* ---------------------------------------------------------------------- */

apl_int_t apl_spin_lock(
    struct apl_spin_t*  aps_spin)
{   
    apl_int_t li_ret;
    
    li_ret = pthread_mutex_lock(&aps_spin->ms_mtx);

    if (0 != li_ret)
    {
        apl_set_errno(li_ret);
        return APL_INT_C(-1);
    }

    return 0;
}   

/* ---------------------------------------------------------------------- */

apl_int_t apl_spin_unlock(
    struct apl_spin_t*  aps_spin)
{   
    apl_int_t li_ret;
    
    li_ret = pthread_mutex_unlock(&aps_spin->ms_mtx);

    if (0 != li_ret)
    {
        apl_set_errno(li_ret);
        return APL_INT_C(-1);
    }

    return 0;
}   

/* ---------------------------------------------------------------------- */

