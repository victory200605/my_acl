
#include "acl/Reactor.h"

ACL_NAMESPACE_START

//////////////////////////////////////////////  Handle Repository ///////////////////////////////////////////
void CHandleRepository::Bind( apl_int_t aiHandle, IEventHandler* apoEvent, apl_int_t aiMask )
{
    CEventNode& loEventNode = this->moHandlerRep[aiHandle];
    
    loEventNode.mpoEvent = apoEvent;
    loEventNode.miMask   = aiMask;
    loEventNode.mbIsSuspended = false;
}

void CHandleRepository::Unbind( apl_int_t aiHandle )
{
    this->moHandlerRep.erase(aiHandle);
}
    
CHandleRepository::NodeType* CHandleRepository::Find( apl_int_t aiHandle )
{
    std::map<apl_int_t, CEventNode>::iterator loIter = this->moHandlerRep.find(aiHandle);
        
    if ( loIter != this->moHandlerRep.end() )
    {
        return &loIter->second;
    }
    else
    {
        return NULL;
    }
}

apl_size_t CHandleRepository::Size(void)
{
    return this->moHandlerRep.size();
}

//////////////////////////////////////////////////////////////////////////////
CReactor::CReactor(void)
    : miDeactivated(1)
    , mpoTimerQueue(NULL)
{
}
    
CReactor::~CReactor()
{
    this->Close();
}
    
apl_int_t CReactor::Initialize( apl_size_t auSize, CTimerQueue* apoTimerQueue )
{
    apl_int_t liResult = 0;
    
    this->miDeactivated = 1;
    this->mpoTimerQueue = apoTimerQueue;
    
    if (this->mpoTimerQueue == NULL)
    {
        //Default
        ACL_NEW_ASSERT(this->mpoTimerQueue, CTimerQueue(DEFAULT_TIMER_SIZE) );
    }
    
    liResult = this->moPollee.Initialize(auSize);
    
    if (liResult == 0)
    {
        this->miDeactivated = 0;
    }
    
    return liResult;
}

apl_int_t CReactor::Close( void )
{
    this->miDeactivated = 1;
    
    ACL_DELETE(this->mpoTimerQueue);
    
    return this->moPollee.Close();
}

apl_int_t CReactor::RunEventLoop(void)
{
    CTimeValue loTimeout(1);
        
    while (true)
    {
        apl_int_t liResult = this->HandleEvent(loTimeout);

        if (liResult == -1 && this->miDeactivated)
        {
            this->miDeactivated = 0;
            return 0;
        }
        else if (liResult == -1)
        {
            return -1;
        }
    }
    
    this->miDeactivated = 0;
}

apl_ssize_t CReactor::HandleEvent( CTimeValue const& aoTimeout )
{
    apl_int_t liResult = this->WorkPending( aoTimeout );
    if (liResult <= 0)
    {
        return liResult;
    }

    return this->Dispatch();
}

apl_ssize_t CReactor::Dispatch(void)
{
    apl_int_t liResult = 0;
    
    // Handle timer early since they may have higher latency
    // constraints than I/O handlers.  Ideally, the order of
    // dispatching should be a strategy...
    if ( ( liResult = this->DispatchTimerEvent() ) != 0 )
    {
        return liResult;
    }
    
    return this->DispatchIOEvent();
}

apl_ssize_t CReactor::DispatchTimerEvent(void)
{
    if (this->mpoTimerQueue == NULL || this->mpoTimerQueue->IsEmpty() )
    {
        // Empty timer queue so cannot have any expired timers.
        return 0;
    }
    
    // Get the current time
    CTimestamp loCurTime;
    loCurTime.Update(CTimestamp::PRC_USEC);
    loCurTime += this->mpoTimerQueue->TimerSkew();

    // Look for a node in the timer queue whose timer <= the present
    // time.
    CTimerQueue::DispatchInfoType loInfo;
    if (this->mpoTimerQueue->DispatchInfo(loCurTime, loInfo))
    {
        this->mpoTimerQueue->Upcall(loInfo, loCurTime);

        // We have dispatched a timer
        return 1;
    }

    return 0;
}

