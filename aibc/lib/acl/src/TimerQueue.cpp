
#include "acl/TimerQueue.h"

ACL_NAMESPACE_START

CTimerNode::CTimerNode(void)
    : mpoEventHandler(NULL)
    , mpvAct(NULL)
    , moTimerValue(0)
    , moInterval(0)
    , miTimerID(-1)
    , miHeapSlot(-1)
{
}

CTimerNode::~CTimerNode(void)
{
}

void CTimerNode::Set( 
    IEventHandler* apoEventHandler,
    void const* apvAct,
    CTimestamp const& aoTimerValue,
    CTimeValue const& aoInterval )
{
    this->mpoEventHandler = apoEventHandler;
    this->mpvAct = apvAct;
    this->moTimerValue = aoTimerValue;
    this->moInterval = aoInterval;
}

/// Get the type.
IEventHandler* CTimerNode::GetEventHandler(void)
{
    return this->mpoEventHandler;
}

/// Set the type.
void CTimerNode::SetEventHandler( IEventHandler* apoEventHandler )
{
    this->mpoEventHandler = apoEventHandler;
}

/// Get the asynchronous completion token.
void const* CTimerNode::GetAct(void)
{
    return this->mpvAct;
}

/// Set the asynchronous completion token.
void CTimerNode::SetAct( void* apvAct )
{
    this->mpvAct = apvAct;
}

/// Get the timer value.
CTimestamp const& CTimerNode::GetTimerValue(void) const
{
    return this->moTimerValue;
}

/// Set the timer value.
void CTimerNode::SetTimerValue( CTimestamp const& aoTimerValue )
{
    this->moTimerValue = aoTimerValue;
}

/// Get the timer interval.
CTimeValue const& CTimerNode::GetInterval(void) const
{
    return this->moInterval;
}

/// Set the timer interval.
void CTimerNode::SetInterval( CTimeValue const& aoInterval )
{
    this->moInterval = aoInterval;
}

/// Get the timer_id.
apl_int_t CTimerNode::GetTimerID(void) const
{
    return this->miTimerID;
}

/// Set the timer_id.
void CTimerNode::SetTimerID( apl_int_t aiTimerID )
{
    this->miTimerID = aiTimerID;
}

void CTimerNode::GetDispatchInfo( DispatchInfoType& aoInfo )
{
    // Yes, do a copy
    aoInfo.mpoEventHandler = this->mpoEventHandler;
    aoInfo.mpvAct = this->mpvAct;
    aoInfo.miRecurringTimer = this->moInterval > CTimeValue::ZERO;
}

apl_int_t CTimerNode::GetHeapSlot(void)
{
    return this->miHeapSlot;
}

void CTimerNode::SetHeapSlot( apl_int_t aiSlot )
{
    this->miHeapSlot = aiSlot;
}

//////////////////////////////////////////// Timer Queue /////////////////////////////////
CTimerQueue::CTimerQueue( apl_size_t auSize )
    : mppoNodeHeap(NULL)
    , mpoPreallocatedNode(NULL)
    , muCurSize(0)
    , muFreeSize(0)
    , muMaxSize(auSize)
    , moTimerSkew(0)
{
    this->muMaxSize = muMaxSize <= 0 ? 1 : muMaxSize;
    
    ACL_NEW_N_ASSERT(this->mppoNodeHeap, NodeType*, this->muMaxSize);
    apl_memset(this->mppoNodeHeap, 0, sizeof(NodeType*) * this->muMaxSize);
        
    ACL_NEW_N_ASSERT(this->mpoPreallocatedNode, NodeType, this->muMaxSize);
        
    for (apl_size_t liN = 0; liN < this->muMaxSize; ++liN)
    {
        this->mpoPreallocatedNode[liN].SetTimerID(liN);
        
        this->FreeNode(&this->mpoPreallocatedNode[liN]);
    }
}

