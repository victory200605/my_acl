/**
 *
 */
#ifndef APL_FILE_H
#define APL_FILE_H

#include "apl/config.h"
#include "apl/common.h"
#include "apl/types.h"

APL_BEGIN_C_DECLARE

/**
 *
 */
int apl_file_open(
    apl_file_t* restrict    p_file, 
    char const* restrict    p_fname, 
    int                     oflag, 
    mode_t                  mode
);

/**
 *
 */
int apl_file_close(
    apl_file_t*     p_file
);

/**
 *
 */
int apl_file_dup(
    apl_file_t* restrict    p_fromfile, 
    apl_file_t* restrict    p_tofile
);

/**
 *
 */
int apl_file_read(
    apl_file_t* restrict    p_file, 
    apl_byte_t* restrict    p_buf, 
    size_t                  maxsz, 
    size_t* restrict        p_rest
);

/**
 *
 */
int apl_file_write(
    apl_file_t* restrict        p_file, 
    apl_byte_t const* restrict  p_buf,
    size_t                      maxsz, 
    size_t* restrict            p_rest
);

/**
 *
 */
int apl_file_read_n(
    apl_file_t* restrict    p_file, 
    apl_byte_t* restrict    p_buf, 
    size_t                  maxsz, 
    size_t* restrict        p_rest
);

/**
 *
 */
int apl_file_write_n(
    apl_file_t* restrict        p_file, 
    apl_byte_t const* restrict  p_buf,
    size_t                      maxsz, 
    size_t* restrict            p_rest
);

/**
 *
 */
int apl_file_pread(
    apl_file_t* restrict    p_file, 
    off_t                   offset, 
    apl_byte_t* restrict    p_buf, 
    size_t                  maxsz, 
    size_t* restrict        p_rest
);

/**
 *
 */
int apl_file_pwrite(
    apl_file_t* restrict        p_file, 
    off_t                       offset, 
    apl_byte_t const* restrict  p_buf, 
    size_t                      maxsz, 
    size_t* restrict            p_rest
);

/**
 *
 */
int apl_file_pread_n(
    apl_file_t* restrict    p_file, 
    off_t                   offset, 
    apl_byte_t* restrict    p_buf, 
    size_t                  maxsz, 
    size_t* restrict        p_rest
);

/**
 *
 */
int apl_file_pwrite_n(
    apl_file_t* restrict        p_file, 
    off_t                       offset, 
    apl_byte_t const* restrict  p_buf, 
    size_t                      maxsz, 
    size_t* restrict            p_rest
);

/**
 *
 */
int apl_file_allocate(
    apl_file_t*     p_file,
    off_t           start,
    off_t           len,
);

/**
 *
 */
int apl_file_truncate(
    apl_file_t*     p_file,
    off_t           len,
);

/**
 *
 */
int apl_file_get_flag(
    apl_file_t* restrict    p_file,
    int* restrict           p_flag
);


/**
 *
 */
int apl_file_set_flag(
    apl_file_t*     p_file,
    int             flag
);

/**
 *
 */
int apl_file_get_access_mode(
    apl_file_t* restrict    p_file,
    mode_t* restrict        mode 
);

/**
 *
 */
int apl_file_set_access_mode(
    apl_file_t*     p_file,
    mode_t          mode 
);

/**
 *
 */
int apl_file_rdlock(
    apl_file_t*     p_file,
    off_t           start,
    off_t           len,
    short           whence 
);

/**
 * 
 */
int apl_file_wrlock(
    apl_file_t*     p_file,
    off_t           start,
    off_t           len,
    short           whence   
);

/**
 *
 */
int apl_file_try_rdlock(
    apl_file_t*     p_file,
    off_t           start,
    off_t           len,
    short           whence   
);

/**
 * 
 */
int apl_file_try_wrlock(
    apl_file_t*     p_file,
    off_t           start,
    off_t           len,
    short           whence   
);

/**
 *
 */
int apl_file_unlock(
    apl_file_t*     p_file,
    off_t           start,
    off_t           len,
    short           whence
);


/**
 *
 */
int apl_file_advise(
    apl_file_t*     p_file,
    off_t           start,
    off_t           len,
    int             advice
);


APL_END_C_DECLARE

#endif // APL_FILE_H

