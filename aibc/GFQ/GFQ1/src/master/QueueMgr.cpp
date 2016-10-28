//=============================================================================
/**
 * \file    QueueMgr.cpp
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: QueueMgr.cpp,v 1.2 2012/03/07 03:11:50 daizh Exp $
 *
 * History
 * 2008.04.04 first release by DZH
 */
//=============================================================================
#include "QueueMgr.h"
#include "gfq/GFQProtocol.h"

// start namespace
AI_GFQ_NAMESPACE_START

//-------------------------queue head-----------------------//
stQueueHead::stQueueHead( const char* apsQueueName, int32_t aiQueueID, size_t aiQueueCapacity ) :
    ciQueueGlobalID(time(NULL)),
    ciQueueID(aiQueueID),
    ciSegmentCnt(0),
    ciQueueCapacity(aiQueueCapacity)
{
	StringCopy( ccQueueName, apsQueueName, AI_MAX_NAME_LEN );
	
	memset( &coQueueChild, 0, sizeof(LISTHEAD) );
	memset( &coQueueListChild, 0, sizeof(LISTHEAD) );
	
	ai_init_list_head( &coSegmentHead );
}

//-------------------------queue name hash array---------------------//
stQueueNameHash::stQueueNameHash()
{
	ai_init_list_head( &coQueueHead );
}

//-----------------------------Segment node-----------------------------//
stSegmentNode::stSegmentNode() : ciSegmentID(0), ciQueueGlobalID(0), cpoQueueHead(NULL)
{
	memset( &coSegmentChild, 0, sizeof(LISTHEAD) );
}

// unlink from queue
void stSegmentNode::Unlink()
{
    if ( cpoQueueHead == NULL ) return;
	
	// Decrease segment count
	if ( coSegmentChild.cpNext != NULL && coSegmentChild.cpPrev != NULL )
	{
	    cpoQueueHead->ciSegmentCnt--;
	}
	
    AI_DUAL_UNLINK( coSegmentChild );
}

//----------------------------- Queue -----------------------------//
clsQueueMgr::clsQueueMgr() : cpoQueueHash(NULL),ciQueueNum(0),ciQueueID(0)
{
	AI_NEW_ARRAY( cpoQueueHash, stQueueNameHash, AI_QUEUE_HASH_SIZE );

    ai_init_list_head( &coQueueList );
}

clsQueueMgr::~clsQueueMgr()
{
	AI_DELETE( cpoQueueHash );
}

TQueueHandle clsQueueMgr::SearchQueue( const char* apsQueueName )
{
	int32_t liIndex = HashString( apsQueueName, strlen(apsQueueName) ) %  AI_QUEUE_HASH_SIZE;
	LISTHEAD* lpoHead = &cpoQueueHash[ liIndex ].coQueueHead;
	LISTHEAD* lpoCurr = lpoHead->cpNext;
	for( ; lpoCurr != NULL && lpoCurr != lpoHead; lpoCurr = lpoCurr->cpNext )
	{
		stQueueHead* lpoQueueHead = AI_GET_STRUCT_PTR( lpoCurr , stQueueHead, coQueueChild );
		if ( strcmp( lpoQueueHead->ccQueueName, apsQueueName ) == 0 )
		{
			return lpoQueueHead;
		}
	}
	return INVALID_QUEUE_HANDLE;
}

TQueueHandle clsQueueMgr::CreateQueue( const char* apsQueueName, int32_t aiCapacity )
{
	int32_t liIndex = HashString( apsQueueName, strlen(apsQueueName) ) %  AI_QUEUE_HASH_SIZE;
	LISTHEAD* lpoHead = &cpoQueueHash[ liIndex ].coQueueHead;
	
	stQueueHead* lpoQueueHead = NULL;
	AI_NEW_INIT_ASSERT( lpoQueueHead, stQueueHead, apsQueueName, ciQueueID++, aiCapacity );
	
	ai_list_add_tail( &lpoQueueHead->coQueueChild, lpoHead );
	ai_list_add_tail( &lpoQueueHead->coQueueListChild, &coQueueList );
	
	ciQueueNum++;
	
	return lpoQueueHead;
}

int32_t clsQueueMgr::DelQueue( TQueueHandle atQueueHandle )
{
	assert( atQueueHandle != INVALID_QUEUE_HANDLE );
	int32_t liRet = AI_NO_ERROR;
	
	while ( ( liRet = PopSegment( atQueueHandle ) ) == AI_NO_ERROR ) {}
	
	if ( liRet != AI_ERROR_END_OF_QUEUE )
	{
		// Oh, can't delete segment
		return liRet;
	}
	
    AI_DUAL_UNLINK( atQueueHandle->coQueueChild );
    AI_DUAL_UNLINK( atQueueHandle->coQueueListChild );
    
    AI_DELETE( atQueueHandle );
    
    ciQueueNum--;
    
    return AI_NO_ERROR;
}

