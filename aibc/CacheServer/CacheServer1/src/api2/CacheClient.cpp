
#include <sys/timeb.h>

#include "cacheserver/CacheClient.h"
#include "../common/Protocol.h"

using namespace cacheserver;

// process request error
#define AI_DECODE_RESPONSE( response, header, body, ret )           \
	header.Decode(response);                                        \
	if ( header.GetCmd() == AI_CACHESERVER_RESPONSE_ERROR )         \
	{                                                               \
		clsResponse loResponseBody;                                 \
		loResponseBody.Decode( response );                          \
		                                                            \
		ret = loResponseBody.GetStat();                             \
	}                                                               \
	else                                                            \
	{                                                               \
		body.Decode( response );                                    \
		ret = body.GetStat();                                       \
	}

// do request to server
template< class TAsyncClient, class TRequest, class TResponse >
int DoRequestTo( TAsyncClient& aoAsyncClient, int aiRequestCmd, TRequest& aoRequest, TResponse& aoResponse  )
{
	clsHeader loHeader( aiRequestCmd, &aoRequest );
	clsHeader loRespHeader;
	AIBC::AIChunkEx loRequestChunk;
	AIBC::AIChunkEx loResponseChunk(64 + AI_MAX_KEY_LEN + AI_MAX_VALUE_LEN);
	int liRepSize = 0;
	int liRet = AI_NO_ERROR;
	
	loHeader.Encode(loRequestChunk);
	
	if ( ( liRet = aoAsyncClient.SendRequest( loRequestChunk.BasePtr(), loRequestChunk.GetSize(), NULL ) ) != 0 )
	{
	    AIBC::AIWriteLOG( NULL, AILOG_LEVEL_ERROR, "[Do Request]Send Request fail - ASYNCCODE:%d, MSG:%s", liRet, strerror(errno) );
		return AI_ERROR_SOCKET_SEND;
	}
    
	if ( ( liRet = aoAsyncClient.RecvResponse( loResponseChunk.BasePtr(), loResponseChunk.GetSize(), liRepSize ) ) != 0 )
	{
	    AIBC::AIWriteLOG( NULL, AILOG_LEVEL_ERROR, "[Do Request]Recv Response fail - ASYNCCODE:%d, MSG:%s", liRet, strerror(errno) );
		return AI_ERROR_SOCKET_RECV;
	}
	
	loResponseChunk.Reset();
	loResponseChunk.WritePtr(liRepSize);

	AI_DECODE_RESPONSE( loResponseChunk, loRespHeader, aoResponse, liRet );
	
	return liRet;
}

stCacheServerInfo::stCacheServerInfo()
    : ciPort(-1)
	, ciConnection(0)
	, ciMinRange(0)
	, ciMaxRange(0)
	, cpoAsyncClient(NULL)
{
    ::memset( this->csIpAddr, 0, sizeof(this->csIpAddr) );
}

stCacheServerInfo::~stCacheServerInfo()
{
    AI_DELETE( this->cpoAsyncClient );
}

clsCacheClient::clsCacheClient()
    : ciIsWriteLog(0)
    , ciServerCnt(0)
    , ciServerIdx(0)
    , cpoCacheServer(NULL)
{
	::memset( this->cpoCacheServerInfos, 0, sizeof(this->cpoCacheServerInfos) );
}

clsCacheClient::~clsCacheClient()
{
    this->Close();
	AI_DELETE_N( this->cpoCacheServer );
}

