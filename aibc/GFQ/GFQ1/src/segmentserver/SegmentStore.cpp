
#include "SegmentStore.h"

AI_GFQ_NAMESPACE_START

//---------------------------------------------------------------//
stSegmentHead::stSegmentHead(void)
{
	memset( this, 0, sizeof(stSegmentHead) );
	ciQueueGlobalID = -1;
}

void stSegmentHead::Reset(void)
{
    memset( ccQueueName, 0, AI_MAX_NAME_LEN );
	ciWritePos      = 0;
	ciReadPos       = 0;
	ciNodeSize      = 0;
	ciQueueGlobalID = -1;
}

size_t stSegmentHead::GetSize(void)
{
    return ( ciWritePos - ciReadPos <= 0 ? 0 : ciWritePos - ciReadPos);
}

int64_t stSegmentHead::GetQueueGlobalID(void)
{
    return ciQueueGlobalID;
}

const char* stSegmentHead::GetQueueName(void)
{
    return ccQueueName;
}

//---------------------------------------------------------------//
clsSegmentStore::clsSegmentStore( char const* apcSegmentFileDir ) 
    : cpoSegmentHead(NULL)
    , cpoMetaData(NULL)
    , cpoSegmentLock(NULL)
{
    strncpy(this->macName, apcSegmentFileDir, sizeof(this->macName) );
}

clsSegmentStore::~clsSegmentStore()
{
}