int32_t clsQueueMgr::DelQueue( const char* apsQueueName )
{
	stQueueHead* lpoQueueHead = NULL;
	if ( ( lpoQueueHead = SearchQueue( apsQueueName ) ) != NULL )
	{// Sorry, queue unexisted
		return AI_ERROR_NO_FOUND_QUEUE;
	}
	
	return DelQueue( lpoQueueHead );
}

int32_t clsQueueMgr::GetWriteSegment( TQueueHandle atQueueHandle, stSegmentNode** appoSegment )
{
	assert( atQueueHandle != INVALID_QUEUE_HANDLE );
	
	LISTHEAD* lpoTmp = atQueueHandle->coSegmentHead.cpPrev;
	if ( lpoTmp == NULL || lpoTmp == &atQueueHandle->coSegmentHead )
	{
		return AI_ERROR_END_OF_QUEUE;
	}
	
	*appoSegment = AI_GET_STRUCT_PTR( lpoTmp , stSegmentNode, coSegmentChild );
	
	return AI_NO_ERROR;
}

int32_t clsQueueMgr::GetReadSegment( TQueueHandle atQueueHandle, stSegmentNode** appoSegment )
{
	assert( atQueueHandle != INVALID_QUEUE_HANDLE );
	
	LISTHEAD* lpoTmp = atQueueHandle->coSegmentHead.cpNext;
	if ( lpoTmp == NULL || lpoTmp == &atQueueHandle->coSegmentHead )
	{
		return AI_ERROR_END_OF_QUEUE;
	}
	
	*appoSegment = AI_GET_STRUCT_PTR( lpoTmp , stSegmentNode, coSegmentChild );
	
	return AI_NO_ERROR;
}

int32_t clsQueueMgr::InsertSegment( TQueueHandle atQueueHandle, stSegmentNode* apoSegment )
{
	assert( atQueueHandle != INVALID_QUEUE_HANDLE && apoSegment != NULL );
	
	apoSegment->cpoQueueHead = atQueueHandle;
	LISTHEAD* lpoHead = &atQueueHandle->coSegmentHead;
	LISTHEAD* lpoCurr = lpoHead->cpNext;
	for( ; lpoCurr != NULL && lpoCurr != lpoHead; lpoCurr = lpoCurr->cpNext )
	{
		stSegmentNode* lpoSegmentNode = AI_GET_STRUCT_PTR( lpoCurr , stSegmentNode, coSegmentChild );
		if ( apoSegment->ciQueueGlobalID > lpoSegmentNode->ciQueueGlobalID )
		{
			continue;
		}
		break;
	}

	ai_list_add_mid( &apoSegment->coSegmentChild, lpoCurr->cpPrev, lpoCurr );
	
	// Increase segment count
	atQueueHandle->ciSegmentCnt++;
	
	return AI_NO_ERROR;
}

int32_t clsQueueMgr::PopSegment( TQueueHandle atQueueHandle )
{
	assert( atQueueHandle != INVALID_QUEUE_HANDLE );
	int32_t liRet = AI_NO_ERROR;

	if ( ai_list_is_empty( &atQueueHandle->coSegmentHead ) )
	{
		return AI_ERROR_END_OF_QUEUE;
	}

    stSegmentNode* lpoFirstSegment = AI_GET_STRUCT_PTR( 
        atQueueHandle->coSegmentHead.cpNext,
        stSegmentNode,
        coSegmentChild );

	{
		//TO DO
		//update module segment flag, may be modify by socket
		TModuleHandle ltMHandle = GetModuleMgr()->GetModuleHandle( lpoFirstSegment );
		//request
		clsRequestFree loReqFree;
		loReqFree.SetSegmentID( lpoFirstSegment->ciSegmentID );
		
		clsResponseFree loRespFree;
		
		liRet = DoRequestTo( *GetModuleMgr()->GetModuleServer(ltMHandle), AI_GFQ_REQUEST_FREE, loReqFree, loRespFree );
		if (liRet != AI_NO_ERROR)
		{
		    GetModuleMgr()->MasterFreeErrorObserver(ltMHandle, liRet);
    	}
    	else
    	{
    	    LISTHEAD* lpoTemp = NULL;
            ai_list_del_head( lpoTemp, &atQueueHandle->coSegmentHead );
            
            lpoFirstSegment = AI_GET_STRUCT_PTR( lpoTemp , stSegmentNode, coSegmentChild );
            lpoFirstSegment->Unlink();
            lpoFirstSegment->ciQueueGlobalID = -1;
            lpoFirstSegment->cpoQueueHead    = NULL;
            lpoFirstSegment->ciSegmentSize   = 0;
            
            GetModuleMgr()->FreeSegment( lpoFirstSegment );
            atQueueHandle->ciSegmentCnt--;
    	}
	}

	return AI_NO_ERROR;
}

