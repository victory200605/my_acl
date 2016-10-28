
#ifndef AIBC_GFQ_QUEUE_H
#define AIBC_GFQ_QUEUE_H

#include "gfq2/Utility.h"
#include "ServerNodeManager.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Queue info
 */
class CQueueInfo
{
public:
    GFQ_MEMBER_METHOD_STR(QueueName);
    GFQ_MEMBER_METHOD_U(QueueID);
    GFQ_MEMBER_METHOD_U(Size);
    GFQ_MEMBER_METHOD_U(Capacity);
    GFQ_MEMBER_METHOD_U(SegmentCount);

private:
    std::string moQueueName;
    apl_size_t  muQueueID;
    apl_size_t  muSize;
    apl_size_t  muCapacity;
    apl_size_t  muSegmentCount;
};

/** 
 * @brief Base queue type for Global File Queue interface
 */
class IQueue
{
public:
    virtual ~IQueue(void) {};
    
    virtual apl_int_t InsertSegment(
        CSegmentHandle aoHandle, 
        apl_size_t auSize, 
        apl_int64_t ai64Timestamp ) = 0;

    virtual apl_int_t UpdateSegment(
        CSegmentHandle aoHandle, 
        apl_size_t auSize, 
        apl_int64_t ai64Timestamp ) = 0;

    virtual apl_int_t AllocateSegmentFinished( 
        CSegmentHandle aoHandle, 
        apl_int_t aiStatus ) = 0;

    virtual apl_int_t DeallocateSegmentFinished( 
        CSegmentHandle aoHandle, 
        apl_int_t aiStatus ) = 0;

    virtual apl_int_t GetReadableSegment( 
        CSegmentHandle aoOldHandle, 
        apl_int_t aiOldSegmentStatus, 
        CSegmentInfo& aoSegmentInfo ) = 0;

    virtual apl_int_t GetWriteableSegment( 
        CSegmentHandle aoOldHandle, 
        apl_int_t aiOldSegmentStatus,
        CSegmentInfo& aoSegmentInfo ) = 0;

    virtual apl_int_t UpdateConfiguration(void) = 0;

    virtual apl_int_t ClearInvalidSegment( apl_size_t auServerNodeID ) = 0;
    
    virtual apl_int_t Clear(void) = 0;
    
    virtual apl_int_t HandleTimedout( acl::CTimeValue const& aoTimedout ) = 0;
    
    virtual apl_int_t BindNodeGroup( char const* apcGroupName ) = 0;

// Attribute
    virtual char const* GetQueueName(void) = 0;

    virtual apl_size_t GetQueueID(void) = 0;
    
    virtual apl_size_t GetSegmentCount(void) = 0;

    virtual apl_size_t GetCapacity(void) = 0;
    
    virtual apl_size_t GetSize(void) = 0;
    
    virtual bool IsEmpty(void) = 0;
};

AIBC_GFQ_NAMESPACE_END

#endif//GFQ_QUEUE_H

