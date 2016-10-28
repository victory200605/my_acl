/**
 * @file mman.h
 * @author  $Author: fzz $
 * @date    $Date: 2010/05/19 07:07:31 $
 * @version $Revision: 1.9 $
 */
#ifndef APL_MMAN_H
#define APL_MMAN_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include "apl/fcntl.h"
#include <sys/mman.h>

/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

/**
 * Page can be read.
 */
#define APL_PROT_READ       (PROT_READ)

/**
 * Page can be written.
 */
#define APL_PROT_WRITE      (PROT_WRITE)

/**
 * Page can be executed.
 */
#define APL_PROT_EXEC       (PROT_EXEC)

/**
 * Page cannot be accessed.
 */
#define APL_PROT_NONE       (PROT_NONE)

/**
 * Share changes.
 */
#define APL_MAP_SHARED      (MAP_SHARED)

/**
 * PChanges are private.
 */
#define APL_MAP_PRIVATE     (MAP_PRIVATE)

/**
 * Interpret addr exactly.
 */
#define APL_MAP_FIXED       (MAP_FIXED)


/**
 * Perform asynchronous writes.
 */
#define APL_MS_ASYNC        (MS_ASYNC)

/**
 * Perform synchronous writes.
 */
#define APL_MS_SYNC         (MS_SYNC)

/**
 * Invalidate mappings.
 */
#define APL_MS_INVALIDATE   (MS_INVALIDATE)

/**
 * Lock currently mapped pages.
 */
#define APL_MCL_CURRENT     (MCL_CURRENT)

/**
 * Lock pages that become mapped.
 */
#define APL_MCL_FUTURE      (MCL_FUTURE)


#if defined(HAVE_POSIX_MADVISE)
#   define APL_MADV_NORMAL      (POSIX_MADV_NORMAL)
#   define APL_MADV_SEQUENTIAL  (POSIX_MADV_SEQUENTIAL)
#   define APL_MADV_RANDOM      (POSIX_MADV_RANDOM)
#   define APL_MADV_WILLNEED    (POSIX_MADV_WILLNEED)
#   define APL_MADV_DONTNEED    (POSIX_MADV_DONTNEED)
#else
#   define APL_MADV_NORMAL      (0)
#   define APL_MADV_SEQUENTIAL  (0)
#   define APL_MADV_RANDOM      (0)
#   define APL_MADV_WILLNEED    (0)
#   define APL_MADV_DONTNEED    (0)
#endif


/**
 * The application has no advice to give on its behavior with respect to the specified data.
 *
 * @def APL_MADV_NORMAL
 */

/**
 * The application expects to access the specified data sequentially from lower offsets to higher offsets.
 *
 * @def APL_MADV_SEQUENTIAL
 */

/**
 * The application expects to access the specified data in a random order.
 *
 * @def APL_MADV_RANDOM
 */

/**
 * The application expects to access the specified data in the near future.
 *
 * @def APL_MADV_WILLNEED
 */

/**
 * The application expects that it will not access the specified data in the near future.
 *
 * @def APL_MADV_DONTNEED
 */



/**
 * The the error return of apl_mmap() function 
 */
#define APL_MAP_FAILED          (MAP_FAILED)


