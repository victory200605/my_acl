
#include "acl/Thread.h"

ACL_NAMESPACE_START

apl_int_t CThread::Spawn(
    apl_thread_t* aptThreadID,
    apl_thr_func_ptr afFunc,
    void* apvArg,
    apl_int_t aiScope,
    apl_size_t aiStackSize )
{
    return apl_thread_create(aptThreadID, afFunc, apvArg, aiStackSize, aiScope);
}

apl_int_t CThread::Spawn(
    apl_thr_func_ptr afFunc,
    void* apvArg,
    apl_int_t aiScope,
    apl_size_t aiStackSize )
{
    apl_thread_t ltThrd;
    return apl_thread_create(&ltThrd, afFunc, apvArg, aiStackSize, aiScope);
}
    
apl_int_t CThread::SpawnN(
    apl_thread_t aptThreadID[],
    apl_size_t aiN,
    apl_thr_func_ptr afFunc,
    void* apvArg,
    apl_int_t aiScope,
    apl_size_t aiStackSize )
{
    for ( apl_size_t liN = 0; liN < aiN; liN++ )
    {
        if ( CThread::Spawn(&aptThreadID[liN], afFunc, apvArg, aiScope, aiStackSize) != 0 )
        {
            return liN;
        }
    }

    return aiN;
}
    
apl_int_t CThread::SpawnN(
    apl_size_t aiN,
    apl_thr_func_ptr afFunc,
    void* apvArg,
    apl_int_t aiScope,
    apl_size_t aiStackSize )
{
    for ( apl_size_t liN = 0; liN < aiN; liN++ )
    {
        if ( CThread::Spawn(afFunc, apvArg, aiScope, aiStackSize) != 0 )
        {
            return liN;
        }
    }

    return aiN;
}
    
void CThread::Exit( void* apvReturn )
{
    apl_thread_exit(apvReturn);
}

apl_int_t CThread::Yield(void)
{
    return apl_thread_yield();
}

apl_thread_t CThread::Self(void)
{
    return apl_thread_self();
}

ACL_NAMESPACE_END
