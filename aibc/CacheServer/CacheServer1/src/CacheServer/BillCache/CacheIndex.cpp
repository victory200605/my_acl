//=============================================================================
/**
* \file    clsCacheIndex.cpp
* \brief store file
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: CacheIndex.cpp,v 1.1 2011/01/05 08:34:16 daizh Exp $
*
* History
* 2008.06.18 first release by DZH
*/
//=============================================================================
#include "CacheIndex.h"
#include "FileReader.h"

AI_CACHE_NAMESPACE_START

//----------------------------------stHashHead---------------------------------//
stHashHead::stHashHead()
{
    ai_init_list_head( &coLinkHead );
}
/*
//---------------------------------stFreeListHead------------------------------//
stFreeListHead::stFreeListHead()
{
    ai_init_list_head( &coFreeList );
}
*/
//----------------------------------stFileIdxNode---------------------------------//
stFileIdxNode::stFileIdxNode()
{
    memset( this, 0, sizeof(stFileIdxNode) );
}

//----------------------------------clsCacheIndexIterator---------------------------------//
clsCacheIndexIterator::clsCacheIndexIterator( LISTHEAD* apoNode )
    : cpoNode(apoNode)
{
}

void clsCacheIndexIterator::operator ++ ()
{
    if ( this->cpoNode != NULL )
    {
        this->cpoNode = this->cpoNode->cpNext;
    }
}

bool clsCacheIndexIterator::operator == ( const clsCacheIndexIterator& aoRhs )
{
    return this->cpoNode == aoRhs.cpoNode;
}

bool clsCacheIndexIterator::operator != ( const clsCacheIndexIterator& aoRhs )
{
    return this->cpoNode != aoRhs.cpoNode;
}

LISTHEAD* clsCacheIndexIterator::NodePtr()
{
    return this->cpoNode;
}

//----------------------------------clsCacheIndex------------------------------//
clsCacheIndex::clsCacheIndex() :
    cpoHashHead(NULL),
	cpoIndexNodes(NULL),
//	ciCurrListIdx(0),
	ciSize(0),
	ciTimePoint(time(NULL) - 3600)
{
    //initialize hash node
    AI_BC_NEW_ARRAY_ASSERT( cpoHashHead, stHashHead, AI_HASH_SIZE, cache_index_malloc_hash );
    
    ai_init_list_head( &coTimeList );
}



clsCacheIndex::~clsCacheIndex()
{
    AI_DELETE_ARRAY( cpoHashHead );
    
    Close();
}

size_t clsCacheIndex::HashString( const char* apcString, size_t aiLength )
{
	//unsigned long llH = 0; 
	unsigned int	liRetValue = 1;
	unsigned int	liTempValue = 4;

	while( aiLength-- ) /* from MySQL */
	{ 
		liRetValue ^= (((liRetValue & 63)+liTempValue)*((unsigned int)(unsigned char) *apcString++))+ (liRetValue << 8);
		liTempValue += 3; 
	} 
	
	return liRetValue; 
}

size_t clsCacheIndex::SubHashString( const char* apsPtr, size_t aiLength )
{
	const char *lpsEnd = apsPtr + aiLength; 
	unsigned int loHash; 
	for ( loHash = 0; apsPtr < lpsEnd; apsPtr++ ) 
	{ 
		loHash *= 16777619; 
		loHash ^= (unsigned int) *(unsigned char*) apsPtr; 
	} 
	return (loHash); 
}

void clsCacheIndex::SetTimePoint( int aiTimePoint )
{
    ciTimePoint = aiTimePoint;
}

int clsCacheIndex::Create( const char* apsIndexFileName, int aiCount, size_t aiSize )
{
    stMetaData loMetaData( aiCount, aiSize);
    off_t liFileSize = loMetaData.GetSize() * sizeof(stFileIdxNode) + sizeof(stMetaData);
    if (  liFileSize > (off_t)AI_MAX_S_FILE_SIZE )
    {//sorry, out of range, I can't support it
#if defined(AI_ENABLE_BILLCACHE_LOG)
        AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
		    "Create index fail, fact file size = %ld / support file size = %ld", liFileSize, AI_MAX_S_FILE_SIZE );
#endif
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    AIFile loFile;
    if ( loFile.Open( apsIndexFileName, O_RDWR | O_CREAT | O_TRUNC, 0766 ) != AI_NO_ERROR )
    {//Can't not open file, are you sure the path is right
#if defined(AI_ENABLE_BILLCACHE_LOG)
        AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
		    "Create index, Open file %s fail, MSG:%s", apsIndexFileName, strerror(errno) );
#endif
        return AI_ERROR_OPEN_FILE;
    }

    if ( loFile.Write( &loMetaData, sizeof(stMetaData) ) != sizeof(stMetaData) )
    {//oh,no. please check file system
#if defined(AI_ENABLE_BILLCACHE_LOG)
        AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
		    "Create index, Write file %s fail, MSG:%s", apsIndexFileName, strerror(errno) );
