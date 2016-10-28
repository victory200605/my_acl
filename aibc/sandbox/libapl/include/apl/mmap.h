/**
 *
 */
#ifndef APL__MMAP_H
#define APL__MMAP_H

/**
 *
 */
int apl_mmap_map_file(
    apl_mmap_t* restrict    p_mmap,
    apl_file_t* restrict    p_file,
    off_t                   offset,
    size_t                  len,
    int                     mflag,
    int                     prot
);

/**
 *
 */
int apl_mmap_map_shm(
    apl_mmap_t* restrict    p_mmap,
    apl_file_t* restrict    p_file,
    off_t                   offset,
    size_t                  len,
    int                     mflag,
    int                     prot
);

/**
 *
 */
int apl_mmap_unmap(
    apl_mmap_t*     p_mmap
);

/**
 *
 */
int apl_mmap_lock(
    apl_mmap_t*     p_mmap
);

/**
 *
 */ 
int apl_mmap_unlock(
    apl_mmap_t*     p_mmap
);


#endif // APL__MMAP_H

