
#include "acl/Singleton.h"
#include "acl/IndexDict.h"
#include "gfq2/GFQMessage.h"
#include "aaf/LogService.h"
#include "FIFOQueue.h"
#include "EventObserver.h"
#include "MasterConfig.h"

AIBC_GFQ_NAMESPACE_START

CFIFOQueue::CFIFOQueue(
    char const* apcName, 
    apl_size_t auQueueID, 
    apl_size_t auCapacity,
    char const* apcBindGroup )
    : moQueueName(apcName)
    , muQueueID(auQueueID)
    , muCapacity(auCapacity)
    , miTotalSize(0)
    , moGroupName(apcBindGroup)
    , mpoSrvManager(GetServerNodeManager() )
{
}

CFIFOQueue::~CFIFOQueue(void)
{
}
    
apl_int_t CFIFOQueue::InsertSegment( 
    CSegmentHandle aoHandle, 
    apl_size_t auSize, 
    apl_int64_t ai64Timestamp )
{
    if (aoHandle.IsInvalid() )
    {
        return ERROR_INVALID_SEGMENT;
    }

    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    SegmentMapType::iterator loIter = this->moSegmentMap.find(aoHandle);
    if (loIter != this->moSegmentMap.end() )
    {
        //Segment existed and return immediately
        return 0;
    }

    SegmentListType::iterator loBefore = this->moActivatedSegments.end();

    //Find insert slot, all segment will sort by timestamp
    //If no found will append to the back
    for (SegmentListType::iterator loTemp = this->moActivatedSegments.begin();
        loTemp != this->moActivatedSegments.end(); ++loTemp)
    {
        if (ai64Timestamp <= (*loTemp)->moCreateTime.Nsec() )
        {
            loBefore = loTemp;
            
            break;
        }
    }

    return this->Insert(loBefore, aoHandle, auSize, ai64Timestamp, 0);
}

apl_int_t CFIFOQueue::UpdateSegment( 
    CSegmentHandle aoHandle, 
    apl_size_t auSize, 
    apl_int64_t ai64Timestamp )
{
    apl_int_t liEvent = 0;

    {
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
        SegmentMapType::iterator loIter = this->moSegmentMap.find(aoHandle);
        if (loIter != this->moSegmentMap.end() )
        {
            CSegmentNodeIndex& loNodeIndex = loIter->second;

            //Trigger queue size event for EventObserver
            if (this->moSegmentMap.size() == 1)
            {
                if (loNodeIndex.mpoNode->muSize > 0 && auSize == 0)
                {
                    liEvent = EVENT_QUEUE_EMPTY;
                }
                else if (loNodeIndex.mpoNode->muSize == 0 && auSize > 0)
                {
                    liEvent = EVENT_QUEUE_NONEMPTY;
                }
            }

            this->miTotalSize += ((apl_int_t)auSize - (apl_int_t)loNodeIndex.mpoNode->muSize);
            loNodeIndex.mpoNode->muSize = auSize;
        }
    }
 
    if (liEvent != 0)
    {
        //Send readable event
        GFQ_SEND_EVENT(CLASS_QUEUE_EVENT, this->moQueueName, liEvent);
    }

    return 0;
}

apl_int_t CFIFOQueue::AllocateSegmentFinished( CSegmentHandle aoHandle, apl_int_t aiStatus )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    SegmentMapType::iterator loIter = this->moSegmentMap.find(aoHandle);
    if (loIter != this->moSegmentMap.end() )
    {
        loIter->second.mpoNode->miStatus = aiStatus;
    }

    return 0;
}

apl_int_t CFIFOQueue::DeallocateSegmentFinished( CSegmentHandle aoHandle, apl_int_t aiStatus )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    SegmentMapType::iterator loIter = this->moSegmentMap.find(aoHandle);
    if (loIter != this->moSegmentMap.end() )
    {
        CSegmentNodeIndex* lpoNodeIndex = &loIter->second;

        if(!lpoNodeIndex->mpoNode->mbIsRemoving)
        {
            return 0;
        }

        //Deallocate if successfully, remove the segment from queue
        //Oterwise the segment will be hook back the available list
        if (aiStatus == 0)
        {
            this->Remove(loIter);
        }
        else
        {
            this->RemovingRollback(loIter);
        }
    }

    return 0;
}

