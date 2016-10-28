/**
 * @file ThreadManager.h
 */

#ifndef ACL_THREADMANAGER_H
#define ACL_THREADMANAGER_H

#include "acl/Utility.h"
#include "acl/TimeValue.h"
#include "acl/Timestamp.h"
#include "acl/Synch.h"
#include "acl/stl/list.h"

ACL_NAMESPACE_START

/**
 * Class CThreadManager
 */
class CThreadManager
{
    class CThreadDescriptor
    {
    public:
        CThreadDescriptor( apl_int_t aiGrpID, apl_int_t aiScope );
        
        ~CThreadDescriptor();
        
        apl_thread_t GetThreadID(void);
        
        apl_int_t GetGrpID(void);
        
        apl_int_t GetRunState(void);
        
        apl_int_t GetScope(void);
        
        void* GetReturn(void);
        
        void SetThreadID( apl_thread_t aiThreadID );
        
        void SetGrpID( apl_int_t aiGrpID );
        
        void SetRunState( apl_int_t aiState );
        
        void SetReturn( void* apvReturn );
        
        void Reference(void);
        
        void Release(void);
        
        void Lock(void);
        
        void Unlock(void);
        
    protected:
        apl_thread_t miThreadID;
        
        apl_int_t miScope;
        
        apl_int_t miGrpID;
        
        apl_int_t miRunState;
        
        void* mpvReturn;
        
        CLock moLock;
        
        apl_int_t miReference;
    };
    
    class CThreadAdapter
    {
    public:
        CThreadAdapter(
            apl_thr_func_ptr afFunc,
            void* apvArg,
            CThreadDescriptor* apoThreadDesc );
       
        ~CThreadAdapter(void);
        
        void Invoke(void);
        
    protected:
        apl_thr_func_ptr mfFunc;
        void* mpvArg;
        CThreadDescriptor* mpoThreadDesc;
    };

public:

    /**
     * thread run state enumeration
     */
    enum ERunState
    {
        /**
         * THR_IDLE = 0
         */
        THR_IDLE = 0,
        
        /**
         * THR_SPAWNED
         */
        THR_SPAWNED,
        
        /**
         * THR_RUNNING
         */
        THR_RUNNING,
        
        /**
         * THR_TERMINATED
         */
        THR_TERMINATED
    };
    
    typedef std::list<CThreadDescriptor*>::iterator DescIterType;
    
public:
 
    /**
     * Constructor
     */
    CThreadManager(void);
    
    /**
     * destructor 
     */
    ~CThreadManager(void);
    
    /**
     * Spawn the thread
     *
     * @param [out] aptThreadID    the new thread id
     * @param [in]  afFunc         the thread start routine
     * @param [in]  apvArg         the start routine's sole argument
     * @param [in]  aiScope        the type of scheduling contention scope
     * @param [in]  aiGrpID        thread belongs groupid
     * @param [in]  aiStackSize    the stack size
     * @retval 0    If success
     * @retval-1    If fail  
     */
    apl_int_t Spawn(
        apl_thread_t* aptThreadID,
        apl_thr_func_ptr afFunc,
        void* apvArg,
        apl_int_t aiScope = APL_THREAD_USER,
        apl_int_t aiGrpID = -1,
        apl_size_t aiStackSize = 0 );
    
    /**
     * Spawn the thread
     *
     * @param [in]  afFunc         the thread start routine
     * @param [in]  apvArg         the start routine's sole argument
     * @param [in]  aiScope        the type of scheduling contention scope
     * @param [in]  aiGrpID        thread belongs groupid
     * @param [in]  aiStackSize    the stack size
     * @retval 0    If success
     * @retval-1    If fail  
     */
    apl_int_t Spawn(
        apl_thr_func_ptr afFunc,
        void* apvArg,
        apl_int_t aiScope = APL_THREAD_USER,
        apl_int_t aiGrpID = -1,
        apl_size_t aiStackSize = 0 );

