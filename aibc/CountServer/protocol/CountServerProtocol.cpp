
#include "CountServerProtocol.h"
#include "acl/Stream.h"

CCSPHead::CCSPHead()
    : miMsgID(0)
    , miResult(0)
    , miLength(0)
{
}

ssize_t CCSPHead::Encode( char* apcBuffer, size_t auSize )
{
    ssize_t liRetSize = 0;
    
    if (auSize < HEAD_SIZE)
    {
        return -1;
    }
    
    apl_int32_t liMsgID  = apl_hton32(this->miMsgID);
    apl_int32_t liResult = apl_hton32(this->miResult);
    apl_int32_t liLength = apl_hton32(this->miLength);
    
    apl_memcpy(apcBuffer + liRetSize, &liMsgID, sizeof(this->miMsgID) );
    liRetSize += sizeof(this->miMsgID);
    
    apl_memcpy(apcBuffer + liRetSize, &liResult, sizeof(this->miResult) );
    liRetSize += sizeof(this->miResult);
    
    apl_memcpy(apcBuffer + liRetSize, &liLength, sizeof(this->miLength) );
    liRetSize += sizeof(this->miLength);
       
    return liRetSize;
}

ssize_t CCSPHead::Decode( char const* apcBuffer, size_t auSize )
{
    ssize_t liRetSize = 0;
    
    if (auSize < HEAD_SIZE)
    {
        return -1;
    }
    
    apl_memcpy(&this->miMsgID,  apcBuffer + liRetSize, sizeof(this->miMsgID) );
    liRetSize += sizeof(this->miMsgID);
    
    apl_memcpy(&this->miResult, apcBuffer + liRetSize, sizeof(this->miResult) );
    liRetSize += sizeof(this->miResult);
    
    apl_memcpy(&this->miLength, apcBuffer + liRetSize, sizeof(this->miLength) );
    liRetSize += sizeof(this->miLength);
    
    this->miMsgID  = apl_ntoh32(this->miMsgID);
    this->miResult = apl_ntoh32(this->miResult);
    this->miLength = apl_ntoh32(this->miLength);
    
    return liRetSize;
}

int32_t CCSPHead::GetMsgID(void)
{
    return this->miMsgID;
}

void CCSPHead::SetMsgID( apl_int32_t aiMsgID )
{
    this->miMsgID = aiMsgID;
}

int32_t CCSPHead::GetLength(void)
{
    return this->miLength;
}

void CCSPHead::SetLength( apl_int32_t aiLength )
{
    this->miLength = aiLength;
}

/////////////////////////////////////////////////////////////////////////////////////////////
CCSPRequest::CCSPRequest()
    : miCmdID(0)
    , miNum(0)
    , miLastNum(0)
{
    this->macKey[0] = '\0';
}

ssize_t CCSPRequest::Encode( char* apcBuffer, size_t auSize )
{
    ssize_t liResult = 0;
    
    if (auSize < REQUEST_SIZE)
    {
        return -1;
    }
    
    this->SetLength(REQUEST_SIZE - HEAD_SIZE);
    
    if ( (liResult = CCSPHead::Encode(apcBuffer, auSize) ) <= 0 )
    {
        return liResult;
    }
    
    apl_int32_t liCmdID   = apl_hton32(this->miCmdID);
    apl_int32_t liNum     = apl_hton32(this->miNum);
    apl_int32_t liLastNum = apl_hton32(this->miLastNum);
    
    apl_memcpy(apcBuffer + liResult, &liCmdID, sizeof(this->miCmdID) );
    liResult += sizeof(this->miCmdID);
    
    apl_memcpy(apcBuffer + liResult, this->macKey, sizeof(this->macKey) );
    liResult += sizeof(this->macKey);
    
    apl_memcpy(apcBuffer + liResult, &liNum, sizeof(this->miNum) );
    liResult += sizeof(this->miNum);
    
    apl_memcpy(apcBuffer + liResult, &liLastNum, sizeof(this->miLastNum) );
    liResult += sizeof(this->miLastNum);
       
    return liResult;
}

