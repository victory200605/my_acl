//=============================================================================
/**
 * \file    SegmentServer.cpp
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: SegmentServer.cpp,v 1.1 2010/11/23 06:35:53 daizh Exp $
 *
 * History
 * 2008.04.04 first release by DZH
 */
//=============================================================================
#include "SegmentServer.h"
#include "MemoryStore.h"

AI_GFQ_NAMESPACE_START

// thread param sub struct constructor
clsSegmentServer::stThreadParam::stThreadParam( 
    clsSegmentServer* apoServer, const ASYNCMATCH* apoMatch, char* apsData, size_t aiSize ) :
    cpoServer(apoServer),
    coData( apsData, aiSize ),
    ciCmd(0)
{
    memcpy( &coMatch, apoMatch, sizeof(ASYNCMATCH) );
}

clsSegmentServer::clsSegmentServer( const unsigned short aiBindPort ) : 
    clsAsyncServer( "", aiBindPort ),
    cpoThreadPool(NULL),
    ciMasterPort(0),
    cpoMaster(NULL),
    ciShutDownTimeout(0),
    ciPrintInterval(10), // Interval 10 sec
    ciHeartBeatInterval(10),
    ciRegisterIdlePercent(0),
    cdTrapLimitPercent(100),
    cbIsService(false)
{
}

clsSegmentServer::~clsSegmentServer()
{
    cpoThreadPool = NULL;
}

