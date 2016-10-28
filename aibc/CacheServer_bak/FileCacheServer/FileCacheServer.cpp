
#include "FileCacheServer.h"
#include "AIProcBase.h"

AI_CACHE_NAMESPACE_START

// thread param sub struct constructor
clsFileCacheServer::stThreadParam::stThreadParam( 
	clsFileCacheServer* apoServer, const AIBC::ASYNCMATCH* apoMatch, char* apsData, size_t aiSize ) :
	cpoServer(apoServer),
	coData( apsData, aiSize )
{
	memcpy( &coMatch, apoMatch, sizeof(AIBC::ASYNCMATCH) );
}

clsFileCacheServer* clsFileCacheServer::Instance()
{
	static clsFileCacheServer* slpoServer = NULL;
	if ( slpoServer == NULL )
	{
		AI_NEW_ASSERT( slpoServer, clsFileCacheServer ); 
	}
	return slpoServer;
}

clsFileCacheServer::clsFileCacheServer()
    : clsAsyncServer( "", 0 )
    , cpoThreadPool(NULL)
    , ciShutDownTimeout(0)
    , ciListenPort(0)
    , ciPrintStatInterval(0)
    , ciTimeoutInterval(0)
    , ciClearTimeoutInterval(0)
{
}

clsFileCacheServer::~clsFileCacheServer()
{
    AI_DELETE( this->cpoThreadPool )
}

