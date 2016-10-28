/**
 * @file heap.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.5 $
 */
#ifndef APL_HEAP_H
#define APL_HEAP_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include <stdlib.h>


/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

/** 
 * A memory allocator.
 *
 * This function shall allocate unused space for an object whose size in 
 * bytes is specified by au_size.
 *
 * @param[in]    au_size    indicate the allocate unused space size.
 *
 * @retval !=APL_NULL   successful,the pointer points to the beginning of the 
 *                      allocated space.
 * @retval APL_NULL     fail, errno indicate the error.
 */
void* apl_malloc(
    apl_size_t  au_size);

/** 
 * Memory reallocator.
 *
 * This function shall change the size of the memory object pointed to by 
 * apv_ptr to the size specified by au_size. The contents of the object shall 
 * remain unchanged up to the lesser of the new and old sizes. If au_size is 
 * 0 and apv_ptr is not a null pointer, the object pointed to is freed. If 
 * the space cannot be allocated, the object shall remain unchanged.
 *
 * @param[in]    apv_ptr    a pointer points to the object which need to 
 *                          realloc.
 * @param[in]    au_size    new size of the object.
 *
 * @retval !=APL_NULL   successful,the pointer points to the beginning of the 
 *                      allocated space.
 * @retval APL_NULL     fail, errno indicate the error.
 */
void* apl_realloc(
    void*       apv_ptr, 
    apl_size_t  au_size) ;

/** 
 * Free allocated memory.
 *
 * This function shall cause the space pointed to by apv_ptr to be 
 * deallocated; that is, made available for further allocation.
 *
 * @param[in]   apv_ptr     A pointer points to the space that would be 
 *                          deallovated.
 */
void apl_free(void* apv_ptr);


/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_HEAP_H */

