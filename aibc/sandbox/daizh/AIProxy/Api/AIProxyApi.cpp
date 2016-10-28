
#include "AIProxyApi.h"
#include "AIString.h"

START_AIPROXY_NAMESPACE

////////////////////////////////////////// AIProxyApiChannel ////////////////////////////////////////////////
void AIProxyApiChannel::SetProxyApi( AIProxyApi* apoProxyApi )
{
    this->cpoProxyApi = apoProxyApi;
}

int AIProxyApiChannel::Input( const char* apcData, size_t aiSize )
{
    return this->cpoProxyApi->ChannelInput( apcData, aiSize );
}

int AIProxyApiChannel::Error( int aiErrno )
{
    return this->cpoProxyApi->ChannelError( this, aiErrno );
}

int AIProxyApiChannel::Exit()
{
    return this->cpoProxyApi->ChannelExit( this );
}
    
int AIProxyApiChannel::SendTo( AISocketHandle atHandle, const char* apcData, size_t aiSize )
{
    return this->cpoProxyApi->GetProtocol()->Send( atHandle, apcData, aiSize );
}

int AIProxyApiChannel::RecvFrom( AISocketHandle atHandle, AIChunkEx& aoBuffer, int aiTimeout )
{
    return this->cpoProxyApi->GetProtocol()->Recv( atHandle, aoBuffer, aiTimeout );
}

////////////////////////////////////////// AIProxyApi ////////////////////////////////////////////////
AIProxyApi::stMsgNode::stMsgNode( AIChunkEx* apoBuffer ) : 
    ciReferenceCnt(0),
    cbIsSignal(false),
    cpoBuffer(apoBuffer)
{
}

int AIProxyApi::stMsgNode::Wait( AITime_t aiTimeOut )
{
    int liRetCode = 0;
    
    this->coLock.Lock();
    if ( !cbIsSignal )
    {
        liRetCode = this->coCondition.WaitFor( this->coLock, aiTimeOut );
    }
    this->coLock.Unlock();
    return liRetCode;
}

void AIProxyApi::stMsgNode::Signal()
{
    this->coLock.Lock();
        this->cbIsSignal = true;
    this->coLock.Unlock();
    
    this->coCondition.Signal();
}

void AIProxyApi::stMsgNode::Reference()
{
    this->coLock.Lock();
        this->ciReferenceCnt++;
    this->coLock.Unlock();
}

void AIProxyApi::stMsgNode::Release()
{
    int liReferenceCnt = 0;
    stMsgNode* lpoMsgNode = this;
    
    this->coLock.Lock();
        liReferenceCnt = --this->ciReferenceCnt;
    this->coLock.Unlock();
    if ( liReferenceCnt <= 0 )
    {
        AI_DELETE(lpoMsgNode);
    }
}

AIProxyApi::stApiParam::stApiParam( 
    AIProxyApi* apoProxyApi, stMsgNode* apoMsgNode, uint32_t aiMsgID, const char* apcData, size_t aiSize ) :
    ciMsgID(aiMsgID),
    coData(apcData, aiSize),
    cpoProxyApi(apoProxyApi),
    cpoMsgNode(apoMsgNode)
{
    this->cpoMsgNode->Reference();
}
AIProxyApi::stApiParam::~stApiParam()
{
    this->cpoMsgNode->Release();
}

//////////////////////////////////////////////////////////////////////////////////////
AIProxyApi::AIProxyApi( AIProtocol* apoProtocol /*= NULL*/ ) :
    ciMsgID(0),
    cpoProtocol(apoProtocol),
    cbIsClosed(false),
    ciConnecteTimeOut(10)
{
    if ( this->cpoProtocol == NULL )
    {
        AI_NEW_ASSERT( this->cpoProtocol, AIInnerProtocol );
    }
}

AIProxyApi::~AIProxyApi()
{
    if ( this->cpoProtocol != NULL )
    {
        AI_DELETE( this->cpoProtocol );
    }
}

int AIProxyApi::Initialize()
{
    return this->coTask.Initialize( 1, 100000 );
}

int AIProxyApi::Connect( const char* apcIpAddr, int aiPort, int aiConnCnt, int aiTimeOut )
{
    int liRetCode = 0;
    AIProxyApiChannel* lpoChannel = NULL;
    
    ciConnecteTimeOut = aiTimeOut;
    
    for ( int liN = 0; liN < aiConnCnt; liN++ )
    {
        AI_NEW_ASSERT( lpoChannel, AIProxyApiChannel );
        lpoChannel->SetProxyApi(this);
        if ( ( liRetCode = this->coConnector.Connect( apcIpAddr, aiPort, *lpoChannel, aiTimeOut ) ) != 0 )
        {
            AI_DELETE( lpoChannel );
            return liRetCode;
        }
        lpoChannel->Ready();
        this->coChannelPool.PutChannel( "ProxyClient", lpoChannel );
    }

    return liRetCode;
}

void AIProxyApi::Close()
{
    this->cbIsClosed = true;
    while ( true )
    {
        AIChannelPtr loChannelPtr = this->coChannelPool.GetChannel("ProxyClient");
        if ( loChannelPtr != NULL )
        {
            loChannelPtr->CloseHandle();
        }
        else
        {
            break;
        }
    }
}

size_t AIProxyApi::GetConnectionCnt()
{
    return this->coChannelPool.GetChannelCnt( "ProxyClient" );
}
    
