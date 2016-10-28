
#ifndef AIBC_GFQ_FIFOQUEUE_H
#define AIBC_GFQ_FIFOQUEUE_H

#include "gfq2/Utility.h"
#include "acl/stl/list.h"
#include "Queue.h"
#include "ServerNodeManager.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Base queue type for Global File Queue interface
 */
class CFIFOQueue : public IQueue
{
    struct CSegmentNode
    {
        CSegmentNode( CSegmentHandle const& aoHandle, apl_size_t auSize, apl_int64_t ai64Timestamp, apl_int_t aiStatus )
            : moHandle(aoHandle)
            , muSize(auSize)
            , miStatus(aiStatus)
            , moCreateTime(ai64Timestamp)
            , mbIsRemoving(false)
        {
        }

        CSegmentHandle moHandle;
        apl_size_t     muSize;
        apl_int_t      miStatus;
        acl::CTimestamp moCreateTime;
        acl::CTimestamp moRemoveTime;
        bool mbIsRemoving;
    };
    typedef std::list<CSegmentNode*> SegmentListType;

    struct CSegmentNodeIndex
    {
        CSegmentNodeIndex(void)
            : mpoNode(APL_NULL)
        {
        }

        CSegmentNodeIndex( SegmentListType::iterator aoIndex, CSegmentNode* apoNode )
            : moIndex(aoIndex)
            , mpoNode(apoNode)
        {
        }

        SegmentListType::iterator moIndex;
        CSegmentNode* mpoNode;
    };

    typedef std::map<CSegmentHandle, CSegmentNodeIndex> SegmentMapType;

public:
    CFIFOQueue(
        char const* apcName,
        apl_size_t auQueueID,
        apl_size_t auCapacity,
        char const* apcBindGroup );

    virtual ~CFIFOQueue(void);
    
    virtual apl_int_t InsertSegment(
        CSegmentHandle aoHandle,
        apl_size_t auSize, 
        apl_int64_t ai64Timestamp );
    
    virtual apl_int_t UpdateSegment( 
        CSegmentHandle aoHandle, 
        apl_size_t auSize, 
        apl_int64_t ai64Timestamp );

    virtual apl_int_t AllocateSegmentFinished(
        CSegmentHandle aoHandle, 
        apl_int_t aiStatus );

    virtual apl_int_t DeallocateSegmentFinished(
        CSegmentHandle aoHandle, 
        apl_int_t aiStatus );

    virtual apl_int_t GetReadableSegment(
        CSegmentHandle aoOldHandle,
        apl_int_t aiOldSegmentStatus,
        CSegmentInfo& aoSegmentInfo );

    virtual apl_int_t GetWriteableSegment( 
        CSegmentHandle aoOldHandle,
        apl_int_t aiOldSegmentStatus,
        CSegmentInfo& aoSegmentInfo );

    virtual apl_int_t UpdateConfiguration(void);

    virtual apl_int_t ClearInvalidSegment( apl_size_t auServerNodeID );
    
    virtual apl_int_t Clear(void);

    virtual apl_int_t HandleTimedout( acl::CTimeValue const& aoTimedout );

    virtual apl_int_t BindNodeGroup( char const* apcGroupName );

    virtual char const* GetQueueName(void);

    virtual apl_size_t GetQueueID(void);
    
    virtual apl_size_t GetSegmentCount(void);

    virtual apl_size_t GetCapacity(void);
    
    virtual apl_size_t GetSize(void);
    
    virtual bool IsEmpty(void);

protected:
    apl_int_t Allocate(void);

    apl_int_t Deallocate( CSegmentHandle aoHandle, bool abIsForce = false );

    apl_int_t Insert(
        SegmentListType::iterator aoBefore, 
        CSegmentHandle aoHandle, 
        apl_size_t auSize, 
        apl_int64_t ai64Timestamp,
        apl_int_t aiStatus );

    void RemoveInvalidSegment( SegmentListType::iterator aoIter );

    void Remove( SegmentMapType::iterator aoIter );
    
    void MoveToRemovingQueue( SegmentMapType::iterator aoIter );
    
    void RemovingRollback( SegmentMapType::iterator aoIter );

private:
    std::string moQueueName;

    apl_size_t muQueueID;

    apl_size_t muSegmentCount;

    apl_size_t muCapacity;
    
    apl_int_t miTotalSize;

    std::string moGroupName;

    SegmentListType moActivatedSegments;
    
    SegmentListType moRemovingSegments;
    
    SegmentMapType moSegmentMap;

    acl::CLock moLock;

    CServerNodeManager* mpoSrvManager;
};

AIBC_GFQ_NAMESPACE_END

#endif//GFQ_QUEUE_H

