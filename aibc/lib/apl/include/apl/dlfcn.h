/**
 * @file dlfcn.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.5 $
 */
#ifndef APL_DLFCN_H
#define APL_DLFCN_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include <dlfcn.h>

/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

/**
 * Relocations are performed at an implementation-defined time.
 *
 * Relocations shall be performed at an implementation-defined time, ranging 
 * from the time of the apl_dlopen() call until the first reference to a 
 * given symbol occurs. Specifying APL_RTLD_LAZY should improve performance 
 * on implementations supporting dynamic symbol binding as a process may not 
 * reference all of the functions in any given object. And, for systems 
 * supporting dynamic symbol resolution for normal process execution, this 
 * behavior mimics the normal handling of process execution.
 */
#define APL_RTLD_LAZY   (RTLD_LAZY)

/**
 * Relocations are performed when the object is loaded.
 *
 * All necessary relocations shall be performed when the object is first 
 * loaded. This may waste some processing if relocations are performed for 
 * functions that are never referenced. This behavior may be useful for 
 * applications that need to know as soon as an object is loaded that all 
 * symbols referenced during execution are available.
 */
#define APL_RTLD_NOW    (RTLD_NOW)

/**
 * All symbols are available for relocation processing of other modules.
 *
 * The object's symbols shall be made available for the relocation processing 
 * of any other object. In addition, symbol lookup using apl_dlopen(0, mode) 
 * and an associated apl_dlsym() allows objects loaded with this mode to be 
 * searched.
 */
#define APL_RTLD_GLOBAL (RTLD_GLOBAL)

/**
 * All symbols are not made available for relocation processing by other 
 * modules.
 *
 * The object's symbols shall not be made available for the relocation
 * processing of any other object.
 */
#define APL_RTLD_LOCAL  (RTLD_LOCAL)


/** 
 * Gain access to an executable object file.
 *
 * Make an executable object file specified by apc_path available to the 
 * calling program.
 *
 * Typically such files are executable objects such as shared libraries, 
 * relocatable files, or programs.
 *
 * @param[in]    apc_path   The file pathname.
 * @param[in]    au_flags   This parameter describes how this function shall 
 *                          operate upon apc_path with respect to the 
 *                          processing of relocations and the scope of 
 *                          visibility of the symbols provided within apc_path.
 *
 * @retval  APL_NULL    fail, call apl_dlerror() will get more detail infomation.
 * @retval  !=APL_NULL  successful, a handle will return.
 */
void* apl_dlopen(
    char const* apc_path,
    apl_uint_t  au_flags);

/** 
 * Close a dlopen object.
 *
 * Inform the system that the object referenced by a apv_handle is no longer 
 * needed by the application.
 *
 * @param[in]   apv_handle    a pointer points to a object which would close.
 *
 * @retval  0   successful.
 * @retval  -1  fail, call apl_dlerror() will get more detail infomation.
 */
apl_int_t apl_dlclose(
    void*       apv_handle);

/** 
 * Obtain the address of a symbol from a dlopen object.
 *
 * Obtain the address of a symbol defined within an object made accessible 
 * through an apl_dlopen() call.
 *
 * @param[in]    aps_handle    this value return from a call from apl_dlopen().
 * @param[in]    apc_symname   a symbol's name as a character string.
 *
 * @retval  APL_NULL    failure, call apl_dlerror() for more detail.
 * @retval  !=APL_NULL  successful, a handle will return.
 */
void* apl_dlsym(
    void*       aps_handle,
    char const* apc_symname);

/** 
 * Get diagnostic information.
 *
 * Return a null-terminated character string that describes the last error 
 * that occurred during dynamic linking processing.
 *
 * @retval  APL_NULL    failure.
 * @retval  !=APL_NULL  a null-terminated character string.
 */
char const* apl_dlerror(void);


/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_DLFCN_H */

