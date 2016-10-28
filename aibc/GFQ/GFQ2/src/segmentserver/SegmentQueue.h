#ifndef AIBC_GFQ2_SEGMENTQUEUE_H
#define AIBC_GFQ2_SEGMENTQUEUE_H

#include "gfq2/Utility.h"
#include "acl/MemoryBlock.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"
#include "acl/Timestamp.h"
#include "acl/IndexDict.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Segment info define
 */
class CSegmentInfo
{
public:
    GFQ_MEMBER_METHOD_U(SegmentID);
    GFQ_MEMBER_METHOD_I64(Timestamp);
    GFQ_MEMBER_METHOD_STR(QueueName);
    GFQ_MEMBER_METHOD_U(Size);
    GFQ_MEMBER_METHOD_U(Capacity);

private:
    apl_size_t   muSegmentID;
    apl_int64_t  mi64Timestamp;
    std::string  moQueueName;
    apl_size_t   muSize;
    apl_size_t   muCapacity;
};

/** 
 * @brief Segment Queue interface class
 */
class ISegmentQueue
{
public:
    virtual ~ISegmentQueue(void) {};

    /** 
     * @brief Open segment queue
     */
    virtual apl_int_t Open(void) = 0;

    /** 
     * @brief Check storage file and datastruct
     */
    virtual apl_int_t Check(void) = 0;

    /** 
     * @brief Recover storage data and datastruct
     */
    virtual apl_int_t Recover(void) = 0;

    /** 
     * @brief Clear segment queue data by specified queue name and release all resource
     */
    virtual apl_int_t Clear( char const* apcQueueName ) = 0;

    /** 
     * @brief Clear segment queue all data and release all resource
     */
    virtual apl_int_t ClearAll(void) = 0;

    /** 
     * @brief Close segment queue
     */
    virtual void Close(void) = 0;

    /** 
     * @brief Allocate segment by specified SegmentID and QueueName
     */
    virtual apl_int_t Allocate( apl_size_t auSegmentID, char const* apcQueueName, apl_int64_t ai64Timestamp ) = 0;

    /** 
     * @brief Deallocate segment by specified SegmentID
     */
    virtual apl_int_t Deallocate( apl_size_t auSegmentID ) = 0;

    /** 
     * @brief Put data into segment queue by specified SegmentID and StayTime
     */
    virtual apl_int_t Push( apl_size_t auSegmentID, char const* apcQueueName, char const* apcData, apl_size_t auLen ) = 0;

    /** 
     * @brief Get data from segment queue by specified SegmentID and StayTime
     */
    virtual apl_int_t Pop( apl_size_t auSegmentID, char const* apcQueueName, apl_int_t aiStayTime, acl::CMemoryBlock& aoData ) = 0;

    virtual apl_int_t GetSegmentInfo( apl_size_t auSegmentID, CSegmentInfo& aoSegmentInfo ) = 0;
    
    virtual apl_int_t GetAllSegmentInfo( std::vector<CSegmentInfo>& aoSegmentInfos ) = 0;

    virtual apl_size_t GetSegmentCount(void) = 0;

    virtual apl_size_t GetSegmentCapacity(void) = 0;
    
    virtual void Dump(void) = 0;
    
    virtual void Dump( acl::CIndexDict& aoMessage ) = 0;
};

AIBC_GFQ_NAMESPACE_END

#endif

