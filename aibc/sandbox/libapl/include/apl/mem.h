/**
 *
 */
#ifndef APL__MEM_H
#define APL__MEM_H

/**
 *
 */
int apl_mem_alloc(
    apl_mem_t*  p_mem,
    size_t      sz
);

/**
 *
 */
int apl_mem_alloc_align(
    apl_mem_t*  p_mem,
    size_t      sz,
    size_t      align
);

/**
 *
 */
int apl_mem_realloc(
    apl_mem_t*  p_mem,
    size_t      newsz
);

/**
 *
 */
int apl_mem_free(
    apl_mem_t* p_mem,
);

/**
 *
 */
int apl_mem_get_size(
    apl_mem_t*  p_mem,
    size_t*     p_sz
);

/**
 *
 */
int apl_mem_lock(
    apl_mem_t* p_mem
);

/**
 *
 */
int apl_mem_unlock(
    apl_mem_t* p_mem
);

/**
 *
 */
int apl_mem_advise(
    apl_mem_t*  p_mem,
    int         flag 
);

/**
 *
 */
int apl_mem_lockall();

/**
 *
 */
int apl_mem_unlockall();

#endif // APL__MEM_H

