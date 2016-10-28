
#include "FileCache.h"

AI_CACHE_NAMESPACE_START

const char* const BCVERSION = "v01.00.001";

/**
 * Align value 'x' to boundary 'b' which should be power of 2
 * !get from fastdb stdtp.h
 */
#define DOALIGN(x,b)   (((x) + (b) - 1) & ~((b) - 1))

int stMetaDataHeader::Encode( AIBC::AIChunkEx& aoBuffer )
{
    AIBC::AIChunkExOut loOutput(aoBuffer);
    
    loOutput.PutMem( this->csVersion, sizeof(this->csVersion) );
    loOutput.PutNInt32( this->ciFileCount );
    loOutput.PutNInt32( this->ciNodeCount );
    loOutput.PutNInt32( this->ciKeySize );
    loOutput.PutNInt32( this->ciValueSize );
    
    return 0;
}

int stMetaDataHeader::Decode( AIBC::AIChunkEx& aoBuffer )
{
    AIBC::AIChunkExIn loInput(aoBuffer);
    
    loInput.ToMem( this->csVersion, sizeof(this->csVersion) );
    loInput.ToHInt32( this->ciFileCount );
    loInput.ToHInt32( this->ciNodeCount );
    loInput.ToHInt32( this->ciKeySize );
    loInput.ToHInt32( this->ciValueSize );
    
    return 0;
}

size_t stMetaDataHeader::GetSize()
{
    return sizeof(int)*4 + sizeof(this->csVersion);
}

////////////////////////////////////////////////////////////////////////
clsFileCache::clsFileCache() 
    : cbIsOpened (false)
{
}

clsFileCache::~clsFileCache()
{
    this->Close( );
}

void clsFileCache::SetTimePoint( int aiTimePoint )
{
    this->coCacheIndex.SetTimePoint( aiTimePoint );
}

