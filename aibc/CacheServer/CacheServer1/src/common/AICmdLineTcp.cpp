
#include "AICmdLineTcp.h"

AI_CACHE_NAMESPACE_START

// Const Define
// Max line lenght
const int AI_MAX_LINE_LEN = 4096;

#define AI_EOL "\r\n"
// End const

////////////////////////////////////////// *Protocol* ////////////////////////////////////////
////
int AICmdLineParamter::Decode( const char* apcData, size_t aiSize )
{
    int liOpt = 0;
    const char* lpcNext = NULL;
    const char* lpcCurr = apcData;
    const char* lpcEnd  = NULL;
    
    if ( apcData[aiSize-1] == '\n' ) aiSize--;
    if ( apcData[aiSize-1] == '\r' ) aiSize--;
        
    lpcEnd = apcData + aiSize;
    
    lpcNext = (const char*)::memchr( lpcCurr, ' ', aiSize );
    if ( lpcNext == NULL )
    {
        this->coCmd.cpcFirst = lpcCurr;
        this->coCmd.ciLen = aiSize;
        return 0;
    }
    else
    {
        this->coCmd.cpcFirst = lpcCurr;
        this->coCmd.ciLen = lpcNext - lpcCurr;
    }
    
    lpcCurr = lpcNext + 1;
    do
    {
        while( lpcCurr < lpcEnd && *lpcCurr == ' '  ) lpcCurr++;
        
        liOpt = lpcCurr[0] - 'a';
        
        if ( lpcCurr[1] != '=' 
            || ( liOpt < 0 || liOpt > (int)('z' - 'a') ) )
        {
            return -1;
        }

        lpcNext = (const char*)::memchr( lpcCurr, ' ', lpcEnd - lpcCurr );
        if ( lpcNext == NULL )
        {
            this->coParamter[liOpt].cpcFirst = lpcCurr + 2;
            this->coParamter[liOpt].ciLen = lpcEnd - lpcCurr - 2;
            break;
        }
        else
        {
            this->coParamter[liOpt].cpcFirst = lpcCurr + 2;
            this->coParamter[liOpt].ciLen = lpcNext - lpcCurr - 2;
            lpcCurr = lpcNext + 1;
        }
    }
    while(true);
    
    return 0;
}

int AICmdLineParamter::Encode( AIBC::AIChunkEx& aoBuffer, const char* apcFormat, ... )
{
    ///Check protocol format, end by \r\n
    const char* lpcLast = ::strstr( apcFormat, "\r\n" );
    if ( lpcLast != NULL && lpcLast[3] != '\0' )
    {
        return -1;
    }
    
    aoBuffer.Resize(AIM_MAX_LINE_LEN);
    aoBuffer.Reset();
    
    ///va all param value
    va_list loVl;
    va_start( loVl, apcFormat );
    vsnprintf( aoBuffer.BasePtr(), aoBuffer.GetSize(), apcFormat, loVl );
    va_end( loVl );
    
    ///resize buffer
    size_t liLen = strlen(aoBuffer.BasePtr());
    aoBuffer.WritePtr( liLen );

    if ( lpcLast == NULL )
    {
        ///End of encode
        aoBuffer.Resize( liLen + 2 );
        ::memcpy( aoBuffer.WritePtr(), "\r\n", 2 );
        aoBuffer.WritePtr(2);
    }

    return 0;
}
////////////////////////////////////////////////////////////////////////////////////
AICmdLineChannel::AICmdLineChannel( CHANNEL_MODE aiMode )
    : AIBaseChannel(aiMode)
{
}

AICmdLineChannel::~AICmdLineChannel()
{
}

/// Reserve transmit interface for protocol
int AICmdLineChannel::SendTo( AISocketHandle atHandle, const char* apcData, size_t aiSize ) 
{
    int    liRetCode = 0;
    size_t liEolLen = strlen(AI_EOL);
    
    do
    {
        if ( (liRetCode = AIBC::ai_socket_senddata( atHandle, apcData, (int)aiSize )) != (int)aiSize )
        {
            break;
        }
        if ( strncmp( apcData + aiSize - liEolLen, AI_EOL, liEolLen ) != 0
            && (liRetCode = AIBC::ai_socket_senddata( atHandle, AI_EOL, liEolLen )) != (int)liEolLen )
        {
            break;
        }
        
        return AI_SYNC_NO_ERROR;
    }
    while( false );
            
    return AI_SYNC_ERROR_SEND;
}

int AICmdLineChannel::RecvFrom( AISocketHandle atHandle, AIBC::AIChunkEx& aoBuffer, int aiTimeout )
{
    int    liRetCode  = 0;
    
    aoBuffer.Reset();
    aoBuffer.Resize(AI_MAX_LINE_LEN);
    
    do
    {
        if ( (liRetCode = AIBC::ai_socket_recvline( 
            atHandle, aoBuffer.BasePtr(), (int)aoBuffer.GetSize(), int(aiTimeout) )) <= 0 )
        {
            break;
        }
        
        if ( liRetCode == 2 ) continue;
        
        // Decode protocol header
        aoBuffer.Resize(liRetCode);
        aoBuffer.WritePtr(liRetCode);

        return AI_SYNC_NO_ERROR;
    }
    while( true );
    
    switch( liRetCode )
    {
        case 0: //time out
            liRetCode = AI_SYNC_ERROR_TIMEOUT;
            break;
        case AIBC::AI_SOCK_ERROR_BROKEN: //connection close
            liRetCode = AI_SYNC_ERROR_BROKEN;
            break;
        default: //socket error for recv
            liRetCode = AI_SYNC_ERROR_RECV;
            break;
    }

    return liRetCode;
}

AI_CACHE_NAMESPACE_END
