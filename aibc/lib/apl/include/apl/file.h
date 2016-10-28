/**
 * @file file.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.11 $
 */
#ifndef APL_FILE_H
#define APL_FILE_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include "apl/fcntl.h"
#include <stdio.h>
#include <unistd.h>

#ifdef HAVE_SYS_FCNTL_H
#   include <sys/fcntl.h>
#endif

#ifdef HAVE_FCNTL_H
#   include <fcntl.h>
#endif

/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

#if defined(HAVE_POSIX_FADVISE)
#   define APL_FADV_NORMAL      (POSIX_FADV_NORMAL)
#   define APL_FADV_SEQUENTIAL  (POSIX_FADV_SEQUENTIAL)
#   define APL_FADV_RANDOM      (POSIX_FADV_RANDOM)
#   define APL_FADV_WILLNEED    (POSIX_FADV_WILLNEED)
#   define APL_FADV_DONTNEED    (POSIX_FADV_DONTNEED)
#   define APL_FADV_NOREUSE     (POSIX_FADV_NOREUSE)
#else
#   define APL_FADV_NORMAL      (0)
#   define APL_FADV_SEQUENTIAL  (0)
#   define APL_FADV_RANDOM      (0)
#   define APL_FADV_WILLNEED    (0)
#   define APL_FADV_DONTNEED    (0)
#   define APL_FADV_NOREUSE     (0)
#endif

/**
 * The application has no advice to give on its behavior with respect to the 
 * specified data.
 *
 * @def APL_FADV_NORMAL
 */

/**
 * The application expects to access the specified data sequentially from 
 * lower offsets to higher offsets.
 *
 * @def APL_FADV_SEQUENTIAL
 */

/**
 * The application expects to access the specified data in a random order.
 *
 * @def APL_FADV_RANDOM
 */

/**
 * The application expects to access the specified data in the near future.
 *
 * @def APL_FADV_WILLNEED
 */

/**
 * The application expects that it will not access the specified data in the 
 * near future.
 *
 * @def APL_FADV_DONTNEED
 */

/**
 * The application expects to access the specified data once and then not 
 * reuse it thereafter.
 *
 * @def APL_FADV_NOREUSE
 */





/**
 * Set file offset to current plus ai64_offset.
 */
#define APL_SEEK_CUR    (SEEK_CUR)

/**
 * Set file offset to EOF plus ai64_offset.
 */
#define APL_SEEK_END    (SEEK_END)

/**
 * Set file offset to offset.
 */
#define APL_SEEK_SET    (SEEK_SET)


