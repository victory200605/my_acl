
#include "MemCacheServer.h"

AI_CACHE_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////
clsMChannel::clsMChannel( clsMemCacheServer* apoCache )
    : cpoCache(apoCache)
{
}

int clsMChannel::Input( const char* apcData, size_t aiSize )
{
    int               liRetCode = 0;
    AIBC::AIChunkEx   loBuffer(0);
    AICmdLineParamter loParamter;
    
    // Trace log
    AIBC::AIChunkEx loHexBuff(0);
    AI_CACHE_TRACE( "Channel [%d] Receive request [Length=%d]/[Package=\n%s]", aiSize, 
        UStringToHex( apcData, aiSize, loHexBuff ) );

    if ( loParamter.Decode( apcData, aiSize ) != 0 )
    {
        AI_CACHE_ERROR( "Channel [%d] invalid request, %s", this->GetChannelID(), apcData );
        AICmdLineParamter::Encode( loBuffer, "resp r=%d", AI_ERROR_INVALID_REQUST );
    }
    else
    {
        if ( ::strncmp( loParamter.GetCmdPtr(), "get", 3 ) == 0 )
        {
            this->Get( loParamter, loBuffer );
        }
        else if ( ::strncmp( loParamter.GetCmdPtr(), "add", 3 ) == 0 )
        {
            this->Add( loParamter, loBuffer );
        }
        else if ( ::strncmp( loParamter.GetCmdPtr(), "put", 3 ) == 0 )
        {
            this->Put( loParamter, loBuffer );
        }
        else if ( ::strncmp( loParamter.GetCmdPtr(), "del", 3 ) == 0 )
        {
            this->Del( loParamter, loBuffer );
        }
        else
        {
            AI_CACHE_ERROR( "Channel [%d] invalid request, %s", this->GetChannelID(), apcData );
            AICmdLineParamter::Encode( loBuffer, "resp r=%d", AI_ERROR_INVALID_REQUST );
        }
    }

    if ( ( liRetCode = this->Send( loBuffer.BasePtr(), loBuffer.GetSize() ) ) != 0 )
    {
        AI_CACHE_DEBUG( "Channel [%d] Response fail, MSGSYNCCODE:%d", this->GetChannelID(), liRetCode  );
    }
    
    // Trace log
    AI_CACHE_TRACE( "Channel [%d] Response request [Length=%d]/[Package=\n%s]", aiSize, 
        UStringToHex( loBuffer.BasePtr(), loBuffer.GetSize(), loHexBuff ) );
    
    return 0;
}

int clsMChannel::Error( int aiErrno )
{
    return 0;
}

int clsMChannel::Exit()
{
    AI_CACHE_INFO( "Channel [%d] connect from [%s:%d] exit now", 
        this->GetChannelID(), this->GetRemoteIp(), this->GetRemotePort() );
    
    return 0;
}

void clsMChannel::Add( AICmdLineParamter& aoParamter, AIBC::AIChunkEx& aoResponse )
{
    int    liRetCode     = 0;
    size_t liKeyLen      = aoParamter.GetLen('k');
    size_t liValueLen    = aoParamter.GetLen('v');
    const char* lpcKey   = aoParamter.GetPtr('k');
    const char* lpcValue = aoParamter.GetPtr('v');
    char lsTmpKey[AI_MAX_KEY_LEN] = {0};
    
    ::memcpy( lsTmpKey, lpcKey, (liKeyLen < AI_MAX_KEY_LEN ? liKeyLen : AI_MAX_KEY_LEN - 1) );
    
    if ( lpcKey == NULL || lpcValue == NULL || liKeyLen <= 0 )
    {
        AI_CACHE_ERROR( "Channel [%d] Add key/value fail, invalid request", this->GetChannelID() );
        AICmdLineParamter::Encode( aoResponse, "add-resp r=%d", AI_ERROR_INVALID_REQUST );
    }
    else
    {
        if ( ( liRetCode = this->cpoCache->Add( lpcKey, liKeyLen, lpcValue, liValueLen ) ) != 0 )
        {
            AI_CACHE_ERROR( "Channel [%d] Add key/value fail, [key=%s],MSGCODE:%d, MSG:%s", 
                this->GetChannelID(), lsTmpKey, liRetCode, StrError(liRetCode)  );
        }
        else
        {
            AI_CACHE_DEBUG( "Channel [%d] Add key/value success, [key=%s]", this->GetChannelID(), lsTmpKey  );
        }
        
        AICmdLineParamter::Encode( aoResponse, "add-resp r=%d", liRetCode );
    }
}

