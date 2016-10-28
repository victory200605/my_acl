//=============================================================================
/**
 * \file    GFQueueApi.cpp
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: GFQueueApi.cpp,v 1.1 2010/11/23 06:35:53 daizh Exp $
 *
 * History
 * 2008.04.04 first release by DZH
 */
//=============================================================================
#include "gfq/GFQueueApi.h"

void GetProcessName( char* apsName, size_t aiSize )
{
    snprintf( apsName, aiSize, "%s-%ld", gsGlobalProgName, (long)getpid() );
}

//// class define
//////////////////////////////////////////////////////////////////////////////////////
stErrorCache::stErrorCache() :
    ciErrNo(AI_NO_ERROR), ciTimestamp(0), ciTimeInterval(0)
{
}

bool stErrorCache::IsHitCache()
{
    if ( ( ciErrNo == AI_ERROR_END_OF_QUEUE 
            || ciErrNo == AI_ERROR_NO_ENOUGH_SPACE )
         && ( time(NULL) - ciTimestamp < ciTimeInterval ) )
    {
        return true;
    }
    
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////
stSegmentServer::stSegmentServer()
{
	memset( this, 0, sizeof(stSegmentServer) );
}

stSegmentServer::~stSegmentServer()
{
    if ( cpoAsyncClient == NULL ) return;
    
    cpoAsyncClient->ShutDown();
	AI_DELETE( cpoAsyncClient );
}

//////////////////////////////////////////////////////////////////////////////////////
stSegmentInfoNode::stSegmentInfoNode() :
    ciSegmentID(-1),
	ciModuleID(-1),
	ciQueueGlobalID(-1),
	cpoSegmentServer(NULL)
{
    memset( ccQueueName, 0, AI_MAX_NAME_LEN );
}

//////////////////////////////////////////////////////////////////////////////////////
clsGFQueueApi::clsGFQueueApi() : ciConnectCnt(1), ciTimeOut(6), cpoAsyncClient(NULL)
{
    memset( ccUserName, 0, sizeof(ccUserName) );
    memset( ccPassWord, 0, sizeof(ccPassWord) );
}

clsGFQueueApi::~clsGFQueueApi()
{
	AI_DELETE( cpoAsyncClient );
}
	
int32_t clsGFQueueApi::Initialize( const char* apsConfigFile )
{
	char      lcMasterIp[AI_IP_ADDR_LEN];
	int32_t   lcMasterPort;
	AIConfig* lpIni = NULL;
	
	if ( ( lpIni = AIGetIniHandler(apsConfigFile) ) == NULL)
	{
		AIWriteLOG( NULL, AILOG_LEVEL_ERROR, "[GFQApi]:Load GFQueue Client config file %s fail", apsConfigFile );
		return AI_ERROR_OPEN_FILE;
	}
	
	lpIni->GetIniString( "Master", "UserName", ccUserName, sizeof(ccUserName) );
	lpIni->GetIniString( "Master", "ServerIP", lcMasterIp, sizeof(lcMasterIp) );
	lcMasterPort         = lpIni->GetIniInt( "Master", "ServerPort", -1 );
	ciConnectCnt         = lpIni->GetIniInt( "Master", "ConnectCnt", 1 );
	ciTimeOut            = lpIni->GetIniInt( "Master", "TimeOut", 10 );
    ciErrorCacheInterval = lpIni->GetIniInt( "Master", "ErrorCacheInterval", 2 );
    
	AI_NEW_INIT_ASSERT( cpoAsyncClient, clsAsyncClient, lcMasterIp, lcMasterPort, ciConnectCnt, ciTimeOut );

	if ( cpoAsyncClient->StartDaemon( ciTimeOut ) < ciConnectCnt )
	{
	    // May be create thread fail, recycle AsyncClient here
	    cpoAsyncClient->ShutDown();
	    AI_DELETE(cpoAsyncClient);
	    
		return AI_ERROR_SYSTEM;
	}
	
	if ( strlen( ccUserName ) == 0 )
	{
	    GetProcessName( ccUserName, sizeof(ccUserName) );
    }
    
	return AI_NO_ERROR;
}

void clsGFQueueApi::Shutdown()
{
	cpoAsyncClient->ShutDown();
}

int32_t clsGFQueueApi::Get( const char* apsQueueName, AIChunk& aoData, int32_t aiWhenSec /* = 0 */)
{
	int    liRet = AI_NO_ERROR;
	size_t liQueueNameLen = strlen(apsQueueName);
	clsRequestGet  loReqGet;
	clsResponseGet loRespGet;
	AIChunkEx      loDataChunk(AI_MAX_DATA_LEN);
	stSegmentInfoNode loSegmentInfoNode;
	
	// Queue Name length check
	if ( liQueueNameLen <= 0 || liQueueNameLen >= AI_MAX_NAME_LEN )
	{
		return AI_ERROR_OUT_OF_RANGE;
	}
	
	if ( GetSegmentInfo( coReadSegments, apsQueueName, loSegmentInfoNode ) != AI_NO_ERROR )
	{
	    // Check error cache, if hit and return it immediately
	    if ( loSegmentInfoNode.coErrorCache.IsHitCache() )
	    {
	        liRet = loSegmentInfoNode.coErrorCache.ciErrNo;
	        
	        AI_GFQ_LOG( liRet, "[GFQApi]:Get Read Segment fail, MSGCODE:%d, MSG=%s, [Queue Name=%s][Hit Error Cache]", 
				liRet, StrError(liRet), apsQueueName );
				
	        return liRet;
	    }
	    
		if ( ( liRet = GetReadSegment( apsQueueName, loSegmentInfoNode ) ) != AI_NO_ERROR )
		{
			AI_GFQ_LOG( liRet, "[GFQApi]:Get Read Segment fail, MSGCODE:%d, MSG=%s, [Queue Name=%s]", 
				liRet, StrError(liRet), apsQueueName );
			return liRet;
		}
	}
	
	for( int liIt = 0; liIt < DEAD_CIRCLE_NUM; liIt++ ) //Ignore dead circle
	{
		loReqGet.SetSegmentID(loSegmentInfoNode.ciSegmentID);
		loReqGet.SetQueueGlobalID(loSegmentInfoNode.ciQueueGlobalID);
		loReqGet.SetQueueName(loSegmentInfoNode.ccQueueName);
		loReqGet.SetWhenSec(aiWhenSec);
		loReqGet.SetUserName(ccUserName);
		clsAsyncClient* lpoAsyncClient = loSegmentInfoNode.cpoSegmentServer->cpoAsyncClient;
		
		liRet = DoRequestTo( *lpoAsyncClient, AI_GFQ_REQUEST_GET, loReqGet, loRespGet );
		if ( liRet == AI_NO_ERROR )
		{
			loDataChunk.Reset();
			loRespGet.GetData( loDataChunk );
			aoData.Resize(loDataChunk.WritePtr() - loDataChunk.ReadPtr());
			memcpy( aoData.GetPointer(), loDataChunk.ReadPtr(), loDataChunk.WritePtr() - loDataChunk.ReadPtr() );
						
			return liRet;
		}

		// Dead circle, is impossible, it will be break and save ret code liRet
		if ( liIt >= DEAD_CIRCLE_NUM - 1 ) break;
		
		if ( ( liRet = GetReadSegmentSecond( apsQueueName, liRet, loSegmentInfoNode ) ) != AI_NO_ERROR )
		{
			AI_GFQ_LOG( liRet, "[GFQApi]:Get Read Segment second fail, MSGCODE:%d, MSG=%s, [Queue Name=%s]", 
				liRet, StrError(liRet), apsQueueName );
			return liRet;
		}
		//Yes, second get read segment success
	}
	// Break, dead circle
	AI_GFQ_ERROR( "[GFQApi]:Get Read Segment exception, MSGCODE:%d, MSG=%s, [Queue Name=%s]", 
		liRet, StrError(liRet), apsQueueName );
	return liRet;
}

int32_t clsGFQueueApi::Put( const char* apsQueueName, const AIChunk& aoData )
{
	int    liRet = AI_NO_ERROR;
	size_t liQueueNameLen = strlen(apsQueueName);
	clsRequestPut  loReqPut;
	clsResponsePut loRespPut;
	AIChunkEx      loDataChunk(AI_MAX_DATA_LEN);
	stSegmentInfoNode loSegmentInfoNode;
	
	// Queue Name length check
	if ( liQueueNameLen <= 0 || liQueueNameLen >= AI_MAX_NAME_LEN )
	{
		return AI_ERROR_OUT_OF_RANGE;
	}
	
	// Data length check
	if ( aoData.GetSize() <= 0 || aoData.GetSize() > AI_MAX_DATA_LEN )
	{
		return AI_ERROR_OUT_OF_RANGE;
	}
	
	memcpy( loDataChunk.WritePtr(), aoData.GetPointer(), aoData.GetSize() );
	loDataChunk.WritePtr(aoData.GetSize());
	
	if ( GetSegmentInfo( coWriteSegments, apsQueueName, loSegmentInfoNode ) != AI_NO_ERROR )
	{
	    // Check error cache, if hit and return it immediately
	    if ( loSegmentInfoNode.coErrorCache.IsHitCache() )
	    {
	        liRet = loSegmentInfoNode.coErrorCache.ciErrNo;
	        
	        AI_GFQ_LOG( liRet, "[GFQApi]:Get Write Segment fail, MSGCODE:%d, MSG=%s, [Queue Name=%s][Hit Error Cache]", 
				liRet, StrError(liRet), apsQueueName );
				
	        return liRet;
	    }
	    
		if ( ( liRet = GetWriteSegment( apsQueueName, loSegmentInfoNode ) ) != AI_NO_ERROR )
		{
			AI_GFQ_LOG( liRet, "[GFQApi]:Get Write Segment fail, MSGCODE:%d, MSG=%s, [Queue Name=%s]", 
				liRet, StrError(liRet), apsQueueName );
			return liRet;
		}
	}
	
	for( int liIt = 0; liIt < DEAD_CIRCLE_NUM; liIt++ ) //Ignore dead circle
	{
		loReqPut.SetSegmentID(loSegmentInfoNode.ciSegmentID);
		loReqPut.SetQueueGlobalID(loSegmentInfoNode.ciQueueGlobalID);
		loReqPut.SetQueueName(loSegmentInfoNode.ccQueueName);
		loReqPut.SetUserName(ccUserName);
		loReqPut.SetData( loDataChunk );
		clsAsyncClient* lpoAsyncClient = loSegmentInfoNode.cpoSegmentServer->cpoAsyncClient;
		
		liRet = DoRequestTo( *lpoAsyncClient, AI_GFQ_REQUEST_PUT, loReqPut, loRespPut );
		//liRet = AI_ERROR_SOCKET_RECV;
		if ( liRet == AI_NO_ERROR )
		{
			return liRet;
		}

		// Dead circle, is impossible, it will be break and save ret code liRet
		if ( liIt >= DEAD_CIRCLE_NUM - 1 ) break;

		if ( ( liRet = GetWriteSegmentSecond( apsQueueName, liRet, loSegmentInfoNode ) ) != AI_NO_ERROR )
		{
			AI_GFQ_LOG( liRet, "[GFQApi]:Get Write Segment second fail, MSGCODE:%d, MSG=%s, [Queue Name=%s]", 
				liRet, StrError(liRet), apsQueueName );
			return liRet;
		}
		//Yes, second get read segment success
	}
	// Break, dead circle
	AI_GFQ_ERROR( "[GFQApi]:Get Write Segment exception, MSGCODE:%d, MSG=%s, [Queue Name=%s]", 
		liRet, StrError(liRet), apsQueueName );
	return liRet;
}

int32_t clsGFQueueApi::GetStat( const char* apsQueueName, int* apiSize, int* apiCapaciy )
{
	clsRequestQueueStat loRequestStat;
	clsResponseQueueStat loResponseStat;
	int32_t liRet = AI_NO_ERROR;

	loRequestStat.SetUserName(ccUserName);
	loRequestStat.SetQueueName(apsQueueName);
	
	liRet = DoRequestTo( *cpoAsyncClient, AI_GFQ_REQUEST_QUEUE_STAT, loRequestStat, loResponseStat );
	
	if ( liRet == AI_NO_ERROR )
	{
		if ( apiSize != NULL ) *apiSize = loResponseStat.GetQueueSize();
		if ( apiCapaciy != NULL ) *apiCapaciy = loResponseStat.GetQueueCapacity();
	}
	
	return liRet;
}
 
int32_t clsGFQueueApi::GetModuleCount( int* apiCount )
{
	clsRequestModuleCount loRequest;
	clsResponseModuleCount loResponse;
	int32_t liRet = AI_NO_ERROR;

	loRequest.SetUserName(ccUserName);
	
	liRet = DoRequestTo( *cpoAsyncClient, AI_GFQ_REQUEST_MODULE_COUNT, loRequest, loResponse );
	
	if ( liRet == AI_NO_ERROR )
	{
		if ( apiCount != NULL ) *apiCount = loResponse.GetModuleCount();
	}
	
	return liRet;
}

int32_t clsGFQueueApi::ReloadQueueInfo( )
{
    clsRequestReloadQueueInfo loRequest;
	clsResponseReloadQueueInfo loResponse;
	int32_t liRet = AI_NO_ERROR;
	
	loRequest.SetUserName(ccUserName);
	
	liRet = DoRequestTo( *cpoAsyncClient, AI_GFQ_REQUEST_RELOADQUEUEINFO, loRequest, loResponse );
	
	if ( liRet != AI_NO_ERROR )
	{
		AI_GFQ_ERROR( "[GFQApi]:Notify reload queue Info, MSGCODE:%d, MSG=%s", liRet, StrError(liRet) );
	}
	
	return liRet;
}

int32_t clsGFQueueApi::PrintQueues( )
{
    clsRequestPrintQueue loRequest;
	clsResponsePrintQueue loResponse;
	int32_t liRet = AI_NO_ERROR;
	
	loRequest.SetUserName(ccUserName);

	liRet = DoRequestTo( *cpoAsyncClient, AI_GFQ_REQUEST_PRINT_QUEUES, loRequest, loResponse );
	
	if ( liRet != AI_NO_ERROR )
	{
		AI_GFQ_ERROR( "[GFQApi]:Notify print queue list, MSGCODE:%d, MSG=%s", liRet, StrError(liRet) );
	}
	
	return liRet;
}
    
int32_t clsGFQueueApi::PrintModules( )
{
    clsRequestPrintModule loRequest;
	clsResponsePrintModule loResponse;
	int32_t liRet = AI_NO_ERROR;
	
	loRequest.SetUserName(ccUserName);

	liRet = DoRequestTo( *cpoAsyncClient, AI_GFQ_REQUEST_PRINT_MODULES, loRequest, loResponse );
	
	if ( liRet != AI_NO_ERROR )
	{
		AI_GFQ_ERROR( "[GFQApi]:Notify print module list, MSGCODE:%d, MSG=%s", liRet, StrError(liRet) );
	}
	
	return liRet;
}

int32_t clsGFQueueApi::GetWriteSegment( const char* apsQueueName, stSegmentInfoNode& aoSegmentInfo )
{
	int liRet = AI_NO_ERROR;
	clsRequestWriteSeg loReqWriteSeg;
	clsResponseWriteSeg loRespWriteSeg;
	
	loReqWriteSeg.SetUserName(ccUserName);
	loReqWriteSeg.SetQueueName(apsQueueName);
	
	{
		liRet = DoRequestTo( *cpoAsyncClient, AI_GFQ_REQUEST_WRITESEG, loReqWriteSeg, loRespWriteSeg );
		if ( liRet != AI_NO_ERROR )
		{
		    aoSegmentInfo.coErrorCache.ciErrNo        = liRet;
		    aoSegmentInfo.coErrorCache.ciTimestamp    = time(NULL);
		    aoSegmentInfo.coErrorCache.ciTimeInterval = ciErrorCacheInterval;
		    SetSegmentInfo( coWriteSegments, apsQueueName, aoSegmentInfo );
		    
			return liRet;
		}
        aoSegmentInfo.coErrorCache.ciErrNo = liRet;
		aoSegmentInfo.ciSegmentID          = loRespWriteSeg.GetSegmentID();
		aoSegmentInfo.ciModuleID           = loRespWriteSeg.GetModuleID();
		aoSegmentInfo.ciQueueGlobalID      = loRespWriteSeg.GetQueueGlobalID();
		StringCopy( aoSegmentInfo.ccQueueName, apsQueueName, AI_MAX_NAME_LEN );
		
		if ( ( liRet = CreateSegmentServerConnection( 
			loRespWriteSeg.GetIpAddr(), loRespWriteSeg.GetPort(), &(aoSegmentInfo.cpoSegmentServer) ) ) != AI_NO_ERROR )
		{
			return GetWriteSegmentSecond( apsQueueName, liRet, aoSegmentInfo );
		}

		SetSegmentInfo( coWriteSegments, apsQueueName, aoSegmentInfo );
	}
	
	return AI_NO_ERROR;
}

int32_t clsGFQueueApi::GetReadSegment( const char* apsQueueName, stSegmentInfoNode& aoSegmentInfo )
{
	int liRet = AI_NO_ERROR;
	clsRequestReadSeg  loReqReadSeg;
	clsResponseReadSeg loRespReadSeg;
	
	loReqReadSeg.SetUserName(ccUserName);
	loReqReadSeg.SetQueueName(apsQueueName);
	
	{
		liRet = DoRequestTo( *cpoAsyncClient, AI_GFQ_REQUEST_READSEG, loReqReadSeg, loRespReadSeg );
		if ( liRet != AI_NO_ERROR )
		{
		    aoSegmentInfo.coErrorCache.ciErrNo        = liRet;
		    aoSegmentInfo.coErrorCache.ciTimestamp    = time(NULL);
		    aoSegmentInfo.coErrorCache.ciTimeInterval = ciErrorCacheInterval;
		    SetSegmentInfo( coReadSegments, apsQueueName, aoSegmentInfo );
		    
			return liRet;
		}
        
        aoSegmentInfo.coErrorCache.ciErrNo = liRet;
		aoSegmentInfo.ciSegmentID          = loRespReadSeg.GetSegmentID();
		aoSegmentInfo.ciModuleID           = loRespReadSeg.GetModuleID();
		aoSegmentInfo.ciQueueGlobalID      = loRespReadSeg.GetQueueGlobalID();
		StringCopy( aoSegmentInfo.ccQueueName, apsQueueName, AI_MAX_NAME_LEN );

		if ( ( liRet = CreateSegmentServerConnection( 
			loRespReadSeg.GetIpAddr(), loRespReadSeg.GetPort(), &(aoSegmentInfo.cpoSegmentServer) ) ) != AI_NO_ERROR )
		{
			return GetReadSegmentSecond( apsQueueName, liRet, aoSegmentInfo );
		}
		
		SetSegmentInfo( coReadSegments, apsQueueName, aoSegmentInfo );
	}
	
	return AI_NO_ERROR;
}

int32_t clsGFQueueApi::GetWriteSegmentSecond( const char* apsQueueName, int32_t aiRetcode, stSegmentInfoNode& aoSegmentInfo )
{
	int liRet = AI_NO_ERROR;
	clsRequestWriteSegSecond  loReqWriteSeg;
	clsResponseWriteSegSecond loRespWriteSeg;
	
	loReqWriteSeg.SetUserName(ccUserName);
	loReqWriteSeg.SetQueueName( apsQueueName );
	loReqWriteSeg.SetSegmentID( aoSegmentInfo.ciSegmentID );
	loReqWriteSeg.SetModuleID( aoSegmentInfo.ciModuleID );
	loReqWriteSeg.SetRetcode( aiRetcode );

	do
	{
		liRet = DoRequestTo( *cpoAsyncClient, AI_GFQ_REQUEST_WRITESEG_SECOND, loReqWriteSeg, loRespWriteSeg );
		if ( liRet != AI_NO_ERROR )
		{
		    aoSegmentInfo.coErrorCache.ciErrNo        = liRet;
		    aoSegmentInfo.coErrorCache.ciTimestamp    = time(NULL);
		    aoSegmentInfo.coErrorCache.ciTimeInterval = ciErrorCacheInterval;
		    SetSegmentInfo( coWriteSegments, apsQueueName, aoSegmentInfo );
		    
			return liRet;
		}
		
		aoSegmentInfo.coErrorCache.ciErrNo = liRet;
		aoSegmentInfo.ciSegmentID          = loRespWriteSeg.GetSegmentID();
		aoSegmentInfo.ciModuleID           = loRespWriteSeg.GetModuleID();
		aoSegmentInfo.ciQueueGlobalID      = loRespWriteSeg.GetQueueGlobalID();

		if ( ( liRet = CreateSegmentServerConnection( 
			loRespWriteSeg.GetIpAddr(), loRespWriteSeg.GetPort(), &(aoSegmentInfo.cpoSegmentServer) ) ) != AI_NO_ERROR )
		{
			return GetWriteSegmentSecond( apsQueueName, liRet, aoSegmentInfo );
		}

		SetSegmentInfo( coWriteSegments, apsQueueName, aoSegmentInfo );
		
		return AI_NO_ERROR;
		
	}while(false);
	
	return liRet;
}

int32_t clsGFQueueApi::GetReadSegmentSecond( const char* apsQueueName, int32_t aiRetcode, stSegmentInfoNode& aoSegmentInfo )
{
	int liRet = AI_NO_ERROR;
	clsRequestReadSegSecond  loReqReadSeg;
	clsResponseReadSegSecond loRespReadSeg;
	
	loReqReadSeg.SetUserName(ccUserName);
	loReqReadSeg.SetQueueName( apsQueueName );
	loReqReadSeg.SetSegmentID( aoSegmentInfo.ciSegmentID );
	loReqReadSeg.SetModuleID( aoSegmentInfo.ciModuleID );
	loReqReadSeg.SetRetcode( aiRetcode );

	do
	{
		liRet = DoRequestTo( *cpoAsyncClient, AI_GFQ_REQUEST_READSEG_SECOND, loReqReadSeg, loRespReadSeg );
		if ( liRet != AI_NO_ERROR )
		{
		    aoSegmentInfo.coErrorCache.ciErrNo        = liRet;
		    aoSegmentInfo.coErrorCache.ciTimestamp    = time(NULL);
		    aoSegmentInfo.coErrorCache.ciTimeInterval = ciErrorCacheInterval;
		    SetSegmentInfo( coReadSegments, apsQueueName, aoSegmentInfo );
		    
			return liRet;
		}
		
		aoSegmentInfo.coErrorCache.ciErrNo = liRet;
		aoSegmentInfo.ciSegmentID          = loRespReadSeg.GetSegmentID();
		aoSegmentInfo.ciModuleID           = loRespReadSeg.GetModuleID();
		aoSegmentInfo.ciQueueGlobalID      = loRespReadSeg.GetQueueGlobalID();

		if ( ( liRet = CreateSegmentServerConnection( 
			loRespReadSeg.GetIpAddr(), loRespReadSeg.GetPort(), &(aoSegmentInfo.cpoSegmentServer) ) ) != AI_NO_ERROR )
		{
			return GetReadSegmentSecond( apsQueueName, liRet, aoSegmentInfo );
		}
		
		SetSegmentInfo( coReadSegments, apsQueueName, aoSegmentInfo );
		
		return AI_NO_ERROR;
		
	}while(false);
	
	return liRet;
}

int32_t clsGFQueueApi::SetSegmentInfo( TSegmentHashTable& aoHashList, const char* apsQueueName, stSegmentInfoNode& aoSegmentInfo )
{
	// lock scope
	AISmartLock loLock(aoHashList.coLock);
		
	stSegmentInfoNode* lpoSegmentInfo = NULL;
	TSegmentIter loIter = aoHashList.coTable.Find( apsQueueName );
	if ( loIter != aoHashList.coTable.End() )
	{
	    lpoSegmentInfo = (stSegmentInfoNode*)loIter;
		lpoSegmentInfo->ciSegmentID      = aoSegmentInfo.ciSegmentID;
		lpoSegmentInfo->ciModuleID       = aoSegmentInfo.ciModuleID;
		lpoSegmentInfo->ciQueueGlobalID  = aoSegmentInfo.ciQueueGlobalID;
		lpoSegmentInfo->cpoSegmentServer = aoSegmentInfo.cpoSegmentServer;
		lpoSegmentInfo->coErrorCache     = aoSegmentInfo.coErrorCache;
		StringCopy( lpoSegmentInfo->ccQueueName, apsQueueName, AI_MAX_NAME_LEN );
	}
	else
	{
		lpoSegmentInfo = aoHashList.coTable.CreateNode();
		lpoSegmentInfo->ciSegmentID      = aoSegmentInfo.ciSegmentID;
		lpoSegmentInfo->ciModuleID       = aoSegmentInfo.ciModuleID;
		lpoSegmentInfo->ciQueueGlobalID  = aoSegmentInfo.ciQueueGlobalID;
		lpoSegmentInfo->cpoSegmentServer = aoSegmentInfo.cpoSegmentServer;
		lpoSegmentInfo->coErrorCache     = aoSegmentInfo.coErrorCache;
		StringCopy( lpoSegmentInfo->ccQueueName, apsQueueName, AI_MAX_NAME_LEN );
		
		aoHashList.coTable.Insert(lpoSegmentInfo);
	}
	
	return AI_NO_ERROR;
}

int32_t clsGFQueueApi::GetSegmentInfo( TSegmentHashTable& aoHashList, const char* apsQueueName, stSegmentInfoNode& aoSegmentInfo )
{
	// lock scope
	AISmartLock loLock(aoHashList.coLock);
		
	stSegmentInfoNode* lpoSegmentInfo = NULL;
	TSegmentIter loIter = aoHashList.coTable.Find( apsQueueName );
	if ( loIter != coReadSegments.coTable.End() )
	{
		lpoSegmentInfo = (stSegmentInfoNode*)loIter;
		
		aoSegmentInfo.ciSegmentID      = lpoSegmentInfo->ciSegmentID;
	    aoSegmentInfo.ciModuleID       = lpoSegmentInfo->ciModuleID;
	    aoSegmentInfo.ciQueueGlobalID  = lpoSegmentInfo->ciQueueGlobalID;
	    aoSegmentInfo.coErrorCache     = lpoSegmentInfo->coErrorCache;
	    aoSegmentInfo.cpoSegmentServer = lpoSegmentInfo->cpoSegmentServer;
	    StringCopy( aoSegmentInfo.ccQueueName, lpoSegmentInfo->ccQueueName, AI_MAX_NAME_LEN );
	    
	    return aoSegmentInfo.coErrorCache.ciErrNo;
	}

	return AI_ERROR_NO_FOUND_QUEUE;
}

int32_t clsGFQueueApi::CreateSegmentServerConnection( const char* apsServerIp, int32_t aiPort, stSegmentServer** apoSegmentServer )
{
	if ( apoSegmentServer != NULL ) *apoSegmentServer = NULL;
	
	// lock scope
	AISmartLock loLock(coServers.coLock);
		
	char lcIpPort[AI_IP_ADDR_LEN];
	snprintf( lcIpPort, AI_IP_ADDR_LEN, "%s:%d", apsServerIp, aiPort );
	TServerIter loIter = coServers.coTable.Find( lcIpPort );
	if ( loIter == coServers.coTable.End() )
	{
		stSegmentServer* lpSegmentServer = coServers.coTable.CreateNode();
		lpSegmentServer->ciServerPort    = aiPort;
		StringCopy( lpSegmentServer->ccServerIP, apsServerIp, AI_IP_ADDR_LEN );
		StringCopy( lpSegmentServer->ccServerIPPort, lcIpPort, AI_IP_ADDR_LEN );
		
		AI_NEW_INIT_ASSERT( lpSegmentServer->cpoAsyncClient, clsAsyncClient, 
				lpSegmentServer->ccServerIP, lpSegmentServer->ciServerPort, ciConnectCnt, ciTimeOut );
		
		if ( lpSegmentServer->cpoAsyncClient->StartDaemon( ciTimeOut, true ) <= 0 )
		{
		    AI_GFQ_ERROR( "[GFQApi]:Connect SegmentServer %s:%d fail, MSGCODE:%d, MSG=%s", 
				lpSegmentServer->ccServerIP, lpSegmentServer->ciServerPort, AI_ERROR_CONNECT_REJECT, StrError(AI_ERROR_CONNECT_REJECT) );
			
			coServers.coTable.DestoryNode( lpSegmentServer );
			return AI_ERROR_CONNECT_REJECT;
		}
		
		AI_GFQ_TRACE( "[GFQApi]:Connect SegmentServer %s:%d success", lpSegmentServer->ccServerIP, lpSegmentServer->ciServerPort );
		
		loIter = coServers.coTable.Insert(lpSegmentServer);
	}
	
	if ( apoSegmentServer != NULL ) *apoSegmentServer = (stSegmentServer*)loIter;
	
	return AI_NO_ERROR;
}