int clsFileCacheServer::Startup( const char *apcConfigFile )
{
    AIBC::AIConfig* lpoIni = NULL;
    int   liRetCode  = AI_NO_ERROR;
    int   liThreadCount     = 0;
    int   liThreadPoolCache = 0;
    int   liTimePoint       = 0;
    char  lcCacheDir[128]   = {0};

	if ( (lpoIni = AIBC::AIGetIniHandler(apcConfigFile)) == NULL )
    {
        AI_CACHE_ERROR( "Load config file %s fail", apcConfigFile );
        return 1;
    }
    
    AI_CACHE_INFO( "%s", ">>>>>>>>>>>>>>>>>>>>>>>>>>Server<<<<<<<<<<<<<<<<<<<<<<<<<<<" );

    //load config variable
    liThreadCount          = lpoIni->GetIniInt( "CacheServer", "ThreadPool", 500 );
    liThreadPoolCache      = lpoIni->GetIniInt( "CacheServer", "ThreadPoolCache", 5000 );
    ciShutDownTimeout      = lpoIni->GetIniInt( "CacheServer", "ShutDownTimeout", 0 );
    ciTimeoutInterval      = lpoIni->GetIniInt( "CacheServer", "DataTimeout", 86400 );
    ciClearTimeoutInterval = lpoIni->GetIniInt( "CacheServer", "ClearTimeoutInterval", 0 );
    ciPrintStatInterval    = lpoIni->GetIniInt( "CacheServer", "PrintStatInterval", 0 );
    liTimePoint            = lpoIni->GetIniInt( "CacheServer", "TimePoint", 3600 );
    lpoIni->GetIniString( "CacheServer", "DataDir", lcCacheDir, sizeof(lcCacheDir) );

    this->coFileCache.SetTimePoint( liTimePoint );
    if ( ( liRetCode = this->coFileCache.Open( lcCacheDir ) ) != AI_NO_ERROR )
    {
        AI_CACHE_ERROR( "Open Cache %s fail, MSGCODE:%d, MSG:%s", lcCacheDir, liRetCode, StrError(liRetCode) );
        return 1;
    }
    
    //Module info
	int  liMaxChildCount   = 0;
	int  liAdminListenPort = 0;
	int  liServerPort = -1;
	char lcModuleID[AI_MAX_NAME_LEN];
	char lcAdminListenIpAddr[AI_IP_ADDR_LEN];
    
    //module config info
    liMaxChildCount        = lpoIni->GetIniInt( "SERVER", "MaxChildCount", -1 );
    liAdminListenPort      = lpoIni->GetIniInt( "SERVER", "AdminListenPort", -1 );
    liServerPort           = lpoIni->GetIniInt( "SERVER", "CacheServerPort", -1 );
    lpoIni->GetIniString( "SERVER", "ModuleID", lcModuleID, sizeof(lcModuleID) );
    lpoIni->GetIniString( "SERVER", "AdminListenIpAddr", lcAdminListenIpAddr, sizeof(lcAdminListenIpAddr) );
    this->ciListenPort = liServerPort;
    
    AI_NEW_ASSERT( this->cpoThreadPool, AITask );
    if ( this->cpoThreadPool->Initialize( liThreadCount, liThreadPoolCache ) != 0 )
    {
        AI_CACHE_ERROR( "Initialize Task fail, MSG:May be thread pool create fail" );
        return 1;
    }
    
    if ( this->ciClearTimeoutInterval > 0 )
    {
        pthread_t liThrd;
        if ( pthread_create( &liThrd, NULL, AutoDelTimeoutEntry, this ) != 0 )
        {
            AI_CACHE_ERROR( "Start auto delete timeout record thread fail, MSG:%s", strerror(errno) );
            return -1;
        }
    }
    
    //module configure
	{
	    AI_CACHE_INFO( "%s", "----------------Module Config Variable-----------------" );
	    AI_CACHE_INFO( "* ModuleID                = %s", lcModuleID);
        AI_CACHE_INFO( "* MaxChildCount           = %d", liMaxChildCount);
        AI_CACHE_INFO( "* AdminListenPort         = %d", liAdminListenPort);
        AI_CACHE_INFO( "* AdminListenIpAddr       = %s", lcAdminListenIpAddr);
        AI_CACHE_INFO( "* CacheServerPort         = %d", liServerPort);
	}

	//complete configure
	{
	    AI_CACHE_INFO( "%s", "-----------------Server Config Variable----------------" );
	    AI_CACHE_INFO( "* Listen Port             = %d", ciListenPort);
        AI_CACHE_INFO( "* Thread Pool             = %d", liThreadCount);
        AI_CACHE_INFO( "* Thread Pool Cache       = %d", liThreadPoolCache);
        AI_CACHE_INFO( "* Print Interval          = %d", ciPrintStatInterval);
        AI_CACHE_INFO( "* Data timeout            = %d", ciTimeoutInterval);
	}
	
	{
	    AI_CACHE_INFO( "%s", "--------------------Cache Info------------------------" );
        AI_CACHE_INFO( "* File Count              = %d", this->coFileCache.GetFileCount() );
        AI_CACHE_INFO( "* Single File Capacity    = %d", this->coFileCache.GetCapacity() / this->coFileCache.GetFileCount() );
        AI_CACHE_INFO( "* Capacity                = %d", this->coFileCache.GetCapacity() );
        AI_CACHE_INFO( "* Size                    = %d", this->coFileCache.GetSize() );
        AI_CACHE_INFO( "* Max Key size            = %d", this->coFileCache.GetKeySize() );
        AI_CACHE_INFO( "* Max Value size          = %d", this->coFileCache.GetValueSize() );
	}
	
	if ( ( liRetCode = this->StartDaemon( "CacheServerPort", AIBC::GetNewConnection, AIBC::GetListenSocket ) ) != 0 )
	{
	    if ( liRetCode == ASYNC_ERROR_LISTENFAIL )
	    {
            AI_CACHE_ERROR( "Server startup fail, ErrMsg=AsyncServer Listen port fail, SysMsg:%s", strerror(errno) );
        }
        else
        {
            AI_CACHE_ERROR( "Server startup fail, ErrMsg=System error, SysMsg:%s", strerror(errno) );
	    }
	    
		return -1;
	}
	
	AI_CACHE_INFO( "%s", "Server startup success" );
	
	return 0;
}

void clsFileCacheServer::Shutdown()
{
    if ( this->cpoThreadPool != NULL )
    {
        this->cpoThreadPool->Close();
    }
    clsAsyncServer::ShutDown();
        
    AI_CACHE_INFO( "%s", "Server shutdown..." );
}

