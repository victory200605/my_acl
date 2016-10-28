///////////////////////////////////////////////////////////
//  clsBillCache.cpp
//  Implementation of the Class clsBillCache
//  Created on:      15-ÎåÔÂ-2008 14:43:49
//  Original author: dzh
///////////////////////////////////////////////////////////
#include "BillCache.h"
#include "AIDir.h"

AI_CACHE_NAMESPACE_START

clsBillCache::clsBillCache() : cbIsOpened (false)
{
}

clsBillCache::~clsBillCache()
{
    Close( );
}

void clsBillCache::SetTimePoint( int aiTimePoint )
{
    coCacheIndex.SetTimePoint( aiTimePoint );
}

void clsBillCache::SetSyncLimit( int aiIOTime )
{
    coCacheData.SetSyncLimit( aiIOTime );
}

int clsBillCache::Create( const char* apsDirPath, int aiCount, int aiSize )
{
    AIDir::Make(apsDirPath);
        
    char lcTmpFileName[AI_MAX_FILENAME_LEN];
    memset( lcTmpFileName, 0, AI_MAX_FILENAME_LEN );
    int liRet = 0;
    
    //Do for index, load
    snprintf( lcTmpFileName, AI_MAX_FILENAME_LEN, "%s/__Index", apsDirPath );
    if ( ( liRet = coCacheIndex.Create( lcTmpFileName, aiCount, aiSize ) ) != AI_NO_ERROR )
    {
        return liRet;
    }
    
    //Do for store data file
    snprintf( lcTmpFileName, AI_MAX_FILENAME_LEN, "%s/__data", apsDirPath );
    if ( ( liRet = coCacheData.Create( lcTmpFileName, aiCount, aiSize ) ) != AI_NO_ERROR )
    {
        return liRet;
    }
    
	return AI_NO_ERROR;
}

int clsBillCache::Open( const char* apsDirPath )
{
    if ( cbIsOpened )
    {
        Close();
    }
        
    char lcTmpFileName[AI_MAX_FILENAME_LEN];
    memset( lcTmpFileName, 0, AI_MAX_FILENAME_LEN );
    int liRet = 0;
    
    //Do for index, load
    snprintf( lcTmpFileName, AI_MAX_FILENAME_LEN, "%s/__Index", apsDirPath );
    if ( ( liRet = coCacheIndex.Open( lcTmpFileName ) ) != AI_NO_ERROR )
    {
        return liRet;
    }
    TStat loStat;
    coCacheIndex.GetStat( loStat );
    
    //Do for store data file
    snprintf( lcTmpFileName, AI_MAX_FILENAME_LEN, "%s/__data", apsDirPath );
    if ( ( liRet = coCacheData.Open( 
           lcTmpFileName, loStat.GetNodeCount(), loStat.GetNodeCapacity() ) ) != AI_NO_ERROR )
    {
        return liRet;
    }
    
    cbIsOpened = true;
    
	return AI_NO_ERROR;
}

int clsBillCache::Clean( )
{
    if ( !cbIsOpened ) return AI_ERROR_UNOPENED;
    
    // local lock protected
    clsLock loLock( coLock );
        
    return coCacheIndex.DelAll();
}

void clsBillCache::Close( )
{
    if ( !cbIsOpened ) return;
    
    // local lock protected
    clsLock loLock( coLock );
    
    coCacheData.Close();
    coCacheIndex.Close();
}

