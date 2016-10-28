//////////////////////////////////////////////////////////
//  clsModuleMgr.cpp
//  Implementation of the Class clsModuleMgr
//  Created on:      14-ÁùÔÂ-2008 21:14:50
//  Original author: dzh
///////////////////////////////////////////////////////////

//#include "ModuleMgr.h"
	
//------------------Module Head -------------------//
inline stModuleHead::stModuleHead( const char* apsModuleName, int32_t aiModuleID, 
	const char* apsIpAddr, int32_t aiPort )
{
	StringCopy( ccIpAddr, apsIpAddr, AI_IP_ADDR_LEN ); // module ip address
	ciPort = aiPort; // module port
	cpoAsyncClient = NULL;
	ciLastHeartBeatTime = time(NULL);
	
	StringCopy( ccModuleName, apsModuleName, AI_MAX_NAME_LEN );
	ciModuleID        = aiModuleID;
	ciFreeSegmentCnt  = 0;
	ciUsingSegmentCnt = 0;
	ciErrorCnt        = 0;
	
	memset( &coModuleChild, 0, sizeof(LISTHEAD) );
	memset( &coFreeMoudleChild, 0, sizeof(LISTHEAD) );
	memset( &coMoudleListChild, 0, sizeof(LISTHEAD) );
	
	ai_init_list_head( &coUsingSegHead );
	ai_init_list_head( &coFreeSegHead );
}

inline stModuleHead::~stModuleHead()
{
	if ( cpoAsyncClient == NULL ) return;
		
	cpoAsyncClient->ShutDown();
	
	AI_DELETE( cpoAsyncClient );
}

inline void stModuleHead::Unlink()
{
	AI_DUAL_UNLINK( coModuleChild );
	AI_DUAL_UNLINK( coFreeMoudleChild );
	AI_DUAL_UNLINK( coMoudleListChild );
}

//------------------Module Name Hash-------------------//
inline stModuleNameHash::stModuleNameHash()
{
    ai_init_list_head( &coModuleHead );
}

//---------------------------------ModuleMgr stSegmentNode--------------------------------//
template< class TSegment >
clsModuleMgr<TSegment>::stSegmentNode::stSegmentNode() : cpoModuleHead(NULL)
{
	memset( &coSegmentChild, 0, sizeof(LISTHEAD) );
}

template< class TSegment >
void clsModuleMgr<TSegment>::stSegmentNode::UnlinkModule()
{
	if ( cpoModuleHead == NULL ) return;
	
    AI_DUAL_UNLINK( coSegmentChild );
}

//-------------------------------------- ModuleMgr ---------------------------------------//
template< class TSegment >
clsModuleMgr<TSegment>* clsModuleMgr<TSegment>::cpoInstance = NULL;

//template< class TSegment >
//clsModuleMgr<TSegment>* clsModuleMgr<TSegment>::Instance()
//{
//	if ( cpoInstance == NULL )
//	{
//		AI_NEW_ASSERT( cpoInstance, clsModuleMgr<TSegment> );
//	}
//	return cpoInstance;
//}

template< class TSegment >
clsModuleMgr<TSegment>::clsModuleMgr() : cpoModuleHash(NULL), ciMuduleNum(0), ciModuleID(0),ciErrorLimit(-1)
{
	AI_NEW_ARRAY_ASSERT( cpoModuleHash, stModuleNameHash, AI_MODULE_HASH_SIZE );
	
    ai_init_list_head( &coFreeModule );
    ai_init_list_head( &coModuleList );
}

template< class TSegment >
clsModuleMgr<TSegment>::~clsModuleMgr()
{
	Close();
	AI_DELETE_ARRAY( cpoModuleHash );
}

