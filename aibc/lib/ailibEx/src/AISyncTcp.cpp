
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "AIChunkEx.h"
#include "AILogSys.h"
#include "AISyncTcp.h"
#include "AIModule.h"

///start namespace
AIBC_NAMESPACE_START

// Const Define
// Time out value
const int AI_SYNC_TIMEOUT = 20;

// Channel conntion time out value
const int CHANNEL_CONN_TIMEOUT = 10;

const int CHANNEL_PENDING_TIMEOUT = 10;

// Max line lenght
const int AI_MAX_LINE_LEN = 1024;
const int AI_MAX_BUFFER_LEN = 1024*1024;

#define AI_EOL "\r\n"
// End const

void __SyncWriteLog( const char* apcFormat, ... )
{
    char    lsBuffer[4096] = {0};
    va_list loVl;
    va_start( loVl, apcFormat );
    vsnprintf( lsBuffer, sizeof(lsBuffer), apcFormat, loVl );
    va_end( loVl );
    AIM::AIModuleLog::Instance()->Write( "%s", lsBuffer );
}

////////////////////////////////////////////////////////////////////////////////////
//AIBaseChannel
#define SYNC_CHANNEL_ASSERT( msg ) \
    if ( !cbIsClient && !this->IsDual() ) return AI_SYNC_ERROR_SEND;

AIBaseChannel::AIBaseChannel() :
    cbIsClosed(true),
    cbIsExited(true),
    cbIsClient(false),
    cbIsReady(false)
{
    ctHandle[0] = AI_INVALID_SYNCHANDLE;
    ctHandle[1] = AI_INVALID_SYNCHANDLE;
}

AIBaseChannel::~AIBaseChannel()
{
    this->Close();
}

int AIBaseChannel::Initialize( AI_SYNCHANDLE atHandle[], 
    const char* apsRemoteIp, int aiRemotePort, int aiChannelID, bool abIsClient /* = false */ )
{
    int liRetCode = 0;
    
    if ( !this->IsClosed() )
    {
        return AI_SYNC_ERROR_ACTIVE;
    }
    
    // Set
    ctHandle[0]  = atHandle[0];
    ctHandle[1]  = atHandle[1];
    ciRemotePort = aiRemotePort;
    ciChannelID  = aiChannelID;
    cbIsClient   = abIsClient;
    cbIsClosed   = false;
    strncpy( csRemoteIp, apsRemoteIp, sizeof(csRemoteIp) );
    
    // Start deamon thread
    if ( !cbIsClient || ctHandle[0] != ctHandle[1] )
    {
        // Client single channel do not need InputHandle
        if ( ( liRetCode = ::pthread_create( &ciThrd, NULL, InputHandle, this ) ) != 0 )
        {
            cbIsClosed = true;
            return AI_SYNC_ERROR_THREAD;
        }
        cbIsExited = false;
    }
    
    return liRetCode;
}

void AIBaseChannel::CloseHandle()
{
    cbIsClosed = true;
    
    AIBC::ai_socket_close( ctHandle[0] );
    AIBC::ai_socket_close( ctHandle[1] );
}

void AIBaseChannel::Close()
{
    this->CloseHandle();
    
    while( true )
    {
        if ( cbIsExited == true 
            || ( pthread_kill( ciThrd, 0 ) == ESRCH )  )
        {
            cbIsExited = true;
            break;
        }
        AISleepFor( AI_TIME_MSEC * 100 );
    }
}

bool AIBaseChannel::IsClosed()
{
    return cbIsClosed;
}

bool AIBaseChannel::IsDual()
{
    return ctHandle[0] > 0 && ctHandle[1] > 0 && ctHandle[0] != ctHandle[1];
}

int AIBaseChannel::Request( const char* apsData, size_t aiSize )
{
    SYNC_CHANNEL_ASSERT( server_single_channel_unimplement );
    
    int  liRetCode = 0;
    AISmartLock loLock( coRequestLock );
    
    // Send request
    if ( ( liRetCode = this->Send( ctHandle[0], apsData, aiSize ) ) != 0 )
    {
        return liRetCode;
    }

    return liRetCode;
}

int AIBaseChannel::Request( const char* apsData, size_t aiSize, AIChunkEx& aoResponse, int aiTimeout /*=AI_SYNC_TIMEOUT*/ )
{
    SYNC_CHANNEL_ASSERT( server_single_channel_unimplement );
    
    int  liRetCode    = 0;  
    AISmartLock loLock( coRequestLock );
    
    //Send request
    if ( ( liRetCode = this->Send( ctHandle[0], apsData, aiSize ) ) != 0 )
    {
        return liRetCode;
    }
    
    //Recv response
    if ( ( liRetCode = this->Recv( ctHandle[0], aoResponse, AI_SYNC_TIMEOUT ) ) != 0 )
    {
        return liRetCode;
    }
    
    return liRetCode;
}

