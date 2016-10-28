//=============================================================================
/**
* \file    clsBillCache.h
* \brief store file
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: Cache.h,v 1.1 2011/01/10 08:46:55 daizh Exp $
*
* History
* 2008.06.18 first release by DZH
*/
//=============================================================================
#ifndef AI_CACHE_H_2008
#define AI_CACHE_H_2008

#include "cacheserver/CacheUtility.h"
#include "BillCache/CacheIndex.h"

AI_CACHE_NAMESPACE_START

class ICache
{
public:
	virtual ~ICache() {};

	virtual int Open( const char* apsDirPath ) = 0;
	
    virtual void Close(void) = 0;
	
	// put key/value. aiFlag 
	virtual int Put( const AIChunkEx& aoKey, const AIChunkEx& aoValue, int aiFlag = DONT_OVERWRITE ) = 0;
	virtual int Get( const AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp = NULL, int aiFlag = DO_DELETE ) = 0;
	virtual int Put( const char* apsKey, const AIChunkEx& aoValue, int aiFlag = DONT_OVERWRITE ) = 0;
	virtual int Get( const char* apsKey, AIChunkEx& aoValue, int* apiTimestamp = NULL, int aiFlag = DO_DELETE ) = 0;
	virtual int Update( const AIChunkEx& aoKey, const AIChunkEx& aoValue ) = 0;
	virtual int DelTimeOut( int aiTimestamp, AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp = NULL ) = 0;
	
	virtual void GetStat( clsCacheStat& aoStat ) = 0;
};

AI_CACHE_NAMESPACE_END

#endif // AI_BILLCACHE_H_2008
