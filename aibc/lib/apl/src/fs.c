#include "apl/fs.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/* ---------------------------------------------------------------------- */

apl_int_t apl_link(
    char const* apc_path1,
    char const* apc_path2)
{
    return (apl_int_t) link(apc_path1, apc_path2);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_symlink(
    char const* apc_path1,
    char const* apc_path2)
{
    return (apl_int_t) symlink(apc_path1, apc_path2);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_unlink(
    char const* apc_path)
{
    return (apl_int_t) unlink(apc_path);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_rename(
    char const* apc_path1,
    char const* apc_path2)
{
    return (apl_int_t) rename(apc_path1, apc_path2);
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_readlink(
    char const* apc_path,
    char*       apc_link,
    apl_size_t  au_linklen)
{
    return (apl_ssize_t) readlink(apc_path, apc_link, (size_t)au_linklen);
}

/* ---------------------------------------------------------------------- */

char* apl_realpath(
    char const* apc_relpath,
    char*       apc_abspath,
    apl_size_t  au_abspathlen)
{
    return (char*) realpath(apc_relpath, apc_abspath); 
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_lstat(
    char const*         apc_path,
    struct apl_stat_t*  aps_stat) 
{
    return (apl_int_t) lstat(apc_path, &(aps_stat->ms_stat));
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_stat(
    char const*         apc_path,
    struct apl_stat_t*  aps_stat) 
{
    return (apl_int_t) stat(apc_path, &(aps_stat->ms_stat));
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_fstat(
    apl_handle_t        ai_fildes,
    struct apl_stat_t*  aps_stat)
{
    return (apl_int_t) fstat((int)ai_fildes, &(aps_stat->ms_stat));
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_access(
    char const*     apc_path,
    apl_int_t       ai_amode)
{
    return (apl_int_t) access(apc_path, (int)ai_amode);
}

/* ---------------------------------------------------------------------- */

