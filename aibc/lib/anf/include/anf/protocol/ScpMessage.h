
#ifndef ANF_SCPMESSAGE_H
#define ANF_SCPMESSAGE_H

#include "acl/Stream.h"
#include "acl/DateTime.h"
#include "acl/MemoryBlock.h"
#include "acl/StrAlgo.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"
#include "acl/stl/map.h"
#include "anf/Utility.h"

ANF_NAMESPACE_START

namespace scp{

//status code
const apl_int_t MSG_STATUS_OK                             = 200;
const apl_int_t MSG_STATUS_ACCEPTED                       = 202;
const apl_int_t MSG_STATUS_NO_CONTENT                     = 204;
const apl_int_t MSG_STATUS_REGISTERED_OK                  = 205;
const apl_int_t MSG_STATUS_REGISTERED_NOT_OK              = 206;
const apl_int_t MSG_STATUS_LOGOUT_OK                      = 207;
const apl_int_t MSG_STATUS_LOGOUT_NOT_OK                  = 208;
const apl_int_t MSG_STATUS_BAD_REQUEST                    = 400;
const apl_int_t MSG_STATUS_UNAUTHORIZED                   = 401;
const apl_int_t MSG_STATUS_FORBIDDEN                      = 403;
const apl_int_t MSG_STATUS_METHOD_NOT_ALLOWED             = 405;
const apl_int_t MSG_STATUS_REQUEST_TIMEOUT                = 408;
const apl_int_t MSG_STATUS_LENGTH_REQUIRED                = 411;
const apl_int_t MSG_STATUS_REQUEST_ENTITY_TOO_LARGE       = 413;
const apl_int_t MSG_STATUS_SERVER_ERROR                   = 500;
const apl_int_t MSG_STATUS_NOT_IMPLEMENTED                = 501;
const apl_int_t MSG_STATUS_SCPLINKERR                     = 502;
const apl_int_t MSG_STATUS_SERVICE_NOT_AVAILABLE          = 503;
const apl_int_t MSG_STATUS_CMPP_VERSION_NOT_SUPPORTED     = 505;
const apl_int_t MSG_STATUS_SERVER_TIMEOUT                 = 506;

//message type
const apl_int_t MSG_TYPE_REQUEST                          = 0;
const apl_int_t MSG_TYPE_LOGIN                            = 1;
const apl_int_t MSG_TYPE_SHAKEHAND                        = 2;
const apl_int_t MSG_TYPE_LOGOUT                           = 3;
const apl_int_t MSG_TYPE_AUTHENTICATION                   = 4;
const apl_int_t MSG_TYPE_RESPOND                          = 5;
const apl_int_t MSG_TYPE_LOGIN_RSP                        = 6;
const apl_int_t MSG_TYPE_SHAKEHAND_RSP                    = 7;
const apl_int_t MSG_TYPE_LOGOUT_RSP                       = 8;
const apl_int_t MSG_TYPE_AUTHENTICATION_RSP               = 9;
const apl_int_t MSG_TYPE_TIMEOUT                          = 10;
const apl_int_t MSG_TYPE_INVAILD                          = 11;

//warning
const apl_int_t WRN_NOT_ENOUGH_SPACE                      =  90000;
                                                          
//error no                                                
const apl_int_t ERR_REQUEST_LINE                          = -90001;
const apl_int_t ERR_STATUS_LINE                           = -90002;
const apl_int_t ERR_GENERAL_HEADER                        = -90003;
const apl_int_t ERR_AUTH_REQUEST_HEADER                   = -90004;
const apl_int_t ERR_LOGOUT_REQUEST_HEADER                 = -90005;
const apl_int_t ERR_AUTH_RESPONSE_HEADER                  = -90006;
const apl_int_t ERR_RETRYAFTER_RESPONSE_HEADER            = -90007;
const apl_int_t ERR_ENTITY_HEADER                         = -90008;
const apl_int_t ERR_MESSAGE_HEADER                        = -90009;
const apl_int_t ERR_CONTENT_LENGTH                        = -90010;
const apl_int_t ERR_MESSAGE_LENGTH                        = -90011;
const apl_int_t ERR_INVALID_MESSAGE                       = -90012;

const apl_int_t SCP_FIELD_LEN = 128;

//define Version
const char* const CMPP_VERSION = "CMPP/1.2";

///operator macro    
#define SCP_DECLARE_MEMBER_OPERATOR_N( member ) \
    void Set##member( apl_int_t ai##member )              { this->mi##member = ai##member; } \
    apl_int_t  Get##member()const                         { return this->mi##member; }

#define SCP_DECLARE_MEMBER_OPERATOR_U( member ) \
    void Set##member( apl_size_t au##member )             { this->mu##member = au##member; } \
    apl_size_t  Get##member()const                        { return this->mu##member; }

#define SCP_DECLARE_MEMBER_OPERATOR_U32( member ) \
    void Set##member( apl_uint32_t au##member )           { this->mu##member = au##member; } \
    apl_uint32_t  Get##member()const                      { return this->mu##member; }

#define SCP_DECLARE_MEMBER_OPERATOR_B( member ) \
    void Set##member( bool abIs##member )                 { this->mbIs##member = abIs##member; } \
    bool  Is##member()const                               { return this->mbIs##member; }
    
#define SCP_DECLARE_MEMBER_OPERATOR_S( member ) \
    void Set##member( const char* apc##member )           { this->mo##member = apc##member; acl::stralgo::TrimIf(this->mo##member, IsQuote); } \
    const char* Get##member()const                        { return this->mo##member.c_str(); }

#define SCP_DECLARE_MEMBER_OPERATOR_CS( member ) \
    void Set##member( const char* apc##member )           { apl_strncpy( this->mac##member, apc##member, sizeof(this->mac##member) ); } \
    const char* Get##member()const                        { return this->mac##member; }

#define SCP_DECLARE_MEMBER_OPERATOR_VS( member ) \
    void Push##member( const char* apc##member )          { this->mo##member.push_back( apc##member ); acl::stralgo::TrimIf(this->mo##member.back(), IsQuote); } \
    size_t Get##member##Count()const                      { return this->mo##member.size(); } \
    const char* Get##member( apl_size_t aiN )const        { return this->mo##member[aiN].c_str(); }

#define SCP_DECLARE_MEMBER_OPERATOR_MB( member ) \
    acl::CMemoryBlock& Get##member()                      { return this->mo##member; }
///end

//inline function
inline bool IsNumber( char ac ) { return (ac >= '0' && ac <= '9'); }
inline bool IsQuote( char ac )  { return (ac == ' ' || ac == '"' || ac == '\''); }
inline bool IsSpace( char ac )  { return ac == ' ' || ac == '\t';}
inline bool IsClear( char ac )  { return IsSpace(ac) || IsQuote(ac);  }

/// common-header
class CMessageHeader
{
public:
    struct CNode
    {
        CNode(void);
        
