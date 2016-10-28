
#include "QueueManager.h"
#include "EventObserver.h"
#include "acl/Singleton.h"

AIBC_GFQ_NAMESPACE_START

#define GFQ2_HANDLE_TIMEDOUT 60

/** 
 * @brief Manage all queue resource, include creating and destroying queue
 */
CQueueManager::CQueueManager( CQueueFactory* apoQueueFactory )
    : muQueueID(0)
    , mpoQueueFactory(apoQueueFactory ? apoQueueFactory : acl::Instance<CQueueFactory>() )
{
    this->miTimerID = acl::Instance<acl::CTimerQueueAdapter>()->Schedule(
        this,
        APL_NULL,
        acl::CTimeValue(GFQ2_HANDLE_TIMEDOUT),
        acl::CTimeValue(GFQ2_HANDLE_TIMEDOUT) );

    ACL_ASSERT(this->miTimerID != -1);
}

CQueueManager::~CQueueManager(void)
{
    acl::Instance<acl::CTimerQueueAdapter>()->Cancel(this->miTimerID, APL_NULL);
}

apl_int_t CQueueManager::InsertSegment( 
    char const* apcName, 
    CSegmentHandle aoHandle,
    apl_size_t auSegmentSize, 
    apl_int64_t ai64Timestamp )
{
    apl_int_t liRetCode = 0;
    IQueue* lpoQueue = APL_NULL;

    acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock); 
    
    QueueMapType::iterator loIter = this->moQueueMap.find(apcName);
    if (loIter == this->moQueueMap.end() )
    {
        if ( (liRetCode = this->CreateQueue0(apcName, &lpoQueue) ) != 0)
        {
            return liRetCode;
        }
    }
    else
    {
        lpoQueue = loIter->second;
    }

    return lpoQueue->InsertSegment(aoHandle, auSegmentSize, ai64Timestamp);
}

apl_int_t CQueueManager::UpdateSegment( 
    char const* apcName, 
    CSegmentHandle aoHandle, 
    apl_size_t auSegmentSize,
    apl_int64_t ai64Timestamp )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock); 
    
    QueueMapType::iterator loIter = this->moQueueMap.find(apcName);
    if (loIter == this->moQueueMap.end() )
    {
        return ERROR_SEGMENT_UNEXISTED;
    }

    IQueue* lpoQueue = loIter->second;

    return lpoQueue->UpdateSegment(aoHandle, auSegmentSize, ai64Timestamp);
}

apl_int_t CQueueManager::AllocateSegmentFinished(
    char const* apcName,
    CSegmentHandle aoHandle,
    apl_int_t aiStatus )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock); 
    
    QueueMapType::iterator loIter = this->moQueueMap.find(apcName);
    if (loIter == this->moQueueMap.end() )
    {
        return ERROR_QUEUE_UNEXISTED;
    }

    IQueue* lpoQueue = loIter->second;
    
    return lpoQueue->AllocateSegmentFinished(aoHandle, aiStatus);
}

apl_int_t CQueueManager::DeallocateSegmentFinished(
    char const* apcName,
    CSegmentHandle aoHandle,
    apl_int_t aiStatus )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock); 
    
    QueueMapType::iterator loIter = this->moQueueMap.find(apcName);
    if (loIter == this->moQueueMap.end() )
    {
        return ERROR_QUEUE_UNEXISTED;
    }

    IQueue* lpoQueue = loIter->second;
    
    return lpoQueue->DeallocateSegmentFinished(aoHandle, aiStatus);
}

apl_int_t CQueueManager::GetReadableSegment( 
    char const* apcName,
    CSegmentHandle aoOldHandle, 
    apl_int_t aiOldSegmentStatus,
    CSegmentInfo& aoSegmentInfo )
{
    CSegmentHandle loNewHandle;
    
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock); 
    
    QueueMapType::iterator loIter = this->moQueueMap.find(apcName);
    if (loIter == this->moQueueMap.end() )
    {
        return ERROR_QUEUE_UNEXISTED;
    }
    IQueue* lpoQueue = loIter->second;

    return lpoQueue->GetReadableSegment(aoOldHandle, aiOldSegmentStatus, aoSegmentInfo);
}