#endif
        return AI_ERROR_WRITE_FILE;
    }
    
    stFileIdxNode loFileIdxNode;
    memset( &loFileIdxNode, 0, sizeof(stFileIdxNode) );
    size_t liSize = loMetaData.GetSize();
    for( size_t liN = 0; liN < liSize; liN++ )
    {
        if ( loFile.Write( &loFileIdxNode, sizeof(stFileIdxNode) ) != sizeof(stFileIdxNode) )
        {//oh,no. please check file system
#if defined(AI_ENABLE_BILLCACHE_LOG)
            AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
		        "Create index, Write file %s fail, MSG:%s", apsIndexFileName, strerror(errno) );
#endif
            return AI_ERROR_WRITE_FILE;
        }
    }
    loFile.Close();
    return AI_NO_ERROR;
}

int clsCacheIndex::Compare( const void* apL, const void* apR )
{//for stIndexNode timestamp sort
    stIndexNode* lpoLNode = *(stIndexNode**)apL;
    stIndexNode* lpoRNode = *(stIndexNode**)apR;
    return lpoRNode->ciTimestamp - lpoLNode->ciTimestamp;
}

int clsCacheIndex::Open( const char* apsIndexFileName )
{
    //Close it first before open
    Close();
    
    if ( coFile.Open( apsIndexFileName, O_RDWR ) != AI_NO_ERROR )
    {//oh,open file fail 
#if defined(AI_ENABLE_BILLCACHE_LOG)
        AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
		    "Open index, Open file %s fail, MSG:%s", apsIndexFileName, strerror(errno) );
#endif
        return AI_ERROR_OPEN_FILE;
    }

    if ( coFile.Read( 0, &coMetaData, sizeof(stMetaData) ) != sizeof(stMetaData) )
    {//oh,read file fail
#if defined(AI_ENABLE_BILLCACHE_LOG)
        AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
		    "Open index, Read file %s meta data fail, MSG:%s", apsIndexFileName, strerror(errno) );
#endif
        return AI_ERROR_READ_FILE;
    }
    
    size_t liSize = coMetaData.GetSize();
    off_t liExpectSize = sizeof(stMetaData) + sizeof(stFileIdxNode) * liSize;
    if ( coFile.GetSize() != liExpectSize )
    {//oh,may be the file was destroyed
#if defined(AI_ENABLE_BILLCACHE_LOG)
        AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
		    "Open index, Invalid file %s, fact file size = %ld / expect file size = %ld, MSG:%s", 
		    apsIndexFileName, coFile.GetSize(), liExpectSize, strerror(errno) );
