//=============================================================================
/**
* \file    clsBillCache.h
* \brief store file
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: BillCache.h,v 1.2 2011/01/10 08:46:55 daizh Exp $
*
* History
* 2008.06.18 first release by DZH
*/
//=============================================================================
#ifndef AI_BILLCACHE_H_2008
#define AI_BILLCACHE_H_2008

#include "CacheData.h"
#include "CacheIndex.h"
#include "../Cache.h"


AI_CACHE_NAMESPACE_START

// cache stat type
typedef clsCacheIndex::TStat TStat;
typedef clsCacheIndex::TIterator TIterator;

class clsBillCache : public ICache
{
public:
    // local lock
    class clsLock
    {
    public:
        clsLock( AIMutexLock& aoLock ) : coLock(aoLock) { coLock.Lock(); }
        ~clsLock( ) { coLock.Unlock(); }
    protected:
        AIMutexLock& coLock;
    };
    
public:
	clsBillCache();
	virtual ~clsBillCache();

	void SetTimePoint( int aiTimePoint );
	void SetSyncLimit( int aiIOTime );
	
	//create bill cache
	int Create( const char* apsDirPath, int aiCount, int aiSize );
	
	int Open( const char* apsDirPath );
	int Clean( );
	void Close( );
	
	// put key/value. aiFlag 
	int Put( const AIChunkEx& aoKey, const AIChunkEx& aoValue, int aiFlag = DONT_OVERWRITE );
	int Get( const AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp = NULL, int aiFlag = DO_DELETE );
	int Put( const char* apsKey, const AIChunkEx& aoValue, int aiFlag = DONT_OVERWRITE );
	int Get( const char* apsKey, AIChunkEx& aoValue, int* apiTimestamp = NULL, int aiFlag = DO_DELETE );
	int Update( const AIChunkEx& aoKey, const AIChunkEx& aoValue );
	int DelTimeOut( int aiTimestamp, AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp = NULL );
	
	///For CacheServer Iterator
	TIterator Begin();
	TIterator End();
	int Get( TIterator aoIter, AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp = NULL );
	///
	
	void GetStat( TStat& aoStat );

protected:
    bool IsInvalidKey( const AIChunkEx& aoKey );
    bool IsInvalidValue( const AIChunkEx& aoValue );
    
protected:
    clsCacheData coCacheData;
	clsCacheIndex coCacheIndex;
	
	bool cbIsOpened;
	
	AIMutexLock coLock;
};

AI_CACHE_NAMESPACE_END

#endif // AI_BILLCACHE_H_2008