apl_int_t CQueueManager::GetWriteableSegment( 
    char const* apcName, 
    CSegmentHandle aoOldHandle,
    apl_int_t aiOldSegmentStatus, 
    CSegmentInfo& aoSegmentInfo )
{
    apl_int_t liRetCode = 0;
    IQueue* lpoQueue = APL_NULL;
    CSegmentHandle loNewHandle;
    
    acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock); 
    
    QueueMapType::iterator loIter = this->moQueueMap.find(apcName);
    if (loIter == this->moQueueMap.end() )
    {
        //If unexisted and create it
        if ( (liRetCode = this->CreateQueue0(apcName, &lpoQueue) ) != 0)
        {
            return liRetCode;
        }
    }
    else
    {
        lpoQueue = loIter->second;
    }

    return lpoQueue->GetWriteableSegment(aoOldHandle, aiOldSegmentStatus, aoSegmentInfo);
}

apl_int_t CQueueManager::UpdateConfiguration(void)
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    for (apl_size_t luN = 0; luN < this->moQueueList.size(); luN++)
    {
        IQueue* lpoQueue = this->moQueueList[luN];
        
        lpoQueue->UpdateConfiguration();
    }

    return 0;
}

apl_int_t CQueueManager::ClearQueueInvalidSegment( apl_size_t auServerNodeID )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    for (apl_size_t luN = 0; luN < this->moQueueList.size(); luN++)
    {
        IQueue* lpoQueue = this->moQueueList[luN];
        
        lpoQueue->ClearInvalidSegment(auServerNodeID);
    }

    return 0;
}

apl_int_t CQueueManager::ClearQueue( char const* apcName )
{
    CSegmentHandle loNewHandle;
    
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock); 
    
    QueueMapType::iterator loIter = this->moQueueMap.find(apcName);
    if (loIter == this->moQueueMap.end() )
    {
        return ERROR_QUEUE_UNEXISTED;
    }

    IQueue* lpoQueue = loIter->second;

    return lpoQueue->Clear();
}

apl_size_t CQueueManager::GetQueueList( std::vector<std::string>& aoResult, apl_size_t auOffset, apl_size_t auLimit )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    apl_int_t liLimit(auLimit);

    for (apl_size_t luN = auOffset; luN < this->moQueueList.size() && liLimit > 0; luN++, liLimit--)
    {
        IQueue* lpoQueue = this->moQueueList[luN];

        aoResult.push_back(lpoQueue->GetQueueName() );
    }

    return aoResult.size();
}

apl_size_t CQueueManager::GetQueueList( acl::CBitArray& aoResult )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock); 
    
    aoResult.Resize(this->muQueueID);
    aoResult.SetAll(true);

    //Never delete queue because all is true
    //for (QueueListType::iterator loIter = this->moQueueList.begin();
    //     loIter != this->moQueueList.end(); ++loIter)
    //{
    //    IQueue* lpoQueue = *loIter;

    //    aoResult.Set(lpoQueue->GetQueueID(), true);
    //    luSize++;
    //}

    return this->muQueueID;
}

apl_size_t CQueueManager::GetNonEmptyQueueList( acl::CBitArray& aoResult )
{
    apl_size_t luSize = 0;

    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock); 
    
    aoResult.Resize(this->muQueueID);
    aoResult.SetAll(false);

    for (QueueListType::iterator loIter = this->moQueueList.begin();
         loIter != this->moQueueList.end(); ++loIter)
    {
        IQueue* lpoQueue = *loIter;

        if (!lpoQueue->IsEmpty() )
        {
            aoResult.Set(lpoQueue->GetQueueID(), true);
            luSize++;
        }
    }

    return luSize;
}

apl_int_t CQueueManager::GetQueueInfo( char const* apcName, CQueueInfo& aoQueueInfo )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock); 
    
    QueueMapType::iterator loIter = this->moQueueMap.find(apcName);
    if (loIter == this->moQueueMap.end() )
    {
        return ERROR_QUEUE_UNEXISTED;
    }

    return this->GetQueueInfo(loIter->second, aoQueueInfo);
}

