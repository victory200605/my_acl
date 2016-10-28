//=============================================================================
/**
 * \file    MasterServer.cpp
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: MasterServer.cpp,v 1.1 2010/11/23 06:35:53 daizh Exp $
 *
 * History
 * 2008.04.04 first release by DZH
 */
//=============================================================================
#include "MasterServer.h"

AI_GFQ_NAMESPACE_START

// thread param sub struct constructor
clsMasterServer::stThreadParam::stThreadParam( 
	clsMasterServer* apoServer, const ASYNCMATCH* apoMatch, char* apsData, size_t aiSize ) :
	cpoServer(apoServer),
	coData( apsData, aiSize ),
	ciCmd(0)
{
	memcpy( &coMatch, apoMatch, sizeof(ASYNCMATCH) );
}

clsMasterServer::clsMasterServer( const unsigned short aiBindPort )
    : clsAsyncServer( "", aiBindPort )
    , cpoThreadPool(NULL)
    , ciShutDownTimeout(5)
    , ciListenPort(-1)
    , ciPrintInterval(10)
    , ciHeartBeatInterval(10)
    , cdTrapLimitPercent(100)
{
#if defined(_AI_LIB_EX_OLD)
    cbShutdown = false;
#endif
}

clsMasterServer::~clsMasterServer()
{
    cpoThreadPool = NULL;
}

int clsMasterServer::Initialize(const char *apcConfigFile)
{
    AIConfig* lpoIni                 = NULL;
    int   liRet                  = AI_NO_ERROR;
    int   liThreadCount          = 0;
    int   liThreadPoolCache      = 0;
    int   liQueueDefaultCapacity = 0;
    int   liErrorLimitValue      = 0;
    char  lcLibName[AI_MAX_NAME_LEN];
    
     //Module info
	int liMaxChildCount   = 0;
	int liAdminListenPort = 0;
	int liMasterServerPort = -1;
	char lcModuleID[AI_MAX_NAME_LEN];
	char lcAdminListenIpAddr[AI_IP_ADDR_LEN];
    
	if ((lpoIni = AIGetIniHandler(apcConfigFile)) == NULL)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "Load config file %s fail", apcConfigFile);
        return 1;
    }
    
    AI_GFQ_INFO( ">>>>>>>>>>>>>>>>>>>>>>>>>>MasterServer<<<<<<<<<<<<<<<<<<<<<<<<<<<" );

    //load config variable
    ciListenPort           = lpoIni->GetIniInt( "Master", "ListenPort", -1 );
    liThreadCount          = lpoIni->GetIniInt( "Master", "ThreadPool", 500 );
    liThreadPoolCache      = lpoIni->GetIniInt( "Master", "ThreadPoolCache", 5000 );
    ciShutDownTimeout      = lpoIni->GetIniInt( "Master", "ShutDownTimeout", 0 );
    ciPrintInterval        = lpoIni->GetIniInt( "Master", "PrintInterval", 0 );
    ciHeartBeatInterval    = lpoIni->GetIniInt( "Master", "HeartBeatInterval", 30 );
    liQueueDefaultCapacity = lpoIni->GetIniInt( "Master", "QueueDefaultCapacity", 199999999 );
    liErrorLimitValue      = lpoIni->GetIniInt( "Master", "ErrorLimitValue", 1000 );
    cdTrapLimitPercent     = lpoIni->GetIniInt( "Master", "TrapLimitPercent", 100 );
    
    coGFQueue.SetErrorLimit(liErrorLimitValue);
    coGFQueue.SetQueueDefaultCapacity(liQueueDefaultCapacity);
	if ( lpoIni->GetIniString( "Master", "LibName", lcLibName, sizeof(lcLibName) ) > 0 )
	{
		if ( ( liRet = coGFQueue.LoadPermisionFunc(lcLibName) ) != AI_NO_ERROR )
		{
			AI_GFQ_LOG( liRet, "Load Permision Function fail, MSGCODE:%d, MSG:%s, [Lib Name=%s]", 
				liRet, StrError(liRet), lcLibName );
        	return 1;
		}
	}

    //module config info
    liMaxChildCount        = lpoIni->GetIniInt( "SERVER", "MaxChildCount", -1 );
    liAdminListenPort      = lpoIni->GetIniInt( "SERVER", "AdminListenPort", -1 );
    liMasterServerPort     = lpoIni->GetIniInt( "SERVER", "MasterServerPort", -1 );
    lpoIni->GetIniString( "SERVER", "ModuleID", lcModuleID, sizeof(lcModuleID) );
    lpoIni->GetIniString( "SERVER", "AdminListenIpAddr", lcAdminListenIpAddr, sizeof(lcAdminListenIpAddr) );
    ciListenPort  = liMasterServerPort;
    
    AI_NEW_ASSERT( cpoThreadPool, AITask );
    if ( cpoThreadPool->Initialize( liThreadCount, liThreadPoolCache ) != 0 )
    {
        AI_GFQ_ERROR( "Initialize Task fail, MSG:May be thread pool create fail" );
        return 1;
    }
    ciThreadCount = liThreadCount;
    
    //module configure
	{
	    AI_GFQ_INFO( "----------------Module Config Variable-----------------" );
	    AI_GFQ_INFO( "* ModuleID                = %s", lcModuleID);
        AI_GFQ_INFO( "* MaxChildCount           = %d", liMaxChildCount);
        AI_GFQ_INFO( "* AdminListenPort         = %d", liAdminListenPort);
        AI_GFQ_INFO( "* AdminListenIpAddr       = %s", lcAdminListenIpAddr);
        AI_GFQ_INFO( "* MasterServerPort         = %d", liMasterServerPort);
	}

	//complete configure
	{
	    AI_GFQ_INFO( "----------------Master Config Variable-----------------" );
	    AI_GFQ_INFO( "* Listen Port             = %d", ciListenPort);
        AI_GFQ_INFO( "* Thread Pool             = %d", liThreadCount);
        AI_GFQ_INFO( "* Thread Pool Cache       = %d", liThreadPoolCache);
        AI_GFQ_INFO( "* Print Interval          = %d", ciPrintInterval);
        AI_GFQ_INFO( "* Heart Beat Interval     = %d", ciHeartBeatInterval);
        AI_GFQ_INFO( "* Queue Default Capacity  = %d", liQueueDefaultCapacity);
        AI_GFQ_INFO( "* Lib Name                = %s", lcLibName);
        AI_GFQ_INFO( "* Trap Limit Percent      = %d", (int)cdTrapLimitPercent);
	}
	
	return 0;
}

