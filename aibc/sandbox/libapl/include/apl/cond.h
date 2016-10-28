/**
 *
 */
#ifndef APL__COND_H
#define APL__COND_H

#include "apl/common.h"
#include "apl/types.h"

APL_BEGIN_C_DECLARE 

/**
 *
 */
int apl_cond_create(
    apl_cond_t* restrict    p_cond,
    bool                    pshr
);

/**
 *
 */
int apl_cond_destroy(
    apl_cond_t* restrict   p_cond,
);    

/**
 *
 */
int apl_cond_wait(
    apl_cond_t* restrict    p_cond,
    apl_mutex_t* restrict   p_mtx
);    

/**
 *
 */
int apl_cond_signal(
    apl_cond_t* restrict   p_cond,
);    

/**
 *
 */
int apl_cond_broadcast(
    apl_cond_t* restrict   p_cond,
);    


APL_END_C_DECLARE


#endif // APL__COND_H

