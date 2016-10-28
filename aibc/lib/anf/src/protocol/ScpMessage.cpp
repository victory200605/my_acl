
#include "acl/Hashs.h"
#include "acl/SString.h"
#include "acl/Singleton.h"
#include "anf/protocol/ScpMessage.h"

ANF_NAMESPACE_START

namespace scp {
    
const apl_size_t SCP_MAX_BUFFER_LEN = 1024;

inline bool IsIpAddress( const char* apcStr )
{
    return true;
    //ignore
    //if( *apcStr == '.' || *apcStr == '\0' )
    //{
    //    return false;
    //}
    //
    //while( *(apcStr) != '\0' )
    //{
    //    if ( !( (*(apcStr) >= '0' && *(apcStr) <= '9') || *(apcStr) == '.' ) )
    //    {
    //        return false;
    //    }
    //    apcStr++;
    //}
    //
    //return *(apcStr-1) == '.' ? false : true;
}

inline apl_int_t ReadLine(acl::CMemoryBlock& aoMemoryBlock, char* apcLine, apl_size_t auSize)
{
    apl_size_t luLength = aoMemoryBlock.GetLength();
    char* lpcReadPtr = aoMemoryBlock.GetReadPtr();
    char* lpcFirst = lpcReadPtr;
    char* lpcLast  = NULL;

    while(true)
    {
        lpcLast = (char*)apl_memchr(lpcFirst, '\r', luLength);
        if (lpcLast == NULL)
        {
            return -1;
        }
        else if ( *(lpcLast + 1) != '\n')
        {
            luLength -= (lpcLast - lpcFirst + 1);
            lpcFirst = lpcLast + 1;
        }
        else //hit '\r\n'
        {
            break;
        }
    }
    
    apl_size_t luLineLength = lpcLast - lpcReadPtr;
    
    if (luLineLength >= auSize)
    {
        return -1;
    }

    apl_memcpy(apcLine, lpcReadPtr, luLineLength);
    apcLine[luLineLength] = '\0';
    
    aoMemoryBlock.SetReadPtr(luLineLength + 2);
    
    return luLineLength;
}

inline apl_int_t Write(acl::CMemoryBlock& aoMemoryBlock, char const* apcPtr)
{
    apl_size_t liLength = apl_strlen(apcPtr);
    char* lpcWritePtr = aoMemoryBlock.GetWritePtr();
    
    if (aoMemoryBlock.Write(apcPtr, liLength) != (apl_ssize_t)liLength)
    {
        aoMemoryBlock.SetWritePtr(lpcWritePtr);
        return -1;
    }
    
    return 0;
}

#define SCP_RETURN_IF( op, ret ) if (op) { return ret;}

///////////////////////////////////////////////////////////////////////////////////////////////////
CMessageHeader::CNode::CNode(void)
    : mpoNext(NULL)
{
    this->macKey[0] = '\0';
    this->macValue[0] = '\0';
}

void CMessageHeader::CNode::Set( const char* apcKey, const char* apcValue )
{
    apl_strncpy(this->macKey, apcKey, sizeof(this->macKey) );
    apl_strncpy(this->macValue, apcValue, sizeof(this->macValue) );
}

char const* CMessageHeader::CNode::GetKey(void)
{
    return this->macKey;
}
        
char const* CMessageHeader::CNode::GetValue(void)
{
    return this->macValue;
}
        
CMessageHeader::CNode* CMessageHeader::CNode::GetNext(void)
{
    return this->mpoNext;
}

CMessageHeader::CMessageHeader( apl_size_t auBktNum)
    : muBktNum(auBktNum > 0 ? auBktNum : 100)
    , muElemNum(0)
{
    ACL_NEW_N_ASSERT(this->mppoBucket, CNode*, this->muBktNum);
    ACL_NEW_N_ASSERT(this->mpoElements, CNode, this->muBktNum);
    
    apl_memset(this->mppoBucket, 0, sizeof(CNode*) * this->muBktNum);
}
    
CMessageHeader::~CMessageHeader()
{
    ACL_DELETE_N(this->mppoBucket);
    ACL_DELETE_N(this->mpoElements);
}

apl_int_t CMessageHeader::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_ssize_t liResult = -1;
    char lacBuffer[SCP_MAX_BUFFER_LEN];
    
    while( (liResult = ReadLine(aoMemoryBlock, lacBuffer, sizeof(lacBuffer) ) ) > 0 )
    {
        char* lpcValue = (char*)apl_memchr(lacBuffer, ':', liResult);
        if (lpcValue == NULL)
        {
            lpcValue = (char*)apl_memchr(lacBuffer, '=', liResult);
        }
        if (lpcValue == NULL)
        {
            return -1;
        }
        
        *lpcValue++ = '\0';
        
        acl::stralgo::TrimLeftIf(lacBuffer, IsSpace);
        //acl::stralgo::TrimIf(lpcValue, IsClear);
            
        if ( !this->Insert(lacBuffer, lpcValue) )
        {
            return -1;
        }
    }
    
    return liResult == 0 ? 0 : -1;
}
    