CTimerQueue::~CTimerQueue (void)
{
    this->Close();
    
    ACL_DELETE_N(this->mppoNodeHeap);
    ACL_DELETE_N(this->mpoPreallocatedNode);
}

void CTimerQueue::Close(void)
{
    while(this->muCurSize > 0)
    {
        NodeType* lpoFirst = this->GetFirst();
        
        this->Cancel(lpoFirst->GetEventHandler(), false);
    }
}

apl_size_t CTimerQueue::GetSize(void)
{
    return this->muCurSize;
}

bool CTimerQueue::IsEmpty (void) const
{
    return this->muCurSize == 0;
}

CTimestamp const& CTimerQueue::EarliestTime(void) const
{
    return this->mppoNodeHeap[0]->GetTimerValue();
}

apl_int_t CTimerQueue::Schedule(
    IEventHandler* apoEventHandler,
    void const* apvAct,
    CTimestamp const& aoFutureTime,
    CTimeValue const& aoInterval )
{
    TSmartLock<CLock> loLock(this->moLock);
    
    if ( this->muCurSize < this->muMaxSize )
    {
        // Obtain the memory to the new node.
        NodeType* lpoTemp = this->AllocNode();
        if ( lpoTemp == NULL )
        {
            return -1;
        }

        lpoTemp->Set(
            apoEventHandler,
            apvAct,
            aoFutureTime,
            aoInterval );

        this->Insert(lpoTemp);
        
        return lpoTemp->GetTimerID();
    }
    else
    {
        return -1;
    }
}

apl_int_t CTimerQueue::ResetInterval( apl_int_t aiTimerID, CTimeValue const& aoInterval )
{
    // Check to see if the timer_id is out of range
    if ( aiTimerID < 0 || (apl_size_t)aiTimerID > this->muMaxSize)
    {
        return -1;
    }
    
    this->mpoPreallocatedNode[aiTimerID].SetInterval(aoInterval);
    
    return 0;
}

apl_ssize_t CTimerQueue::Cancel( IEventHandler* apoEventHandler, bool abIsDontCallHandleClose )
{
    apl_ssize_t liNumberOfCancellations = 0;

    {
        TSmartLock<CLock> loLock(this->moLock);
        
        // Try to locate the CTimerNode that matches the TimerID.
        for ( apl_size_t liN = 0; liN < this->muCurSize; )
        {
            if (this->mppoNodeHeap[liN]->GetEventHandler() == apoEventHandler)
            {
                NodeType* lpoTemp = this->Remove(liN);
    
                liNumberOfCancellations++;
    
                this->FreeNode(lpoTemp);
    
                // We reset to zero so that we don't miss checking any nodes
                // if a reheapify occurs when a node is removed.  There
                // may be a better fix than this, however.
                liN = 0;
            }
            else
            {
                liN++;
            }
        }
    }

    if (!abIsDontCallHandleClose)
    {
        this->CancelEvent( apoEventHandler );
    }

    return liNumberOfCancellations;
}

apl_int_t CTimerQueue::Cancel( apl_int_t aiTimerID, void const** appvAct, bool abIsDontCallHandleClose )
{
    IEventHandler* lpoEventHandler = NULL;
    
    // Check to see if the timer_id is out of range
    if ( aiTimerID < 0 || (apl_size_t)aiTimerID > this->muMaxSize)
    {
        return -1;
    }
    
    do
    {
        TSmartLock<CLock> loLock(this->moLock);
        
        apl_int_t liHeapSlot = this->mpoPreallocatedNode[aiTimerID].GetHeapSlot();
    
        // Check to see if timer_id is still valid.
        if ( liHeapSlot < 0 )
        {
            return -1;
        }
    
        if ( aiTimerID != this->mppoNodeHeap[liHeapSlot]->GetTimerID() )
        {
            return -1;
        }
        else
        {
            NodeType* lpoTemp = this->Remove(liHeapSlot);
                
            lpoEventHandler = lpoTemp->GetEventHandler();
                
            if (appvAct != NULL)
            {
                *appvAct = lpoTemp->GetAct();
            }
                
            this->FreeNode(lpoTemp);
        }
    }while(false);
    
    if (!abIsDontCallHandleClose)
    {
        this->CancelEvent(lpoEventHandler);
    }
    
    return 0;
}