ssize_t CCSPRequest::Decode( char const* apcBuffer, size_t auSize )
{
    ssize_t liResult = 0;
    
    if (auSize < REQUEST_SIZE)
    {
        return -1;
    }
    
    if ( (liResult = CCSPHead::Decode(apcBuffer, auSize) ) <= 0 )
    {
        return liResult;
    }
    
    if ( this->GetLength() != REQUEST_SIZE - HEAD_SIZE )
    {
        return -1;
    }
    
    apl_memcpy(&this->miCmdID, apcBuffer + liResult, sizeof(this->miCmdID) );
    liResult += sizeof(this->miCmdID);
    
    apl_memcpy(this->macKey, apcBuffer + liResult, sizeof(this->macKey) );
    liResult += sizeof(this->macKey);
    
    apl_memcpy(&this->miNum, apcBuffer + liResult, sizeof(this->miNum) );
    liResult += sizeof(this->miNum);
    
    apl_memcpy(&this->miLastNum, apcBuffer + liResult, sizeof(this->miLastNum) );
    liResult += sizeof(this->miLastNum);
    
    this->miCmdID   = apl_ntoh32(this->miCmdID);
    this->miNum     = apl_ntoh32(this->miNum);
    this->miLastNum = apl_ntoh32(this->miLastNum);
       
    return liResult;
}

int32_t CCSPRequest::GetCmdID(void)
{
    return this->miCmdID;
}

void CCSPRequest::SetCmdID( apl_int32_t aiCmdID )
{
    this->miCmdID = aiCmdID;
}

char const* CCSPRequest::GetKey(void)
{
    return this->macKey;
}

void CCSPRequest::SetKey( char const* apcKey )
{
    apl_strncpy(this->macKey, apcKey, sizeof(this->macKey) );
}

int32_t CCSPRequest::GetNum(void)
{
    return this->miNum;
}

void CCSPRequest::SetNum( int32_t aiNum )
{
    this->miNum = aiNum;
}

int32_t CCSPRequest::GetLastNum(void)
{
    return this->miLastNum;
}

void CCSPRequest::SetLastNum( int32_t aiNum )
{
    this->miLastNum = aiNum;
}

////////////////////////////////////////////////////////////////////////////////////////////////
CCSPResponse::CCSPResponse()
    : miCmdID(0)
    , miState(0)
{
}

ssize_t CCSPResponse::Encode( char* apcBuffer, size_t auSize )
{
    ssize_t liResult = 0;
    
    if (auSize < RESPONSE_SIZE)
    {
        return -1;
    }
    
    this->SetLength(RESPONSE_SIZE - HEAD_SIZE);
    
    if ( (liResult = CCSPHead::Encode(apcBuffer, auSize) ) <= 0 )
    {
        return liResult;
    }

    apl_int32_t liCmdID     = apl_hton32(this->miCmdID);
    apl_int32_t liState     = apl_hton32(this->miState);
    apl_int64_t li64Timestamp = apl_hton64(this->mi64Timestamp);
    
    apl_memcpy(apcBuffer + liResult, &liCmdID, sizeof(this->miCmdID) );
    liResult += sizeof(this->miCmdID);
    
    apl_memcpy(apcBuffer + liResult, &liState, sizeof(this->miState) );
    liResult += sizeof(this->miState);
    
    apl_memcpy(apcBuffer + liResult, &li64Timestamp, sizeof(this->mi64Timestamp) );
    liResult += sizeof(this->mi64Timestamp);
       
    return liResult;
}
    
ssize_t CCSPResponse::Decode( char const* apcBuffer, size_t auSize )
{
    ssize_t liResult = 0;
    
    if (auSize < RESPONSE_SIZE)
    {
        return -1;
    }
    
    if ( (liResult = CCSPHead::Decode(apcBuffer, auSize) ) <= 0 )
    {
        return liResult;
    }
    
    if ( this->GetLength() != RESPONSE_SIZE - HEAD_SIZE )
    {
        return -1;
    }
    
    apl_memcpy(&this->miCmdID, apcBuffer + liResult, sizeof(this->miCmdID) );
    liResult += sizeof(this->miCmdID);
    
    apl_memcpy(&this->miState, apcBuffer + liResult, sizeof(this->miState) );
    liResult += sizeof(this->miState);
    
    apl_memcpy(&this->mi64Timestamp, apcBuffer + liResult, sizeof(this->mi64Timestamp) );
    liResult += sizeof(this->mi64Timestamp);
    
    this->miCmdID = apl_ntoh32(this->miCmdID);
    this->miState = apl_ntoh32(this->miState);
    this->mi64Timestamp = apl_ntoh64(this->mi64Timestamp);
       
    return liResult;
}

int32_t CCSPResponse::GetCmdID(void)
{
    return this->miCmdID;
}

void CCSPResponse::SetCmdID( apl_int32_t aiMsgID )
{
    this->miCmdID = aiMsgID;
}

int32_t CCSPResponse::GetState(void)
{
    return this->miState;
}

void CCSPResponse::SetState( apl_int32_t aiState )
{
    this->miState = aiState;
}

int64_t CCSPResponse::GetTimestamp(void)
{
    return this->mi64Timestamp;
}
    
void CCSPResponse::SetTimestamp( int64_t ai64Timestamp )
{
    this->mi64Timestamp = ai64Timestamp;
}