        void Set( const char* apcKey, const char* apcValue );
        
        char const* GetKey(void);
        
        char const* GetValue(void);
        
        CNode* GetNext(void);
        
        char macKey[128];
        char macValue[256];
        
        CNode* mpoNext;
    };
    
public:
    CMessageHeader( apl_size_t auBktNum = 100 );
    
    ~CMessageHeader(void);
    
    apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );
    
    CNode* Find( const char* apcKey ) const;
    
    bool Insert( const char* apcKey, const char* atValue );
    
    void Clear(void);

public:
    apl_size_t muBktNum;
    CNode**    mppoBucket;
    CNode*     mpoElements;
    apl_size_t muElemNum;
};

/// scp-general-header
class CGeneralHeader
{
public:
    CGeneralHeader(void);
    
    apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    apl_int_t Decode( CMessageHeader const& aoMessageHeader );
    
    SCP_DECLARE_MEMBER_OPERATOR_U32(CSeq);
    SCP_DECLARE_MEMBER_OPERATOR_CS(From);
    SCP_DECLARE_MEMBER_OPERATOR_CS(To);
    SCP_DECLARE_MEMBER_OPERATOR_CS(Timestamp);
    SCP_DECLARE_MEMBER_OPERATOR_N(ServiceKey); 
    
    void UpdateTimestamp(void);
    
protected:
    apl_uint32_t muCSeq;
    char  macFrom[SCP_FIELD_LEN];
    char  macTo[SCP_FIELD_LEN];
    char  macTimestamp[SCP_FIELD_LEN];
    apl_int_t    miServiceKey;
};

///scp-auth-request-header
class CAuthRequestHeader
{
public:
    CAuthRequestHeader(void);
    
    apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    apl_int_t Decode( CMessageHeader const& aoMessageHeader );

    SCP_DECLARE_MEMBER_OPERATOR_S(Account);
    SCP_DECLARE_MEMBER_OPERATOR_S(AuthDigest);
    SCP_DECLARE_MEMBER_OPERATOR_S(Algorithm);
    SCP_DECLARE_MEMBER_OPERATOR_S(Nonce);

protected:
    std::string moAccount;
    std::string moAuthDigest;
    std::string moAlgorithm;
    std::string moNonce;
};

///scp-logout-request-header
class CLogoutRequestHeader
{
public:
    CLogoutRequestHeader(void);
    
    apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    apl_int_t Decode( CMessageHeader const& aoMessageHeader );
    
    SCP_DECLARE_MEMBER_OPERATOR_S(User);

protected:
    std::string moUser;
};

///scp-auth-response-header
class CAuthResponseHeader
{
public:
    CAuthResponseHeader(void);
        
    apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    apl_int_t Decode( CMessageHeader const& aoMessageHeader );
    
    SCP_DECLARE_MEMBER_OPERATOR_S(Nonce);
    SCP_DECLARE_MEMBER_OPERATOR_VS(Algorithm);

protected:
    std::string moNonce;
    std::vector<std::string> moAlgorithm;
};

///scp-retry-after-response-header
class CRetryAfterResponseHeader
{
public:
    CRetryAfterResponseHeader(void);
        
    apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    apl_int_t Decode( CMessageHeader const& aoMessageHeader );
    
    SCP_DECLARE_MEMBER_OPERATOR_N(RetryAfter);

protected:
    apl_int_t miRetryAfter;
};