int clsSegmentServer::Initialize( const char *apcConfigFile, const char* apsClientConfig )
{
    AIConfig *lpoIni = NULL;
    AIConfig *lpoClientIni = NULL;
    int  liRet = AI_NO_ERROR;
    int  liTimeOut = 5;
    int  liThreadCount = 0;
    int  liThreadPoolCache = 0;
    char lcStorage[64];
    int  liSegmentCount = 0;
    int  liSegmentCapacity = 0;
    char lcSegmentFileDir[AI_MAX_NAME_LEN];
    
    //Module info
    int liMaxChildCount   = 0;
    int liAdminListenPort = 0;
    int liSegmentServerPort = -1;
    char lcModuleID[AI_MAX_NAME_LEN];
    char lcAdminListenIpAddr[AI_IP_ADDR_LEN];
    
    if ((lpoIni = AIGetIniHandler(apcConfigFile)) == NULL)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "Load config file %s fail", apcConfigFile);
        return 1;
    }
    
    if ((lpoClientIni = AIGetIniHandler(apsClientConfig)) == NULL)
    {
        AI_GFQ_ERROR( "Load config file %s fail", apsClientConfig);
        return 1;
    }

    AI_GFQ_INFO( ">>>>>>>>>>>>>>>>>>>>>>>>>>SegmentServer<<<<<<<<<<<<<<<<<<<<<<<<<<<" );
    
    //module config info
    liMaxChildCount        = lpoIni->GetIniInt( "SERVER", "MaxChildCount", -1 );
    liAdminListenPort      = lpoIni->GetIniInt( "SERVER", "AdminListenPort", -1 );
    liSegmentServerPort    = lpoIni->GetIniInt( "SERVER", "SegmentServerPort", -1 );
    lpoIni->GetIniString( "SERVER", "ModuleID", lcModuleID, sizeof(lcModuleID) );
    lpoIni->GetIniString( "SERVER", "AdminListenIpAddr", lcAdminListenIpAddr, sizeof(lcAdminListenIpAddr) );
    
    //load config variable
    liThreadCount     = lpoIni->GetIniInt( "SegmentServer", "ThreadPool", 500 );
    liThreadPoolCache = lpoIni->GetIniInt( "SegmentServer", "ThreadPoolCache", 5000 );
    ciShutDownTimeout = lpoIni->GetIniInt( "SegmentServer", "ShutDownTimeout", 0 );
    ciPrintInterval   = lpoIni->GetIniInt( "SegmentServer", "PrintInterval", 10 );
    ciHeartBeatInterval = lpoIni->GetIniInt( "SegmentServer", "HeartBeatInterval", 10 );
    ciRegisterIdlePercent = lpoIni->GetIniInt( "SegmentServer", "RegisterIdlePercent", 50 );
    cdTrapLimitPercent    = lpoIni->GetIniInt( "SegmentServer", "TrapLimitPercent", 100 );
    
    //load segment store config
    lpoIni->GetIniString("SegmentServer", "ServerIp", ccServerIp, sizeof(ccServerIp));
    ciServerPort = lpoIni->GetIniInt( "SegmentServer", "ListenPort", -1 );
    lpoIni->GetIniString("SegmentServer", "ModuleName", ccModuleName, sizeof(ccModuleName));
    lpoIni->GetIniString("SegmentServer", "Storage", lcStorage, sizeof(lcStorage));
    if (strcmp(lcStorage, "memory") == 0)
    {
        liSegmentCount    = lpoIni->GetIniInt( "SegmentServer", "SegmentCount", 10);
        liSegmentCapacity = lpoIni->GetIniInt( "SegmentServer", "SegmentCapacity", 10000);
        
        AI_NEW_ASSERT(this->cpoSegmentStore, CMemoryStore(liSegmentCount, liSegmentCapacity) );
    }
    else
    {
        lpoIni->GetIniString("SegmentServer", "SegmentDir", lcSegmentFileDir, sizeof(lcSegmentFileDir));
        
        AI_NEW_ASSERT(this->cpoSegmentStore, clsSegmentStore(lcSegmentFileDir) );
    }
    
    if ( (liRet = this->cpoSegmentStore->Initialize() ) != 0)
    {
        AI_GFQ_ERROR( "Open Segment store %s fail, MSGCODE:%d, MSG:%s", lcSegmentFileDir, liRet, StrError(liRet) );
    }

    //load master address
    lpoClientIni->GetIniString("Master", "ServerIP", ccMasterIp, sizeof(ccMasterIp));
    ciMasterPort = lpoClientIni->GetIniInt( "Master", "ServerPort", 8012 );
    liTimeOut    = lpoClientIni->GetIniInt( "Master", "TimeOut", 5 );

    ciServerPort = liSegmentServerPort;
    StringCopy( ccServerIp,    lcAdminListenIpAddr, sizeof(ccServerIp) );
    StringCopy( ccModuleName,  lcModuleID, sizeof(ccModuleName) );

    AI_NEW_ASSERT( cpoThreadPool, AITask );
    if ( cpoThreadPool->Initialize( liThreadCount, liThreadPoolCache ) != 0 )
    {
        AI_GFQ_ERROR( "Initialize Task fail, MSG:May be thread pool create fail" );
        return 1;
    }
    ciThreadCount = liThreadCount;
    
    // Create client
    AI_NEW_ASSERT( cpoMaster,  clsAsyncClient( ccMasterIp, ciMasterPort, 1, liTimeOut ) );
    
    //module configure
    {
        AI_GFQ_INFO( "----------------Module Config Variable-----------------" );
        AI_GFQ_INFO( "* ModuleID                = %s", lcModuleID);
        AI_GFQ_INFO( "* MaxChildCount           = %d", liMaxChildCount);
        AI_GFQ_INFO( "* AdminListenPort         = %d", liAdminListenPort);
        AI_GFQ_INFO( "* AdminListenIpAddr       = %s", lcAdminListenIpAddr);
        AI_GFQ_INFO( "* SegmentServerPort       = %d", liSegmentServerPort);
    }

    //segment configure
    {
        AI_GFQ_INFO( "------------Segement Server Config Variable------------" );
        AI_GFQ_INFO( "* Listen Port             = %d", ciServerPort);
        AI_GFQ_INFO( "* Thread Pool             = %d", liThreadCount);
        AI_GFQ_INFO( "* Thread Pool Cache       = %d", liThreadPoolCache);
        AI_GFQ_INFO( "* Shutdown Timeout        = %d", ciShutDownTimeout);
        AI_GFQ_INFO( "* Master Port             = %d", ciMasterPort);
        AI_GFQ_INFO( "* Master IpAdress         = %s", ccMasterIp);
        AI_GFQ_INFO( "* Print Interval          = %d", ciPrintInterval);
        AI_GFQ_INFO( "* Heart Beat Interval     = %d", ciHeartBeatInterval);
        AI_GFQ_INFO( "* Trap Limit Percent      = %d", (int)cdTrapLimitPercent);
    }
    
    {
        AI_GFQ_INFO( "------------Store Node Info----------------------------" );
        AI_GFQ_INFO( "* Segment Count           = %d", cpoSegmentStore->GetSegmentCount() );
        AI_GFQ_INFO( "* Segment Capacity        = %d", cpoSegmentStore->GetSegmentCapacity());
        AI_GFQ_INFO( "* Module Name             = %s", ccModuleName);
        AI_GFQ_INFO( "* Server Ip               = %s", ccServerIp);
        AI_GFQ_INFO( "* Server Port             = %d", ciServerPort);

        if (strcmp(lcStorage, "memory") == 0)
        {
            AI_GFQ_INFO( "* Storage Type            = %s", "memory");
            AI_GFQ_INFO( "* Segment Count           = %d", liSegmentCount);
            AI_GFQ_INFO( "* Segment Capacity        = %d", liSegmentCapacity);
        }
        else
        {
            AI_GFQ_INFO( "* Storage Type            = %s", "file");
            AI_GFQ_INFO( "* Storage Path            = %s", lcSegmentFileDir);
        }
    }
    
    return 0;
}