void clsMChannel::Put( AICmdLineParamter& aoParamter, AIBC::AIChunkEx& aoResponse )
{
    int    liRetCode     = 0;
    size_t liKeyLen      = aoParamter.GetLen('k');
    size_t liValueLen    = aoParamter.GetLen('v');
    const char* lpcKey   = aoParamter.GetPtr('k');
    const char* lpcValue = aoParamter.GetPtr('v');
    char lsTmpKey[AI_MAX_KEY_LEN] = {0};
    
    ::memcpy( lsTmpKey, lpcKey, (liKeyLen < AI_MAX_KEY_LEN ? liKeyLen : AI_MAX_KEY_LEN - 1) );
    
    if ( lpcKey == NULL || lpcValue == NULL || liKeyLen <= 0 )
    {
        AI_CACHE_ERROR( "Channel [%d] Put key/value fail, invalid request", this->GetChannelID() );
        AICmdLineParamter::Encode( aoResponse, "put-resp r=%d", AI_ERROR_INVALID_REQUST );
    }
    else
    {
        if ( ( liRetCode = this->cpoCache->Put( lpcKey, liKeyLen, lpcValue, liValueLen ) ) != 0 )
        {
            AI_CACHE_ERROR( "Channel [%d] Put key/value fail, [key=%s],MSGCODE:%d, MSG:%s", 
                this->GetChannelID(), lsTmpKey, liRetCode, StrError(liRetCode)  );
        }
        else
        {
            AI_CACHE_DEBUG( "Channel [%d] Put key/value success, [key=%s]", this->GetChannelID(), lsTmpKey  );
        }
        
        AICmdLineParamter::Encode( aoResponse, "put-resp r=%d", liRetCode );
    }
}

void clsMChannel::Get( AICmdLineParamter& aoParamter, AIBC::AIChunkEx& aoResponse )
{
    int    liRetCode   = 0;
    size_t liKeyLen    = aoParamter.GetLen('k');
    const char* lpcKey = aoParamter.GetPtr('k');
    AIBC::AIChunkEx   loValue(0);
    char lsTmpKey[AI_MAX_KEY_LEN] = {0};
    
    ::memcpy( lsTmpKey, lpcKey, (liKeyLen < AI_MAX_KEY_LEN ? liKeyLen : AI_MAX_KEY_LEN - 1) );
    
    if ( lpcKey == NULL || liKeyLen <= 0 )
    {
        AI_CACHE_ERROR( "Channel [%d] Get key/value fail, invalid request", this->GetChannelID() );
        AICmdLineParamter::Encode( aoResponse, "get-resp r=%d", AI_ERROR_INVALID_REQUST );
        return;
    }
    else
    {
        if ( ( liRetCode = this->cpoCache->Get( lpcKey, liKeyLen, loValue ) ) != 0 )
        {
            AI_CACHE_ERROR( "Channel [%d] Get key/value fail, [key=%s],MSGCODE:%d, MSG:%s", 
                this->GetChannelID(), lsTmpKey, liRetCode, StrError(liRetCode)  );
            AICmdLineParamter::Encode( aoResponse, "resp r=%d", liRetCode );
        }
        else
        {
            AI_CACHE_DEBUG( "Channel [%d] Get key/value success, [key=%s]", this->GetChannelID(), lsTmpKey  );
            AICmdLineParamter::Encode( aoResponse, "get-resp r=%d v=%s", liRetCode, loValue.BasePtr() );
        }
    }
}

