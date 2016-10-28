
#ifndef __AI_CACHEDATA_H__
#define __AI_CACHEDATA_H__

#include "../include/Utility.h"
#include "FileArrayStorage.h"

AI_CACHE_NAMESPACE_START

struct stDataNodeHeader
{
    stDataNodeHeader();
    
    // Encode data node to buffer
    int Encode( 
        AIBC::AIChunkEx& aoBuffer, 
        const AIBC::AIChunkEx& aoKey, 
        const AIBC::AIChunkEx& aoValue, 
        int aiTimestamp );
    
    // Decode data node from buffer
    int Decode( 
        AIBC::AIChunkEx& aoBuffer, 
        AIBC::AIChunkEx& aoKey, 
        AIBC::AIChunkEx& aoValue );
    
    static size_t GetNodeSize( size_t aiKeySize, size_t aiValueSize );
    
    // Attribute
    int ciKeySize;
	int ciValueSize;
	int ciTimestamp;
};

/////////////////////////////////////////////////////////////////////////////
class clsCacheData : public clsFileArrayStorage
{
public:
    typedef clsFileArrayStorage TBase;

public:
    //Default Constructor
    clsCacheData();
    
    //Default Destructor
    ~clsCacheData();
    
    // Read key/value by array index
    int Read( int aiN, AIBC::AIChunkEx& aoKey, AIBC::AIChunkEx& aoValue, int* apiTimestamp );
    
    // Write key/value by array index
    int Write( int aiN, const AIBC::AIChunkEx& aoKey, const AIBC::AIChunkEx& aoValue, int aiTimestamp );

    // Calculate storage node size by keysize and valuesize
	static size_t GetNodeSize( size_t aiKeySize, size_t aiValueSize );
    
protected:
    // Node buffer for node read and write, ATTENSION:it's not multi-thread safe
    AIBC::AIChunkEx        coNodeBuffer;
    stDataNodeHeader coNodeHeader;
};

AI_CACHE_NAMESPACE_END

#endif // __AI_CACHEDATA_H__
