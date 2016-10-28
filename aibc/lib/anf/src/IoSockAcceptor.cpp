
#include "anf/IoSockAcceptor.h"
#include "anf/IoProcessor.h"
#include "anf/IoPollingProcessor.h"
#include "acl/SockAcceptor.h"
#include "anf/IoFilterChain.h"

ANF_NAMESPACE_START

CIoSockAcceptor::CIoSockAcceptor(void)
{
}

CIoSockAcceptor::~CIoSockAcceptor(void)
{
    this->Dispose();
}

apl_int_t CIoSockAcceptor::Bind0( acl::CSockAddr const& aoAddress )
{
    // Create a bind request as a Future operation. When the selector
    // have handled the registration, it will signal this future.
    CAcceptorOperationFuture::PointerType loRequest(new CAcceptorOperationFuture(aoAddress) );

    // adds the Registration request to the queue for the Workers
    // to handle
    this->moRegisterQueue.Push(loRequest);

    // creates the Acceptor instance and has the local
    // executor kick it off.
    if (this->StartupAcceptor() != 0)
    {
        return -1;
    }
    
    // As we just started the acceptor, we have to unblock the poll()
    // in order to process the bind request we just have added to the 
    // registerQueue.
    this->Wakeup();
    
    // Now, we wait until this request is completed.
    loRequest->Await();

    // Set current thread errno
    if (loRequest->IsException() )
    {
        apl_set_errno(loRequest->GetErrno() );
    }
    
    return loRequest->GetState();
}

apl_int_t CIoSockAcceptor::Unbind0( acl::CSockAddr const& aoAddress )
{
    // Create a unbind request as a Future operation. When the selector
    // have handled the registration, it will signal this future.
    CAcceptorOperationFuture::PointerType loRequest(new CAcceptorOperationFuture(aoAddress) );

    // adds the Registration request to the queue for the Workers
    // to handle
    this->moCancelQueue.Push(loRequest);

    // creates the Acceptor instance and has the local
    // executor kick it off.
    if ( this->StartupAcceptor() != 0)
    {
        return -1;
    }
    
    // As we just started the acceptor, we have to unblock the poll()
    // in order to process the bind request we just have added to the 
    // registerQueue.
    this->Wakeup();
    
    // Now, we wait until this request is completed.
    loRequest->Await();
    
    // Set current thread errno
    if (loRequest->IsException() )
    {
        apl_set_errno(loRequest->GetErrno() );
    }
    
    return loRequest->GetState();
}

apl_int_t CIoSockAcceptor::Dispose(void)
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

apl_int_t CIoSockAcceptor::StartupAcceptor(void)
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
            if (this->moPoll.Initialize(this->GetConfiguration()->GetMaxListenerCount() ) != 0)
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

void CIoSockAcceptor::Wakeup(void)
{
    this->moPoll.Wakeup();
}

apl_ssize_t CIoSockAcceptor::RegisterHandles(void)
{
    apl_ssize_t liHandles = 0;
    CAcceptorOperationFuture::PointerType loFuture;
    
    while(true)
    {
        if (!this->moRegisterQueue.Pop(loFuture) )
        {
            break;
        }
        
        acl::CSockAcceptor loAcceptor;
        
        //Bind local address, and startup service
        if (loAcceptor.Open(loFuture->GetLocalAddress() ) != 0)
        {
            //ERROR acceptor bind operation future
            loFuture->Exception(E_SOCK_BIND, apl_get_errno() );
        }
        else if (this->moPoll.AddHandle(loAcceptor.GetHandle(), APL_POLLIN) != 0)
        {
            //ERROR poll operation future
            loFuture->Exception(E_POLL_ADD, apl_get_errno() );
        }
        else
        {
            this->moBindingAddresses[loAcceptor.GetHandle()].Set(loFuture->GetLocalAddress() );
            loFuture->Done();
        }
        
        loAcceptor.SetHandle(ACL_INVALID_HANDLE);
        
        liHandles++;
    }
    
    return liHandles;
}
    
