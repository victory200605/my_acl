
#ifndef __AI_CACHE_FILEARRAYSTORAGE_H__
#define __AI_CACHE_FILEARRAYSTORAGE_H__

#include "../include/Utility.h"

AI_CACHE_NAMESPACE_START
 
class clsFileArrayStorage
{
public:
	clsFileArrayStorage( );
	
	~clsFileArrayStorage();
	
	// Create aiFileCount files for storage, 
	// and every file can contain aiNodeCount element
	int Create( 
        const char* apsFileName, 
        size_t aiFileCount,
        size_t aiNodeCount,
        size_t aiNodeSize );
	
	// Open all file, prepare to read and write
    int Open( 
        const char* apsFileName, 
        size_t aiFileCount,
        size_t aiNodeCount,
        size_t aiNodeSize );
    
    // Close all file 
    void Close();
    
    // Read node by index
	int Read( int aiN, AIBC::AIChunkEx& aoBuffer );
	
	// Write node by index
	int Write( int aiN, const AIBC::AIChunkEx& aoBuffer );
	
	size_t GetFileCount()             { return this->ciFileCount; }
	
	size_t GetNodeCount()             { return this->ciNodeCount; }
    
    size_t GetNodeSize()              { return this->ciNodeSize; }
    
protected:
    int GetFileIdx( int aiN );
    
    off_t GetFileOffset( int aiN );
    
protected:
    // Storage node information
    size_t ciFileCount;
    size_t ciNodeCount;
    size_t ciNodeSize;
    
    // File access objects
    AIBC::AIFile* cpoFiles;
};

AI_CACHE_NAMESPACE_END

#endif // __AI_STOREFILE_H__