int AIBaseChannel::Response( const char* apsData, size_t aiSize )
{
    AISmartLock loLock( coResponseLock );
    
    return this->Send( ctHandle[1], apsData, aiSize );
}

int AIBaseChannel::Input( const char* apsData, size_t aiSize )
{
    return 0;
}

int AIBaseChannel::Error( AI_SYNCHANDLE atHandle, int aiErrno )
{
    this->CloseHandle();
    return 0;
}

int AIBaseChannel::Exit()
{
    return 0;
}

void* AIBaseChannel::InputHandle( void* apParam )
{
    int              liRetCode  = 0;
    int              liTimeWhisper = 0; //unit usec
    AIBaseChannel*   lpoChannel = (AIBaseChannel*)apParam;
    AIChunkEx        loChunk;
    
    pthread_detach(pthread_self());
    
    do
    {
        while( !lpoChannel->IsClosed() && !lpoChannel->IsReady() )
        {
            AISleepFor( AI_TIME_MSEC * 100 );
            if ( ( ( liTimeWhisper = liTimeWhisper + 100 ) / 1000 ) /*sec*/ > CHANNEL_PENDING_TIMEOUT )
            {
                break;
            }
            continue;
        }
        
        while( !lpoChannel->IsClosed() )
        {
            liRetCode = lpoChannel->Recv( lpoChannel->ctHandle[1], loChunk, AI_SYNC_TIMEOUT );
            if ( liRetCode == AI_SYNC_NO_ERROR )
            {
                lpoChannel->Input( loChunk.BasePtr(), loChunk.GetSize() );
            }
            else if ( liRetCode != AI_SYNC_ERROR_TIMEOUT )
            {
                break;
            }
        }
        
    }while(false);

    lpoChannel->cbIsExited = true;
    lpoChannel->Exit();

    return NULL;
}

/////////////////////////////////////// *Protocol* //////////////////////////////////////////////
// CmdLineConnection
const char* clsCmdLineConnection::S_SINGLE = "HELLO";
const char* clsCmdLineConnection::S_DUAL = "HELLI";

clsCmdLineConnection::clsCmdLineConnection( int aiType )
{
    this->SetType( aiType );
}
    
int clsCmdLineConnection::Encode( AIChunkEx& aoChunk )
{
    ///Noting to send
    aoChunk.Resize(0);
    return 0;
}

int clsCmdLineConnection::Decode( AIChunkEx& aoChunk )
{
    if ( strncasecmp( aoChunk.BasePtr(), S_SINGLE, strlen(S_SINGLE) ) == 0 )
    {
        ciType = AIBaseChannel::T_SINGLE;
    }
    else if ( strncasecmp( aoChunk.BasePtr(), S_DUAL, strlen(S_DUAL) ) == 0 )
    {
        ciType = AIBaseChannel::T_DUAL;
    }
    else
    {
        ciType = AIBaseChannel::T_UNKNOW;
    }
    
    return 0;
}

size_t clsCmdLineConnection::GetSize()
{
    if ( ciType == AIBaseChannel::T_SINGLE )
    {
        return strlen(S_SINGLE);
    }
    else
    {
        return strlen(S_DUAL);
    }
}

void clsCmdLineConnection::SetType( int aiType )
{
    ciType = aiType;
}

int clsCmdLineConnection::GetType()
{
    return ciType;
}
////clsDefaultConnection
clsDefaultConnection::clsDefaultConnection( int aiType )
{
    this->SetType( aiType );
}
    
int clsDefaultConnection::Encode( AIChunkEx& aoChunk )
{
    AIChunkExOut loOutput(aoChunk);
    
    loOutput.PutNInt32( ciType );

    return 0;
}

int clsDefaultConnection::Decode( AIChunkEx& aoChunk )
{
    AIChunkExIn loInput(aoChunk);
    
    AI_RETURN_IF( -1, loInput.ToHInt32( ciType ) != 0 );
    
    return 0;
}

size_t clsDefaultConnection::GetSize()
{
    return sizeof(ciType);
}

void clsDefaultConnection::SetType( int aiType )
{
    ciType = aiType;
}

int clsDefaultConnection::GetType()
{
    return ciType;
}

//clsHandShake
clsHandShake::clsHandShake() :
    ciChannelID(0),
	ciConnSlot(0),
	ciStat(0)
{
}

