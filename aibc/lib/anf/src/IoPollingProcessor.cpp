
#include "acl/MemoryBlock.h"
#include "acl/Any.h"
#include "anf/IoPollingProcessor.h"
#include "anf/IoFilterChain.h"
#include "anf/IoService.h"

ANF_NAMESPACE_START

CIoPollingProcessor::CIdleNotifier::CIdleNotifier( acl::CTimestamp const& aoCurrTime )
    : moCurrTime(aoCurrTime)
{
}

void CIoPollingProcessor::CIdleNotifier::operator ()( SessionMapType::iterator& aoIter )
{
    aoIter->second->NotifyIdleSession(aoIter->second, this->moCurrTime);
}

CIoPollingProcessor::CSuspendSessionWrite::CSuspendSessionWrite(
    SessionPtrType& aoSession,
    CIoPollingProcessor* apoProcessor )
    : moSession(aoSession)
    , mpoProcessor(apoProcessor)
{
}

void CIoPollingProcessor::CSuspendSessionWrite::operator () (void)
{
    if (this->mpoProcessor->mpoConfiguration->IsAutoSuspendWrite() )
    {
        apl_int32_t liNewEvent = this->moSession->GetEvent();
    
        //Set WriteSuspended = true
        this->SetWriteSuspended(this->moSession, true);

        //Clear POLLOUT event
        ACL_CLR_BITS(liNewEvent, APL_POLLOUT);
    
        //Update socket event
        this->mpoProcessor->UpdateTrafficMaskNow(
            this->moSession,
            this->moSession->GetEvent(),
            liNewEvent );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
CIoPollingProcessor::CIoPollingProcessor(void)
    : mpoHandler(NULL)
    , mpoConfiguration(NULL)
    , mbIsDisposed(false)
    , mbIsActivated(false)
    , moPollTimeout(1)
    , moRecvMessage( (acl::CMemoryBlock*)0)
{
    this->moLastIdleCheckTime.Update(acl::CTimestamp::PRC_USEC);
}
    
CIoPollingProcessor::~CIoPollingProcessor(void)
{
}

void CIoPollingProcessor::SetConfiguration( CIoConfiguration* apoConfiguration )
{
    this->mpoConfiguration = apoConfiguration;
}
    
void CIoPollingProcessor::SetHandler( IoHandler* apoHandler )
{
    this->mpoHandler = apoHandler;
}
    
apl_int_t CIoPollingProcessor::Dispose( bool abIsWait )
{
    {
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);

        this->mbIsDisposed = true;

        //Set active state false
        this->mbIsActivated = false;
    }

    this->Wakeup();

    if (abIsWait)
    {
        //Wait run thread exit
        this->moThreadManager.WaitAll();
    }

    return 0;
}

apl_int_t CIoPollingProcessor::Add( SessionPtrType& aoSession )
{
    //Set processor which provide I/O schedule service to this session
    aoSession->SetProcessor(this);
    
    //Push new session into NewSession Queue and wait for processing
    this->moNewSessions.Push(aoSession);

    //Startup processor schedule thread, maybe it is running
    if ( this->StartupProcessor() != 0)
    {
        return -1;
    }
    
    this->Wakeup();
    
    return 0;
}

apl_int_t CIoPollingProcessor::Flush( SessionPtrType& aoSession )
{
    this->ScheduleFlush(aoSession);
    
    return 0;
}

apl_int_t CIoPollingProcessor::UpdateTrafficControl( SessionPtrType& aoSession )
{
    this->ScheduleTrafficControl(aoSession);
    
    return 0;
}

apl_int_t CIoPollingProcessor::Remove( SessionPtrType& aoSession )
{
    this->ScheduleRemove(aoSession);
    
    return 0;
}

apl_size_t CIoPollingProcessor::GetSize(void)
{
    return this->moSessions.size();
}

apl_int_t CIoPollingProcessor::StartupProcessor(void)
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    if (this->mbIsDisposed)
    {
        apl_set_errno(APL_ECANCELED);

        return -1;
    }

    if (!this->mbIsActivated)
    {
        do
        {
            //Initialize poll
            if (this->moPoll.Initialize(
                this->mpoConfiguration->GetMaxConnectionCount() ) != 0)
            {
                break;
            }
            
            this->mbIsActivated = true;
            
            if (this->moThreadManager.Spawn(Run, this) != 0)
            {
                break;
            }
            
            
            return 0;
        }
        while(false);
        
        this->moPoll.Close();
        this->mbIsActivated = false;
        
        return -1;
    }
    else
    {
        return 0;
    }
}

