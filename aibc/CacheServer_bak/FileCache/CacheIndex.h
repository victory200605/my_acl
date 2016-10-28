
#ifndef __AI_CACHEINDEX_H__
#define __AI_CACHEINDEX_H__     
 
#include "../include/Utility.h"
#include "BitMap.h"

// namespace define 
AI_CACHE_NAMESPACE_START

//hash head, for hash array
struct stHashHead
{
    stHashHead();
	AIBC::LISTHEAD coLinkHead;
};

//File index node, store in single index file
struct stFileIdxNode
{
    stFileIdxNode( size_t aiMaxKeySize );
    
    int Encode( AIBC::AIChunkEx& aoBuffer, int aiTimestamp );
    int Encode( AIBC::AIChunkEx& aoBuffer, int aiTimestamp, const AIBC::AIChunkEx& aoKey );
    int Decode( AIBC::AIChunkEx& aoBuffer, int& aiTimestamp, AIBC::AIChunkEx& aoKey );
    
    void FixKey( AIBC::AIChunkEx& aoBuffer, const AIBC::AIChunkEx& aoKey );
    size_t GetKeyOff();
    size_t GetTimestampOff();
    
    static size_t GetNodeSize( size_t aiMaxKeySize );

    //Attribute
	size_t ciMaxKeySize;
};

//index node, heap array
struct stIndexNode
{
	int ciTimestamp;
	int ciSubKey;
	AIBC::LISTHEAD coLinkChild;
	AIBC::LISTHEAD coTimeChild;
};

// const invalid node handle define
const int AI_INVALID_NODE_HANDLE = -1;

//iterator
class clsCacheIndexIterator
{
public:
    clsCacheIndexIterator( AIBC::LISTHEAD* apoNode );
    
    void operator ++ ();
    bool operator == ( const clsCacheIndexIterator& aoRhs );
    bool operator != ( const clsCacheIndexIterator& aoRhs );
    
    AIBC::LISTHEAD* NodePtr();
    
protected:
    AIBC::LISTHEAD* cpoNode;
};

/////////////////////////////////////////////// clsCacheIndex //////////////////////////////////
class clsCacheIndex
{
public:
    typedef int TNodeHandle;
    typedef clsCacheIndexIterator TIterator;
    
public:
	clsCacheIndex();
	
    ~clsCacheIndex();
    
    //set time point to use quick sort
	void SetTimePoint(int aiTimePoint);
	
	int Create( const char* apsIndexFileName, size_t aiCapacity, size_t aiKeySize );
	
	//load index node from index file
	int Open( const char* apsIndexFileName, size_t aiCapacity, size_t aiKeySize );
	
	void Close();
	
	//////////////////////////////////////////////////////////////////////////
	/// Index operator
	TNodeHandle GetFreeNode();

	//put key to cache index
	int Put( TNodeHandle atHandle, const AIBC::AIChunkEx& aoKey, int aiTimestamp );
	
	int Update( TNodeHandle atHandle, int aiTimestamp );
		    
	//get index handle by key
	int Get( const AIBC::AIChunkEx& aoKey, TNodeHandle* aptHandle );
	
    int Del( TNodeHandle atHandle );
    
    int DelAll();
	
	//get timeout index node
	int GetTimeOut( int aiTimestamp, TNodeHandle* aptHandle );
	/////////////////////////////////////////////////////////////////////////
	
	size_t GetCapacity();
	
	size_t GetSize();
	
	size_t GetCurrAllocPos();

	size_t GetNodeSize();
	
	static size_t GetNodeSize( size_t aiKeySize );

    //poll all index node
	TIterator   Begin();
	TIterator   End();
	TNodeHandle ToHandle( TIterator aoIter );
	
//operator static function
protected:
	static int Compare( const void* apL, const void* apR );

//operator member function
protected:
    void SetCapacity( size_t aiCapacity );
    void SetKeySize( size_t aiKeySize );
    size_t GetKeySize();
    
    void SetFreeNode( TNodeHandle atHandle );
    
    /// Handle to any
    off_t GetOffset( TNodeHandle atHandle );
    stIndexNode* GetNode( TNodeHandle atHandle );
    TNodeHandle GetHandle( stIndexNode* apoIndexNode );
    
    /// Hash link and time out link
    void AddToHashLink( TNodeHandle atHandle, int aiKey, int aiSubKey );
    void AddToTimeLink( TNodeHandle atHandle, bool abTail = true );
    void RemoveFromHashLink( TNodeHandle atHandle );
    void RemoveFromTimeLink( TNodeHandle atHandle );
   
//attribute
protected:
	stHashHead* cpoHashHead;
	stIndexNode* cpoIndexNodes;
	
	//for out of time
	AIBC::AIMutexLock coTimeOutLock;
	AIBC::LISTHEAD coTimeList;
	
	//use to index node allocate, and resource control
	clsBitMap coBitMap;
	
	//cache state variable
	size_t ciSize;
	size_t ciCapacity;
	size_t ciKeySize;
	
	int ciTimePoint;
	
	//index node read/write buffer
	AIBC::AIChunkEx coNodeBuffer;
	AIBC::AIChunkEx coFixKey;

	//access index file
    AIBC::AIFile coFile;
};

AI_CACHE_NAMESPACE_END

#endif // AI_CACHEINDEX_H_2008