CMessageHeader::CNode* CMessageHeader::Find( const char* apcKey ) const
{
    apl_size_t luKey = apl_strlen(apcKey) % this->muBktNum;
    CNode* lpoFirst = this->mppoBucket[luKey];
    for ( ; lpoFirst != NULL; lpoFirst = lpoFirst->mpoNext )
    {
        if (apl_strcasecmp(apcKey, lpoFirst->macKey) == 0)
        {
            return lpoFirst;
        }
    }
    
    return NULL;
}
    
bool CMessageHeader::Insert( const char* apcKey, const char* apcValue )
{
    if (this->muElemNum >= this->muBktNum)
    {
        return -1;
    }
    
    apl_size_t luKey = apl_strlen(apcKey) % this->muBktNum;
    
    CNode*& lpoFirst = this->mppoBucket[luKey];
    if (lpoFirst == NULL)
    {
        lpoFirst = &this->mpoElements[this->muElemNum];
        lpoFirst->Set(apcKey, apcValue);
    }
    else
    {
        CNode* lpoNew = &this->mpoElements[this->muElemNum];
        lpoNew->Set(apcKey, apcValue);

        lpoNew->mpoNext = lpoFirst;
        lpoFirst = lpoNew;
    }

	this->muElemNum++;
	
	return true;
}
    
void CMessageHeader::Clear(void)
{
    if (this->muElemNum > 0)
    {
        apl_memset(this->mppoBucket, 0, sizeof(CNode*) * this->muBktNum );
    	this->muElemNum = 0;
    }
}

/// scp-general-header
CGeneralHeader::CGeneralHeader(void)
    : muCSeq(0)
    , miServiceKey(-1)
{
    this->macFrom[0] = '\0';
    this->macTo[0] = '\0';
    this->macTimestamp[0] = '\0';
}
    
apl_int_t CGeneralHeader::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    char lacBuffer[SCP_MAX_BUFFER_LEN];
    
    if ( apl_strlen(this->GetTimestamp() ) <= 0)
    {
        this->UpdateTimestamp();
    }

    if( this->GetServiceKey() >= 0 )
    {
        apl_snprintf( 
            lacBuffer, sizeof(lacBuffer), 
            "CSeq:%"APL_PRIu32"\r\n"
            "ServiceKey:%"APL_PRIdINT"\r\n"
            "From:%s\r\n"
            "To:%s\r\n"
            "TimeStamp:%s\r\n",
            (apl_size_t)this->GetCSeq(),
            (apl_int_t)this->GetServiceKey(),
            this->GetFrom(),
            this->GetTo(),
            this->GetTimestamp() );
    }
    else
    {
        apl_snprintf( 
            lacBuffer, sizeof(lacBuffer), 
            "CSeq:%"APL_PRIu32"\r\n"
            "From:%s\r\n"
            "To:%s\r\n"
            "TimeStamp:%s\r\n",
            (apl_size_t)this->GetCSeq(),
            this->GetFrom(),
            this->GetTo(),
            this->GetTimestamp() );
    }

    return Write(aoMemoryBlock, lacBuffer);
}

apl_int_t CGeneralHeader::Decode( CMessageHeader const& aoMessageHeader )
{
    CMessageHeader::CNode* lpoNode = NULL;
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("CSeq") ) == NULL, -1);
    //blank character before number, exp seq: 123
    //if ( !acl::stralgo::All(lpoNode->GetValue(), IsNumber) )
    //{
    //    return -1;
    //}
    this->SetCSeq( apl_strtoi32(lpoNode->GetValue(), NULL, 10) );
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("From") ) == NULL, -1);
    if ( !IsIpAddress(lpoNode->GetValue() ) )
    {
        return -1;
    }
    this->SetFrom(lpoNode->GetValue() );
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("To") ) == NULL, -1);
    if ( !IsIpAddress(lpoNode->GetValue() ) )
    {
        return -1;
    }
    this->SetTo(lpoNode->GetValue() );
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("TimeStamp") ) == NULL, -1);
    if ( apl_strlen(lpoNode->GetValue() ) < 20 )
    {
        return -1;
    }
    this->SetTimestamp(lpoNode->GetValue() );
    
    if( (lpoNode = aoMessageHeader.Find("ServiceKey") ) != NULL )
    {
        if ( !acl::stralgo::All(lpoNode->GetValue(), IsNumber) )
        {
            return -1;
        }
        this->SetServiceKey(apl_strtoi32(lpoNode->GetValue(), NULL, 10) );
    }
    
    return 0;
}

void CGeneralHeader::UpdateTimestamp(void)
{
    acl::CDateTime loDateTime;
    
    loDateTime.Update();
    
    this->SetTimestamp( loDateTime.Format("%Y-%m-%dT%H:%M:%SZ") );
}

////////////////////////////////////////////////////////////////////////////////////////////////
/// scp-request-header-auth
CAuthRequestHeader::CAuthRequestHeader(void)
    : moAlgorithm("MD5")
    , moNonce("aaaa")
{
}

