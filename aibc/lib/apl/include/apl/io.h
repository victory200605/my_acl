/**
 * @file io.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.8 $
 */
#ifndef APL_IO_H
#define APL_IO_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include <sys/uio.h>
/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */


/**
 * File number of stdin
 */
#define APL_STDIN_FILENO    (STDIN_FILENO)

/**
 * File number of stdout
 */
#define APL_STDOUT_FILENO   (STDOUT_FILENO)

/**
 * File number of stderr
 */
#define APL_STDERR_FILENO   (STDERR_FILENO)



#define apl_iovec_t iovec


/**
 * Specifies the invalid file descriptor.
 */
#define APL_INVALID_HANDLE      APL_INT_C(-1)

/** 
 * Check is file descriptor value
 *
 * @param[in]    h    the file descriptor wanted to be check.
 *
 * @retval !=0  It is a valid file descriptor.
 * @retval 0    It is a invalid file descriptor.
 */
#define APL_IS_VALID_HANDLE(h)  (h >= 0)


/** 
 * Close a file descriptor.
 *
 * This function shall deallocate the file descriptor indicated by ai_fildes.
 * - When all file descriptors associated with a pipe or FIFO special file 
 *   are closed, any data remaining in the pipe or FIFO shall be discarded.
 * - When all file descriptors associated with an open file description have 
 *   been closed, the open file description shall be freed.
 *
 * @param[in]    ai_fildes    file descriptor would deallocate.
 *
 * @retval 0    successful.
 * @retval -1   fail, errno indicate the error.
 */
apl_int_t apl_close(
    apl_handle_t    ai_fildes);

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
 * @param[in]    ai_fildes    file descriptor.
 * @param[out]   apv_buf      pointer refered to buffer that save the read data.
 * @param[in]    au_nbyte     read data length
 *
 * @retval >=0  read successful,the interger indicate the number of bytes actually read.
 * @retval -1   read fail, errno inducate the error.
 */
apl_ssize_t apl_read(
    apl_handle_t    ai_fildes,
    void*           apv_buf,
    apl_size_t      au_nbyte) ;

/** 
 * Write on a file.
 *
 * This function shall attempt to write au_nbyte bytes from the buffer 
 * pointed to by apv_buf to the file associated with the open file 
 * descriptor, ai_fildes.
 *
 * @param[in]    ai_fildes      file descriptor.
 * @param[in]    apv_buf        pointer refered to the data that writed to the file.
 * @param[in]    au_nbyte       the lengh of data which would write to the file.
 *
 * @retval >=0  write successful, the interger indicate the number of bytes actually write.
 * @retval -1   write fail,errno inducate the error.
 */
apl_ssize_t apl_write(
    apl_handle_t    ai_fildes,
    void const*     apv_buf,
    apl_size_t      au_nbyte) ;

/** 
 * Read a vector.
 *
 * This function shall be equivalent to apl_read(), except the apl_readv() 
 * function shall place the input data into the au_iovcnt buffers specified 
 * by the members of the aps_iov array.Each apl_iovec_t entry specifies the 
 * base address and length of an area in memory where data should be placed.
 * The apl_readv() function shall always fill an area completely before 
 * proceeding to the next.
 *
 * @param[in]    ai_fildes    file descriptor.
 * @param[out]   aps_iov      a apl_iovec_t struct array used as buffers.
 * @param[in]    au_iovcnt    the aps_iov array length.
 *
 * @retval >=0  read successful,the interger indicate the number of bytes actually read.
 * @retval -1   read fail, errno inducate the error.
 */
apl_ssize_t apl_readv(
    apl_handle_t                ai_fildes,
    struct apl_iovec_t const*   aps_iov,
    apl_size_t                  au_iovcnt) ;

/** 
 * Write a vector.
 *
 * This function shall be equivalent to apl_write(), except the apl_writev() 
 * function shall gather output data from the au_iovcnt buffers specified by 
 * the members of the aps_iov array.Each apl_iovec_t entry specifies the base 
 * address and length of an area in memory from which data should be written. 
 * The apl_writev() function shall always write a complete area before 
 * proceeding to the next.
 *
 * @param[in]   ai_fildes    file descriptor.
 * @param[in]   aps_iov      a apl_iovec_t struct array used as buffers.
 * @param[in]   au_iovcnt    the aps_iov array length.
 *
 * @retval >=0  write successful, the interger indicate the number of bytes 
 *              actually write.
 * @retval -1   write fail,errno inducate the error.
 */
apl_ssize_t apl_writev(
    apl_handle_t                ai_fildes,
    struct apl_iovec_t const*   aps_iov,
    apl_size_t                  au_iovcnt) ;

/** 
 * Duplicate an open file descriptor.
 *
 * This function shall duplicate the file descriptor ai_fildes. The 
 * duplicated file descriptor shall share the file item of ai_fildes.
 *
 * @param[in]    ai_fildes    the file descriptor wanted to copy.
 *
 * @retval >=0  successful, the return value is a new file descriptor that 
 *              the system is not used.
 * @retval -1   fail, errno indicate the error.
 */
apl_handle_t apl_dup(
    apl_handle_t    ai_fildes) ;

/** 
 * Duplicate an open file descriptor
 *
 * This function shall duplicate the file descriptor ai_fildes.it is 
 * equivalent to apl_dup() except follow:
 *
 * -# The function will close the ai_fildes2.
 * -# The new file descriptor of duplicate shall equal to or geater than 
 *    ai_fildes2.
 * -# If ai_fildes == ai_fildes2, this function will return ai_fildes2 
 *    without closing.
 *
 * @param[in]   ai_fildes   the file descriptor wanted to copy.
 * @param[in]   ai_fildes2  the least value the new file descriptor would be.
 *
 * @retval >=0  successful, the return value is a new file descriptor that 
 *              the system is not used.
 * @retval -1   fail, errno indicate the error.
 */
apl_handle_t apl_dup2(
    apl_handle_t    ai_fildes,
    apl_handle_t    ai_fildes2) ;


/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */


#endif /* APL_IO_H */