apl_ssize_t CReactor::DispatchIOEvent(void)
{
    if( this->moFirst != this->moLast )
    {
        apl_int_t& liRevents = this->moFirst.Event();
        apl_int_t  liHandle  = this->moFirst.Handle();
    
        // Figure out what to do first in order to make it easier to manage
        // the bit twiddling and possible pfds increment before releasing
        // the token for dispatch.
        // Note that if there's an error (such as the handle was closed
        // without being removed from the event set) the EPOLLHUP and/or
        // EPOLLERR bits will be set in revents.
        bool lbIsDispOut = false;
        bool lbIsDispExc = false;
        bool lbIsDispIn  = false;
        
        if ( ACL_BIT_ENABLED( liRevents, APL_POLLOUT ) )
        {
            lbIsDispOut = true;
            ACL_CLR_BITS( liRevents, APL_POLLOUT );
        }
        else if ( ACL_BIT_ENABLED( liRevents, APL_POLLPRI ) )
        {
            lbIsDispExc = true;
            ACL_CLR_BITS( liRevents, APL_POLLPRI );
        }
        else if ( ACL_BIT_ENABLED( liRevents, APL_POLLIN ) )
        {
            lbIsDispIn = true;
            ACL_CLR_BITS( liRevents, APL_POLLIN );
        }
        else if ( ACL_BIT_ENABLED( liRevents, APL_POLLERR ) )
        {
            this->RemoveHandler( liHandle, IEventHandler::ALL_EVENTS_MASK );
            ++this->moFirst;
            return 1;
        }
        else
        {
            //Unkown event liRevents;
            apl_errprintf("DispatchIO h %d unknown events 0x%x\n", liHandle, liRevents);
            //assert(false);
        }
        
        // Increment the poapl_int_ter to the next element before we
        // release the token.  Otherwise event handlers end up being
        // dispatched multiple times for the same poll.
        if ( liRevents == 0 )
        {
            ++this->moFirst;
        }
        
        CHandleRepository::NodeType* lpoNode = this->moHandlerRep.Find( liHandle );
    
        if ( lpoNode != NULL && lpoNode->GetEventHandler() != NULL )
        {
            IEventHandler* lpoEh = lpoNode->GetEventHandler();
            
            // Dispatch the detected event
            if (lbIsDispOut)
            {
                apl_int_t liStatus = this->Upcall( lpoEh, &IEventHandler::HandleOutput, liHandle );
            
                if ( liStatus < 0 )
                {
                    // Note that the token is reacquired in remove_handler().
                    this->RemoveHandler( liHandle, IEventHandler::WRITE_MASK );
                }
                
                return 1;
            }
            
            if (lbIsDispExc)
            {
                apl_int_t liStatus = this->Upcall( lpoEh, &IEventHandler::HandleException, liHandle );
            
                if ( liStatus < 0 )
                {
                    // Note that the token is reacquired in remove_handler().
                    this->RemoveHandler( liHandle, IEventHandler::EXCEPT_MASK );
                }
                
                return 1;
            }
            
            if (lbIsDispIn)
            {
                apl_int_t liStatus = this->Upcall( lpoEh, &IEventHandler::HandleInput, liHandle );
            
                if ( liStatus < 0 )
                {
                    // Note that the token is reacquired in remove_handler().
                    this->RemoveHandler( liHandle, IEventHandler::READ_MASK );
                }
                
                return 1;
            }
        }
    }
      
    return 0;
}

apl_int_t CReactor::Upcall(
    IEventHandler* apoEventHandler,
    apl_int_t ( IEventHandler::*callback )( apl_int_t ),
    apl_int_t aiHandle )
{
    // If the handler returns positive value (requesting a reactor
    // callback) just call back as many times as the handler requests
    // it.  Other threads are off handling other things.
    apl_int_t liStatus = 0;

    do
    {
        liStatus = ( apoEventHandler->*callback )( aiHandle );
    }
    while( liStatus > 0);

    return liStatus;
}