int clsSegmentServer::HeartBeat(void)
{
    int liRet = 0;
    clsRequestHeartBeat  loRequest;
    clsResponseHeartBeat loResponse;
    
    loRequest.SetModuleName( ccModuleName );
    if ( ( liRet= DoRequestTo( *cpoMaster, AI_GFQ_REQUEST_HEARTBEAT, loRequest, loResponse) ) != AI_NO_ERROR )
    {
        AI_GFQ_ERROR( "HeartBeat fail, MSGCODE:%d, MSG:%s", liRet, StrError(liRet) );
    }
    else
    {
        AI_GFQ_DEBUG( "HeartBeat success" );
    }
    
    return liRet;
}

int clsSegmentServer::DelModule(void)
{
    int liRet = 0;
    clsRequestDelModule  loRequest;
    clsResponseDelModule loResponse;
    
    loRequest.SetModuleName( ccModuleName );
    if ( ( liRet= DoRequestTo( *cpoMaster, AI_GFQ_REQUEST_DEL_MODULE, loRequest, loResponse) ) != AI_NO_ERROR )
    {
        AI_GFQ_ERROR( "Delete module fail, MSGCODE:%d, MSG:%s", liRet, StrError(liRet) );
    }
    else
    {
        AI_GFQ_DEBUG( "Delete module fail" );
    }
    
    return liRet;
}