//thread operator
void clsFileCacheServer::Callback( const AIBC::ASYNCMATCH *apoMatch, char *apcData, const int aiSize )
{
    stThreadParam* lpoParam = NULL;
    AI_NEW_ASSERT( lpoParam, stThreadParam( this, apoMatch, apcData, aiSize ) );
    
    // Trace log
    AIBC::AIChunkEx loHexBuff(0);
    AI_CACHE_TRACE( "Receive request, [MsgID=%d]/[ThreadID=%d]/[Length=%d]/[Package=\n%s]", 
        apoMatch->ciMsgID, apoMatch->ctThreadID, aiSize, 
        UStringToHex( lpoParam->coData.BasePtr(), lpoParam->coData.GetSize(), loHexBuff ) );

    if ( this->cpoThreadPool->Put( HandleRequestEntry, lpoParam ) < 0 )
	{
		AI_DELETE( lpoParam );
		
		AI_CACHE_ERROR( "Attach thread fail MSGCODE:%d, MSG:%s", 
		    AI_ERROR_THREAD_POOL_FULL, StrError( AI_ERROR_THREAD_POOL_FULL ) );
		
		//for error report
		clsResponse loResponse;
		loResponse.SetStat(AI_ERROR_THREAD_POOL_FULL);
        this->SendResponse( apoMatch, AI_CACHESERVER_RESPONSE_ERROR, &loResponse );
	}
}

void* clsFileCacheServer::HandleRequestEntry( void *apvParam )
{
	stThreadParam* lpoParam = (stThreadParam*)apvParam;

    // Trace log
    AI_CACHE_TRACE( "Handle request now, [MsgID=%d]/[ThreadID=%d]", lpoParam->coMatch.ciMsgID, lpoParam->coMatch.ctThreadID );
    
	lpoParam->cpoServer->HandleRequestThread( &lpoParam->coMatch, lpoParam->coData );
	
	AI_DELETE( lpoParam );
	
	return NULL;
}

void* clsFileCacheServer::AutoDelTimeoutEntry( void* apvParam )
{
    clsFileCacheServer* lpoCache = static_cast<clsFileCacheServer*>(apvParam);
    clsRequestDelTimeOut  loRequest;
    clsResponseDelTimeOut loResponse;
    
    while( !AIM::IsShutdown() 
        && lpoCache->ciClearTimeoutInterval > 0 )
    {
        lpoCache->DoRequest( loRequest, loResponse );
        //Ignore
        if ( loResponse.GetStat() != AI_NO_ERROR )
        {
            AIBC::AISleepFor( AI_TIME_SEC * lpoCache->ciClearTimeoutInterval );
        }
    }
    
    return NULL;
}

void clsFileCacheServer::HandleRequestThread( AIBC::ASYNCMATCH *apoMatch, AIBC::AIChunkEx& aoChunk )
{
	clsHeader loHeader;
	loHeader.Decode( aoChunk );
    
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
	    
	switch ( loHeader.GetCmd() )
	{
		SERVER_CASE( AI_CACHESERVER_REQUEST_PUT, clsRequestPut, AI_CACHESERVER_RESPONSE_PUT, clsResponsePut )
	    SERVER_CASE( AI_CACHESERVER_REQUEST_GET, clsRequestGet, AI_CACHESERVER_RESPONSE_GET, clsResponseGet )
	    SERVER_CASE( AI_CACHESERVER_REQUEST_PUT_OW, clsRequestPutOW, AI_CACHESERVER_RESPONSE_PUT_OW, clsResponsePutOW )
	    SERVER_CASE( AI_CACHESERVER_REQUEST_GET_ND, clsRequestGetND, AI_CACHESERVER_RESPONSE_GET_ND, clsResponseGetND )
	    SERVER_CASE( AI_CACHESERVER_REQUEST_UPDATE, clsRequestUpdate, AI_CACHESERVER_RESPONSE_UPDATE, clsResponseUpdate )
	    SERVER_CASE( AI_CACHESERVER_REQUEST_DELTIMEOUT, clsRequestDelTimeOut, AI_CACHESERVER_RESPONSE_DELTIMEOUT, clsResponseDelTimeOut )
	    SERVER_CASE( AI_CACHESERVER_REQUEST_STAT, clsRequestStat, AI_CACHESERVER_RESPONSE_STAT, clsResponseStat )
	}
	
	// Process exception
	AIBC::AIChunkEx   loHexBuff(0); 
	clsResponse loErrorResponse;
    loErrorResponse.SetStat( AI_ERROR_INVALID_REQUST );
    AI_CACHE_ERROR( "Server invalid request - [CMD=0x%X]/[Size=%d]/[Package=\n%s]",
        loHeader.GetCmd(), aoChunk.GetSize(),
        UStringToHex( aoChunk.BasePtr(), aoChunk.GetSize(), loHexBuff ) );
    this->SendResponse( apoMatch, AI_CACHESERVER_RESPONSE_ERROR, &loErrorResponse );
}

