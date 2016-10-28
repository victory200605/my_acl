#include "apl/dir.h"
#include "apl/limits.h"
#include "apl/errno.h"
#include "apl/heap.h"
#include <assert.h>
#include <sys/stat.h>
#include <stdio.h>

/* ---------------------------------------------------------------------- */
#define APL_DIR_MAGIC       APL_INT_C(0x1f2f3e4e)

#define APL_DIR_CHECK(dir, ret)  \
    do { \
        if (APL_DIR_MAGIC != dir->mi_magic) \
        { \
            apl_set_errno(APL_EBADF); \
            return ret; \
        } \
    } while(0)

/* ---------------------------------------------------------------------- */

apl_int_t apl_mkdir(
    char const* apc_path,
    apl_int_t   ai_mode) 
{
    return (apl_int_t) mkdir(apc_path, (mode_t)ai_mode);
}
    
/* ---------------------------------------------------------------------- */

apl_int_t apl_rmdir(
    char const* apc_path)
{
    return (apl_int_t) rmdir(apc_path);
}
    
/* ---------------------------------------------------------------------- */

apl_int_t apl_remove(
    char const* apc_path)
{
    return (apl_int_t) remove(apc_path);
}
    
/* ---------------------------------------------------------------------- */

apl_int_t apl_opendir(
    struct apl_dir_t*   aps_dir,
    char const*         apc_path)
{
    aps_dir->mps_dir = opendir(apc_path);

    if (aps_dir->mps_dir)
    {
        aps_dir->mps_dirent = (struct dirent*)apl_malloc(sizeof(struct dirent) + APL_NAME_MAX + 1);
        assert(aps_dir->mps_dirent);

        aps_dir->mi_magic = APL_DIR_MAGIC;
    }

    return (aps_dir->mps_dir == APL_NULL) ? APL_INT_C(-1) : 0;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_closedir(
    struct apl_dir_t*   aps_dir)
{
    apl_int_t   li_ret;

    APL_DIR_CHECK(aps_dir, APL_INT_C(-1));

    if (aps_dir->mps_dir)
    {
        assert(aps_dir->mps_dirent);

        apl_free(aps_dir->mps_dirent);

        li_ret = (apl_int_t) closedir(aps_dir->mps_dir);
    
        aps_dir->mi_magic = 0;
    }
    else
    {
        li_ret = APL_EBADF;
    }

    return li_ret;
}
    
/* ---------------------------------------------------------------------- */

struct apl_dirent_t* apl_readdir(
    struct apl_dir_t*       aps_dir)
{
    struct dirent*  aps_ret;
    apl_int_t       li_ret;

    APL_DIR_CHECK(aps_dir, APL_NULL);

    li_ret = (apl_int_t) readdir_r(aps_dir->mps_dir, aps_dir->mps_dirent, &aps_ret);

    return 0 != li_ret ? APL_NULL : (struct apl_dirent_t*)aps_ret;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_seekdir(
    struct apl_dir_t*   aps_dir,
    apl_size_t          au_pos)
{
    APL_DIR_CHECK(aps_dir, APL_INT_C(-1));

    seekdir(aps_dir->mps_dir, (long)au_pos);

    return 0;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_telldir(
   struct apl_dir_t*    aps_dir) 
{
    APL_DIR_CHECK(aps_dir, APL_INT_C(-1));

    return (apl_int_t) telldir(aps_dir->mps_dir);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_rewinddir(
   struct apl_dir_t*    aps_dir) 
{
    APL_DIR_CHECK(aps_dir, APL_INT_C(-1));

    rewinddir(aps_dir->mps_dir);

    return 0;
}

/* ---------------------------------------------------------------------- */