void clsMChannel::Del( AICmdLineParamter& aoParamter, AIBC::AIChunkEx& aoResponse )
{
    int    liRetCode   = 0;
    size_t liKeyLen    = aoParamter.GetLen('k');
    const char* lpcKey = aoParamter.GetPtr('k');
    AIBC::AIChunkEx   loValue(0);
    char lsTmpKey[AI_MAX_KEY_LEN] = {0};
    
    ::memcpy( lsTmpKey, lpcKey, (liKeyLen < AI_MAX_KEY_LEN ? liKeyLen : AI_MAX_KEY_LEN - 1) );
    
    if ( lpcKey == NULL || liKeyLen <= 0 )
    {
        AI_CACHE_ERROR( "Channel [%d] Del key/value fail, invalid request", this->GetChannelID() );
        AICmdLineParamter::Encode( aoResponse, "del-resp r=%d", AI_ERROR_INVALID_REQUST );
        return;
    }
    else
    {
        if ( ( liRetCode = this->cpoCache->Del( lpcKey, liKeyLen, loValue ) ) != 0 )
        {
            AI_CACHE_ERROR( "Channel [%d] Del key/value fail, [key=%s],MSGCODE:%d, MSG:%s", 
                this->GetChannelID(), lsTmpKey, liRetCode, StrError(liRetCode)  );
                
            AICmdLineParamter::Encode( aoResponse, "del-resp r=%d", liRetCode );
        }
        else
        {
            AI_CACHE_DEBUG( "Channel [%d] Del key/value success, [key=%s]", this->GetChannelID(), lsTmpKey  );
            
            AICmdLineParamter::Encode( aoResponse, "del-resp r=%d v=%s", liRetCode, loValue.BasePtr() );
        }
    }
}
    
/////////////////////////////////////////////////////////////////////////////////////
clsMemCacheServer::stValueNode::stValueNode( const char* apcValue, size_t aiSize )
    : ciTimestamp(time(NULL))
    , coStrValue(apcValue, aiSize)
{
}

/////////////////////////////////////////////////////////////////////////////////////
clsMemCacheServer::stValuePtr::stValuePtr()
    : cpoValue(NULL)
{
}

clsMemCacheServer::stValuePtr::stValuePtr( stValueNode* apoValue )
    : cpoValue(apoValue)
{
}
        
clsMemCacheServer::stValuePtr& clsMemCacheServer::stValuePtr::operator = ( stValueNode* apoRhs )
{
    this->cpoValue = apoRhs;
    return *this;
}

clsMemCacheServer::stValueNode* clsMemCacheServer::stValuePtr::operator -> ()
{
    return this->cpoValue;
}

clsMemCacheServer::stValuePtr::operator clsMemCacheServer::stValueNode* ()
{
    return this->cpoValue;
}

/////////////////////////////////////////////////////////////////////////////////////
clsMemCacheServer* clsMemCacheServer::Instance()
{
    static clsMemCacheServer* slpoMemCache = NULL;
    
    if ( slpoMemCache == NULL )
    {
        AI_NEW_ASSERT( slpoMemCache, clsMemCacheServer );
    }
    
    return slpoMemCache;
}

clsMemCacheServer::clsMemCacheServer()
    : coHashMap(AI_HASH_SIZE)
    , ciLocalTimeoutInterval(0)
    , ciPrintStatInterval(0)
    , ciClearTimeoutInterval(0)
    , ciIsHasCacheServer(1)
    , ciMaxCapacity(0)
{
}
    
clsMemCacheServer::~clsMemCacheServer()
{
}

int clsMemCacheServer::Accept( AIChannelPtr aoChannelPtr )
{
    AI_CACHE_INFO( "Channel [%d] connect from [%s:%d] login", 
        aoChannelPtr->GetChannelID(), aoChannelPtr->GetRemoteIp(), aoChannelPtr->GetRemotePort() );
    
    aoChannelPtr->Ready();
    
    return 0;
}

AIBaseChannel* clsMemCacheServer::CreateChannel()
{
    clsMChannel* lpoChannel = NULL;
    
    AI_NEW_ASSERT( lpoChannel, clsMChannel(this) );
    
    return lpoChannel;
}

