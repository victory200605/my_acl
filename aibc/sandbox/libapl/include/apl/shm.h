/**
 *
 */
#ifndef APL__SHM_H
#define APL__SHM_H

#include "apl/common.h"

APL_BEGIN_C_DECLARE

/**
 *
 */
int apl_shm_open(
    apl_shm_t* restrict     p_shm,
    char const*             p_name,
    int                     oflag,
    mode_t                  mode
);


/**
 *
 */
int apl_shm_close(
    apl_shm_t*  p_shm
);

/**
 *
 */
int apl_shm_truncate(
    apl_shm_t*  p_shm,
    off_t       len
);

/**
 *
 */
int apl_shm_unlink(
    char const* p_name
);


APL_END_C_DECLARE

#endif // APL__SHM_H

