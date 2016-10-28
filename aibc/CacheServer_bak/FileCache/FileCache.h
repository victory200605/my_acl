
#ifndef __AI_FILECACHE_H__
#define __AI_FILECACHE_H__

#include "CacheData.h"
#include "CacheIndex.h"

AI_CACHE_NAMESPACE_START

// cache iterator type
typedef clsCacheIndex::TIterator TIterator;

//meta data, save index meta info
struct stMetaDataHeader
{
    stMetaDataHeader(
        const char* apcVersion = "", 
        int aiFileCount = 0, 
        int aiNodeCount = 0,
        int aiKeySize = 0,
        int aiValueSize = 0 )
        : ciFileCount(aiFileCount)
        , ciNodeCount(aiNodeCount)
        , ciKeySize(aiKeySize)
        , ciValueSize(aiValueSize) 
    {
        ::snprintf( this->csVersion, sizeof(this->csVersion), "%s", apcVersion );
    }
    
    int Encode( AIBC::AIChunkEx& aoBuffer );
    int Decode( AIBC::AIChunkEx& aoBuffer );
    size_t GetSize();
    
    char csVersion[AI_MAX_VERSION_LEN];
    int  ciFileCount;
    int  ciNodeCount;
    int  ciKeySize;
    int  ciValueSize;
};

class clsFileCache
{
public:
    // local lock
    class clsLock
    {
    public:
        clsLock( AIBC::AIMutexLock& aoLock ) : coLock(aoLock) { coLock.Lock(); }
        ~clsLock( ) { coLock.Unlock(); }
    protected:
        AIBC::AIMutexLock& coLock;
    };
    
public:
	clsFileCache();
	virtual ~clsFileCache();

	void SetTimePoint( int aiTimePoint );
	
	//create bill cache
	int Create( const char* apsDirPath, int aiFileCount, int aiNodeCount, size_t aiKeySize, size_t aiValueSize );
	
	int Open( const char* apsDirPath );
	int Clean();
	void Close();
	
	// put key/value. aiFlag 
	int Put( const AIBC::AIChunkEx& aoKey, const AIBC::AIChunkEx& aoValue, int aiFlag = NO_OVERWRITE );
	int Get( const AIBC::AIChunkEx& aoKey, AIBC::AIChunkEx& aoValue, int aiFlag = F_DELETE );
	int Put( const char* apsKey, const AIBC::AIChunkEx& aoValue, int aiFlag = NO_OVERWRITE );
	int Get( const char* apsKey, AIBC::AIChunkEx& aoValue, int aiFlag = F_DELETE );
	int Update( const AIBC::AIChunkEx& aoKey, const AIBC::AIChunkEx& aoValue );
	int Update( const char* apsKey, const AIBC::AIChunkEx& aoValue );
	int DelTimeOut( int aiTimestamp, AIBC::AIChunkEx& aoKey, AIBC::AIChunkEx& aoValue );
	
	///For CacheServer Iterator
	TIterator Begin();
	TIterator End();
	int Get( TIterator aoIter, AIBC::AIChunkEx& aoKey, AIBC::AIChunkEx& aoValue, int* apiTimestamp = NULL );
	///
	
	//get interface
	size_t GetKeySize()                                   { return this->coMetaDataHeader.ciKeySize; }
	
	size_t GetValueSize()                                 { return this->coMetaDataHeader.ciValueSize; }
	
    size_t GetSize()                                      { return this->coCacheIndex.GetSize(); }
    
    size_t GetCapacity()                                  { return this->coCacheIndex.GetCapacity(); }
    
    size_t GetFileCount()                                 { return this->coCacheData.GetFileCount(); }
    
    size_t GetCurrAllocPos()                              { return this->coCacheIndex.GetCurrAllocPos(); }

protected:
    int  CreateHeaderFile( const char* apcFileName, stMetaDataHeader& aoHeader );
    int  LoadHeader( const char* apcFileName, stMetaDataHeader& aoHeader );

    bool IsInvalidKey( const AIBC::AIChunkEx& aoKey );
    bool IsInvalidValue( const AIBC::AIChunkEx& aoValue );
    
protected:
    clsCacheData     coCacheData;
	clsCacheIndex    coCacheIndex;
	
	stMetaDataHeader coMetaDataHeader;
	
	bool cbIsOpened;
	
	AIBC::AIMutexLock coLock;
};

AI_CACHE_NAMESPACE_END

#endif // AI_BILLCACHE_H_2008
