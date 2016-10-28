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
int apl_nsem_open(
    apl_nsem_t* restrict    p_nsem,
    char const* restrict    p_name,
    int                     oflag,
    mode_t                  mode
);

/**
 *
 */
int apl_nsem_close(
    apl_nsem_t*  p_nsem
);

/**
 *
 */
int apl_nsem_unlink(
    char const*     p_name,
);

/**
 *
 */
int apl_nsem_get_value(
    apl_nsem_t* restrict    p_nsem,
    int* restrict           p_value
);

/**
 *
 */
int apl_nsem_post(
    apl_nsem_t*  p_nsem
);

/**
 *
 */
int apl_nsem_wait(
    apl_nsem_t*  p_nsem
);

/**
 *
 */
int apl_nsem_try_wait(
    apl_nsem_t*  p_nsem
);

/**
 *
 */
int apl_nsem_timed_wait(
    apl_nsem_t*     p_nsem,
    apl_time_t      abstime
);

APL_END_C_DECLARE


#endif // APL__SEM_H