int clsMasterServer::Startup()
{
    int liRet = AI_NO_ERROR;
    
    if ( (liRet = AI_GFQ_INITIALIZE_TRAP() ) != 0 )
    {
        AI_GFQ_ERROR( "MasterServer initialize statserver fail, ErrMsg=%s, RetCode:%d", strerror(errno), liRet );
        
        return liRet;
    }
    
    if ( ( liRet = StartDaemon( "MasterServerPort", GetNewConnection, GetListenSocket ) ) != 0 )
	{
	    switch( liRet )
	    {
	        case ASYNC_ERROR_LISTENFAIL:
                AI_GFQ_ERROR( "MasterServer startup fail, ErrMsg=AsyncServer Listen port fail, SysMsg:%s", strerror(errno) );
	            break;
	        default:
	            AI_GFQ_ERROR( "MasterServer startup fail, ErrMsg=System error, SysMsg:%s", strerror(errno) );
	            break;
	    }
	    
	    AI_GFQ_CLOSE_TRAP();
	    
		return 1;
	}

	AI_GFQ_INFO( "MasterServer startup success");
	fprintf(stderr, "MasterServer startup success\n");
	
	int liTimeCount = 0;
	int liPrintStatTimeCount = 0;
	while ( !IsShutdown() )
	{
		// Do head beat
		if ( liTimeCount > 2 )
		{
			coGFQueue.CheckModule(ciHeartBeatInterval);
			liTimeCount = 0;
		}
		
		// Do Print status
		if ( ciPrintInterval > 0 && liPrintStatTimeCount > ciPrintInterval )
		{
			PrintStat( );
			liPrintStatTimeCount = 0;
		}

		sleep(1);
		liTimeCount += 1;
		liPrintStatTimeCount += 1;
	}
	
	clsAsyncServer::ShutDown();
	
    AI_DELETE( cpoThreadPool );
    
    AI_GFQ_CLOSE_TRAP();
    
    AI_GFQ_INFO( "MasterServer shutdown" );

    return 0;
}

bool clsMasterServer::IsShutdown()
{
    return giGlobalShutDown == 1;
}

//thread operator
void clsMasterServer::Callback( const ASYNCMATCH *apoMatch, char *apcData, const int aiSize )
{
    clsHeader      loHeader;
    stThreadParam* lpoParam = NULL;
    
    AI_NEW_INIT_ASSERT( lpoParam, stThreadParam, this, apoMatch, apcData, aiSize );
    
    // Trace log
    AIChunkEx loHexBuff(1);
    AI_GFQ_TRACE( "Receive request, [MsgID=%d]/[ThreadID=%d]/[Length=%d]/[Package=\n%s]", 
        apoMatch->ciMsgID, apoMatch->ctThreadID, aiSize, 
        UStringToHex( lpoParam->coData.BasePtr(), lpoParam->coData.GetSize(), loHexBuff ) );
    
    // Decode package header
	loHeader.Decode( lpoParam->coData );
	lpoParam->ciCmd = loHeader.GetCmd();
	
	// Segment Server request
    switch( lpoParam->ciCmd )
    {
        case AI_GFQ_REQUEST_ADD_MODULE:
	    case AI_GFQ_REQUEST_ADD_SEG:
	    case AI_GFQ_REQUEST_ADD_SEG_TRY:
	    case AI_GFQ_REQUEST_DEL_MODULE:
	    {
            this->HandleRequestEntry( lpoParam );
            return;
	    }
	    default: break;
    };
  
    // Other request do by thread pool, but it may be full
    if ( cpoThreadPool->Put( HandleRequestEntry, lpoParam ) != 0 )
	{
	    AI_GFQ_TRAP(
            AIM::GetModuleID(),
            4002,
            "Master ThreadPool full",
            "Master ThreadPool full, Current=%d, Max=%d, Wait=%d, Capacity=%d",
            cpoThreadPool->GetActiveThreadCount(),
            ciThreadCount,
            cpoThreadPool->GetCacheSize(),
            cpoThreadPool->GetCacheCapacity() );
            
		AI_DELETE( lpoParam );
		
		AI_GFQ_ERROR( "Attach thread fail MSGCODE:%d, MSG:%s", 
		              AI_ERROR_THREAD_POOL_FULL, StrError( AI_ERROR_THREAD_POOL_FULL ) );
		
		//for error report
		clsResponse loResponse;
		loResponse.SetStat(AI_ERROR_THREAD_POOL_FULL);
        SendResponse( apoMatch, AI_GFQ_RESPONSE_ERROR, &loResponse );
	}
	else
	{
	    double ldWait     = cpoThreadPool->GetCacheSize();
	    int    liCapacity = cpoThreadPool->GetCacheCapacity();
	    double ldCurrent  = ( ldWait / (liCapacity <= 0 ? 1 : liCapacity) )  * 100;
	        
	    if ( ldCurrent >= this->cdTrapLimitPercent )
	    {
    	    AI_GFQ_TRAP(
                AIM::GetModuleID(),
                4002,
                "Master ThreadPool full",
                "Master ThreadPool full, Current=%d, Max=%d, Wait=%d, Capacity=%d",
                cpoThreadPool->GetActiveThreadCount(),
                ciThreadCount,
                (int)ldWait,
                liCapacity );
        }
	}
}

