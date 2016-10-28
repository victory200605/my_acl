#include "apl/fcntl.h"
#include <fcntl.h>

/* ---------------------------------------------------------------------- */

static inline 
apl_int_t apl_fcntl_get_int(
    apl_handle_t    ai_fildes,
    apl_int_t       ai_cmd,
    apl_int_t*      api_flags)
{
    int li_ret = (apl_int_t) fcntl((int)ai_fildes, (int)ai_cmd);
  
    if (li_ret < 0)
    {  
        return (apl_int_t)li_ret;
    }
    else
    {
        *api_flags = li_ret;
        return 0;
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_fcntl_rdlock(
    apl_handle_t    ai_fildes,
    apl_int_t       ai_whence,
    apl_off_t       ai64_start,
    apl_off_t       ai64_length,
    apl_int_t       ai_isblock) 
{
    struct flock    ls_flock;

    ls_flock.l_type     = F_RDLCK;
    ls_flock.l_whence   = ai_whence;
    ls_flock.l_start    = ai64_start;
    ls_flock.l_len      = ai64_length;

    return (apl_int_t) fcntl((int)ai_fildes, (ai_isblock ? F_SETLKW : F_SETLK), &ls_flock);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_fcntl_wrlock(
    apl_handle_t    ai_fildes,
    apl_int_t       ai_whence,
    apl_off_t       ai64_start,
    apl_off_t       ai64_length,
    apl_int_t       ai_isblock) 
{
    struct flock    ls_flock;

    ls_flock.l_type     = F_WRLCK;
    ls_flock.l_whence   = ai_whence;
    ls_flock.l_start    = ai64_start;
    ls_flock.l_len      = ai64_length;

    return (apl_int_t) fcntl((int)ai_fildes, (ai_isblock ? F_SETLKW : F_SETLK), &ls_flock);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_fcntl_unlock(
    apl_handle_t    ai_fildes,
    apl_int_t       ai_whence,
    apl_off_t       ai64_start,
    apl_off_t       ai64_length)
{
    struct flock    ls_flock;

    ls_flock.l_type     = F_UNLCK;
    ls_flock.l_whence   = ai_whence;
    ls_flock.l_start    = ai64_start;
    ls_flock.l_len      = ai64_length;

    return (apl_int_t) fcntl((int)ai_fildes, F_SETLKW, &ls_flock);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_fcntl_getfd(
    apl_handle_t    ai_fildes,
    apl_int_t*      api_flags)
{
    return apl_fcntl_get_int(ai_fildes, F_GETFD, api_flags);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_fcntl_setfd(
    apl_handle_t    ai_fildes,
    apl_int_t       ai_flags)
{
    return (apl_int_t) fcntl((int)ai_fildes, F_SETFD, (int)ai_flags);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_fcntl_getfl(
    apl_handle_t    ai_fildes,
    apl_int_t*      api_flags)
{
    return apl_fcntl_get_int(ai_fildes, F_GETFL, api_flags);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_fcntl_setfl(
    apl_handle_t    ai_fildes,
    apl_int_t       ai_flags)
{
    return (apl_int_t) fcntl((int)ai_fildes, F_SETFL, (int)ai_flags);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_fcntl_getown(
    apl_handle_t    ai_fildes,
    apl_int_t*      api_flags)
{
    return apl_fcntl_get_int(ai_fildes, F_GETOWN, api_flags);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_fcntl_setown(
    apl_handle_t    ai_fildes,
    apl_int_t       ai_flags)
{
    return (apl_int_t) fcntl((int)ai_fildes, F_SETOWN, (int)ai_flags);
}

/* ---------------------------------------------------------------------- */