apl_int_t CFIFOQueue::GetReadableSegment( 
    CSegmentHandle aoOldHandle, 
    apl_int_t aiOldSegmentStatus, 
    CSegmentInfo& aoSegmentInfo )
{
    apl_int_t liRetCode = 0;
    CServerNodeInfo loServerNodeInfo;

    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    if (!aoOldHandle.IsInvalid() )
    {
        //Process client error 
        switch(aiOldSegmentStatus)
        {
            case ERROR_END_OF_SEGMENT:
            {
                liRetCode = this->Deallocate(aoOldHandle);
                break;
            }
            case ERROR_EMPTY_OF_SEGMENT:
            {
                liRetCode = this->Deallocate(aoOldHandle);
                break;
            }
            case ERROR_INVALID_MESSAGE:
            {
                return aiOldSegmentStatus;
            }
            default:
            {
                break;
            }
        };
        
        if (liRetCode != 0)
        {
            return liRetCode;
        }
    }

    while(true)
    {
        //Always return the first segment
        SegmentListType::iterator loFirst = this->moActivatedSegments.begin();
        if (loFirst != this->moActivatedSegments.end() )
        {
            CSegmentNode* lpoNode = *loFirst;
            if (this->mpoSrvManager->GetServerNodeInfo(
                lpoNode->moHandle.GetServerNodeID(), loServerNodeInfo) ==0 )
            {
                aoSegmentInfo.SetSegmentID(lpoNode->moHandle.GetSegmentID() );
                aoSegmentInfo.SetServerNodeName(loServerNodeInfo.GetServerNodeName() );
                aoSegmentInfo.SetServerIp(loServerNodeInfo.GetServerIp() );
                aoSegmentInfo.SetServerPort(loServerNodeInfo.GetServerPort() );
                aoSegmentInfo.SetTimestamp(lpoNode->moCreateTime.Nsec() );
                aoSegmentInfo.SetSize(lpoNode->muSize);

                return 0;
            }
            else
            {
                //Clear invalid segment, maybe the ServerNode is shutdown
                this->RemoveInvalidSegment(loFirst);
            }
        }
        else
        {
            return ERROR_END_OF_QUEUE;
        }
    }

    return ERROR_END_OF_QUEUE;
}

apl_int_t CFIFOQueue::GetWriteableSegment( 
    CSegmentHandle aoOldHandle,
    apl_int_t aiOldSegmentStatus,
    CSegmentInfo& aoSegmentInfo )
{
    apl_int_t       liRetCode = 0;
    CServerNodeInfo loServerNodeInfo;
    acl::CTimestamp loCurrTime;

    //Get current timestamp
    loCurrTime.Update();

    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    if (this->moActivatedSegments.size() == 0 && (liRetCode = this->Allocate() ) != 0 )
    {
        return liRetCode;
    }

    CSegmentNode& loBack = *this->moActivatedSegments.back();

    if (loBack.miStatus < 0 
        || (loBack.miStatus > 0 && (loCurrTime.Sec() - loBack.moCreateTime.Sec() ) >= 5) )
    {
        //Allocate fail, or timedout, maybe server node catch a exception now, remove it
        CServerNodeInfo loNodeInfo;

        if (this->mpoSrvManager->GetServerNodeInfo(loBack.moHandle.GetServerNodeID(), loNodeInfo) == 0)
        {
            this->mpoSrvManager->DelServerNode(loNodeInfo.GetServerNodeName() );

            AAF_LOG_ERROR(
                "FIFOQueue the allocate segment fail or timedout, delete the server node now, "
                "(ServerNodeName=%s,SegmentID=%"APL_PRIuINT",Status=%"APL_PRIdINT",ElapseTime=%"APL_PRIdINT")",
                loNodeInfo.GetServerNodeName(),
                loBack.moHandle.GetServerNodeID(),
                loBack.miStatus,
                (apl_int_t)(loCurrTime.Sec() - loBack.moCreateTime.Sec() ) );
        }
    }
    else if (!aoOldHandle.IsInvalid() )
    {
        switch(aiOldSegmentStatus)
        {
            case ERROR_END_OF_SEGMENT:
            {
                if (aoOldHandle == loBack.moHandle && (liRetCode = this->Allocate() ) != 0 )
                {
                    return liRetCode;
                }

                break;
            }
            case ERROR_INVALID_MESSAGE:
            case ERROR_OUT_OF_RANGE:
            {
                return aiOldSegmentStatus;
            }
            default:
            {
                //Retry it
                break;
            }
        };
    }
    
    while(true)
    {
        //Always return the back segment
        SegmentListType::reverse_iterator loBackIter = this->moActivatedSegments.rbegin();
        CSegmentNode* lpoNode = *loBackIter;

        if (this->mpoSrvManager->GetServerNodeInfo(
            lpoNode->moHandle.GetServerNodeID(), loServerNodeInfo) ==0 )
        {
            aoSegmentInfo.SetSegmentID(lpoNode->moHandle.GetSegmentID() );
            aoSegmentInfo.SetServerNodeName(loServerNodeInfo.GetServerNodeName() );
            aoSegmentInfo.SetServerIp(loServerNodeInfo.GetServerIp() );
            aoSegmentInfo.SetServerPort(loServerNodeInfo.GetServerPort() );
            aoSegmentInfo.SetTimestamp(lpoNode->moCreateTime.Nsec() );
            aoSegmentInfo.SetSize(lpoNode->muSize);

            return 0;
        }

        //Needn't to clear invalid segment, will be do in read operation

        //Allocate and append a new segment
        if ( (liRetCode = this->Allocate() ) != 0)
        {
            return liRetCode;
        }
    }

    return ERROR_NO_ENOUGH_SPACE;
}

