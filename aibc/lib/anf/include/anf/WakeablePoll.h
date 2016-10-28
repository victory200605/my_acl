
#ifndef ANF_WAKEABLEPOLL_H
#define ANF_WAKEABLEPOLL_H

#include "acl/Poll.h"
#include "acl/Synch.h"
#include "acl/SockHandle.h"
#include "anf/Utility.h"

ANF_NAMESPACE_START

/**
 * Wakeable Poll, when {@link #Wakeup()} is invoked, 1 byte data will be send to sockpair to wakeup poll APL_POLLIN event
 */
class CWakeablePoll : public acl::CPoll
{
public:
    CWakeablePoll(void);
    
    ~CWakeablePoll(void);
    
    apl_int_t Initialize( apl_size_t auCapacity );
    
    void Close(void);
    
    apl_int_t Wakeup(void);
    
    apl_int_t FinishWakeup(void);
    
    bool IsWakeupHandle( apl_handle_t aiHandle );
    
private:
    acl::CSockHandlePair moSockPair;
};

ANF_NAMESPACE_END

#endif// ANF_WAKEABLEPOLL_H