void* clsMasterServer::HandleRequestEntry( void *apvParam )
{
	stThreadParam* lpoParam = (stThreadParam*)apvParam;
    
    // Trace log
    AI_GFQ_TRACE( "Handle request now, [MsgID=%d]/[ThreadID=%d]", lpoParam->coMatch.ciMsgID, lpoParam->coMatch.ctThreadID );
    
	lpoParam->cpoServer->HandleRequestThread( &lpoParam->coMatch, lpoParam->ciCmd, lpoParam->coData );
	
	AI_DELETE( lpoParam );
	
    return NULL;
}

void clsMasterServer::HandleRequestThread( ASYNCMATCH *apoMatch, int aiCmd, AIChunkEx& aoChunk )
{
    // Statistic request, unprotected value
    coRequestStat.Request();
    
    // case define
#define SERVER_CASE( requestvalue, request, responsevalue, response )           \
    case requestvalue:                                                          \
    {                                                                           \
        request loRequest;                                                      \
        response loResponse;                                                    \
    	if ( loRequest.Decode(aoChunk) != 0 )                                   \
    	{                                                                       \
    	    break;                                                              \
    	}                                                                       \
    	this->DoRequest( loRequest, loResponse );                               \
    	this->SendResponse( apoMatch, responsevalue, &loResponse );             \
    	return;                                                                 \
    }
		    
	switch ( aiCmd )
	{
		SERVER_CASE( AI_GFQ_REQUEST_WRITESEG, clsRequestWriteSeg, AI_GFQ_RESPONSE_WRITESEG, clsResponseWriteSeg )
	    SERVER_CASE( AI_GFQ_REQUEST_READSEG, clsRequestReadSeg, AI_GFQ_RESPONSE_READSEG, clsResponseReadSeg )
	    SERVER_CASE( AI_GFQ_REQUEST_WRITESEG_SECOND, clsRequestWriteSegSecond, AI_GFQ_RESPONSE_WRITESEG, clsResponseWriteSegSecond )
	    SERVER_CASE( AI_GFQ_REQUEST_READSEG_SECOND, clsRequestReadSegSecond, AI_GFQ_RESPONSE_READSEG, clsResponseReadSegSecond )
	    SERVER_CASE( AI_GFQ_REQUEST_HEARTBEAT, clsRequestHeartBeat, AI_GFQ_RESPONSE_HEARTBEAT, clsResponseHeartBeat )
	    SERVER_CASE( AI_GFQ_REQUEST_ADD_MODULE, clsRequestAddModule, AI_GFQ_RESPONSE_ADD_MODULE, clsResponseAddModule )
	    SERVER_CASE( AI_GFQ_REQUEST_ADD_SEG, clsRequestAddSeg, AI_GFQ_RESPONSE_ADD_SEG, clsResponseAddSeg )
	    SERVER_CASE( AI_GFQ_REQUEST_ADD_SEG_TRY, clsRequestAddSegTry, AI_GFQ_RESPONSE_ADD_SEG, clsResponseAddSeg )
	    SERVER_CASE( AI_GFQ_REQUEST_DEL_MODULE, clsRequestDelModule, AI_GFQ_RESPONSE_DEL_MODULE, clsResponseDelModule )
	    SERVER_CASE( AI_GFQ_REQUEST_QUEUE_STAT, clsRequestQueueStat, AI_GFQ_RESPONSE_QUEUE_STAT, clsResponseQueueStat )
	    SERVER_CASE( AI_GFQ_REQUEST_PRINT_QUEUES, clsRequestPrintQueue, AI_GFQ_RESPONSE_PRINT_QUEUES, clsResponsePrintQueue )
	    SERVER_CASE( AI_GFQ_REQUEST_PRINT_MODULES, clsRequestPrintModule, AI_GFQ_RESPONSE_PRINT_MODULES, clsResponsePrintModule )
	    SERVER_CASE( AI_GFQ_REQUEST_MODULE_COUNT, clsRequestModuleCount, AI_GFQ_RESPONSE_MODULE_COUNT, clsResponseModuleCount )
	    SERVER_CASE( AI_GFQ_REQUEST_RELOADQUEUEINFO, clsRequestReloadQueueInfo, AI_GFQ_RESPONSE_RELOADQUEUEINFO, clsResponseReloadQueueInfo )
	}

	// Process exception
	AIChunkEx   loHexBuff; 
	clsResponse loError;
    loError.SetStat( AI_ERROR_INVALID_REQUST );
    UStringToHex( aoChunk.BasePtr(), aoChunk.GetSize(), loHexBuff );
    AI_GFQ_ERROR( "Master invalid request - [CMD=0x%X]/[Package=\n%s]/[Size=%d]", aiCmd, loHexBuff.BasePtr(), aoChunk.GetSize() );
    SendResponse( apoMatch, AI_GFQ_RESPONSE_ERROR, &loError );
}