apl_int_t CFIFOQueue::UpdateConfiguration(void)
{
    CMasterConfig::CQueueInfo loQueueInfo;

    if (!acl::Instance<CMasterConfig>()->FindQueueInfo(this->moQueueName.c_str(), loQueueInfo) )
    {
        return ERROR_INVALID_QUEUE;
    }

    this->muCapacity = loQueueInfo.muCapacity;

    return 0;
}

apl_int_t CFIFOQueue::Clear(void)
{
    apl_int_t liRetCode = 0;
    CServerNodeInfo loServerNodeInfo;

    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    while(true)
    {
        //Always return the first segment
        SegmentListType::iterator loFirst = this->moActivatedSegments.begin();
        if (loFirst != this->moActivatedSegments.end() )
        {
            CSegmentNode* lpoNode = *loFirst;
            
            if (this->mpoSrvManager->GetServerNodeInfo(
                lpoNode->moHandle.GetServerNodeID(), loServerNodeInfo) ==0 )
            {
                if ( (liRetCode = this->Deallocate(lpoNode->moHandle, true) ) != 0)
                {
                    return liRetCode;
                }
            }
            else
            {
                //Clear invalid segment, maybe the ServerNode is shutdown
                this->RemoveInvalidSegment(loFirst);
            }

            continue;
        }

        break;
    }

    return 0;
}

apl_int_t CFIFOQueue::ClearInvalidSegment( apl_size_t auServerNodeID )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    SegmentMapType::iterator loItlow = 
        this->moSegmentMap.lower_bound( CSegmentHandle(auServerNodeID, 0) );
    SegmentMapType::iterator loItup = 
        this->moSegmentMap.upper_bound( CSegmentHandle(auServerNodeID, APL_UINT_MAX) );
    
    for (SegmentMapType::iterator loIter = loItlow; loIter != loItup; )
    {
        CSegmentNodeIndex* lpoNodeIndex = &loIter->second;
        CSegmentNode* lpoNode = lpoNodeIndex->mpoNode;

        if (lpoNode->mbIsRemoving)
        {
            this->moRemovingSegments.erase(lpoNodeIndex->moIndex);
        }
        else
        {
            this->moActivatedSegments.erase(lpoNodeIndex->moIndex);
        }
        
        this->moSegmentMap.erase(loIter++);
    
        this->miTotalSize -= lpoNode->muSize;

        ACL_DELETE(lpoNode);
    }

    return 0;
}

apl_int_t CFIFOQueue::HandleTimedout( acl::CTimeValue const& aoTimedout )
{
    acl::CTimestamp loNow(acl::CTimestamp::PRC_SEC);

    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    while(this->moRemovingSegments.size() > 0)
    {
        CSegmentNode* lpoNode = *this->moRemovingSegments.begin();

        if (loNow.Sec() - lpoNode->moRemoveTime.Sec() > aoTimedout.Sec() )
        {
            SegmentMapType::iterator loIter = this->moSegmentMap.find(lpoNode->moHandle);
            ACL_ASSERT(loIter != this->moSegmentMap.end() );

            this->RemovingRollback(loIter);

            continue;
        }

        break;
    }

    return 0;
}

