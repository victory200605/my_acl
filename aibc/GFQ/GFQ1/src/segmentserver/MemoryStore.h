
#ifndef GFQ_MEMORYSTORAGE_H
#define GFQ_MEMORYSTORAGE_H

#include "BaseStore.h"
#include "gfq/GFQUtility.h" 

AI_GFQ_NAMESPACE_START

/** 
 * @brief  Memory segment storage
 */
class CMemoryStore : public IBaseStore
{
    struct CDataNode
    {
        size_t  muLength;
        int32_t miTimestamp;
        char    macData[AI_MAX_DATA_LEN];
    };

    struct CSegment
    {
        CSegment(void)
            : miQueueGlobalID(-1)
            , muReadPos(0)
            , muWritePos(0)
        {
            memset(this->macQueueName, 0, sizeof(this->macQueueName) );
        }

        int32_t  miSegmentID;
        int64_t  miQueueGlobalID;
        char     macQueueName[AI_MAX_NAME_LEN];
        size_t   muReadPos;
        size_t   muWritePos;
        AI_STD::vector<CDataNode> moDatas;
        AIMutexLock  moLock;
    };

public:
    CMemoryStore( size_t auSegmentCount, size_t auSegmentCapacity );

    virtual ~CMemoryStore(void);

    virtual int Initialize(void);

    virtual void Close(void);

    virtual int Alloc( int32_t aiSegmentID, int64_t aiQueueGlobalID, char const* apcQueueName );

    virtual int Free( int32_t aiSegmentID );

    virtual int Put(
        int32_t aiSegmentID,
        int64_t aiQueueGlobalID,
        char const* apcQueueName,
        AIChunkEx const& aoData );
    
    virtual int Get(
        int32_t aiSegmentID,
        int64_t aiQueueGlobalID,
        char const* apcQueueName, 
        int32_t aiWhenSec, 
        AIChunkEx& aoData );

    virtual int GetSegmentInfo( int32_t aiSegmentID, CSegmentInfo& aoSegmentInfo );
    
    virtual int32_t GetSegmentCount(void);

    virtual int32_t GetSegmentCapacity(void);

private:
    size_t muSegmentCount;
    
    size_t muSegmentCapacity;
    
    AI_STD::vector<CSegment> moSegments;
};

AI_GFQ_NAMESPACE_END

#endif//GFQ_MEMORYSTORAGE_H

