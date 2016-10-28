/**
 *
 */
#ifndef APL__RWLOCK_H
#define APL__RWLOCK_H

#include "apl/common.h"
#include "apl/types.h"

APL_BEGIN_C_DECLARE 

/**
 *
 */
int apl_rwlock_create(
    apl_rwlock_t* restrict   p_rwl,
);    

/**
 *
 */
int apl_rwlock_destroy(
    apl_rwlock_t* restrict   p_rwl,
);    

/**
 *
 */
int apl_rwlock_rdlock(
    apl_rwlock_t* restrict   p_rwl,
);    

/**
 *
 */
int apl_rwlock_wrlock(
    apl_rwlock_t* restrict   p_rwl,
);    

/**
 *
 */
int apl_rwlock_try_rdlock(
    apl_rwlock_t* restrict   p_rwl,
);    

/**
 *
 */
int apl_rwlock_try_wrlock(
    apl_rwlock_t* restrict   p_rwl,
);    

/**
 *
 */
int apl_rwlock_unlock(
    apl_rwlock_t* restrict   p_rwl,
);    


APL_END_C_DECLARE


#endif // APL__RWLOCK_H