int clsCacheClient::Initialize( const char* apcConfigFile )
{
	char      lsField[20];
	char      lsBuf[20];
	AIBC::AIConfig* lpIni       = NULL;
	int       liHashValue = 0;
	
	if ((lpIni = AIBC::AIGetIniHandler(apcConfigFile)) == NULL)
	{
		AI_CACHE_ERROR( "Load cache config file %s fail", apcConfigFile);
		return -1;
	}

	this->ciIsWriteLog = lpIni->GetIniInt("Log", "OpenApiLog", 0);
	
	//check server count
	for ( int i=0; true; i++ )
	{
		::sprintf( lsField, "Server%d", i + 1 );
		if ( lpIni->GetIniString( lsField, "ServerIP", lsBuf, sizeof(lsBuf) ) == 0 )
		{
			break;
		}
		this->ciServerCnt++;
	}
	
	AI_NEW_N_ASSERT( this->cpoCacheServer, stCacheServerInfo, this->ciServerCnt );
	for ( int i = 0; i< this->ciServerCnt; i++ )
	{
		sprintf( lsField, "Server%d", i+1 );
		lpIni->GetIniString(lsField, "ServerIP", cpoCacheServer[i].csIpAddr, sizeof(cpoCacheServer[i].csIpAddr));
		this->cpoCacheServer[i].ciPort = lpIni->GetIniInt(lsField, "ServerPort", -1);
		this->cpoCacheServer[i].ciConnection = lpIni->GetIniInt(lsField, "Connection", 1);
		this->cpoCacheServer[i].ciMinRange = lpIni->GetIniInt(lsField, "MinRange", 0);
		this->cpoCacheServer[i].ciMaxRange = lpIni->GetIniInt(lsField, "MaxRange", 99);
		this->cpoCacheServer[i].ciTimeout  = lpIni->GetIniInt(lsField, "ConnTimeout", 5);
		
		if ( this->SetHashRangeValue( 
		    this->cpoCacheServer[i].ciMinRange,
		    this->cpoCacheServer[i].ciMaxRange,
		    &this->cpoCacheServer[i], &liHashValue ) != 0 )
		{
		    AI_CACHE_ERROR( "Server hash range value `%d` conflict, [%s],[Lower=%d/Upper=%d]", 
		        liHashValue, lsField, cpoCacheServer[i].ciMinRange, cpoCacheServer[i].ciMaxRange );
		    return -1;
		}
		
		AI_NEW_ASSERT( this->cpoCacheServer[i].cpoAsyncClient,
		    AIBC::clsAsyncClient( 
		        this->cpoCacheServer[i].csIpAddr, 
		        this->cpoCacheServer[i].ciPort, 
		        this->cpoCacheServer[i].ciConnection,
		        this->cpoCacheServer[i].ciTimeout ) );
        
		this->cpoCacheServer[i].cpoAsyncClient->StartDaemon(3);
	}
	
	if ( this->CheckHashRangeValue( &liHashValue ) != 0 )
	{
		AI_CACHE_ERROR( "Server hash range value `%d` undefine", liHashValue );
		return -1;
	}

	for ( int i = 0; i < this->ciServerCnt; i++ )
	{
		AI_CACHE_INFO( "============================= Server%d =========================", i + 1 );
		AI_CACHE_INFO( "   IpAddr        = %s", this->cpoCacheServer[i].csIpAddr );
		AI_CACHE_INFO( "   Port          = %d", this->cpoCacheServer[i].ciPort );
		AI_CACHE_INFO( "   Conn count    = %d", this->cpoCacheServer[i].ciConnection );
		AI_CACHE_INFO( "   Conn timeout  = %d", this->cpoCacheServer[i].ciTimeout );
		AI_CACHE_INFO( "   Hash Lower    = %d", this->cpoCacheServer[i].ciMinRange );
		AI_CACHE_INFO( "   Hash Upper    = %d", this->cpoCacheServer[i].ciMaxRange );
	}
	
	return 0;
}