template< class TSegment >
int32_t clsModuleMgr<TSegment>::AddModule( const char* apsModuleName, const char* apsIpAddr, int32_t aiPort )
{
    int32_t liRet = AI_NO_ERROR;
    
	if ( SearchModule( apsModuleName ) != NULL )
	{
		return AI_ERROR_DUP_MODULE;
	}
	
	int32_t liIndex = HashString( apsModuleName, strlen(apsModuleName) ) %  AI_MODULE_HASH_SIZE;
	LISTHEAD* lpoHead = &cpoModuleHash[ liIndex ].coModuleHead;
	
	stModuleHead* lpoModuleHead = NULL;
	AI_NEW_INIT_ASSERT( lpoModuleHead, stModuleHead, apsModuleName, ciModuleID++, apsIpAddr, aiPort );
	AI_NEW_INIT_ASSERT( lpoModuleHead->cpoAsyncClient, clsAsyncClient, apsIpAddr, aiPort, 1, 6 );

    liRet = lpoModuleHead->cpoAsyncClient->StartDaemon( 6, true );
	if ( liRet < 0 )
	{// Can't connect to segment server
		AI_DELETE( lpoModuleHead );
		return AI_ERROR_CONNECT_REJECT;
	}
	else if ( liRet != 1 )
	{// Create pthread fail
	    AI_DELETE( lpoModuleHead );
		return AI_ERROR_SYSTEM;
	}
	
	ai_list_add_tail( &lpoModuleHead->coModuleChild, lpoHead );
	ai_list_add_tail( &lpoModuleHead->coMoudleListChild, &coModuleList );
	
	ciMuduleNum++;
	
	return AI_NO_ERROR;
}

template< class TSegment >
int32_t clsModuleMgr<TSegment>::DelModule( const char* apsModuleName )
{
	stModuleHead* lpoModuleHead = NULL;
	if ( ( lpoModuleHead = SearchModule( apsModuleName ) ) == NULL )
	{// Sorry, module unexisted
		return AI_ERROR_NO_FOUND_MODULE;
	}
	
	return DelModule( lpoModuleHead );
}

template< class TSegment >
int32_t clsModuleMgr<TSegment>::DelModule( TModuleHandle atModuleHandle )
{
	assert( atModuleHandle != INVALID_MODULE_HANDLE );
	
	//Delete free segment
	LISTHEAD* lpoTemp = NULL;
	stSegmentNode* lpoSegment = NULL;
	while( !ai_list_is_empty( &atModuleHandle->coFreeSegHead ) )
	{
		//Delete from list
		ai_list_del_head( lpoTemp, &atModuleHandle->coFreeSegHead );
		lpoSegment = AI_GET_STRUCT_PTR( lpoTemp , stSegmentNode, coSegmentChild );
		
		AI_DELETE( lpoSegment );
	}
	
	//Delete using segment
	while( !ai_list_is_empty( &atModuleHandle->coUsingSegHead ) )
	{
		//Delete from list
		ai_list_del_head( lpoTemp, &atModuleHandle->coUsingSegHead );
		lpoSegment = AI_GET_STRUCT_PTR( lpoTemp , stSegmentNode, coSegmentChild );
		
		lpoSegment->Unlink();
		
		AI_DELETE( lpoSegment );
	}
	
	atModuleHandle->Unlink();

	AI_DELETE( atModuleHandle );
	
	atModuleHandle = INVALID_MODULE_HANDLE;
	
	ciMuduleNum--;
	
	return AI_NO_ERROR;
}

template< class TSegment >
TModuleHandle clsModuleMgr<TSegment>::SearchModule( const char* apsModuleName )
{
	int32_t liIndex = HashString( apsModuleName, strlen(apsModuleName) ) %  AI_MODULE_HASH_SIZE;
	LISTHEAD* lpoHead = &cpoModuleHash[ liIndex ].coModuleHead;
	LISTHEAD* lpoCurr = lpoHead->cpNext;
	for( ; lpoCurr != NULL && lpoCurr != lpoHead; lpoCurr = lpoCurr->cpNext )
	{
		stModuleHead* lpoModuleHead = AI_GET_STRUCT_PTR( lpoCurr , stModuleHead, coModuleChild );

		if ( strcmp( lpoModuleHead->ccModuleName, apsModuleName ) == 0 )
		{
			return lpoModuleHead;
		}
	}
	return INVALID_MODULE_HANDLE;
}