apl_ssize_t CTimerQueue::Expire( CTimestamp const& aoCurrentTime )
{
    apl_ssize_t liNumberOfTimersExpired = 0;
    apl_int_t   liResult = 0;

    DispatchInfoType loInfo;

    while ( ( liResult = this->DispatchInfo( aoCurrentTime, loInfo) ) == 1 )
    {
        this->Upcall(loInfo, aoCurrentTime);

        ++liNumberOfTimersExpired;
    }

    return liNumberOfTimersExpired;
}

apl_ssize_t CTimerQueue::Expire(void)
{
    CTimestamp loTimestamp;
    
    loTimestamp.Update(CTimestamp::PRC_USEC);
    loTimestamp += this->moTimerSkew;
    
    return this->Expire(loTimestamp);
}

apl_ssize_t CTimerQueue::DispatchInfo( CTimestamp const& aoCurrentTime, DispatchInfoType& aoInfo )
{
    TSmartLock<CLock> loLock(this->moLock);
    
    if ( this->IsEmpty() )
    {
        return 0;
    }

    NodeType* lpoExpired = NULL;

    if (this->EarliestTime() <= aoCurrentTime)
    {
        lpoExpired = this->RemoveFirst();

        // Get the dispatch info
        lpoExpired->GetDispatchInfo(aoInfo);

        // Check if this is an interval timer.
        if ( lpoExpired->GetInterval() > CTimeValue::ZERO )
        {
            // Make sure that we skip past values that have already
            // "expired".
            do
            {
                lpoExpired->SetTimerValue(
                    lpoExpired->GetTimerValue() + lpoExpired->GetInterval() );
            }
            while (lpoExpired->GetTimerValue() <= aoCurrentTime);

            // Since this is an interval timer, we need to reschedule
            // it.
            this->Reschedule(lpoExpired);
        }
        else
        {
            // Call the factory method to free up the node.
            this->FreeNode(lpoExpired);
        }
        
        return 1;
    }

    return 0;
}

CTimeValue* CTimerQueue::CalculateTimeout( CTimeValue const& aoMaxWaitTime, CTimeValue* apoTheTimeout )
{
    if ( apoTheTimeout == 0 )
    {
        return NULL;
    }

    if ( this->IsEmpty() )
    {
        // Nothing on the Timer_Queue, so use whatever the caller gave us.
        *apoTheTimeout = aoMaxWaitTime;
    }
    else
    {
        CTimestamp loCurTime;
        
        loCurTime.Update(CTimestamp::PRC_USEC);

        if (this->EarliestTime() > loCurTime)
        {
            // The earliest item on the Timer_Queue is still in the
            // future.  Therefore, use the smaller of (1) caller's wait
            // time or (2) the delta time between now and the earliest
            // time on the Timer_Queue.

            *apoTheTimeout = this->EarliestTime () - loCurTime;
            
            if ( aoMaxWaitTime < *apoTheTimeout )
            {
                *apoTheTimeout = aoMaxWaitTime;
            }
        }
        else
        {
            // The earliest item on the Timer_Queue is now in the past.
            // Therefore, we've got to "poll" the Reactor, i.e., it must
            // just check the descriptors and then dispatch timers, etc.
            *apoTheTimeout = CTimeValue::ZERO;
        }
    }
    
    return apoTheTimeout;
}