apl_ssize_t CIoSockAcceptor::ProcessHandles(void)
{
    apl_int_t          liRetCode = 0;
    apl_ssize_t        liHandles = 0;
    acl::CSockAcceptor loAcceptor;
    acl::CSockStream   loPeer;
    acl::CSockAddr     loLocalAddress;
    acl::CSockAddr     loRemoteAddress;
        
    for(acl::CPoll::IteratorType loHandle = this->moPoll.Begin(); 
        loHandle != this->moPoll.End(); ++loHandle)
    {
        if (this->moPoll.IsWakeupHandle(loHandle.Handle() ) )
        {
            this->moPoll.FinishWakeup();
            continue;
        }

        loAcceptor.SetHandle(loHandle.Handle() );
        
        if (loAcceptor.Accept(loPeer, &loRemoteAddress) != 0)
        {
            //ERROR service exception
            this->GetHandler()->Exception(CThrowable(E_SOCK_ACCEPT, apl_get_errno(), "sock accept fail") ) ;
        }
        else
        {
            SessionPtrType loSession(this->NewSession() );
            
            //Get local address
            loPeer.GetLocalAddr(loLocalAddress);

            loSession->SetHandle(loPeer.GetHandle() );
            loSession->SetAddress(loLocalAddress, loRemoteAddress);
            
            this->InitSession(loSession);

            if ( (liRetCode = this->GetProcessor()->Add(loSession) ) != 0)
            {
                //ERROR processor exception
                this->GetHandler()->Exception(CThrowable(liRetCode, apl_get_errno(), "processor add fail") ) ;
                
                this->DestroySession(loSession);
            }
        }
        
        loAcceptor.SetHandle(ACL_INVALID_HANDLE);
        
        liHandles++;
    }
    
    return liHandles;
}
    
apl_ssize_t CIoSockAcceptor::UnregisterHandles(void)
{
    apl_ssize_t liHandles = 0;
    CAcceptorOperationFuture::PointerType loFuture;
    
    while(true)
    {
        if (!this->moCancelQueue.Pop(loFuture) )
        {
            break;
        }
        
        for (BindingAddressMapType::iterator loIter = this->moBindingAddresses.begin(); 
            loIter != this->moBindingAddresses.end(); ++loIter)
        {
            if (loIter->second.GetPort() == loFuture->GetLocalAddress().GetPort() 
                && strcmp(loIter->second.GetIpAddr(), loFuture->GetLocalAddress().GetIpAddr() ) == 0 )
            {
                this->moPoll.DelHandle(loIter->first);
                
                apl_close(loIter->first);
                
                this->moBindingAddresses.erase(loIter);
                
                liHandles++;
                
                break;
            }
        }
    }
    
    return liHandles;
}

void* CIoSockAcceptor::Run(void* apvParam )
{
    CIoSockAcceptor* lpoAcceptor = static_cast<CIoSockAcceptor*>(apvParam);
    acl::CTimeValue  loTimeout(1);
    apl_int_t        liHandles = 0;

    while(lpoAcceptor->IsActivated() )
    {
        apl_ssize_t liSelected = lpoAcceptor->moPoll.Poll(loTimeout);

        liHandles += lpoAcceptor->RegisterHandles();
        
        if (liSelected > 0)
        {
            // We have some connection request, let's process 
            // them here. 
            lpoAcceptor->ProcessHandles();
        }
        else if (liSelected < 0)
        {
            lpoAcceptor->GetHandler()->Exception(
                CThrowable(E_POLL_SELECT, apl_get_errno(), "Acceptor poll select fail") ) ;
        }
        
        liHandles -= lpoAcceptor->UnregisterHandles();
    }

    // Unbind all local address
    for (BindingAddressMapType::iterator loIter = lpoAcceptor->moBindingAddresses.begin(); 
        loIter != lpoAcceptor->moBindingAddresses.end(); ++loIter)
    {
        lpoAcceptor->Unbind(loIter->second);
    }    
    
    liHandles -= lpoAcceptor->UnregisterHandles();
    
    return NULL;
}

inline CIoSession* CIoSockAcceptor::NewSession(void)
{
    CIoSession* lpoSession = NULL;
    
    ACL_NEW_ASSERT(lpoSession, CIoSession);
    
    return lpoSession;
}

ANF_NAMESPACE_END
