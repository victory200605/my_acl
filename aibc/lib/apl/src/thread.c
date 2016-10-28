#include "apl/thread.h"
#include "apl/limits.h"
#include "apl/errno.h"
#include <pthread.h>
#include <sched.h>
#include <assert.h>
/* ---------------------------------------------------------------------- */

apl_int_t apl_thread_create(
    apl_thread_t*       api_thr_id,
    apl_thr_func_ptr    apf_thr_func,
    void*               apv_arg,
    apl_size_t          au_stacksize,
    apl_uint_t          au_flags)
{
    pthread_t       ls_thrid;
    pthread_attr_t  ls_thrattr;
    apl_int_t       li_ret;
    apl_int_t       li_ret2;
    int             li_scope;
    const char*     lpc_stacksize;

    if (au_flags != 0 && au_flags != APL_THREAD_KERNEL && au_flags != APL_THREAD_USER)
    {
        return APL_EINVAL;
    }

    if (au_stacksize == 0)
    {
        lpc_stacksize = getenv("APL_THREAD_STACK_SIZE");

        if (lpc_stacksize != APL_NULL)
        {
            //even in convert error, 0 will return
            au_stacksize = apl_strtou32(lpc_stacksize, APL_NULL, 10);
        }
    }

    if (au_stacksize > 0 && au_stacksize < APL_THREAD_STACK_MIN)
    {
        return APL_EINVAL;
    }

    li_ret = pthread_attr_init(&ls_thrattr);

    if (li_ret != 0) 
    { 
        return li_ret;
    } 

    if (au_stacksize != 0)
    {
        li_ret = pthread_attr_setstacksize(&ls_thrattr, (size_t)au_stacksize);

        if (li_ret != 0 && li_ret != APL_ENOTSUP) 
        { 
            goto DESTROY_AND_EXIT; 
        }
    }

    li_ret = pthread_attr_setdetachstate(&ls_thrattr, PTHREAD_CREATE_DETACHED);

    if (li_ret != 0 && li_ret != APL_ENOTSUP) 
    { 
        goto DESTROY_AND_EXIT; 
    }

    if (APL_THREAD_KERNEL == au_flags)
    {
        li_scope = PTHREAD_SCOPE_SYSTEM;
    }
    else if (APL_THREAD_USER == au_flags)
    {
        li_scope = PTHREAD_SCOPE_PROCESS;
    }
    else
    {
        li_ret = APL_EINVAL;
        goto DESTROY_AND_EXIT;
    }

    li_ret = pthread_attr_setscope(&ls_thrattr, li_scope);
    
    if (li_ret != 0 && li_ret != APL_ENOTSUP) 
    { 
        goto DESTROY_AND_EXIT; 
    }
    
    li_ret = pthread_create(&ls_thrid, &ls_thrattr, apf_thr_func, apv_arg);

    if (api_thr_id)
    {
        *api_thr_id = (apl_int_t)ls_thrid;
    }

DESTROY_AND_EXIT:
    li_ret2 = pthread_attr_destroy(&ls_thrattr);
    assert(0 == li_ret2);

    APL_ERRNO_RETURN(li_ret);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_thread_yield(void)
{
    return (apl_int_t) sched_yield();
}

/* ---------------------------------------------------------------------- */

void apl_thread_exit(void* apv_ret)
{
    pthread_exit(apv_ret);
}

/* ---------------------------------------------------------------------- */

apl_thread_t apl_thread_self(void)
{
    return (apl_thread_t) pthread_self();
}

/* ---------------------------------------------------------------------- */

