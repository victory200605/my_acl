
#ifndef	__AI_CACHECLIENT_API_H__
#define	__AI_CACHECLIENT_API_H__

#include "CacheUtility.h"

#define	CACHECLIENT_CONFIG_FILE	"../../config/CacheClientV2.ini"

struct stCacheServerInfo
{
    stCacheServerInfo();
    ~stCacheServerInfo();
    
	char csIpAddr[cacheserver::AI_IP_ADDR_LEN];
	int ciPort;
	int ciConnection;
	int ciTimeout;
	int ciMinRange;
	int ciMaxRange;
	AIBC::clsAsyncClient* cpoAsyncClient;
};

class clsCacheClient
{
public:
	clsCacheClient();
	~clsCacheClient();
	
	int  Initialize( const char* apcConfigFile = CACHECLIENT_CONFIG_FILE );
    int  Initialize( stCacheServerInfo* apoSrvList, int aiCount);
	void Close();

    int Put( const char* apcKey, size_t aiKeySize, const char* apcValue, size_t aiValueSize, int aiFlag = cacheserver::DONT_OVERWRITE );
    
	int Put( const char* apcKey, const char* apcValue, size_t aiValueSize, int aiFlag = cacheserver::DONT_OVERWRITE );
	
	int Put( const AIBC::AIChunkEx& aoKey, const AIBC::AIChunkEx& aoValue, int aiFlag = cacheserver::DONT_OVERWRITE );
	
	int Get( const char* apcKey, size_t aiKeySize, AIBC::AIChunkEx& aoValue, int aiFlag = cacheserver::DO_DELETE );
	
	int Get( const char* apcKey, AIBC::AIChunkEx& aoValue, int aiFlag = cacheserver::DO_DELETE );

	int Get( const AIBC::AIChunkEx& aoKey, AIBC::AIChunkEx& aoValue, int aiFlag = cacheserver::DO_DELETE );

    int Update( const char* apcKey, size_t aiKeySize, const char* apcValue, size_t aiValueSize );
    
	int Update( const char* apcKey, const char* apcValue, size_t aiValueSize );
	
	int Update( const AIBC::AIChunkEx& aoKey, const AIBC::AIChunkEx& aoValue );
	
	int GetTimeout( AIBC::AIChunkEx& aoKey, AIBC::AIChunkEx& aoValue );
	
	int GetStat( int aiServerIdx, int* apiCapacity, int* apiSize, int* apiFileCount );
	
	int GetServerCount();
    
// operator
protected:
    int SetHashRangeValue( int aiLower, int aiUpper, stCacheServerInfo* apoServer, int* apiHashVal );
    int CheckHashRangeValue( int* apiHashVal );
	stCacheServerInfo* GetServerByKey( const char* apcKey, size_t aiKeySize );

// attribute
protected:
	AIBC::AIMutexLock coDelTimeOutLock;
	
	int  ciIsWriteLog;
	int  ciServerCnt;
	
    size_t ciServerIdx;

	stCacheServerInfo *cpoCacheServer;
	
	stCacheServerInfo* cpoCacheServerInfos[cacheserver::AI_SERVER_HASH_SIZE];
};

#endif //AI_CACHECLIENT_API_H