int clsSegmentServer::RegisterToMaster( const char* apsModuleName, const char* apsServerIP, int aiServerPort )
{
    int liRet = AI_NO_ERROR;
    
    int liSegmentCount = cpoSegmentStore->GetSegmentCount();
    clsRequestAddSeg loAddSeg;
    CSegmentInfo     loSegmentInfo;
    
    loAddSeg.SetModuleName( apsModuleName );
    loAddSeg.SetSegmentCapacity(cpoSegmentStore->GetSegmentCapacity());
    clsResponseAddSeg loResponseAddSeg;
    
    // Try to add segment
    for ( int liIt = 0; liIt < liSegmentCount; liIt++ )
    {
        cpoSegmentStore->GetSegmentInfo(liIt, loSegmentInfo);
        
        loAddSeg.SetSegmentID(liIt);
        loAddSeg.SetSegmentSize(loSegmentInfo.GetSize() );
        loAddSeg.SetQueueGlobalID(loSegmentInfo.GetQueueGlobalID() );
        loAddSeg.SetQueueName(loSegmentInfo.GetQueueName() );

        if ( ( liRet= DoRequestTo( *cpoMaster, AI_GFQ_REQUEST_ADD_SEG_TRY, loAddSeg, loResponseAddSeg  ) ) != AI_NO_ERROR )
        {
            AI_GFQ_ERROR( "Register Segment Server %s to [%s:%d] fail, MSGCODE:%d, MSG:%s [Try to add segment]", 
                ccModuleName, ccMasterIp, ciMasterPort, liRet, StrError(liRet) );
            return liRet;
        }
    }
    
    clsRequestAddModule loAddModule;
    loAddModule.SetModuleName(apsModuleName);
    loAddModule.SetIpAddr(apsServerIP);
    loAddModule.SetPort(aiServerPort);
    clsResponseAddModule loResponseAddModule;
    if ( ( liRet= DoRequestTo( *cpoMaster, AI_GFQ_REQUEST_ADD_MODULE, loAddModule, loResponseAddModule  ) ) != AI_NO_ERROR )
    {
        AI_GFQ_ERROR( "Register Segment Server %s to [%s:%d] fail, MSGCODE:%d, MSG:%s [Add module]", 
            ccModuleName, ccMasterIp, ciMasterPort, liRet, StrError(liRet) );
        return liRet;
    }

    // Add segment start
    for ( int liIt = 0; liIt < liSegmentCount; liIt++ )
    {
        cpoSegmentStore->GetSegmentInfo(liIt, loSegmentInfo);
        
        loAddSeg.SetSegmentID(liIt);
        loAddSeg.SetSegmentSize(loSegmentInfo.GetSize() );
        loAddSeg.SetQueueGlobalID(loSegmentInfo.GetQueueGlobalID() );
        loAddSeg.SetQueueName(loSegmentInfo.GetQueueName() );

        if ( ( liRet= DoRequestTo( *cpoMaster, AI_GFQ_REQUEST_ADD_SEG, loAddSeg, loResponseAddSeg  ) ) != AI_NO_ERROR )
        {
            AI_GFQ_ERROR( "Register Segment Server %s to [%s:%d] fail, MSGCODE:%d, MSG:%s [Add segment]", 
                ccModuleName, ccMasterIp, ciMasterPort, liRet, StrError(liRet) );
            
            this->DelModule();
            
            return liRet;
        }
    }
    
    AI_GFQ_INFO( "Register Segment Server %s success, [Master Ip=%s:%d]", ccModuleName, ccMasterIp, ciMasterPort );

    return AI_NO_ERROR;
}