int clsHandShake::Encode( AIChunkEx& aoChunk )
{
    AIChunkExOut loOut( aoChunk );
	loOut.PutNInt32( ciChannelID );
	loOut.PutNInt32( ciConnSlot );
	loOut.PutNInt32( ciStat );
	return 0;
}

int clsHandShake::Decode( AIChunkEx& aoChunk )
{
    AIChunkExIn loIn( aoChunk );
	AI_RETURN_IF( -1, loIn.ToHInt32(ciChannelID) != 0 );
	AI_RETURN_IF( -1, loIn.ToHInt32(ciConnSlot) != 0 );
	AI_RETURN_IF( -1, loIn.ToHInt32(ciStat) != 0 );
	return 0;
}

size_t clsHandShake::GetSize()
{
    return sizeof(clsHandShake);
}

int Recv( AI_SYNCHANDLE atHandle, clsCmdLineConnection& aoProtocol )
{
    int       liRetCode = 0;
    AIChunkEx loChunk( AI_MAX_LINE_LEN );
    
    // Resv handshake command
    liRetCode = AIBC::ai_socket_peekdata( atHandle, loChunk.BasePtr(), (int)loChunk.GetSize(), AI_SYNC_TIMEOUT );
    if( liRetCode <= 0 )
    {
        return AI_SYNC_ERROR_RECV;
    }
    
    loChunk.Resize( liRetCode );
    loChunk.WritePtr( liRetCode );
    
    aoProtocol.Decode(loChunk);
    
    return AI_SYNC_NO_ERROR;
}

//////////////////////////////////////////////////////////////////////////////////////
// AICmdLineChannel
const char* AICmdLineChannel::STAG = "APPENDED=";
 
size_t AICmdLineChannel::DecodeDataSize( AIChunkEx& aoChunk )
{
    if ( aoChunk.GetCapacity() == aoChunk.GetSize() )
    {
        aoChunk.Resize( aoChunk.GetSize() + 1 );
        aoChunk.Resize( aoChunk.GetSize() - 1 );    
    }
    
    aoChunk.BasePtr()[aoChunk.GetSize()] = '\0';

    const char* lpcNext = strstr( aoChunk.BasePtr(), STAG );
    if ( lpcNext != NULL )
    {
        return atoi(lpcNext + strlen(STAG));
    }

    return 0;
}

int AICmdLineChannel::Send( AI_SYNCHANDLE atHandle, const char* apsData, size_t aiSize )
{
    int    liRetCode = 0;
    size_t liEolLen = strlen(AI_EOL);
    
    do
    {
        if ( (liRetCode = AIBC::ai_socket_senddata( atHandle, apsData, (int)aiSize )) != (int)aiSize )
        {
            break;
        }
        if ( strncmp( apsData + aiSize - liEolLen, AI_EOL, liEolLen ) != 0
            && (liRetCode = AIBC::ai_socket_senddata( atHandle, AI_EOL, liEolLen )) != (int)liEolLen )
        {
            break;
        }
        
        return AI_SYNC_NO_ERROR;
    }
    while( false );
    
    AI_SYNC_ERROR( "[Channel]:Send data to [%s:%d] fail, MSGCODE:%d", this->GetRemoteIp(), this->GetRemotePort(), liRetCode );
            
    // Call back error hook method
    this->Error( atHandle, AI_SYNC_ERROR_SEND );
            
    return AI_SYNC_ERROR_SEND;
}

int AICmdLineChannel::Recv( AI_SYNCHANDLE atHandle, AIChunkEx& aoChunk, int aiTimeout )
{
    int    liRetCode  = 0;
    size_t liDataSize = 0;
    
    aoChunk.Reset();
    aoChunk.Resize(AI_MAX_LINE_LEN);
    
    do
    {
        if ( (liRetCode = AIBC::ai_socket_recvline( 
            atHandle, aoChunk.BasePtr(), (int)aoChunk.GetSize(), int(aiTimeout) )) <= 0 )
        {
            break;
        }
        
        if ( liRetCode == 2 ) continue;
        
        // Decode protocol header
        aoChunk.Resize(liRetCode);
        aoChunk.WritePtr(liRetCode);
        liDataSize = this->DecodeDataSize( aoChunk );
        
        AI_SYNC_TRACE( "[Channel]:Recv data trace, [Cmd=%s]", aoChunk.BasePtr() );
        
        if( liDataSize > 0 )
        {
            aoChunk.Resize( liRetCode + liDataSize );

            if ( (liRetCode = AIBC::ai_socket_recvdata( 
                atHandle, aoChunk.WritePtr(), (int)liDataSize, int(aiTimeout) )) != (int)liDataSize )
            {
                break;
            }
        }

        return AI_SYNC_NO_ERROR;
    }
    while( true );
    
    switch( liRetCode )
    {
        case 0: //time out
            liRetCode = AI_SYNC_ERROR_TIMEOUT;
            break;
        case AI_SOCK_ERROR_BROKEN: //connection close
            liRetCode = AI_SYNC_ERROR_BROKEN;
            break;
        default: //socket error for recv
            if ( !this->IsClosed() )
            {
                AI_SYNC_ERROR( "[Channel]:Recv data from [%s:%d] fail, MSGCODE:%d", csRemoteIp, ciRemotePort, liRetCode );
                liRetCode = AI_SYNC_ERROR_RECV;
            }
            else
            {
                liRetCode = AI_SYNC_ERROR_BROKEN;
            }
            
            // Call back error hook method
            this->Error( atHandle, liRetCode );
            break;
    }

    return liRetCode;
}