/** 
 *  This function shall establish the connection between a file and a file 
 *  descriptor.
 *
 *  It shall create an open file description that refers to a file and a file 
 *  descriptor that refers to that open file description.
 * 
 *  The file offset used to mark the current position within the file shall be 
 *  set to the beginning of the file.
 * 
 *  Values for ai_oflag are constructed by a bitwise-inclusive OR of flags 
 *  from the following list, defined in <apl/fcntl.h>. Applications shall 
 *  specify exactly one of the first three values (file access modes) below in 
 *  the value of oflag:
 *  <ul>
 *  <li>
 *      APL_O_RDONLY \n
 *      Open for reading only.
 *  </li>
 *
 *  <li>
 *      APL_O_WRONLY \n
 *      Open for writing only.
 *  </li>
 *
 *  <li>
 *      APL_O_RDWR \n
 *      Open for reading and writing. The result is undefined if this flag is 
 *      applied to a FIFO.
 *  </li> 
 *  </ul>
 *
 *  Any combination of the following may be used:
 * 
 *  <ul>
 *  <li> 
 *      APL_O_APPEND \n
 *      If set, the file offset shall be set to the end of the file prior to 
 *      each write.
 *  </li>
 *
 *  <li>
 *      APL_O_CREAT \n
 *      If the file exists, this flag has no effect except as noted under
 *      APL_O_EXCL below. Otherwise, the file shall be created; the user ID 
 *      of the file shall be set to the effective user ID of the process; the 
 *      group ID of the file shall be set to the group ID of the file's 
 *      parent directory or to the effective group ID of the process; and the 
 *      access permission bits of the file mode shall be set to the value of 
 *      the third argument taken as type apl_mode_t modified as follows: a 
 *      bitwise AND is performed on the file-mode bits and the corresponding 
 *      bits in the complement of the process' file mode creation mask. Thus, 
 *      all bits in the file mode whose corresponding bit in the file mode 
 *      creation mask is set are cleared. When bits other than the file 
 *      permission bits are set, the effect is unspecified. The third 
 *      argument does not affect whether the file is open for reading, 
 *      writing, or for both. Implementations shall provide a way to 
 *      initialize the file's group ID to the group ID of the parent 
 *      directory. Implementations may, but need not, provide an 
 *      implementation-defined way to initialize the file's group ID to the 
 *      effective group ID of the calling process.
 *  </li>
 *
 *  <li>
 *      APL_O_EXCL \n
 *      If APL_O_CREAT and APL_O_EXCL are set, apl_open() shall fail if the 
 *      file exists. The check for the existence of the file and the creation 
 *      of the file if it does not exist shall be atomic with respect to 
 *      other threads executing apl_open() naming the same filename in the 
 *      same directory with APL_O_EXCL and APL_O_CREAT set. If APL_O_EXCL and 
 *      APL_O_CREAT are set, and path names a symbolic link, apl_open() shall 
 *      fail and set errno to [APL_EEXIST], regardless of the contents of the 
 *      symbolic link. If APL_O_EXCL is set and APL_O_CREAT is not set, the 
 *      result is undefined.
 *  </li>
 *
 *  <li>
 *      APL_O_NONBLOCK \n
 *      When opening a FIFO with APL_O_RDONLY or APL_O_WRONLY set:
 *
 *      <ul>
 *      <li>
 *          If APL_O_NONBLOCK is set, an apl_open() for reading-only shall 
 *          return without delay. An apl_open() for writing-only shall return 
 *          an error if no process currently has the file open for reading.
 *      </li>
 *
 *      <li>
 *          If APL_O_NONBLOCK is clear, an apl_open() for reading-only shall 
 *          block the calling thread until a thread opens the file for 
 *          writing. An apl_open() for writing-only shall block the calling 
 *          thread until a thread opens the file for reading.
 *      </li>
 *      </ul>
 *
 *      When opening a block special or character special file that supports 
 *      non-blocking opens:
 *      
 *      <ul> 
 *      <li>
 *          If APL_O_NONBLOCK is set, the apl_open() function shall return 
 *          without blocking for the device to be ready or available. 
 *          Subsequent behavior of the device is device-specific.
 *      </li>
 *
 *      <li>
 *          If APL_O_NONBLOCK is clear, the apl_open() function shall block 
 *          the calling thread until the device is ready or available before 
 *          returning.
 *      </li>
 *      </ul>
 *
 *      Otherwise, the behavior of O_NONBLOCK is unspecified.
 *  </li>
 *
 *  <li>
 *      APL_O_TRUNC \n
 *      If the file exists and is a regular file, and the file is 
 *      successfully opened APL_O_RDWR or APL_O_WRONLY, its length shall be 
 *      truncated to 0, and the mode and owner shall be unchanged. It shall 
 *      have no effect on FIFO special files or terminal device files. Its 
 *      effect on other file types is implementation-defined. The result of 
 *      using APL_O_TRUNC with APL_O_RDONLY is undefined.
 *  </li>
 *  </ul>
 *
 *  If APL_O_CREAT is set and the file did not previously exist, upon 
 *  successful completion, apl_open() shall mark for update the st_atime, 
 *  st_ctime, and st_mtime fields of the file and the st_ctime and st_mtime 
 *  fields of the parent directory.
 *
 *  If APL_O_TRUNC is set and the file did previously exist, upon successful 
 *  completion, apl_open() shall mark for update the st_ctime and st_mtime 
 *  fields of the file.
 *
 *  @param[in] apc_path The file pathname
 *  @param[in] ai_ofalg The file status flags that used to opening file,
 *  @param[in] ai_mode  The access permission bits of the file mode,it does 
 *                      not affect whether the file is open for reading, 
 *                      writing, or for both.
 *
 *  @retval >=0 The lowest numbered unused file descriptor which is a 
 *              non-negative integer.open file successful.
 *  @retval -1  Open file fail, no files shall be created or modified, and 
 *              the errno indicate the error.
 */