//request operator
int clsMasterServer::DoRequest( clsRequestAddModule& aoRequest, clsResponseAddModule& aoResponse )
{
	int liRet = AI_NO_ERROR;
	aoResponse.SetStat(AI_NO_ERROR);
	
	if ( ( liRet = coGFQueue.AddModule( aoRequest.GetModuleName(), aoRequest.GetIpAddr(), aoRequest.GetPort() ) ) != AI_NO_ERROR )
	{
		aoResponse.SetStat(liRet);
		
		AI_GFQ_LOG( liRet, "Master add module fail - MSGCODE:%d, MSG:%s, [Module Name=%s]/[Ip=%s]/[Port=%d]",
            liRet, StrError( liRet ), aoRequest.GetModuleName(), aoRequest.GetIpAddr(), aoRequest.GetPort() );
	}
	else
	{
		AI_GFQ_DEBUG( "Master add module success, [Module Name=%s]/[Ip=%s]/[Port=%d]",
            aoRequest.GetModuleName(), aoRequest.GetIpAddr(), aoRequest.GetPort() );
	}
	
	return liRet;
}

int clsMasterServer::DoRequest( clsRequestDelModule& aoRequest, clsResponseDelModule& aoResponse )
{
	int liRet = AI_NO_ERROR;
	aoResponse.SetStat(AI_NO_ERROR);
	
	if ( ( liRet = coGFQueue.DelModule( aoRequest.GetModuleName() ) ) != AI_NO_ERROR )
	{
		aoResponse.SetStat(liRet);
		
		AI_GFQ_LOG( liRet, "Master del module fail - MSGCODE:%d, MSG:%s, [Module Name=%s]",
            liRet, StrError( liRet ), aoRequest.GetModuleName() );
	}
	else
	{
		AI_GFQ_DEBUG( "Master del module success, [Module Name=%s]", aoRequest.GetModuleName() );
	}
	
	return liRet;
}

int clsMasterServer::DoRequest( clsRequestAddSegTry& aoRequest, clsResponseAddSeg& aoResponse )
{
    int liRet = AI_NO_ERROR;
	aoResponse.SetStat(AI_NO_ERROR);
	
	stSegmentServerInfo loSegmentInfo;
	loSegmentInfo.ciSegmentID       = aoRequest.GetSegmentID();
	loSegmentInfo.ciQueueGlobalID   = aoRequest.GetQueueGlobalID();
	loSegmentInfo.ciSegmentSize     = aoRequest.GetSegmentSize();
	loSegmentInfo.ciSegmentCapacity = aoRequest.GetSegmentCapacity();
	StringCopy( loSegmentInfo.ccModuleName, aoRequest.GetModuleName(), AI_MAX_NAME_LEN );
	StringCopy( loSegmentInfo.ccQueueName, aoRequest.GetQueueName(), AI_MAX_NAME_LEN );
	
	if ( ( liRet = coGFQueue.AddSegmentTry( loSegmentInfo ) ) != AI_NO_ERROR )
	{
		aoResponse.SetStat(liRet);
		
		AI_GFQ_LOG( liRet, 
		    "Master try to add segment fail - MSGCODE:%d, MSG:%s, [Module Name=%s]/[Segment ID=%d]/[Queue Name=%s]/[Queue GlobalID=%d]",
            liRet, StrError( liRet ), aoRequest.GetModuleName(), aoRequest.GetSegmentID(),
            aoRequest.GetQueueName(), aoRequest.GetQueueGlobalID() );
	}
	else
	{
		AI_GFQ_DEBUG(  
			"Master try to add segment success, [Module Name=%s]/[Segment ID=%d]/[Queue Name=%s]/[Queue GlobalID=%d]",
            aoRequest.GetModuleName(), aoRequest.GetSegmentID(), aoRequest.GetQueueName(), aoRequest.GetQueueGlobalID() );
	}
	
	return liRet;
}

int clsMasterServer::DoRequest( clsRequestAddSeg& aoRequest, clsResponseAddSeg& aoResponse )
{
	int liRet = AI_NO_ERROR;
	aoResponse.SetStat(AI_NO_ERROR);
	
	stSegmentServerInfo loSegmentInfo;
	loSegmentInfo.ciSegmentID       = aoRequest.GetSegmentID();
	loSegmentInfo.ciQueueGlobalID   = aoRequest.GetQueueGlobalID();
	loSegmentInfo.ciSegmentSize     = aoRequest.GetSegmentSize();
	loSegmentInfo.ciSegmentCapacity = aoRequest.GetSegmentCapacity();
	StringCopy( loSegmentInfo.ccModuleName, aoRequest.GetModuleName(), AI_MAX_NAME_LEN );
	StringCopy( loSegmentInfo.ccQueueName, aoRequest.GetQueueName(), AI_MAX_NAME_LEN );
	
	if ( ( liRet = coGFQueue.AddSegment( loSegmentInfo ) ) != AI_NO_ERROR )
	{
		aoResponse.SetStat(liRet);
		
		AI_GFQ_LOG( liRet, 
		    "Master add segment fail - MSGCODE:%d, MSG:%s, [Module Name=%s]/[Segment ID=%d]/[Queue Name=%s]/[Queue GlobalID=%d]",
            liRet, StrError( liRet ), aoRequest.GetModuleName(), aoRequest.GetSegmentID(),
            aoRequest.GetQueueName(), aoRequest.GetQueueGlobalID() );
	}
	else
	{
		AI_GFQ_DEBUG(  
			"Master add segment success, [Module Name=%s]/[Segment ID=%d]/[Queue Name=%s]/[Queue GlobalID=%d]",
            aoRequest.GetModuleName(), aoRequest.GetSegmentID(), aoRequest.GetQueueName(), aoRequest.GetQueueGlobalID() );
	}
	
	return liRet;
}

