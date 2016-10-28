//=============================================================================
/**
* \file    clsStoreFile.h
* \brief store file
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: StoreFile.h,v 1.2 2010/12/20 08:19:04 daizh Exp $
*
* History
* 2008.06.18 first release by DZH
*/
//=============================================================================
#ifndef AI_STOREFILE_H_2008
#define AI_STOREFILE_H_2008

#include "gfq/GFQUtility.h"
 
template< class TData >
class clsStoreFile
{
public:
    class clsFileTrace : public AIFile // file for multi-thread
    {
    public:
        ssize_t Read( off_t atOffset, void* apBuff, size_t aiSize );
        ssize_t Write( off_t atOffset, const void* apBuff, size_t aiSize );
    
    protected:
        AIMutexLock coLock;
    };

public:
	clsStoreFile( );
	virtual ~clsStoreFile();
	
	// create aiCount files for storage, and every file can contain aiSize element
	int32_t Create( const char* apsFileName, int32_t aiCount, size_t aiSize );
	
	// open all file, prepare to read and write
    int32_t Open( const char* apsFileName, int32_t aiCount, size_t aiSize );
    
    // set aiIOTime I/O time at single file and must sync to disk
    void SetSyncLimit( int32_t aiIOTime )            { ciSyncIOTime = aiIOTime; }
    
    // close all file 
    void Close();
    
    // do read in aiIdx
	int32_t Read( int32_t aiIdx, TData& aoData );
	
	// do write in aiIdx
	int32_t Write( int32_t aiIdx, const TData& aoData );
	
	size_t GetSize()              { return ciSize; }
	size_t GetCount()             { return ciCount; }

protected:
    int32_t GetFileIdx( int32_t aiIdx );
    off_t GetFileOffset( int32_t aiIdx );
    
protected:
    int32_t ciSyncIOTime;
    
    int32_t ciCount;
    size_t ciSize;
    
    clsFileTrace* cpoFiles;
};

#include "StoreFile.cpp"

#endif // AI_STOREFILE_H_2008
