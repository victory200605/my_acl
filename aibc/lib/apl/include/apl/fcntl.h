#ifndef APL_FCNTL_H
#define APL_FCNTL_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"

/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

/** 
 * Open for reading only. 
 */
#define APL_O_RDONLY    (O_RDONLY)

/** 
 * Open for writing only. 
 */
#define APL_O_WRONLY    (O_WRONLY)

/**
 * Open for reading and writing. 
 */
#define APL_O_RDWR      (O_RDWR)

/** 
 * Set append mode. 
 *
 * If set, the file offset shall be set to the end of the file prior to each 
 * write.
 */
#define APL_O_APPEND    (O_APPEND)

/** 
 * Create file if it does not exist. 
 *
 * If the file exists, this flag has no effect except as noted under 
 * APL_O_EXCL below. Otherwise, the file shall be created; the user ID of the 
 * file shall be set to the effective user ID of the process; the group ID of 
 * the file shall be set to the group ID of the file's parent directory or to 
 * the effective group ID of the process; and the access permission bits of 
 * the file mode shall be set to the value of the third argument taken as type 
 * apl_mode_t modified as follows: a bitwise AND is performed on the file-mode 
 * bits and the corresponding bits in the complement of the process' file mode 
 * creation mask. Thus, all bits in the file mode whose corresponding bit in 
 * the file mode creation mask is set are cleared. When bits other than the 
 * file permission bits are set, the effect is unspecified. The third argument 
 * does not affect whether the file is open for reading, writing, or for both. 
 * Implementations shall provide a way to initialize the file's group ID to 
 * the group ID of the parent directory. Implementations may, but need not, 
 * provide an implementation-defined way to initialize the file's group ID to 
 * the effective group ID of the calling process.
 */
#define APL_O_CREAT     (O_CREAT)

/** 
 * Exclusive use flag.
 *
 * If APL_O_CREAT and APL_O_EXCL are set, apl_open() shall fail if the file 
 * exists. The check for the existence of the file and the creation of the 
 * file if it does not exist shall be atomic with respect to other threads 
 * executing apl_open() naming the same filename in the same directory with 
 * APL_O_EXCL and APL_O_CREAT set. If APL_O_EXCL and APL_O_CREAT are set, and 
 * path names a symbolic link, apl_open() shall fail and set errno to 
 * [APL_EEXIST], regardless of the contents of the symbolic link. If 
 * APL_O_EXCL is set and APL_O_CREAT is not set, the result is undefined.
 */
#define APL_O_EXCL      (O_EXCL)

/** 
 * Non-blocking mode.
 *
 * When opening a FIFO with APL_O_RDONLY or APL_O_WRONLY set:
 *  - If APL_O_NONBLOCK is set, an apl_open() for reading-only shall return 
 *    without delay. An apl_open() for writing-only shall return an error if 
 *    no process currently has the file open for reading.
 *  - If APL_O_NONBLOCK is clear, an apl_open() for reading-only shall block 
 *    the calling thread until a thread opens the file for writing. An 
 *    apl_open() for writing-only shall block the calling thread until a 
 *    thread opens the file for reading.
 *
 * When opening a block special or character special file that supports 
 * non-blocking opens:
 *
 *  - If APL_O_NONBLOCK is set, the apl_open() function shall return without 
 *    blocking for the device to be ready or available. Subsequent behavior 
 *    of the device is device-specific.
 *
 *  - If APL_O_NONBLOCK is clear, the apl_open() function shall block the 
 *    calling thread until the device is ready or available before returning.
 *
 * Otherwise, the behavior of APL_O_NONBLOCK is unspecified.
 */
#define APL_O_NONBLOCK  (O_NONBLOCK)

/** 
 * Truncate flag. 
 *
 * If the file exists and is a regular file, and the file is successfully 
 * opened APL_O_RDWR or APL_O_WRONLY, its length shall be truncated to 0, and 
 * the mode and owner shall be unchanged. It shall have no effect on FIFO 
 * special files or terminal device files. Its effect on other file types is 
 * implementation-defined. The result of using APL_O_TRUNC with APL_O_RDONLY 
 * is undefined.
 */
#define APL_O_TRUNC     (O_TRUNC)


/** 
 * Close the file descriptor upon execution of an exec family function. 
 */
#define APL_FD_CLOEXEC  (FD_CLOEXEC)




/** 
 * Set a file segment shared lock.
 *
 * @param[in] ai_fildes     File descriptor.
 * @param[in] ai_whence     Starting offset of the file. the values should be
 *                          one of the APL_SEEK_CUR, APL_SEEK_END or 
 *                          APL_SEEK_SET.
 * @param[in] ai64_start    Relative offset of the file.
 * @param[in] ai64_length   Specifies the length of segment to lock.
 * @param[in] ai_isblock    Specifies the blocking flag,the value is as follow:
 *                           - !=0  This function shall block by other lock.
 *                           - ==0  This function shall non-block.
 * @retval 0    Otherwise. successful.
 * @retval -1   Failure, errno indicate the error.
 */