int clsMasterServer::DoRequest( clsRequestWriteSeg& aoRequest, clsResponseWriteSeg& aoResponse )
{
	int liRet = AI_NO_ERROR;
	aoResponse.SetStat(AI_NO_ERROR);
	
	stSegmentClientInfo loSegmentInfo;
	if ( ( liRet = coGFQueue.GetWriteSegment( aoRequest.GetQueueName(), loSegmentInfo ) ) != AI_NO_ERROR )
	{
		aoResponse.SetStat(liRet);
		
		AI_GFQ_LOG( liRet, "Master get write segment fail - MSGCODE:%d, MSG:%s, [User=%s]/[Queue Name=%s]",
            liRet, StrError( liRet ), aoRequest.GetUserName(), aoRequest.GetQueueName() );
	}
	else
	{  
		aoResponse.SetSegmentID( loSegmentInfo.ciSegmentID );
		aoResponse.SetModuleID( loSegmentInfo.ciModuleID );
		aoResponse.SetQueueGlobalID( loSegmentInfo.ciQueueGlobalID );
		aoResponse.SetIpAddr( loSegmentInfo.ccIpAddr );
		aoResponse.SetPort( loSegmentInfo.ciPort );
		
		AI_GFQ_DEBUG( "Master get write segment success, "
			"[User=%s]/[Queue Name=%s]/[Segment ID=%d]/[Module ID=%d]/[Queue GlobalID=%d]/[Ip Addr=%s]/[Port=%d]", 
			aoRequest.GetUserName(), aoRequest.GetQueueName(), loSegmentInfo.ciSegmentID, loSegmentInfo.ciModuleID, 
			loSegmentInfo.ciQueueGlobalID, loSegmentInfo.ccIpAddr, loSegmentInfo.ciPort );
	}
	
	return liRet;
}

int clsMasterServer::DoRequest( clsRequestWriteSegSecond& aoRequest, clsResponseWriteSegSecond& aoResponse )
{
	int liRet = AI_NO_ERROR;
	int liBehave = TModuleMgr::SG_RETRY;
	    
	aoResponse.SetStat(AI_NO_ERROR);
	
	if ( ( liRet = coGFQueue.WriteSegmentFail( aoRequest.GetQueueName(), aoRequest.GetSegmentID(), 
		aoRequest.GetModuleID(), aoRequest.GetRetcode(), &liBehave ) ) != AI_NO_ERROR )
	{
		aoResponse.SetStat(liRet);
		
		AI_GFQ_LOG( liRet, 
		    "Master process api put data exception fail - MSGCODE:%d, MSG:%s, [User=%s]/[Queue Name=%s]/[Segment ID=%d]/[Module ID=%d]/[Retcode=%d]",
            liRet, StrError( liRet ), aoRequest.GetUserName(), aoRequest.GetQueueName(), aoRequest.GetSegmentID(), aoRequest.GetModuleID(), aoRequest.GetRetcode() );
        
        return liRet;
	}
	
	switch( liBehave )
	{
	    case TModuleMgr::SG_RETRY:
	    {
	        AI_GFQ_DEBUG( 
		        "Master process api put data exception success, [User=%s]/[Queue Name=%s]/[Segment ID=%d]/[Module ID=%d]/[Retcode=%d][RETRY]",
                aoRequest.GetUserName(), aoRequest.GetQueueName(), aoRequest.GetSegmentID(), aoRequest.GetModuleID(), aoRequest.GetRetcode() );
            break;
        }
        case TModuleMgr::SG_SWITCH:
	    {
	        AI_GFQ_DEBUG(
		        "Master process api put data exception success, [User=%s]/[Queue Name=%s]/[Segment ID=%d]/[Module ID=%d]/[Retcode=%d][SWITCH]",
                aoRequest.GetUserName(), aoRequest.GetQueueName(), aoRequest.GetSegmentID(), aoRequest.GetModuleID(), aoRequest.GetRetcode() );
            break;
        }
        case TModuleMgr::SG_RETURN:
        {
            AI_GFQ_DEBUG(
		        "Master process api put data exception success, [User=%s]/[Queue Name=%s]/[Segment ID=%d]/[Module ID=%d]/[Retcode=%d][RETURN]",
                aoRequest.GetUserName(), aoRequest.GetQueueName(), aoRequest.GetSegmentID(), aoRequest.GetModuleID(), aoRequest.GetRetcode() );
        
            // Report error here
            aoResponse.SetStat( aoRequest.GetRetcode() );
        
            return liRet;
         }
	}
	
	stSegmentClientInfo loSegmentInfo;
	if ( ( liRet = coGFQueue.GetWriteSegment( aoRequest.GetQueueName(), loSegmentInfo ) ) != AI_NO_ERROR )
	{
		aoResponse.SetStat(liRet);
		
		AI_GFQ_LOG( liRet, "Master get writeable segment second fail - MSGCODE:%d, MSG:%s, [User=%s]/[Queue Name=%s]/[Retcode=%d]",
            liRet, StrError( liRet ), aoRequest.GetUserName(), aoRequest.GetQueueName(), aoRequest.GetRetcode() );
	}
	else
	{
		aoResponse.SetSegmentID( loSegmentInfo.ciSegmentID );
		aoResponse.SetModuleID( loSegmentInfo.ciModuleID );
		aoResponse.SetQueueGlobalID( loSegmentInfo.ciQueueGlobalID );
		aoResponse.SetIpAddr( loSegmentInfo.ccIpAddr );
		aoResponse.SetPort( loSegmentInfo.ciPort );
		
		AI_GFQ_DEBUG( "Master get writeable segment second success, "
			"[User=%s]/[Queue Name=%s]/[Segment ID=%d]/[Module ID=%d]/[Queue GlobalID=%d]/[Ip Addr=%s]/[Port=%d]", 
			aoRequest.GetUserName(), aoRequest.GetQueueName(), loSegmentInfo.ciSegmentID, loSegmentInfo.ciModuleID, 
			loSegmentInfo.ciQueueGlobalID, loSegmentInfo.ccIpAddr, loSegmentInfo.ciPort );
	}
	
	return liRet;
}