apl_int_t CAuthRequestHeader::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    char lacBuffer[SCP_MAX_BUFFER_LEN];
    
    apl_snprintf(
        lacBuffer, sizeof(lacBuffer),
        "Authorization:account=\"%s\"\r\n"
        "nonce=\"%s\"\r\n"
        "auth-digest=\"%s\"\r\n"
        "algorithm=\"%s\"\r\n",
        this->GetAccount(),
        this->GetNonce(),
        this->GetAuthDigest(),
        this->GetAlgorithm() );
        
    return Write(aoMemoryBlock, lacBuffer);
}

apl_int_t CAuthRequestHeader::Decode( CMessageHeader const& aoMessageHeader )
{
    CMessageHeader::CNode* lpoNode = NULL;
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("Authorization") ) == NULL, -1);
    if ( apl_strncasecmp(lpoNode->GetValue(), "account=", 8) != 0 )
    {
        return -1;
    }
    this->SetAccount(lpoNode->GetValue() + 8);
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("nonce") ) == NULL, -1);
    this->SetNonce(lpoNode->GetValue());
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("auth-digest") ) == NULL, -1);
    this->SetAuthDigest(lpoNode->GetValue());
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("algorithm") ) == NULL, -1);
    this->SetAlgorithm(lpoNode->GetValue());
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// scp-request-header-logout
CLogoutRequestHeader::CLogoutRequestHeader(void)
{
}

apl_int_t CLogoutRequestHeader::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    char lacBuffer[SCP_MAX_BUFFER_LEN];
    
    apl_snprintf(
        lacBuffer, sizeof(lacBuffer),
        "Logout:gw-user=\"%s\"\r\n",
        this->GetUser() );

    return Write(aoMemoryBlock, lacBuffer);
}

apl_int_t CLogoutRequestHeader::Decode( CMessageHeader const& aoMessageHeader )
{
    CMessageHeader::CNode* lpoNode = NULL;
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("Logout") ) == NULL, -1);
    if ( apl_strncasecmp(lpoNode->GetValue(), "gw-user=", 8) != 0 )
    {
        return -1;
    }
    this->SetUser(lpoNode->GetValue() + 8);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///scp-auth-response-header
CAuthResponseHeader::CAuthResponseHeader(void)
{
}

apl_int_t CAuthResponseHeader::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    char lacBuffer[SCP_MAX_BUFFER_LEN];

    apl_snprintf( lacBuffer, sizeof(lacBuffer), "Authenticate:nonce=\"%s\"\r\n", this->GetNonce() );

    if (Write(aoMemoryBlock, lacBuffer) != 0)
    {
        return -1;
    }
    
    for( apl_size_t luN = 0; luN < this->GetAlgorithmCount(); luN++ )
    {
        apl_snprintf( lacBuffer, sizeof(lacBuffer), "algorithm=\"%s\"\r\n", this->GetAlgorithm(luN) );
        if (Write(aoMemoryBlock, lacBuffer) != 0)
        {
            return -1;
        }
    }
    
    return 0;
}

apl_int_t CAuthResponseHeader::Decode( CMessageHeader const& aoMessageHeader )
{
    CMessageHeader::CNode* lpoNode = NULL;
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("authenticate") ) == NULL, -1);
    if ( apl_strncasecmp(lpoNode->GetValue(), "nonce=", 6) != 0 )
    {
        return -1;
    }
    this->SetNonce(lpoNode->GetValue() + 6);
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("algorithm") ) == NULL, -1);
    this->PushAlgorithm(lpoNode->GetValue() );
    
    for (lpoNode = lpoNode->GetNext(); lpoNode != NULL; lpoNode = lpoNode->GetNext() )
    {
        if (apl_strcasecmp(lpoNode->GetKey(), "algorithm") == 0)
        {
            this->PushAlgorithm(lpoNode->GetValue());
        }
    }
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///scp-retry-after-response-header
CRetryAfterResponseHeader::CRetryAfterResponseHeader(void)
    : miRetryAfter(0)
{
}
        
apl_int_t CRetryAfterResponseHeader::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    char lacBuffer[SCP_MAX_BUFFER_LEN];

    apl_snprintf( lacBuffer, sizeof(lacBuffer), "retry-after:%"APL_PRIdINT"\r\n", this->GetRetryAfter() );

    if (Write(aoMemoryBlock, lacBuffer) != 0)
    {
        return -1;
    }
    
    return 0;
}

apl_int_t CRetryAfterResponseHeader::Decode( CMessageHeader const& aoMessageHeader )
{
    CMessageHeader::CNode* lpoNode = NULL;
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("retry-after") ) == NULL, -1);
    if ( !acl::stralgo::All(lpoNode->GetValue(), IsNumber) )
    {
        return -1;
    }
    this->SetRetryAfter(apl_strtoi32(lpoNode->GetValue(), NULL, 10) );
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
CEntityHeader::CEntityHeader(void)
    : muContentLength(0)
{
    apl_strncpy(this->macContentType, "application/XML", sizeof(this->macContentType) );
}
    