#endif
        return AI_ERROR_INVALID_FILE;
    }
    
    // Create bitmap
    coBitMap.Create(liSize);

    // Malloc index node
    AI_BC_NEW_ARRAY_ASSERT( cpoIndexNodes, stIndexNode, liSize, cache_index_malloc_index_node );
    stIndexNode** lppoTmpTimeLink = NULL;//for timestamp sort
    AI_BC_NEW_ARRAY_ASSERT( lppoTmpTimeLink, stIndexNode*, liSize, cache_index_malloc_tmp_time_link_for_sort );
    stFileIdxNode loFileIdxNode;
    int liActiveNodeCnt = 0;

    clsFileReader<stFileIdxNode, 2000> loFileReader( coFile, sizeof(stMetaData) );
    for( size_t liIt = 0; liIt < liSize; liIt++ )
    {
        if ( loFileReader.Read( &loFileIdxNode ) != sizeof(stFileIdxNode) )
        {//oh,read file fail
#if defined(AI_ENABLE_BILLCACHE_LOG)
            AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
		        "Open index, Read file %s from buffer fail, MSG:%s", apsIndexFileName, strerror(errno) );
#endif
            Close();
            AI_DELETE_ARRAY( lppoTmpTimeLink );
            return AI_ERROR_READ_FILE;
        }

        cpoIndexNodes[liIt].ciTimestamp = loFileIdxNode.ciTimestamp;
        if ( cpoIndexNodes[liIt].ciTimestamp <= 0 )
        {
            PutFreeNode( GetHandle( &cpoIndexNodes[liIt] ) );
        }
        else
        {
            if ( cpoIndexNodes[liIt].ciTimestamp > ciTimePoint )
            {
                //tail to time link
                AddToTimeLink( GetHandle( &cpoIndexNodes[liIt] ) );
            }
            else
            {
                //add to temporary time link, for sort in [OO, TimePoint]
                lppoTmpTimeLink[liActiveNodeCnt++] = cpoIndexNodes + liIt;
            }
            AddToHashLink( GetHandle( &cpoIndexNodes[liIt] ), 
                HashString( loFileIdxNode.csMsgID, loFileIdxNode.ciKeySize) % AI_HASH_SIZE,
                SubHashString( loFileIdxNode.csMsgID, loFileIdxNode.ciKeySize) % AI_HASH_SIZE );
        }
    }
    
    // find free first
    coBitMap.Alloc();
    
    //sort by timestamp
    qsort( lppoTmpTimeLink, liActiveNodeCnt, sizeof(stIndexNode*), clsCacheIndex::Compare );
    for( int liTIt = 0; liTIt < liActiveNodeCnt; liTIt++ )
    {
        //tail to time link
        AddToTimeLink( GetHandle( lppoTmpTimeLink[liTIt] ), false );
    }
    
    AI_DELETE_ARRAY( lppoTmpTimeLink );
	
	return AI_NO_ERROR;
}

void clsCacheIndex::Close()
{
    if ( cpoIndexNodes == NULL ) return;
    
    AI_DELETE_ARRAY( cpoIndexNodes );
}

clsCacheIndex::TNodeHandle clsCacheIndex::GetFreeNode()
{
    //start lock scope
    //coFreeListLock.Lock();
    
    //free list head
    TNodeHandle liHandle = coBitMap.Alloc();
    if ( liHandle < 0 ) return AI_INVALID_NODE_HANDLE;
    
    return liHandle;
}

void clsCacheIndex::PutFreeNode( TNodeHandle atHandle )
{
    //invalid node handle, danger
    ASSERT( put_free_node_valid_handle, atHandle != AI_INVALID_NODE_HANDLE );
    
    //start lock scope
    //coFreeListLock.Lock();
    GetNode(atHandle)->ciSubKey = 0;
    
    GetNode(atHandle)->ciTimestamp = 0;

    //end lock scope
	//coFreeListLock.UnLock();
}

void clsCacheIndex::AddToHashLink( TNodeHandle atHandle, int aiKey, int aiSubKey )
{
    //invalid node handle, danger
    ASSERT( add_to_hash_link_valid_handle, atHandle != AI_INVALID_NODE_HANDLE );
    
    GetNode(atHandle)->ciSubKey = aiSubKey;
    
    ai_list_add_tail( &GetNode(atHandle)->coLinkChild, &cpoHashHead[aiKey].coLinkHead );
    ciSize++;
    coBitMap.SetUsing( GetIdx(atHandle) );
}

void clsCacheIndex::AddToTimeLink( TNodeHandle atHandle, bool abTail /* = true */ )
{
    //invalid node handle, danger
    ASSERT( add_to_time_link_valid_handle, atHandle != AI_INVALID_NODE_HANDLE );

    if ( abTail )
    {
        ai_list_add_tail( &GetNode(atHandle)->coTimeChild, &coTimeList );
    }
    else
    {
        ai_list_add_head( &GetNode(atHandle)->coTimeChild, &coTimeList );
    }
}

void clsCacheIndex::RemoveFromLink( TNodeHandle atHandle )
{
    //invalid node handle, danger
    ASSERT( add_to_time_link_valid_handle, atHandle != AI_INVALID_NODE_HANDLE );
    
    //Do for time list
    GetNode(atHandle)->coTimeChild.cpPrev->cpNext = GetNode(atHandle)->coTimeChild.cpNext;
    GetNode(atHandle)->coTimeChild.cpNext->cpPrev = GetNode(atHandle)->coTimeChild.cpPrev;
    
    //Do for hash link
    GetNode(atHandle)->coLinkChild.cpPrev->cpNext = GetNode(atHandle)->coLinkChild.cpNext;
    GetNode(atHandle)->coLinkChild.cpNext->cpPrev = GetNode(atHandle)->coLinkChild.cpPrev;
    ciSize--;
    coBitMap.SetFree( GetIdx(atHandle) );
}