apl_ssize_t CReactor::WorkPending( CTimeValue const& aoTimeout )
{
    apl_int_t liResult = 0;
    apl_int_t liTimersPending = 0;
    
    if (this->miDeactivated == 1)
    {
        return -1;
    }
    
    if (this->moFirst != this->moLast)
    {
        return 1;
    }
    
    if (this->mpoTimerQueue != NULL)
    {
        CTimeValue  loTimerBuf(0);
        CTimeValue* lpoThisTimeout = 0;
    
        lpoThisTimeout = this->mpoTimerQueue->CalculateTimeout(aoTimeout, &loTimerBuf);
    
        // Check if we have timers to fire.
        liTimersPending = ( (lpoThisTimeout != 0 && *lpoThisTimeout != aoTimeout) ? 1 : 0);
    }
    
    liResult = this->moPollee.Poll(aoTimeout);
    
    this->moFirst = this->moPollee.Begin();
    this->moLast  = this->moPollee.End();

    return (liResult == 0 && liTimersPending != 0 ? 1 : liResult);
}

apl_int_t CReactor::ScheduleTimer(
    IEventHandler* apoEventHandler,
    void const* apvAct,
    CTimestamp const& aoFutureTime,
    CTimeValue const& aoInterval )
{
    if (this->mpoTimerQueue == NULL)
    {
        return -1;
    }
    
    apl_int_t liResult = this->mpoTimerQueue->Schedule(apoEventHandler, apvAct, aoFutureTime, aoInterval);
    
    return liResult;
}

apl_int_t CReactor::ScheduleTimer(
    IEventHandler* apoEventHandler,
    void const* apvAct,
    CTimeValue const& aoAfter,
    CTimeValue const& aoInterval )
{
    CTimestamp loFutureTime;
    
    loFutureTime.Update(CTimestamp::PRC_USEC);
    loFutureTime += aoAfter;
    
    return this->ScheduleTimer( apoEventHandler, apvAct, loFutureTime, aoInterval );
}

apl_int_t CReactor::CancelTimer( apl_int_t aiTimerID, void const** appvAct )
{
    if (this->mpoTimerQueue == NULL)
    {
        return -1;
    }

    apl_int_t liResult = this->mpoTimerQueue->Cancel(aiTimerID, appvAct, false);
    
    return liResult;
}
    
apl_int_t CReactor::RegisterHandler( apl_int_t aiHandle, IEventHandler* apoEvent, apl_int_t aiMask )
{
    CHandleRepository::NodeType* lpoNode = this->moHandlerRep.Find(aiHandle);
    apl_int_t liEvents = 0;
    
    if ( aiHandle == ACL_INVALID_HANDLE
        || aiMask == IEventHandler::NULL_MASK )
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    if ( lpoNode == NULL )
    {
        if ( this->moHandlerRep.Size() >= this->moPollee.GetCapacity() )
        {
            apl_set_errno(APL_ENOSPC);
            return -1;
        }
        
        liEvents = this->ReactorMaskToPollEvent( aiMask );
        
        if ( this->AddMaskOpts( aiHandle, liEvents ) != 0 )
        {
            return -1;
        }
        
        this->moHandlerRep.Bind( aiHandle, apoEvent, aiMask );
    }
    else
    {
        apl_int_t liNewMask = lpoNode->GetMask();
        
        if ( ACL_BIT_ENABLED( liNewMask, aiMask ) )
        {
            return 0;
        }
        
        ACL_SET_BITS( liNewMask, aiMask );
        
        liEvents = this->ReactorMaskToPollEvent( liNewMask );
        
        if ( this->UpdateMaskOpts( aiHandle, liEvents ) != 0 )
        {
            return -1;
        }
        
        lpoNode->SetMask(liNewMask);
    }

    return 0;
}
    
