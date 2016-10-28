
#include "ScpServer.h"
#include "ScpProtocol.h"

int clsScpProtocol::Decode( const char* apcData, size_t aiSize, AIProxy::AIProtocol::THeader& aoHeader )
{
    AI_STD::string  loStrData( apcData, aiSize );
    
    ///Search field CSeq
    size_t liFirst = loStrData.find( "CSeq:" );
    AI_RETURN_IF( -1, liFirst == AI_STD::string::npos );
    
    size_t liLast  = loStrData.find( "\r\n", liFirst );
    AI_RETURN_IF( -1, liLast  == AI_STD::string::npos );
    
    aoHeader.SetSequenceNum( atoll(loStrData.substr(liFirst + 5, liLast-liFirst-5).c_str()) );
    aoHeader.SetLength( aiSize );
    aoHeader.SetDataPtr( apcData );
    
    if ( aiSize > 11 && ::strncasecmp( apcData + 4, "REQUEST", 7 ) == 0 )
    {
        aoHeader.SetType( AIProxy::AIProtocol::REQUEST );
    }
    else
    {
        aoHeader.SetType( AIProxy::AIProtocol::RESPONSE );
    }
    
    return 0;
}

int clsScpProtocol::Encode( const AIProxy::AIProtocol::THeader& aoHeader, AIChunkEx& aoBuffer )
{
    int liNewLength = 0;
    AI_STD::string  loStrData( aoHeader.GetDataPtr(), aoHeader.GetLength() );
    char            lsSequenceNum[AIProxy::AI_MAX_MSGID_LEN] = {0};
    AIChunkExOut    loOutput(aoBuffer);
    
    aoBuffer.Reset();
    aoBuffer.Resize(0);
    
    ::snprintf( lsSequenceNum, sizeof(lsSequenceNum), "%u", aoHeader.GetSequenceNum() );
    
    ///Repace field CSeq
    size_t liFirst = loStrData.find( "CSeq:" );
    AI_RETURN_IF( -1, liFirst == AI_STD::string::npos );
    size_t liLast  = loStrData.find( "\r\n", liFirst );
    AI_RETURN_IF( -1, liLast  == AI_STD::string::npos );
    
    loOutput.PutMem( aoHeader.GetDataPtr(),  liFirst + 5 );
    loOutput.PutMem( lsSequenceNum, strlen(lsSequenceNum) );
    loOutput.PutMem( aoHeader.GetDataPtr() + liLast, aoHeader.GetLength() - liLast );
    
    liNewLength = aoBuffer.GetSize();
    liNewLength = htonl(liNewLength);
    ::memcpy( aoBuffer.BasePtr(), &liNewLength, sizeof(liNewLength) );
    
    return 0;
}

int clsScpProtocol::Send( AISocketHandle aoHandle, const char* apcData, size_t aiSize )
{
    HANDLE_WRITE( aoHandle, apcData, aiSize, 10 );
    return 0;
}

int clsScpProtocol::Recv( AISocketHandle aoHandle, AIChunkEx& aoBuffer, int aiTimeOut )
{
    int liLength = 0;
    
    aoBuffer.Reset();
    aoBuffer.Resize( sizeof(liLength) );
    
    HANDLE_READ( aoHandle, aoBuffer.BasePtr(), aoBuffer.GetSize(), aiTimeOut );
    aoBuffer.WritePtr( sizeof(liLength) );
    
    ::memcpy( &liLength, aoBuffer.BasePtr(), sizeof(liLength) );
    liLength = ::ntohl(liLength);
    
    if ( liLength <= 4 || liLength >= AI_SYNC_MAX_BUFFER_SIZE )
    {
        return AI_SYNC_ERROR_LENGTH;
    }
    
    aoBuffer.Resize( liLength );
    HANDLE_READ( aoHandle, aoBuffer.WritePtr(), aoBuffer.GetSize() - 4, aiTimeOut );
    aoBuffer.WritePtr( liLength - 4 );
    
    return 0;
}

