#ifndef AIBC_GFQ2_SEGMENTQUEUE_H
#define AIBC_GFQ2_SEGMENTQUEUE_H

#include "Utility.h"
#include "Storage.h"
#include "Page.h"
#include "acl/MemMap.h"
#include "acl/Synch.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Segment Queue class define, support variable-length record storage
 */
class CSegmentQueue
{
    /** 
     * @brief Storage checkpoint for reading or writing operation atomicity
     */
    struct CStorageCheckpoint
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
    };

    /** 
     * @brief Storage Header define for page allocating and deallocating
     */
    struct CStorageHeader
    {
        apl_int32_t miFirstFreePage;
        
        apl_int32_t miFreePageCount;

        CStorageCheckpoint moCheckpoint;

        CSegmentHeader maoSegmentHeader[1];
    };

public:
    CSegmentQueue(void);

    ~CSegmentQueue(void);

    static apl_int_t Create( char const* apcPath, CStorageMetadata& aoMetadata );
    
    apl_int_t Open( char const* apcPath, char const* apcType, apl_int_t aiBufferPoolSize );

    void Close(void);

    apl_int_t Allocate( apl_size_t auSegmentID, char const* apcQueueName, apl_int64_t ai64Timestamp );

    apl_int_t Deallocate( apl_size_t auSegmentID );

    apl_int_t Get( apl_size_t auSegmentID, char const* apcQueueName, apl_int_t aiStayTime, std::string& aoData );

    apl_int_t Put( apl_size_t auSegmentID, char const* apcQueueName, char const* apcBuffer, apl_size_t auLen );

    apl_int_t Clear(void);

protected:
    apl_int_t IntegrityCheck(void);
    
    apl_int_t Deallocate0( CSegmentHeader* apoSegmentHeader );

    apl_int_t AllocatePage( CSegmentHeader* apoSegmentHeader );

    apl_int_t DeallocatePage( CSegmentHeader* apoSegmentHeader );

    apl_int_t CheckCheckpointAndRecover(void);

private:
    CStorageHeader* mpoStorageHeader;
    
    CSegmentHeader* mpoSegmentHeader;

    apl_int32_t* mpiNextPageEntry;

    apl_int_t miSegmentCount;
    
    apl_int_t miSegmentCapacity;

    apl_int_t miPageSize;
    
    apl_int_t miTotalPageCount;

    acl::CLock* mpoLock;

    acl::CLock moLock;

    CStorage moStorage;

    CBufferPool moBufferPool;
};

AIBC_GFQ_NAMESPACE_END

#endif

