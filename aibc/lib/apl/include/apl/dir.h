/**
 * @file dir.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.10 $
 */
#ifndef APL_DIR_H
#define APL_DIR_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include <dirent.h>


/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */
/** designates a structure capable of representing any directory entry */
#define apl_dirent_t dirent


/**
 * A type representing a directory stream
 */
struct apl_dir_t
{
    DIR*            mps_dir;
    struct dirent*  mps_dirent;
    apl_int_t       mi_magic;
};

/** 
 * Make a directory.
 *
 * This function shall create a new directory with name apc_path. 
 *
 * The file permission bits of the new directory shall be initialized from 
 * ai_mode. These file permission bits of the ai_mode argument shall be 
 * modified by the process's file creation mask.
 *
 * When bits in ai_mode other than the file permission bits are set, the 
 * meaning of these additional bits is implementation-defined.
 * 
 * @param[in]   apc_path    the directory which be wanted to creat.
 * @param[in]   ai_mode     the permission bits which to be set.
 *
 * @retval  0   create successful.
 * @retval  -1  no directory shall be created, and errno shall be set to 
 *              indicate the error.
 */
apl_int_t apl_mkdir(
    char const*     apc_path,
    apl_int_t       ai_mode);

/** 
 * Remove a directory.
 *
 * This function shall remove a directory whose name is given by apc_path. 
 *
 * The directory shall be removed only if it is an empty directory.
 *
 *  -   If the directory is the root directory or the current working 
 *      directory of any process, it is unspecified whether the function 
 *      succeeds, or whether it shall fail and set errno to [APL_EBUSY].
 *
 *  -   If path names a symbolic link, then rmdir() shall fail and set errno to
 *      [APL_ENOTDIR].
 *
 *  -   If the path argument refers to a path whose final component is either 
 *      dot or dot-dot, rmdir() shall fail.
 * 
 *  -   If the directory's link count becomes 0 and no process has the 
 *      directory open, the space occupied by the directory shall be freed
 *      and the directory shall no longer be accessible. If one or more 
 *      processes have the directory open when the last link is removed, 
 *      the dot and dot-dot entries, if present, shall be removed before 
 *      rmdir() returns and no new entries may be created in the directory, 
 *      but the directory shall not be removed until all references to the 
 *      directory are closed.
 *
 *  -   If the directory is not an empty directory, rmdir() shall fail and 
 *      set errno to [APL_EEXIST] or [APL_ENOTEMPTY].
 *
 * @param[in]   apc_path    the the directory which be wanted to remove.
 *
 * @retval  0   remove successful.
 * @retval  -1  the named directory shall not be changed,and errno set to 
 *              indicate the error.
 */
apl_int_t apl_rmdir(
    char const*     apc_path);

/** 
 * Remove a file or directory
 *
 * This function shall cause the file named by the pathname pointed to by 
 * apc_path to be no longer accessible by that name.
 *
 * A subsequent attempt to open that file using that name shall fail, unless
 * it is created anew.
 *
 * @param[in]   apc_path    pointed to the patchname which is wanted to remove.
 *
 * @retval  0   remove successful.
 * @retval  -1  the named file shall not be changed, and errno indicate the 
 *              error.
 */
apl_int_t apl_remove(
    char const*     apc_path);

/** 
 * Open a directory.
 * 
 * This function shall open a directory stream corresponding to the directory 
 * named by the apc_path argument.
 *
 * The directory stream is positioned at the first entry.
 *
 * @param[in]   apc_path    the directory that wanted to open
 * @param[out]  aps_dir     a pointer points to the object of type ap_dir_t.
 *
 * @retval  0   successful.
 * @retval  -1  fail, and errno indicate the error.
 */
apl_int_t apl_opendir(
    struct apl_dir_t*   aps_dir,
    char const*         apc_path);

/** 
 * Close a directory stream.
 *
 * This function shall lose the directory stream referred to by the argument aps_dir.
 *
 * Upon return, the value of aps_dir may no longer point to an accessible object of the type apl_dir_t.
 *
 * @param[in]   aps_dir     the pointer which referred to the directory stream.
 *
 * @retval  0   close successful.
 * @retval  -1  close fail and errno set to indicate the error.
 */
apl_int_t apl_closedir(
    struct apl_dir_t*   aps_dir);

/** 
 * Read a directory.
 *
 * This function shall return a pointer to a structure representing the 
 * directory entry at the current position in the directory stream specified 
 * by the argument aps_dir, and position the directory stream at the next 
 * entry. It shall return a null pointer upon reaching the end of the 
 * directory stream.
 *
 * After a call to fork(), either the parent or child (but not both) may 
 * continue processing the directory stream using apl_readdir(), 
 * apl_rewinddir(), or apl_seekdir(). If both the parent and child processes 
 * use these functions, the result is undefined.
 *
 * @param[in]   aps_dir     the pointer which referred to the directory stream.
 *
 * @retval  APL_NULL    fail, errno indicate the error.
 * @retval  !=APL_NULL  successful, the return pointer is to a structure 
 *                      representing the directory.
 */
struct apl_dirent_t* apl_readdir(
    struct apl_dir_t*   aps_dir);

/** 
 * Set the position of a directory stream.
 *
 * This function shall set the position of the next apl_readdir() operation on
 * the directory stream specified by aps_dir to the position specified by
 * au_pos.
 *
 * The value of au_pos should have been returned from an earlier call to 
 * apl_telldir(). The new position reverts to the one associated with the 
 * directory stream when apl_telldir() was performed.
 *
 * If the value of au_pos was not obtained from an earlier call to 
 * apl_telldir(), or if a call to apl_rewinddir() occurred between the call to
 * apl_telldir() and the call to apl_seekdir(), the results of subsequent 
 * calls to apl_readdir() are unspecified.
 *
 * @param[in]   aps_dir     the pointer which referred to the directory stream.
 * @param[in]   au_pos      index which set the position of the directory stream
 *
 * @retval 0    set position successful.
 * @retval -1   set position fail.
 */
apl_int_t apl_seekdir(
    struct apl_dir_t*   aps_dir,
    apl_size_t          au_pos);

/** 
 * Current location of a named directory stream.
 *
 * This function shall obtain the current location associated with the 
 * directory stream specified by aps_dir.
 *
 * @param[in]   aps_dir     the pointer which referred to the directory stream.
 *
 * @return  the current location of the specified directory stream.
 */
apl_int_t apl_telldir(
   struct apl_dir_t*    aps_dir);

/** 
 * Reset the position of a directory stream to the beginning of a directory.
 *
 * This function shall reset the position of the directory stream to which 
 * aps_dir refers to the beginning of the directory.
 *
 * After a call to the fork() function, either the parent or child (but not 
 * both) may continue processing the directory stream using apl_readdir(), 
 * apl_rewinddir(), or apl_seekdir().
 *
 * If both the parent and child processes use these functions, the result is 
 * undefined.
 *
 * @param[in] aps_dir   the pointer which referred to the directory stream.
 *
 * @retval  0   reset successful.
 * @retval  -1  reset fail.
 */
apl_int_t apl_rewinddir(
   struct apl_dir_t*    aps_dir);

/* ----------------------- */  APL_DECLARE_END  /* ----------------------- */

#endif /* APL_DIR_H */

