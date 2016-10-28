
#include "anf/WakeablePoll.h"

ANF_NAMESPACE_START

CWakeablePoll::CWakeablePoll(void)
{
}
    
CWakeablePoll::~CWakeablePoll(void)
{
    this->Close();
}
    
apl_int_t CWakeablePoll::Initialize( apl_size_t auCapacity )
{
    do
    {
        if (acl::CPoll::Initialize(auCapacity) != 0)
        {
            break;
        }
        
        //Open sockpair to wakeup schedule-thread
        if (this->moSockPair.Open(APL_AF_UNIX, APL_SOCK_STREAM) != 0)
        {
            break;
        }
                
        //Add wakeup handle
        if (this->AddHandle(this->moSockPair.GetSecond().GetHandle(), APL_POLLIN) != 0)
        {
            break;
        }
        
        this->moSockPair.GetFirst().Enable(acl::CSockHandle::OPT_NONBLOCK);
        this->moSockPair.GetSecond().Enable(acl::CSockHandle::OPT_NONBLOCK);
        
        return 0;
    }
    while(false);
    
    this->Close();
    
    return -1;
}

void CWakeablePoll::Close(void)
{
    this->moSockPair.GetFirst().Close();
    this->moSockPair.GetSecond().Close();
    acl::CPoll::Close();
}

apl_int_t CWakeablePoll::Wakeup(void)
{
    char lacBuffer[1] = {0};

    //send 1 byte signal data to wakeup schedule-thread 
    return apl_send(this->moSockPair.GetFirst().GetHandle(), lacBuffer, 1, 0);
}
    
apl_int_t CWakeablePoll::FinishWakeup(void)
{
    //Clear wakeup signal data
    char lacBuffer[1024];
    
    return apl_recv(this->moSockPair.GetSecond().GetHandle(), lacBuffer, sizeof(lacBuffer), 0);
}

bool CWakeablePoll::IsWakeupHandle( apl_handle_t aiHandle )
{
    return this->moSockPair.GetSecond().GetHandle() == aiHandle;
}

ANF_NAMESPACE_END
