/**
 * @file thread.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.9 $
 */ 
#ifndef APL_THREAD_H
#define APL_THREAD_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"

/* -------------------- */ APL_DECLARE_BEGIN /* -------------------- */


/**
 * a flag that create a kernel thread
 */
#define APL_THREAD_KERNEL   APL_UINT_C(0)

/**
 * a flag that create a user thread
 */
#define APL_THREAD_USER     APL_UINT_C(1)


/** the variable used to store thread ID. */
typedef apl_uint_t  apl_thread_t;


typedef void* (*apl_thr_func_ptr)(void*);


/**
 * Create thread.
 *
 * @param[out]  api_thr_id      the new thread id
 * @param[in]   apf_thr_func    the thread start routine
 * @param[in]   apv_arg         the start routine's sole argument
 * @param[in]   au_stacksize    the stack size
 * @param[in]   au_flags        the type of scheduling contention scope
 *
 * @retval 0    success.
 * @retval -1   failed, an error number shall be returned to indicate the error
 */ 
apl_int_t apl_thread_create(
    apl_thread_t*       api_thr_id,
    apl_thr_func_ptr    apf_thr_func,
    void*               apv_arg,
    apl_size_t          au_stacksize,
    apl_uint_t          au_flags);


/**
 * Yield the processor.
 *
 * @retval 0    success.
 * @retval -1   failed, and set errno to indicate the error
 */ 
apl_int_t apl_thread_yield(void);


/**
 * Terminate the calling thread.
 *
 * @param[in]   apv_ret the value to be made available to any successful join 
 *              with the terminating thread
 */ 
void apl_thread_exit(void* apv_ret);


/**
 * Get the calling thread ID.
 *
 * @return Return the thread ID of the calling thread.
 */ 
apl_thread_t apl_thread_self(void);


/* -------------------- */  APL_DECLARE_END  /* -------------------- */

#endif /* APL_THREAD_H */