template< class TSegment >
TSegment* clsModuleMgr<TSegment>::CreateSegment( TModuleHandle atModuleHandle )
{
	// Create segment node
	stSegmentNode* lpoSegment = NULL;
	AI_NEW_ASSERT( lpoSegment, stSegmentNode );
	
	lpoSegment->cpoModuleHead = atModuleHandle;
	
	return lpoSegment;
}

template< class TSegment >
void clsModuleMgr<TSegment>::DestorySegment( TSegment*& apoSegment )
{
	assert( apoSegment != NULL );
	
	AI_DELETE( apoSegment );
}

template< class TSegment >
int32_t clsModuleMgr<TSegment>::PutFreeSegment( TSegment* apoSegment )
{
	stSegmentNode* lpoSegment = (stSegmentNode*)apoSegment;
	assert( lpoSegment != NULL && lpoSegment->cpoModuleHead != NULL );

	lpoSegment->cpoModuleHead->ciFreeSegmentCnt++;
	
	lpoSegment->UnlinkModule();
	
	ai_list_add_tail( &lpoSegment->coSegmentChild, &lpoSegment->cpoModuleHead->coFreeSegHead );
	
	if ( lpoSegment->cpoModuleHead->ciFreeSegmentCnt == 1 )
	{
		// Put ModuleHead to free module link once
		ai_list_add_tail( &lpoSegment->cpoModuleHead->coFreeMoudleChild, &coFreeModule );
	}
	
	return AI_NO_ERROR;
}

template< class TSegment >
int32_t clsModuleMgr<TSegment>::PutUsingSegment( TSegment* apoSegment )
{
	stSegmentNode* lpoSegment = (stSegmentNode*)apoSegment;
	assert( lpoSegment != NULL && lpoSegment->cpoModuleHead != NULL );

	lpoSegment->UnlinkModule();

	ai_list_add_tail( &lpoSegment->coSegmentChild, &lpoSegment->cpoModuleHead->coUsingSegHead );
	
	lpoSegment->cpoModuleHead->ciUsingSegmentCnt++;
	
	return AI_NO_ERROR;
}

template< class TSegment >
TSegment* clsModuleMgr<TSegment>::AllocSegment()
{
	LISTHEAD* lpoTemp = NULL;
	stModuleHead* lpoModuleHead = NULL;
	while( true )
	{
		if ( ai_list_is_empty(&coFreeModule) )
		{
			//Oh, not enough free segment
			return NULL;
		}
		//Delete from list
		ai_list_del_head( lpoTemp, &coFreeModule );
		lpoModuleHead = AI_GET_STRUCT_PTR( lpoTemp , stModuleHead, coFreeMoudleChild );
		if ( ai_list_is_empty(&lpoModuleHead->coFreeSegHead) )
		{
			//Empty module, It should not be here
			continue;
		}
		
		break;
	}
	
	ai_list_del_head( lpoTemp, &lpoModuleHead->coFreeSegHead );
	stSegmentNode* lpoSegmentNode = AI_GET_STRUCT_PTR( lpoTemp , stSegmentNode, coSegmentChild );
	lpoModuleHead->ciFreeSegmentCnt--;
	if ( lpoModuleHead->ciFreeSegmentCnt != 0 )
	{
		//Yes, I still have free segment
		ai_list_add_tail( &lpoModuleHead->coFreeMoudleChild, &coFreeModule );
	}
	
	// Put segment into using list
	PutUsingSegment( lpoSegmentNode );

	return lpoSegmentNode;
}

template< class TSegment >
int32_t clsModuleMgr<TSegment>::FreeSegment( TSegment* apoSegment )
{
	stSegmentNode* lpoSegment = (stSegmentNode*)apoSegment;
	assert( lpoSegment != NULL && lpoSegment->cpoModuleHead != NULL );
	
	lpoSegment->cpoModuleHead->ciUsingSegmentCnt--;
	
	return PutFreeSegment( apoSegment );
}