bool clsScpProtocol::IsHandshake( const char* apcData, size_t aiSize )
{
    if ( aiSize > 17 && ::strncasecmp( apcData + 4, "SHAKEHAND_RSP", 13 ) == 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//////////////////////////////////// SCP-InnerProtocol //////////////////////////////////////////
int clsScpInnerProtocol::Authenticate( AISocketHandle aoHandle, char* apcName, size_t aiSize )
{
    return AIProxy::AIInnerProtocol::Authenticate( aoHandle, apcName, aiSize );
    int liRetCode = 0;
    if ( ( liRetCode = this->HandleLogin( aoHandle ) ) != 0 )
    {
        return liRetCode;
    }
    
    if ( ( liRetCode = this->HandleAuth( aoHandle ) ) != 0 )
    {
        return liRetCode;
    }
    
    return AIProxy::AIInnerProtocol::Authenticate( aoHandle, apcName, aiSize );
}

int clsScpInnerProtocol::HandleLogin( AISocketHandle aoHandle )
{
    SCPMsgHeader loRequestMsgHeader;
    SCPMsgHeader loResponseMsgHeader(MSG_TYPE_LOGIN_RSP);
    AIChunkEx    loRequestBuffer(0);
    AIChunkEx    loResponseBuffer(0);
    AIChunkEx    loScpResponseBuffer(0);
    AIProtocol::THeader loHeader;
    AIProtocol*  lpoProtocol = AIProxy::GetInnerServer()->GetProtocol();

    ///Recv login request
    AI_RETURN_IF( -11, this->Recv( aoHandle, loRequestBuffer, 3 ) != 0 );
    
    AI_RETURN_IF( -12, lpoProtocol->Decode( loRequestBuffer.BasePtr(), loRequestBuffer.GetSize(), loHeader ) != 0 );
    
    ///Decode data
    AI_RETURN_IF( -13, loRequestMsgHeader.Decode(loRequestBuffer) != 0 );
    
    ///Check type
    AI_RETURN_IF( -14, loRequestMsgHeader.GetType() != MSG_TYPE_LOGIN );
    
    ///Response
    loResponseMsgHeader.GeneralHeader()->SetCSeq( loRequestMsgHeader.GeneralHeader()->GetCSeq() );
    loResponseMsgHeader.GeneralHeader()->SetFrom( loRequestMsgHeader.GeneralHeader()->GetTo() );
    loResponseMsgHeader.GeneralHeader()->SetTo( loRequestMsgHeader.GeneralHeader()->GetFrom() );
    loResponseMsgHeader.GeneralHeader()->SetServiceKey( loRequestMsgHeader.GeneralHeader()->GetServiceKey() );
    loResponseMsgHeader.SetStatus( 401 );
    loResponseMsgHeader.Encode( loScpResponseBuffer );
    
    loHeader.SetType( AIProtocol::RESPONSE );
    loHeader.SetLength( loScpResponseBuffer.GetSize() );
    loHeader.SetDataPtr( loScpResponseBuffer.BasePtr() );

    AI_RETURN_IF( -15, lpoProtocol->Encode( loHeader, loResponseBuffer ) != 0 );
    
    AI_RETURN_IF( -16, this->Send( aoHandle, loResponseBuffer.BasePtr(), loResponseBuffer.GetSize() ) != 0 );
    
    return 0;
}

int clsScpInnerProtocol::HandleAuth( AISocketHandle aoHandle )
{
    SCPMsgHeader loRequestMsgHeader;
    SCPMsgHeader loResponseMsgHeader(MSG_TYPE_AUTHENTICATION_RSP);
    AIChunkEx    loRequestBuffer(0);
    AIChunkEx    loResponseBuffer(0);
    AIChunkEx    loScpResponseBuffer(0);
    AIProtocol::THeader loHeader;
    AIProtocol*  lpoProtocol = AIProxy::GetInnerServer()->GetProtocol();

    ///Recv login request
    AI_RETURN_IF( -21, this->Recv( aoHandle, loRequestBuffer, 3 ) != 0 );
    
    AI_RETURN_IF( -22, lpoProtocol->Decode( loRequestBuffer.BasePtr(), loRequestBuffer.GetSize(), loHeader ) != 0 );
    
    ///Decode data
    AI_RETURN_IF( -23, loRequestMsgHeader.Decode(loRequestBuffer) != 0 );
    
    ///Check type
    AI_RETURN_IF( -24, loRequestMsgHeader.GetType() != MSG_TYPE_AUTHENTICATION );
    
    ///Response
    loResponseMsgHeader.GeneralHeader()->SetCSeq( loRequestMsgHeader.GeneralHeader()->GetCSeq() );
    loResponseMsgHeader.GeneralHeader()->SetFrom( loRequestMsgHeader.GeneralHeader()->GetTo() );
    loResponseMsgHeader.GeneralHeader()->SetTo( loRequestMsgHeader.GeneralHeader()->GetFrom() );
    loResponseMsgHeader.GeneralHeader()->SetServiceKey( loRequestMsgHeader.GeneralHeader()->GetServiceKey() );
    loResponseMsgHeader.SetStatus( 205 );
    loResponseMsgHeader.Encode( loScpResponseBuffer );
    
    loHeader.SetType( AIProtocol::RESPONSE );
    loHeader.SetLength( loScpResponseBuffer.GetSize() );
    loHeader.SetDataPtr( loScpResponseBuffer.BasePtr() );
    AI_RETURN_IF( -25, lpoProtocol->Encode( loHeader, loResponseBuffer ) != 0 );
    
    AI_RETURN_IF( -26, this->Send( aoHandle, loResponseBuffer.BasePtr(), loResponseBuffer.GetSize() ) != 0 );
    
    return 0;
}

int clsScpInnerProtocol::Decode( const char* apcData, size_t aiSize, THeader& aoHeader )
{
    return clsScpProtocol::Decode( apcData, aiSize, aoHeader );
}

int clsScpInnerProtocol::Encode( const THeader& aoHeader, AIChunkEx& aoBuffer )
{
    return clsScpProtocol::Encode( aoHeader, aoBuffer );
}
    
int clsScpInnerProtocol::Send( AISocketHandle aoHandle, const char* apcData, size_t aiSize )
{
    return clsScpProtocol::Send( aoHandle, apcData, aiSize );
}

int clsScpInnerProtocol::Recv( AISocketHandle aoHandle, AIChunkEx& aoBuffer, int aiTimeOut )
{
    return clsScpProtocol::Recv( aoHandle, aoBuffer, aiTimeOut );
}

//////////////////////////////////// SCP-OuterProtocol //////////////////////////////////////////
clsScpOuterProtocol::clsScpOuterProtocol()
{
}

clsScpOuterProtocol::~clsScpOuterProtocol()
{
}

int clsScpOuterProtocol::Authenticate( AISocketHandle aoHandle, const char* apcName )
{
    int           liRetCode = 0;
    SCPMsgHeader  loResponseMsgHeader;
    clsScpOuterServer* lpoServer = AIProxy::GetOuterServer<clsScpOuterServer*>();
    clsScpOuterServer::stServerNode loServerNode;
    
    if ( !lpoServer->GetServerNode( apcName, loServerNode ) )
    {
        return -1;
    }
    
    if ( ( liRetCode = this->Login( 
        aoHandle, loServerNode.csClientIp, loServerNode.csServerIp, loServerNode.ciForP2P, loResponseMsgHeader) ) != 0 )
    {
        return liRetCode;
    }
    if ( ( liRetCode = 
        this->Auth(
            aoHandle, 
            loServerNode.csClientIp, 
            loServerNode.csServerIp, 
            loServerNode.ciForP2P,
            loServerNode.csGwID, 
            loServerNode.csPassword, 
            loResponseMsgHeader ) ) != 0 )
    {
        return liRetCode;
    }
    
    return 0;
}

int clsScpOuterProtocol::Login( 
        AISocketHandle aoHandle, 
        const char* apcClientIp, 
        const char* apcServerIp, 
        int aiForP2P, 
        SCPMsgHeader& aoResponseMsgHeader )
{
    AIChunkEx    loBuffer(0);
    AIChunkEx    loResponse(0);
    SCPMsgHeader loMsgHeader(MSG_TYPE_LOGIN);

    loMsgHeader.GeneralHeader()->SetCSeq( 0 );
    loMsgHeader.GeneralHeader()->SetFrom( apcClientIp );
    loMsgHeader.GeneralHeader()->SetTo( apcServerIp );
    loMsgHeader.GeneralHeader()->SetServiceKey( ( aiForP2P != 0 ? 13 : 12 ) );
    
    AI_RETURN_IF( -11, loMsgHeader.Encode( loBuffer ) != 0 );
    
    AI_RETURN_IF( -12, this->Send( aoHandle, loBuffer.BasePtr(), loBuffer.GetSize() ) != 0 );
    
    AI_RETURN_IF( -13, this->Recv( aoHandle, loResponse, 3 ) != 0 );
    
    AI_RETURN_IF( -14, aoResponseMsgHeader.Decode( loResponse ) != 0 );
    
    AI_RETURN_IF( -15, aoResponseMsgHeader.GetType() != MSG_TYPE_LOGIN_RSP || aoResponseMsgHeader.GetStatus() != 401 );
    
    return 0;
}

int clsScpOuterProtocol::Auth( 
        AISocketHandle aoHandle, 
        const char* apcClientIp, 
        const char* apcServerIp, 
        int aiForP2P,
        const char* apcGwID,
        const char* apcPassword,
        SCPMsgHeader& aoResponseMsgHeader )
{
    AIChunkEx    loBuffer(0);
    AIChunkEx    loResponse(0);
    SCPMsgHeader loMsgHeader( MSG_TYPE_AUTHENTICATION );
    
    loMsgHeader.GeneralHeader()->SetCSeq( 0 );
    loMsgHeader.GeneralHeader()->SetFrom( apcClientIp );
    loMsgHeader.GeneralHeader()->SetTo( apcServerIp );
    loMsgHeader.GeneralHeader()->SetServiceKey( ( aiForP2P != 0 ? 13 : 12 ) );
    
    loMsgHeader.RequestHeader()->SetGwID( apcGwID );
    loMsgHeader.RequestHeader()->SetAlgorithm( "MD5" );
    loMsgHeader.RequestHeader()->SetNonce( aoResponseMsgHeader.ResponseHeader()->GetNonce() );
    loMsgHeader.RequestHeader()->MakeAuthDigest( aoResponseMsgHeader.GeneralHeader()->GetTimestamp(), apcPassword );

    AI_RETURN_IF( -21, loMsgHeader.Encode(loBuffer) != 0 );
    
    AI_RETURN_IF( -22, this->Send( aoHandle, loBuffer.BasePtr(), loBuffer.GetSize() ) != 0 );
    
    AI_RETURN_IF( -23, this->Recv( aoHandle, loResponse, 3 ) != 0 );
    
    AI_RETURN_IF( -24, loMsgHeader.Decode( loResponse ) != 0 );
    
    if ( loMsgHeader.GetType() != MSG_TYPE_AUTHENTICATION_RSP || loMsgHeader.GetStatus() != 205 )
    {
        return loMsgHeader.GetStatus();
    }
    
    return 0;
}

int clsScpOuterProtocol::Decode( const char* apcData, size_t aiSize, THeader& aoHeader )
{
    return clsScpProtocol::Decode( apcData, aiSize, aoHeader );
}

int clsScpOuterProtocol::Encode( const THeader& aoHeader, AIChunkEx& aoBuffer )
{
    return clsScpProtocol::Encode( aoHeader, aoBuffer );
}
    
int clsScpOuterProtocol::Send( AISocketHandle aoHandle, const char* apcData, size_t aiSize )
{
    return clsScpProtocol::Send( aoHandle, apcData, aiSize );
}

int clsScpOuterProtocol::Recv( AISocketHandle aoHandle, AIChunkEx& aoBuffer, int aiTimeOut )
{
    return clsScpProtocol::Recv( aoHandle, aoBuffer, aiTimeOut );
}
