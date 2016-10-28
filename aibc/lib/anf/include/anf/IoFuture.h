
#ifndef ANF_IOFUTURE_H
#define ANF_IOFUTURE_H

#include "acl/TimeValue.h"
#include "acl/Synch.h"
#include "anf/Utility.h"

ANF_NAMESPACE_START

/**
 * Represents the completion of an asynchronous operation.
 */
class CIoFuture
{
public:
    CIoFuture(void);
    
    virtual ~CIoFuture(void);
    
    /**
     * Wait for the asynchronous operation to complete with the specified timeout.
     *
     * @return 0 if the operation is completed. otherwise -1 shall be return
     */
    apl_int_t Await(acl::CTimeValue const& aoTimeout = acl::CTimeValue::MAXTIME);

    /**
     * Returns if the asynchronous operation is completed.
     */
    bool IsDone(void);
    
    /**
     * Returns if the asynchronous operation is canceled.
     */
    bool IsCancelled(void);
    
    /**
     * Returns if the asynchronous operation is caught exception.
     */
    bool IsException(void);
    
    /**
     * Signal asynchronous operation completed
     */
    void Done(void);
   
    /**
     * Signal asynchronous operation exception
     */
    void Exception( apl_int_t aiState, apl_int_t aiErrno );
    
    /**
     * Returns operation state if the asynchronous operation is caught exception.
     */
    apl_int_t GetState(void);
    
    /**
     * Returns operation system errno if the asynchronous operation is caught exception.
     */
    apl_int_t GetErrno(void);

protected:
    /**
     * Signal asynchronous operation cancel, but current version is not support
     */
    void Cancel(void);

    void Set( bool abIsDone, bool abIsCancelled, apl_int_t aiState, apl_int_t aiErrno );

private:
    bool mbIsDone;
    
    bool mbIsCancelled;
    
    apl_int_t miState;
    
    apl_int_t miErrno;
    
    acl::CLock moLock;
        
    acl::CCondition moCond;
};

ANF_NAMESPACE_END

#endif//ANF_IOFUTURE_H