int clsSegmentStore::Create( const char* apsSegmentFileDir, int32_t aiCount, int32_t aiSegCount, size_t aiSize )
{
	char lcFileName[AI_MAX_NAME_LEN];
	snprintf( lcFileName, AI_MAX_NAME_LEN, "%s/", apsSegmentFileDir );
	AIDir::Make(lcFileName);
	snprintf( lcFileName, AI_MAX_NAME_LEN, "%s/__segment", apsSegmentFileDir );
	
    stMetaData loMetaData( aiCount, aiSegCount, aiSize);
    size_t liSize = loMetaData.ciSegCount * loMetaData.ciCount;
    
    if ( liSize * sizeof( stSegmentHead ) + sizeof(stMetaData) > AI_MAX_S_FILE_SIZE )
    {//sorry, out of range, I can't support it
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    AIFile loFile;
    if ( loFile.Open( lcFileName, O_RDWR | O_CREAT | O_TRUNC, 0766 ) != AIFile::AI_NO_ERROR )
    {//Can't not open file, are you sure the path is right
        return AI_ERROR_OPEN_FILE;
    }

    if ( loFile.Write( &loMetaData, sizeof(stMetaData) ) != sizeof(stMetaData) )
    {//oh,no. please check file system
        return AI_ERROR_WRITE_FILE;
    }
    
    stSegmentHead loSegment;
    for( size_t liN = 0; liN < liSize; liN++ )
    {
    	loSegment.ciSegmentID = liN;
        if ( loFile.Write( &loSegment, sizeof(stSegmentHead) ) != sizeof(stSegmentHead) )
        {//oh,no. please check file system
            return AI_ERROR_WRITE_FILE;
        }
    }
    loFile.Close();
    
    snprintf( lcFileName, AI_MAX_NAME_LEN, "%s/__data", apsSegmentFileDir );
    
    return coStoreFile.Create( lcFileName, aiCount, aiSegCount * aiSize );
}

//load index node from index file
int clsSegmentStore::Open( const char* apsSegmentFileDir )
{
    int32_t liRetCode = 0;
	char    lcFileName[AI_MAX_NAME_LEN];
	snprintf( lcFileName, AI_MAX_NAME_LEN, "%s/__segment", apsSegmentFileDir );
	
	if ( coMapFile.Map( lcFileName, PROT_READ | PROT_WRITE, MAP_SHARED ) != AIMapFile::AI_NO_ERROR )
	{//oh,map file fail
		return AI_ERROR_MAP_FILE;
	}
	
	cpoMetaData = (stMetaData*)coMapFile.GetAddr();
	size_t liSize = ( cpoMetaData->ciSegCount * cpoMetaData->ciCount ) * sizeof(stSegmentHead) + sizeof(stMetaData);
	if ( liSize != coMapFile.GetSize() )
	{//oh,may be the file was destroyed
		return AI_ERROR_INVALID_FILE;
	}
	
	// Convert segment head struct pointer
	cpoSegmentHead = (stSegmentHead*)( (stMetaData*)coMapFile.GetAddr() + 1 );
    
    snprintf( lcFileName, AI_MAX_NAME_LEN, "%s/__data", apsSegmentFileDir );
    
    liRetCode = coStoreFile.Open( lcFileName, cpoMetaData->ciCount, cpoMetaData->ciSegCount * cpoMetaData->ciSize );
    if (liRetCode == 0)
    {
        AI_NEW_ARRAY_ASSERT(cpoSegmentLock, AIMutexLock, cpoMetaData->ciSegCount * cpoMetaData->ciCount );
        AI_NEW_ARRAY_ASSERT(cpoAllocFreeLock, AIMutexLock, cpoMetaData->ciSegCount * cpoMetaData->ciCount );
    }
    
    return liRetCode;
}

int clsSegmentStore::Clear( const char* apsSegmentFileDir )
{
	int32_t liRet = AI_NO_ERROR;
	
	if ( ( liRet = Open( apsSegmentFileDir ) ) != AI_NO_ERROR )
	{
		return liRet;
	}
	
	size_t liSize = ( cpoMetaData->ciSegCount * cpoMetaData->ciCount );
	for ( size_t liIt = 0; liIt < liSize; liIt++ )
	{
		cpoSegmentHead[liIt].Reset();
	}
	
	return AI_NO_ERROR;
}

int clsSegmentStore::Initialize(void)
{
    return this->Open(this->macName);
}

void clsSegmentStore::Close()
{
	coMapFile.Unmap();
	
	cpoSegmentHead = NULL;
	cpoMetaData = NULL;
	
	coStoreFile.Close();
	
	AI_DELETE_ARRAY(cpoSegmentLock);
	AI_DELETE_ARRAY(cpoAllocFreeLock);
}

int clsSegmentStore::Alloc( int32_t aiSegIdx, int64_t aiQueueGlobalID, const char* apsQueueName )
{
	assert( cpoMetaData != NULL && cpoSegmentHead != NULL );
	// Segment count
	size_t liSegmentCount = cpoMetaData->ciSegCount * cpoMetaData->ciCount;
	
	assert( aiSegIdx >= 0 && aiSegIdx < (int64_t)liSegmentCount );
	
	// lock scope
	AISmartLock loLock(cpoAllocFreeLock[aiSegIdx]);
	
	if (cpoSegmentHead[aiSegIdx].ciQueueGlobalID > 0)
	{
	    AI_GFQ_ERROR(
	        "Alloc [%"PRId32"] Permision denied, [Queue GlobalID=%"PRId64"]/[Queue Name=%s]/[ReadPos=%"PRIu64"]/[WritePos=%"PRIu64"]", 
	    	aiSegIdx,
	    	cpoSegmentHead[aiSegIdx].ciQueueGlobalID,
	    	cpoSegmentHead[aiSegIdx].ccQueueName,
	    	(uint64_t)cpoSegmentHead[aiSegIdx].ciReadPos,
	    	(uint64_t)cpoSegmentHead[aiSegIdx].ciWritePos );
	    	
	    return AI_ERROR_PERMISION_DENIED;
	}

	// Reset segment
	cpoSegmentHead[aiSegIdx].Reset();

	// Set for queue
	cpoSegmentHead[aiSegIdx].ciQueueGlobalID = aiQueueGlobalID;
	StringCopy( cpoSegmentHead[aiSegIdx].ccQueueName, apsQueueName, AI_MAX_NAME_LEN );
	
	return AI_NO_ERROR;
}

int clsSegmentStore::Free( int32_t aiSegIdx )
{
    assert( cpoMetaData != NULL && cpoSegmentHead != NULL );
	// Segment count
	size_t liSegmentCount = cpoMetaData->ciSegCount * cpoMetaData->ciCount;
	
	assert( aiSegIdx >= 0 && aiSegIdx < (int64_t)liSegmentCount );
	
	// lock scope
	AISmartLock loLock(cpoAllocFreeLock[aiSegIdx]);
	
	if (cpoSegmentHead[aiSegIdx].ciReadPos < cpoSegmentHead[aiSegIdx].ciWritePos)
	{
	    AI_GFQ_ERROR(
	        "Free [%"PRId32"] Permision denied, [Queue GlobalID=%"PRId64"]/[Queue Name=%s]/[ReadPos=%"PRIu64"]/[WritePos=%"PRIu64"]", 
	    	aiSegIdx,
	    	cpoSegmentHead[aiSegIdx].ciQueueGlobalID,
	    	cpoSegmentHead[aiSegIdx].ccQueueName,
	    	(uint64_t)cpoSegmentHead[aiSegIdx].ciReadPos,
	    	(uint64_t)cpoSegmentHead[aiSegIdx].ciWritePos );
	    	
	    return AI_ERROR_PERMISION_DENIED;
	}

	// Reset segment
	cpoSegmentHead[aiSegIdx].Reset();
	
	return AI_NO_ERROR;
}

bool clsSegmentStore::IsPermision( int32_t aiSegIdx, int64_t aiQueueGlobalID, const char* apsQueueName )
{
	if ( cpoSegmentHead[aiSegIdx].ciQueueGlobalID == aiQueueGlobalID &&
		 strcmp( cpoSegmentHead[aiSegIdx].ccQueueName, apsQueueName ) == 0 )
	{
		return true;
	}
	
	AI_GFQ_DEBUG( "Permision denied, [Queue GlobalID=%"PRId64"/%"PRId64"],[Queue Name=%s/%s]", 
		cpoSegmentHead[aiSegIdx].ciQueueGlobalID, aiQueueGlobalID, cpoSegmentHead[aiSegIdx].ccQueueName, apsQueueName );
	
	return false;
}

int clsSegmentStore::Put( int32_t aiSegIdx, int64_t aiQueueGlobalID, const char* apsQueueName, 
		const AIChunkEx& aoData )
{
    assert( cpoMetaData != NULL && cpoSegmentHead != NULL );
    
    // Segment count
	int32_t liSegmentCount = cpoMetaData->ciSegCount * cpoMetaData->ciCount;
	assert( aiSegIdx >= 0 && aiSegIdx < liSegmentCount );
	
	int32_t liRet = AI_NO_ERROR;
	int32_t liIdx = 0;
	
    // lock scope
	AISmartLock loLock(cpoSegmentLock[aiSegIdx]);
	
	{
	    AISmartLock loLock(cpoAllocFreeLock[aiSegIdx]);
	    
    	if ( !IsPermision( aiSegIdx, aiQueueGlobalID, apsQueueName ) )
    	{
    	    return AI_ERROR_PERMISION_DENIED;
    	}
    	
    	size_t liWritePos = cpoSegmentHead[aiSegIdx].ciWritePos;
    	liIdx = aiSegIdx * cpoMetaData->ciSize + liWritePos;
    	if ( liWritePos >= cpoMetaData->ciSize )
    	{
    	    return AI_ERROR_END_OF_SEGMENT;
    	}
    	
    	cpoSegmentHead[aiSegIdx].ciWritePos++;
    }
	
	stDataNode loDataNode;
	loDataNode.ciDataSize  = aoData.WritePtr() - aoData.ReadPtr();
	loDataNode.ciTimestamp = time(NULL);
	memcpy( loDataNode.ccData, aoData.ReadPtr(), loDataNode.ciDataSize );

	if ( ( liRet = coStoreFile.Write( liIdx, loDataNode ) ) != AI_NO_ERROR )
	{
	    AI_GFQ_ERROR( 
            "Write file fail, MSGCODE:%d, MSG:%s, SYSMSG:%s [Index=%d]/[Size=%d]/[FileCount=%d]/[FileSize=%d]", 
            liRet,
            StrError(liRet),
            strerror(errno),
            liIdx,
            sizeof(loDataNode),
            coStoreFile.GetCount(),
            coStoreFile.GetSize() );
		    
	    //AISmartLock loLock(cpoAllocFreeLock[aiSegIdx]);
	    //case 1
	    //{
	    //   call Free(); fail, because ReadPos must less than WritePos
	    //   WritePos--; ok
	    //}
	    //case 2
	    //{
	    //   WritePos--; ok
	    //   call Free() and reset ReadPos/WritePos; ok, if ReadPos equeue WritePos
	    //}
	    //case 3
	    //{
	    //   WritePos--; ok
	    //   call Free(); fail, if ReadPos less than WritePos
	    //}
	    
	    cpoSegmentHead[aiSegIdx].ciWritePos--;
	}

	return liRet;
}

int clsSegmentStore::Get( int32_t aiSegIdx, int64_t aiQueueGlobalID, const char* apsQueueName, 
        int32_t aiWhenSec, AIChunkEx& aoData )
{
    assert( cpoMetaData != NULL && cpoSegmentHead != NULL );
    // Segment count
	int32_t liSegmentCount = cpoMetaData->ciSegCount * cpoMetaData->ciCount;
	assert( aiSegIdx >= 0 && aiSegIdx < liSegmentCount );
	
    // lock scope
	AISmartLock loLock(cpoSegmentLock[aiSegIdx]);
	
	int32_t liRet = AI_NO_ERROR;
	int32_t liIdx = 0;

	{
	    AISmartLock loLock(cpoAllocFreeLock[aiSegIdx]);
	    
    	size_t  liReadPos = cpoSegmentHead[aiSegIdx].ciReadPos;
    	liIdx     = aiSegIdx * cpoMetaData->ciSize + liReadPos;
    	
    	if ( !IsPermision( aiSegIdx, aiQueueGlobalID, apsQueueName ) )
    	{
    		return AI_ERROR_PERMISION_DENIED;
    	}
    	
    	if ( liReadPos >= cpoMetaData->ciSize )
    	{
    	    // end of segment
    	    return AI_ERROR_END_OF_SEGMENT;
    	}
    	
    	if ( liReadPos >= cpoSegmentHead[aiSegIdx].ciWritePos )
    	{
    	    // segment empty
    	    return AI_ERROR_EMPTY_OF_SEGMENT;
    	}
	}
	
	stDataNode loDataNode;
	if ( ( liRet = coStoreFile.Read( liIdx, loDataNode ) ) != AI_NO_ERROR )
	{
		AI_GFQ_ERROR( 
            "Read file fail, MSGCODE:%d, MSG:%s, SYSMSG:%s [Index=%d]/[Size=%d]/[FileCount=%d]/[FileSize=%d]", 
            liRet,
            StrError(liRet),
            strerror(errno),
            liIdx,
            sizeof(loDataNode),
            coStoreFile.GetCount(),
            coStoreFile.GetSize() );
		return liRet;
	}
	
	if ( aiWhenSec > 0 && loDataNode.ciTimestamp > time(NULL) - aiWhenSec )
	{
		// Have not out time record
		return AI_ERROR_NO_FOUND_OUTTIME;
	}
	
	{
	    //AISmartLock loLock(cpoAllocFreeLock[aiSegIdx]);
	    //case 1
	    //{
	    //   call Free(); fail, because ReadPos must less than WritePos
	    //   ReadPos++; ok
	    //}
	    //case 2
	    //{
	    //   ReadPos++; ok
	    //   call Free() and reset ReadPos/WritePos; ok, if ReadPos equeue WritePos
	    //}
	    //case 3
	    //{
	    //   ReadPos++; ok
	    //   call Free(); fail, if ReadPos less than WritePos
	    //}
	    
	    cpoSegmentHead[aiSegIdx].ciReadPos++;
	}

	if ( loDataNode.ciDataSize <= 0 || loDataNode.ciDataSize > AI_MAX_DATA_LEN )
	{
		AI_GFQ_ERROR( "Invalid data node, [Data Size=%d]", loDataNode.ciDataSize);
		return AI_ERROR_INVALID_DATA;
	}
	
	aoData.Resize( loDataNode.ciDataSize );
	memcpy( aoData.WritePtr(), loDataNode.ccData, loDataNode.ciDataSize );
	aoData.WritePtr(loDataNode.ciDataSize);
	
	return AI_NO_ERROR;
}

int32_t clsSegmentStore::GetSegmentCount()
{
	assert( cpoMetaData != NULL );
	
	return cpoMetaData->ciCount * cpoMetaData->ciSegCount;
}

int32_t clsSegmentStore::GetSegmentCapacity()
{
    assert( cpoMetaData != NULL );
    
	return cpoMetaData->ciSize;
}

int clsSegmentStore::GetSegmentInfo( int32_t aiSegIdx, CSegmentInfo& aoSegmentInfo )
{
	assert( cpoSegmentHead != NULL && aiSegIdx >= 0 && aiSegIdx < GetSegmentCount() );
	
	AISmartLock loLock(cpoAllocFreeLock[aiSegIdx]);

    stSegmentHead* lpoHead = cpoSegmentHead + aiSegIdx;

    aoSegmentInfo.SetSegmentID(lpoHead->ciSegmentID);
    aoSegmentInfo.SetSize(lpoHead->ciWritePos - lpoHead->ciReadPos);
    aoSegmentInfo.SetQueueGlobalID(lpoHead->ciQueueGlobalID);
    aoSegmentInfo.SetQueueName(lpoHead->ccQueueName);

    return 0;
}

//stSegmentHead* clsSegmentStore::GetSegmentHead( int32_t aiSegIdx )
//{
//	assert( cpoSegmentHead != NULL && aiSegIdx >= 0 && aiSegIdx < GetSegmentCount() );
//	
//	return cpoSegmentHead + aiSegIdx;
//}

AI_GFQ_NAMESPACE_END
