
#include "anf/IoSockConnector.h"
#include "anf/IoProcessor.h"
#include "acl/SockConnector.h"
#include "anf/IoFilterChain.h"

ANF_NAMESPACE_START

CIoSockConnector::CIoSockConnector(void)
{
}

CIoSockConnector::~CIoSockConnector(void)
{
    this->Dispose();
}

CConnectFuture::PointerType CIoSockConnector::Connect0( acl::CSockAddr const& aoAddress )
{
    // Create a connection request as a Future operation. When the selector
    // have handled the register, it will signal this future.
    CConnectFuture::PointerType loRequest(new CConnectFuture(aoAddress) );

    // adds the connection request to the queue for the Workers
    // to handle
    this->moRegisterQueue.Push(loRequest);

    // creates the Connector instance and has the local
    // executor kick it off.
    if (this->StartupConnector() != 0)
    {
        loRequest->Exception(-1, apl_get_errno() );
        return loRequest;
    }
    
    // As we just started the connector, we have to unblock the poll()
    // in order to process the bind request we just have added to the 
    // registerQueue.
    this->Wakeup();
    
    return loRequest;
}

apl_int_t CIoSockConnector::Dispose(void)
{
    {
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
        this->SetDisposed(true);
        this->SetActivated(false);
    }
    
    this->Wakeup();

    this->moThreadManager.WaitAll();  
    
    if (!this->IsSharedProcessor() && this->GetProcessor() != NULL)
    {
        this->GetProcessor()->Dispose();
    }
    
    return 0;
}

apl_int_t CIoSockConnector::StartupConnector(void)
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    if (this->IsDisposed() )
    {
        apl_set_errno(APL_ECANCELED);

        return -1;
    }
    
    if (!this->IsActivated() )
    {
        do
        {
            this->DefaultProcessor();
            
            //Initailize poll
            if (this->moPoll.Initialize(
                this->GetConfiguration()->GetMaxProgressConnectionCount() ) != 0)
            {
                break;
            }

            this->SetActivated(true);
            this->UpdateActivationTime();
            
            //Startup schedule-thread
            if (this->moThreadManager.Spawn(Run, this) != 0)
            {
                break;
            }
            
            //Successful
            return 0;
        }
        while(false);
        
        this->moPoll.Close();
        this->SetActivated(false);
        
        return -1;
    }
    else
    {
        return 0;
    }
}

void CIoSockConnector::Wakeup(void)
{
    this->moPoll.Wakeup();
}

apl_ssize_t CIoSockConnector::RegisterNew(void)
{
    apl_ssize_t liHandles = 0;
    apl_ssize_t liCount = this->moRegisterQueue.Size();
    if (liCount > 0)
    { 
        CConnectFuture::PointerType loFuture;
        
        do
        {
            if (!this->moRegisterQueue.Pop(loFuture) )
            {
                break;
            }
            
            acl::CSockConnector loConnector;
            acl::CSockStream    loPeer;
            
            //Connect to remote address
            apl_int_t liRetCode = loConnector.Connect(loPeer, loFuture->GetRemoteAddress() );
            if (liRetCode == 0)
            {
                this->FinishConnect(loFuture, loPeer.GetHandle() );
            }
            else if (liRetCode != 0 && apl_get_errno() != APL_EINPROGRESS)
            {
                //ERROR signal acceptor operation future
                loFuture->Exception(E_SOCK_CONNECT, apl_get_errno() );
            }
            else
            {
                CProgressNode& loNode = this->moProgressHandles[loPeer.GetHandle()];
                loNode.moFuture = loFuture;
                loNode.moTimestamp.Update();
                
                this->moPoll.AddHandle(loPeer.GetHandle(), APL_POLLOUT);
            }
            
            loPeer.SetHandle(ACL_INVALID_HANDLE);
            
            liHandles++;
        }
        while(--liCount > 0);//end do{...} while
    }
    
    return liHandles;
}
    