int clsFileCache::Create( 
    const char* apsDirPath, 
    int aiFileCount, 
    int aiNodeCount, 
    size_t aiKeySize, 
    size_t aiValueSize )
{
    int  liRetCode = 0;
    char lcTmpFileName[AI_MAX_FILENAME_LEN] = {0};
    
    AIBC::AIDir::Make(apsDirPath);
        
    aiKeySize   = DOALIGN(aiKeySize, 8);
    aiValueSize = DOALIGN(aiValueSize, 8);

    //Do for header
    stMetaDataHeader loHeaderMeta( BCVERSION, aiFileCount, aiNodeCount, aiKeySize, aiValueSize );
    snprintf( lcTmpFileName, AI_MAX_FILENAME_LEN, "%s/__Header", apsDirPath );
    if ( ( liRetCode = this->CreateHeaderFile( lcTmpFileName, loHeaderMeta ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }
    
    //Do for index, load
    snprintf( lcTmpFileName, AI_MAX_FILENAME_LEN, "%s/__Index", apsDirPath );
    if ( ( liRetCode = coCacheIndex.Create( 
        lcTmpFileName, 
        loHeaderMeta.ciFileCount * loHeaderMeta.ciNodeCount,
        loHeaderMeta.ciKeySize ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }
    
    //Do for store data file
    snprintf( lcTmpFileName, AI_MAX_FILENAME_LEN, "%s/__data", apsDirPath );
    if ( ( liRetCode = coCacheData.Create( 
        lcTmpFileName, 
        loHeaderMeta.ciFileCount,
        loHeaderMeta.ciNodeCount,
        clsCacheData::GetNodeSize( loHeaderMeta.ciKeySize, loHeaderMeta.ciValueSize ) ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }
    
	return AI_NO_ERROR;
}

int clsFileCache::Open( const char* apsDirPath )
{
    char lcTmpFileName[AI_MAX_FILENAME_LEN] = {0};
    int  liRetCode = 0;
    
    if ( this->cbIsOpened ) this->Close();

    //Do for header
    snprintf( lcTmpFileName, AI_MAX_FILENAME_LEN, "%s/__Header", apsDirPath );
    if ( ( liRetCode = this->LoadHeader( lcTmpFileName, this->coMetaDataHeader ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }
    
    //Do for index, load
    snprintf( lcTmpFileName, AI_MAX_FILENAME_LEN, "%s/__Index", apsDirPath );
    if ( ( liRetCode = coCacheIndex.Open( 
        lcTmpFileName, 
        this->coMetaDataHeader.ciFileCount * this->coMetaDataHeader.ciNodeCount,
        this->coMetaDataHeader.ciKeySize ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }
    
    //Do for store data file
    snprintf( lcTmpFileName, AI_MAX_FILENAME_LEN, "%s/__data", apsDirPath );
    if ( ( liRetCode = coCacheData.Open( 
        lcTmpFileName,
        this->coMetaDataHeader.ciFileCount,
        this->coMetaDataHeader.ciNodeCount,
        clsCacheData::GetNodeSize( this->coMetaDataHeader.ciKeySize, this->coMetaDataHeader.ciValueSize ) ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }
    
    cbIsOpened = true;
    
	return AI_NO_ERROR;
}

int clsFileCache::CreateHeaderFile( const char* apcFileName, stMetaDataHeader& aoHeader )
{
    AIBC::AIFile    loFile;
    AIBC::AIChunkEx loBuffer(0);
    
    off_t liFileSize = aoHeader.ciFileCount * aoHeader.ciNodeCount * clsCacheIndex::GetNodeSize(aoHeader.ciKeySize);
    if (  liFileSize > (off_t)AI_MAX_S_FILE_SIZE )
    {
        //sorry, out of range, I can't support it
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    if ( (size_t)aoHeader.ciKeySize > AI_MAX_KEY_LEN || (size_t)aoHeader.ciValueSize > AI_MAX_VALUE_LEN )
    {
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    aoHeader.Encode(loBuffer);

    if ( loFile.Open( apcFileName, O_RDWR | O_CREAT | O_TRUNC, 0766 ) != AI_NO_ERROR )
    {
        //Can't not open file, are you sure the path is right
        return AI_ERROR_OPEN_FILE;
    }

    if ( (size_t)loFile.Write( loBuffer.BasePtr(), loBuffer.GetSize() ) != loBuffer.GetSize() )
    {
        //Oh,no. please check file system
        return AI_ERROR_WRITE_FILE;
    }
    
    return 0;
}

int clsFileCache::LoadHeader( const char* apcFileName, stMetaDataHeader& aoHeader )
{
    AIBC::AIFile    loFile;
    AIBC::AIChunkEx loBuffer(aoHeader.GetSize());

    if ( loFile.Open( apcFileName, O_RDONLY, 0766 ) != AI_NO_ERROR )
    {
        //Can't not open file, are you sure the path is right
        return AI_ERROR_OPEN_FILE;
    }

    if ( (size_t)loFile.Read( loBuffer.BasePtr(), loBuffer.GetSize() ) != loBuffer.GetSize() )
    {
        //Oh,no. please check file system
        return AI_ERROR_READ_FILE;
    }
    
    loBuffer.WritePtr( loBuffer.GetSize() );
    
    aoHeader.Decode(loBuffer);
    off_t liFileSize = aoHeader.ciFileCount * aoHeader.ciNodeCount * clsCacheIndex::GetNodeSize(aoHeader.ciKeySize);
    if (  liFileSize > (off_t)AI_MAX_S_FILE_SIZE )
    {
        return AI_ERROR_INVALID_FILE;
    }
    
    if ( (size_t)aoHeader.ciKeySize > AI_MAX_KEY_LEN || (size_t)aoHeader.ciValueSize > AI_MAX_VALUE_LEN )
    {
        return AI_ERROR_INVALID_FILE;
    }
    
    if ( ::strcmp( aoHeader.csVersion, BCVERSION ) != 0 )
    {
        return AI_ERROR_UNCOMPATIBLE_VERSION;
    }
    
    return 0;
}

int clsFileCache::Clean( )
{
    if ( !this->cbIsOpened ) return AI_ERROR_UNOPENED;
    
    // local lock protected
    clsLock loLock( coLock );
        
    return this->coCacheIndex.DelAll();
}

void clsFileCache::Close( )
{
    // local lock protected
    clsLock loLock( coLock );
        
    this->coCacheData.Close();
    this->coCacheIndex.Close();
}

int clsFileCache::Put( const AIBC::AIChunkEx& aoKey, const AIBC::AIChunkEx& aoValue, int aiFlag /*= NO_OVERWRITE*/ )
{
    int liRetCode   = 0;
    int liTimestamp = 0;
    clsCacheIndex::TNodeHandle ltHandle = AI_INVALID_NODE_HANDLE;
    
    if ( !this->cbIsOpened ) return AI_ERROR_UNOPENED;
    
    if ( this->IsInvalidKey( aoKey ) || this->IsInvalidValue( aoValue ) )
    {
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    // local lock protected
    clsLock loLock( coLock );

    //try to check duple record
    if ( ( liRetCode = this->coCacheIndex.Get( aoKey, &ltHandle ) ) == AI_NO_ERROR )
    {
        //replace node?
        if ( aiFlag == OVERWRITE )
        {
            if ( ( liRetCode = this->coCacheIndex.Del( ltHandle ) ) != AI_NO_ERROR )
            {//del node fail
                return liRetCode;
            }
        }
        else
        {
            //duple record
            return AI_ERROR_DUP_RECORD;
        }
    }

    if ( ( ltHandle = this->coCacheIndex.GetFreeNode() ) == AI_INVALID_NODE_HANDLE )
    {
        //Can't get a free handle
        return AI_ERROR_NO_ENOUGH_SPACE;
    }

    //write to data file
    liTimestamp = time(NULL);
    if ( ( liRetCode = this->coCacheData.Write( ltHandle, aoKey, aoValue, liTimestamp ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }

    //commit to index
    if ( ( liRetCode = this->coCacheIndex.Put( ltHandle, aoKey, liTimestamp ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }

	return AI_NO_ERROR;
}

int clsFileCache::Update( const AIBC::AIChunkEx& aoKey, const AIBC::AIChunkEx& aoValue )
{
    int liRetCode   = 0;
    int liTimestamp = 0;
    clsCacheIndex::TNodeHandle ltHandle = AI_INVALID_NODE_HANDLE;
    
    if ( !this->cbIsOpened ) return AI_ERROR_UNOPENED;
    
    if ( this->IsInvalidKey( aoKey ) || this->IsInvalidValue( aoValue ) )
    {
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    // local lock protected
    clsLock loLock( coLock );

    //try to check duple record
    if ( ( liRetCode = this->coCacheIndex.Get( aoKey, &ltHandle ) ) == AI_NO_ERROR )
    {
        //Del first
        if ( ( liRetCode = this->coCacheIndex.Del( ltHandle ) ) != AI_NO_ERROR )
        {//del node fail
            return liRetCode;
        }
    }
    else
    {
        return AI_WARN_NO_FOUND;
    }

    if ( ( ltHandle = this->coCacheIndex.GetFreeNode() ) == AI_INVALID_NODE_HANDLE )
    {
        //Can't get a free handle
        return AI_ERROR_NO_ENOUGH_SPACE;
    }

    //write to data file
    liTimestamp = time(NULL);
    if ( ( liRetCode = this->coCacheData.Write( ltHandle, aoKey, aoValue, liTimestamp ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }

    //commit to index
    if ( ( liRetCode = this->coCacheIndex.Put( ltHandle, aoKey, liTimestamp ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }

	return AI_NO_ERROR;
}

int clsFileCache::Update( const char* apsKey, const AIBC::AIChunkEx& aoValue )
{
    AIBC::AIChunkEx loKey( apsKey, strlen(apsKey) + 1 );

    return this->Update( loKey, aoValue );
}

int clsFileCache::Get( const AIBC::AIChunkEx& aoKey, AIBC::AIChunkEx& aoValue, int aiFlag )
{
    static AIBC::AIChunkEx loTmpKey( this->GetKeySize() ); //Ignore it
    int liRetCode = AI_NO_ERROR;
    clsCacheIndex::TNodeHandle ltHandle = AI_INVALID_NODE_HANDLE;

    if ( !this->cbIsOpened ) return AI_ERROR_UNOPENED;
    
    if ( this->IsInvalidKey( aoKey ) )
    {
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    aoValue.Reset();
    
    // local lock protected
    clsLock loLock( coLock );
    
    if ( ( liRetCode = this->coCacheIndex.Get( aoKey, &ltHandle ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }
    
    //read from data file
    if ( ( liRetCode = this->coCacheData.Read( ltHandle, loTmpKey, aoValue, NULL ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }
    
    if ( aiFlag == F_DELETE )
    {
        //delete from hash index
        if ( ( liRetCode = this->coCacheIndex.Del( ltHandle ) ) != AI_NO_ERROR )
        {
            //oh,delete node fail
            return liRetCode;
        }
    }
    else
    {
        if ( ( liRetCode = this->coCacheIndex.Update( ltHandle,  time(NULL) ) ) != AI_NO_ERROR )
        {
            //oh,update node timestamp fail
            return liRetCode;
        }
    }
    
	return AI_NO_ERROR;
}

int clsFileCache::Put( const char* apsKey, const AIBC::AIChunkEx& aoValue, int aiFlag /*= NO_OVERWRITE*/ )
{
    AIBC::AIChunkEx loKey( apsKey, strlen(apsKey) + 1 );

    return this->Put( loKey, aoValue, aiFlag );
}

int clsFileCache::Get( const char* apsKey, AIBC::AIChunkEx& aoValue, int aiFlag )
{
    AIBC::AIChunkEx loKey( apsKey, strlen(apsKey) + 1 );
    
    return this->Get( loKey, aoValue, aiFlag );
}

int clsFileCache::DelTimeOut( int aiTimestamp, AIBC::AIChunkEx& aoKey, AIBC::AIChunkEx& aoValue )
{
    int liRetCode = AI_NO_ERROR;
    clsCacheIndex::TNodeHandle ltHandle = AI_INVALID_NODE_HANDLE;
    
    if ( !this->cbIsOpened ) return AI_ERROR_UNOPENED;
        
    aoValue.Reset();
    
    // local lock protected
    clsLock loLock( coLock );

    if ( ( liRetCode = this->coCacheIndex.GetTimeOut( aiTimestamp, &ltHandle ) ) != AI_NO_ERROR )
    {
        //Oh, del time out record fail, may i have not you want
        return liRetCode;
    }
    
    //read from data file
    if ( ( liRetCode = this->coCacheData.Read( ltHandle, aoKey, aoValue, NULL ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }
    
    //delete from hash index
    if ( ( liRetCode = this->coCacheIndex.Del( ltHandle ) ) != AI_NO_ERROR )
    {
        //oh,delete node fail
        return liRetCode;
    }
	
	return liRetCode;
}

TIterator clsFileCache::Begin()
{
    return this->coCacheIndex.Begin();
}

TIterator clsFileCache::End()
{
    return this->coCacheIndex.End();
}

int clsFileCache::Get( TIterator aoIter, AIBC::AIChunkEx& aoKey, AIBC::AIChunkEx& aoValue, int* apiTimestamp )
{
    if ( !this->cbIsOpened ) return AI_ERROR_UNOPENED; 
    
    // local lock protected
    clsLock loLock( coLock );
    
    int liRetCode = AI_NO_ERROR;
    clsCacheIndex::TNodeHandle ltHandle = this->coCacheIndex.ToHandle(aoIter);
    
    aoKey.Reset();
    aoValue.Reset();
    
    //read from data file
    if ( ( liRetCode = this->coCacheData.Read( ltHandle, aoKey, aoValue, apiTimestamp ) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }

	return liRetCode;
}

bool clsFileCache::IsInvalidKey( const AIBC::AIChunkEx& aoKey )
{
    if ( aoKey.GetSize() <= 0 || aoKey.GetSize() > this->GetKeySize() ) return true;
    
    return false;
}

bool clsFileCache::IsInvalidValue( const AIBC::AIChunkEx& aoValue )
{
    if ( aoValue.GetSize() <= 0 || aoValue.GetSize() > this->GetValueSize() ) return true;
    
    return false;
}

AI_CACHE_NAMESPACE_END