int clsBillCache::Put( const AIChunkEx& aoKey, const AIChunkEx& aoValue, int aiFlag /*= NO_OVERWRITE*/ )
{
    if ( !cbIsOpened ) return AI_ERROR_UNOPENED;
    
    if ( IsInvalidKey( aoKey ) || IsInvalidValue( aoValue ) )
    {
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    // local lock protected
    clsLock loLock( coLock );
    
    int liRet = 0;
    clsCacheIndex::TNodeHandle ltHandle;

    //try to check duple record
    if ( ( liRet = coCacheIndex.Get( aoKey, &ltHandle ) ) == AI_NO_ERROR )
    {
        //replace node?
        if ( aiFlag == DO_OVERWRITE )
        {
            if ( ( liRet = coCacheIndex.Del( ltHandle ) ) != AI_NO_ERROR )
            {//del node fail
                return liRet;
            }
        }
        else
        {
            //duple record
            return AI_ERROR_DUP_RECORD;
        }
    }

    ltHandle = coCacheIndex.GetFreeNode();
    if ( ltHandle == AI_INVALID_NODE_HANDLE )
    {//Can't get a free handle
        return AI_ERROR_NO_ENOUGH_SPACE;
    }

    //write to data file
    int liTimestamp = time(NULL);
    int liIdx = coCacheIndex.GetIdx(ltHandle);// for CacheData
    if ( ( liRet = coCacheData.Write( liIdx, aoKey, aoValue, liTimestamp ) ) != AI_NO_ERROR )
    {
        coCacheIndex.PutFreeNode( ltHandle );
        return liRet;
    }

    //commit to index
    if ( ( liRet = coCacheIndex.Put( ltHandle, aoKey, liTimestamp ) ) != AI_NO_ERROR )
    {
        coCacheIndex.PutFreeNode( ltHandle );
        return liRet;
    }

	return AI_NO_ERROR;
}

int clsBillCache::Get( const AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp /* = NULL */, int aiFlag )
{
    if ( !cbIsOpened ) return AI_ERROR_UNOPENED;
    
    if ( IsInvalidKey( aoKey ) )
    {
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    // local lock protected
    clsLock loLock( coLock );
    
    int liRet = AI_NO_ERROR;
    clsCacheIndex::TNodeHandle ltHandle;
    if ( ( liRet = coCacheIndex.Get( aoKey, &ltHandle ) ) != AI_NO_ERROR )
    {
        return liRet;
    }
    
    //read from data file
    int liIdx = coCacheIndex.GetIdx(ltHandle);
    AIChunkEx loTmpKey;
    if ( ( liRet = coCacheData.Read( liIdx, loTmpKey, aoValue, apiTimestamp ) ) != AI_NO_ERROR )
    {
        return liRet;
    }
    
    //delete from hash index
    if ( aiFlag == DO_DELETE && ( liRet = coCacheIndex.Del( ltHandle ) ) != AI_NO_ERROR )
    {//oh,delete node fail
        return liRet;
    }
    
	return AI_NO_ERROR;
}

int clsBillCache::Put( const char* apsKey, const AIChunkEx& aoValue, int aiFlag /*= NO_OVERWRITE*/ )
{
    size_t liSize = strlen(apsKey) + 1;
    AIChunkEx loKey(liSize);
    memcpy( loKey.BasePtr(), apsKey, liSize );
    
    return Put( loKey, aoValue, aiFlag );
}

int clsBillCache::Get( const char* apsKey, AIChunkEx& aoValue, int* apiTimestamp /*= NULL*/, int aiFlag )
{
    size_t liSize = strlen(apsKey) + 1;
    AIChunkEx loKey(liSize);
    memcpy( loKey.BasePtr(), apsKey, liSize );
    
    return Get( loKey, aoValue, apiTimestamp );
}

int clsBillCache::Update( const AIChunkEx& aoKey, const AIChunkEx& aoValue )
{
    if ( !cbIsOpened ) return AI_ERROR_UNOPENED;
    
    if ( IsInvalidKey( aoKey ) || IsInvalidValue( aoValue ) )
    {
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    // local lock protected
    clsLock loLock( coLock );
    
    int liRet = 0;
    clsCacheIndex::TNodeHandle ltHandle;

    //try to check duple record
    if ( ( liRet = coCacheIndex.Get( aoKey, &ltHandle ) ) == AI_NO_ERROR )
    {
        if ( ( liRet = coCacheIndex.Del( ltHandle ) ) != AI_NO_ERROR )
        {//del node fail
            return liRet;
        }
    }
    else
    {
        return AI_WARN_NO_FOUND;
    }

    ltHandle = coCacheIndex.GetFreeNode();
    if ( ltHandle == AI_INVALID_NODE_HANDLE )
    {//Can't get a free handle
        return AI_ERROR_NO_ENOUGH_SPACE;
    }

    //write to data file
    int liTimestamp = time(NULL);
    int liIdx = coCacheIndex.GetIdx(ltHandle);// for CacheData
    if ( ( liRet = coCacheData.Write( liIdx, aoKey, aoValue, liTimestamp ) ) != AI_NO_ERROR )
    {
        coCacheIndex.PutFreeNode( ltHandle );
        return liRet;
    }

    //commit to index
    if ( ( liRet = coCacheIndex.Put( ltHandle, aoKey, liTimestamp ) ) != AI_NO_ERROR )
    {
        coCacheIndex.PutFreeNode( ltHandle );
        return liRet;
    }

	return AI_NO_ERROR;
}

int clsBillCache::DelTimeOut( int aiTimestamp, AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp /* = NULL */ )
{
    if ( !cbIsOpened ) return AI_ERROR_UNOPENED; 
    
    // local lock protected
    clsLock loLock( coLock );
    
    int liRet = AI_NO_ERROR;
    
    clsCacheIndex::TNodeHandle ltHandle;
    if ( ( liRet = coCacheIndex.GetTimeOut( aiTimestamp, &ltHandle ) ) != AI_NO_ERROR )
    {
        //Oh, del time out record fail, may i have not you want
        return liRet;
    }
    
    //read from data file
    int liIdx = coCacheIndex.GetIdx(ltHandle);
    if ( ( liRet = coCacheData.Read( liIdx, aoKey, aoValue, apiTimestamp ) ) != AI_NO_ERROR )
    {
        return liRet;
    }
    
    //delete from hash index
    if ( ( liRet = coCacheIndex.Del( ltHandle ) ) != AI_NO_ERROR )
    {
        //oh,delete node fail
        return liRet;
    }
	
	return liRet;
}

TIterator clsBillCache::Begin()
{
    return this->coCacheIndex.Begin();
}

TIterator clsBillCache::End()
{
    return this->coCacheIndex.End();
}

int clsBillCache::Get( TIterator aoIter, AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp )
{
    if ( !cbIsOpened ) return AI_ERROR_UNOPENED; 
    
    // local lock protected
    clsLock loLock( coLock );
    
    int                        liRet = AI_NO_ERROR;
    clsCacheIndex::TNodeHandle ltHandle = this->coCacheIndex.ToHandle(aoIter);
    int                        liIdx    = this->coCacheIndex.GetIdx(ltHandle);
    
    aoKey.Reset();
    aoValue.Reset();
    //read from data file
    if ( ( liRet = this->coCacheData.Read( liIdx, aoKey, aoValue, apiTimestamp ) ) != AI_NO_ERROR )
    {
        return liRet;
    }

	return liRet;
}
	    

void clsBillCache::GetStat( clsCacheStat& aoStat )
{
	coCacheIndex.GetStat( aoStat );
}

bool clsBillCache::IsInvalidKey( const AIChunkEx& aoKey )
{
    if ( aoKey.GetSize() <= 0 || aoKey.GetSize() > AI_MAX_KEY_LEN ) return true;
    
    return false;
}

bool clsBillCache::IsInvalidValue( const AIChunkEx& aoValue )
{
    if ( aoValue.GetSize() <= 0 || aoValue.GetSize() > AI_MAX_VALUE_LEN ) return true;
    
    return false;
}

AI_CACHE_NAMESPACE_END