/** 
 * Map pages of memory.
 *
 * This function shall establish a mapping between a process' address space 
 * and a file.
 *
 * The mmap() function shall establish a mapping between the address space of 
 * the process at an address which shall return for au_addrlen bytes to the 
 * memory object represented by the file descriptor ai_fildes at offset 
 * ai64_offset for au_addrlen bytes.
 *
 * @param[in]   apv_addr    pointed to the memory address which wants to be 
 *                          mapped.
 *                          - APL_NULL \n
 *                            indicate granting the system complete freedom 
 *                            in selecting address.
 *                          - !=APL_NULL \n
 *                            indicate suggesting the system select the 
 *                            mapping address near apv_addr.
 *                            If ai_flags is seted to APL_MAP_FIXED, the 
 *                            function is informed that the value of mapping 
 *                            address shall be apv_addr.
 *                            If the address of apv_addr can not be used to 
 *                            map, the function will fail.
 *
 * @param[in]   au_addrlen  indicate the length of file which represented by 
 *                          ai_fildes to map to memory.
 *                          This argument shall be multiple of the page size.
 * @param[in]   ai_prot     Determines whether read, write, execute, or some 
 *                          combination of accesses are permitted to the data 
 *                          being mapped.
 *                          - APL_PROT_READ \n
 *                            Data can be read.
 *                          - APL_PROT_WRITE \n
 *                            Data can be written.
 *                          - APL_PROT_EXEC \n
 *                            Data can be executed.
 *                          - APL_PROT_NONE\n
 *                            Data cannot be accessed.
 *
 * @param[in]   ai_flags    Provides other information about the handling of 
 *                          the mapped data.
 *                          - APL_MAP_SHARED \n
 *                            Changes are shared. If APL_MAP_SHARED is 
 *                            specified, write references shall change the 
 *                            underlying object.
 *                          - APL_MAP_PRIVATE \n
 *                            Changes are private.
 *                            If APL_MAP_PRIVATE is specified, modifications 
 *                            to the mapped data by the calling process shall 
 *                            be visible only to the calling process and 
 *                            shall not change the underlying object. 
 *                          - APL_MAP_FIXED \n
 *                            Interpret apv_addr exactly. When APL_MAP_FIXED 
 *                            is set in the flags argument, the function is 
 *                            informed that the value of mapping address 
 *                            shall be apv_addr.
 *
 * @param[in]   ai_fildes   file descriptor represents to the file which 
 *                          wants to map to memory.
 * @param[in]   ai64_offset offset in the file represented by ai_fildes.
 *
 * @retval APL_MAP_FAILED   fail, errno indicate the error.
 * @retval !=APL_MAP_FAILED successful, it will return the address at which 
 *                          the mapping was placed.
 */
void* apl_mmap(
    void*           apv_addr,
    apl_size_t      au_addrlen,
    apl_int_t       ai_prot,
    apl_int_t       ai_flags,
    apl_handle_t    ai_fildes,
    apl_off_t       ai64_offset);

/** 
 * Unmap pages of memory.
 *
 * This function shall remove any mappings for those entire pages containing 
 * any part of the address space of the process starting at apv_addr and 
 * continuing for au_addrlen bytes.
 *
 * - If there are no mappings in the specified address range, then this 
 *   function has no effect.
 * - If a mapping to be removed was private, any modifications made in this 
 *   address range shall be discarded.
 *
 * @param[in]   apv_addr    address of starting removing.It shall be a 
 *                          multiple of the page size.
 * @param[in]   au_addrlen  length of removing.
 *
 * @retval 0    successful.
 * @retval -1   fail, errno indicate the error.
 */
apl_int_t apl_munmap(
    void*           apv_addr,
    apl_size_t      au_addrlen);

/** 
 * Synchronize memory with physical storage.
 *
 * This function shall write all modified data to permanent storage 
 * locations, if any, in those whole pages containing any part of the address 
 * space of the process starting at address apv_addr and continuing for au_addrlen bytes.
 *
 * When the function is called on APL_MAP_PRIVATE mappings, any modified data 
 * shall not be written to the underlying object and shall not cause such 
 * data to be made visible to other processes.
 *
 * @param[in]   apv_addr    the start memory address that need to write to 
 *                          permanent storage locations,it shall be a 
 *                          multiple of the page size.
 * @param[in]   au_addrlen  the length of data shall be written.
 * @param[in]   ai_flags    This argument is constructed from the 
 *                          bitwise-inclusive OR of one or more of the 
 *                          following flags:
 *                          - APL_MS_ASYNC \n
 *                            Perform asynchronous writes. If this flag is 
 *                            specified, function shall return immediately 
 *                            once all the write operations are initiated or 
 *                            queued for servicing.
 *                          - APL_MS_SYNC \n
 *                            Perform synchronous writes. If this flag is 
 *                            specified, function shall not return until all 
 *                            write operations are completed as defined for 
 *                            synchronized I/O data integrity completion.
 *                          - APL_MS_INVALIDATE \n
 *                            Invalidate cached data.If this flag is 
 *                            specified, function shall invalidate all cached 
 *                            copies of mapped data that are inconsistent 
 *                            with the permanent storage locations.
 *
 * @retval 0    successful.
 * @retval -1   fail, errno indicate the error.
 */
apl_int_t apl_msync(
    void*           apv_addr,
    apl_size_t      au_addrlen,
    apl_int_t       ai_flags);