int clsMemCacheServer::Startup( const char* apcConfigFile )
{
    AIBC::AIConfig* lpoIni = NULL;
    int       liRetCode  = AI_NO_ERROR;

	if ( (lpoIni = AIBC::AIGetIniHandler(apcConfigFile)) == NULL )
    {
        AI_CACHE_ERROR( "Load config file %s fail", apcConfigFile );
        return 1;
    }
    
    AI_CACHE_INFO( "%s", ">>>>>>>>>>>>>>>>>>>>>>>>>>Server<<<<<<<<<<<<<<<<<<<<<<<<<<<" );
	char lcCacheClientIni[1024];

    //load config variable
    this->ciLocalTimeoutInterval  = lpoIni->GetIniInt( "CacheServer", "LocalTimeout", 10 );
    this->ciClearTimeoutInterval  = lpoIni->GetIniInt( "CacheServer", "ClearTimeoutInterval", 0 );
    this->ciPrintStatInterval     = lpoIni->GetIniInt( "CacheServer", "PrintStatInterval", 0 );
    this->ciIsHasCacheServer      = lpoIni->GetIniInt( "CacheServer", "HasCacheServer", 1 );
    this->ciMaxCapacity           = lpoIni->GetIniInt( "CacheServer", "MaxCapacity", 5000000 );
    lpoIni->GetIniString( "CacheServer", "CacheClientIniPath", lcCacheClientIni, sizeof(lcCacheClientIni) );
    
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
    lpoIni->GetIniString( "SERVER", "AdminListenIpAddr", lcAdminListenIpAddr, sizeof(lcAdminListenIpAddr ) );

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
	}

	//complete configure
	{
	    AI_CACHE_INFO( "%s", "-------------Server Config Variable--------------" );
	    AI_CACHE_INFO( "* Listen Port             = %d", liServerPort);
	    AI_CACHE_INFO( "* Has cache server        = %d", this->ciIsHasCacheServer);
	    AI_CACHE_INFO( "* Local timeout           = %d", this->ciLocalTimeoutInterval);
        AI_CACHE_INFO( "* Print Interval          = %d", this->ciPrintStatInterval);
        AI_CACHE_INFO( "* Max Capacity            = %u", this->ciMaxCapacity);
        AI_CACHE_INFO( "* Auto clear timeout interval = %d", this->ciClearTimeoutInterval);
        AI_CACHE_INFO( "* Cache client ini path   = %s", lcCacheClientIni);
	}
	
	if ( this->IsHasCacheServer() )
	{
    	AI_CACHE_INFO( "Start Initialize cacheserver api..." );
    	if ( this->coCacheClient.Initialize(lcCacheClientIni) != 0 )
    	{
    	    AI_CACHE_ERROR( "Initialize cacheserver api fail" );
    	    return -1;
    	}
    }
	
	if ( ( liRetCode = AIBaseAcceptor::Startup( lcAdminListenIpAddr, liServerPort ) ) != 0 )
	{
	    if ( liRetCode == ASYNC_ERROR_LISTENFAIL )
	    {
            AI_CACHE_ERROR( "Server startup fail, ErrMsg=AsyncServer Listen port fail, SysMsg:%s", strerror(errno) );
        }
        else
        {
            AI_CACHE_ERROR( "Server startup fail Addr=%s:%d, ErrMsg=System error, SysMsg:%s", 
                lcAdminListenIpAddr, liServerPort, strerror(errno) );
	    }
	    
	    return -1;
	}
	else
	{
	    AI_CACHE_INFO( "%s", "Server startup success" );
	    return 0;
	}
}

bool clsMemCacheServer::IsHasCacheServer()
{
    return this->ciIsHasCacheServer == 1;
}

void clsMemCacheServer::Shutdown()
{
    this->coCacheClient.Close();
    this->Close();
    while ( this->DelTimeout( time(NULL) + 3600 ) == AI_NO_ERROR ){}; //Delete all
}

void* clsMemCacheServer::AutoDelTimeoutEntry( void* apvParam )
{
    clsMemCacheServer* lpoCache = static_cast<clsMemCacheServer*>(apvParam);
    
    pthread_detach( pthread_self() );
    
    while( !AIM::IsShutdown() 
        && lpoCache->ciClearTimeoutInterval > 0 )
    {
        //Ignore
        if ( lpoCache->DelTimeout( time(NULL) - lpoCache->ciLocalTimeoutInterval ) != AI_NO_ERROR )
        {
            AIBC::AISleepFor( AI_TIME_SEC * lpoCache->ciClearTimeoutInterval );
        }
    }
    
    return NULL;
}

