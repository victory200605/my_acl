#include "apl/file.h"
#include "apl/errno.h"

/* ---------------------------------------------------------------------- */

apl_handle_t apl_open(
    char const*     apc_path,
    apl_int_t       ai_oflags,
    apl_int_t       ai_mode)
{
    return (apl_handle_t) open(apc_path, ai_oflags, ai_mode);
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_pread(
    apl_handle_t    ai_fildes,
    void*           apv_buf,
    apl_size_t      au_nbyte,
    apl_off_t       ai64_offset) 
{
    return (apl_ssize_t) pread(
        (int)ai_fildes, 
        apv_buf, 
        (size_t)au_nbyte, 
        (off_t)ai64_offset);
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_pwrite(
    apl_handle_t    ai_fildes,
    void const*     apv_buf,
    apl_size_t      au_nbyte,
    apl_off_t       ai64_offset)
{
    return (apl_ssize_t) pwrite(
        (int)ai_fildes, 
        apv_buf, 
        (size_t)au_nbyte,
        (off_t)ai64_offset);
}

/* ---------------------------------------------------------------------- */

apl_off_t apl_lseek(
    apl_handle_t    ai_fildes,
    apl_off_t       ai64_offset,
    apl_int_t       ai_whence)
{
    return (apl_off_t) lseek(
        (int)ai_fildes,
        (off_t)ai64_offset,
        (int)ai_whence);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_fsync(
    apl_handle_t    ai_fildes)
{
    return (apl_int_t) fsync((int)ai_fildes);
}

/* ---------------------------------------------------------------------- */

void apl_sync(void) 
{
    sync();
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_ftruncate(
    apl_handle_t      ai_fildes,
    apl_off_t     ai64_length)
{
    return (apl_int_t) ftruncate((int)ai_fildes, (off_t)ai64_length);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_fallocate(
    apl_handle_t    ai_fildes,
    apl_off_t       ai64_offset,
    apl_off_t       ai64_length)
{
#if defined(HAVE_POSIX_FALLOCATE)
    return (apl_int_t) posix_fallocate(
        (int)ai_fildes,
        (off_t)ai64_offset,
        (off_t)ai64_length);
#else
    apl_off_t   li64_exp;
    apl_off_t   li64_len;

    if (ai64_offset < 0 || ai64_length < 0)
    {
        apl_set_errno(APL_EINVAL);
        return APL_INT_C(-1);
    }

    li64_len = apl_lseek(ai_fildes, 0, APL_SEEK_END);
  
    if (APL_INT64_C(-1) == li64_len)
    {
        return APL_INT_C(-1);
    }
  
    li64_exp = ai64_offset + ai64_length; 

    if (li64_exp <= li64_len)
    {
        return 0;
    }

    char        lac_data[4096] = {0};
    while (li64_len < li64_exp)
    {
        apl_off_t   li64_need = li64_exp - li64_len;
        apl_ssize_t li_ret;

        if (((apl_off_t)sizeof(lac_data)) > li64_need)
        {
            li_ret = apl_write(ai_fildes, lac_data, (apl_size_t)li64_need);
        }
        else
        {
            li_ret = apl_write(ai_fildes, lac_data, sizeof(lac_data));
        }

        if (li_ret < 0)
        {
            return li_ret;
        }
        else
        {
            li64_len += li_ret;
        }
    }  

    return 0;
#endif
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_fadvise(
    apl_handle_t    ai_fildes,
    apl_off_t       ai64_offset,
    apl_off_t       ai64_length,
    apl_int_t       ai_advice) 
{
#if defined(HAVE_POSIX_FADVISE) && !defined(APL_OS_HPUX)
    return (apl_int_t) posix_fadvise(
        (int)ai_fildes,
        (off_t)ai64_offset,
        (off_t)ai64_length,
        (int)ai_advice);
#else
    apl_off_t li64_len = apl_lseek(ai_fildes, 0, APL_SEEK_END);

    if (APL_INT64_C(-1) == li64_len )
    {
        return APL_INT_C(-1);
    }

    if (ai64_offset < 0 
        || ai64_length < 0
        || ai64_offset + ai64_length > li64_len)
    {
        apl_set_errno(APL_EINVAL);
        return APL_INT_C(-1);
    }

    return 0;
#endif
}

/* ---------------------------------------------------------------------- */