template< class TSegment >
TModuleHandle clsModuleMgr<TSegment>::GetModuleHandle( const TSegment* apoSegment )
{ 
	assert( apoSegment != NULL && ((stSegmentNode*)apoSegment)->cpoModuleHead != NULL );
	
	return ((stSegmentNode*)apoSegment)->cpoModuleHead; 
}

template< class TSegment >
int32_t clsModuleMgr<TSegment>::GetModuleID( const TSegment* apoSegment )
{
	assert( apoSegment != NULL && ((stSegmentNode*)apoSegment)->cpoModuleHead != NULL );
	
	return ((stSegmentNode*)apoSegment)->cpoModuleHead->ciModuleID; 
}

template< class TSegment >
const char* clsModuleMgr<TSegment>::GetModuleIpAddr( const TSegment* apoSegment )
{
	assert( apoSegment != NULL && ((stSegmentNode*)apoSegment)->cpoModuleHead != NULL );
	
	return ((stSegmentNode*)apoSegment)->cpoModuleHead->ccIpAddr;
}

template< class TSegment >
int32_t clsModuleMgr<TSegment>::GetModulePort( const TSegment* apoSegment )
{
 	assert( apoSegment != NULL && ((stSegmentNode*)apoSegment)->cpoModuleHead != NULL );
 	
	return ((stSegmentNode*)apoSegment)->cpoModuleHead->ciPort;
}

template< class TSegment >
size_t clsModuleMgr<TSegment>::GetModuleCount()
{
	return ciMuduleNum;
}

template< class TSegment >
size_t clsModuleMgr<TSegment>::GetSegmentCount( TModuleHandle atModuleHandle )
{
	assert( atModuleHandle != INVALID_MODULE_HANDLE );
	
	return atModuleHandle->ciUsingSegmentCnt + atModuleHandle->ciFreeSegmentCnt;
}

template< class TSegment >
size_t clsModuleMgr<TSegment>::GetUsingSegmentCount( TModuleHandle atModuleHandle )
{
	assert( atModuleHandle != INVALID_MODULE_HANDLE );
	
	return atModuleHandle->ciUsingSegmentCnt;
}

template< class TSegment >
size_t clsModuleMgr<TSegment>::GetFreeSegmentCount( TModuleHandle atModuleHandle )
{
	assert( atModuleHandle != INVALID_MODULE_HANDLE );
	
	return atModuleHandle->ciFreeSegmentCnt;
}

template< class TSegment >
void clsModuleMgr<TSegment>::UpdateHeartBeatTime( TModuleHandle atModuleHandle )
{
    assert( atModuleHandle != INVALID_MODULE_HANDLE );
	
	atModuleHandle->ciLastHeartBeatTime = time(NULL);
}

template< class TSegment >
int32_t clsModuleMgr<TSegment>::GetLastHeartBeatTime( TModuleHandle atModuleHandle )
{
    assert( atModuleHandle != INVALID_MODULE_HANDLE );
	
	return atModuleHandle->ciLastHeartBeatTime;
}

template< class TSegment >
clsModuleIter clsModuleMgr<TSegment>::ModuleBegin()
{
	return clsModuleIter( coModuleList.cpNext, &coModuleList );
}
	
template< class TSegment >
clsModuleIter clsModuleMgr<TSegment>::ModuleEnd()
{
	return clsModuleIter( &coModuleList, &coModuleList );
}

template< class TSegment >
clsAsyncClient* clsModuleMgr<TSegment>::GetModuleServer( TModuleHandle atModuleHandle )
{
	assert( atModuleHandle != INVALID_MODULE_HANDLE );
	
	return atModuleHandle->cpoAsyncClient;
}

template< class TSegment >
void clsModuleMgr<TSegment>::MasterErrorObserver( TModuleHandle atModuleHandle, int32_t aiErrno )
{
    assert( atModuleHandle != INVALID_MODULE_HANDLE );
	
	AI_GFQ_ERROR( "Delete segment server module %s,MSGCODE:%d, MSG:%s, [Master]", 
        atModuleHandle->ccModuleName, aiErrno, StrError(aiErrno) );
    DelModule( atModuleHandle );
}

