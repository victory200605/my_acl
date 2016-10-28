#include "apl/dlfcn.h"

/* ---------------------------------------------------------------------- */

void* apl_dlopen(
    char const* apc_path,
    apl_uint_t  au_flags)
{
    return (void*)dlopen(apc_path, au_flags);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_dlclose(
    void*       apv_handle)
{
    return dlclose(apv_handle);
}

/* ---------------------------------------------------------------------- */

void* apl_dlsym(
    void*       apv_handle,
    char const* apc_symname)
{
    return (void*)dlsym(apv_handle, apc_symname);
}

/* ---------------------------------------------------------------------- */

char const* apl_dlerror(void)
{
    return (char const*)dlerror();
}

/* ---------------------------------------------------------------------- */

