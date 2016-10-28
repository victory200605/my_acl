
#ifndef ACL_THREAD_H
#define ACL_THREAD_H

#include "acl/Utility.h"
#include "acl/TimeValue.h"
#include "acl/Timestamp.h"

ACL_NAMESPACE_START

class CThread
{
public:
    static apl_int_t Spawn(
        apl_thread_t* aptThreadID,
        apl_thr_func_ptr afFunc,
        void* apvArg,
        apl_int_t aiScope = APL_THREAD_USER,
        apl_size_t aiStackSize = 0 );
    
    static apl_int_t Spawn(
        apl_thr_func_ptr afFunc,
        void* apvArg,
        apl_int_t aiScope = APL_THREAD_USER,
        apl_size_t aiStackSize = 0 );
        
    static apl_int_t SpawnN(
        apl_thread_t aptThreadID[],
        apl_size_t aiN,
        apl_thr_func_ptr afFunc,
        void* apvArg,
        apl_int_t aiScope = APL_THREAD_USER,
        apl_size_t aiStackSize = 0 );
        
    static apl_int_t SpawnN(
        apl_size_t aiN,
        apl_thr_func_ptr afFunc,
        void* apvArg,
        apl_int_t aiScope = APL_THREAD_USER,
        apl_size_t aiStackSize = 0 );
        
    static void Exit( void* apvReturn );
    
    static apl_int_t Yield(void);
    
    static apl_thread_t Self(void);
};

ACL_NAMESPACE_END

#endif //ACL_THREAD_H

