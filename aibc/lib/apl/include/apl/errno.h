/**
 * @file errno.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.8 $
 */
#ifndef APL_ERRNO_H
#define APL_ERRNO_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include <errno.h>


/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

#define APL_H_ERRNO_MASK        APL_INT_C(0xF0000000)
#define APL_GAI_ERRNO_MASK      APL_INT_C(0x0F000000)

/** Argument list too long. */
#define APL_E2BIG               ((apl_int_t)E2BIG)

/** Permission denied. */
#define APL_EACCES              ((apl_int_t)EACCES)

/** Address not available. */
#define APL_EADDRINUSE          ((apl_int_t)EADDRINUSE)

/** Address not available. */
#define APL_EADDRNOTAVAIL       ((apl_int_t)EADDRNOTAVAIL)

/** Address family not supported. */
#define APL_EAFNOSUPPORT        ((apl_int_t)EAFNOSUPPORT)

/** Resource unavailable, try again (may be the same value as [APL_EWOULDBLOCK]). */
#define APL_EAGAIN              ((apl_int_t)EAGAIN)

/** Connection already in progress. */
#define APL_EALREADY            ((apl_int_t)EALREADY)

/** Bad file descriptor. */
#define APL_EBADF               ((apl_int_t)EBADF)

/** Bad message. */
#define APL_EBADMSG             ((apl_int_t)EBADMSG)

/** Device or resource busy. */
#define APL_EBUSY               ((apl_int_t)EBUSY)

/** Operation canceled. */
#define APL_ECANCELED           ((apl_int_t)ECANCELED)

/** No child processes. */
#define APL_ECHILD              ((apl_int_t)ECHILD)

/** Connection aborted. */
#define APL_ECONNABORTED        ((apl_int_t)ECONNABORTED)

/** Connection refused. */
#define APL_ECONNREFUSED        ((apl_int_t)ECONNREFUSED)

/** Connection reset. */
#define APL_ECONNRESET          ((apl_int_t)ECONNRESET)

/** Resource deadlock would occur. */
#define APL_EDEADLK             ((apl_int_t)EDEADLK)

/** Destination address required. */
#define APL_EDESTADDRREQ        ((apl_int_t)EDESTADDRREQ)

/** Mathematics argument out of domain of function. */
#define APL_EDOM                ((apl_int_t)EDOM)

/** Reserved. */
#define APL_EDQUOT              ((apl_int_t)EDQUOT)

/** File exists. */
#define APL_EEXIST              ((apl_int_t)EEXIST)

/** Bad address. */
#define APL_EFAULT              ((apl_int_t)EFAULT)

/** File too large. */
#define APL_EFBIG               ((apl_int_t)EFBIG)

/** Host is unreachable. */
#define APL_EHOSTUNREACH        ((apl_int_t)EHOSTUNREACH)

/** Identifier removed. */
#define APL_EIDRM               ((apl_int_t)EIDRM)

/** Illegal byte sequence. */
#define APL_EILSEQ              ((apl_int_t)EILSEQ)

/** Operation in progress. */
#define APL_EINPROGRESS         ((apl_int_t)EINPROGRESS)

/** Interrupted function. */
#define APL_EINTR               ((apl_int_t)EINTR)

/** Invalid argument. */
#define APL_EINVAL              ((apl_int_t)EINVAL)

/** I/O error. */
#define APL_EIO                 ((apl_int_t)EIO)

/** Socket is connected. */
#define APL_EISCONN             ((apl_int_t)EISCONN)

/** Is a directory. */
#define APL_EISDIR              ((apl_int_t)EISDIR)

/** Too many levels of symbolic links. */
#define APL_ELOOP               ((apl_int_t)ELOOP)

/** Too many open files. */
#define APL_EMFILE              ((apl_int_t)EMFILE)

/** Too many links. */
#define APL_EMLINK              ((apl_int_t)EMLINK)

/** Message too large. */
#define APL_EMSGSIZE            ((apl_int_t)EMSGSIZE)

/** Reserved. */
#define APL_EMULTIHOP           ((apl_int_t)EMULTIHOP)

/** Filename too long. */
#define APL_ENAMETOOLONG        ((apl_int_t)ENAMETOOLONG)

/** Network is down. */
#define APL_ENETDOWN            ((apl_int_t)ENETDOWN)

/** Connection aborted by network. */
#define APL_ENETRESET           ((apl_int_t)ENETRESET)

/** Network unreachable. */
#define APL_ENETUNREACH         ((apl_int_t)ENETUNREACH)

/** Too many files open in system. */
#define APL_ENFILE              ((apl_int_t)ENFILE)

/** No buffer space available. */
#define APL_ENOBUFS             ((apl_int_t)ENOBUFS)

/** No message is available on the STREAM head read queue. */
#define APL_ENODATA             ((apl_int_t)ENODATA)

/** No such device. */
#define APL_ENODEV              ((apl_int_t)ENODEV)

/** No such file or directory. */
#define APL_ENOENT              ((apl_int_t)ENOENT)

/** Executable file format error. */
#define APL_ENOEXEC             ((apl_int_t)ENOEXEC)

