
#include "anf/IoService.h"
#include "anf/IoSimpleProcessorPool.h"
#include "anf/IoPollingProcessor.h"

ANF_NAMESPACE_START

acl::TNumber<apl_uint64_t, acl::CLock> CIoService::moSessionID(0);

struct CCloseAction
{
    template<class T> void operator () ( T& iter )
    {
        iter->second->Close(iter->second);
    }
};

CIoService::CIoService(void)
    : mbIsDisposed(false)
    , mbIsActivated(false)
{
    CIoFilterChain* lpoFilterChain = APL_NULL;

    ACL_NEW_ASSERT(lpoFilterChain, CIoFilterChain);

    this->moFilterChainPtr.Set(lpoFilterChain, false);
}
    
CIoService::~CIoService(void)
{
    CCloseAction loCloseAction;

    this->moSessions.All(loCloseAction);

    while(this->moSessions.Size() > 0)
    {
        apl_sleep(APL_TIME_MSEC*10);
    }
}

bool CIoService::IsDisposed(void)
{
    return this->mbIsDisposed;
}

bool CIoService::IsActivated(void)
{
    return this->mbIsActivated;
}

acl::CTimestamp CIoService::GetActivationTime(void)
{
    return this->moActivationTime;
}

IoHandler* CIoService::GetHandler(void)
{
    return this->moHandlerPtr.GetPtr();
}

void CIoService::SetHandler( IoHandler* apoHandler )
{
    this->moHandlerPtr.Set(apoHandler, true);
}

void CIoService::GetManagedSessions( std::map<apl_uint64_t, SessionPtrType>& aoSessions )
{
    this->moSessions.Copy(aoSessions);
}

apl_size_t CIoService::GetManagedSessionCount(void)
{
    return this->moSessions.Size();
}

CIoConfiguration* CIoService::GetConfiguration(void)
{
    if (this->moConfigurationPtr.IsEmpty() )
    {
        CIoConfiguration* lpoConfiguration = APL_NULL;
        
        ACL_NEW_ASSERT(lpoConfiguration, CIoConfiguration);

        this->moConfigurationPtr.Set(lpoConfiguration, false);
    }

    return this->moConfigurationPtr.GetPtr();
}

CIoFilterChain* CIoService::GetFilterChain(void)
{
    return this->moFilterChainPtr.GetPtr();
}

IoProcessor* CIoService::GetProcessor(void)
{
    return this->moProcessorPtr.GetPtr();
}

void CIoService::SetProcessor( IoProcessor* apoProcessor )
{
    this->moProcessorPtr.Set(apoProcessor, true);
}

bool CIoService::IsSharedProcessor(void)
{
    return this->moProcessorPtr.IsShared();
}

void CIoService::SetConfiguration( CIoConfiguration* apoConfiguration )
{
    this->moConfigurationPtr.Set(apoConfiguration, true);
}

bool CIoService::FindManagedSession( apl_uint64_t au64SessionID, SessionPtrType& aoSession )
{
    return this->moSessions.Find(au64SessionID, aoSession);
}

void CIoService::InitSession( SessionPtrType& aoSession )
{
    aoSession->SetService(this);
    aoSession->SetHandler(this->GetHandler() );
    aoSession->SetReadBufferSize(this->GetConfiguration()->GetReadBufferSize() );
    aoSession->SetSessionID(++this->moSessionID);
    
    this->GetFilterChain()->CopyTo(aoSession->GetFilterChain() );
    
    this->moSessions.Insert(aoSession->GetSessionID(), aoSession);
}
    
void CIoService::DestroySession( SessionPtrType& aoSession )
{
    apl_close(aoSession->GetHandle() );
    aoSession->SetHandle(ACL_INVALID_HANDLE);
    
    this->moSessions.Erase(aoSession->GetSessionID() );
}

void CIoService::SetDisposed( bool abFlag )
{
    this->mbIsDisposed = abFlag;
}

void CIoService::SetActivated( bool abFlag )
{
    this->mbIsActivated = abFlag;
}

void CIoService::UpdateActivationTime(void)
{
    this->moActivationTime.Update();
}

void CIoService::DefaultProcessor(void)
{
    if (this->GetProcessor() == NULL)
    {
        IoProcessor* lpoProcessor = NULL;
        
        if (this->GetConfiguration()->GetProcessorPoolSize() > 1)
        {
            ACL_NEW_ASSERT(lpoProcessor, 
                 TIoSimpleProcessorPool<CIoPollingProcessor>(this->GetConfiguration()->GetProcessorPoolSize() ) );
        }
        else
        {
            ACL_NEW_ASSERT(lpoProcessor, CIoPollingProcessor);
        }
        
        lpoProcessor->SetConfiguration(this->GetConfiguration() );
        lpoProcessor->SetHandler(this->GetHandler() );
        
        this->moProcessorPtr.Set(lpoProcessor, false);
    }
}

ANF_NAMESPACE_END