//request operator
int clsFileCacheServer::DoRequest( clsRequestPut& aoRequest, clsResponsePut& aoResponse )
{    
    int liRetCode = AI_NO_ERROR;
    
    if ( ( liRetCode = this->coFileCache.Put( aoRequest.GetKey(), aoRequest.GetValue() ) ) != AI_NO_ERROR )
    {
        AI_CACHE_ERROR( "Cache put record fail, MSGCODE:%d, MSG:%s, [key=%s]", 
            liRetCode, StrError(liRetCode), aoRequest.GetKey().BasePtr() );
    }
    else
    {
        AI_CACHE_DEBUG( "Cache put record success, [key=%s]", aoRequest.GetKey().BasePtr() );
    }
    
    aoResponse.SetStat( liRetCode );
    
    return 0;
}

int clsFileCacheServer::DoRequest( clsRequestGet& aoRequest, clsResponseGet& aoResponse )
{
    int liRetCode = AI_NO_ERROR;
    
    if ( ( liRetCode = this->coFileCache.Get( aoRequest.GetKey(), aoResponse.GetValue() ) ) != AI_NO_ERROR )
    {
        AI_CACHE_LOG( liRetCode, "Cache get record fail, MSGCODE:%d, MSG:%s, [key=%s]", 
            liRetCode, StrError(liRetCode), aoRequest.GetKey().BasePtr() );
    }
    else
    {
        AI_CACHE_DEBUG( "Cache get record success, [key=%s]", aoRequest.GetKey().BasePtr() );
    }
    
    aoResponse.SetStat( liRetCode );

    return 0;
}

int clsFileCacheServer::DoRequest( clsRequestPutOW& aoRequest, clsResponsePutOW& aoResponse )
{    
    int liRetCode = AI_NO_ERROR;
    
    if ( ( liRetCode = this->coFileCache.Put( aoRequest.GetKey(), aoRequest.GetValue(), OVERWRITE ) ) != AI_NO_ERROR )
    {
        AI_CACHE_LOG( liRetCode, "Cache put record fail, MSGCODE:%d, MSG:%s, [key=%s]/[OVERWRITE]", 
            liRetCode, StrError(liRetCode), aoRequest.GetKey().BasePtr() );
    }
    else
    {
        AI_CACHE_DEBUG( "Cache put record success, [key=%s]/[OVERWRITE]", aoRequest.GetKey().BasePtr() );
    }
    
    aoResponse.SetStat( liRetCode );
    
    return 0;
}

int clsFileCacheServer::DoRequest( clsRequestGetND& aoRequest, clsResponseGetND& aoResponse )
{
    int liRetCode = AI_NO_ERROR;
    
    if ( ( liRetCode = this->coFileCache.Get( aoRequest.GetKey(), aoResponse.GetValue(), NO_DELETE ) ) != AI_NO_ERROR )
    {
        AI_CACHE_LOG( liRetCode, "Cache get record fail, MSGCODE:%d, MSG:%s, [key=%s]/[NO_DELETE]", 
            liRetCode, StrError(liRetCode), aoRequest.GetKey().BasePtr() );
    }
    else
    {
        AI_CACHE_DEBUG( "Cache get record success, [key=%s]/[NO_DELETE]", aoRequest.GetKey().BasePtr() );
    }
    
    aoResponse.SetStat( liRetCode );

    return 0;
}