apl_int_t CEntityHeader::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    char lacBuffer[SCP_MAX_BUFFER_LEN];

    apl_snprintf(
        lacBuffer, sizeof(lacBuffer),
        "Content-Length:%"APL_PRIdINT"\r\n"
        "Content-Type:%s\r\n",
        this->GetContentLength(),
        this->GetContentType() );

    if (Write(aoMemoryBlock, lacBuffer) != 0)
    {
        return -1;
    }
    
    return 0;
}

apl_int_t CEntityHeader::Decode( CMessageHeader const& aoMessageHeader )
{
    CMessageHeader::CNode* lpoNode = NULL;
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("content-length") ) == NULL, -1);
    if ( !acl::stralgo::All(lpoNode->GetValue(), IsNumber) )
    {
        return -1;
    }
    this->SetContentLength(apl_strtoi32(lpoNode->GetValue(), NULL, 10) );
    
    SCP_RETURN_IF( (lpoNode = aoMessageHeader.Find("content-type") ) == NULL, -1);
    this->SetContentType(lpoNode->GetValue());
    
    return 0;
}

std::string MakeAuthDigest(
    const char* apcAccount,
    const char* apcPassword,
    const char* apcNonce,
    const char* apcTimestamp )
{
    char lpcBuffer[129] = {0};
    char lpcDigest[33] = {0};
    acl::CMD5 loMd5;
    acl::CMD5::DigestType loDigest;

    apl_snprintf(
        lpcBuffer, sizeof(lpcBuffer),
        "%s:%s:%s:%s",
        apcTimestamp,
        apcNonce,
        apcAccount,
        apcPassword );

    loDigest = loMd5.Final(lpcBuffer, apl_strlen(lpcBuffer) );

    for(apl_int_t i = 0; i < 16; i++)
    {
        apl_snprintf(lpcDigest + 2 * i, 2 + 1, "%02x", loDigest.GetUstr()[i]);
    }
    
    return lpcDigest;
}

//////////////////////////////////////////// message //////////////////////////////////////////
CScpMessage::CScpMessage( apl_int_t aiType )
    : miType(aiType)
{
    apl_strncpy(this->macCmppVersion, CMPP_VERSION, sizeof(this->macCmppVersion) );
    this->macMethod[0] = '\0';
}

CScpMessage::~CScpMessage(void)
{
}

std::string CScpMessage::GetMessageType(void)
{
    return this->GetMethod();
}

void CScpMessage::SetMessageID(apl_size_t auMsgId)
{
    this->SetCSeq(auMsgId);
}

apl_size_t CScpMessage::GetMessageID()
{
    return this->GetCSeq();
}

void CScpMessage::SetMethod( char const* apcMethod )
{
    apl_strncpy(this->macMethod, apcMethod, sizeof(this->macMethod) );
}

char const* CScpMessage::GetMethod(void)
{
    if (apl_strlen(this->macMethod) <= 0)
    {
        apl_strncpy(this->macMethod, TypeToMethod(this->GetType() ), sizeof(this->macMethod) );
    }
    
    return this->macMethod;
}

bool CScpMessage::CheckType(void)
{
    return apl_strcmp(TypeToMethod(this->GetType() ), this->GetMethod() ) == 0;
}

/// request
CRequest::CRequest( apl_int_t aiType )
    : CScpMessage(aiType)
{
}

CRequest::~CRequest(void)
{
}

apl_int_t CRequest::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    char lacBuffer[SCP_MAX_BUFFER_LEN];

    apl_snprintf( lacBuffer, sizeof(lacBuffer), "%s %s\r\n", this->GetMethod(), this->GetCmppVersion() );
    
    SCP_RETURN_IF( Write(aoMemoryBlock, lacBuffer) != 0, WRN_NOT_ENOUGH_SPACE);

    SCP_RETURN_IF(CGeneralHeader::Encode(aoMemoryBlock) != 0, WRN_NOT_ENOUGH_SPACE);

    return 0;
}

apl_int_t CRequest::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    char* lpcMethod = NULL;
    char* lpcCmppVersion = NULL;
    char* lpcEnd = NULL;
    char  lacBuffer[SCP_MAX_BUFFER_LEN];
    apl_ssize_t liResult = 0;
    
    SCP_RETURN_IF( (liResult = ReadLine(aoMemoryBlock, lacBuffer, sizeof(lacBuffer) ) ) <= 0, ERR_REQUEST_LINE);
    
    lpcMethod = lacBuffer;
    if ( (lpcCmppVersion = (char*)apl_memchr(lpcMethod, ' ', liResult) ) == NULL)
    {
        return ERR_REQUEST_LINE;
    }
    *lpcCmppVersion++ = '\0';
    
    if ( (lpcEnd = (char*)apl_memchr(
        lpcCmppVersion, ' ', liResult - (lpcCmppVersion - lpcMethod) ) ) != NULL)
    {
        return ERR_REQUEST_LINE;
    }

    //SCP_RETURN_IF(this->GetType() != MethodToType(loTokenizer.GetField(0) ), ERR_REQUEST_LINE);
    this->SetMethod(lpcMethod);
    this->SetCmppVersion(lpcCmppVersion);
  
    SCP_RETURN_IF( this->GetMessageHeader().Decode(aoMemoryBlock) != 0, ERR_MESSAGE_HEADER );
    
    SCP_RETURN_IF(CGeneralHeader::Decode(this->GetMessageHeader() ) != 0, ERR_GENERAL_HEADER);
        
    return 0;
}