void CIoPollingProcessor::Wakeup(void)
{
    this->moPoll.Wakeup();
}

apl_ssize_t CIoPollingProcessor::HandleNewSessions(void)
{
    apl_int_t  liAddedSessions = 0;
    
    if (this->moNewSessions.Size() > 0)
    {
        SessionPtrType loSession;
            
        // Loop on the new sessions blocking queue, to count
        // the number of sessions who has been created
        do
        {
            if (!this->moNewSessions.Pop(loSession) )
            {
                // We don't have new sessions
                break;
            }
            
            loSession->Enable(acl::CSockHandle::OPT_NONBLOCK);

            if (this->mpoConfiguration->IsAutoResumeRead() )
            {
                if (this->moPoll.AddHandle(loSession->GetHandle(), APL_POLLIN) == 0)
                {
                    // A new session has been created 
                    this->SetReadSuspended(loSession, false);
                    this->SetEvent(loSession, APL_POLLIN);
                    this->moSessions[loSession->GetHandle()] = loSession;
                    liAddedSessions++;
                }
                else
                {
                    loSession->GetFilterChain()->FireSessionException(
                        loSession, CThrowable(E_POLL_ADD, apl_get_errno(), "poll add handle fail") );
                }
            }
            else
            {
                this->moSessions[loSession->GetHandle()] = loSession;
                liAddedSessions++;
            }

            loSession->GetFilterChain()->FireSessionOpened(loSession);
        }
        while(this->moNewSessions.Size() > 0);
    }

    return liAddedSessions;
}

void CIoPollingProcessor::HandleFlush( acl::CTimestamp const& aoCurrTime )
{ 

    if (this->moFlushingSessions.Size() > 0)
    {
        // Loop on the new sessions blocking queue, to count
        // the number of sessions who has been created
        SessionPtrType loSession;
        
        do
        {
            if (!this->moFlushingSessions.Pop(loSession) )
            {
                break;
            }
            
            this->FlushNow(loSession, aoCurrTime);
        }
        while(this->moFlushingSessions.Size() > 0);
    }
}
        
void CIoPollingProcessor::HandleUpdateTrafficMask(void)
{
    if (this->moTrafficControllingSessions.Size() > 0)
    {
        SessionPtrType loSession;
            
        // Loop on the new sessions blocking queue, to count
        // the number of sessions who has been created
        do
        {
            if (!this->moTrafficControllingSessions.Pop(loSession) )
            {
                // We don't have new sessions
                break;
            }

            apl_int32_t liNewEvent = 0;

            if (!loSession->IsWriteSuspended() )
            {
                ACL_SET_BITS(liNewEvent, APL_POLLOUT);
            }

            if (!loSession->IsReadSuspended() )
            {
                ACL_SET_BITS(liNewEvent, APL_POLLIN);
            }

            this->UpdateTrafficMaskNow(loSession, loSession->GetEvent(), liNewEvent);
        }
        while(this->moTrafficControllingSessions.Size() > 0);
    }
}

void CIoPollingProcessor::HandleRemove(void)
{
    if (this->moRemovingSessions.Size() > 0)
    {
        // Loop on the new sessions blocking queue, to count
        // the number of sessions who has been created
        SessionPtrType loSession;
            
        do
        {
            if (!this->moRemovingSessions.Pop(loSession) )
            {
                // We don't have new sessions
                break;
            }

            this->RemoveNow(loSession);
        }
        while(this->moRemovingSessions.Size() > 0);
    }
}
   
void CIoPollingProcessor::Process( acl::CTimestamp const& aoCurrTime )
{
    acl::CPoll::IteratorType loFirst = this->moPoll.Begin();
    acl::CPoll::IteratorType loLast  = this->moPoll.End();
    
    //Process all polled handle event
    for(; loFirst != loLast; ++loFirst)
    {
        apl_int_t& liRevents = loFirst.Event();
        apl_int_t  liHandle  = loFirst.Handle();
        
        //Wakeup handle will be skipped
        if (this->moPoll.IsWakeupHandle(liHandle) )
        {
            this->moPoll.FinishWakeup();
            
            continue;
        }
        
        SessionMapType::iterator loIter = this->moSessions.find(liHandle);
        if (loIter == this->moSessions.end() )
        {
            //ACL_ASSERT(false);
            //Exception maybe delete handle from poll fail lasttime
            //FIX this Exception here
            this->moPoll.DelHandle(liHandle);

            continue;
        }
        
        SessionPtrType& loSession = loIter->second;
       
        while(liRevents != 0)
        {
            if (ACL_BIT_ENABLED(liRevents, APL_POLLIN|APL_POLLPRI) )
            {
                this->ReadNow(loSession, aoCurrTime);
                ACL_CLR_BITS(liRevents, APL_POLLIN );
            }
            else if (ACL_BIT_ENABLED(liRevents, APL_POLLOUT) )
            {
                this->FlushNow(loSession, aoCurrTime);
                ACL_CLR_BITS(liRevents, APL_POLLOUT );
            }
            else if (ACL_BIT_ENABLED(liRevents, APL_POLLERR|APL_POLLHUP) )
            {
                loSession->Close(loSession);
                break;
            }
            else
            {
                //ERROR
                loSession->GetFilterChain()->FireSessionException(
                    loSession, CThrowable(E_POLL_INVALID_EVENT, liRevents, "poll invalid event fail") );
                
                loSession->Close(loSession);
                
                break;
            }
        }
    }
}

