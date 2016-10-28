#include "apl/mman.h"
#include <unistd.h>
#include <sys/mman.h>


/* ---------------------------------------------------------------------- */

void* apl_mmap( 
    void*           apv_addr,
    apl_size_t      au_addrlen,
    apl_int_t       ai_prot,
    apl_int_t       ai_flags,
    apl_handle_t    ai_fildes,
    apl_off_t       ai64_offset)
{
    return (void*) mmap(apv_addr, 
        (size_t)au_addrlen, 
        (int)ai_prot, 
        (int)ai_flags, 
        (int)ai_fildes,
        (off_t)ai64_offset);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_munmap(
    void*       apv_addr,
    apl_size_t  au_addrlen) 
{
    return (apl_int_t) munmap(apv_addr, (size_t)au_addrlen);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_msync(
    void*       apv_addr,
    apl_size_t  au_addrlen,
    apl_int_t   ai_flags)
{
    return (apl_int_t) msync(apv_addr, (size_t)au_addrlen, (int)ai_flags);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mlock(
    void const* apv_addr,
    apl_size_t  au_addrlen)
{
    return (apl_int_t) mlock(apv_addr, (size_t)au_addrlen);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_munlock(
    void const* apv_addr,
    apl_size_t  au_addrlen)
{
    return (apl_int_t) munlock(apv_addr, (size_t)au_addrlen);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mlockall(
    apl_int_t   ai_flags)
{
    return (apl_int_t) mlockall((int)ai_flags);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_munlockall(void)
{
    return (apl_int_t) munlockall();
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_madvise(
    void*       apv_addr,
    apl_size_t  au_addrlen,
    apl_int_t   ai_advice)
{
#if defined(HAVE_POSIX_MADVISE)
    return (apl_int_t) posix_madvise(apv_addr, (size_t)au_addrlen, (int)ai_advice);
#else
    return 0;
#endif 
}

/* ---------------------------------------------------------------------- */

apl_handle_t apl_shm_open(
    char const* apc_name,
    apl_int_t   ai_oflags,
    apl_int_t   ai_mode)
{
    return (apl_handle_t)shm_open(apc_name, (int)ai_oflags, (mode_t)ai_mode);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_shm_unlink(
    char const* apc_name)
{
    return (apl_int_t)shm_unlink(apc_name);
}

/* ---------------------------------------------------------------------- */