apl_int_t CFIFOQueue::BindNodeGroup( char const* apcGroupName )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    this->moGroupName = apcGroupName;

    return 0;
}

char const* CFIFOQueue::GetQueueName(void)
{
    return this->moQueueName.c_str();
}

apl_size_t CFIFOQueue::GetQueueID(void)
{
    return this->muQueueID;
}

apl_size_t CFIFOQueue::GetSegmentCount(void)
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    return this->moActivatedSegments.size();
}

apl_size_t CFIFOQueue::GetCapacity(void)
{
    return this->muCapacity;
}

apl_size_t CFIFOQueue::GetSize(void)
{
    //apl_size_t luSize = 0;
    //std::vector<CSegmentNode> loSegments;

    //{//Get all handles
    //    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    //    for (SegmentListType::iterator loIter = this->moActivatedSegments.begin();
    //        loIter != this->moActivatedSegments.end(); ++loIter)
    //    {
    //        CSegmentNode* lpoNode = *loIter;
    //        loSegments.push_back(*lpoNode);
    //    }
    //}

    //for (std::vector<CSegmentNode>::iterator loIter = loSegments.begin();
    //    loIter != loSegments.end(); ++loIter)
    //{
    //    if (!this->mpoSrvManager->IsInvalidServerNode(loIter->moHandle.GetServerNodeID() ) )
    //    {
    //        luSize += loIter->muSize;
    //    }
    //}

    return this->miTotalSize;
}

bool CFIFOQueue::IsEmpty(void)
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    if (this->moActivatedSegments.size() > 1 )
    {
        return false;
    }
    else if (this->moActivatedSegments.size() == 1 && (*this->moActivatedSegments.begin())->muSize > 0)
    {
        return false;
    }

    return true;
}

apl_int_t CFIFOQueue::Allocate(void)
{
    apl_int_t   liRetCode = 0;
    apl_int64_t li64Timestamp = apl_clock_gettime();

    while(true)
    {
        if (this->miTotalSize >= (apl_int_t)this->muCapacity)
        {
            return ERROR_NO_ENOUGH_SPACE;
        }

        //Allocate segment
        CSegmentHandle loHandle = this->mpoSrvManager->Allocate(this->moGroupName);
        if (loHandle.IsInvalid() )
        {
            return ERROR_NO_ENOUGH_SPACE;
        }

        liRetCode = this->Insert(this->moActivatedSegments.end(), loHandle, 0, li64Timestamp, 1);
        if (liRetCode != 0)
        {
            return liRetCode;
        }

        do
        {
            anf::SessionPtrType loSession;
            acl::CIndexDict loRequest;

            if (this->mpoSrvManager->GetSession(loHandle, loSession) != 0)
            {
                break;
            }

            //Send allocate request to segmentserver
            loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_ALLOCATE);
            loRequest.Set(anf::BENC_TAG_SEQ_NO, 1);
            loRequest.Set(GFQ_TAG_SEGMENT_ID, loHandle.GetSegmentID() );
            loRequest.Set(GFQ_TAG_QUEUE_NAME, this->moQueueName.c_str() );
            loRequest.Set(GFQ_TAG_TIMESTAMP, li64Timestamp);

            anf::CWriteFuture::PointerType loWriteFuture = loSession->Write(loSession, &loRequest);
            if (loWriteFuture->IsException() )
            {
                break;
            }

            return 0;
        }
        while(false);
        
        //Send allocate request exception, remove invalid segment and reallocate
        SegmentMapType::iterator loIter = this->moSegmentMap.find(loHandle);
        ACL_ASSERT(loIter != this->moSegmentMap.end() );
        
        CSegmentNodeIndex* lpoNodeIndex = &loIter->second;
        
        this->RemoveInvalidSegment(lpoNodeIndex->moIndex);

        //Retry it
    }
}