int clsSegmentServer::Startup(void)
{
    int liRet = AI_NO_ERROR;

    if ( ( liRet = this->StartDaemon( "SegmentServerPort", GetNewConnection, GetListenSocket ) ) != 0 )
    {
        switch( liRet )
        {
            case ASYNC_ERROR_LISTENFAIL:
                AI_GFQ_ERROR( "SegmentServer startup fail, ErrMsg=AsyncServer Listen port fail, SysMsg:%s", strerror(errno) );
                break;
            default:
                AI_GFQ_ERROR( "SegmentServer startup fail, ErrMsg=System error, SysMsg:%s", strerror(errno) );
                break;
        }

        return -1;
    }

    do
    {
        if ( (liRet = AI_GFQ_INITIALIZE_TRAP() ) != 0 )
        {
            AI_GFQ_ERROR( "SegmentServer initialize statserver fail, ErrMsg=%s, RetCode:%d", strerror(errno), liRet );
            
            break;
        }
        
        //Connect To Master
        liRet = cpoMaster->StartDaemon(30);
        if ( liRet != 1 )
        {
            AI_GFQ_ERROR( "Connect to Master [%s:%d] fail, MSGCODE:%d, MSG:%s", 
                ccMasterIp, ciMasterPort, AI_ERROR_SYSTEM, StrError(AI_ERROR_SYSTEM) );
            // create connect thread false

            break;
        }
        
        cbIsService = true;
    
        if ( ( liRet = this->RegisterToMaster(ccModuleName, ccServerIp, ciServerPort) ) != AI_NO_ERROR )
        {
            break;
        }
        
        AI_GFQ_INFO( "SegmentServer startup success");
        fprintf(stderr, "SegmentServer startup success\n");
                
        int liTimeCount = 0;
        int liHeartBeatTimeCount = 0;
        while ( !IsShutdown() )
        {
            if ( liHeartBeatTimeCount >= ciHeartBeatInterval )
            {
                if (this->HeartBeat() == AI_ERROR_NO_FOUND_MODULE)
                {
                    cbIsService = false;
                }
                
                liHeartBeatTimeCount = 0;
            }
            
            if ( !cbIsService )
            {
                if ( (float(cpoThreadPool->GetActiveThreadCount() ) / (size_t)ciThreadCount) * 100  
                     < ciRegisterIdlePercent )
                {
                    cbIsService = true;
                
                    this->RegisterToMaster(ccModuleName, ccServerIp, ciServerPort);
                }
                else
                {
                    AI_GFQ_INFO(
                        "Delay Register because [Busy=%f%% >= %d%%]",
                        (float(cpoThreadPool->GetActiveThreadCount() ) / (size_t)ciThreadCount) * 100,
                        ciRegisterIdlePercent );
                }
            }
            
            if ( liTimeCount >= ciPrintInterval )
            {
                this->PrintStat();
                liTimeCount = 0;
            }
            
            sleep(1);
            liTimeCount++; // Time count
            liHeartBeatTimeCount++;
        }
    }while(false);
    
    //Stop server
    clsAsyncServer::ShutDown();
    
    //Destroy thread
    AI_DELETE( cpoThreadPool );

    //Logout module
    this->DelModule();
    cpoMaster->ShutDown();
    AI_DELETE(cpoMaster);
    
    //Close trap api
    AI_GFQ_CLOSE_TRAP();
    
    //Close segment store
    cpoSegmentStore->Close();
    
    AI_GFQ_ERROR( "SegmentServer shutdown" );
   
    return 0;
}

bool clsSegmentServer::IsShutdown()
{
    return giGlobalShutDown == 1;
}

//thread operator
void clsSegmentServer::Callback( const ASYNCMATCH *apoMatch, char *apcData, const int aiSize )
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
    
    // Master Server request
    switch( lpoParam->ciCmd )
    {
        case AI_GFQ_REQUEST_ALLOC:
	    case AI_GFQ_REQUEST_FREE:
	    case AI_GFQ_REQUEST_SEGMENTSIZE:
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
            4003,
            "SegmentServer ThreadPool full",
            "SegmentServer ThreadPool full, Current=%d, Max=%d, Wait=%d, Capacity=%d",
            cpoThreadPool->GetActiveThreadCount(),
            ciThreadCount,
            cpoThreadPool->GetCacheSize(),
            cpoThreadPool->GetCacheCapacity() );
 
        AI_DELETE( lpoParam );
        
        AI_GFQ_ERROR( "Attach thread fail MSGCODE:%d, MSG:%s",  AI_ERROR_THREAD_POOL_FULL, StrError( AI_ERROR_THREAD_POOL_FULL ) );
        
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
                4003,
                "SegmentServer ThreadPool full",
                "SegmentServer ThreadPool full, Current=%d, Max=%d, Wait=%d, Capacity=%d",
                cpoThreadPool->GetActiveThreadCount(),
                ciThreadCount,
                (int)ldWait,
                liCapacity );
        }
	}
}

