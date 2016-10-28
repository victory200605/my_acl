
#include "CacheIndex.h"

AI_CACHE_NAMESPACE_START

//////////////////////////////////// stHashHead ///////////////////////////////////
stHashHead::stHashHead()
{
    AIBC::ai_init_list_head( &coLinkHead );
}

////////////////////////////////////stFileIdxNode////////////////////////////////
stFileIdxNode::stFileIdxNode( size_t aiMaxKeySize )
    : ciMaxKeySize(aiMaxKeySize)
{
}

int stFileIdxNode::Encode( AIBC::AIChunkEx& aoBuffer, int aiTimestamp )
{
    AIBC::AIChunkExOut loOutput(aoBuffer);
    
    aoBuffer.Reset();
    aoBuffer.Resize(sizeof(int));
    
    loOutput.PutNInt32(aiTimestamp);
    
    return 0;
}

int stFileIdxNode::Encode( AIBC::AIChunkEx& aoBuffer, int aiTimestamp, const AIBC::AIChunkEx& aoKey )
{
    AIBC::AIChunkExOut loOutput(aoBuffer);
    
    aoBuffer.Reset();
    aoBuffer.Resize( stFileIdxNode::GetNodeSize(this->ciMaxKeySize) );
    
    AI_RETURN_IF( -1,  ( aoKey.GetSize() > this->ciMaxKeySize ) );
    
    loOutput.PutNInt32(aiTimestamp);
    loOutput.PutMem( aoKey.BasePtr(), aoKey.GetSize() );
    
    ::memset( aoBuffer.WritePtr(), 0, aoBuffer.GetSize() - (sizeof(int) + aoKey.GetSize()) );
    
    return 0;
}

int stFileIdxNode::Decode( AIBC::AIChunkEx& aoBuffer, int& aiTimestamp, AIBC::AIChunkEx& aoKey )
{
    AIBC::AIChunkExIn loInput(aoBuffer);
    
    aoBuffer.Reset();
    aoBuffer.WritePtr( aoBuffer.GetSize() );
    aoKey.Resize( this->ciMaxKeySize );
    
    loInput.ToHInt32( aiTimestamp );
    loInput.ToMem( aoKey.BasePtr(), this->ciMaxKeySize );
    
    return 0;
}

void stFileIdxNode::FixKey( AIBC::AIChunkEx& aoBuffer, const AIBC::AIChunkEx& aoKey )
{
    aoBuffer.Resize( this->ciMaxKeySize );
    
    if ( aoKey.GetSize() >= this->ciMaxKeySize )
    {
        ::memcpy( aoBuffer.BasePtr(), aoKey.BasePtr(), this->ciMaxKeySize );
    }
    else
    {
        ::memcpy( aoBuffer.BasePtr(), aoKey.BasePtr(), aoKey.GetSize() );
        ::memset( aoBuffer.BasePtr() + aoKey.GetSize(), 0, aoBuffer.GetSize() - aoKey.GetSize() );
    }
}

size_t stFileIdxNode::GetKeyOff()
{
    return sizeof(int);
}

size_t stFileIdxNode::GetTimestampOff()
{
    return 0;
}
    
size_t stFileIdxNode::GetNodeSize( size_t aiMaxKeySize )
{
    return sizeof(int) + aiMaxKeySize;
}

////////////////////////////////////clsCacheIndexIterator///////////////////////////
clsCacheIndexIterator::clsCacheIndexIterator( AIBC::LISTHEAD* apoNode )
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

AIBC::LISTHEAD* clsCacheIndexIterator::NodePtr()
{
    return this->cpoNode;
}

////////////////////////////////////clsCacheIndex////////////////////////////--//
clsCacheIndex::clsCacheIndex() 
    : cpoHashHead(NULL)
	, cpoIndexNodes(NULL)
	, ciSize(0)
	, ciCapacity(0)
	, ciKeySize(0)
	, ciTimePoint(time(NULL) - 3600)
{
    //initialize hash node
    AI_NEW_N_ASSERT( this->cpoHashHead, stHashHead, AI_HASH_SIZE );
    
    AIBC::ai_init_list_head( &this->coTimeList );
}



clsCacheIndex::~clsCacheIndex()
{
    AI_DELETE_N( cpoHashHead );
    
    this->Close();
}

void clsCacheIndex::SetTimePoint( int aiTimePoint )
{
    this->ciTimePoint = aiTimePoint;
}

