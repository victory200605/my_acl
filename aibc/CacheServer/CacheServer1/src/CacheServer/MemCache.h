//=============================================================================
/**
* \file    clsBillCache.h
* \brief store file
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: MemCache.h,v 1.1 2011/01/10 08:46:55 daizh Exp $
*
* History
* 2008.06.18 first release by DZH
*/
//=============================================================================
#ifndef AI_MEMCACHE_H_2008
#define AI_MEMCACHE_H_2008

#include <string> 
#include <map> 
#include "Cache.h"
#include "cacheserver/CacheUtility.h"

AI_CACHE_NAMESPACE_START

class clsMemCache : public ICache
{
public:
    clsMemCache();

	virtual ~clsMemCache();

	virtual int Open( const char* apsDirPath );
	
    virtual void Close(void);
	
	// put key/value. aiFlag 
	virtual int Put( const AIChunkEx& aoKey, const AIChunkEx& aoValue, int aiFlag = DONT_OVERWRITE );
	virtual int Get( const AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp = NULL, int aiFlag = DO_DELETE );
	virtual int Put( const char* apsKey, const AIChunkEx& aoValue, int aiFlag = DONT_OVERWRITE );
	virtual int Get( const char* apsKey, AIChunkEx& aoValue, int* apiTimestamp = NULL, int aiFlag = DO_DELETE );
	virtual int Update( const AIChunkEx& aoKey, const AIChunkEx& aoValue );
	virtual int DelTimeOut( int aiTimestamp, AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp = NULL );
	
	virtual void GetStat( clsCacheStat& aoStat );

private:
    std::map<std::string, std::string> moMap;

    AIMutexLock moLock;
};

AI_CACHE_NAMESPACE_END

#endif // AI_BILLCACHE_H_2008