void CTimerQueue::Upcall( DispatchInfoType& aoInfo, CTimestamp const& aoCurTime )
{
    if ( aoInfo.mpoEventHandler->HandleTimeout(aoCurTime, aoInfo.mpvAct) == -1 )
    {
        this->Cancel(aoInfo.mpoEventHandler, false);
    }
}

void CTimerQueue::CancelEvent( IEventHandler* apoEventHandler )
{
    if ( apoEventHandler != NULL )
    {
        apoEventHandler->HandleClose( ACL_INVALID_HANDLE, IEventHandler::TIMER_MASK);
    }
}

CTimerQueue::NodeType* CTimerQueue::RemoveFirst(void)
{
    if (this->muCurSize == 0)
    {
        return NULL;
    }
    
    NodeType* lpoTemp = this->Remove(0);

    return lpoTemp;
}

CTimerQueue::NodeType* CTimerQueue::GetFirst(void)
{
    return this->muCurSize == 0 ? NULL : this->mppoNodeHeap[0];
}

void CTimerQueue::Reschedule( NodeType* apoExpired )
{
    this->Insert(apoExpired);
}

CTimerQueue::NodeType* CTimerQueue::AllocNode(void)
{
    if ( this->moFrees.empty() )
    {
        return NULL;
    }
    else
    {
        NodeType* lpoTemp = *this->moFrees.begin();
        this->moFrees.pop_front();
        
        this->muFreeSize--;
        
        return lpoTemp;
    }
}

void CTimerQueue::FreeNode( NodeType* apoNode )
{
    apoNode->SetHeapSlot(-1);
    
    this->moFrees.push_back(apoNode);
    
    this->muFreeSize++;
}

void CTimerQueue::Insert( NodeType* apoNewNode )
{
    apl_size_t luSlot   = this->muCurSize;
    apl_size_t luParent = this->muCurSize - 1;
    
    while (luSlot > 0)
    {
        // If the parent node is greater than the <moved_node> we need
        // to copy it down.
        if ( apoNewNode->GetTimerValue() < this->mppoNodeHeap[luParent]->GetTimerValue() )
        {
            this->mppoNodeHeap[luSlot] = this->mppoNodeHeap[luParent];
            this->mppoNodeHeap[luParent]->SetHeapSlot(luSlot);

            luSlot = luParent;
            luParent--;
        }
        else
        {
            break;
        }
    }

    // Insert the new node into its proper resting place in the heap and
    // update the corresponding slot in the parallel <timer_ids> array.
    this->mppoNodeHeap[luSlot] = apoNewNode;
    apoNewNode->SetHeapSlot(luSlot);
  
    this->muCurSize++;
}

CTimerQueue::NodeType* CTimerQueue::Remove( apl_size_t auSlot )
{
    NodeType* lpoRemovedNode = this->mppoNodeHeap[auSlot];

    // NOTE - the muCurSize is being decremented since the queue has one
    // less active timer in it. However, this CTimerNode is not being
    // freed, and there is still a place for it in mppoNodeHeap(the timer ID
    // is not being relinquished). The node can still be rescheduled, or
    // it can be freed via FreeNode.
    --this->muCurSize;

    // Only try to reheapify if we're not deleting the last entry.
    if (auSlot < this->muCurSize)
    {
        for ( apl_size_t liN = auSlot + 1; liN <= this->muCurSize; liN++ )
        {
            this->mppoNodeHeap[auSlot] = this->mppoNodeHeap[liN];
            this->mppoNodeHeap[auSlot]->SetHeapSlot(auSlot);
            auSlot = liN;
        }
    }
    
    return lpoRemovedNode;
}

/// Set the timer skew for the Timer_Queue.
void CTimerQueue::TimerSkew ( CTimeValue const& aoSkew )
{
    this->moTimerSkew = aoSkew;
}

/// Get the timer skew for the Timer_Queue.
CTimeValue const& CTimerQueue::TimerSkew(void) const
{
    return this->moTimerSkew;
}

ACL_NAMESPACE_END

