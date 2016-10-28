
#ifndef __AI_MEMCACHESERVER_H__
#define __AI_MEMCACHESERVER_H__

#include "../include/AICacheTcp.h"
#include "../include/AICmdLineTcp.h"
#include "../include/Utility.h"
#include "../include/CacheClient.h"
#include "stl/ordered_hash_map.h"

AI_CACHE_NAMESPACE_START

class clsMemCacheServer;
class clsMChannel : public AICmdLineChannel
{
public:
    clsMChannel( clsMemCacheServer* apoCache );
    
    virtual int Input( const char* apcData, size_t aiSize );
    virtual int Error( int aiErrno );
    virtual int Exit();
    
protected:
    void Add( AICmdLineParamter& aoParamter, AIBC::AIChunkEx& aoResponse );
    void Put( AICmdLineParamter& aoParamter, AIBC::AIChunkEx& aoResponse );
    void Get( AICmdLineParamter& aoParamter, AIBC::AIChunkEx& aoResponse );
    void Del( AICmdLineParamter& aoParamter, AIBC::AIChunkEx& aoResponse );
    
protected:
    clsMemCacheServer* cpoCache;
};

/// Memory cache
class clsMemCacheServer : public AIBaseAcceptor
{
public:
    /// Cache node
    struct stValueNode
    {
        stValueNode( const char* apcValue, size_t aiSize );
        
        int ciTimestamp;
        AI_STD::string coStrValue;
    };
    
    /// Value pointer
    struct stValuePtr
    {
        stValuePtr();
        
        stValuePtr( stValueNode* apoValue );
        
        stValuePtr& operator = ( stValueNode* apoRhs );
        stValueNode* operator -> ();
        operator stValueNode* ();
        
        stValueNode* cpoValue;
    };

    typedef AI_STD::ordered_hash_map< AI_STD::string, stValuePtr > THashMap;
    typedef THashMap::iterator TIterator;
        
public:
    static clsMemCacheServer* Instance();
    
    ~clsMemCacheServer();
    
    int Startup( const char* apcConfigFile );
    void Shutdown();
    void PrintStat();

/// record operator
    int Add( const char* apcKey, size_t aiKeyLen, const char* apcValue, size_t aiValueLen );
    int Put( const char* apcKey, size_t aiKeyLen, const char* apcValue, size_t aiValueLen );
    int Get( const char* apcKey, size_t aiKeyLen, AIBC::AIChunkEx& aoValue );
    int Del( const char* apcKey, size_t aiKeyLen, AIBC::AIChunkEx& aoValue );
    int DelTimeout( int aiTimestamp );
    
protected:
    clsMemCacheServer();
    bool IsHasCacheServer();
    void EnsureSafeCapacity();

///
protected:
    inline int ErrorCacheServer( int aiRet, int aiDefault )
    {
        switch( aiRet )
        {
            default:                                return aiDefault;
        }
    }

/// overwrite virtual interface
protected:
    virtual int Accept( AIChannelPtr aoChannelPtr );
    virtual AIBaseChannel* CreateChannel();

/// static operator
protected:
    static void* AutoDelTimeoutEntry( void* apvParam );
    
/// Hashmap operator
protected:
    bool IsExisted( const AI_STD::string& aoKey );
    bool Insert( const AI_STD::string& aoKey, stValueNode* apoValue );
    void InsertReplace( const AI_STD::string& aoKey, stValueNode* apoValue );
    bool Erase( const AI_STD::string& aoKey, AIBC::AIChunkEx& aoValue );
    bool Get( const AI_STD::string& aoKey, AIBC::AIChunkEx& aoValue );
    
/// Attribute
protected:
    AIBC::AIMutexLock coLock;
    THashMap    coHashMap;
    
    ///config variable
    int ciLocalTimeoutInterval;
    int ciPrintStatInterval;
    int ciClearTimeoutInterval;
    int ciIsHasCacheServer;
    size_t ciMaxCapacity;
    
    ///cache server
    clsCacheClient coCacheClient;
};

AI_CACHE_NAMESPACE_END

#endif //__AI_MEMCACHE_H__
