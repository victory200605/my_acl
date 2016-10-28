#ifndef AIBC_GFQ2_SEGMENTQUEUEVLR_H
#define AIBC_GFQ2_SEGMENTQUEUEVLR_H

#include "../SegmentQueue.h"
#include "VLRStorage.h"
#include "VLRPage.h"
#include "gfq2/Utility.h"
#include "acl/MemMap.h"
#include "acl/Synch.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Segment Queue class define, support Variable-Length-Record storage
 */
class CVLRSegmentQueue : public ISegmentQueue
{
    /** 
     * @brief Storage checkpoint for reading or writing operation atomicity
     */
    struct CCheckpoint
    {
        apl_int8_t  miEnable;

        apl_int32_t miType;

        apl_int32_t maiPoint[64];
    };

    /** 
     * @brief Segment Header define for SegmentQueue
     */
    struct CSegmentHeader
    {
        apl_int32_t miSegmentID;

        apl_int8_t miFree;

        char macQueueName[64];

        apl_int64_t mi64Timestamp;

        apl_int32_t miReadPage;

        apl_int32_t miWritePage;

        apl_int32_t miReadOffset;
        
        apl_int32_t miSize;

        apl_int32_t miStatus;
    };

    /** 
     * @brief Storage Header define for page allocating and deallocating
     */
    struct CStorageHeader
    {
        apl_int32_t miFirstFreePage;
        
        apl_int32_t miFreePageCount;
        
        apl_int32_t miStatus;

        CCheckpoint moCheckpoint;

        CSegmentHeader maoSegmentHeader[1];
    };

public:
    CVLRSegmentQueue( char const* apcPath, char const* apcBufferPoolType, apl_int_t aiBufferPoolSize );

    ~CVLRSegmentQueue(void);

    static apl_int_t Create(
        char const* apcPath,
        apl_size_t auSegmentCapacity,
        apl_size_t auSegmentCount,
        apl_size_t auFileCount,
        apl_size_t auPageCount,
        apl_size_t auPageSize );
    
    apl_int_t Open(void);

    apl_int_t Check(void);
    
    apl_int_t Recover(void);
    
    apl_int_t Clear( char const* apcQueueName );
    
    apl_int_t ClearAll(void);

    void Close(void);

    apl_int_t Allocate( apl_size_t auSegmentID, char const* apcQueueName, apl_int64_t ai64Timestamp );

    apl_int_t Deallocate( apl_size_t auSegmentID );

    apl_int_t Push( apl_size_t auSegmentID, char const* apcQueueName, char const* apcData, apl_size_t auLen );

    apl_int_t Pop( apl_size_t auSegmentID, char const* apcQueueName, apl_int_t aiStayTime, acl::CMemoryBlock& aoData );

    apl_int_t GetSegmentInfo( apl_size_t auSegmentID, CSegmentInfo& aoSegmentInfo );
    
    apl_int_t GetAllSegmentInfo( std::vector<CSegmentInfo>& aoSegmentInfos );

    apl_size_t GetSegmentCount(void);

    apl_size_t GetSegmentCapacity(void);
    
    void Dump(void);
    
    void Dump( acl::CIndexDict& aoMessage );

protected:
    static void InitSegmentHeader( CSegmentHeader* apoSegmentHeader );
    
    apl_int_t Deallocate0( CSegmentHeader* apoSegmentHeader );

    apl_int_t AllocatePage( CSegmentHeader* apoSegmentHeader );

    apl_int_t DeallocatePage( CSegmentHeader* apoSegmentHeader );

    apl_int_t CheckCheckpointAndRecover(void);

private:
    //perameter
    std::string moPath;

    std::string moBufferPoolType;

    apl_int_t miBufferPoolSize;

    //Segment Queue Datastruct
    CStorageHeader* mpoStorageHeader;
    
    CSegmentHeader* mpoSegmentHeader;

    apl_int32_t* mpiNextPageEntry;

    apl_int_t miSegmentCount;
    
    apl_int_t miSegmentCapacity;

    apl_int_t miPageSize;
    
    apl_int_t miTotalPageCount;

    CVLRStorage moStorage;

    CVLRBufferPool moBufferPool;

    //Stat.
    apl_uint64_t mu64WriteTimes;
    
    apl_uint64_t mu64PrevWriteTimes;
    
    apl_uint64_t mu64ReadTimes;
    
    apl_uint64_t mu64PrevReadTimes;
    
    acl::CLock moLock;
};

AIBC_GFQ_NAMESPACE_END

#endif