// AISyncChannel
int AISyncChannel::Send( AI_SYNCHANDLE atHandle, const char* apsData, size_t aiSize )
{
    int          liRetCode = 0;
    stHeader     loHeader = { 0, aiSize };
    AIChunkEx    loBuffer( sizeof(loHeader) + aiSize  );
    AIChunkExOut loOutput(loBuffer);
    
    loOutput.PutNInt32( loHeader.ciMsgID );
    loOutput.PutNInt32( loHeader.ciLenght );
    loOutput.PutMem( apsData, aiSize );
    
    if ( (liRetCode = AIBC::ai_socket_senddata( atHandle, loBuffer.BasePtr(), (int)loBuffer.GetSize() )) != (int)loBuffer.GetSize() )
    {
        AI_SYNC_ERROR( "[Channel]:Send data to [%s:%d] fail, MSGCODE:%d", this->GetRemoteIp(), this->GetRemotePort(), liRetCode );
        // Call back error hook method
        this->Error( atHandle, AI_SYNC_ERROR_SEND );
        
        return AI_SYNC_ERROR_SEND;
    }
        
    return AI_SYNC_NO_ERROR;
}

int AISyncChannel::Recv( AI_SYNCHANDLE atHandle, AIChunkEx& aoChunk, int aiTimeout )
{
    int      liRetCode  = 0;
    stHeader loHeader;
    
    aoChunk.Reset();
    aoChunk.Resize(sizeof(stHeader));
    
    do
    {
        if ( (liRetCode = AIBC::ai_socket_recvdata( 
            atHandle, aoChunk.BasePtr(), (int)aoChunk.GetSize(), int(aiTimeout) )) != (int)aoChunk.GetSize() )
        {
            break;
        }
        
        aoChunk.WritePtr(liRetCode);
        AIChunkExIn loInput(aoChunk);
        // Decode protocol header    
        loInput.ToHInt32( loHeader.ciMsgID );
        loInput.ToHInt32( loHeader.ciLenght );

        AI_SYNC_TRACE( "[Channel]:Recv data trace, [Cmd=%s]", aoChunk.BasePtr() );
        
        if ( loHeader.ciLenght <= 0 || loHeader.ciLenght >= AI_MAX_BUFFER_LEN )
        {
            //Exception packet
            return AI_SYNC_ERROR_LENGTH;
        }
        
        aoChunk.Resize( loHeader.ciLenght );

        if ( (liRetCode = AIBC::ai_socket_recvdata( 
            atHandle, aoChunk.BasePtr(), (int)aoChunk.GetSize(), int(aiTimeout) )) != (int)aoChunk.GetSize() )
        {
            break;
        }
        
        aoChunk.WritePtr(liRetCode);
        
        return AI_SYNC_NO_ERROR;
    }
    while( true );
    
    switch( liRetCode )
    {
        case 0: //time out
            liRetCode = AI_SYNC_ERROR_TIMEOUT;
            break;
        case AI_SOCK_ERROR_BROKEN: //connection close
            liRetCode = AI_SYNC_ERROR_BROKEN;
            break;
        default: //socket error for recv
            if ( !this->IsClosed() )
            {
                AI_SYNC_ERROR( "[Channel]:Recv data from [%s:%d] fail, MSGCODE:%d", csRemoteIp, ciRemotePort, liRetCode );
                liRetCode = AI_SYNC_ERROR_RECV;
            }
            else
            {
                liRetCode = AI_SYNC_ERROR_BROKEN;
            }
            
            // Call back error hook method
            this->Error( atHandle, liRetCode );
            break;
    }

    return liRetCode;
}

///end namespace
AIBC_NAMESPACE_END