void* clsSegmentServer::HandleRequestEntry( void *apvParam )
{
    stThreadParam* lpoParam = (stThreadParam*)apvParam;
    
    // Trace log
    AI_GFQ_TRACE( "Handle request now, [MsgID=%d]/[ThreadID=%d]", lpoParam->coMatch.ciMsgID, lpoParam->coMatch.ctThreadID );
    
    lpoParam->cpoServer->HandleRequestThread( &lpoParam->coMatch, lpoParam->ciCmd, lpoParam->coData );
    
    AI_DELETE( lpoParam );
    
    return NULL;
}

void clsSegmentServer::HandleRequestThread( ASYNCMATCH *apoMatch, int aiCmd, AIChunkEx& aoChunk )
{
    // Statistic request, unprotected value
    coRequestStat.Request();
    
    // case request or response process define macro
#define SERVER_CASE( requestvalue, request, responsevalue, response )           \
    case requestvalue:                                                          \
    {                                                                           \
        request loRequest;                                                      \
        response loResponse;                                                    \
        if ( loRequest.Decode(aoChunk) != 0 )                                   \
        {                                                                       \
            break;                                                              \
        }                                                                       \
        if (cbIsService)                                                        \
        {                                                                       \
            this->DoRequest( loRequest, loResponse );                           \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            loResponse.SetStat(AI_ERROR_SERVER_DENIED);                         \
        }                                                                       \
        this->SendResponse( apoMatch, responsevalue, &loResponse );             \
        return;                                                                 \
    }
    
    // Lock scope, can't handle request when re register module 
    //clsSmartRLock loLock( coRWHandleRequestLock );
    
    switch ( aiCmd )
    {
        //SERVER_CASE( AI_GFQ_REQUEST_HEARTBEAT, clsRequestHeartBeat, AI_GFQ_RESPONSE_HEARTBEAT, clsResponseHeartBeat )
        SERVER_CASE( AI_GFQ_REQUEST_PUT, clsRequestPut, AI_GFQ_RESPONSE_PUT, clsResponsePut )
        SERVER_CASE( AI_GFQ_REQUEST_GET, clsRequestGet, AI_GFQ_RESPONSE_GET, clsResponseGet )
        SERVER_CASE( AI_GFQ_REQUEST_ALLOC, clsRequestAlloc, AI_GFQ_RESPONSE_ALLOC, clsResponseAlloc )
        SERVER_CASE( AI_GFQ_REQUEST_FREE, clsRequestFree, AI_GFQ_RESPONSE_FREE, clsResponseFree )
        SERVER_CASE( AI_GFQ_REQUEST_SEGMENTSIZE, clsRequestSegmentSize, AI_GFQ_RESPONSE_SEGMENTSIZE, clsResponseSegmentSize )
    }
    
    // Process exception
    AIChunkEx   loHexBuff;
    clsResponse loError;
    loError.SetStat( AI_ERROR_INVALID_REQUST );
    UStringToHex( aoChunk.BasePtr(), aoChunk.GetSize(), loHexBuff );
    AI_GFQ_ERROR( "Segment Server command invalid request - [CMD=0x%X]/[Package=\n%s]/[Size=%d]", aiCmd, loHexBuff.BasePtr(), aoChunk.GetSize() );
    this->SendResponse( apoMatch, AI_GFQ_RESPONSE_ERROR, &loError );
}