/// Login Request
CLoginRequest::CLoginRequest(void)
    : CRequest(MSG_TYPE_LOGIN)
{
}

apl_int_t CLoginRequest::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CRequest::Encode(aoMemoryBlock) ) != 0, liRetCode);
    
    SCP_RETURN_IF(aoMemoryBlock.Write("\r\n", 2) != 2, WRN_NOT_ENOUGH_SPACE);
    
    return 0;
}

apl_int_t CLoginRequest::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CRequest::Decode(aoMemoryBlock) ) != 0, liRetCode);
    
    return 0;
}

/// AuthRequest
CAuthRequest::CAuthRequest(void)
    : CRequest(MSG_TYPE_AUTHENTICATION)
{
}
    
apl_int_t CAuthRequest::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CRequest::Encode(aoMemoryBlock) ) != 0, liRetCode);
    
    SCP_RETURN_IF(CAuthRequestHeader::Encode(aoMemoryBlock) != 0, WRN_NOT_ENOUGH_SPACE);
    
    SCP_RETURN_IF(aoMemoryBlock.Write("\r\n", 2) != 2, WRN_NOT_ENOUGH_SPACE);
    
    return 0;
}

apl_int_t CAuthRequest::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CRequest::Decode(aoMemoryBlock) ) != 0, liRetCode);
    
    SCP_RETURN_IF(CAuthRequestHeader::Decode(this->GetMessageHeader() ) != 0, ERR_AUTH_REQUEST_HEADER);

    return 0;
}

/// LogoutRequest
CLogoutRequest::CLogoutRequest(void)
    : CRequest(MSG_TYPE_LOGOUT)
{
}
    
apl_int_t CLogoutRequest::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CRequest::Encode(aoMemoryBlock) ) != 0, liRetCode);
    
    SCP_RETURN_IF(CLogoutRequestHeader::Encode(aoMemoryBlock) != 0, WRN_NOT_ENOUGH_SPACE);
    
    SCP_RETURN_IF(aoMemoryBlock.Write("\r\n", 2) != 2, WRN_NOT_ENOUGH_SPACE);
    
    return 0;
}

apl_int_t CLogoutRequest::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CRequest::Decode(aoMemoryBlock) ) != 0, liRetCode);
    
    SCP_RETURN_IF(CLogoutRequestHeader::Decode(this->GetMessageHeader() ) != 0, ERR_AUTH_REQUEST_HEADER);
    
    return 0;
}

/// ShakehandRequest
CShakehandRequest::CShakehandRequest(void)
    : CRequest(MSG_TYPE_SHAKEHAND)
{
}
    
apl_int_t CShakehandRequest::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CRequest::Encode(aoMemoryBlock) ) != 0, liRetCode);
    
    SCP_RETURN_IF(aoMemoryBlock.Write("\r\n", 2) != 2, WRN_NOT_ENOUGH_SPACE);
    
    return 0;
}

apl_int_t CShakehandRequest::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CRequest::Decode(aoMemoryBlock) ) != 0, liRetCode);

    return 0;
}

/// EntityRequest
CEntityRequest::CEntityRequest(void)
    : CRequest(MSG_TYPE_REQUEST)
    , moBody(NULL, 0, acl::CMemoryBlock::DONT_DELETE)
{
}
    
apl_int_t CEntityRequest::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CRequest::Encode(aoMemoryBlock) ) != 0, liRetCode);
    
    this->SetContentLength(this->GetBody().GetLength() );
    
    if (this->GetBody().GetLength() > 0)
    {
        SCP_RETURN_IF(CEntityHeader::Encode(aoMemoryBlock) != 0, WRN_NOT_ENOUGH_SPACE);
        
        SCP_RETURN_IF(aoMemoryBlock.Write("\r\n", 2) != 2, WRN_NOT_ENOUGH_SPACE);
        
        SCP_RETURN_IF( 
            aoMemoryBlock.Write(this->GetBody().GetReadPtr(), this->GetBody().GetLength() ) != (apl_ssize_t)this->GetBody().GetLength(),
            WRN_NOT_ENOUGH_SPACE );
    }
    else
    {
        SCP_RETURN_IF(aoMemoryBlock.Write("\r\n", 2) != 2, WRN_NOT_ENOUGH_SPACE);
    }
    
    return 0;
}