int clsMasterServer::DoRequest( clsRequestReadSeg& aoRequest, clsResponseReadSeg& aoResponse )
{
	int liRet = AI_NO_ERROR;
	aoResponse.SetStat(AI_NO_ERROR);

	stSegmentClientInfo loSegmentInfo;
	if ( ( liRet = coGFQueue.GetReadSegment( aoRequest.GetQueueName(), loSegmentInfo ) ) != AI_NO_ERROR )
	{
	    int liTmpRet = liRet;
	    // convert retcode
	    if ( liRet == AI_ERROR_NO_FOUND_QUEUE ) liTmpRet = AI_ERROR_END_OF_QUEUE;
	    
		aoResponse.SetStat(liTmpRet);
		
		AI_GFQ_LOG( liRet, "Master get readable segment fail - MSGCODE:[%d/%d], MSG:%s, [User=%s]/[Queue Name=%s]",
            liRet, liTmpRet, StrError( liRet ), aoRequest.GetUserName(), aoRequest.GetQueueName() );
	}
	else
	{
		aoResponse.SetSegmentID( loSegmentInfo.ciSegmentID );
		aoResponse.SetModuleID( loSegmentInfo.ciModuleID );
		aoResponse.SetQueueGlobalID( loSegmentInfo.ciQueueGlobalID );
		aoResponse.SetIpAddr( loSegmentInfo.ccIpAddr );
		aoResponse.SetPort( loSegmentInfo.ciPort );
		
		AI_GFQ_DEBUG( "Master get readable segment success, "
			"[User=%s]/[Queue Name=%s]/[Segment ID=%d]/[Module ID=%d]/[Queue GlobalID=%d]/[Ip Addr=%s]/[Port=%d]", 
			aoRequest.GetUserName(), aoRequest.GetQueueName(), loSegmentInfo.ciSegmentID, loSegmentInfo.ciModuleID, 
			loSegmentInfo.ciQueueGlobalID, loSegmentInfo.ccIpAddr, loSegmentInfo.ciPort );
	}
	
	return liRet;
}