apl_int_t CReactor::RemoveHandler( apl_int_t aiHandle, apl_int_t aiMask )
{
    CHandleRepository::NodeType* lpoNode = this->moHandlerRep.Find(aiHandle);
    apl_int_t liOldMask = lpoNode != NULL ? lpoNode->GetMask() : 0;
    apl_int_t liNewMask = liOldMask;
    apl_int_t liEvent   = 0;

    ACL_CLR_BITS( liNewMask, aiMask );
    
    liEvent = this->ReactorMaskToPollEvent(liNewMask);
    
    if ( lpoNode == NULL || lpoNode->GetEventHandler() == NULL )
    {
        return -1;
    }
    
    if ( !lpoNode->IsSuspended() && this->UpdateMaskOpts( aiHandle, liEvent ) != 0 )
    {
        return -1;
    }

    if ( ACL_BIT_DISABLED( liNewMask, IEventHandler::DONT_CALL ) )
    {
        lpoNode->GetEventHandler()->HandleClose( aiHandle, aiMask );
    }

    // If there are no longer any outstanding events on the given handle
    // then remove it from the handler repository.
    if ( liNewMask == IEventHandler::NULL_MASK )
    {
        this->moHandlerRep.Unbind( aiHandle );
    }
    else
    {
        lpoNode->SetMask(liNewMask );
    }

    return 0; 
}
    
apl_int_t CReactor::SuspendHandler( apl_int_t aiHandle )
{
    apl_int_t liRetCode = 0;
    CHandleRepository::NodeType* lpoNode = this->moHandlerRep.Find(aiHandle);
    
    if ( lpoNode == NULL || lpoNode->IsSuspended() )
    {
        return -1;
    }
    
    liRetCode = this->UpdateMaskOpts(aiHandle, 0);
    if (liRetCode == 0)
    {
        lpoNode->SetSuspended(true);
    }
    
    return liRetCode;
}

apl_int_t CReactor::ResumeHandler( apl_int_t aiHandle )
{
    apl_int_t liRetCode = 0;
    apl_int_t liMask  = 0;
    apl_int_t liEvent = 0;
    CHandleRepository::NodeType* lpoNode = this->moHandlerRep.Find(aiHandle);
    
    if ( lpoNode == NULL || !lpoNode->IsSuspended() )
    {
        return -1;
    }
    
    liMask  = lpoNode->GetMask();
    liEvent = this->ReactorMaskToPollEvent(liMask);
    
    if ( liMask == IEventHandler::NULL_MASK )
    {
        return -1;
    }
    
    liRetCode = this->AddMaskOpts(aiHandle, liEvent);
    if (liRetCode == 0)
    {
        lpoNode->SetSuspended(false);
    }

    return liRetCode;
}

apl_int_t CReactor::UpdateMaskOpts( apl_int_t aiHandle, apl_int_t aiEvent )
{
    apl_int_t liRetCode = this->moPollee.DelHandle(aiHandle);
    if (liRetCode == 0 && aiEvent != 0)
    {
        liRetCode = this->moPollee.AddHandle(aiHandle, aiEvent);
    }

    return liRetCode;
}

apl_int_t CReactor::AddMaskOpts( apl_int_t aiHandle, apl_int_t aiEvent )
{
    return this->moPollee.AddHandle(aiHandle, aiEvent);
}

apl_int_t CReactor::ReactorMaskToPollEvent( apl_int_t aiMask )
{
    if ( aiMask == IEventHandler::NULL_MASK )
    {
        // No event.  Remove from apl_int_terest set.
        return 0;
    }
    
    apl_int_t liEvents = 0;

    // READ, ACCEPT, and CONNECT flag will place the handle in the
    // read set.
    if ( ACL_BIT_ENABLED( aiMask, IEventHandler::READ_MASK)
        || ACL_BIT_ENABLED( aiMask, IEventHandler::ACCEPT_MASK)
        || ACL_BIT_ENABLED( aiMask, IEventHandler::CONNECT_MASK) )
    {
        ACL_SET_BITS( liEvents, APL_POLLIN );
    }

    // WRITE and CONNECT flag will place the handle in the write set.
    if ( ACL_BIT_ENABLED( aiMask, IEventHandler::WRITE_MASK)
        || ACL_BIT_ENABLED( aiMask, IEventHandler::CONNECT_MASK) )
    {
        ACL_SET_BITS( liEvents, APL_POLLOUT );
    }

    // EXCEPT flag will place the handle in the except set.
    if ( ACL_BIT_ENABLED( aiMask, IEventHandler::EXCEPT_MASK) )
    {
        ACL_SET_BITS( liEvents, APL_POLLPRI );
    }

    return liEvents;
}

void CReactor::Deactivate(void)
{
    this->miDeactivated = 1;
}

ACL_NAMESPACE_END