    /**
     * Spawn the thread
     *
     * @param [out] aptThreadID[]    the new thread id's array
     * @param [in]  aiN              the size of array
     * @param [in]  afFunc           the thread start routine
     * @param [in]  apvArg           the start routine's sole argument
     * @param [in]  aiScope          the type of scheduling contention scope
     * @param [in]  aiGrpID          thread belongs groupid
     * @param [in]  aiStackSize      the stack size
     * @retval 0    success thread numbers
     * @retval-1    If no success 
     */        
    apl_int_t SpawnN(
        apl_thread_t aptThreadID[],
        apl_size_t aiN,
        apl_thr_func_ptr afFunc,
        void* apvArg,
        apl_int_t aiScope = APL_THREAD_USER,
        apl_int_t aiGrpID = -1,
        apl_size_t aiStackSize = 0 );
 
    /**
     * Spawn the thread
     *
     * @param [in]  aiN              the size of array
     * @param [in]  afFunc           the thread start routine
     * @param [in]  apvArg           the start routine's sole argument
     * @param [in]  aiScope          the type of scheduling contention scope
     * @param [in]  aiGrpID          thread belongs groupid
     * @param [in]  aiStackSize      the stack size
     * @retval 0    success thread numbers
     * @retval-1    If no success
     */       
    apl_int_t SpawnN(
        apl_size_t aiN,
        apl_thr_func_ptr afFunc,
        void* apvArg,
        apl_int_t aiScope = APL_THREAD_USER,
        apl_int_t aiGrpID = -1,
        apl_size_t aiStackSize = 0 );
    
    /**
     * wait the specified thread which finish 
     *
     * @param [in] atThreadID     the specified threadID
     * @param [in] appvStatus     the return of thread
     * @param [in] aoTimeout      set timeout value
     *     -CTimeValue::MAXTIME never timeout
     *     -CTimeValue::ZERO    none-block
     *     ->0                  set the timeout 
     * @retval 0    If success
     * @retval -1   If fail or timeout
     */
    apl_int_t Wait(
        apl_thread_t atThreadID,
        void** appvStatus = NULL,
        CTimeValue const& aoTimeout = CTimeValue::MAXTIME );
 
    /**
     * wait the specified trdGrpID which finish
     *
     * @param [in] aiGrpID       the specified threadGroupID
     * @param [in] aoTimeout     set timeout value
     *     -CTimeValue::MAXTIME never timeout
     *     -CTimeValue::ZERO    none-block
     *     ->0                  set the timeout
     * @retval 0    If success
     * @retval -1   If fail or timeout
     */       
    apl_int_t WaitGrp(
        apl_int_t aiGrpID,
        CTimeValue const& aoTimeout = CTimeValue::MAXTIME );
 
    /**
     * Wait all thread which finish
     *
     * @param [in] aoTimeout    set timeout value
     *     -CTimeValue::MAXTIME never timeout
     *     -CTimeValue::ZERO    none-block
     *     ->0                  set the timeout
     * @retval 0    If success
     * @retval -1   If fail or timeout
     */
    apl_int_t WaitAll(
        CTimeValue const& aoTimeout = CTimeValue::MAXTIME );
    
    /*
    apl_int_t Cancel(apl_thread_t atThreadID);
    
    apl_int_t CancelGrp(apl_int_t aiGrpID);
    
    apl_int_t CancelAll(void);
    */
    
    /**
     * get the runstate of thread
     *
     * @param [in] atThreadID    the specified threadID
     * @return     ERunState.
     */ 
    apl_int_t GetRunState( apl_thread_t atThreadID );
 
    /**
     * get the scope of thread
     *
     * @param [in] atThreadID    the specified threadID
     * @return    APL_THREAD_KERNEL,APL_THREAD_USER.
     */
    apl_int_t GetScope( apl_thread_t atThreadID );
    
    /**
     * get the count of thread
     *
     * @return    >=0.
     */
    apl_size_t GetCount(void);

protected:
    DescIterType Begin(void);
    
    DescIterType End(void);
    
    DescIterType Find( apl_thread_t atThreadID );
    
    apl_int_t Wait( DescIterType aoIter, void** appvStatus, CTimeValue const& aoTimeout );
    
    void Clear(void);
    
    void Erase( DescIterType aoIter );
    
    void Append( CThreadDescriptor* apoDesc );
    
protected:
    static void* ThreadEntry( void* apvParam );
    
private:
    apl_int_t miGrpID;
    
    std::list<CThreadDescriptor*> moDescList;
        
    apl_size_t muListSize;
};

ACL_NAMESPACE_END

#endif //ACL_THREADMANAGER_H