void CIoPollingProcessor::NotifyIdleSessions( acl::CTimestamp const& aoCurrTime )
{
    if (aoCurrTime.Usec() - this->moLastIdleCheckTime.Usec() > this->moPollTimeout.Usec() )
    {
        //Notify all idle timeout sessions
        CIdleNotifier loNotifier(aoCurrTime);
        
        this->moLastIdleCheckTime = aoCurrTime;
        for (SessionMapType::iterator loIter = this->moSessions.begin();
             loIter != this->moSessions.end(); ++loIter)
        {
            loNotifier(loIter);
        }
    }
}

void CIoPollingProcessor::ReadNow( SessionPtrType& aoSession, acl::CTimestamp const& aoCurrTime )
{
    apl_ssize_t liResult = 0;

    acl::CMemoryBlock* lpoBuffer = aoSession->GetReadBuffer();
    
    {//Clean up MemoryBlock
        if (lpoBuffer->GetSize() == 0)
        {
            //Reset MemoryBlock to free all space
            lpoBuffer->Resize(aoSession->GetService()->GetConfiguration()->GetReadBufferSize() );
        }

        apl_size_t luLength   = lpoBuffer->GetLength();
        char*      lpcBasePtr = lpoBuffer->GetBase();
        char*      lpcReadPtr = lpoBuffer->GetReadPtr();
        if (lpcReadPtr > lpcBasePtr)
        {
            apl_memmove(lpcBasePtr, lpcReadPtr, luLength);
            lpoBuffer->Reset();
            lpoBuffer->SetWritePtr(luLength);
        }
        else if (lpoBuffer->GetSpace() == 0)
        {
            //Auto increase buffer space size by double
            lpoBuffer->Resize(lpoBuffer->GetSize() * 2);
        }
    }
    
    liResult = this->Recv(aoSession, lpoBuffer->GetWritePtr(), lpoBuffer->GetSpace() );
    if (liResult <= 0)
    {
        //Connection reset by peer
        aoSession->Close(aoSession);
    }
    else
    {
        lpoBuffer->SetWritePtr(liResult);
        
        acl::CMemoryBlock** lppoTemp = this->moRecvMessage.CastTo<acl::CMemoryBlock*>();
        ACL_ASSERT(lppoTemp!= NULL);

        *lppoTemp = lpoBuffer; 
        
        //Fire a new message received
        aoSession->GetFilterChain()->FireMessageReceived(aoSession, this->moRecvMessage);
        aoSession->IncreaseReadBytes(liResult, aoCurrTime);
    }
}

void CIoPollingProcessor::FlushNow( SessionPtrType& aoSession, acl::CTimestamp const& aoCurrTime )
{
    if (aoSession->GetWriteRequestQueue().Size() == 0)
    {
        //Queue sync action to update suspend state
        CSuspendSessionWrite loSuspendSessionWrite(aoSession, this);
            
        aoSession->GetWriteRequestQueue().IfEmptyDo(loSuspendSessionWrite);
    }
    else
    {
        //Flush all write request in current queue
        CWriteFuture::PointerType lpoWriteFuture;
        
        do
        {
            if (!aoSession->GetWriteRequestQueue().Pop(lpoWriteFuture) )
            {
                break;
            }

            if (aoSession->GetService()->GetConfiguration()->GetMaxQueue() > 0
              && aoSession->GetService()->GetConfiguration()->GetQueueTimeout().Nsec() > 0 )
            {
                aoSession->GetWriteRequestCond().Signal();
            }
 
            apl_int_t liRetCode = this->FlushRequestNow(aoSession, lpoWriteFuture, aoCurrTime);
            if (liRetCode == 1)
            {
                apl_int32_t liNewEvent = aoSession->GetEvent();
            
                //Set WriteSuspended = true
                this->SetWriteSuspended(aoSession, false);

                //Clear POLLOUT event
                ACL_SET_BITS(liNewEvent, APL_POLLOUT);
    
                //Resume write event, and request will be flush next time
                this->UpdateTrafficMaskNow(
                    aoSession,
                    aoSession->GetEvent(),
                    liNewEvent );
                
                break;
            }
            else if (liRetCode < 0)
            {
                //FIXED do for all message
                //break;
            }
        }
        while(aoSession->GetWriteRequestQueue().Size() > 0);
    }
}

