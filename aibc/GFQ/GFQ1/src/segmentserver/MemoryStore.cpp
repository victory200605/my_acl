
#include "MemoryStore.h"

AI_GFQ_NAMESPACE_START

CMemoryStore::CMemoryStore( size_t auSegmentCount, size_t auSegmentCapacity )
    : muSegmentCount(auSegmentCount)
    , muSegmentCapacity(auSegmentCapacity)
{
}

CMemoryStore::~CMemoryStore(void)
{
}

int CMemoryStore::Initialize(void)
{
    this->moSegments.resize(this->muSegmentCount);

    for (AI_STD::vector<CSegment>::iterator loIter = this->moSegments.begin();
         loIter != this->moSegments.end(); ++loIter)
    {
        loIter->moDatas.resize(this->muSegmentCapacity);
    }

    return 0;
}

void CMemoryStore::Close(void)
{
}

int CMemoryStore::Alloc( int32_t aiSegmentID, int64_t aiQueueGlobalID, char const* apcQueueName )
{
    assert(aiSegmentID >= 0 && aiSegmentID < (int32_t)this->muSegmentCount);

    AISmartLock loGuard(this->moSegments[aiSegmentID].moLock);
    
    if (this->moSegments[aiSegmentID].miQueueGlobalID <= 0)
    {
        this->moSegments[aiSegmentID].miQueueGlobalID = aiQueueGlobalID;
        
        strncpy(this->moSegments[aiSegmentID].macQueueName,
            apcQueueName, sizeof(this->moSegments[aiSegmentID].macQueueName) );

        return AI_NO_ERROR;
    }
    else
    {
	    AI_GFQ_ERROR(
	        "Alloc [%"PRId32"] Permision denied, [Queue GlobalID=%"PRId64"]/[Queue Name=%s]/[ReadPos=%"PRIu64"]/[WritePos=%"PRIu64"]", 
	    	aiSegmentID,
	    	this->moSegments[aiSegmentID].miQueueGlobalID,
	    	this->moSegments[aiSegmentID].macQueueName,
	    	(uint64_t)this->moSegments[aiSegmentID].muReadPos,
	    	(uint64_t)this->moSegments[aiSegmentID].muWritePos );
	    	
	    return AI_ERROR_PERMISION_DENIED;
    }
}

int CMemoryStore::Free( int32_t aiSegmentID )
{
    if (aiSegmentID >= (int32_t)this->muSegmentCount)
    {
        return -1;
    }

    AISmartLock loGuard(this->moSegments[aiSegmentID].moLock);
    
    if (this->moSegments[aiSegmentID].muReadPos >= this->moSegments[aiSegmentID].muWritePos)
    {
        this->moSegments[aiSegmentID].miQueueGlobalID = -1;
        this->moSegments[aiSegmentID].muReadPos = 0;
        this->moSegments[aiSegmentID].muWritePos = 0;

        return AI_NO_ERROR;
    }
    else
    {
	    AI_GFQ_ERROR(
	        "Free [%"PRId32"] Permision denied, [Queue GlobalID=%"PRId64"]/[Queue Name=%s]/[ReadPos=%"PRIu64"]/[WritePos=%"PRIu64"]", 
	    	aiSegmentID,
	    	this->moSegments[aiSegmentID].miQueueGlobalID,
	    	this->moSegments[aiSegmentID].macQueueName,
	    	(uint64_t)this->moSegments[aiSegmentID].muReadPos,
	    	(uint64_t)this->moSegments[aiSegmentID].muWritePos );
	    	
	    return AI_ERROR_PERMISION_DENIED;
    }
}