int clsFileCacheServer::DoRequest( clsRequestUpdate& aoRequest, clsResponseUpdate& aoResponse )
{
    int liRetCode = AI_NO_ERROR;
    
    if ( ( liRetCode = this->coFileCache.Update( aoRequest.GetKey(), aoRequest.GetValue() ) ) != AI_NO_ERROR )
    {
        AI_CACHE_LOG( liRetCode, "Cache update record fail, MSGCODE:%d, MSG:%s, [key=%s]", 
            liRetCode, StrError(liRetCode), aoRequest.GetKey().BasePtr() );
    }
    else
    {
        AI_CACHE_DEBUG( "Cache update record success, [key=%s]", aoRequest.GetKey().BasePtr() );
    }
    
    aoResponse.SetStat( liRetCode );

    return 0;
}

int clsFileCacheServer::DoRequest( clsRequestDelTimeOut& aoRequest, clsResponseDelTimeOut& aoResponse )
{
    int liRetCode = AI_NO_ERROR;

    if ( ( liRetCode = this->coFileCache.DelTimeOut( 
        time(NULL) - ciTimeoutInterval, aoResponse.GetKey(), aoResponse.GetValue() ) ) != AI_NO_ERROR )
    {
        AI_CACHE_LOG( liRetCode, "Cache delete time out record fail, MSGCODE:%d, MSG:%s", liRetCode, StrError( liRetCode ));
    }
    else
    {
        AI_CACHE_DEBUG( "Cache delete time out record success, [key=%s]", aoResponse.GetKey().BasePtr() );
    }
    
    aoResponse.SetStat( liRetCode );

    return 0;
}

int clsFileCacheServer::DoRequest( clsRequestStat& aoRequest, clsResponseStat& aoResponse )
{    
    aoResponse.SetFileCount( this->coFileCache.GetFileCount() );
    aoResponse.SetCacheCapacity( this->coFileCache.GetCapacity() );
    aoResponse.SetCacheSize( this->coFileCache.GetSize() );
    aoResponse.SetStat( AI_NO_ERROR );
    
    AI_CACHE_DEBUG( "%s", "Cache get status success" );

    return 0;
}
void clsFileCacheServer::PrintStat()
{
    static int sliTimestamp = time(NULL);
    
    if ( sliTimestamp + this->ciPrintStatInterval <= time(NULL) )
    {
        AI_CACHE_INFO( "%s", "-----------------------------------------------------------------------\n" );
        AI_CACHE_INFO( " Get record number = %d", this->coFileCache.GetSize() );
        AI_CACHE_INFO( " Current allocate point = %d", this->coFileCache.GetCurrAllocPos() );
    	AI_CACHE_INFO( " ThreadPool Actives = %d / Max = %d", 
    	    this->cpoThreadPool->GetActiveThreadCount(), this->cpoThreadPool->GetMaxThreadCount() );
    	AI_CACHE_INFO( "%s", "-----------------------------------------------------------------------\n" );
        
        sliTimestamp = time(NULL);
    }
}

//send response operator
void clsFileCacheServer::SendResponse( const AIBC::ASYNCMATCH *apoMatch, int aiCmd, clsBody* apoBody )
{
    int liRetCode = AI_NO_ERROR;
	clsHeader loHeader( aiCmd, apoBody );
    AIBC::AIChunkEx loChunk( loHeader.GetSize() );
    loHeader.Encode( loChunk );

	if ( ( liRetCode = this->SendResult( apoMatch, loChunk.BasePtr(), loChunk.GetSize(), 0 ) ) != 0 )
	{
	    AI_CACHE_ERROR( "Send response fail - ASYNCCODE:%d, MSG:%s, [Response Cmd=0x%x]\n", liRetCode, strerror(errno), aiCmd );
	}
	// Trace log
    AI_CACHE_TRACE( "Responsed request, [MsgID=%d]/[ThreadID=%d]", apoMatch->ciMsgID, apoMatch->ctThreadID );
}

AI_CACHE_NAMESPACE_END

AIBC_NAMESPACE_START
DECLARE_CMDOPT_BEGIN()
DECLARE_CMDOPT_END()
AIBC_NAMESPACE_END

DEFINE_CACHE_MAIN( __Main, clsFileCacheServer )
HANDLE_MAIN_MACRO( __Main, "|CacheServerPort|" )