int clsCacheClient::Initialize( stCacheServerInfo* apoSrvList, int aiCount )
{
    int       liHashValue = 0;

    AI_NEW_N_ASSERT( this->cpoCacheServer, stCacheServerInfo, aiCount );
    for ( int i = 0; i< aiCount; i++ )
    {
        strncpy(cpoCacheServer[i].csIpAddr, apoSrvList[i].csIpAddr, sizeof(cpoCacheServer[i].csIpAddr));
        this->cpoCacheServer[i].ciPort = apoSrvList[i].ciPort;
        this->cpoCacheServer[i].ciConnection = apoSrvList[i].ciConnection;
        this->cpoCacheServer[i].ciMinRange = apoSrvList[i].ciMinRange;
        this->cpoCacheServer[i].ciMaxRange = apoSrvList[i].ciMaxRange;
        this->cpoCacheServer[i].ciTimeout  = apoSrvList[i].ciTimeout;

        if ( this->SetHashRangeValue( 
            this->cpoCacheServer[i].ciMinRange,
            this->cpoCacheServer[i].ciMaxRange,
            &this->cpoCacheServer[i], &liHashValue ) != 0 )
        {
/*
            AI_CACHE_ERROR( "Server hash range value `%d` conflict, [%s],[Lower=%d/Upper=%d]", 
                liHashValue, lsField, cpoCacheServer[i].ciMinRange, cpoCacheServer[i].ciMaxRange );
            return -1;
*/
        }

        AI_NEW_ASSERT( this->cpoCacheServer[i].cpoAsyncClient,
            AIBC::clsAsyncClient( 
            this->cpoCacheServer[i].csIpAddr, 
            this->cpoCacheServer[i].ciPort, 
            this->cpoCacheServer[i].ciConnection,
            this->cpoCacheServer[i].ciTimeout ) );

        this->cpoCacheServer[i].cpoAsyncClient->StartDaemon(3);
    }

/*
    if ( this->CheckHashRangeValue( &liHashValue ) != 0 )
    {
        AI_CACHE_ERROR( "Server hash range value `%d` undefine", liHashValue );
        return -1;
    }
*/

    for ( int i = 0; i < this->ciServerCnt; i++ )
    {
        AI_CACHE_INFO( "============================= Server%d =========================", i + 1 );
        AI_CACHE_INFO( "   IpAddr        = %s", this->cpoCacheServer[i].csIpAddr );
        AI_CACHE_INFO( "   Port          = %d", this->cpoCacheServer[i].ciPort );
        AI_CACHE_INFO( "   Conn count    = %d", this->cpoCacheServer[i].ciConnection );
        AI_CACHE_INFO( "   Conn timeout  = %d", this->cpoCacheServer[i].ciTimeout );
        AI_CACHE_INFO( "   Hash Lower    = %d", this->cpoCacheServer[i].ciMinRange );
        AI_CACHE_INFO( "   Hash Upper    = %d", this->cpoCacheServer[i].ciMaxRange );
    }
    
    return 0;
}

void clsCacheClient::Close()
{
	for ( int i = 0; i < this->ciServerCnt; i++ )
	{
		this->cpoCacheServer[i].cpoAsyncClient->ShutDown();
	}
}

int clsCacheClient::Put( const char* apcKey, size_t aiKeySize, const char* apcValue, size_t aiValueSize, int aiFlag )
{
    int liRetCode = 0;
    int liCommand = (aiFlag == DO_OVERWRITE? AI_CACHESERVER_REQUEST_PUT_OW : AI_CACHESERVER_REQUEST_PUT);
	stCacheServerInfo*      lpoServer = this->GetServerByKey( apcKey, aiKeySize );
	clsRequestPut  loRequest;
	clsResponsePut loResponse;
	    
	loRequest.GetKey().Resize( aiKeySize );
	loRequest.GetValue().Resize( aiValueSize );
	
	::memcpy( loRequest.GetKey().BasePtr(), apcKey, aiKeySize );
	::memcpy( loRequest.GetValue().BasePtr(), apcValue, aiValueSize );
	
	if ( ( liRetCode = DoRequestTo( 
	     *lpoServer->cpoAsyncClient, liCommand, loRequest, loResponse ) ) != AI_NO_ERROR )
	{
	    if ( this->ciIsWriteLog )
	    {
		    AI_CACHE_LOG( liRetCode, 
		        "Put key/value fail, MSGCODE:%d, MSG:%s, [Key=%s]", liRetCode, StrError(liRetCode), apcKey );
	    }
	}
	else
	{
	    if ( this->ciIsWriteLog )
	    {
	        AI_CACHE_DEBUG( "Put key/value success, [key=%s]", apcKey );
	    }
	}
	
	return liRetCode;
}
	
int clsCacheClient::Put( const char* apcKey, const char* apcValue, size_t aiValueSize, int aiFlag )
{
    return this->Put( apcKey, strlen(apcKey) + 1, apcValue, aiValueSize, aiFlag );
}
	
int clsCacheClient::Put( const AIBC::AIChunkEx& aoKey, const AIBC::AIChunkEx& aoValue, int aiFlag )
{
    return this->Put( aoKey.BasePtr(), aoKey.GetSize(), aoValue.BasePtr(), aoValue.GetSize(), aiFlag );
}