int clsCacheIndex::Create( const char* apsIndexFileName, size_t aiCapacity, size_t aiKeySize )
{
    AIBC::AIFile    loFile;
    AIBC::AIChunkEx loBuffer( this->GetNodeSize(aiKeySize) );
    
    // Create Bitmap
    char lcBitMapName[AI_MAX_FILENAME_LEN];
    snprintf( lcBitMapName, AI_MAX_FILENAME_LEN, "%s.bm", apsIndexFileName );
    if ( this->coBitMap.Create( lcBitMapName, aiCapacity ) != AI_NO_ERROR )
    {
		return AI_ERROR_OPEN_FILE;
    }
    
    if ( loFile.Open( apsIndexFileName, O_RDWR | O_CREAT | O_TRUNC, 0766 ) != AI_NO_ERROR )
    {
        //Can't not open file, are you sure the path is right
        return AI_ERROR_OPEN_FILE;
    }
    
    ::memset( loBuffer.BasePtr(), 0, loBuffer.GetSize() );

    for( size_t liN = 0; liN < aiCapacity; liN++ )
    {
        if ( (size_t)loFile.Write( loBuffer.BasePtr(), loBuffer.GetSize() ) != loBuffer.GetSize() )
        {
            //oh,no. please check file system
            return AI_ERROR_WRITE_FILE;
        }
    }
    
    loFile.Close();
    
    return AI_NO_ERROR;
}

int clsCacheIndex::Open( const char* apsIndexFileName, size_t aiCapacity, size_t aiKeySize )
{
    int           liActiveNodeCnt = 0;
    int           liTimestamp = 0;
    AIBC::AIChunkEx     loKey(aiKeySize);
    stIndexNode** lppoTmpTimeLink = NULL;//for timestamp sort
    stFileIdxNode loFileIdxNode(aiKeySize);
    size_t        liNodeSize   = clsCacheIndex::GetNodeSize(aiKeySize);
    off_t         liExpectSize = liNodeSize * aiCapacity;
    char          lcBitMapName[AI_MAX_FILENAME_LEN] = {0};
    
    // Resize buffer
    this->coNodeBuffer.Resize( liNodeSize );
    this->coFixKey.Resize( aiKeySize );
    this->SetKeySize( aiKeySize );
    this->SetCapacity( aiCapacity );
    
    // Open Bitmap file
    ::snprintf( lcBitMapName, AI_MAX_FILENAME_LEN, "%s.bm", apsIndexFileName );
    if ( this->coBitMap.Open( lcBitMapName ) != AI_NO_ERROR )
    {
		return AI_ERROR_MAP_FILE;
    }
    
    if ( this->coBitMap.GetSize() != aiCapacity )
    {
        return AI_ERROR_INVALID_FILE;
    }
    
    //// Open index file
    if ( this->coFile.Open( apsIndexFileName, O_RDWR ) != AI_NO_ERROR )
    {
        //oh,open file fail 
        return AI_ERROR_OPEN_FILE;
    }

    if ( this->coFile.GetSize() != liExpectSize )
    {
        //oh,may be the file was destroyed
        return AI_ERROR_INVALID_FILE;
    }
    
    // Malloc index node
    AI_DELETE_N( this->cpoIndexNodes );
    AI_NEW_N_ASSERT( this->cpoIndexNodes, stIndexNode, aiCapacity );
    AI_NEW_N_ASSERT( lppoTmpTimeLink, stIndexNode*, aiCapacity );
    
    for( size_t liIt = 0; liIt < aiCapacity; liIt++ )
    {
        if ( !this->coBitMap.IsUsing( liIt ) )
    	{ 
    		this->SetFreeNode( this->GetHandle( &this->cpoIndexNodes[liIt] ) );
    		continue;
    	}
    	
    	off_t liOff = this->GetOffset( this->GetHandle( &this->cpoIndexNodes[liIt] ) );
    	if ( (size_t)this->coFile.Read( liOff, this->coNodeBuffer.BasePtr(), liNodeSize ) != liNodeSize )
        {
            //oh,read file fail
            AI_DELETE_N( this->cpoIndexNodes );
            AI_DELETE_N( lppoTmpTimeLink );
            
            return AI_ERROR_READ_FILE;
        }
        
        assert( loFileIdxNode.Decode( this->coNodeBuffer, liTimestamp, loKey ) == 0 );

        this->cpoIndexNodes[liIt].ciTimestamp = liTimestamp;
        if ( this->cpoIndexNodes[liIt].ciTimestamp <= 0 )
        {
            //alignment bitmap
			this->coBitMap.SetFree(liIt);
			
            this->SetFreeNode( this->GetHandle( &cpoIndexNodes[liIt] ) );
        }
        else
        {
            if ( this->cpoIndexNodes[liIt].ciTimestamp > ciTimePoint )
            {
                //tail to time link
                this->AddToTimeLink( this->GetHandle( &cpoIndexNodes[liIt] ) );
            }
            else
            {
                //add to temporary time link, for sort in [OO, TimePoint]
                lppoTmpTimeLink[liActiveNodeCnt++] = cpoIndexNodes + liIt;
            }
            
            this->AddToHashLink( 
                this->GetHandle( &cpoIndexNodes[liIt] ), 
                HashString( loKey.BasePtr(), loKey.GetSize() ) % AI_HASH_SIZE,
                SubHashString( loKey.BasePtr(), loKey.GetSize() ) % AI_HASH_SIZE );
            
            // Allocated node, add size and set bitmap
            this->ciSize++;
        }
    }
    
    // prefind free pos first
    this->coBitMap.AlignUsingSize(this->ciSize);
    
    //sort by timestamp
    ::qsort( lppoTmpTimeLink, liActiveNodeCnt, sizeof(stIndexNode*), clsCacheIndex::Compare );
    for( int liTIt = 0; liTIt < liActiveNodeCnt; liTIt++ )
    {
        //add to time link front
        this->AddToTimeLink( GetHandle( lppoTmpTimeLink[liTIt] ), false );
    }
    
    AI_DELETE_N( lppoTmpTimeLink );
	
	return AI_NO_ERROR;
}