int32_t clsQueueMgr::PushSegment( TQueueHandle atQueueHandle )
{
	assert( atQueueHandle != INVALID_QUEUE_HANDLE );
	int32_t liRet = AI_NO_ERROR;
	
	stSegmentNode* lpoSegment = NULL;
	
	size_t liQueueSize = GetQueueSize( atQueueHandle );
	if ( liQueueSize >= atQueueHandle->ciQueueCapacity )
	{
		return AI_ERROR_NO_ENOUGH_SPACE;
	}
	
	while( true )
	{
		lpoSegment = GetModuleMgr()->AllocSegment();
		if ( lpoSegment == NULL )
		{
			return AI_ERROR_NO_ENOUGH_SPACE;
		}
		
		lpoSegment->ciQueueGlobalID = atQueueHandle->ciQueueGlobalID++;
		lpoSegment->cpoQueueHead    = atQueueHandle;
		lpoSegment->ciSegmentSize   = lpoSegment->ciSegmentCapacity;
		
		{
			//TO DO
			//update module segment flag, may be modify by socket
			TModuleHandle ltMHandle = GetModuleMgr()->GetModuleHandle( lpoSegment );
			//request
			clsRequestAlloc loAlloc;
			loAlloc.SetSegmentID( lpoSegment->ciSegmentID );
			loAlloc.SetQueueGlobalID( lpoSegment->ciQueueGlobalID );
			loAlloc.SetQueueName( lpoSegment->cpoQueueHead->ccQueueName );
			clsResponseAlloc loRespAloc;
			
			liRet = DoRequestTo( *GetModuleMgr()->GetModuleServer(ltMHandle), AI_GFQ_REQUEST_ALLOC, loAlloc, loRespAloc );
			//liRet = DoRequestTo( *GetModuleMgr()->GetModuleServer(ltMHandle), AI_GFQ_REQUEST_ALLOC, loAlloc );
			if ( liRet != AI_NO_ERROR )
			{
				GetModuleMgr()->MasterErrorObserver( ltMHandle, liRet );
				continue;
			}
		}
		
		ai_list_add_tail( &lpoSegment->coSegmentChild, &atQueueHandle->coSegmentHead );
		atQueueHandle->ciSegmentCnt++;
		
		return AI_NO_ERROR;	 
	}

	return AI_NO_ERROR;
}

bool clsQueueMgr::IsLastSegment( const stSegmentNode* apoSegmentNode )
{
	assert( apoSegmentNode != NULL );
	
	LISTHEAD* lpoTmp = apoSegmentNode->coSegmentChild.cpNext;
	if ( lpoTmp == &(apoSegmentNode->cpoQueueHead->coSegmentHead) )
	{
		return true;
	}
	
	return false;
}

int32_t clsQueueMgr::GetSegmentID( const stSegmentNode* apoSegment )
{
	assert( apoSegment != NULL );
	
	return apoSegment->ciSegmentID;
}

int64_t clsQueueMgr::GetQueueGlobalID( const stSegmentNode* apoSegment )
{
	assert( apoSegment != NULL );
	
	return apoSegment->ciQueueGlobalID;
}

int32_t clsQueueMgr::GetQueueID( const stSegmentNode* apoSegment )
{
	assert( apoSegment != NULL && apoSegment->cpoQueueHead != NULL );
	
	return apoSegment->cpoQueueHead->ciQueueID;
}

TQueueHandle clsQueueMgr::GetQueueHandle( const stSegmentNode* apoSegment )
{
    assert( apoSegment != NULL && apoSegment->cpoQueueHead != NULL );
	
	return apoSegment->cpoQueueHead;
}
	
