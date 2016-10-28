//=============================================================================
/**
* \file    clsStoreFile.h
* \brief store file
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: CacheIndex.h,v 1.1 2011/01/05 08:34:16 daizh Exp $
*
* History
* 2008.06.18 first release by DZH
*/
//=============================================================================
#ifndef AI_CACHEINDEX_H_2008
#define AI_CACHEINDEX_H_2008     

#include "AIFile.h"
#include "AIDualLink.h"
#include "CacheData.h"
#include "BitMap.h"

// namespace define 
AI_CACHE_NAMESPACE_START

//hash head, for hash array
struct stHashHead
{
    stHashHead();
	LISTHEAD coLinkHead;
};
/*
//free list, for node file
struct stFreeListHead
{
    stFreeListHead();
    LISTHEAD coFreeList;
};
*/
//file index node, store in single index file
struct stFileIdxNode
{
    stFileIdxNode();
    
	int ciTimestamp;
	int ciKeySize;
	char csMsgID[AI_MAX_KEY_LEN];
};

//index node, heap array
struct stIndexNode
{
	int ciTimestamp;
	int ciSubKey;
	LISTHEAD coLinkChild;
	LISTHEAD coTimeChild;
};

//meta data, save index meta info
struct stMetaData
{
    stMetaData( int aiCount = 0, size_t aiSize = 0 ) : 
        ciCount(aiCount), ciSize(aiSize) 
    {}
    off_t GetSize()   { return ciCount * ciSize; }
    
    int ciCount;
    size_t ciSize;
};

// const invalid node handle define
const int AI_INVALID_NODE_HANDLE = -1;

// forward
class clsCacheStat;

//iterator
class clsCacheIndexIterator
{
public:
    clsCacheIndexIterator( LISTHEAD* apoNode );
    
    void operator ++ ();
    bool operator == ( const clsCacheIndexIterator& aoRhs );
    bool operator != ( const clsCacheIndexIterator& aoRhs );
    
    LISTHEAD* NodePtr();
    
protected:
    LISTHEAD* cpoNode;
};

//------------------------------------------clsCacheIndex-------------------------------//
class clsCacheIndex
{
public:
    typedef int TNodeHandle;
    typedef clsCacheIndexIterator TIterator;

public:
    typedef clsCacheStat TStat;
    
public:
	clsCacheIndex();
	virtual ~clsCacheIndex();
    
    //set time point to use quick sort
	void SetTimePoint(int aiTimePoint);
	
	int Create( const char* apsIndexFileName, int aiCount, size_t aiSize );
	
	//load index node from index file
	int Open( const char* apsIndexFileName );
	void Close();
	
	TNodeHandle GetFreeNode();
	void PutFreeNode( TNodeHandle atHandle );

	//put key value
	int Put( TNodeHandle atHandle, const AIChunkEx& aoKey, int aiTimestamp );
		    
	//get data node index
	int Get( const AIChunkEx& aoKey, TNodeHandle* aptHandle );
	
    int Del( TNodeHandle atHandle );
    
    int DelAll();
	
	//get time index node
	int GetTimeOut( int aiTimestamp, TNodeHandle* aptHandle );
	
	void GetStat( TStat& aoStat );
    
	int GetIdx( TNodeHandle atHandle );

	TIterator   Begin();
	TIterator   End();
	TNodeHandle ToHandle( TIterator aoIter );
	
//operator static function
protected:
	static int Compare( const void* apL, const void* apR );

//operator member function
protected:
    size_t SubHashString( const char* apsPtr, size_t aiLength );
    size_t HashString( const char* apsPtr, size_t aiLength );
    
    off_t GetOffset( TNodeHandle atHandle );
    stIndexNode* GetNode( TNodeHandle atHandle );
    TNodeHandle GetHandle( stIndexNode* apoIndexNode );
    
    void AddToHashLink( TNodeHandle atHandle, int aiKey, int aiSubKey );
    void AddToTimeLink( TNodeHandle atHandle, bool abTail = true );
    void RemoveFromLink( TNodeHandle atHandle );
   
//attribute
protected:
	stHashHead* cpoHashHead;
	stIndexNode* cpoIndexNodes;
	
	//for out of time
	AIMutexLock coTimeOutLock;
	LISTHEAD coTimeList;
	
	//for free node
	clsBitMap coBitMap;
	
	//cache size
	size_t ciSize;
	
	//index meta data
	stMetaData coMetaData;
	
	int ciTimePoint;
    AIFile coFile;
    
    //lock define
    //AIMutexLockNull coFreeListLock; //null lock, all lock operator were null
    //clsRWMutexNull  coRWLock;
};

//------------------------------------------clsCacheStat--------------------------------//
class clsCacheStat
{
public:
    clsCacheStat() : 
        ciSize(0),
        ciCapacity(0),
        ciMaxCapacity(0),
        ciNodeCount(0),
        ciNodeCapacity(0)
    {}
    //set interface
    void SetSize( size_t aiSize )                       { ciSize = aiSize; }
    void SetCapacity( size_t aiCapacity )               { ciCapacity = aiCapacity; }
    void SetMaxCapacity( size_t aiCapacity )            { ciMaxCapacity = aiCapacity; }
    void SetNodeCount( int aiN )                        { ciNodeCount = aiN; }
    void SetNodeCapacity( size_t aiCapacity )           { ciNodeCapacity = aiCapacity; }
    void SetCurrAllocPoint( int aiAllocPoint )          { ciCurrAllocPoint = aiAllocPoint; }
    
    //get interface
    size_t GetSize( )                                   { return ciSize; }
    size_t GetCapacity( )                               { return ciCapacity; }
    size_t GetMaxCapacity( )                            { return ciMaxCapacity; }
    int GetNodeCount( )                                 { return ciNodeCount; }
    size_t GetNodeCapacity( )                           { return ciNodeCapacity; }
    int GetCurrAllocPoint( )                            { return ciCurrAllocPoint; }
    
private:
    size_t ciSize;
    size_t ciCapacity;
    size_t ciMaxCapacity;
    int    ciNodeCount;
    size_t ciNodeCapacity;
    int    ciCurrAllocPoint;
};

AI_CACHE_NAMESPACE_END

#endif // AI_CACHEINDEX_H_2008