void clsCacheIndex::Close()
{
    AI_DELETE_N( this->cpoIndexNodes );
}

clsCacheIndex::TNodeHandle clsCacheIndex::GetFreeNode()
{
    //free list head
    TNodeHandle liHandle = coBitMap.GetFree(); // Node index is handle
    return liHandle < 0 ? AI_INVALID_NODE_HANDLE : liHandle;
}

int clsCacheIndex::Put( clsCacheIndex::TNodeHandle atHandle, const AIBC::AIChunkEx& aoKey, int aiTimestamp )
{
    //invalid node handle, danger
    assert( atHandle != AI_INVALID_NODE_HANDLE );
    assert( aoKey.GetSize() <= this->GetKeySize() );
    
    char*         lpcKeyPtr = NULL;
    stFileIdxNode loFileIdxNode(this->GetKeySize());

    assert( loFileIdxNode.Encode( this->coNodeBuffer, aiTimestamp, aoKey ) == 0 );
    
    lpcKeyPtr = this->coNodeBuffer.BasePtr() + loFileIdxNode.GetKeyOff();
    
    {//writer lock scope    
        if ( (size_t)this->coFile.Write( 
                this->GetOffset(atHandle), 
                this->coNodeBuffer.BasePtr(), 
                this->GetNodeSize() ) != this->GetNodeSize() )
        {
            //oh,write file fail
            AI_CACHE_ERROR( "Put record, Write file fail, [Offset=%ld]/[Index], MSG:%s", this->GetOffset(atHandle), strerror(errno) );
            return AI_ERROR_WRITE_FILE;
        }
    
        //add to time link and hash table
        this->GetNode(atHandle)->ciTimestamp = aiTimestamp;
        this->AddToHashLink( 
            atHandle, 
            HashString( lpcKeyPtr, this->GetKeySize() ) % AI_HASH_SIZE,
            SubHashString( lpcKeyPtr, this->GetKeySize() ) % AI_HASH_SIZE );
        AddToTimeLink( atHandle );
        
        // Allocated node, add size and set bitmap
        this->ciSize++;
        this->coBitMap.Alloc( atHandle );
        assert( this->coBitMap.GetUsingSize() == this->ciSize );
    }
    
	return AI_NO_ERROR;
}

int clsCacheIndex::Update( TNodeHandle atHandle, int aiTimestamp )
{
    //invalid node handle, danger
    assert( atHandle != AI_INVALID_NODE_HANDLE );
     
    stFileIdxNode loFileIdxNode(this->GetKeySize());

    assert( loFileIdxNode.Encode( this->coNodeBuffer, aiTimestamp ) == 0 );
    assert( this->coNodeBuffer.GetSize() == sizeof(int) );
    
    if ( (size_t)this->coFile.Write( 
            this->GetOffset( atHandle ) + loFileIdxNode.GetTimestampOff(), 
            this->coNodeBuffer.BasePtr(), this->coNodeBuffer.GetSize() ) != this->coNodeBuffer.GetSize() )
    {
        //oh,write file fail
        AI_CACHE_ERROR( "Update record timestamp fail, [Offset=%ld]/[Index], MSG:%s", 
            this->GetOffset(atHandle) + loFileIdxNode.GetTimestampOff(), strerror(errno) );
        return AI_ERROR_WRITE_FILE;
    }
    
    this->GetNode(atHandle)->ciTimestamp = aiTimestamp;
    
    this->RemoveFromTimeLink( atHandle );
    this->AddToTimeLink( atHandle );
    
    return AI_NO_ERROR;
}