/** 
 * Lock a range of process address space. 
 *
 * This function shall cause those whole pages containing any part of the 
 * address space of the process starting at address apv_addr and continuing 
 * for au_addrlen bytes to be memory-resident until unlocked or until the 
 * process exits or execs another process image.
 *
 * Upon successful return from this function, pages in the specified range 
 * shall be locked and memory-resident.
 *
 * @param[in]   apv_addr    the start address of memory to lock.
 * @param[in]   au_addrlen  length of memory to lock.
 *
 * @retval 0    successful.
 * @retval -1   fail, no change is made to any locks in the address space of 
 *              the process, and errno indicate the error.
 */
apl_int_t apl_mlock(
    void const*     apv_addr,
    apl_size_t      au_addrlen);

/** 
 * Unlock a range of process address space.
 * 
 * This function shall unlock those whole pages containing any part of the 
 * address space of the process starting at address apv_addr and continuing 
 * for au_addrlen bytes, regardless of how many times apl_mlock() has been 
 * called by the process for any of the pages in the specified range.
 *
 * @param[in]    apv_addr    the start address of memory to unlock.
 * @param[in]    au_addrlen  length of memory to unlock.
 *
 * @retval 0    successful.
 * @retval -1   fail, no change is made to any locks in the address space of 
 *              the process, and errno indicate the error.
 */
apl_int_t apl_munlock(
    void const*     apv_addr,
    apl_size_t      au_addrlen);

/** 
 * Lock the address space of a process.
 *
 * This function shall cause all of the pages mapped by the address space of 
 * a process to be memory-resident until unlocked or until the process exits 
 * or execs another process image.
 *
 * @param[in]   ai_flags    This argument determines whether the pages to be 
 *                          locked are those currently mapped by the address 
 *                          space of the process, those that are mapped in 
 *                          the future, or both.
 *                          - APL_MCL_CURRENT \n
 *                            Lock all of the pages currently mapped into the 
 *                            address space of the process.
 *                          - APL_MCL_FUTURE \n
 *                            Lock all of the pages that become mapped into 
 *                            the address space of the process in the future, 
 *                            when those mappings are established.
 *
 * @retval 0    successful.
 * @retval -1   fail, no change is made to any locks in the address space of 
 *              the process, and errno indicate the error.
 */
apl_int_t apl_mlockall(
    apl_int_t       ai_flags);

/** 
 * Unlock the address space of a process.
 *
 * This function shall unlock all currently mapped pages of the address space 
 * of the process.
 *
 * @retval 0    successful.
 * @retval -1   fail, no change is made to any locks in the address space of 
 *              the process, and errno indicate the error.
 */
apl_int_t apl_munlockall(void);

/** 
 * Memory advisory information and alignment control.
 *
 * This function shall advise the implementation on the expected behavior of 
 * the application with respect to the data in the memory starting at address 
 * apv_addr, and continuing for au_addrlen bytes.
 *
 * @param[in]   apv_addr    the start address of memory that advise.
 * @param[in]   au_addrlen  length of the memory space to advise.
 * @param[in]   ai_advice   this parameter may be one of the following values:
 *                          - APL_MADV_NORMAL \n
 *                            Specifies that the application has no advice to 
 *                            give on its behavior with respect to the 
 *                            specified range.
 *                            It is the default characteristic if no advice 
 *                            is given for a range of memory.
 *                          - APL_MADV_SEQUENTIAL \n
 *                            Specifies that the application expects to 
 *                            access the specified range sequentially from 
 *                            lower addresses to higher addresses.
 *                          - APL_MADV_RANDOM \n
 *                            Specifies that the application expects to 
 *                            access the specified range in a random order.
 *                          - APL_MADV_WILLNEED \n
 *                            Specifies that the application expects to 
 *                            access the specified range in the near future.
 *                          - APL_MADV_DONTNEED \n
 *                            Specifies that the application expects that it 
 *                            will not access the specified range in the near 
 *                            future.
 * @retval 0    successful.
 * @retval -1   fail, the error number will return.
 */
apl_int_t apl_madvise(
    void*           apv_addr,
    apl_size_t      au_addrlen,
    apl_int_t       ai_advice);

