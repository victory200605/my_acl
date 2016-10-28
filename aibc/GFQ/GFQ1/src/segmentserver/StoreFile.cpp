//=============================================================================
/**
* \file    clsStoreFile.cpp
* \brief store file
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: StoreFile.cpp,v 1.1 2010/11/23 06:35:53 daizh Exp $
*
* History
* 2008.06.18 first release by DZH
*/
//=============================================================================

//#include "StoreFile.h"
#include "AITime.h"

//-------------------------------Implement Multi-thread File---------------------------------//
template< class TData >
ssize_t clsStoreFile<TData>::clsFileTrace::Read( off_t atOffset, void* apBuff, size_t aiSize )
{
    ssize_t liResult = 0;
    
    AISmartLock loLock(coLock);
    
    AI_GFQ_TIMETRACE( (liResult = AIFile::Read( atOffset, apBuff, aiSize ) ),
        1,/*level*/
        "FileTrace::Read(%"PRId32", %"PRId64", %"PRId32")", 
        (int32_t)this->GetHandle(),
        (int64_t)atOffset,
        (int32_t)aiSize );
    
    return liResult;
}

template< class TData >
ssize_t clsStoreFile<TData>::clsFileTrace::Write( off_t atOffset, const void* apBuff, size_t aiSize )
{
    ssize_t liResult = 0;
    
    AISmartLock loLock(coLock);
    
    AI_GFQ_TIMETRACE( (liResult = AIFile::Write( atOffset, apBuff, aiSize ) ), 
        1,/*level*/
        "FileTrace::Write(%"PRId32", %"PRId64", %"PRId32")", 
        (int32_t)this->GetHandle(),
        (int64_t)atOffset,
        (int32_t)aiSize );
        
    return liResult;
}

//------------------------------Implement Store File-----------------------------------------//
template< class TData >
clsStoreFile<TData>::clsStoreFile( ) : 
    ciSyncIOTime(0),
    ciCount(0),
    ciSize(0),
    cpoFiles(NULL)
{
}

template< class TData >
clsStoreFile<TData>::~clsStoreFile()
{
    Close();
}

template< class TData >
int32_t clsStoreFile<TData>::GetFileIdx( int32_t aiIdx )
{
    return aiIdx / ciSize; // calculate file index
}

template< class TData >
off_t clsStoreFile<TData>::GetFileOffset( int32_t aiIdx )
{
    return aiIdx % ciSize; // calculate file offset
}

template< class TData >
int32_t clsStoreFile<TData>::Create( const char* apsFileName, int32_t aiCount, size_t aiSize )
{
    if ( aiSize * sizeof(TData) > AI_MAX_S_FILE_SIZE )
    {//sorry, out of range, I can't support it
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    char lcFileName[AI_MAX_FILENAME_LEN];
    memset( lcFileName, 0, AI_MAX_FILENAME_LEN );
    
    AIFile loFile;
    TData loDataNode; // define temporary data node
    memset( &loDataNode, 0, sizeof(TData) );
    
    for( int32_t liIt = 0; liIt < aiCount; liIt++ )
    {
        snprintf( lcFileName, AI_MAX_FILENAME_LEN, "%s%d", apsFileName, liIt );
        if ( loFile.Open( lcFileName, O_RDWR | O_CREAT | O_TRUNC, 0766 ) != AI_NO_ERROR )
        {//open file fail
            return AI_ERROR_OPEN_FILE;
        }
        
        for( size_t liN = 0; liN < aiSize; liN++ )
        {
            if ( loFile.Write( &loDataNode, sizeof(TData) ) != sizeof(TData) )
            {//oh,no please check file system
                return AI_ERROR_WRITE_FILE;
            }
        }
        
        loFile.Close();
    }
    
    return AI_NO_ERROR;
}

template< class TData >
int32_t clsStoreFile<TData>::Open( const char* apsFileName, int32_t aiCount, size_t aiSize )
{
    // Close it frist before open
    Close();
    
    ciCount = aiCount;
    ciSize = aiSize;
    
    // create multi-thread file object
    AI_NEW_ARRAY_ASSERT( cpoFiles, clsFileTrace, aiCount );

    char lcFileName[AI_MAX_FILENAME_LEN];
    memset( lcFileName, 0, AI_MAX_FILENAME_LEN );
    
    for( int32_t liIt = 0; liIt < aiCount; liIt++ )
    {
        snprintf( lcFileName, AI_MAX_FILENAME_LEN, "%s%d", apsFileName, liIt );
        if ( cpoFiles[liIt].Open( lcFileName, O_RDWR ) != AI_NO_ERROR )
        {//open file fail
            AI_DELETE_ARRAY( cpoFiles );
            return AI_ERROR_OPEN_FILE;
        }
    }
    
    return AI_NO_ERROR;
}

template< class TData >
void clsStoreFile<TData>::Close()
{
    if ( cpoFiles == NULL ) return;
        
    for( int32_t liIt = 0; liIt < ciCount; liIt++ )
    {
        cpoFiles[liIt].Close();
    }
    
    AI_DELETE_ARRAY( cpoFiles );
}

template< class TData >
int32_t clsStoreFile<TData>::Read( int32_t aiIdx, TData& aoData )
{
    //Danger, please open frist
    assert( cpoFiles != NULL );
    
    int32_t liFileIdx = GetFileIdx( aiIdx );
    off_t liOffset = GetFileOffset( aiIdx ) * sizeof(TData);
    
    if ( liFileIdx < 0 || liFileIdx >= ciCount )
    {//oh,out of range
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    {// read-write lock scope
        if ( cpoFiles[liFileIdx].Read( liOffset, &aoData, sizeof(TData) ) != sizeof(TData) )
        {
            return AI_ERROR_READ_FILE;
        }
    }
    
	return AI_NO_ERROR;
}

template< class TData >
int32_t clsStoreFile<TData>::Write( int32_t aiIdx, const TData& aoData )
{
    //Danger, please open frist
    assert( cpoFiles != NULL );
    
    int32_t liFileIdx = GetFileIdx( aiIdx );
    off_t liOffset = GetFileOffset( aiIdx ) * sizeof(aoData);
    
    if ( liFileIdx < 0 || liFileIdx >= ciCount )
    {//oh,out of range
        return AI_ERROR_OUT_OF_RANGE;
    }

    {// read-write lock scope
        if ( cpoFiles[liFileIdx].Write( liOffset, &aoData, sizeof(TData) ) != sizeof(TData) )
        {
            return AI_ERROR_WRITE_FILE;
        }
    }
    
	return AI_NO_ERROR;
}