/** No locks available. */
#define APL_ENOLCK              ((apl_int_t)ENOLCK)

/** Reserved. */
#define APL_ENOLINK             ((apl_int_t)ENOLINK)

/** Not enough space. */
#define APL_ENOMEM              ((apl_int_t)ENOMEM)

/** No message of the desired type. */
#define APL_ENOMSG              ((apl_int_t)ENOMSG)

/** Protocol not available. */
#define APL_ENOPROTOOPT         ((apl_int_t)ENOPROTOOPT)

/** No space left on device. */
#define APL_ENOSPC              ((apl_int_t)ENOSPC)

/** No STREAM resources. */
#define APL_ENOSR               ((apl_int_t)ENOSR)

/** Not a STREAM. */
#define APL_ENOSTR              ((apl_int_t)ENOSTR)

/** Function not supported. */
#define APL_ENOSYS              ((apl_int_t)ENOSYS)

/** The socket is not connected. */
#define APL_ENOTCONN            ((apl_int_t)ENOTCONN)

/** Not a directory. */
#define APL_ENOTDIR             ((apl_int_t)ENOTDIR)

/** Directory not empty. */
#define APL_ENOTEMPTY           ((apl_int_t)ENOTEMPTY)

/** Not a socket. */
#define APL_ENOTSOCK            ((apl_int_t)ENOTSOCK)

/** Not supported. */
#define APL_ENOTSUP             ((apl_int_t)ENOTSUP)

/** Inappropriate I/O control operation. */
#define APL_ENOTTY              ((apl_int_t)ENOTTY)

/** No such device or address. */
#define APL_ENXIO               ((apl_int_t)ENXIO)

/** Operation not supported on socket. */
#define APL_EOPNOTSUPP          ((apl_int_t)EOPNOTSUPP)

/** Value too large to be stored in data type. */
#define APL_EOVERFLOW           ((apl_int_t)EOVERFLOW)

/** Operation not permitted. */
#define APL_EPERM               ((apl_int_t)EPERM)

/** Broken pipe. */
#define APL_EPIPE               ((apl_int_t)EPIPE)

/** Protocol error. */
#define APL_EPROTO              ((apl_int_t)EPROTO)

/** Protocol not supported. */
#define APL_EPROTONOSUPPORT     ((apl_int_t)EPROTONOSUPPORT)

/** Protocol wrong type for socket. */
#define APL_EPROTOTYPE          ((apl_int_t)EPROTOTYPE)

/** Result too large. */
#define APL_ERANGE              ((apl_int_t)ERANGE)

/** Read-only file system. */
#define APL_EROFS               ((apl_int_t)EROFS)

/** Invalid seek. */
#define APL_ESPIPE              ((apl_int_t)ESPIPE)

/** No such process. */
#define APL_ESRCH               ((apl_int_t)ESRCH)

/** Reserved. */
#define APL_ESTALE              ((apl_int_t)ESTALE)

/** Stream ioctl() timeout. */
#define APL_ETIME               ((apl_int_t)ETIME)

/** Connection timed out.  */
#define APL_ETIMEDOUT           ((apl_int_t)ETIMEDOUT)

/** Text file busy. */
#define APL_ETXTBSY             ((apl_int_t)ETXTBSY)

/** Operation would block (may be the same value as APL_EAGAIN). */
#define APL_EWOULDBLOCK         ((apl_int_t)EWOULDBLOCK)

/** Cross-device link. */
#define APL_EXDEV               ((apl_int_t)EXDEV)



/** 
 * Get error return value.
 *
 * @return    the value of errno.
 */
apl_int_t apl_get_errno(void);

/** 
 * Set error return value.
 *
 * @param[in]   ai_errno    the value that wanted to set to errno.
 */
void apl_set_errno(
    apl_int_t ai_errno);

/** 
 * Get error message string.
 *
 * This function shall map the error number in ai_errno to a locale-dependent
 * error message string and shall return a pointer to it.
 *
 * @param[in]   ai_errno    Error number.
 *
 * @return  A pointer to the generated message string.
 */
char const* apl_strerror(
    apl_int_t ai_errno);

/** 
 * Write error messages to standard error.
 *
 * This function shall map the error number accessed through the symbol errno 
 * to a language-dependent error message, which shall be written to the 
 * standard error stream as follows:
 *  - First, the string pointed to by apc_fmt followed by a colon and a space.
 *  - Then an error message string followed by a newline.
 *
 * The contents of the error message strings shall be the same as those 
 * returned by strerror() with argument errno.
 *
 * @param[in] apc_msg   the string point which point to the string will write 
 *                      to the standard error stream when error happen.
 */
void apl_perror(
    char const* apc_msg);

/** 
 * Print formatted output to standard error.
 *
 * This fuction print a string pointed by apc_fmt to a standard error output.
 *
 * @param[in]   apc_fmt     A string that wants to print.
 *
 * @retval >=0  success, the value is the number of bytes transmitted.
 * @retval <0   failure.
 */
apl_ssize_t apl_errprintf(
    char const* apc_fmt,
    ...);

/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */


#endif /* APL_ERRNO_H */