apl_int_t CFIFOQueue::Deallocate( CSegmentHandle aoHandle, bool abIsForce )
{
    SegmentMapType::iterator loIter = this->moSegmentMap.find(aoHandle);
    if (loIter != this->moSegmentMap.end() )
    {
        CSegmentNodeIndex* lpoNodeIndex = &loIter->second;

        if (lpoNodeIndex->mpoNode->mbIsRemoving)
        {
            return 0;
        }

        if (!abIsForce && this->moSegmentMap.size() == 1)
        {
            //Reader and writer will be allocated in the same segment
            //Ignore and retry it
            return ERROR_END_OF_QUEUE;
        }

        this->MoveToRemovingQueue(loIter);

        do
        {
            anf::SessionPtrType loSession;
            acl::CIndexDict loRequest;

            if (this->mpoSrvManager->GetSession(aoHandle, loSession) != 0)
            {
                break;
            }

            //Send allocate request to segmentserver
            loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_DEALLOCATE);
            loRequest.Set(anf::BENC_TAG_SEQ_NO, 1);
            loRequest.Set(GFQ_TAG_SEGMENT_ID, aoHandle.GetSegmentID() );
            loRequest.Set(GFQ_TAG_QUEUE_NAME, this->moQueueName.c_str() );
            loRequest.Set(GFQ_TAG_FORCE, abIsForce ? 1 : 0);

            anf::CWriteFuture::PointerType loWriteFuture = loSession->Write(loSession, &loRequest);
            if (loWriteFuture->IsException() )
            {
                break;
            }

            return 0;
        }
        while(false);

        //Send deallocate request fail and remove it
        this->Remove(loIter);
        
        return ERROR_BROKEN_SESSION;
    }
    
    return 0;
}

apl_int_t CFIFOQueue::Insert( 
    SegmentListType::iterator aoBefore, 
    CSegmentHandle aoHandle,
    apl_size_t auSize, 
    apl_int64_t ai64Timestamp,
    apl_int_t aiStatus )
{
    CSegmentNodeIndex& loNodeIndex = this->moSegmentMap[aoHandle];
    CSegmentNode* lpoNode = APL_NULL;

    if (loNodeIndex.mpoNode != APL_NULL)
    {
        return ERROR_SEGMENT_EXISTED;
    }
    
    ACL_NEW_ASSERT(lpoNode, CSegmentNode(aoHandle, auSize, ai64Timestamp, aiStatus) );

    loNodeIndex.moIndex = this->moActivatedSegments.insert(aoBefore, lpoNode);
    loNodeIndex.mpoNode = lpoNode;

    this->miTotalSize += auSize;

    return 0;
}

void CFIFOQueue::RemoveInvalidSegment( SegmentListType::iterator aoIter )
{
    CSegmentNode* lpoNode = *aoIter;

    this->moSegmentMap.erase(lpoNode->moHandle);
    this->moActivatedSegments.erase(aoIter);

    this->miTotalSize -= lpoNode->muSize;
    
    ACL_DELETE(lpoNode);
}

void CFIFOQueue::Remove( SegmentMapType::iterator aoIter )
{
    CSegmentNodeIndex* lpoNodeIndex = &aoIter->second;
    CSegmentNode* lpoNode = lpoNodeIndex->mpoNode;

    ACL_ASSERT(lpoNode->mbIsRemoving);

    this->moRemovingSegments.erase(lpoNodeIndex->moIndex);
    this->moSegmentMap.erase(aoIter);
    
    this->mpoSrvManager->Deallocate(lpoNode->moHandle);
    
    this->miTotalSize -= lpoNode->muSize;

    ACL_DELETE(lpoNode);
}

void CFIFOQueue::MoveToRemovingQueue( SegmentMapType::iterator aoIter )
{
    CSegmentNodeIndex* lpoNodeIndex = &aoIter->second;
    CSegmentNode* lpoNode = lpoNodeIndex->mpoNode;
    
    this->moActivatedSegments.erase(lpoNodeIndex->moIndex);
    lpoNodeIndex->moIndex = this->moRemovingSegments.insert(this->moRemovingSegments.end(), lpoNode);
    lpoNode->moRemoveTime.Update(acl::CTimestamp::PRC_SEC);
    lpoNode->mbIsRemoving = true;
}

void CFIFOQueue::RemovingRollback( SegmentMapType::iterator aoIter )
{
    CSegmentNodeIndex* lpoNodeIndex = &aoIter->second;
    CSegmentNode* lpoNode = lpoNodeIndex->mpoNode;

    this->moRemovingSegments.erase(lpoNodeIndex->moIndex);
    lpoNodeIndex->moIndex = this->moActivatedSegments.insert(this->moActivatedSegments.begin(), lpoNode);
    lpoNode->mbIsRemoving = false;
}

AIBC_GFQ_NAMESPACE_END