void clsMemCacheServer::EnsureSafeCapacity()
{
    if ( this->coHashMap.size() >= this->ciMaxCapacity )
    {
        this->DelTimeout( time(NULL) + 3600 );
    }
}

int clsMemCacheServer::Add( const char* apcKey, size_t aiKeyLen, const char* apcValue, size_t aiValueLen )
{
    int liRetCode = 0;
    AI_STD::string loKey( apcKey, aiKeyLen );

    if ( this->IsExisted(loKey) )
    {
        return AI_ERROR_DUP_RECORD;
    }
    
    stValueNode* lpoValue = NULL;
    AI_NEW_ASSERT( lpoValue, stValueNode( apcValue, aiValueLen ) );
    
    if ( this->IsHasCacheServer() )
    {
        // Insert to CacheServer,flag = NO_OVERWRITE
        liRetCode = this->coCacheClient.Put( 
            loKey.c_str(),
            loKey.length() + 1,
            lpoValue->coStrValue.c_str(), 
            lpoValue->coStrValue.length() + 1, DONT_OVERWRITE);
        if ( liRetCode != AI_NO_ERROR )
        {
            AI_DELETE( lpoValue );
            
            return liRetCode;
        }
    }
    
    this->EnsureSafeCapacity();
    
    if ( !this->Insert( loKey, lpoValue ) )
    {
        AI_DELETE( lpoValue );
        return AI_ERROR_DUP_RECORD;
    }
    else
    {
        return AI_NO_ERROR;
    }
}
    
int clsMemCacheServer::Put( const char* apcKey, size_t aiKeyLen, const char* apcValue, size_t aiValueLen )
{
    int            liRetCode = 0;
    stValueNode*   lpoValue = NULL;
    AI_STD::string loKey( apcKey, aiKeyLen );
    
    AI_NEW_ASSERT( lpoValue, stValueNode( apcValue, aiValueLen ) );
    
    if ( this->IsHasCacheServer() )
    {
        // Insert to CacheServer,flag = OVERWRITE
        liRetCode = this->coCacheClient.Put( 
            loKey.c_str(), 
            loKey.length() + 1,
            lpoValue->coStrValue.c_str(), 
            lpoValue->coStrValue.length() + 1, DO_OVERWRITE);
        if ( liRetCode != AI_NO_ERROR )
        {
            AI_DELETE( lpoValue );
            
            return liRetCode;
        }
    }
    
    this->EnsureSafeCapacity();
    
    this->InsertReplace( loKey, lpoValue );
    
    return AI_NO_ERROR;
}

int clsMemCacheServer::Get( const char* apcKey, size_t aiKeyLen, AIBC::AIChunkEx& aoValue )
{
    int            liRetCode = 0;
    AI_STD::string loKey( apcKey, aiKeyLen );
        
    if ( !this->Get( loKey, aoValue ) )
    {
        if ( this->IsHasCacheServer() )
        {
            // Get from CacheServer, flag = NO_DELETE
            liRetCode = this->coCacheClient.Get( loKey.c_str(), loKey.length() + 1, aoValue, DONT_DELETE );
            if ( liRetCode == AI_NO_ERROR )
            {
                stValueNode* lpoValue = NULL;
                AI_NEW_ASSERT( lpoValue, stValueNode( aoValue.BasePtr(), aoValue.GetSize() ) );
                
                this->EnsureSafeCapacity();
                
                if ( !this->Insert( loKey, lpoValue) )
                {
                    AI_DELETE( lpoValue );
                }

                return AI_NO_ERROR;
            }
        }
        
        return AI_WARN_NO_FOUND;
    }
    
    return AI_NO_ERROR;
}

