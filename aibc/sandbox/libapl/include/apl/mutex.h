/**
 *
 */
#ifndef APL__MUTEX_H
#define APL__MUTEX_H

#include "apl/common.h"

APL_BEGIN_C_DECLARE 

/**
 *
 */
int apl_mutex_create(
    apl_mutex_t* restrict   p_mtx,
    bool                    pshr
);

/**
 *
 */
int apl_mutex_destroy(
    apl_mutex_t* restrict   p_mtx,
);    

/**
 *
 */
int apl_mutex_lock(
    apl_mutex_t* restrict   p_mtx,
);    

/**
 *
 */
int apl_mutex_try_lock(
    apl_mutex_t* restrict   p_mtx,
);    

/**
 *
 */
int apl_mutex_unlock(
    apl_mutex_t* restrict   p_mtx,
);    


APL_END_C_DECLARE


#endif // APL__MUTEX_H