apl_int_t apl_fcntl_rdlock(
    apl_handle_t    ai_fildes,
    apl_int_t       ai_whence,
    apl_off_t       ai64_start,
    apl_off_t       ai64_length,
    apl_int_t       ai_isblock);

/** 
 * Set a file segment exclusive lock.
 *
 * @param[in] ai_fildes     File descriptor.
 * @param[in] ai_whence     Starting offset of the file. the values should be
 *                          one of the APL_SEEK_CUR, APL_SEEK_END or 
 *                          APL_SEEK_SET.
 * @param[in] ai64_start    Relative offset of the file.
 * @param[in] ai64_length   Specifies the length of segment to lock.
 * @param[in] ai_isblock    Specifies the blocking flag,the value is as follow:
 *                           - !=0  This function shall block by other lock.
 *                           - ==0  This function shall non-block.
 * @retval 0    Otherwise. successful.
 * @retval -1   Failure, errno indicate the error.
 */
apl_int_t apl_fcntl_wrlock(
    apl_handle_t    ai_fildes,
    apl_int_t       ai_whence,
    apl_off_t       ai64_start,
    apl_off_t       ai64_length,
    apl_int_t       ai_isblock);

/** 
 * Clear a file segment lock.
 *
 * @param[in] ai_fildes     File descriptor.
 * @param[in] ai_whence     Starting offset of the file. the values should be
 *                          one of the APL_SEEK_CUR, APL_SEEK_END or 
 *                          APL_SEEK_SET.
 * @param[in] ai64_start    Relative offset of the file.
 * @param[in] ai64_length   Specifies the length of space to unlock.
 *
 * @retval 0    Otherwise. successful.
 * @retval -1   Failure, errno indicate the error.
 */
apl_int_t apl_fcntl_unlock(
    apl_handle_t    ai_fildes,
    apl_int_t       ai_whence,
    apl_off_t       ai64_start,
    apl_off_t       ai64_length);

/** 
 * Get file descriptor flags.
 *
 * @param[in]   ai_fildes   File descriptor.
 * @param[out]  api_flags   A pointer points to the location which hold the 
 *                          file descriptor flags.
 * @retval -1   Failure, errno indicate the error.
 * @retval >=0  Successful, the value is the file descriptor flags.
 */
apl_int_t apl_fcntl_getfd(
    apl_handle_t    ai_fildes,
    apl_int_t*      api_flags);

/** 
 * Set file descriptor flags.
 *
 * @param[in]   ai_fildes   File descriptor.
 * @param[in]   ai_flags    The file descriptor flags which wanted to be set.
 *
 * @retval -1   fail, errno indicate the error.
 * @retval 0    successful.
 */
apl_int_t apl_fcntl_setfd(
    apl_handle_t    ai_fildes,
    apl_int_t       ai_flags);

/** 
 * Get file status flags and file access modes.
 *
 * @param[in]   ai_fildes   File descriptor.
 * @param[out]  api_flags   A pointer points to the location which hold the 
 *                          file status flags.
 *
 * @retval -1   fail, errno indicate the error.
 * @retval >=0  successful, the value is the file status flags and access modes.
 */
apl_int_t apl_fcntl_getfl(
    apl_handle_t    ai_fildes,
    apl_int_t*      api_flags);

/** 
 * Set file status flags and file access modes.
 *
 * @param[in]    ai_fildes    file descriptor.
 * @param[out]   ai_flags    the file status flags which wanted to be set.
 *
 * @retval -1   Fail, errno indicate the error.
 * @retval 0    successful.
 */
apl_int_t apl_fcntl_setfl(
    apl_handle_t    ai_fildes,
    apl_int_t       ai_flags);

/** 
 * Get the process or process group ID specified to receive SIGURG signals 
 * when out-of-band data is available.
 *
 * @param[in]   ai_fildes   File descriptor.
 * @param[out]  api_flags   A pointer points to the location which hold the 
 *                          return socket owner process or process group.
 *
 * @retval -1   fail, errno indicate the error.
 * @retval >=0  successful, the value is the the return socket owner process 
 *              or process group.
 */
apl_int_t apl_fcntl_getown(
    apl_handle_t    ai_fildes,
    apl_int_t*      api_flags);

/** 
 * Set the process or process group ID specified to receive SIGURG signals 
 * when out-of-band data is available.
 *
 * @param[in]    ai_fildes  File descriptor.
 * @param[out]   ai_flags   Socket owner process or process group that want 
 *                          to be set.

 * @retval -1   Fail, errno indicate the error.
 * @retval >=0  Successful, the value is the the return socket owner process 
 *              or process group.
 */
apl_int_t apl_fcntl_setown(
    apl_handle_t    ai_fildes,
    apl_int_t       ai_flags);



/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_FCNTL_H */