apl_int_t CIoPollingProcessor::FlushRequestNow(
    SessionPtrType& aoSession,
    CWriteFuture::PointerType& aoFuture,
    acl::CTimestamp const& aoCurrTime )
{
    apl_ssize_t liResult = 0;
    acl::CMemoryBlock* lpoBlock = NULL;

    if (aoFuture->GetFlushCount() == 0
        && aoCurrTime.Sec() - aoFuture->GetTimestamp().Sec() >= 
            this->mpoConfiguration->GetWriteTimeout().Sec() )
    {
        //ERROR message flush timeout
        aoFuture->Exception(E_TIMEOUT, -1);
        
        aoSession->GetFilterChain()->FireMessageFinished(aoSession, aoFuture);
        
        return 0;
    }
     
    do
    {
        if (!aoFuture->GetMessage().CastTo<acl::CMemoryBlock*>(lpoBlock)
            && (lpoBlock = aoFuture->GetMessage().CastTo<acl::CMemoryBlock>() ) == NULL)
        {
            //ERROR invalid message type
            aoFuture->Exception(E_INVALID_MESSAGE, -1);
            
            aoSession->GetFilterChain()->FireSessionException(
                aoSession, CThrowable(E_INVALID_MESSAGE, -1, "flush invalid message type") );
            
            aoSession->GetFilterChain()->FireMessageFinished(aoSession, aoFuture);
            
            break;
        }
        else
        {
            if (!aoFuture->IsHasRemoteAddress() )
            {
                liResult = this->Send(aoSession, lpoBlock->GetReadPtr(), lpoBlock->GetLength() );
            }
            else
            {
                liResult = this->Send(aoSession, lpoBlock->GetReadPtr(), lpoBlock->GetLength(), aoFuture->GetRemoteAddress() );
            }

            if (liResult < 0)
            {
                apl_int_t liErrno = apl_get_errno();

                if (liErrno != APL_EAGAIN && liErrno != APL_EWOULDBLOCK)
                {
                    aoFuture->Exception(E_SOCK_SEND, apl_get_errno() );
                    
                    if ( liErrno != APL_EPIPE && liErrno != APL_ECONNRESET)
                    {
                        aoSession->GetFilterChain()->FireSessionException(
                            aoSession, CThrowable(E_SOCK_SEND, apl_get_errno(), "sock send fail") );
                    }
                    
                    //Completed and fire message finished
                    aoSession->GetFilterChain()->FireMessageFinished(aoSession, aoFuture);
                    
                    break;
                }
                
                liResult = 0;
            }
            
            lpoBlock->SetReadPtr(liResult);
            aoSession->IncreaseWrittenBytes(liResult, aoCurrTime);
            aoFuture->IncreaseFlushCount(1);

            if (lpoBlock->GetLength() > 0 )
            {
                //Uncompleted request and push back WriteQueue to schedule next time;
                aoSession->GetWriteRequestQueue().PushFront(aoFuture);

                return 1;
            }
            else
            {
                //Completed and fire message finish
                aoFuture->Done();
                aoSession->GetFilterChain()->FireMessageFinished(aoSession, aoFuture);
                
                return 0;
            }
        }
    }
    while(false);
    
    return -1;
}

void CIoPollingProcessor::UpdateTrafficMaskNow( SessionPtrType& aoSession, apl_int_t aiOld, apl_int_t aiNew )
{
    if (aiOld != aiNew)
    {
        //printf("Update traffic old = %d / new = %d\n", aiOld, aiNew);
        this->moPoll.DelHandle(aoSession->GetHandle() );
        
        if (aiNew != 0 && this->moPoll.AddHandle(aoSession->GetHandle(), aiNew) != 0)
        {
            //ERROR remove poll event fail
            aoSession->GetFilterChain()->FireSessionException(
                aoSession, CThrowable(E_POLL_ADD, apl_get_errno(), "poll add handle fail") );
        }
        else
        {
            this->SetEvent(aoSession, aiNew);
        }
    }
}

