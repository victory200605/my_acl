//=============================================================================
/**
* \file    clsStoreFile.h
* \brief store file
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: StoreFile.h,v 1.1 2011/01/05 08:34:16 daizh Exp $
*
* History
* 2008.06.18 first release by DZH
*/
//=============================================================================
#ifndef AI_STOREFILE_H_2008
#define AI_STOREFILE_H_2008

#include "AIFile.h"
#include "CacheDefine.h"

AI_CACHE_NAMESPACE_START

template< class TData >
class clsStoreFile
{
public:
    class clsFileAutoSync : public AIFile // file for multi-thread
    {
    public:
        clsFileAutoSync() : ciWriteTime(0) {}
        //Yes, I only care read and write here
        ssize_t Read( off_t atOffset, void* apBuff, size_t aiSize );
        ssize_t Write( off_t atOffset, const void* apBuff, size_t aiSize );
        int Sync( int aiIOTime );
    protected:
        int ciWriteTime; //read & write lock 
    };

public:
	clsStoreFile( );
	virtual ~clsStoreFile();
	
	// create aiCount files for storage, and every file can contain aiSize element
	int Create( const char* apsFileName, int aiCount, size_t aiSize );
	
	// open all file, prepare to read and write
    int Open( const char* apsFileName, int aiCount, size_t aiSize );
    
    // set aiIOTime I/O time at single file and must sync to disk
    void SetSyncLimit( int aiIOTime )            { ciSyncIOTime = aiIOTime; }
    
    // close all file 
    void Close();
    
    // do read in aiIdx
	int Read( int aiIdx, TData& aoData );
	
	// do write in aiIdx
	int Write( int aiIdx, const TData& aoData );
	
	size_t GetSize()              { return ciSize; }
	size_t GetCount()             { return ciCount; }

protected:
    int GetFileIdx( int aiIdx );
    off_t GetFileOffset( int aiIdx );
    
protected:
    int ciSyncIOTime;
    
    int ciCount;
    size_t ciSize;
    
    clsFileAutoSync* cpoFiles;
};

//-------------------------------Implement Multi-thread File---------------------------------//
template< class TData >
ssize_t clsStoreFile<TData>::clsFileAutoSync::Read( off_t atOffset, void* apBuff, size_t aiSize )
{
    return AIFile::Read( atOffset, apBuff, aiSize );
}

template< class TData >
ssize_t clsStoreFile<TData>::clsFileAutoSync::Write( off_t atOffset, const void* apBuff, size_t aiSize )
{
    ciWriteTime++;
    return AIFile::Write( atOffset, apBuff, aiSize );
}

template< class TData >
int clsStoreFile<TData>::clsFileAutoSync::Sync( int aiIOTime )
{
    if ( ciWriteTime < aiIOTime ) return 0;
        
    ciWriteTime = 0;
    
    return AIFile::Sync();
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
int clsStoreFile<TData>::GetFileIdx( int aiIdx )
{
    return aiIdx / ciSize; // calculate file index
}

template< class TData >
off_t clsStoreFile<TData>::GetFileOffset( int aiIdx )
{
    return aiIdx % ciSize; // calculate file offset
}

template< class TData >
int clsStoreFile<TData>::Create( const char* apsFileName, int aiCount, size_t aiSize )
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
    
    for( int liIt = 0; liIt < aiCount; liIt++ )
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
int clsStoreFile<TData>::Open( const char* apsFileName, int aiCount, size_t aiSize )
{
    // Close it frist before open
    Close();
    
    ciCount = aiCount;
    ciSize = aiSize;
    
    // create multi-thread file object
    AI_NEW_ARRAY_ASSERT( cpoFiles, clsFileAutoSync, aiCount );

    char lcFileName[AI_MAX_FILENAME_LEN];
    memset( lcFileName, 0, AI_MAX_FILENAME_LEN );
    
    for( int liIt = 0; liIt < aiCount; liIt++ )
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
        
    for( int liIt = 0; liIt < ciCount; liIt++ )
    {
        cpoFiles[liIt].Close();
    }
    
    AI_DELETE_ARRAY( cpoFiles );
}

template< class TData >
int clsStoreFile<TData>::Read( int aiIdx, TData& aoData )
{
    //Danger, please open frist
    assert( cpoFiles != NULL );
    
    int liFileIdx = GetFileIdx( aiIdx );
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
int clsStoreFile<TData>::Write( int aiIdx, const TData& aoData )
{
    //Danger, please open frist
    assert( cpoFiles != NULL );
    
    int liFileIdx = GetFileIdx( aiIdx );
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

    //sync file time
    if ( ciSyncIOTime > 0 )
    {
        if ( cpoFiles[liFileIdx].Sync(ciSyncIOTime) != 0 )
        {
            return AI_ERROR_SYNC_FILE;
        }
    }

	return AI_NO_ERROR;
}

AI_CACHE_NAMESPACE_END

#endif // AI_STOREFILE_H_2008
