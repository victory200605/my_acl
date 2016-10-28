
#include "FileArrayStorage.h"

AI_CACHE_NAMESPACE_START

clsFileArrayStorage::clsFileArrayStorage( )
    : ciFileCount(0)
    , ciNodeCount(0)
    , ciNodeSize(0)
    , cpoFiles(NULL)
{
}

clsFileArrayStorage::~clsFileArrayStorage()
{
    this->Close();
}

int clsFileArrayStorage::GetFileIdx( int aiN )
{
    return aiN / this->ciNodeCount; // calculate file index
}

off_t clsFileArrayStorage::GetFileOffset( int aiN )
{
    return aiN % this->ciNodeCount; // calculate file offset
}

int clsFileArrayStorage::Create( 
    const char* apsFileName, size_t aiFileCount, size_t aiNodeCount, size_t aiNodeSize )
{
    AIBC::AIFile loFile;
    char      lcFileName[AI_MAX_FILENAME_LEN] = {0};
    AIBC::AIChunkEx loNodeBuffer(aiNodeSize);
    
    if ( aiNodeCount * aiNodeSize > AI_MAX_S_FILE_SIZE )
    {
        //sorry, out of range, I can't support it
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    ::memset( loNodeBuffer.BasePtr(), 0, loNodeBuffer.GetSize() );

    for( size_t liIt = 0; liIt < aiFileCount; liIt++ )
    {
        snprintf( lcFileName, AI_MAX_FILENAME_LEN, "%s%d", apsFileName, liIt );
        if ( loFile.Open( lcFileName, O_RDWR | O_CREAT | O_TRUNC, 0766 ) != AI_NO_ERROR )
        {
            //open file fail
            return AI_ERROR_OPEN_FILE;
        }
        
        for( size_t liN = 0; liN < aiNodeCount; liN++ )
        {
            if ( (size_t)loFile.Write( loNodeBuffer.BasePtr(), loNodeBuffer.GetSize() ) != loNodeBuffer.GetSize() )
            {//oh,no please check file system
                return AI_ERROR_WRITE_FILE;
            }
        }
        
        loFile.Close();
    }
    
    return AI_NO_ERROR;
}

int clsFileArrayStorage::Open( 
    const char* apsFileName, size_t aiFileCount, size_t aiNodeCount, size_t aiNodeSize )
{
    char lcFileName[AI_MAX_FILENAME_LEN] = {0};
    
    // Close it frist before open
    this->Close();
    
    // Assign variable
    this->ciFileCount = aiFileCount;
    this->ciNodeCount = aiNodeCount;
    this->ciNodeSize  = aiNodeSize;
    
    // create multi-thread file object
    AI_NEW_N_ASSERT( this->cpoFiles, AIBC::AIFile, aiFileCount );
    
    for( size_t liN = 0; liN < aiFileCount; liN++ )
    {
        ::snprintf( lcFileName, AI_MAX_FILENAME_LEN, "%s%d", apsFileName, liN );
        if ( this->cpoFiles[liN].Open( lcFileName, O_RDWR ) != AI_NO_ERROR )
        {
            //open file fail
            AI_DELETE_N( cpoFiles );
            AI_CACHE_ERROR( "Open file fail, [Filename=%ld]/[Data], MSG:%s", lcFileName, strerror(errno) );
            return AI_ERROR_OPEN_FILE;
        }
        
        if ( this->cpoFiles[liN].GetSize() != (off_t)(aiNodeCount * aiNodeSize) )
        {
            AI_CACHE_ERROR( "Invalid file fail, [Filename=%ld]/[Filesize=%d]/[Expectsize=%d]/[Data], MSG:%s", 
                lcFileName, this->cpoFiles[liN].GetSize(), aiNodeCount * aiNodeSize, strerror(errno) );
            return AI_ERROR_INVALID_FILE;
        }
    }
    
    return AI_NO_ERROR;
}

void clsFileArrayStorage::Close()
{
    if ( this->cpoFiles == NULL ) return; //Ignore it
        
    for( size_t liN = 0; liN < this->ciFileCount; liN++ )
    {
        this->cpoFiles[liN].Close();
    }
    
    AI_DELETE_N( this->cpoFiles );
}

int clsFileArrayStorage::Read( int aiN, AIBC::AIChunkEx& aoBuffer )
{
    //Danger, please open frist
    assert( this->cpoFiles != NULL );
    
    int   liFileIdx = GetFileIdx( aiN );
    off_t liOffset = GetFileOffset( aiN ) * this->GetNodeSize();
    
    aoBuffer.Resize( this->GetNodeSize() );
    
    if ( liFileIdx < 0 || (size_t)liFileIdx >= this->ciFileCount )
    {
        //oh,out of range
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    {// read-write lock scope
        if ( (size_t)this->cpoFiles[liFileIdx].Read( 
            liOffset, aoBuffer.BasePtr(), this->GetNodeSize() ) != this->GetNodeSize() )
        {
            return AI_ERROR_READ_FILE;
        }
    }
    
	return AI_NO_ERROR;
}

int clsFileArrayStorage::Write( int aiN, const AIBC::AIChunkEx& aoBuffer )
{
    //Danger, please open frist
    assert( this->cpoFiles != NULL );
    
    int   liFileIdx = GetFileIdx( aiN );
    off_t liOffset = GetFileOffset( aiN ) * this->GetNodeSize();
    
    if ( liFileIdx < 0 || (size_t)liFileIdx >= this->ciFileCount )
    {
        //oh,out of range
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    if ( aoBuffer.GetSize() > this->GetNodeSize() )
    {
        return AI_ERROR_OUT_OF_RANGE;
    }

    {// read-write lock scope
        if ( (size_t)cpoFiles[liFileIdx].Write( 
            liOffset, aoBuffer.BasePtr(), aoBuffer.GetSize() ) != aoBuffer.GetSize() )
        {
            return AI_ERROR_WRITE_FILE;
        }
    }

	return AI_NO_ERROR;
}

AI_CACHE_NAMESPACE_END
