//=============================================================================
/**
 * \file    QueueMgr.h
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: QueueMgr.h,v 1.2 2010/12/20 08:19:04 daizh Exp $
 *
 * History
 * 2008.04.04 first release by DZH
 */
//=============================================================================
#ifndef AI_GFQ_QUEUEMGR_H_2008
#define AI_GFQ_QUEUEMGR_H_2008

#include "ModuleMgr.h"
#include "gfq/GFQUtility.h"

// start namespace
AI_GFQ_NAMESPACE_START

// queue head, for queue name hash array
struct stQueueHead
{
	stQueueHead( const char* apsQueueName, int32_t aiQueueID, size_t aiQueueCapacity );

	int64_t ciQueueGlobalID; // auto increase
	int32_t ciQueueID;
	int32_t ciSegmentCnt;
	size_t  ciQueueCapacity;
	char    ccQueueName[AI_MAX_NAME_LEN];
	
	LISTHEAD coQueueChild;
	LISTHEAD coQueueListChild;
	LISTHEAD coSegmentHead;
};

// queue name hash array 
struct stQueueNameHash
{
    stQueueNameHash();
    
    LISTHEAD coQueueHead;
};

// queue element node
struct stSegmentNode
{
	stSegmentNode();
	
	// unlink from queue
	void Unlink();
	
	int32_t ciSegmentID;
	int64_t ciQueueGlobalID;
	size_t  ciSegmentSize;
	size_t  ciSegmentCapacity;

	stQueueHead* cpoQueueHead;
	LISTHEAD     coSegmentChild;
};

// implememt clsModuleMgr type
typedef clsModuleMgr<stSegmentNode> TModuleMgr;

// queue head handle define
typedef stQueueHead* TQueueHandle;
typedef stQueueHead& TQueueRef;
#define INVALID_QUEUE_HANDLE NULL

// Access queue head single iterator
// This iterator can't random assert
class clsQueueIter
{
public:
	// Construct funcion
	clsQueueIter( LISTHEAD* apoNode, LISTHEAD* apoHead ) : cpoNode(apoNode), cpoHead(apoHead)
	{
		assert( apoNode != NULL && apoHead != NULL );
	}
	
	// ++ Operator overwrite
	void operator ++ ( int )
	{
		if ( cpoNode != NULL && cpoNode != cpoHead )
		{
			cpoNode = cpoNode->cpNext;
		}
	}
	
	// == Operator overwrite
	bool operator == ( const clsQueueIter& apoIter )
	{
		return cpoNode == apoIter.cpoNode && cpoHead == apoIter.cpoHead;
	}
	
	// -> Operator overwrite
	TQueueHandle operator -> ()
	{
		return &(**this);
	}
	
	// * Operator overwrite
	TQueueRef operator * ()
	{
		stQueueHead* lpoQueueHead = AI_GET_STRUCT_PTR( cpoNode , stQueueHead, coQueueListChild );
		return *lpoQueueHead;
	}
	
	// TQueueHandle Type convert
	operator TQueueHandle ()
	{
		return &(**this);
	}
	
protected:
	LISTHEAD* cpoNode;
	LISTHEAD* cpoHead;
};

// sgement module, do for segment allocate and recycle
class clsQueueMgr
{
public:
	typedef clsQueueIter TIterator;
	
public:
	clsQueueMgr();
	virtual ~clsQueueMgr();
	
	// Search module by module name
	TQueueHandle SearchQueue( const char* apsQueueName );
	
	// Create queue
	TQueueHandle CreateQueue( const char* apsQueueName, int32_t aiCapacity );
	
	// Delete queue by queue name
	int32_t DelQueue( const char* apsQueueName );
	
	// Delete queue by queue handle
	int32_t DelQueue( TQueueHandle atQueueHandle );
	
	// Get queue writeable segment, tail of queue segment link
	int32_t GetWriteSegment( TQueueHandle atQueueHandle, stSegmentNode** appoSegment );
	
	// Get queue readable segment, head of queue segment link
	int32_t GetReadSegment( TQueueHandle atQueueHandle, stSegmentNode** appoSegment );
	
	// Insert segment into queue, use for queue reconvert
	int32_t InsertSegment( TQueueHandle atQueueHandle, stSegmentNode* apoSegment );
	
	// Pop segment from queue, segment will be free back module
	int32_t PopSegment( TQueueHandle atQueueHandle );
	
	// Push segment from queue, segment will be alloc from module
	int32_t PushSegment( TQueueHandle atQueueHandle );
	
	// Check segment is the last
	bool IsLastSegment( const stSegmentNode* apoSegmentNode );
	
	// Start get operator
	static int32_t GetSegmentID( const stSegmentNode* apoSegment );
	static int64_t GetQueueGlobalID( const stSegmentNode* apoSegment );
	static int32_t GetQueueID( const stSegmentNode* apoSegment );
	static TQueueHandle GetQueueHandle( const stSegmentNode* apoSegment );
	
	size_t GetQueueCount();
	size_t GetQueuePreciseSize( TQueueHandle atQueueHandle );
	size_t GetQueueSize( TQueueHandle atQueueHandle );
	size_t GetQueueCapacity( TQueueHandle atQueueHandle );
	// End get operator
	
	// For queue head iterator
	TIterator QueueBegin();
	TIterator QueueEnd();

protected:
	stQueueNameHash* cpoQueueHash;
	
	LISTHEAD coQueueList;
	size_t ciQueueNum;
	
	int32_t ciQueueID;
};

///////////////////////////////// Global Module function ///////////////////////////////
TModuleMgr* GetModuleMgr();

//end namespace
AI_GFQ_NAMESPACE_END

#endif //AI_GFQ_QUEUE_MGR_H_2008