void CIoPollingProcessor::RemoveNow( SessionPtrType& aoSession )
{
    if (!aoSession->IsClosed() )
    {
        if (this->moPoll.DelHandle(aoSession->GetHandle() ) != 0)
        { 
            if (aoSession->GetEvent() > 0)
            {
                //ERROR remove poll event fail
                aoSession->GetFilterChain()->FireSessionException(
                    aoSession, 
                    CThrowable(E_POLL_DEL, apl_get_errno(), "poll delete handle fail") );
            }
        }

        //Clear request
        this->ClearWriteRequestQueue(aoSession);

        //Clear session event
        this->SetEvent(aoSession, 0);

        //Release session resources
        this->moSessions.erase(aoSession->GetHandle() );
        
        aoSession->GetService()->DestroySession(aoSession);
        
        aoSession->GetFilterChain()->FireSessionClosed(aoSession);
    }
}

void CIoPollingProcessor::ClearWriteRequestQueue(SessionPtrType& aoSession)
{
    if (aoSession->GetWriteRequestQueue().Size() > 0)
    {
        anf::CWriteFuture::PointerType loFuture;

        while(aoSession->GetWriteRequestQueue().Pop(loFuture) )
        {
            //Completed and fire message finished
            if (aoSession->GetService()->GetConfiguration()->GetMaxQueue() > 0)
            {
                aoSession->GetWriteRequestCond().Broadcast();
            }
            loFuture->Exception(E_SOCK_SEND, APL_EPIPE);
            aoSession->GetFilterChain()->FireMessageFinished(aoSession, loFuture);
        }
    }
}

void CIoPollingProcessor::ScheduleRemove(SessionPtrType& aoSession)
{
    if (this->moRemovingSessions.Push(aoSession) == 1)
    {
        this->Wakeup();
    }
}

void CIoPollingProcessor::ScheduleFlush(SessionPtrType& aoSession)
{
    if (this->moFlushingSessions.Push(aoSession) == 1)
    {
        this->Wakeup();
    }
}

void CIoPollingProcessor::ScheduleTrafficControl(SessionPtrType& aoSession)
{
    if (this->moTrafficControllingSessions.Push(aoSession) == 1)
    {
        this->Wakeup();
    }
}
    
void* CIoPollingProcessor::Run( void* apvParam )
{
    CIoPollingProcessor* lpoProcessor = static_cast<CIoPollingProcessor*>(apvParam);
    acl::CTimestamp loCurrTime;
    apl_int_t       liHandles = 0;
    
    while(lpoProcessor->mbIsActivated)
    {
        apl_ssize_t liSelected = lpoProcessor->moPoll.Poll(lpoProcessor->moPollTimeout);
        
        //printf("selectd flush size=%u\n", lpoProcessor->moFlushingSessions.Size() );
        // have some new session register request, let's process them here
        liHandles += lpoProcessor->HandleNewSessions();
        
        loCurrTime.Update(acl::CTimestamp::PRC_USEC);
        
        if (liSelected > 0)
        {
            // have some connection request, let's process 
            // them here. 
            lpoProcessor->Process(loCurrTime);
        }
        else if (liSelected < 0)
        {
            if (lpoProcessor->mpoHandler != APL_NULL)
            {
                lpoProcessor->mpoHandler->Exception(
                    CThrowable(E_POLL_SELECT, apl_get_errno(), "Processor poll select fail") );
            }
            else
            {
                apl_errprintf("Anf polling processor poll event fail,%s\n", apl_get_errno() );
            }
        }
        
        // have some update traffic request, let's process them here
        lpoProcessor->HandleUpdateTrafficMask();
        
        // have some flush request, let's process them here
        lpoProcessor->HandleFlush(loCurrTime);
        
        // have some remove session request, let's process them here
        lpoProcessor->HandleRemove();
        
        // Notify idle sessions
        lpoProcessor->NotifyIdleSessions(loCurrTime);
    }

    //release all session
    for (SessionMapType::iterator loIter = lpoProcessor->moSessions.begin();
         loIter != lpoProcessor->moSessions.end(); ++loIter )
    {
        lpoProcessor->ScheduleRemove(loIter->second);
    }
    //remove all
    lpoProcessor->HandleRemove();
    
    return NULL;
}

ANF_NAMESPACE_END