int clsCacheIndex::Get( const AIBC::AIChunkEx& aoKey, TNodeHandle* aptHandle )
{
    assert( aoKey.GetSize() <= this->GetKeySize() );
    
    int           liKey        = 0;
    int           liSubKey     = 0;
    stIndexNode*  lpoIndexNode = NULL;
    stFileIdxNode loFileIdxNode(this->GetKeySize());
    
    loFileIdxNode.FixKey( this->coFixKey, aoKey );

    liKey     = HashString( this->coFixKey.BasePtr(), this->coFixKey.GetSize() ) % AI_HASH_SIZE;
    liSubKey  = SubHashString( this->coFixKey.BasePtr(), this->coFixKey.GetSize() ) % AI_HASH_SIZE;

    {//reader lock scope    
        AIBC::LISTHEAD* lpoHead = this->cpoHashHead[liKey].coLinkHead.cpNext;
        for ( ; lpoHead != NULL && lpoHead != &cpoHashHead[liKey].coLinkHead; lpoHead = lpoHead->cpNext )
        {
	        lpoIndexNode = AI_GET_STRUCT_PTR( lpoHead , stIndexNode, coLinkChild );
	        
	        if ( liSubKey != lpoIndexNode->ciSubKey )
	        {
	            continue;
	        }
	        
            if ( this->coFile.Read( 
                    this->GetOffset( GetHandle(lpoIndexNode) ), 
                    this->coNodeBuffer.BasePtr(), 
                    this->GetNodeSize() ) != (ssize_t)this->GetNodeSize() )
            {
                //oh,read file fail
                AI_CACHE_ERROR( "Get record, Read file fail, [Offset=%ld]/[Index], MSG:%s", 
                    this->GetOffset( GetHandle(lpoIndexNode) ), strerror(errno) );
                return AI_ERROR_READ_FILE;
            }
            
            char* lpcKeyPtr = this->coNodeBuffer.BasePtr() + loFileIdxNode.GetKeyOff();
            
            if ( memcmp( lpcKeyPtr, this->coFixKey.BasePtr(), this->GetKeySize() ) == 0 )
            {
                *aptHandle = this->GetHandle(lpoIndexNode);

                return AI_NO_ERROR;
            }
        }
        
        //sorry, can't find it
    }
    
    return AI_WARN_NO_FOUND;
}

int clsCacheIndex::Del( TNodeHandle atHandle )
{   
    //invalid node handle, danger
    assert( atHandle != AI_INVALID_NODE_HANDLE );

    if ( this->GetNode(atHandle)->ciTimestamp == 0 )
    {
        //oh,can't delete it again
        return AI_ERROR_DEL_FREE_NODE;
    }
   
    this->RemoveFromHashLink( atHandle );
    this->RemoveFromTimeLink( atHandle );
    
    this->SetFreeNode( atHandle );
    
    /// Remove hash node complete
    this->ciSize--;
    this->coBitMap.Dealloc( atHandle );
    assert( this->coBitMap.GetUsingSize() == this->ciSize );

    return AI_NO_ERROR;
}

int clsCacheIndex::DelAll()
{
    int liRet = AI_NO_ERROR;
    while( !ai_list_is_empty(&coTimeList) )
    {
        AIBC::LISTHEAD* lpoHead = this->coTimeList.cpNext;

        //get node pointer
	    stIndexNode* lpoIndexNode = AI_GET_STRUCT_PTR( lpoHead , stIndexNode, coTimeChild );
	    if ( ( liRet = this->Del( GetHandle(lpoIndexNode) ) ) != AI_NO_ERROR )
	    {
	        return liRet;
	    }
    }
    
    return AI_NO_ERROR;
}

int clsCacheIndex::GetTimeOut( int aiTimestamp, TNodeHandle* aptHandle )
{
    AIBC::LISTHEAD* lpoHead = coTimeList.cpNext;
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
	    
	    *aptHandle = this->GetHandle(lpoIndexNode);
        liRet = AI_NO_ERROR;
        break;
    }
    
	return liRet;
}