//request operator
int clsSegmentServer::DoRequest( clsRequestAlloc& aoRequest, clsResponseAlloc& aoResponse )
{
    int liRet = AI_NO_ERROR;
    aoResponse.SetStat(AI_NO_ERROR);
    
    {//Lock scope        
        AI_GFQ_TIMETRACE( 
            (liRet = cpoSegmentStore->Alloc( 
                aoRequest.GetSegmentID(), aoRequest.GetQueueGlobalID(), aoRequest.GetQueueName() ) ), 
            0,/*level*/
            "DoRequest::Alloc(%"PRId32", %"PRId64", %s)", 
            (int32_t)aoRequest.GetSegmentID(),
            (int64_t)aoRequest.GetQueueGlobalID(),
            aoRequest.GetQueueName() );
    }
    
    if ( liRet != AI_NO_ERROR )
    {
        aoResponse.SetStat(liRet);
        
        AI_GFQ_LOG( liRet, "Segment server alloc segment fail - MSGCODE:%d, MSG:%s, [Segment ID=%d]/[Queue GlobalID=%lld]/[Queue Name=%s]",
            liRet, StrError( liRet ), aoRequest.GetSegmentID(), aoRequest.GetQueueGlobalID(), aoRequest.GetQueueName() );
    }
    else
    {
        AI_GFQ_DEBUG( "Segment server alloc segment success, [Segment ID=%d]/[Queue GlobalID=%lld]/[Queue Name=%s]",
            aoRequest.GetSegmentID(), aoRequest.GetQueueGlobalID(), aoRequest.GetQueueName() );
    }
    
    return liRet;
}

int clsSegmentServer::DoRequest( clsRequestFree& aoRequest, clsResponseFree& aoResponse )
{
    int liRet = AI_NO_ERROR;
    aoResponse.SetStat(AI_NO_ERROR);
    
    {//Lock scope
        AI_GFQ_TIMETRACE( 
            (liRet = cpoSegmentStore->Free( aoRequest.GetSegmentID() ) ),
            0,/*level*/
            "DoRequest::Free(%"PRId32")", 
            (int32_t)aoRequest.GetSegmentID() );
    }
    
    if ( liRet != AI_NO_ERROR )
    {
        aoResponse.SetStat(liRet);
        
        AI_GFQ_LOG( liRet, "Segment server free segment fail - MSGCODE:%d, MSG:%s, [Segment ID=%d]",
            liRet, StrError( liRet ), aoRequest.GetSegmentID() );
    }
    else
    {
        AI_GFQ_DEBUG( "Segment server free segment success, [Segment ID=%d]", aoRequest.GetSegmentID() );
    }
    
    return liRet;
}
    
int clsSegmentServer::DoRequest( clsRequestPut& aoRequest, clsResponsePut& aoResponse )
{
    int liRet = AI_NO_ERROR;
    aoResponse.SetStat(AI_NO_ERROR);

    AIChunkEx loData(AI_MAX_DATA_LEN);
    aoRequest.GetData( loData );
    
    {//Lock scope        
        AI_GFQ_TIMETRACE(
            (liRet = cpoSegmentStore->Put( 
                aoRequest.GetSegmentID(), aoRequest.GetQueueGlobalID(), aoRequest.GetQueueName(), loData ) ), 
            0,/*level*/
            "DoRequest::Put(%"PRId32", %"PRId64", %s, %"PRId32")", 
            (int32_t)aoRequest.GetSegmentID(),
            (int64_t)aoRequest.GetQueueGlobalID(),
            aoRequest.GetQueueName(),
            (int32_t)loData.GetSize() );
    }

    if ( liRet != AI_NO_ERROR )
    {
        aoResponse.SetStat(liRet);
        
        AI_GFQ_LOG( liRet, "Segment server put data fail - MSGCODE:%d, MSG:%s, [User=%s]/[Segment ID=%d]/[Queue GlobalID=%lld]/[Queue Name=%s]",
            liRet, StrError( liRet ), aoRequest.GetUserName(), aoRequest.GetSegmentID(), aoRequest.GetQueueGlobalID(), aoRequest.GetQueueName() );
    }
    else
    {
        AI_GFQ_DEBUG( "Segment server put data success, [User=%s]/[Segment ID=%d]/[Queue GlobalID=%lld]/[Queue Name=%s]",
            aoRequest.GetUserName(), aoRequest.GetSegmentID(), aoRequest.GetQueueGlobalID(), aoRequest.GetQueueName() );
    }
    
    return liRet;
}