int clsMemCacheServer::Del( const char* apcKey, size_t aiKeyLen, AIBC::AIChunkEx& aoValue )
{
    int            liRetCode = 0;
    AI_STD::string loKey( apcKey, aiKeyLen );
    
    if ( this->IsHasCacheServer() )
    {
        // Del from CacheServer, flag = DELETE
        liRetCode = this->coCacheClient.Get( loKey.c_str(), loKey.length() + 1, aoValue );
        if ( liRetCode != AI_NO_ERROR 
            && liRetCode != AI_WARN_NO_FOUND )
        {
            return liRetCode;
        }
    }
    
    return ( this->Erase( loKey, aoValue ) ? AI_NO_ERROR : AI_WARN_NO_FOUND );
}

bool clsMemCacheServer::IsExisted( const AI_STD::string& aoKey )
{
    AIBC::AISmartLock loLock( this->coLock );
    
    TIterator loIter = this->coHashMap.find( aoKey );
    if ( loIter == this->coHashMap.end() )
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool clsMemCacheServer::Insert( const AI_STD::string& aoKey, stValueNode* apoValue )
{
    AIBC::AISmartLock loLock( this->coLock );
    
    stValuePtr& loValuePtr = this->coHashMap[aoKey];
    if ( loValuePtr == NULL )
    {
        loValuePtr = apoValue;
        return true;
    }
    else
    {
        return false;
    }
}

void clsMemCacheServer::InsertReplace( const AI_STD::string& aoKey, stValueNode* apoValue )
{
    AIBC::AISmartLock loLock( this->coLock );
    
    stValuePtr& loValuePtr = this->coHashMap[aoKey];
    if ( loValuePtr == NULL )
    {
        loValuePtr = apoValue;
    }
    else
    {
        stValueNode* lpoTmpValue = (stValueNode*)loValuePtr;
        
        AI_DELETE( lpoTmpValue );
        
        loValuePtr = apoValue;
    }
}
     
bool clsMemCacheServer::Erase( const AI_STD::string& aoKey, AIBC::AIChunkEx& aoValue )
{
    AIBC::AISmartLock loLock( this->coLock );
    
    TIterator loIter = this->coHashMap.find( aoKey );
    if ( loIter == this->coHashMap.end() )
    {
        return false;
    }
    else
    {
        stValueNode* lpoTmpValue = (stValueNode*)loIter->second;
        
        aoValue.Resize( loIter->second->coStrValue.length() + 1 );
        ::memcpy( aoValue.BasePtr(), loIter->second->coStrValue.c_str(), loIter->second->coStrValue.length() + 1 );
        
        AI_DELETE( lpoTmpValue );
        
        this->coHashMap.erase(loIter);
        
        return true;
    }
}

bool clsMemCacheServer::Get( const AI_STD::string& aoKey, AIBC::AIChunkEx& aoValue )
{
    AIBC::AISmartLock loLock( this->coLock );
    
    TIterator loIter = this->coHashMap.find( aoKey );
    if ( loIter == this->coHashMap.end() )
    {
        return false;
    }
    else
    {
        aoValue.Resize( loIter->second->coStrValue.length() + 1 );
        ::memcpy( aoValue.BasePtr(), loIter->second->coStrValue.c_str(), loIter->second->coStrValue.length() + 1 );
            
        return true;
    }
}

int clsMemCacheServer::DelTimeout( int aiTimestamp )
{
    AIBC::AISmartLock loLock( this->coLock );
    
    TIterator loIter = this->coHashMap.begin();
    if ( loIter != this->coHashMap.end() 
        && loIter->second->ciTimestamp < aiTimestamp )
    {
        stValueNode* lpoTmpValue = (stValueNode*)loIter->second;
        
        AI_DELETE( lpoTmpValue );
        
        this->coHashMap.erase(loIter);
        return AI_NO_ERROR;
    }
    else
    {
        return AI_WARN_NO_FOUND;
    }
}

void clsMemCacheServer::PrintStat()
{
    static int sliTimestamp = time(NULL);
    
    if ( sliTimestamp + this->ciPrintStatInterval <= time(NULL) )
    {
        AI_CACHE_INFO( "%s", "-----------------------------------------------------------" );
	    AI_CACHE_INFO( "* Current size = %d", this->coHashMap.size() );
	    AI_CACHE_INFO( "%s", "-----------------------------------------------------------" );
	    
	    sliTimestamp = time(NULL);
	}
}

AI_CACHE_NAMESPACE_END