stIndexNode* clsCacheIndex::GetNode( TNodeHandle atHandle )
{
    ASSERT( index_nodes_array_is_null, cpoIndexNodes != NULL );
    
    return cpoIndexNodes + atHandle;
}

clsCacheIndex::TNodeHandle clsCacheIndex::GetHandle( stIndexNode* apoIndexNode )
{
    ASSERT( index_nodes_array_is_null, cpoIndexNodes != NULL );
    
    return apoIndexNode - cpoIndexNodes;
}

int clsCacheIndex::GetIdx( TNodeHandle atHandle )
{
    return atHandle;
}

off_t clsCacheIndex::GetOffset( TNodeHandle atHandle )
{
    return sizeof(stMetaData) + GetIdx( atHandle ) * sizeof(stFileIdxNode);
}

int clsCacheIndex::Put( clsCacheIndex::TNodeHandle atHandle, const AIChunkEx& aoKey, int aiTimestamp )
{
    //invalid node handle, danger
    ASSERT( put_valid_handle, atHandle != AI_INVALID_NODE_HANDLE );
    
    if ( aoKey.GetSize() > AI_MAX_KEY_LEN )
    {
        //out of range
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    stFileIdxNode loFileIdxNode;
    
    loFileIdxNode.ciTimestamp = aiTimestamp;
    loFileIdxNode.ciKeySize = aoKey.GetSize();
    memcpy( loFileIdxNode.csMsgID, aoKey.BasePtr(), aoKey.GetSize() );
    
    {//writer lock scope
        //clsRWMutex::Writer loWrite( coRWLock );
    
        if ( coFile.Write( GetOffset(atHandle), &loFileIdxNode, sizeof(stFileIdxNode) ) !=
             sizeof(stFileIdxNode) )
        {//oh,write file fail
#if defined(AI_ENABLE_BILLCACHE_LOG)
            AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
		        "Put record, Write file fail, record offset = %ld, MSG:%s", GetOffset(atHandle), strerror(errno) );
#endif
            return AI_ERROR_WRITE_FILE;
        }
    
        //add to time link and hash table
        GetNode(atHandle)->ciTimestamp = aiTimestamp;
        AddToHashLink( atHandle, HashString(loFileIdxNode.csMsgID, loFileIdxNode.ciKeySize) % AI_HASH_SIZE,
                       SubHashString(loFileIdxNode.csMsgID, loFileIdxNode.ciKeySize) % AI_HASH_SIZE );
        AddToTimeLink( atHandle );
    }
    
	return AI_NO_ERROR;
}

int clsCacheIndex::Get( const AIChunkEx& aoKey, TNodeHandle* aptHandle )
{
    stFileIdxNode loFileIdxNode;
    stIndexNode* lpoIndexNode = NULL;
    int liKey = HashString( aoKey.BasePtr(), aoKey.GetSize() ) % AI_HASH_SIZE;
    int liSubKey = SubHashString( aoKey.BasePtr(), aoKey.GetSize() ) % AI_HASH_SIZE;
    
    // Define for trace
    int liCount = 0;
    
    {//reader lock scope
        //clsRWMutex::Reader loRead( coRWLock );
    
        LISTHEAD* lpoHead = cpoHashHead[liKey].coLinkHead.cpNext;
        for ( ; lpoHead != NULL && lpoHead != &cpoHashHead[liKey].coLinkHead; lpoHead = lpoHead->cpNext )
        {
	        lpoIndexNode = AI_GET_STRUCT_PTR( lpoHead , stIndexNode, coLinkChild );
	        
	        if ( liSubKey != lpoIndexNode->ciSubKey )
	        {
	            continue;
	        }
	        
            if ( coFile.Read( GetOffset( GetHandle(lpoIndexNode) ), &loFileIdxNode, sizeof(stFileIdxNode) ) !=
                 sizeof(stFileIdxNode) )
            {//oh,read file fail
                //printf( "Hit = %d\n", ( sliLen > liCount ? sliLen : sliLen = liCount ) );
#if defined(AI_ENABLE_BILLCACHE_LOG)
                AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
		            "Get record, Read file fail, record offset = %ld, MSG:%s", GetOffset( GetHandle(lpoIndexNode) ), strerror(errno) );
#endif
                return AI_ERROR_READ_FILE;
            }
            
            if ( strncmp( loFileIdxNode.csMsgID, aoKey.BasePtr(), aoKey.GetSize() ) == 0 )
            {
                *aptHandle = GetHandle(lpoIndexNode);
                
                if ( liCount != 0 )
                {
                    AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_DEBUG, "Get record, Compare count = %d", liCount );
                }
                
                return AI_NO_ERROR;
            }
            
            // Do for trace
            liCount++;
        }
        
        //sorry, can't find it
    }
    
    return AI_WARN_NO_FOUND;
}

