#include "apl/synch.h"
#include "apl/time.h"

/* ---------------------------------------------------------------------- */

apl_int_t apl_mutex_init(
    struct apl_mutex_t*     aps_mtx)
{
    apl_int_t li_ret;
   
    li_ret = (apl_int_t) pthread_mutex_init(&(aps_mtx->ms_mtx), NULL);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mutex_destroy(
    struct apl_mutex_t*     aps_mtx)
{
    apl_int_t li_ret;
    
    li_ret = (apl_int_t) pthread_mutex_destroy(&(aps_mtx->ms_mtx));

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mutex_lock(struct apl_mutex_t*     aps_mtx)
{
    int li_ret;

    li_ret = (apl_int_t) pthread_mutex_lock(&(aps_mtx->ms_mtx));

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mutex_unlock(
    struct apl_mutex_t* aps_mtx)
{
    apl_int_t li_ret;
    
    li_ret = (apl_int_t) pthread_mutex_unlock(&(aps_mtx->ms_mtx));

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

