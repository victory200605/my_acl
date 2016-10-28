/**
 * @file fs.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.9 $
 */
#ifndef APL_FS_H
#define APL_FS_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include <sys/stat.h>
#include <unistd.h>


/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

/**
 * Test for existence of file.
 */
#define APL_F_OK    (F_OK)

/**
 * Test for read permission.
 */
#define APL_R_OK    (R_OK)

/**
 * Test for write permission.
 */
#define APL_W_OK    (W_OK)

/**
 * Test for execute (search) permission.
 */
#define APL_X_OK    (X_OK)



struct apl_stat_t
{
    struct stat     ms_stat;
};

/** 
 * Link to a file.
 *
 * This function shall create a new link (directory entry) for the existing 
 * file and the link count of the file shall be incremented by one.
 *
 * @param[in]    apc_path1    points to a pathname naming an existing file.
 * @param[in]    apc_path2    points to a pathname naming the new directory entry to be created.
 *
 * @retval  0   successful.
 * @retval  -1  fail, errno indicate the error.
 */
apl_int_t apl_link(
    char const*     apc_path1,
    char const*     apc_path2);

/** 
 * Make a symbolic link to a file.

 * This function shall create a symbolic link called apc_path2 that contains 
 * the string pointed to by apc_path1.
 *
 * @param[in]    apc_path1
 * @param[in]    apc_path2
 *
 * @retval  0   successful.
 * @retval  -1  fail, errno indicate the error.
 */
apl_int_t apl_symlink(
    char const*     apc_path1,
    char const*     apc_path2);

/** 
 * Remove a directory entry.
 *
 * This function shall remove a link to a file. If apc_path names a symbolic 
 * link, apl_unlink() shall remove the symbolic link named by apc_path and 
 * shall not affect any file or directory named by the contents of the 
 * symbolic link. Otherwise, apl_unlink() shall remove the link named by the 
 * pathname pointed to by path and shall decrement the link count of the file 
 * referenced by the link.
 * 
 * When the file's link count becomes 0 and no process has the file open, the 
 * space occupied by the file shall be freed and the file shall no longer be 
 * accessible.
 *
 * @param[in]   apc_path    the file pathname or a symbolic referred to a file.
 *
 * @retval  0   remove successful.
 * @retval  -1  remove fail, errno indicate the error and the named file shall 
 *              not be changed.
 */
apl_int_t apl_unlink(
    char const*     apc_path);

/** 
 * Rename a file.
 *
 * This function shall rename a file/directory from apc_path1 to apc_path2.
 * 
 * @param[in]   apc_path1
 * @param[in]   apc_path2
 *
 * @retval  0   successful.
 * @retval  -1  fail, errno indicate the error.
 */

apl_int_t apl_rename(
    char const*     apc_path1,
    char const*     apc_path2);

/** 
 * Read the contents of a symbolic link.
 *
 * This function shall place the contents of the symbolic link referred to by 
 * apc_path in the buffer apc_link which has size au_linklen.
 *
 * @param[in]   apc_path    the symbolic link
 * @param[out]  apc_link    buffer store the data read from the symbolic link.
 * @param[in]   au_linklen  indicate the length of read.
 *
 * @retval >=0  successful, the return interger is the count of bytes places in apc_link.
 * @retval -1   fail, errno indicate the error.
 */
apl_ssize_t apl_readlink(
    char const*     apc_path,
    char*           apc_link,
    apl_size_t      au_linklen);

/** 
 * Resolve a pathname.
 *
 * This function shall derive, from the pathname pointed to by apc_relpath, 
 * an absolute pathname that names the same file, whose resolution does not 
 * involve dot, dot-dot, or symbolic links. The generated pathname shall be 
 * stored as a null-terminated string, up to a maximum of {APL_PATH_MAX} 
 * bytes, in the buffer pointed to by apc_abspath.
 *
 * @param[in]    apc_relpath    file pathname.
 * @param[out]   apc_abspath    a pointer pointed to a buffer which stored the result string.
 * @param[out]   au_abspathlen  indicate the result string length.
 *
 * @retval !=APL_NULL   successful, the pointer point to the apc_abspath.
 * @retval APL_NULL     fail, errno indicate the error.
 */
char* apl_realpath(
    char const*     apc_relpath,
    char*           apc_abspath,
    apl_size_t      au_abspathlen);

/** 
 * Get symbolic link status.
 *
 * This function shall get symbolic link status.
 *
 * @param[in]   apc_path    a symbolic link.
 * @param[out]  apc_stat    the return status of apc_path.
 *
 * @retval 0    successful.
 * @retval -1   fail, errno indicate the error.
 */
apl_int_t apl_lstat(
    char const*         apc_path,
    struct apl_stat_t*  apc_stat);

/** 
 * Get file status. 
 *
 * This function shall obtain information about the named file and write it 
 * to the area pointed to by the apc_stat argument.
 *
 * If the named file is a symbolic link, the apl_stat() function shall 
 * continue pathname resolution using the contents of the symbolic link, and 
 * shall return information pertaining to the resulting file if the file 
 * exists.
 *
 * @param[in]   apc_path    a pointer points to a pathname naming a file.
 * @param[out]  apc_stat    a pointer points to a apl_stat_t that store the file 
 *                          information.
 *
 * @retval 0    successful.
 * @retval -1   fail, errno indicate the error.
 */
apl_int_t apl_stat(
    char const*         apc_path,
    struct apl_stat_t*  apc_stat);

/** 
 * Get file status.
 *
 * This function shall obtain information about an open file associated with 
 * the file descriptor ai_fildes, and shall write it to the area pointed to 
 * by aps_stat.
 *
 * @param[in]   ai_fildes   file decriptor.
 * @param[out]  aps_stat    a pointer points to a apl_stat_t that store the 
 *                          file information.
 *
 * @retval 0    successful.
 * @retval -1   fail, errno indicate the error.
 */
apl_int_t apl_fstat(
    apl_handle_t           ai_fildes,
    struct apl_stat_t*  aps_stat);

/** 
 * Determine accessibility of a file.
 *
 * This function shall check the file named by the pathname pointed to by the 
 * apc_path argument for accessibility according to the bit pattern contained 
 * in ai_amode, using the real user ID in place of the effective user ID and 
 * the real group ID in place of the effective group ID.
 *
 * @param[in]   apc_path    A pointer points to a file pathname.
 * @param[in]   ai_amode    A value of bit pattern that indicate the access 
 *                          permission mode. The value of ai_amode is either 
 *                          the bitwise-inclusive OR of the access 
 *                          permissions to be checked (APL_R_OK, APL_W_OK, 
 *                          APL_X_OK) or the existence test (APL_F_OK).
 *
 * @retval 0    successful.
 * @retval -1   fail, errno indicate the error.
 */
apl_int_t apl_access(
    char const*     apc_path,
    apl_int_t       ai_amode);

/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_FS_H */

