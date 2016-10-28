
#ifndef GFQ_GFQ2_SEGMENTQUEUEFLR_H
#define GFQ_GFQ2_SEGMENTQUEUEFLR_H

#include "SegmentQueue.h"
#include "FLRStorage.h"
#include "acl/Synch.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief storage segment manager
 */
class CFLRSegmentQueue : public ISegmentQueue
{
    struct CSegmentHeader
    {
        //Segment queue name
        char          macQueueName[APL_NAME_MAX];
    
        //Segment allocated timestamp
        apl_int64_t   mi64Timestamp;
    
        //Segment read position
        apl_int32_t   miReadPos;
    
        //Segment write position
        apl_int32_t   miWritePos;
    };
    
public:
    CFLRSegmentQueue( char const* apcPath );

    ~CFLRSegmentQueue(void);

    static apl_int_t Create(
        char const* apcPath, 
        apl_size_t auSlotCapacity,
        apl_size_t auSegmentSize,
        apl_size_t auSegmentCount,
        apl_size_t auFileCount );
    
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

private:
    std::string moPath;
        
    apl_size_t muSegmentCount;
    
    apl_size_t muSegmentCapacity;

    CSegmentHeader* mpoSegmentHeader;
    
    acl::CLock* mpoLock;
    
    CFLRStorage moStorage;

    //Stat.
    apl_uint64_t mu64WriteTimes;
    
    apl_uint64_t mu64PrevWriteTimes;

    apl_uint64_t mu64ReadTimes;
    
    apl_uint64_t mu64PrevReadTimes;
};

AIBC_GFQ_NAMESPACE_END

#endif//GFQ_FILESTORAGE_H

