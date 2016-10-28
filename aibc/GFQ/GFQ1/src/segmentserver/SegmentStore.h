
#ifndef AI_AFQ_SEGMENT_H_2008
#define AI_AFQ_SEGMENT_H_2008

#include "gfq/GFQUtility.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "BaseStore.h"

AI_GFQ_NAMESPACE_START

#include "StoreFile.h"

//meta data
struct stMetaData
{
    stMetaData( int32_t aiCount = 0, int32_t aiSegCount = 0, size_t aiSize = 0 ) : 
    	ciCount(aiCount), ciSegCount(aiSegCount), ciSize(aiSize) {}
    int32_t ciCount;
    int32_t ciSegCount;
    size_t  ciSize;
    int64_t ciAlign; // Only use to alignment by 8 byte
};

// segment head
struct stSegmentHead
{
	stSegmentHead(void);
	void Reset(void);
	
	size_t GetSize(void);
	int64_t GetQueueGlobalID(void);
	const char* GetQueueName(void);
	
	int32_t ciSegmentID; //segment id, identity in module
	int32_t ciNodeSize;  //node size
	int64_t ciQueueGlobalID; //queue segment node global id
	char ccQueueName[AI_MAX_NAME_LEN]; //queue name
    //segment pointer
	size_t ciWritePos;
	size_t ciReadPos;
}; //segment head

// data node
struct stDataNode
{
	size_t ciDataSize;
	int32_t ciTimestamp;
	char ccData[AI_MAX_DATA_LEN];
};

class clsSegmentStore : public IBaseStore
{
public:
	clsSegmentStore( const char* apsSegmentFileDir );
	virtual ~clsSegmentStore();
	
	// Create segment store db file
	int32_t Create( const char* apsSegmentFileDir, int32_t aiCount, int32_t aiSegCount, size_t aiSize );
	
	// Load index node from index file
	int32_t Open( const char* apsSegmentFileDir );
	int32_t Clear( const char* apsSegmentFileDir );

    virtual int  Initialize(void);

	virtual void Close(void);
	
	// Alloc segment for queue, process request from master
	virtual int Alloc( int32_t aiSegIdx, int64_t aiQueueGlobalID, const char* apsQueueName );
	
	// Free segment for queue, process request from master
	virtual int Free( int32_t aiSegIdx );

	// Put data to segment
	virtual int Put( int32_t aiSegIdx, int64_t aiQueueGlobalID, const char* apsQueueName, 
		const AIChunkEx& aoData );
	
	// Get data from segment
	virtual int Get( int32_t aiSegIdx, int64_t aiQueueGlobalID, const char* apsQueueName, 
		int32_t aiWhenSec, AIChunkEx& aoData);
	
    virtual int GetSegmentInfo( int32_t aiSegmentID, CSegmentInfo& aoSegmentInfo );

	virtual int32_t GetSegmentCount();
	
	virtual int32_t GetSegmentCapacity();
	
	//stSegmentHead* GetSegmentHead( int32_t aiSegIdx );

protected:
	// Check is permission to access segment
	bool IsPermision( int32_t aiSegIdx, int64_t aiQueueGlobalID, const char* apsQueueName );

protected:
	stSegmentHead* cpoSegmentHead;
	
	stMetaData* cpoMetaData;
	
	clsStoreFile<stDataNode> coStoreFile;
	
	AIMapFile coMapFile;
	
	AIMutexLock* cpoSegmentLock;
	AIMutexLock* cpoAllocFreeLock;

    char macName[AI_MAX_NAME_LEN];
};

AI_GFQ_NAMESPACE_END

#endif //AI_AFQ_SEGMENT_H_2008