apl_int_t CEntityRequest::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CRequest::Decode(aoMemoryBlock) ) != 0, liRetCode);

    liRetCode = CEntityHeader::Decode(this->GetMessageHeader() );
    if (liRetCode == 0)
    {
        SCP_RETURN_IF(aoMemoryBlock.GetLength() < this->GetContentLength(), ERR_CONTENT_LENGTH);
    
        this->GetBody().Reset();
        this->GetBody().Resize(this->GetContentLength() + 1 );
        
        apl_memcpy(this->GetBody().GetReadPtr(), aoMemoryBlock.GetReadPtr(), this->GetContentLength() );
        
        this->GetBody().SetWritePtr(this->GetContentLength() );
        aoMemoryBlock.SetReadPtr(this->GetContentLength() );
        
        *this->GetBody().GetWritePtr() = '\0';
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Response
CResponse::CResponse(apl_int_t aiType)
    : CScpMessage(aiType)
{
}

CResponse::~CResponse(void)
{
}

apl_int_t CResponse::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    char lacBuffer[SCP_MAX_BUFFER_LEN];

    apl_snprintf( 
        lacBuffer, sizeof(lacBuffer),
        "%s %s %"APL_PRIdINT"\r\n",
        this->GetMethod(),
        this->GetCmppVersion(),
        this->GetStatusCode() );

    SCP_RETURN_IF(Write(aoMemoryBlock, lacBuffer) != 0, WRN_NOT_ENOUGH_SPACE);
    
    SCP_RETURN_IF(CGeneralHeader::Encode(aoMemoryBlock) != 0, WRN_NOT_ENOUGH_SPACE);
    
    return 0;
}

apl_int_t CResponse::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    char* lpcMethod = NULL;
    char* lpcCmppVersion = NULL;
    char* lpcStatusCode = NULL;
    char* lpcEnd = NULL;
    char  lacBuffer[SCP_MAX_BUFFER_LEN];
    apl_ssize_t liResult = 0;
    
    SCP_RETURN_IF( (liResult = ReadLine(aoMemoryBlock, lacBuffer, sizeof(lacBuffer) ) ) <= 0, ERR_STATUS_LINE);

    lpcMethod = lacBuffer;
    if ( (lpcCmppVersion = (char*)apl_memchr(lpcMethod, ' ', liResult) ) == NULL)
    {
        return ERR_REQUEST_LINE;
    }
    *lpcCmppVersion++ = '\0';
    
    if ( (lpcStatusCode = (char*)apl_memchr(
        lpcCmppVersion, ' ', liResult - (lpcCmppVersion - lpcMethod) ) ) == NULL)
    {
        return ERR_REQUEST_LINE;
    }
    *lpcStatusCode++ = '\0';
    
    if ( (lpcEnd = (char*)apl_memchr(
        lpcStatusCode, ' ', liResult - (lpcStatusCode - lpcMethod) ) ) != NULL)
    {
        return ERR_REQUEST_LINE;
    }

    //SCP_RETURN_IF(this->GetType() != MethodToType(loTokenizer.GetField(0) ), ERR_STATUS_LINE);
    this->SetMethod(lpcMethod);
    this->SetCmppVersion(lpcCmppVersion);
    this->SetStatusCode( apl_strtoi32(lpcStatusCode, NULL, 10) );

    SCP_RETURN_IF( this->GetMessageHeader().Decode(aoMemoryBlock) != 0, ERR_MESSAGE_HEADER);
    
    SCP_RETURN_IF(CGeneralHeader::Decode(this->GetMessageHeader() ) != 0, ERR_GENERAL_HEADER);
        
    return 0;
}

/// Login-Response
CLoginResponse::CLoginResponse(void)
    : CResponse(MSG_TYPE_LOGIN_RSP)
{
}
    
apl_int_t CLoginResponse::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CResponse::Encode(aoMemoryBlock) ) != 0, liRetCode);

    SCP_RETURN_IF(CAuthResponseHeader::Encode(aoMemoryBlock) != 0, WRN_NOT_ENOUGH_SPACE);
    
    SCP_RETURN_IF(aoMemoryBlock.Write("\r\n", 2) != 2, WRN_NOT_ENOUGH_SPACE);
    
    return 0;
}

apl_int_t CLoginResponse::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CResponse::Decode(aoMemoryBlock) ) != 0, liRetCode);

    SCP_RETURN_IF(CAuthResponseHeader::Decode(this->GetMessageHeader() ) != 0, ERR_AUTH_RESPONSE_HEADER);
    
    return 0;
}

/// Auth-Response
CAuthResponse::CAuthResponse(void)
    : CResponse(MSG_TYPE_AUTHENTICATION_RSP)
{
}
    
apl_int_t CAuthResponse::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CResponse::Encode(aoMemoryBlock) ) != 0, liRetCode);
    
    SCP_RETURN_IF(aoMemoryBlock.Write("\r\n", 2) != 2, WRN_NOT_ENOUGH_SPACE);
    
    return 0;
}

apl_int_t CAuthResponse::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CResponse::Decode(aoMemoryBlock) ) != 0, liRetCode);
        
    return 0;
}

/// Logout-Response
CLogoutResponse::CLogoutResponse(void)
    : CResponse(MSG_TYPE_LOGOUT_RSP)
{
}
    
apl_int_t CLogoutResponse::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CResponse::Encode(aoMemoryBlock) ) != 0, liRetCode);
    
    SCP_RETURN_IF(aoMemoryBlock.Write("\r\n", 2) != 2, WRN_NOT_ENOUGH_SPACE);
    
    return 0;
}

apl_int_t CLogoutResponse::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CResponse::Decode(aoMemoryBlock) ) != 0, liRetCode);

    return 0;
}

