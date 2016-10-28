 /**
  * @file proc.h
  * @author  $Author: hezk $
  * @date    $Date: 2009/08/18 09:31:11 $
  * @version $Revision: 1.15 $
  */
#ifndef APL_SYS_H
#define APL_SYS_H

#include "aibc_config.h"
#include <assert.h>


#if defined(__cplusplus)
#   define APL_DECLARE_BEGIN    extern "C" {
#   define APL_DECLARE_END      }
#else
#   define APL_DECLARE_BEGIN    
#   define APL_DECLARE_END
#endif


/** 
 * Null value for pointers
 */
#define APL_NULL    (0)


/** 
 * Calculate the offfset of member in struct type
 *
 * @param type The struct type
 * @param member The member in struct type
 *
 * @return The offset of member in struct type, in bytes
 */
#define APL_OFFSETOF(type, member)  (((apl_int_t)&(((type*)sizeof(void*))->member)) - sizeof(void*))

/**
 * This macro shall calculate the number of elements that the array can hold.
 * 
 * It can only be used with static array.
 *
 * @param[in]    array     the array wanted to calculate
 *
 * @return    the number of elements that the array can hold
 */
#define APL_SIZEOFARRAY(array)      (sizeof(array)/sizeof((array)[0]))


#define APL_RETURN_IF(ret, cond) \
    do { \
        if (cond) \
        { \
            return ret; \
        } \
    } while(0)


#define APL_ERRNO_RETURN_IF(err, cond) \
    do { \
        if (cond) \
        { \
            apl_set_errno(err); \
            return APL_INT_C(-1); \
        } \
    } while(0)


#define APL_ERRNO_RETURN(err) \
    do { \
        if (0 != (err)) \
        { \
            apl_set_errno(err); \
            return APL_INT_C(-1); \
        } \
        return 0; \
    } while(0)


#endif /* APL_SYS_H */