int clsMasterServer::DoRequest( clsRequestReadSegSecond& aoRequest, clsResponseReadSegSecond& aoResponse )
{
	int liRet = AI_NO_ERROR;
	int liBehave = TModuleMgr::SG_RETRY;
	    
	aoResponse.SetStat(AI_NO_ERROR);
	
	if ( ( liRet = coGFQueue.ReadSegmentFail( aoRequest.GetQueueName(), aoRequest.GetSegmentID(), 
		aoRequest.GetModuleID(), aoRequest.GetRetcode(), &liBehave ) ) != AI_NO_ERROR )
	{
		aoResponse.SetStat(liRet);
		
		AI_GFQ_LOG( liRet, 
		    "Master process api get data exception fail - MSGCODE:%d, MSG:%s, [User=%s]/[Queue Name=%s]/[Segment ID=%d]/[Module ID=%d]/[Retcode=%d]",
            liRet, StrError( liRet ), aoRequest.GetUserName(), aoRequest.GetQueueName(), aoRequest.GetSegmentID(), aoRequest.GetModuleID(), aoRequest.GetRetcode() );
        
        return liRet;
	}
	
	switch( liBehave )
	{
	    case TModuleMgr::SG_RETRY:
	    {
	        AI_GFQ_DEBUG( 
		        "Master process api get data exception success, [User=%s]/[Queue Name=%s]/[Segment ID=%d]/[Module ID=%d]/[Retcode=%d][RETRY]",
                aoRequest.GetUserName(), aoRequest.GetQueueName(), aoRequest.GetSegmentID(), aoRequest.GetModuleID(), aoRequest.GetRetcode() );
            break;
        }
        case TModuleMgr::SG_SWITCH:
	    {
	        AI_GFQ_DEBUG( 
		        "Master process api get data exception success, [User=%s]/[Queue Name=%s]/[Segment ID=%d]/[Module ID=%d]/[Retcode=%d][SWITCH]",
                aoRequest.GetUserName(), aoRequest.GetQueueName(), aoRequest.GetSegmentID(), aoRequest.GetModuleID(), aoRequest.GetRetcode() );
            break;
        }
        case TModuleMgr::SG_RETURN:
        {
            AI_GFQ_DEBUG( 
		        "Master process api get data exception success, [User=%s]/[Queue Name=%s]/[Segment ID=%d]/[Module ID=%d]/[Retcode=%d][RETURN]",
                aoRequest.GetUserName(), aoRequest.GetQueueName(), aoRequest.GetSegmentID(), aoRequest.GetModuleID(), aoRequest.GetRetcode() );
            // Report error here
            aoResponse.SetStat( aoRequest.GetRetcode() );
        
            return liRet;
        }
	}
	
	stSegmentClientInfo loSegmentInfo;
	if ( ( liRet = coGFQueue.GetReadSegment( aoRequest.GetQueueName(), loSegmentInfo ) ) != AI_NO_ERROR )
	{
		aoResponse.SetStat(liRet);
		
		AI_GFQ_LOG( liRet, "Master get readable segment second fail - MSGCODE:%d, MSG:%s, [User=%s]/[Queue Name=%s]/[Retcode=%d]",
            liRet, StrError( liRet ), aoRequest.GetUserName(), aoRequest.GetQueueName(), aoRequest.GetRetcode() );
	}
	else
	{
		aoResponse.SetSegmentID( loSegmentInfo.ciSegmentID );
		aoResponse.SetModuleID( loSegmentInfo.ciModuleID );
		aoResponse.SetQueueGlobalID( loSegmentInfo.ciQueueGlobalID );
		aoResponse.SetIpAddr( loSegmentInfo.ccIpAddr );
		aoResponse.SetPort( loSegmentInfo.ciPort );
		
		AI_GFQ_DEBUG( "Master get readable segment second success, "
			"[User=%s]/[Queue Name=%s]/[Segment ID=%d]/[Module ID=%d]/[Queue GlobalID=%d]/[Ip Addr=%s]/[Port=%d]", 
			aoRequest.GetUserName(), aoRequest.GetQueueName(), loSegmentInfo.ciSegmentID, loSegmentInfo.ciModuleID, 
			loSegmentInfo.ciQueueGlobalID, loSegmentInfo.ccIpAddr, loSegmentInfo.ciPort );
	}
	
	return liRet;
}

int clsMasterServer::DoRequest( clsRequestQueueStat& aoRequest, clsResponseQueueStat& aoResponse )
{
	int liRet     = AI_NO_ERROR;
	size_t liSize     = 0;
	size_t liCapacity = 0;
	
	aoResponse.SetStat(AI_NO_ERROR);
	
	if ( ( liRet = coGFQueue.GetQueueStat( aoRequest.GetQueueName(), &liSize, &liCapacity ) ) != AI_NO_ERROR )
	{
		aoResponse.SetStat(liRet);
		
		AI_GFQ_LOG( liRet, "Master get queue stat fail - MSGCODE:%d, MSG:%s, [User=%s]/[Queue Name=%s]",
            liRet, StrError( liRet ), aoRequest.GetUserName(), aoRequest.GetQueueName() );
	}
	else
	{
		aoResponse.SetQueueSize((int)liSize);
		aoResponse.SetQueueCapacity((int)liCapacity);
		AI_GFQ_DEBUG( "Master get queue stat success, [User=%s]/[Module Name=%s]", aoRequest.GetUserName(), aoRequest.GetQueueName() );
	}
	
	return liRet;
}

int clsMasterServer::DoRequest( clsRequestReloadQueueInfo& aoRequest, clsResponseReloadQueueInfo& aoResponse )
{
	int liRet = AI_NO_ERROR;
	aoResponse.SetStat(AI_NO_ERROR);
	
	if ( ( liRet = coGFQueue.LoadQueueInfo() ) != AI_NO_ERROR )
	{
		aoResponse.SetStat(liRet);
		
		AI_GFQ_LOG( liRet, "Master Load queue info fail - MSGCODE:%d, MSG:%s, [User=%s]",
            liRet, StrError( liRet ), aoRequest.GetUserName() );
	}
	else
	{
	    // Refresh All queue info, e.g queue capacity
	    coGFQueue.RefreshQueue();
	    
		AI_GFQ_DEBUG( "Master Load queue info success, [User=%s]", aoRequest.GetUserName() );
	}
	
	return liRet;
}

int clsMasterServer::DoRequest( clsRequestHeartBeat& aoRequest, clsResponseHeartBeat& aoResponse )
{
    int liRet = AI_NO_ERROR;

    liRet = coGFQueue.HeartBeat( aoRequest.GetModuleName() );
    if (liRet != AI_NO_ERROR)
    {
        AI_GFQ_LOG( liRet, "Master heart beat fail - MSGCODE:%d, MSG:%s, [Module=%s]",
            liRet, StrError( liRet ), aoRequest.GetModuleName() );
    }
    else
    {
        AI_GFQ_DEBUG( "Master heart beat success, [Module=%s]", aoRequest.GetModuleName() );
    }
    
    aoResponse.SetStat(liRet);

    return liRet;
}