size_t clsQueueMgr::GetQueuePreciseSize( TQueueHandle atQueueHandle )
{
	assert( atQueueHandle != INVALID_QUEUE_HANDLE );
	
	if ( ai_list_is_empty( &atQueueHandle->coSegmentHead ) )
	{
		return 0;
	}
	
	int32_t   liRet = AI_NO_ERROR;
	LISTHEAD* lpoFirst = atQueueHandle->coSegmentHead.cpNext;
	LISTHEAD* lpoLast  = atQueueHandle->coSegmentHead.cpPrev;
	LISTHEAD* lpoHead  = &atQueueHandle->coSegmentHead;
	size_t    liSumSize = 0;
	TModuleHandle ltDeleteModule[2] = {NULL, NULL};
        int32_t liDeleteReturnCode[2] = {0, 0};
	
	// Caculate first and last segment size by request segment server
	LISTHEAD* lpoCurr = lpoFirst;
	for (int n = 0; n < 2; n++)
	{// get first segment size
		stSegmentNode* lpoSegment = AI_GET_STRUCT_PTR( lpoCurr , stSegmentNode, coSegmentChild );
		TModuleHandle ltMHandle = GetModuleMgr()->GetModuleHandle( lpoSegment );
		//request
		clsRequestSegmentSize loReqSegSize;
		loReqSegSize.SetSegmentID( lpoSegment->ciSegmentID );
		clsResponseSegmentSize loRespSegSize;
		
		//OP_RETRY( liRet = DoRequestTo( *GetModuleMgr()->GetModuleServer(ltMHandle), AI_GFQ_REQUEST_SEGMENTSIZE, loReqSegSize, loRespSegSize ) );
		liRet = DoRequestTo( *GetModuleMgr()->GetModuleServer(ltMHandle), AI_GFQ_REQUEST_SEGMENTSIZE, loReqSegSize, loRespSegSize );
		if ( liRet == AI_NO_ERROR )
		{
			liSumSize += loRespSegSize.GetSegmentSize();
		}
		else
		{
//			GetModuleMgr()->MasterErrorObserver( ltMHandle, liRet );
			//Delete it at last
			ltDeleteModule[n] = ltMHandle;
			liDeleteReturnCode[n] = liRet;
		}

		if ( lpoCurr == lpoLast ) break; // only two
		
		lpoCurr = lpoLast;
	}
	
	// Caculate other segment size by ciSegmentSize
	for ( lpoCurr = lpoFirst->cpNext; 
	      lpoCurr != NULL && lpoCurr != lpoHead && lpoCurr != lpoLast; lpoCurr = lpoCurr->cpNext )
	{
		stSegmentNode* lpoSegment = AI_GET_STRUCT_PTR( lpoCurr , stSegmentNode, coSegmentChild );
		liSumSize += lpoSegment->ciSegmentSize;
	}

	if (ltDeleteModule[0] != NULL) 
	{
		GetModuleMgr()->MasterErrorObserver(ltDeleteModule[0], liDeleteReturnCode[0]);
	}

	if (ltDeleteModule[1] != NULL) 
	{
		GetModuleMgr()->MasterErrorObserver(ltDeleteModule[1], liDeleteReturnCode[1]);
	}
	
	return liSumSize;
}

size_t clsQueueMgr::GetQueueSize( TQueueHandle atQueueHandle )
{
	assert( atQueueHandle != INVALID_QUEUE_HANDLE );
	LISTHEAD* lpoFirst = atQueueHandle->coSegmentHead.cpNext;
	LISTHEAD* lpoHead  = &atQueueHandle->coSegmentHead;
	LISTHEAD* lpoCurr = NULL;
	size_t liSumSize = 0;
	
	// Caculate other segment size by ciSegmentSize
	for ( lpoCurr = lpoFirst; lpoCurr != lpoHead; lpoCurr = lpoCurr->cpNext )
	{
		stSegmentNode* lpoSegment = AI_GET_STRUCT_PTR( lpoCurr , stSegmentNode, coSegmentChild );
		liSumSize += lpoSegment->ciSegmentSize;
	}
	
	return liSumSize;
}

size_t clsQueueMgr::GetQueueCount()
{
	return ciQueueNum;
}

size_t clsQueueMgr::GetQueueCapacity( TQueueHandle atQueueHandle )
{
	assert( atQueueHandle != INVALID_QUEUE_HANDLE );
	
	return atQueueHandle->ciQueueCapacity;
}

clsQueueIter clsQueueMgr::QueueBegin()
{
	return clsQueueIter( coQueueList.cpNext, &coQueueList );
}

clsQueueIter clsQueueMgr::QueueEnd()
{
	return clsQueueIter( &coQueueList, &coQueueList );
}

///////////////////////////////// Global Module function ///////////////////////////////
TModuleMgr* GetModuleMgr()
{
    static TModuleMgr* lpoInstance = NULL;
    
	if ( lpoInstance == NULL )
	{
		AI_NEW_ASSERT( lpoInstance, TModuleMgr );
	}
	
	return lpoInstance;
}

//end namespace
AI_GFQ_NAMESPACE_END