/// Shakehand-Response
CShakehandResponse::CShakehandResponse(void)
    : CResponse(MSG_TYPE_SHAKEHAND_RSP)
{
}
    
apl_int_t CShakehandResponse::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CResponse::Encode(aoMemoryBlock) ) != 0, liRetCode);
    
    SCP_RETURN_IF(aoMemoryBlock.Write("\r\n", 2) != 2, WRN_NOT_ENOUGH_SPACE);
    
    return 0;
}

apl_int_t CShakehandResponse::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    char* lpcMethod = NULL;
    char* lpcCmppVersion = NULL;
    char  lacBuffer[SCP_MAX_BUFFER_LEN];
    apl_ssize_t liResult = 0;
    
    SCP_RETURN_IF( (liResult = ReadLine(aoMemoryBlock, lacBuffer, sizeof(lacBuffer) ) ) <= 0, ERR_STATUS_LINE);

    lpcMethod = lacBuffer;
    if ( (lpcCmppVersion = (char*)apl_memchr(lpcMethod, ' ', liResult) ) == NULL)
    {
        return ERR_REQUEST_LINE;
    }
    *lpcCmppVersion++ = '\0';

    //SCP_RETURN_IF(this->GetType() != MethodToType(loTokenizer.GetField(0) ), ERR_STATUS_LINE);
    
    this->SetCmppVersion(lpcCmppVersion );
    this->SetStatusCode(MSG_STATUS_OK);
    
    SCP_RETURN_IF( this->GetMessageHeader().Decode(aoMemoryBlock) != 0, ERR_MESSAGE_HEADER );
    
    SCP_RETURN_IF(CGeneralHeader::Decode(this->GetMessageHeader() ) != 0, ERR_GENERAL_HEADER);
        
    return 0;
}

/// Entity-Response
CEntityResponse::CEntityResponse(void)
    : CResponse(MSG_TYPE_RESPOND)
    , moBody(NULL, 0, acl::CMemoryBlock::DONT_DELETE)
{
}
    
apl_int_t CEntityResponse::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CResponse::Encode(aoMemoryBlock) ) != 0, liRetCode);
    
    if (this->GetBody().GetLength() > 0)
    {
        this->SetContentLength(this->GetBody().GetLength() );
    
        SCP_RETURN_IF(CEntityHeader::Encode(aoMemoryBlock) != 0, WRN_NOT_ENOUGH_SPACE);
    
        SCP_RETURN_IF(aoMemoryBlock.Write("\r\n", 2) != 2, WRN_NOT_ENOUGH_SPACE);
    
        SCP_RETURN_IF(
            aoMemoryBlock.Write(this->GetBody().GetReadPtr(), this->GetBody().GetLength() ) != (apl_ssize_t)this->GetBody().GetLength(),
            WRN_NOT_ENOUGH_SPACE );
    }
    else
    {
        SCP_RETURN_IF(aoMemoryBlock.Write("\r\n", 2) != 2, WRN_NOT_ENOUGH_SPACE);
    }

    return 0;
}

apl_int_t CEntityResponse::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    apl_int_t liRetCode = 0;
    
    SCP_RETURN_IF( (liRetCode = CResponse::Decode(aoMemoryBlock) ) != 0, liRetCode);

    liRetCode = CEntityHeader::Decode(this->GetMessageHeader() );
    if (liRetCode == 0)
    {
        SCP_RETURN_IF(aoMemoryBlock.GetLength() < this->GetContentLength(), ERR_CONTENT_LENGTH);
    
        this->GetBody().Reset();
        this->GetBody().Resize(this->GetContentLength() + 1 );
        
        apl_memcpy(this->GetBody().GetReadPtr(), aoMemoryBlock.GetReadPtr(), this->GetContentLength() );
        
        this->GetBody().SetWritePtr(this->GetContentLength() );
        aoMemoryBlock.SetReadPtr(this->GetContentLength() );
        
        *this->GetBody().GetWritePtr() = '\0';
    }
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
CInvalidMessage::CInvalidMessage( char const* apcMethod )
    : CScpMessage(MSG_TYPE_INVAILD)
{
    this->SetMethod(apcMethod);
}

apl_int_t CInvalidMessage::Encode( acl::CMemoryBlock& aoMemoryBlock )
{
    return 0;
}

