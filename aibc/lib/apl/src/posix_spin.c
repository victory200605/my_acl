#include "apl/synch.h"

/* ---------------------------------------------------------------------- */

apl_int_t apl_spin_init(
    struct apl_spin_t*  aps_spin)
{
    apl_int_t li_ret;
   
    li_ret = (apl_int_t) pthread_spin_init(&(aps_spin->ms_spin), 0); 

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_spin_destroy(
    struct apl_spin_t*  aps_spin)
{
    apl_int_t li_ret;
    
    li_ret = (apl_int_t) pthread_spin_destroy(&(aps_spin->ms_spin));

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_spin_lock(
    struct apl_spin_t*  aps_spin)
{
    apl_int_t li_ret;
    
    li_ret = (apl_int_t) pthread_spin_lock(&(aps_spin->ms_spin));

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_spin_unlock(
    struct apl_spin_t*  aps_spin)
{
    apl_int_t li_ret;
    
    li_ret = (apl_int_t) pthread_spin_unlock(&(aps_spin->ms_spin));

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

