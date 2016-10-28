/**
 * @file types.h
 * @author  $Author: daizh $
 * @date    $Date: 2010/12/29 10:45:21 $
 * @version $Revision: 1.6 $
 */
#ifndef APL_TYPES_H
#define APL_TYPES_H

#include "aibc_config.h"
#include "apl/inttypes.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stddef.h>

/** this variable uses to store file decriptor. */
typedef apl_int_t       apl_handle_t;

/** this variable uses to store the length of memory block. */
typedef apl_uint_t      apl_size_t;

/** this variable uses to store the length of memory block, if this variable is nevagive,it indicate error accour. */
typedef apl_int_t       apl_ssize_t;

typedef apl_int_t       apl_ptrdiff_t;

/** this variable uses to store the time value. */
typedef apl_int64_t     apl_time_t;

/** this variable uses to soire the length of a file or the offset of a file. */
typedef apl_int64_t     apl_off_t;

#if defined(APL_OS_HPUX)
typedef int             apl_socklen_t;
#else
typedef socklen_t       apl_socklen_t;
#endif

#endif /* APL_TYPES_H */