int clsSegmentServer::DoRequest( clsRequestGet& aoRequest, clsResponseGet& aoResponse )
{
    int liRet = AI_NO_ERROR;
    aoResponse.SetStat(AI_NO_ERROR);
    
    AIChunkEx loData(AI_MAX_DATA_LEN);
    
    {//Lock scope        
        AI_GFQ_TIMETRACE( 
            (liRet = cpoSegmentStore->Get( 
                aoRequest.GetSegmentID(), aoRequest.GetQueueGlobalID(), aoRequest.GetQueueName(), aoRequest.GetWhenSec(), loData ) ), 
            0,/*level*/
            "DoRequest::Get(%"PRId32", %"PRId64", %s, %"PRId32")", 
            (int32_t)aoRequest.GetSegmentID(),
            (int64_t)aoRequest.GetQueueGlobalID(),
            aoRequest.GetQueueName(),
            (int32_t)aoRequest.GetWhenSec() );
    }
    
    if ( liRet != AI_NO_ERROR )
    {
        aoResponse.SetStat(liRet);
        
        AI_GFQ_LOG( liRet, "Segment server get data fail - MSGCODE:%d, MSG:%s, [User=%s]/[Segment ID=%d]/[Queue GlobalID=%lld]/[Queue Name=%s]",
            liRet, StrError( liRet ), aoRequest.GetUserName(), aoRequest.GetSegmentID(), aoRequest.GetQueueGlobalID(), aoRequest.GetQueueName() );
    }
    else
    {
        aoResponse.SetData( loData );
        
        AI_GFQ_DEBUG( "Segment server get data success, [User=%s]/[Segment ID=%d]/[Queue GlobalID=%lld]/[Queue Name=%s]",
            aoRequest.GetUserName(), aoRequest.GetSegmentID(), aoRequest.GetQueueGlobalID(), aoRequest.GetQueueName() );
    }
    
    return liRet;
}

int clsSegmentServer::DoRequest( clsRequestSegmentSize& aoRequest, clsResponseSegmentSize& aoResponse )
{
    aoResponse.SetStat(AI_NO_ERROR);
    
    CSegmentInfo loSegmentInfo;
    
    if (cpoSegmentStore->GetSegmentInfo( aoRequest.GetSegmentID(), loSegmentInfo) != AI_NO_ERROR)
    {
        aoResponse.SetStat(AI_ERROR_OUT_OF_RANGE);
        
        AI_GFQ_ERROR( "Segment server get segment size fail - MSGCODE:%d, MSG:%s, [Segment ID=%d]",
            AI_ERROR_OUT_OF_RANGE, StrError( AI_ERROR_OUT_OF_RANGE ), aoRequest.GetSegmentID() );
    }
    else
    {
        AI_GFQ_DEBUG( "Segment server get segment size success, [Segment ID=%d]/[Segment size=%d]", 
            aoRequest.GetSegmentID(), loSegmentInfo.GetSize() );
    }
    
    aoResponse.SetSegmentSize( loSegmentInfo.GetSize() );
    
    return AI_NO_ERROR;
}

//send response operator
void clsSegmentServer::SendResponse( const ASYNCMATCH *apoMatch, int aiCmd, clsBody* apoBody )
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

void clsSegmentServer::PrintStat()
{
    AI_GFQ_INFO( "-----------------------------------------------------------------------\n" );
    AI_GFQ_INFO( " Process Request count      = %dps", coRequestStat.Frequency() );
    coRequestStat.Reset();
    
    //for thread pool stat
	AI_GFQ_INFO( " ThreadPool Actives = %d / Max = %d / CacheSize = %d / CacheCapacity = %d", 
	    cpoThreadPool->GetActiveThreadCount(), ciThreadCount, cpoThreadPool->GetCacheSize(), cpoThreadPool->GetCacheCapacity() );
	AI_GFQ_INFO( "-----------------------------------------------------------------------\n" );
}

AI_GFQ_NAMESPACE_END