int clsCacheIndex::Del( TNodeHandle atHandle )
{   
    //invalid node handle, danger
    ASSERT( del_valid_handle, atHandle != AI_INVALID_NODE_HANDLE );
     
    int liTimestamp = 0;
    
    //writer lock scope
    //clsRWMutex::Writer loWrite( coRWLock );
            
    if ( GetNode(atHandle)->ciTimestamp == 0 )
    {//oh,can't delete it again
        return AI_ERROR_DEL_FREE_NODE;
    }
    
    if ( coFile.Write( GetOffset( atHandle ) + AI_OFFSET(stFileIdxNode, ciTimestamp) , 
         &liTimestamp, sizeof(int) ) != sizeof(int) )
    {//oh,write file fail
#if defined(AI_ENABLE_BILLCACHE_LOG)
        AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
		    "Del record, Write file fail, record offset = %ld, MSG:%s", 
		    GetOffset( atHandle ) + AI_OFFSET(stFileIdxNode, ciTimestamp), strerror(errno) );
#endif
        return AI_ERROR_WRITE_FILE;
    }
    
    RemoveFromLink( atHandle );
    
    PutFreeNode( atHandle );

    return AI_NO_ERROR;
}

int clsCacheIndex::DelAll()
{
    int liRet = AI_NO_ERROR;
    while( !ai_list_is_empty(&coTimeList) )
    {
        LISTHEAD* lpoHead = coTimeList.cpNext;

        //get node pointer
	    stIndexNode* lpoIndexNode = AI_GET_STRUCT_PTR( lpoHead , stIndexNode, coTimeChild );
	    if ( ( liRet = Del( GetHandle(lpoIndexNode) ) ) != AI_NO_ERROR )
	    {
	        return liRet;
	    }
    }
    
    return AI_NO_ERROR;
}

int clsCacheIndex::GetTimeOut( int aiTimestamp, TNodeHandle* aptHandle )
{
    //read lock scope
    //clsRWMutex::Reader loRead( coRWLock );
    
    LISTHEAD* lpoHead = coTimeList.cpNext;
    stIndexNode* lpoIndexNode = NULL;
    int liRet = AI_WARN_NO_FOUND;
    
    //poll time list to find time out node
    for ( ; lpoHead != NULL && lpoHead != &coTimeList; lpoHead = lpoHead->cpNext )
    {
        //get node pointer
	    lpoIndexNode = AI_GET_STRUCT_PTR( lpoHead , stIndexNode, coTimeChild );
	    
	    if ( lpoIndexNode->ciTimestamp > aiTimestamp )
	    {
	        // Sorry, can't find it
	        liRet = AI_WARN_NO_FOUND;
	        break;
	    }
	    
	    *aptHandle = GetHandle(lpoIndexNode);
        liRet = AI_NO_ERROR;
        break;
    }
    
	return liRet;
}

clsCacheIndex::TIterator clsCacheIndex::Begin()
{
    return TIterator( this->coTimeList.cpNext );
}

clsCacheIndex::TIterator clsCacheIndex::End()
{
    return TIterator( &this->coTimeList );
}

clsCacheIndex::TNodeHandle clsCacheIndex::ToHandle( TIterator aoIter )
{
    stIndexNode* lpoIndexNode = NULL;
    LISTHEAD*    lpoNode      = aoIter.NodePtr();
    
    lpoIndexNode = AI_GET_STRUCT_PTR( lpoNode , stIndexNode, coTimeChild );
    
    return this->GetHandle(lpoIndexNode);
}

void clsCacheIndex::GetStat( clsCacheStat& aoStat )
{
    aoStat.SetSize( ciSize );
    aoStat.SetCapacity( coMetaData.ciCount * coMetaData.ciSize );
    aoStat.SetMaxCapacity( ( AI_MAX_S_FILE_SIZE - sizeof(stMetaData) ) / sizeof(stFileIdxNode) );
    aoStat.SetNodeCount( coMetaData.ciCount );
    aoStat.SetNodeCapacity( coMetaData.ciSize );
    aoStat.SetCurrAllocPoint( coBitMap.GetCurrAllocPoint() );
}

AI_CACHE_NAMESPACE_END
