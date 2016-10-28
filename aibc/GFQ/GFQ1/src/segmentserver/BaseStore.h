
#ifndef GFQ_BASESTORAGE_H
#define GFQ_BASESTORAGE_H

#include "gfq/GFQUtility.h"

#include "stl/vector.h"

AI_GFQ_NAMESPACE_START

struct CSegmentInfo
{
    CSegmentInfo(void)
        : miSegmentID(-1)
        , miQueueGlobalID(-1)
        , muSize(0)
    {
    }

    void    SetSegmentID( int32_t aiSegmentID )     { this->miSegmentID = aiSegmentID; }
    int32_t GetSegmentID(void)                      { return this->miSegmentID; }

    void    SetQueueGlobalID( int64_t aiQueueID )   { this->miQueueGlobalID = aiQueueID; }
    int64_t GetQueueGlobalID(void)                  { return this->miQueueGlobalID; }

    void    SetQueueName( char const* apcName )     { strncpy(this->macQueueName, apcName, sizeof(this->macQueueName) ); }
    char const* GetQueueName(void)                  { return this->macQueueName; }

    void    SetSize( size_t auSize )                { this->muSize = auSize; }
    size_t  GetSize(void)                           { return this->muSize; }

    int32_t      miSegmentID;
    int64_t      miQueueGlobalID;
    char         macQueueName[AI_MAX_NAME_LEN];
    size_t       muSize;
};

/** 
 * @brief  segment storage class interface
 */
class IBaseStore
{

public:
    virtual ~IBaseStore(void) {};

    virtual int Initialize(void) = 0;

    virtual void Close(void) = 0;

    virtual int Alloc( int32_t aiSegmentID, int64_t aiQueueGlobalID, char const* apcQueueName ) = 0;

    virtual int Free( int32_t aiSegmentID ) = 0;

    virtual int Put(
        int32_t aiSegmentID, 
        int64_t aiQueueGlobalID, 
        char const* apcQueueName, 
        AIChunkEx const& aoData ) = 0;
    
    virtual int Get(
        int32_t aiSegmentID,
        int64_t aiQueueGlobalID,
        char const* apcQueueName, 
        int32_t aiWhenSec, 
        AIChunkEx& aoData ) = 0;

    virtual int GetSegmentInfo( int32_t aiSegmentID, CSegmentInfo& aoSegmentInfo ) = 0;
    
    virtual int32_t GetSegmentCount(void) = 0;

    virtual int32_t GetSegmentCapacity(void) = 0;
};

AI_GFQ_NAMESPACE_END

#endif//GFQ_BASESTORAGE_H