int clsCacheClient::Get( const char* apcKey, size_t aiKeySize, AIBC::AIChunkEx& aoValue, int aiFlag )
{
    int liRetCode = 0;
    int liCommand = (aiFlag == DONT_DELETE? AI_CACHESERVER_REQUEST_GET_ND : AI_CACHESERVER_REQUEST_GET);
	clsRequestGet  loRequest;
	clsResponseGet loResponse;
	stCacheServerInfo*      lpoServer = this->GetServerByKey( apcKey, aiKeySize );
	
	loRequest.GetKey().Resize( aiKeySize );
	
	::memcpy( loRequest.GetKey().BasePtr(), apcKey, aiKeySize );

	if ( (liRetCode = DoRequestTo( 
	    *lpoServer->cpoAsyncClient, liCommand, loRequest, loResponse ) ) != AI_NO_ERROR )
	{
	    if ( this->ciIsWriteLog )
	    {
		    AI_CACHE_LOG( liRetCode,
		        "Get key/value fail, MSGCODE:%d, MSG:%s, [Key=%s]", liRetCode, StrError(liRetCode), apcKey );
	    }
	}
	else
	{
	    if ( this->ciIsWriteLog )
	    {
		    AI_CACHE_DEBUG( 
		        "Get key/value success, [Key=%s]", apcKey );
		        aoValue.Resize( loResponse.GetValue().GetSize() );
	    }
	    ::memcpy( aoValue.BasePtr(), loResponse.GetValue().BasePtr(), aoValue.GetSize() );
	}
	
	return liRetCode;
}

int clsCacheClient::Get( const char* apcKey, AIBC::AIChunkEx& aoValue, int aiFlag )
{
    return this->Get( apcKey, strlen(apcKey) + 1, aoValue, aiFlag );
}

int clsCacheClient::Get( const AIBC::AIChunkEx& aoKey, AIBC::AIChunkEx& aoValue, int aiFlag )
{
    return this->Get( aoKey.BasePtr(), aoKey.GetSize(), aoValue, aiFlag );
}

int clsCacheClient::Update( const char* apcKey, size_t aiKeySize, const char* apcValue, size_t aiValueSize )
{
    int                     liRetCode = 0;
	stCacheServerInfo*      lpoServer = this->GetServerByKey( apcKey, aiKeySize );
	clsRequestUpdate  loRequest;
	clsResponseUpdate loResponse;
	    
	loRequest.GetKey().Resize( aiKeySize );
	loRequest.GetValue().Resize( aiValueSize );
	
	::memcpy( loRequest.GetKey().BasePtr(), apcKey, aiKeySize );
	::memcpy( loRequest.GetValue().BasePtr(), apcValue, aiValueSize );
	
	if ( ( liRetCode = DoRequestTo( 
	     *lpoServer->cpoAsyncClient, AI_CACHESERVER_REQUEST_UPDATE, loRequest, loResponse ) ) != AI_NO_ERROR )
	{
	    if ( this->ciIsWriteLog )
	    {
		    AI_CACHE_LOG( liRetCode,
		        "Update key/value fail, MSGCODE:%d, MSG:%s, [Key=%s]", liRetCode, StrError(liRetCode), apcKey );
	    }
	}
	else
	{
	    if ( this->ciIsWriteLog )
	    {
	        AI_CACHE_DEBUG( "Update key/value success, [key=%s]", apcKey );
	    }
	}
	
	return liRetCode;
}
	
int clsCacheClient::Update( const char* apcKey, const char* apcValue, size_t aiValueSize )
{
    return this->Update( apcKey, strlen(apcKey) + 1, apcValue, aiValueSize );
}
	
int clsCacheClient::Update( const AIBC::AIChunkEx& aoKey, const AIBC::AIChunkEx& aoValue )
{
    return this->Update( aoKey.BasePtr(), aoKey.GetSize(), aoValue.BasePtr(), aoValue.GetSize() );
}

