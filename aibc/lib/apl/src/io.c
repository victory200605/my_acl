#include "apl/io.h"
#include <unistd.h>

/* ---------------------------------------------------------------------- */

apl_int_t apl_close(
    apl_handle_t    ai_fildes)
{
    return (apl_int_t) close(ai_fildes);
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_read(
    apl_handle_t    ai_fildes,
    void*           apv_buf,
    apl_size_t      au_nbyte)
{
    return (apl_ssize_t) read((int)ai_fildes, apv_buf, (size_t)au_nbyte);
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_write(
    apl_handle_t    ai_fildes,
    void const* apv_buf,
    apl_size_t  au_nbyte)
{
    return (apl_ssize_t) write((int)ai_fildes, apv_buf, (size_t)au_nbyte);
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_readv(
    apl_handle_t                ai_fildes,
    struct apl_iovec_t const*   aps_iov,
    apl_size_t                  au_iovcnt) 
{
    return (apl_ssize_t) readv((int)ai_fildes, (struct iovec* const)aps_iov, (int)au_iovcnt);
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_writev(
    apl_handle_t                ai_fildes,
    struct apl_iovec_t const*   aps_iov,
    apl_size_t                  au_iovcnt)
{
    return (apl_ssize_t) writev((int)ai_fildes, (struct iovec* const)aps_iov, (int)au_iovcnt);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_dup(
    apl_handle_t    ai_fildes)
{
    return (apl_int_t) dup((int)ai_fildes);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_dup2(
    apl_handle_t    ai_fildes,
    apl_handle_t    ai_fildes2)
{
    return (apl_int_t) dup2((int)ai_fildes, (int)ai_fildes2);
}

/* ---------------------------------------------------------------------- */