int clsMasterServer::DoRequest( clsRequestPrintQueue& aoRequest, clsResponsePrintQueue& aoResponse )
{
	int                     liRet = AI_NO_ERROR;
	AI_STD::vector<stQueueInfo> loQueueInfos;
	
	aoResponse.SetStat(AI_NO_ERROR);
	
	coGFQueue.GetAllQueueInfo( loQueueInfos );
	
	AI_GFQ_INFO( "---------------------------------------------------------------------------\n" );
	AI_GFQ_INFO( "|                                  Queue Info                             |\n" );
	AI_GFQ_INFO( "---------------------------------------------------------------------------\n" );
	AI_GFQ_INFO( "|%-35s|%-10s|%-15s|%-10s|", "QueueName", "Size", "SegmentCnt", "Capacity" );
	AI_GFQ_INFO( "---------------------------------------------------------------------------\n" );
	for( size_t liIt = 0; liIt < loQueueInfos.size(); liIt++ )
	{
		AI_GFQ_INFO( "|%-35s|%-10d|%-15d|%-10d|", 
		    loQueueInfos[liIt].ccQueueName, loQueueInfos[liIt].ciQueueSize,
		    loQueueInfos[liIt].ciSegmentCnt, loQueueInfos[liIt].ciQueueCapacity );
	}
    AI_GFQ_INFO( "---------------------------------------------------------------------------\n" );
    
	return liRet;
}

int clsMasterServer::DoRequest( clsRequestPrintModule& aoRequest, clsResponsePrintModule& aoResponse )
{
	int                     liRet = AI_NO_ERROR;
	AI_STD::vector<stModuleInfo> loModuleInfos;
	    
	aoResponse.SetStat(AI_NO_ERROR);
	
	coGFQueue.GetAllModuleInfo( loModuleInfos );

    AI_GFQ_INFO( "---------------------------------------------------------------------------\n" );
	AI_GFQ_INFO( "|                                  Module Info                            |\n" );
	AI_GFQ_INFO( "---------------------------------------------------------------------------\n" );
	AI_GFQ_INFO( "|%-30s|%-10s|%-15s|%-15s|", "ModuleName", "SegmentCnt", "UsingSegmentCnt", "FreeSegmentCnt" );
	AI_GFQ_INFO( "---------------------------------------------------------------------------\n" );
	for( size_t liIt = 0; liIt < loModuleInfos.size(); liIt++ )
	{
		AI_GFQ_INFO( "|%-30s|%-10d|%-15d|%-15d|", 
		    loModuleInfos[liIt].ccModuleName, loModuleInfos[liIt].ciSegmentCnt,
		    loModuleInfos[liIt].ciUsingSegmentCnt, loModuleInfos[liIt].ciFreeSegmentCnt );
	}
    AI_GFQ_INFO( "---------------------------------------------------------------------------\n" );
    
	return liRet;
}

int clsMasterServer::DoRequest( clsRequestModuleCount& aoRequest, clsResponseModuleCount& aoResponse )
{
	int                     liRet = AI_NO_ERROR;
	AI_STD::vector<stModuleInfo> loModuleInfos;
	    
	aoResponse.SetStat(AI_NO_ERROR);
	
	coGFQueue.GetAllModuleInfo( loModuleInfos );
	
    AI_GFQ_DEBUG( "Master get module count success, [ModuleCount=%d]", (int)loModuleInfos.size() );
    
    aoResponse.SetModuleCount((int)loModuleInfos.size() );
    
	return liRet;
}

void clsMasterServer::PrintStat()
{
    AI_GFQ_INFO( "-----------------------------------------------------------------------\n" );
    AI_GFQ_INFO( " Queue count                = %d", coGFQueue.GetQueueCount() );
    AI_GFQ_INFO( " Module count               = %d", coGFQueue.GetModuleCount() );
    AI_GFQ_INFO( " Module segment count       = %d", coGFQueue.GetSegmentCount() );
    AI_GFQ_INFO( " Module using segment count = %d", coGFQueue.GetUsingSegmentCount() );
    AI_GFQ_INFO( " Module free segment count  = %d", coGFQueue.GetFreeSegmentCount() );
    AI_GFQ_INFO( " Process Request count      = %dps", coRequestStat.Frequency() );
    coRequestStat.Reset();
    
    //for thread pool stat
	AI_GFQ_INFO( " ThreadPool Actives = %d / Max = %d / CacheSize = %d / CacheCapacity = %d", 
	    cpoThreadPool->GetActiveThreadCount(), ciThreadCount, cpoThreadPool->GetCacheSize(), cpoThreadPool->GetCacheCapacity() );
	AI_GFQ_INFO( "-----------------------------------------------------------------------\n" );
}

//send response operator
void clsMasterServer::SendResponse( const ASYNCMATCH *apoMatch, int aiCmd, clsBody* apoBody )
{
    int liRet = AI_NO_ERROR;
	clsHeader loHeader( aiCmd, apoBody );
    AIChunkEx loChunk( loHeader.GetSize() );
    loHeader.Encode( loChunk );

	if ( ( liRet = this->SendResult( apoMatch, loChunk.BasePtr(), loChunk.GetSize(), 0 ) ) != 0 )
	{
	    AI_GFQ_ERROR( "Send response fail - ASYNCCODE:%d, MSG:%s, [Response Cmd=0x%x]\n", liRet, strerror(errno), aiCmd );
	}
	
	// Trace log
    AI_GFQ_TRACE( "Responsed request, [MsgID=%d]/[ThreadID=%d]", apoMatch->ciMsgID, apoMatch->ctThreadID );
}

AI_GFQ_NAMESPACE_END