///scp-entity-header
class CEntityHeader
{
public:
    CEntityHeader(void);
    
    apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    apl_int_t Decode( CMessageHeader const& aoMessageHeader );
    
    SCP_DECLARE_MEMBER_OPERATOR_U(ContentLength);
    SCP_DECLARE_MEMBER_OPERATOR_CS(ContentType);
    
protected:
    apl_size_t  muContentLength;
    char macContentType[SCP_FIELD_LEN];
};

//Make autn digest
std::string MakeAuthDigest(
    const char* apcAccount,
    const char* apcPassword,
    const char* apcNonce,
    const char* apcTimestamp );

///////////////////////////////////////////////////////////////////////////////////////////////    
//////////////////////////////////////////// message //////////////////////////////////////////
class CScpMessage : public CGeneralHeader
{
public:
    CScpMessage( apl_int_t aiType );
    
    virtual ~CScpMessage(void);
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock ) = 0;
    
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock ) = 0;
    
    virtual std::string GetMessageType();

    virtual void SetMessageID(apl_size_t auMsgId);

    virtual apl_size_t GetMessageID();
    
    bool CheckType(void);
    
    SCP_DECLARE_MEMBER_OPERATOR_CS(CmppVersion);

    apl_int_t GetType(void)                               { return this->miType; }
    
    void SetMethod( char const* apcMethod );
    
    char const* GetMethod(void);

protected:
    CMessageHeader& GetMessageHeader(void)                { return this->moMessageHeader; }

private:
    apl_int_t miType;
    char macCmppVersion[SCP_FIELD_LEN];
    char macMethod[SCP_FIELD_LEN];
    CMessageHeader moMessageHeader;
};

/// request
class CRequest : public CScpMessage
{
public:
    CRequest( apl_int_t aiType );
    
    virtual ~CRequest(void);
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );
};

/// Login-Request
class CLoginRequest : public CRequest
{
public:
    CLoginRequest(void);
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );
};

/// Auth-Request
class CAuthRequest : public CRequest, public CAuthRequestHeader
{
public:
    CAuthRequest(void);
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );
};

/// Logout-Request
class CLogoutRequest : public CRequest, public CLogoutRequestHeader
{
public:
    CLogoutRequest(void);
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );
};

/// Shakehand-Request
class CShakehandRequest : public CRequest
{
public:
    CShakehandRequest(void);
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );
};

/// Entity-Request
class CEntityRequest : public CRequest, public CEntityHeader
{
public:
    CEntityRequest(void);
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );
    
    SCP_DECLARE_MEMBER_OPERATOR_MB(Body)
    
private:
    acl::CMemoryBlock moBody;
};

/// response
class CResponse : public CScpMessage
{
public:
    CResponse(apl_int_t aiType);
    
    virtual ~CResponse(void);
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );
    
    SCP_DECLARE_MEMBER_OPERATOR_N(StatusCode);
private:
    apl_int_t   miStatusCode;
};

/// Login-Response
class CLoginResponse : public CResponse, public CAuthResponseHeader
{
public:
    CLoginResponse(void);
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );
};

/// Auth-Response
class CAuthResponse : public CResponse
{
public:
    CAuthResponse(void);
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );
};

/// Logout-Response
class CLogoutResponse : public CResponse
{
public:
    CLogoutResponse(void);
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );
};

/// Shakehand-Response
class CShakehandResponse : public CResponse
{
public:
    CShakehandResponse(void);
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );
};

/// Entity-Response
class CEntityResponse : public CResponse, public CEntityHeader
{
public:
    CEntityResponse(void);
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );
    
    SCP_DECLARE_MEMBER_OPERATOR_MB(Body)
    
private:
    acl::CMemoryBlock moBody;
};

/// Invalid-message
class CInvalidMessage : public CScpMessage
{
public:
    CInvalidMessage( char const* apcMethod );
    
    virtual apl_int_t Encode( acl::CMemoryBlock& aoMemoryBlock );
    virtual apl_int_t Decode( acl::CMemoryBlock& aoMemoryBlock );

    SCP_DECLARE_MEMBER_OPERATOR_MB(Buffer)

private:
    acl::CMemoryBlock moBuffer;
};

/////////////////////////////////////////////////////////////////////////////////////////////
class CMessageMethodMap
{
public:
    struct CLess
    {
        bool operator () (std::string const& aoLeft, std::string const& aoRight);
    };
    
public:
    CMessageMethodMap(void);
    
    apl_int_t MethodToType( std::string const& aoMethod );
    
    char const* TypeToMethod( apl_int_t aiType );

protected:
    void Insert(std::string const& aoMethod, apl_int_t aiType);
    
private:
    std::map<std::string, apl_int_t> moMethodToType;
    std::map<apl_int_t, std::string> moTypeToMethod;
};

apl_int_t MethodToType( std::string const& aoMethod );
    
char const* TypeToMethod( apl_int_t aiType );

CScpMessage* DecodeMessage( acl::CMemoryBlock& aoBlock );

}

ANF_NAMESPACE_END

#endif//ANF_SCP_MESSAGE_H