int AIProxyApi::Request( const char* apcData, size_t aiSize )
{
    return this->Request( this->ciMsgID++, apcData, aiSize );
}

int AIProxyApi::Request( uint32_t aiMsgID, const char* apcData, size_t aiSize )
{
    AIChunkEx    loBuffer;
    AIChannelPtr loChannelPtr = this->coChannelPool.GetChannel("ProxyClient");
    AIProtocol::THeader loHeader;
        
    if ( loChannelPtr != NULL )
    {    
        loHeader.SetSequenceNum(aiMsgID);
        loHeader.SetType(AIProtocol::REQUEST);
        loHeader.SetDataPtr(apcData);
        loHeader.SetLength(aiSize);

        this->cpoProtocol->Encode( loHeader, loBuffer );
        
        return loChannelPtr->Send( loBuffer.BasePtr(), loBuffer.GetSize() );
    }
    
    return AI_ERROR_SERVER_NO_FOUND;
}

int AIProxyApi::Request( const char* apcData, size_t aiSize, AIChunkEx& aoResponse, int aiTimeOut )
{
    int         liRetCode = 0;
    uint32_t    liMsgID   = this->ciMsgID++;
    AIChunkEx   loBuffer;
    stMsgNode*  lpoMsgNode = NULL;
    stApiParam* lpoApiParam = NULL;
    TMsgNodeMap::iterator loIter;

    AI_NEW_ASSERT( lpoMsgNode, stMsgNode(&aoResponse) );
    AI_NEW_ASSERT( lpoApiParam, stApiParam( this, lpoMsgNode, liMsgID, apcData, aiSize ) );
    
    /// Insert into msg map
    {
        AISmartLock loLock(this->coLock);
        loIter = this->coMsgNode.insert( TMsgNodeMap::value_type(liMsgID, lpoMsgNode) );
    }
    
    lpoMsgNode->Reference();
    
    do
    {
        if ( this->coTask.Put( RequestHandleThread, lpoApiParam ) != 0 )
        {
            liRetCode = AI_ERROR_NO_ENOUGH_SPACE;
            break;
        }
        
        if ( lpoMsgNode->Wait( AI_TIME_SEC * aiTimeOut ) != 0 )
        {
            liRetCode = AI_ERROR_TIMEOUT;
        }
        else
        {
            liRetCode = lpoMsgNode->ciState;
        }
    }
    while(false);
    
    /// Erase into msg map
    {
        AISmartLock loLock(this->coLock);
        this->coMsgNode.erase( liMsgID );
    }
    
    lpoMsgNode->Release();
    
    return liRetCode;
}

int AIProxyApi::Response( const char* apcData, size_t aiSize )
{
    return this->Request( apcData, aiSize );
}

AIProtocol* AIProxyApi::GetProtocol()
{
    return this->cpoProtocol;
}

int AIProxyApi::InputRequest( const char* apcData, size_t aiSize )
{
    return 0;
}

int AIProxyApi::ChannelInput( const char* apcData, size_t aiSize )
{
    AIProtocol::THeader loHeader;
    stMsgNode*          lpoMsgNode = NULL;
    
    if ( this->cpoProtocol->Decode( apcData, aiSize, loHeader ) != 0 )
    {
        //Error
        return -1;
    }
    
    //printf( "Client recv [MsgID=%s]/[Size=%d]\n", loHeader.csMsgID, loHeader.ciLength );
    
    do
    {
        {
            AISmartLock loLock(this->coLock);
            TMsgNodeMap::iterator loIter = this->coMsgNode.find( loHeader.GetSequenceNum() );
            if ( loIter != this->coMsgNode.end() )
            {
                lpoMsgNode = loIter->second;
                lpoMsgNode->Reference();
                break;
            }
        }
        
        if ( loHeader.GetType() == AIProtocol::REQUEST )
        {
            this->InputRequest( apcData, aiSize );
        }
        
        return 0;
    }
    while( false );
    
    //Set Response
    lpoMsgNode->ciState = 0;
    lpoMsgNode->cpoBuffer->Resize(aiSize);
    ::memcpy( lpoMsgNode->cpoBuffer->BasePtr(), apcData, aiSize );
    lpoMsgNode->Signal();
    lpoMsgNode->Release();
    
    return 0;
}

int AIProxyApi::ChannelError( AIChannelPtr aoChannel, int aiErrno )
{
    aoChannel->CloseHandle();
    
    if ( !this->cbIsClosed )
    {
        this->Connect( aoChannel->GetRemoteIp(), aoChannel->GetRemotePort(), 1, 10 );
    }
    
    return 0;
}

int AIProxyApi::ChannelExit( AIChannelPtr aoChannel )
{
    this->coChannelPool.EraseChannel( "ProxyClient", aoChannel );
    return 0;
}

void* AIProxyApi::RequestHandleThread( void* apvParam )
{
    int         liRetCode = 0;
    stApiParam* lpoParam = static_cast<stApiParam*>( apvParam );
    
    if ( ( liRetCode = lpoParam->cpoProxyApi->Request( lpoParam->ciMsgID, lpoParam->coData.BasePtr(), lpoParam->coData.GetSize() ) ) != 0 )
    {
        lpoParam->cpoMsgNode->ciState = liRetCode;
        lpoParam->cpoMsgNode->Signal();
    }
    
    AI_DELETE( lpoParam );
    
    return NULL;
}

END_AIPROXY_NAMESPACE