template< class TSegment >
void clsModuleMgr<TSegment>::MasterFreeErrorObserver( TModuleHandle atModuleHandle, int32_t aiErrno )
{
    assert( atModuleHandle != INVALID_MODULE_HANDLE );
	
	if (aiErrno != AI_ERROR_PERMISION_DENIED)
	{
	    AI_GFQ_ERROR( "Delete segment server module %s,MSGCODE:%d, MSG:%s, [Master Free]", 
            atModuleHandle->ccModuleName, aiErrno, StrError(aiErrno) );
        DelModule( atModuleHandle );
	}
}

template< class TSegment >
int32_t clsModuleMgr<TSegment>::ApiErrorObserver( TModuleHandle atModuleHandle, int32_t aiErrno )
{
	int32_t liRet = SG_RETRY;
	
	switch( aiErrno )
	{
	    // Retry it
	    // File system error
		case AI_ERROR_SYNC_FILE:
		case AI_ERROR_READ_FILE:
		case AI_ERROR_WRITE_FILE:
		case AI_ERROR_INVALID_DATA:
        // System resource error
		case AI_ERROR_THREAD_POOL_FULL:
		{
			//To Do
			liRet = SG_RETRY;
			
			if ( atModuleHandle == INVALID_MODULE_HANDLE ) break;
			
			atModuleHandle->ciErrorCnt++;
			int liErrorLimit = 0;
			if ( ciErrorLimit <= 0 ) liErrorLimit = atModuleHandle->ciUsingSegmentCnt + atModuleHandle->ciFreeSegmentCnt;
			else liErrorLimit = ciErrorLimit;
			
			if ( atModuleHandle->ciErrorCnt > liErrorLimit )
			{
			    liRet = SG_SWITCH;
				AI_GFQ_ERROR( "Delete segment server module %s,MSGCODE:%d, MSG:%s, [Api]/[Error Cnt=%d/Limit=%d]", 
					atModuleHandle->ccModuleName, aiErrno, StrError(aiErrno), atModuleHandle->ciErrorCnt, liErrorLimit );
				DelModule( atModuleHandle );
			}
			break;
		}
		
		case AI_ERROR_SERVER_DENIED:
		{
		    liRet = SG_RETRY;
		    break; 
		}
		
		// Normal error, switch segment
		case AI_ERROR_END_OF_SEGMENT:
		case AI_ERROR_EMPTY_OF_SEGMENT:
		case AI_ERROR_NO_ENOUGH_SPACE:
		case AI_ERROR_PERMISION_DENIED:
		{
		    liRet = SG_SWITCH;
		    
		    if ( atModuleHandle != INVALID_MODULE_HANDLE )
		    {
		    	atModuleHandle->ciErrorCnt = 0;
		    }

		    break;
		}

		// Return it
		case AI_ERROR_CONNECT_REJECT:
		case AI_ERROR_SOCKET_SEND:
        case AI_ERROR_SOCKET_RECV:
		case AI_ERROR_INVALID_REQUST:
		case AI_ERROR_NO_FOUND_OUTTIME:
		default:
		{
		    liRet = SG_RETURN;
		    break;
		}
	};
	
	return liRet;
}

template< class TSegment >
void clsModuleMgr<TSegment>::Close()
{
	LISTHEAD* lpoTemp = NULL;
	for( size_t liIndex = 0; liIndex < AI_MODULE_HASH_SIZE; liIndex++ )
	{
		while( ai_list_is_empty( &cpoModuleHash[ liIndex ].coModuleHead ) )
		{
			//Delete from list
			ai_list_del_head( lpoTemp, &coFreeModule );
			stModuleHead* lpoModuleHead = AI_GET_STRUCT_PTR( lpoTemp , stModuleHead, coFreeMoudleChild );
			DelModule( lpoModuleHead );
		}
	}
}