/**
 * Open a shared memory object.
 *
 * This function shall establish a connection between a shared memory object 
 * and a file descriptor. It shall create an open file description that 
 * refers to the shared memory object and a file descriptor that refers to 
 * that open file description. The file descriptor is used by other functions 
 * to refer to that shared memory object. The name argument points to a 
 * string naming a shared memory object. It is unspecified whether the 
 * name appears in the file system and is visible to other functions that 
 * take pathnames as arguments. The name argument conforms to the 
 * construction rules for a pathname. If name begins with the slash 
 * character, then processes calling apl_shm_open() with the same value of 
 * name refer to the same shared memory object, as long as that name has not 
 * been removed. If name does not begin with the slash character, the effect 
 * is implementation-defined. The interpretation of slash characters other 
 * than the leading slash character in name is implementation-defined.
 *
 * The ai_oflag argument is the bitwise-inclusive OR of the following flags 
 * defined in the <apl/fcntl.h>  header. Applications specify exactly one of 
 * the first two values (access modes) below in the value of oflag:
 *
 * <ul>
 * <li>
 *   APL_O_RDONLY \n
 *     Open for read access only.
 * </li>
 * <li>
 *   APL_O_RDWR \n
 *     Open for read or write access.
 * </li>
 * </ul>
 *
 * Any combination of the remaining flags may be specified in the value of oflag:
 *
 * <ul>
 * <li>
 *   APL_O_CREAT \n
 *     If the shared memory object exists, this flag has no effect, except as 
 *   noted under APL_O_EXCL below. Otherwise, the shared memory object is created; 
 *   the user ID of the shared memory object shall be set to the effective 
 *   user ID of the process; the group ID of the shared memory object is set 
 *   to a system default group ID or to the effective group ID of the process. 
 *   The permission bits of the shared memory object shall be set to the value 
 *   of the mode argument except those set in the file mode creation mask of 
 *   the process. When bits in mode other than the file permission bits are 
 *   set, the effect is unspecified. The mode argument does not affect whether 
 *   the shared memory object is opened for reading, for writing, or for both. 
 *   The shared memory object has a size of zero.
 * </li>
 * <li>
 *   APL_O_EXCL \n
 *     If APL_O_EXCL and APL_O_CREAT are set, apl_shm_open() fails if the shared 
 *   memory object exists. The check for the existence of the shared memory object 
 *   and the creation of the object if it does not exist is atomic with respect 
 *   to other processes executing apl_shm_open() naming the same shared memory 
 *   object with APL_O_EXCL and APL_O_CREAT set. If APL_O_EXCL is set and APL_O_CREAT 
 *   is not set, the result is undefined.
 * </li>
 * <li>
 *   APL_O_TRUNC \n
 *     If the shared memory object exists, and it is successfully opened APL_O_RDWR, 
 *   the object shall be truncated to zero length and the mode and owner shall 
 *   be unchanged by this function call. The result of using APL_O_TRUNC with 
 *   APL_O_RDONLY is undefined.
 * </li>
 *
 *     When a shared memory object is created, the state of the shared memory object, 
 *   including all data associated with the shared memory object, persists until the 
 *   shared memory object is unlinked and all other references are gone. It is 
 *   unspecified whether the name and shared memory object state remain valid after 
 *   a system reboot.
 *
 *  @param[in] apc_name The the name of the memory object 
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
apl_handle_t apl_shm_open(
    char const*     apc_name, 
    apl_int_t       ai_oflags, 
    apl_int_t       ai_mode);

/**
 * Remove a shared memory object.
 *
 *   This function shall remove the name of the shared memory object named by the 
 * string pointed to by name.
 *
 *   If one or more references to the shared memory object exist when the object 
 * is unlinked, the name shall be removed before apl_shm_unlink() returns, but 
 * the removal of the memory object contents shall be postponed until all open 
 * and map references to the shared memory object have been removed.
 *
 *   Even if the object continues to exist after the last apl_shm_unlink(), reuse 
 * of the name shall subsequently cause apl_shm_open() to behave as if no shared 
 * memory object of this name exists (that is, apl_shm_open() will fail if APL_O_CREAT 
 * is not set, or will create a new shared memory object if APL_O_CREAT is set).
 *
 * @param[in] apc_name The the name of the memory object 
 *
 * @retval 0    successful.
 * @retval -1   fail, the error number will return.
 */
apl_int_t apl_shm_unlink(char const* apc_name);


/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_MMAN_H */