apl_handle_t apl_open(
    char const*     apc_path,
    apl_int_t       ai_ofalg,
    apl_int_t       ai_mode);

/** 
 * Read from a file.
 *
 * This function shall attempt to read au_nbyte bytes from the file 
 * associated with the open file descriptor, ai_fildes, into the buffer 
 * pointed to by apv_buf.
 *
 * The behavior of multiple concurrent reads on the same pipe, FIFO, or 
 * terminal device is unspecified.
 *
 * @param[in]   ai_fildes   file descriptor.
 * @param[out]  apv_buf     pointer refered to buffer that save the read data.
 * @param[in]   au_nbyte    read data length
 * @param[in]   ai64_offset offset inside the file
 *
 * @retval >=0  read successful,the interger indicate the number of bytes actually 
 *              read.
 * @retval -1   read fail, errno inducate the error.
 */
apl_ssize_t apl_pread(
    apl_handle_t    ai_fildes,
    void*           apv_buf,
    apl_size_t      au_nbyte,
    apl_off_t       ai64_offset);

/** 
 *
 * This function shall attempt to write au_nbyte bytes from the buffer 
 * pointed to by apv_buf to the file associated with the open file 
 * descriptor ai_fildes.
 *
 * @param[in]   ai_fildes   File descriptor.
 * @param[in]   apv_buf     Pointer refered to the data that writed to the 
 *                          file.
 * @param[in]   au_nbyte    The lengh of data which would write to the file.
 * @param[in]   ai64_offset Offset inside the file.
 *
 * @retval >=0  Write successful, the interger indicate the number of bytes 
 *              actually write.
 * @retval 0    Write fail,errno inducate the error.
 */
apl_ssize_t apl_pwrite(
    apl_handle_t    ai_fildes,
    void const*     apv_buf,
    apl_size_t      au_nbyte,
    apl_off_t       ai64_offset);

/** 
 * Move the read/write file offset.
 *
 * This function shall set the file offset for the open file description 
 * associated with the file descriptor ai_fildes, as follows:
 *
 * - If ai_whence is APL_SEEK_SET, the file offset shall be set to 
 *   ai64_offset bytes.
 *
 * - If ai_whence is APL_SEEK_CUR, the file offset shall be set to 
 *   its current location plus ai64_offset.
 *
 * - If ai_whence is APL_SEEK_END, the file offset shall be set to 
 *   the size of the file plus ai64_offset.
 *
 * @param[in]   ai_fildes      file descriptor.
 * @param[in]   ai64_offset    the file offset.
 * @param[in]   ai_whence      set offset mode, see upon.
 *
 * @retval >=0  Set offset successful, the resulting offset is measured in 
 *              bytes from the beginning of the file.
 * @retval -1   Set offset fail, errno inducate the error.
 */
apl_off_t apl_lseek(
    apl_handle_t    ai_fildes,
    apl_off_t       ai64_offset,
    apl_int_t       ai_whence);

/** 
 * Synchronize changes to a file.
 *
 * This function shall request that all data for the open file descriptor 
 * named by ai_fildes is to be transferred to the storage device associated 
 * with the file described by ai_fildes.
 *
 * The nature of the transfer is implementation-defined. The apl_fsync() 
 * function shall not return until the system has completed that action or 
 * until an error is detected.
 *
 * @param[in]     ai_fildes     file descriptor.
 *
 * @retval 0    Successful.
 * @retval -1   Fail, errno indicate the error.
 */