int clsCacheClient::GetTimeout( AIBC::AIChunkEx& aoKey, AIBC::AIChunkEx& aoValue )
{
    AIBC::AISmartLock loLock( this->coDelTimeOutLock );
    
    int    liRetCode = 0;
    size_t liStartServerIdx  = this->ciServerIdx;
	clsRequestDelTimeOut  loRequest;
	clsResponseDelTimeOut loResponse;
    
    if (this->ciServerCnt <= 0)
    {
        return AI_WARN_NO_FOUND;
    }

    do
	{
		if ( ( liRetCode = DoRequestTo( 
	        *this->cpoCacheServer[this->ciServerIdx].cpoAsyncClient, 
	        AI_CACHESERVER_REQUEST_DELTIMEOUT, loRequest, loResponse ) ) == AI_NO_ERROR )
	    {
		    aoKey.Resize( loResponse.GetKey().GetSize() );
		    aoValue.Resize( loResponse.GetValue().GetSize() );
		
		    ::memcpy( aoKey.BasePtr(), loResponse.GetKey().BasePtr(), aoKey.GetSize() );
		    ::memcpy( aoValue.BasePtr(), loResponse.GetValue().BasePtr(), aoValue.GetSize() );

            if ( this->ciIsWriteLog )
	        {
		        AI_CACHE_DEBUG( "Delete timeout success, [Key=%s]/[Server=%u]", aoKey.BasePtr(), this->ciServerIdx );
	        }
	        break;
	    }
	    else
	    { 
	        if ( this->ciIsWriteLog )
	        {
	            AI_CACHE_LOG( liRetCode, "Delete timeout fail, MSGCODE:%d, MSG:%s, [Server=%u]", 
    		        liRetCode, StrError(liRetCode), this->ciServerIdx );
    	    }
    	    this->ciServerIdx = ++this->ciServerIdx % this->ciServerCnt;
	    }
	}
	while( liStartServerIdx != this->ciServerIdx );
	
	return liRetCode;
}

int clsCacheClient::GetStat( int aiServerIdx, int* apiCapacity, int* apiSize, int* apiFileCount )
{
    int                      liRetCode = 0;
	clsRequestStat  loRequest;
	clsResponseStat loResponse;
	
	if ( ( liRetCode = DoRequestTo( 
        *this->cpoCacheServer[aiServerIdx].cpoAsyncClient, 
        AI_CACHESERVER_REQUEST_STAT, loRequest, loResponse ) ) == AI_NO_ERROR )
    {
        if ( apiCapacity ) *apiCapacity = loResponse.GetCacheCapacity();
        if ( apiSize ) *apiSize = loResponse.GetCacheSize();
        if ( apiFileCount ) *apiFileCount = loResponse.GetFileCount();
        
        if ( this->ciIsWriteLog )
	    {
		    AI_CACHE_LOG( liRetCode,
		        "Getstat fail, MSGCODE:%d, MSG:%s", liRetCode, StrError(liRetCode) );
	    }
	}
	else
	{
	    if ( this->ciIsWriteLog )
	    {
	        AI_CACHE_DEBUG( "Getstat success" );
	    }
	}
	
	return liRetCode;
}

int clsCacheClient::SetHashRangeValue( int aiLower, int aiUpper, stCacheServerInfo* apoServer, int* apiHashVal )
{
    if ( aiLower < 0 || (size_t)aiLower >= AI_SERVER_HASH_SIZE )
    {
        return aiLower;
    }
    if ( aiUpper < 0 || (size_t)aiUpper >= AI_SERVER_HASH_SIZE )
    {
        return aiUpper;
    }
    
    for( int liIdx = aiLower; liIdx <= aiUpper; liIdx++ )
    {
        if ( this->cpoCacheServerInfos[liIdx] != NULL )
        {
            *apiHashVal = liIdx;
            return -1;
        }
        
        this->cpoCacheServerInfos[liIdx] = apoServer;
    }
    return 0;
}

int clsCacheClient::CheckHashRangeValue( int* apiHashVal )
{
    for( size_t liIdx = 0; liIdx < AI_SERVER_HASH_SIZE; liIdx++ )
    {
        if ( this->cpoCacheServerInfos[liIdx] == NULL )
        {
            *apiHashVal = liIdx;
            return -1;
        }
    }
    return 0;
}

stCacheServerInfo* clsCacheClient::GetServerByKey( const char* apcKey, size_t aiKeySize )
{
    int liHashKey = HashString( apcKey, aiKeySize ) % AI_SERVER_HASH_SIZE;
	assert( liHashKey >= 0 );
	
	return this->cpoCacheServerInfos[liHashKey];
}

int clsCacheClient::GetServerCount()
{
    return this->ciServerCnt;
}

