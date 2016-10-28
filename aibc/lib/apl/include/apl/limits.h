/**
 * @file limits.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.12 $
 */
#ifndef APL_LIMITS_H
#define APL_LIMITS_H

#include "aibc_config.h"
#include <limits.h>
#include <unistd.h>

/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

#if defined(NAME_MAX)
#   define APL_NAME_MAX         (NAME_MAX)
#else
#   define APL_NAME_MAX         APL_UINT_C(1024)
#endif
/**
 * Maximum number of bytes in a filename 
 *
 * @def APL_NAME_MAX
 */


#if defined(PATH_MAX)
#   define APL_PATH_MAX         (PATH_MAX)
#else
#   define APL_PATH_MAX         APL_UINT_C(1024)
#endif

/**
 * Maximum number of bytes in a pathname 
 *
 * @def APL_PATH_MAX
 */


#if defined(HOST_NAME_MAX)
#   define APL_HOST_NAME_MAX    (HOST_NAME_MAX)
#elif defined(_POSIX_HOST_NAME_MAX)
#   define APL_HOST_NAME_MAX    (_POSIX_HOST_NAME_MAX)
#else
#   define APL_HOST_NAME_MAX    1024   
#endif
/**
 * Maximum length of a host name 
 *
 * @def APL_HOST_NAME_MAX
 */


#if defined(SEM_VALUE_MAX)
#   define APL_SEM_VALUE_MAX    (SEM_VALUE_MAX)
#else
#   define APL_SEM_VALUE_MAX    (APL_INT_MAX)
#endif
/**
 * The maximum value a semaphore may have. 
 *
 * @def APL_SEM_VALUE_MAX
 */



/**
 * Maximum number of apl_iovec_t structures that one process has available 
 * for use with apl_readv() or apl_writev(). 
 */
#define APL_IOV_MAX             (IOV_MAX)


/**
 * Size in bytes of a page. 
 */
#define APL_PAGE_SIZE           (sysconf(_SC_PAGESIZE))


/**
 * Minimum size in bytes of thread stack storage. 
 */
#define APL_THREAD_STACK_MIN    (PTHREAD_STACK_MIN*2)


/**
 * Maximum number of files that one process can have open at any one time. 
 */
#define APL_OPEN_MAX            (sysconf(_SC_OPEN_MAX))

/**
 * Maximum length of argument to the exec functions including environment 
 * data.
 */
#define APL_ARG_MAX             (sysconf(_SC_ARG_MAX))

/**
 * Maximum number of links to a single file. 
 */
#define APL_LINE_MAX            (LINE_MAX)

/**
 * Maximum number of simultaneous processes per real user ID.
 */
#define APL_CHILD_MAX           (sysconf(_SC_CHILD_MAX))

/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_LIMITS_H */