int CMemoryStore::Put(
    int32_t aiSegmentID, 
    int64_t aiQueueGlobalID,
    char const* apcQueueName,
    AIChunkEx const& aoData )
{
    if (aiSegmentID >= (int32_t)this->muSegmentCount)
    {
        return -1;
    }

    AISmartLock loGuard(this->moSegments[aiSegmentID].moLock);
    
    size_t&   luWritePos = this->moSegments[aiSegmentID].muWritePos;
    size_t    luLength = aoData.WritePtr() - aoData.ReadPtr();
    CSegment& loSegment = this->moSegments[aiSegmentID];

    if (loSegment.miQueueGlobalID != aiQueueGlobalID
        || strcmp(loSegment.macQueueName,  apcQueueName) != 0)
    {
        AI_GFQ_DEBUG( "Permision denied, [Queue GlobalID=%"PRId64"/%"PRId64"],[Queue Name=%s/%s]", 
            loSegment.miQueueGlobalID, 
            aiQueueGlobalID, 
            loSegment.macQueueName, 
            apcQueueName );
        
    	return AI_ERROR_PERMISION_DENIED;
    }

    if (loSegment.muWritePos < this->muSegmentCapacity)
    {
        assert (sizeof(loSegment.moDatas[luWritePos].macData) >= luLength);

        memcpy(loSegment.moDatas[luWritePos].macData, aoData.ReadPtr(), luLength);
        loSegment.moDatas[luWritePos].muLength = luLength;
        loSegment.moDatas[luWritePos].miTimestamp = time(NULL);

        luWritePos++;

        return AI_NO_ERROR;
    }
    else
    {
    	return AI_ERROR_END_OF_SEGMENT;
    }
}
    
int CMemoryStore::Get(
    int32_t aiSegmentID,
    int64_t aiQueueGlobalID,
    char const* apcQueueName,
    int aiWhenSec,
    AIChunkEx& aoData )
{
    if (aiSegmentID >= (int32_t)this->muSegmentCount)
    {
        return -1;
    }
    
    AISmartLock loGuard(this->moSegments[aiSegmentID].moLock);

    CSegment& loSegment = this->moSegments[aiSegmentID];
    
    if (loSegment.miQueueGlobalID != aiQueueGlobalID
        || strcmp(loSegment.macQueueName, apcQueueName) != 0)
    {
        AI_GFQ_DEBUG( "Permision denied, [Queue GlobalID=%"PRId64"/%"PRId64"],[Queue Name=%s/%s]", 
            loSegment.miQueueGlobalID, 
            aiQueueGlobalID, 
            loSegment.macQueueName, 
            apcQueueName );
        
        return AI_ERROR_PERMISION_DENIED;
    }

    if (loSegment.muReadPos < loSegment.muWritePos)
    {
        CDataNode& loDataNode = loSegment.moDatas[loSegment.muReadPos];
        
        if (aiWhenSec > 0 && loDataNode.miTimestamp > time(NULL) - aiWhenSec )
        {
            // Have not out time record
            return AI_ERROR_NO_FOUND_OUTTIME;
        }
        
        loSegment.muReadPos++;

        if (loDataNode.muLength <= 0 || loDataNode.muLength > AI_MAX_DATA_LEN )
        {
            AI_GFQ_ERROR( "Invalid data node, [Data Size=%u]", loDataNode.muLength);
            return AI_ERROR_INVALID_DATA;
        }
        else
        {
            aoData.Resize(loDataNode.muLength);
            memcpy( aoData.WritePtr(), loDataNode.macData, loDataNode.muLength );
            aoData.WritePtr(loDataNode.muLength);
      
            return AI_NO_ERROR;
        }
    }
    else
    {
    	if (loSegment.muReadPos >= this->muSegmentCapacity)
    	{
    	    // end of segment
    	    return AI_ERROR_END_OF_SEGMENT;
    	}
        else
        {
    	    // segment empty
    	    return AI_ERROR_EMPTY_OF_SEGMENT;
        }
    }
}

int CMemoryStore::GetSegmentInfo( int32_t aiSegmentID, CSegmentInfo& aoSegmentInfo )
{
    if (aiSegmentID >= (int32_t)this->muSegmentCount)
    {
        return -1;
    }

    AISmartLock loGuard(this->moSegments[aiSegmentID].moLock);

    aoSegmentInfo.SetSegmentID(aiSegmentID);
    aoSegmentInfo.SetQueueGlobalID(this->moSegments[aiSegmentID].miQueueGlobalID);
    aoSegmentInfo.SetQueueName(this->moSegments[aiSegmentID].macQueueName);
    aoSegmentInfo.SetSize(this->moSegments[aiSegmentID].muWritePos - this->moSegments[aiSegmentID].muReadPos);

    return 0;
}
    
int32_t CMemoryStore::GetSegmentCount(void)
{
    return this->muSegmentCount;
}

int32_t CMemoryStore::GetSegmentCapacity(void)
{
    return this->muSegmentCapacity;
}

AI_GFQ_NAMESPACE_END

