/**
 *
 */
#ifndef APL__THREAD_H
#define APL__THREAD_H

#include "apl/common.h"
#include "apl/types.h"

APL_BEGIN_C_DECLARE 

/**
 *
 */
int apl_thread_create(
    apl_thread_t* restrict      p_thr,
    apl_thread_func restrict    p_func,
    size_t                      ssz,
    size_t                      gsz,
);


APL_END_C_DECLARE


#endif // APL__THREAD_H