apl_ssize_t CIoSockConnector::ProcessConnections(void)
{
    acl::CSockHandle loHandle;
    apl_size_t liHandles = 0;
    acl::CPoll::IteratorType loFirst = this->moPoll.Begin();
    acl::CPoll::IteratorType loLast  = this->moPoll.End();
        
    for(; loFirst != loLast; ++loFirst)
    {
        apl_int_t& liRevents = loFirst.Event();
        apl_int_t  liHandle  = loFirst.Handle();
        
        //Skip wakeup handle
        if (this->moPoll.IsWakeupHandle(liHandle) )
        {
            this->moPoll.FinishWakeup();
            
            continue;
        }
        
        //Connection complete and remove handle from poll
        this->moPoll.DelHandle(liHandle);
        
        ProgressHandleMapType::iterator loIter = this->moProgressHandles.find(liHandle);
        if (loIter == this->moProgressHandles.end() )
        {
            //Exception try to ignore it
            continue;
        }
        
        loHandle.SetHandle(liHandle);
        
        apl_int_t liSockError = 0;
        if ( loHandle.GetOption(acl::CSockHandle::OPT_ERROR, &liSockError) == 0
             && ACL_BIT_ENABLED(liRevents, APL_POLLOUT)
             && liSockError == 0 )
        {
            this->FinishConnect(loIter->second.moFuture, liHandle);
        }
        else
        {
            //ERROR Exception
            loIter->second.moFuture->Exception(E_SOCK_CONNECT, liSockError);

            //close socket handle
            loHandle.Close();
        }
        
        this->moProgressHandles.erase(loIter);
        
        liHandles++;
    }
    
    return liHandles;
}
    
void CIoSockConnector::FinishConnect( CConnectFuture::PointerType& aoFuture, apl_handle_t aiHandle )
{
    apl_int_t        liRetCode  = 0;
    acl::CSockHandle loSockHandle;
    acl::CSockAddr   loLocalAddress;
    acl::CSockAddr   loRemoteAddress;
    SessionPtrType   loSession(this->NewSession() );
    
    //Get sock local and remote address
    loSockHandle.SetHandle(aiHandle);
    loSockHandle.GetLocalAddr(loLocalAddress);
    loSockHandle.GetRemoteAddr(loRemoteAddress);

    //init session
    loSession->SetHandle(aiHandle);
    loSession->SetAddress(loLocalAddress, loRemoteAddress);
    this->InitSession(loSession);

    if ( (liRetCode = this->GetProcessor()->Add(loSession) ) != 0)
    {
        //ERROR processor exception
        this->GetHandler()->Exception(CThrowable(liRetCode, apl_get_errno(), "processor add fail") ) ;
        
        this->DestroySession(loSession);

        aoFuture->Exception(liRetCode, apl_get_errno() );
    }
    else
    {
        aoFuture->SetSession(loSession);
        aoFuture->Done();
    }
}

apl_ssize_t CIoSockConnector::ProcessTimedOutSessions( acl::CTimeValue const& aoTimeout )
{
    apl_size_t liHandles = 0;
    acl::CTimestamp loCurrTime;
    
    loCurrTime.Update();
    
    for (ProgressHandleMapType::iterator loIter = this->moProgressHandles.begin();
        loIter != this->moProgressHandles.end(); )
    {
        if (loCurrTime.Sec() - loIter->second.moTimestamp.Sec() >= aoTimeout.Sec() )
        {
            //timeout
            apl_handle_t liHandle = loIter->first;

            this->moPoll.DelHandle(liHandle);
            loIter->second.moFuture->Exception(E_TIMEOUT, -1);
            apl_close(liHandle);

            this->moProgressHandles.erase(loIter++);
            
            liHandles++;
        }
        else
        {
            ++loIter;
        }
        
        //break;
    }
    
    return liHandles;
}

void* CIoSockConnector::Run(void* apvParam )
{
    CIoSockConnector* lpoConnector = static_cast<CIoSockConnector*>(apvParam);
    apl_int_t         liHandles = 0;
    acl::CTimeValue   loTimeout(1);

    while(lpoConnector->IsActivated() )
    {
        apl_ssize_t liSelected = lpoConnector->moPoll.Poll(loTimeout);
        
        liHandles += lpoConnector->RegisterNew();
        
        if (liSelected > 0)
        {
            // We have some connection request, let's process 
            // them here. 
            lpoConnector->ProcessConnections();
        }
        else if (liSelected < 0)
        {
            lpoConnector->GetHandler()->Exception(
                CThrowable(E_POLL_SELECT, apl_get_errno(), "Connector poll select fail") ) ;
        }
        
        liHandles -= lpoConnector->ProcessTimedOutSessions(lpoConnector->GetConnectTimeout() );
    }
    
    lpoConnector->ProcessTimedOutSessions(acl::CTimeValue::ZERO);
  
    return NULL;
}

inline CIoSession* CIoSockConnector::NewSession(void)
{
    CIoSession* lpoSession = NULL;
    
    ACL_NEW_ASSERT(lpoSession, CIoSession);
    
    return lpoSession;
}

ANF_NAMESPACE_END