apl_int_t apl_fsync(
    apl_handle_t    ai_fildes);

/** 
 * Schedule file system updates.
 *
 * This function shall cause all information in memory that updates file 
 * systems to be scheduled for writing out to all file systems.
 *
 * The writing, although scheduled, is not necessarily complete upon return 
 * from apl_sync().
 */
void apl_sync(void);

/** 
 * Truncate a file to a specified length.
 *
 * This function shall cause the size of the file to be truncated to 
 * ai64_length.
 *
 * @param[in]    ai_fildes      file descriptor.
 * @param[in]    ai64_length    the size which wanted to be truncated.
 *
 * @retval 0    Successful.
 * @retval -1   Failure, errno indicate the error.
 */
apl_int_t apl_ftruncate(
    apl_handle_t    ai_fildes,
    apl_off_t       ai64_length);

/** 
 * File space control.
 *
 * This function shall ensure that any required storage for regular file data 
 * starting at ai64_offset and continuing for ai64_length bytes is allocated 
 * on the file system storage media.
 *
 * If apl_fallocate() returns successfully, subsequent writes to the 
 * specified file data shall not fail due to the lack of free space on the 
 * file system storage media.
 *
 * @param[in]    ai_fildes      file descriptor.
 * @param[in]    ai64_offset    the file offset, indicate the beginning of allocating.
 * @param[in]    ai64_length    the space length is allocated on the file system storage media.
 *
 * @retval  0   successful.
 * @retval  -1  an error number shall be returned to indicate the error.
 *
 */
apl_int_t apl_fallocate(
    apl_handle_t    ai_fildes,
    apl_off_t       ai64_offset,
    apl_off_t       ai64_length);

/** 
 * File advisory information.
 *
 * This function shall advise the implementation on the expected behavior of 
 * the application with respect to the data in the file associated with the 
 * open file descriptor, ai_fildes, starting at ai64_offset and continuing 
 * for ai64_length bytes. The specified range need not currently exist in the 
 * file. If len is zero, all data following ai64_offset is specified. The 
 * implementation may use this information to optimize handling of the specified 
 * data.
 *
 * @param[in]   ai_fildes   file descriptor.
 * @param[in]   ai64_offset file offset.
 * @param[in]   ai64_length the data length.
 * @param[in]   ai_advice   the ai_advice to be applied to the data is 
 *                          specified by the ai_advice parameter and may be 
 *                          one of the following values:
 *                          - APL_FADV_NORMAL \n 
 *                              Specifies that the application has no advice 
 *                              to give on its behavior with respect to the 
 *                              specified data. It is the default 
 *                              characteristic if no advice is given for an 
 *                              open file. 
 *                          - APL_FADV_SEQUENTIAL \n
 *                              Specifies that the application expects to 
 *                              access the specified data sequentially from 
 *                              lower offsets to higher offsets.
 *                          - APL_FADV_RANDOM \n
 *                              Specifies that the application expects to 
 *                              access the specified data in a random order.
 *                          - APL_FADV_WILLNEED \n
 *                              Specifies that the application expects to 
 *                              access the specified data in the near future.
 *                          - APL_FADV_DONTNEED \n
 *                              Specifies that the application expects that 
 *                              it will not access the specified data in the 
 *                              near future.
 *                          - APL_FADV_NOREUSE \n
 *                              Specifies that the application expects to 
 *                              access the specified data once and then not 
 *                              reuse it thereafter.
 *
 * @retval  0   successful.
 * @retval  -1  fail, an error number shall be returned to indicate the error.
 */
apl_int_t apl_fadvise(
    apl_handle_t    ai_fildes,
    apl_off_t       ai64_offset,
    apl_off_t       ai64_length,
    apl_int_t       ai_advice);


/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_FILE_H */