void clsCacheIndex::SetFreeNode( TNodeHandle atHandle )
{
    //invalid node handle, danger
    assert( atHandle != AI_INVALID_NODE_HANDLE );
    
    this->GetNode(atHandle)->ciSubKey = 0;
    this->GetNode(atHandle)->ciTimestamp = 0;
}

int clsCacheIndex::Compare( const void* apL, const void* apR )
{
    //for stIndexNode timestamp sort
    stIndexNode* lpoLNode = *(stIndexNode**)apL;
    stIndexNode* lpoRNode = *(stIndexNode**)apR;
    return lpoRNode->ciTimestamp - lpoLNode->ciTimestamp;
}

void clsCacheIndex::AddToHashLink( TNodeHandle atHandle, int aiKey, int aiSubKey )
{
    //invalid node handle, danger
    assert( atHandle != AI_INVALID_NODE_HANDLE );
    
    this->GetNode(atHandle)->ciSubKey = aiSubKey;
    
    ai_list_add_tail( &GetNode(atHandle)->coLinkChild, &cpoHashHead[aiKey].coLinkHead );
}

void clsCacheIndex::AddToTimeLink( TNodeHandle atHandle, bool abTail /* = true */ )
{
    //invalid node handle, danger
    assert( atHandle != AI_INVALID_NODE_HANDLE );

    if ( abTail )
    {
        ai_list_add_tail( &GetNode(atHandle)->coTimeChild, &coTimeList );
    }
    else
    {
        ai_list_add_head( &GetNode(atHandle)->coTimeChild, &coTimeList );
    }
}

void clsCacheIndex::RemoveFromHashLink( TNodeHandle atHandle )
{
    //invalid node handle, danger
    assert( atHandle != AI_INVALID_NODE_HANDLE );
    
    //Do for hash link
    this->GetNode(atHandle)->coLinkChild.cpPrev->cpNext = GetNode(atHandle)->coLinkChild.cpNext;
    this->GetNode(atHandle)->coLinkChild.cpNext->cpPrev = GetNode(atHandle)->coLinkChild.cpPrev;
}

void clsCacheIndex::RemoveFromTimeLink( TNodeHandle atHandle )
{
    //invalid node handle, danger
    assert( atHandle != AI_INVALID_NODE_HANDLE );
    
    //Do for time list
    this->GetNode(atHandle)->coTimeChild.cpPrev->cpNext = GetNode(atHandle)->coTimeChild.cpNext;
    this->GetNode(atHandle)->coTimeChild.cpNext->cpPrev = GetNode(atHandle)->coTimeChild.cpPrev;
}

stIndexNode* clsCacheIndex::GetNode( TNodeHandle atHandle )
{
    assert( cpoIndexNodes != NULL );
    
    return this->cpoIndexNodes + atHandle;
}

clsCacheIndex::TNodeHandle clsCacheIndex::GetHandle( stIndexNode* apoIndexNode )
{
    assert( cpoIndexNodes != NULL );
    
    return apoIndexNode - this->cpoIndexNodes;
}

size_t clsCacheIndex::GetCapacity()
{
    return this->ciCapacity;
}
	
size_t clsCacheIndex::GetSize()
{
    return this->ciSize;
}

size_t clsCacheIndex::GetCurrAllocPos()
{
    return this->coBitMap.GetCurrAllocPos();
}

size_t clsCacheIndex::GetNodeSize()
{
    return stFileIdxNode::GetNodeSize(this->GetKeySize());
}

size_t clsCacheIndex::GetNodeSize( size_t aiKeySize )
{
    return stFileIdxNode::GetNodeSize(aiKeySize);
}

off_t clsCacheIndex::GetOffset( TNodeHandle atHandle )
{
    return atHandle * this->GetNodeSize();
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
    AIBC::LISTHEAD*    lpoNode      = aoIter.NodePtr();
    
    lpoIndexNode = AI_GET_STRUCT_PTR( lpoNode , stIndexNode, coTimeChild );
    
    return this->GetHandle(lpoIndexNode);
}

void clsCacheIndex::SetCapacity( size_t aiCapacity )
{
    this->ciCapacity = aiCapacity;
}

void clsCacheIndex::SetKeySize( size_t aiKeySize )
{
    this->ciKeySize = aiKeySize;
}

size_t clsCacheIndex::GetKeySize()
{
    return this->ciKeySize;
}

AI_CACHE_NAMESPACE_END