apl_int_t CInvalidMessage::Decode( acl::CMemoryBlock& aoMemoryBlock )
{
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMessageMethodMap::CLess::operator () (std::string const& aoLeft, std::string const& aoRight)
{
    return apl_strcasecmp(aoLeft.c_str(), aoRight.c_str() ) < 0;
}

CMessageMethodMap::CMessageMethodMap(void)
{
    this->Insert("REQUEST",            MSG_TYPE_REQUEST);
    this->Insert("RESPOND",            MSG_TYPE_RESPOND);
    this->Insert("LOGIN",              MSG_TYPE_LOGIN);
    this->Insert("LOGIN_RSP",          MSG_TYPE_LOGIN_RSP);
    this->Insert("LOGOUT",             MSG_TYPE_LOGOUT);
    this->Insert("LOGOUT_RSP",         MSG_TYPE_LOGOUT_RSP);
    this->Insert("SHAKEHAND",          MSG_TYPE_SHAKEHAND);
    this->Insert("SHAKEHAND_RSP",      MSG_TYPE_SHAKEHAND_RSP);
    this->Insert("AUTHENTICATION",     MSG_TYPE_AUTHENTICATION);
    this->Insert("AUTHENTICATION_RSP", MSG_TYPE_AUTHENTICATION_RSP);
    this->Insert("TIMEOUT",            MSG_TYPE_TIMEOUT);
}
    
apl_int_t CMessageMethodMap::MethodToType( std::string const& aoMethod )
{
    std::map<std::string, apl_int_t>::iterator loIter = this->moMethodToType.find(aoMethod);
    
    return loIter != this->moMethodToType.end() ? loIter->second : -1;
}
    
char const* CMessageMethodMap::TypeToMethod( apl_int_t aiType )
{
    std::map<apl_int_t, std::string>::iterator loIter = this->moTypeToMethod.find(aiType);
    
    return loIter != this->moTypeToMethod.end() ? loIter->second.c_str() : "";
}

void CMessageMethodMap::Insert( std::string const& aoMethod, apl_int_t aiType )
{
    this->moMethodToType[aoMethod] = aiType;
    this->moTypeToMethod[aiType] = aoMethod;
}

apl_int_t MethodToType( std::string const& aoMethod )
{
    return acl::Instance<CMessageMethodMap>()->MethodToType(aoMethod);
}
    
char const* TypeToMethod( apl_int_t aiType )
{
    return acl::Instance<CMessageMethodMap>()->TypeToMethod(aiType);
}

CScpMessage* DecodeMessage( acl::CMemoryBlock& aoBlock )
{
    apl_int32_t liLength = 0;
    char* lpcReadPtr = aoBlock.GetReadPtr();
    char* lpcTempPtr = NULL;
    CScpMessage* lpoMessage = NULL;
    
    if (aoBlock.GetLength() < sizeof(liLength) )
    {
        return NULL;
    }
    
    apl_memcpy(&liLength, lpcReadPtr, sizeof(liLength) );
    liLength = apl_ntoh32(liLength);
    
    if ( (apl_int32_t)aoBlock.GetLength() < liLength)
    {
        return NULL;
    }
    
    aoBlock.SetReadPtr( sizeof(liLength) );

    if (liLength > (apl_int32_t)aoBlock.GetSize() )
    {
        ACL_NEW_ASSERT(lpoMessage, CInvalidMessage("TOBIG") );
        
        return lpoMessage;
    }
    
    if ( (lpcTempPtr = (char*)apl_memchr(aoBlock.GetReadPtr(), ' ', aoBlock.GetLength() ) ) == NULL)
    {
        ACL_NEW_ASSERT(lpoMessage, CInvalidMessage("UNKNOWN") );
        
        return lpoMessage;
    }
    
    std::string loMothed(aoBlock.GetReadPtr(), lpcTempPtr - aoBlock.GetReadPtr() );
    
    switch( MethodToType(loMothed) )
    {
        case MSG_TYPE_REQUEST: ACL_NEW_ASSERT(lpoMessage, CEntityRequest); break;
        case MSG_TYPE_LOGIN: ACL_NEW_ASSERT(lpoMessage, CLoginRequest); break;
        case MSG_TYPE_SHAKEHAND: ACL_NEW_ASSERT(lpoMessage, CShakehandRequest); break;
        case MSG_TYPE_LOGOUT: ACL_NEW_ASSERT(lpoMessage, CLogoutRequest); break;
        case MSG_TYPE_AUTHENTICATION: ACL_NEW_ASSERT(lpoMessage, CAuthRequest); break;
        case MSG_TYPE_RESPOND: ACL_NEW_ASSERT(lpoMessage, CEntityResponse); break;
        case MSG_TYPE_LOGIN_RSP: ACL_NEW_ASSERT(lpoMessage, CLoginResponse); break;
        case MSG_TYPE_SHAKEHAND_RSP: ACL_NEW_ASSERT(lpoMessage, CShakehandResponse); break;
        case MSG_TYPE_LOGOUT_RSP: ACL_NEW_ASSERT(lpoMessage, CLogoutResponse); break;
        case MSG_TYPE_AUTHENTICATION_RSP: ACL_NEW_ASSERT(lpoMessage, CAuthResponse); break;
        default:
        {
            ACL_NEW_ASSERT(lpoMessage, CInvalidMessage(loMothed.c_str() ) );
        }
    }
    
    if (lpoMessage->Decode(aoBlock) != 0)
    {
        aoBlock.SetReadPtr(lpcReadPtr);
        
        ACL_DELETE(lpoMessage);

        ACL_NEW_ASSERT(lpoMessage, CInvalidMessage(loMothed.c_str() ) );
    }

    if (aoBlock.GetLength() >= 2 && apl_memcmp(aoBlock.GetReadPtr(), "\r\n", 2) == 0)
    {
        aoBlock.SetReadPtr(2);
    }

    return lpoMessage;
}

}

ANF_NAMESPACE_END
