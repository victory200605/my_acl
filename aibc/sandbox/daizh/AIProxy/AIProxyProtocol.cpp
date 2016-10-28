
#include "AIProxyProtocol.h"
#include "AILib.h"

START_AIPROXY_NAMESPACE

AIProtocol::stHeader::stHeader() : 
    ciSequenceNum(0),
    ciType(AIProtocol::REQUEST),
    ciLength(0),
    cpcDataPtr(NULL)
{
}

int AIProtocol::Send( AISocketHandle aoHandle, const char* apcData, size_t aiSize )
{
    HANDLE_WRITE( aoHandle, apcData, aiSize, 10 );
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int AIInnerProtocol::Authenticate( AISocketHandle aoHandle, char* apcName, size_t aiSize )
{
    static AITypeObject<size_t, AIMutexLock> sliConnectionID;
    ::snprintf( apcName, aiSize, "%d", ++sliConnectionID );
    return 0;
}

int AIInnerProtocol::Encode( const THeader& aoHeader, AIChunkEx& aoBuffer )
{
    AIChunkExOut loOutput(aoBuffer);
    
    ///package size
    aoBuffer.Resize( sizeof(int) * 3 /*header size*/ + aoHeader.GetLength() );
    
    loOutput.PutNInt32( aoHeader.GetSequenceNum() );
    loOutput.PutNInt32( aoHeader.GetType() );
    loOutput.PutNInt32( aoHeader.GetLength() );
    
    if ( aoHeader.GetDataPtr() != NULL )
    {
        loOutput.PutMem( aoHeader.GetDataPtr(), aoHeader.GetLength() );
    }
    
    return 0;
}
    
int AIInnerProtocol::Decode( const char* apcData, size_t aiSize, THeader& aoHeader )
{
    AIChunkEx   loBuffer( apcData, aiSize );
    
    AI_RETURN_IF( -1, this->DecodeHeader( loBuffer, aoHeader ) != 0 );

    AI_RETURN_IF( -1, aoHeader.GetLength() != loBuffer.WritePtr() - loBuffer.ReadPtr() );
    aoHeader.SetDataPtr( apcData + (loBuffer.ReadPtr() - loBuffer.BasePtr()) );
    
    return 0;
}

int AIInnerProtocol::DecodeHeader( AIChunkEx& aoBuffer, THeader& aoHeader )
{
    AIChunkExIn loInput(aoBuffer);
    int         liSequenceNum = 0;
    int         liType        = 0;
    int         liLength      = 0;
    
    AI_RETURN_IF( -1, loInput.ToHInt32( liSequenceNum ) != 0 );
    AI_RETURN_IF( -1, loInput.ToHInt32( liType ) != 0 );
    AI_RETURN_IF( -1, loInput.ToHInt32( liLength ) != 0 );
    
    aoHeader.SetSequenceNum( liSequenceNum );
    aoHeader.SetType( liType );
    aoHeader.SetLength( liLength );
    
    return 0;
}

int AIInnerProtocol::Recv( AISocketHandle aoHandle, AIChunkEx& aoChunk, int aiTimeOut )
{
    THeader loHeader;
    
    aoChunk.Reset();
    aoChunk.Resize( sizeof(int) * 3 /*header size*/ );
    
    HANDLE_READ( aoHandle, aoChunk.BasePtr(), aoChunk.GetSize(), aiTimeOut );
    aoChunk.WritePtr( aoChunk.GetSize() );
    
    AI_RETURN_IF( AI_SYNC_ERROR_DECODE, this->DecodeHeader( aoChunk, loHeader ) != 0 );
    
    if ( loHeader.GetLength() <= 0 || loHeader.GetLength() > AI_SYNC_MAX_BUFFER_SIZE )
    {
        //Exception packet
        return AI_SYNC_ERROR_LENGTH;
    }
    
    aoChunk.Resize( aoChunk.GetSize() + loHeader.GetLength() );
    
    HANDLE_READ( aoHandle, aoChunk.WritePtr(), loHeader.GetLength(), aiTimeOut );
    aoChunk.WritePtr( loHeader.GetLength() );
    
    return AI_SYNC_NO_ERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
int AIOuterProtocol::Authenticate( AISocketHandle aoHandle, const char* apcName )
{
    return 0;
}

END_AIPROXY_NAMESPACE