apl_int_t CQueueManager::GetQueueInfo( apl_size_t auQueueID, CQueueInfo& aoQueueInfo )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    if (auQueueID >= this->moQueueList.size() )
    {
        return ERROR_QUEUE_UNEXISTED;
    }

    return this->GetQueueInfo(this->moQueueList[auQueueID], aoQueueInfo);
}

apl_int_t CQueueManager::GetQueueInfo( IQueue* apoQueue, CQueueInfo& aoQueueInfo )
{
    aoQueueInfo.SetQueueName(apoQueue->GetQueueName() );
    aoQueueInfo.SetQueueID(apoQueue->GetQueueID() );
    aoQueueInfo.SetSize(apoQueue->GetSize() );
    aoQueueInfo.SetSegmentCount(apoQueue->GetSegmentCount() );
    aoQueueInfo.SetCapacity(apoQueue->GetCapacity() );
    
    return 0;
}

apl_int_t CQueueManager::BindNodeGroup( char const* apcQueueName, char const* apcGroupName )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock); 
    
    QueueMapType::iterator loIter = this->moQueueMap.find(apcQueueName);
    if (loIter == this->moQueueMap.end() )
    {
        return ERROR_QUEUE_UNEXISTED;
    }
    IQueue* lpoQueue = loIter->second;
    
    return lpoQueue->BindNodeGroup(apcGroupName);
}

apl_int_t CQueueManager::CreateQueue( char const* apcQueueName, apl_int_t aiOpt, CQueueInfo* apoInfo )
{
    acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock); 

    QueueMapType::iterator loIter = this->moQueueMap.find(apcQueueName);
    if (loIter != this->moQueueMap.end() )
    {
        if (aiOpt == IGNORE_QUEUE_EXISTED)
        {
            if (apoInfo != APL_NULL)
            {
                this->GetQueueInfo(loIter->second, *apoInfo);
            }

            return 0;
        }
        else
        {
            return ERROR_QUEUE_EXISTED;
        }
    }
    else
    {
        IQueue*   lpoQueue = APL_NULL;
        apl_int_t liRetCode = this->CreateQueue0(apcQueueName, &lpoQueue);

        if (liRetCode == 0 && apoInfo != APL_NULL)
        {
            this->GetQueueInfo(lpoQueue, *apoInfo);
        }

        return liRetCode;
    }
}

apl_int_t CQueueManager::HandleTimeout( acl::CTimestamp const& aoCurrTime, void const* apcAct )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    for (apl_size_t luN = 0; luN < this->moQueueList.size(); luN++)
    {
        IQueue* lpoQueue = this->moQueueList[luN];
        
        lpoQueue->HandleTimedout(GFQ2_HANDLE_TIMEDOUT);
    }

    return 0;
}

apl_int_t CQueueManager::CreateQueue0( char const* apcName, IQueue** appoNew )
{
    IQueue* lpoNew = APL_NULL;

    if ((lpoNew = this->mpoQueueFactory->CreateQueue(apcName, this->muQueueID++) ) != APL_NULL)
    {
        this->moQueueMap[apcName] = lpoNew;
        this->moQueueList.push_back(lpoNew);
        
        //SEND : EVENT_QUEUE_CREATE
        GFQ_SEND_EVENT(CLASS_QUEUE_EVENT, apcName, EVENT_QUEUE_CREATE);

        *appoNew = lpoNew;

        return 0;
    }
    else
    {
        return ERROR_INVALID_QUEUE;
    }
}

apl_int_t CQueueManager::DestroyQueue0( char const* apcName )
{
    QueueMapType::iterator loIter = this->moQueueMap.find(apcName);
    if (loIter == this->moQueueMap.end() )
    {
        return ERROR_QUEUE_UNEXISTED;
    }

    IQueue* lpoDelete = loIter->second;

    this->moQueueMap.erase(loIter);
    this->moQueueList[lpoDelete->GetQueueID()] = APL_NULL;

    this->mpoQueueFactory->DestroyQueue(lpoDelete);
    
    //SEND : EVENT_QUEUE_CREATE
    GFQ_SEND_EVENT(CLASS_QUEUE_EVENT, apcName, EVENT_QUEUE_DESTROY);
    
    return 0;
}

CQueueManager* GetQueueManager(void)
{
    return acl::Instance<CQueueManager>();
}

AIBC_GFQ_NAMESPACE_END

