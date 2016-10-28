/**
 *
 */
#ifndef APL__SEM_H
#define APL__SEM_H

#include "apl/common.h"
#include "apl/types.h"

APL_BEGIN_C_DECLARE 

/**
 *
 */
int apl_sem_init(
    apl_sem_t*  p_sem,
    bool        pshr,
    unsigned    value
);

/**
 *
 */
int apl_sem_destroy(
    apl_sem_t*  p_sem,
);

/**
 *
 */
int apl_sem_get_value(
    apl_sem_t* restrict     p_sem,
    int* restrict           p_value
);

/**
 *
 */
int apl_sem_post(
    apl_sem_t*  p_sem
);

/**
 *
 */
int apl_sem_wait(
    apl_sem_t*  p_sem
);

/**
 *
 */
int apl_sem_try_wait(
    apl_sem_t*  p_sem
);

/**
 *
 */
int apl_sem_timed_wait(
    apl_sem_t*  p_sem,
    apl_time_t  abstime 
);


APL_END_C_DECLARE


#endif // APL__SEM_H

