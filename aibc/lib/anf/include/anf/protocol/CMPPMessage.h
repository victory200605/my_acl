
/**
 * @file  CMPPMessage.h
 */
#ifndef CMPPMESSAGE_H
#define CMPPMESSAGE_H

#include "acl/Utility.h"
#include "anf/Utility.h"
#include "acl/DateTime.h"
#include "acl/Hashs.h"
#include "acl/stl/vector.h"
#include "acl/stl/string.h"

ANF_NAMESPACE_START

namespace cmpp
{
#define BUILD_GETTER_SETTER_MU8(member)                                 \
        apl_uint8_t Get##member() const                                 \
        {                                                               \
            return mu8##member;                                         \
        }                                                               \
                                                                        \
        void Set##member(apl_uint8_t au8Value)                          \
        {                                                               \
            mu8##member = au8Value;                                     \
        }


#define BUILD_GETTER_SETTER_MU16(member)                                \
        apl_uint16_t Get##member() const                                \
        {                                                               \
            return mu16##member;                                        \
        }                                                               \
                                                                        \
        void Set##member(apl_uint16_t au16Value)                        \
        {                                                               \
            mu16##member = au16Value;                                   \
        }

#define BUILD_GETTER_SETTER_MU32(member)                                \
        apl_uint32_t Get##member() const                                \
        {                                                               \
            return mu32##member;                                        \
        }                                                               \
                                                                        \
        void Set##member(apl_uint32_t au32Value)                        \
        {                                                               \
            mu32##member = au32Value;                                   \
        }

#define BUILD_GETTER_SETTER_MU64(member)                                \
        apl_uint64_t Get##member() const                                \
        {                                                               \
            return mu64##member;                                        \
        }                                                               \
                                                                        \
        void Set##member(apl_uint64_t au64Value)                        \
        {                                                               \
            mu64##member = au64Value;                                   \
        }

#define BUILD_GETTER_SETTER_CSTR(member)                                \
        const char * Get##member() const                                \
        {                                                               \
            return mac##member;                                         \
        }                                                               \
                                                                        \
        void Set##member( const char * apcValue)                        \
        {                                                               \
            apl_strncpy( mac##member, apcValue, sizeof(mac##member));   \
        }

#define BUILD_GETTER_SETTER_VARSTR(member)                              \
        const char * Get##member() const                                \
        {                                                               \
            return this->mac##member;                                   \
        }                                                               \
                                                                        \
        void Set##member( const void* apcValue)                         \
        {                                                               \
            apl_memcpy( mac##member, apcValue, sizeof(mac##member));   \
        }

#define BUILD_GETTER_SETTER_VARSTR2(member)                              \
        const char * Get##member() const                                \
        {                                                               \
            return this->mac##member;                                   \
        }                                                               \
                                                                        \
        void Set##member( const void* apcValue, apl_size_t auLen)       \
        {                                                               \
            apl_memcpy( mac##member, apcValue, auLen);   \
        }

#define CHECK_EQUALLEN_RETURN(explen, reallen) \
do \
{ \
    if(explen != reallen) \
    { \
        return -1; \
    } \
} while (0);

#define CHECK_LEN_RETURN(explen, reallen) \
do \
{ \
    if(explen < reallen) \
    { \
        return -1; \
    } \
} while (0);
///////////////////////////////////////////////////////////////////////////
#define CMPP_HEADER_LEN                    ((apl_uint32_t)12)
#define CMPP_MAX_CONTENT_LEN               ((apl_uint8_t)255)

#define CMPP_VERSION_2                     ((apl_uint8_t)0x20)
#define CMPP_VERSION_3                     ((apl_uint8_t)0x30)

#define CMPP_DEFAULT_DATE_FORMATE          "%m%d%H%M%S"
#define CMPP_SOURCE_ADDR_LEN               6
#define CMPP_ISMG_ID_LEN                   6
#define CMPP_SHARED_SECRET_LEN             32
#define CMPP_AUTHENTICATOR_LEN             16
#define CMPP_MSG_ID_LEN                    8
#define CMPP_SERVICE_ID_LEN                10
#define CMPP_TERMINAL_ID_LEN               32
#define CMPP_TERMINAL_PSEUDO_LEN           32
#define CMPP_MSG_SRC_LEN                   6
#define CMPP_FEE_TYPE_LEN                  2
#define CMPP_FEE_CODE_LEN                  6
#define CMPP_VALID_TIME_LEN                17
#define CMPP_AT_TIME_LEN                   17
#define CMPP_SRC_ID_LEN                    21
#define CMPP_LINK_ID_LEN                   20
#define CMPP_MAX_DEST_USR                  100

#define CMPP_MAX_PACKAGE                   10*1024

//Deliver
#define CMPP_DEST_ID_LEN                   21

//Status Report
#define CMPP_SR_STATUS_LEN                 7
#define CMPP_SR_TIME_LEN                   10

// CMPP_FWD.ciFwdType
#define FWD_TYPE_MT                        ((apl_uint8_t)0)
#define FWD_TYPE_MO                        ((apl_uint8_t)1)
#define FWD_TYPE_MT_STATUS                 ((apl_uint8_t)2)
#define FWD_TYPE_MO_STATUS                 ((apl_uint8_t)3)

///////////////////////////////////////////////////////////////////////////

#define CMPP_CONNECT                       ((apl_uint32_t)0x00000001)
#define CMPP_CONNECT_RESP                  ((apl_uint32_t)0x80000001)
#define CMPP_TERMINATE                     ((apl_uint32_t)0x00000002)
#define CMPP_TERMINATE_RESP                ((apl_uint32_t)0x80000002)
#define CMPP_SUBMIT                        ((apl_uint32_t)0x00000004)
#define CMPP_SUBMIT_RESP                   ((apl_uint32_t)0x80000004)
#define CMPP_DELIVER                       ((apl_uint32_t)0x00000005)
#define CMPP_DELIVER_RESP                  ((apl_uint32_t)0x80000005)
#define CMPP_QUERY                         ((apl_uint32_t)0x00000006)
#define CMPP_QUERY_RESP                    ((apl_uint32_t)0x80000006)
#define CMPP_CANCEL                        ((apl_uint32_t)0x00000007)
#define CMPP_CANCEL_RESP                   ((apl_uint32_t)0x80000007)
#define CMPP_ACTIVE_TEST                   ((apl_uint32_t)0x00000008)
#define CMPP_ACTIVE_TEST_RESP              ((apl_uint32_t)0x80000008)
#define CMPP_FWD                           ((apl_uint32_t)0x00000009)
#define CMPP_FWD_RESP                      ((apl_uint32_t)0x80000009)
#define CMPP_MT_ROUTE                      ((apl_uint32_t)0x00000010)
#define CMPP_MT_ROUTE_RESP                 ((apl_uint32_t)0x80000010)
#define CMPP_MO_ROUTE                      ((apl_uint32_t)0x00000011)
#define CMPP_MO_ROUTE_RESP                 ((apl_uint32_t)0x80000011)
#define CMPP_GET_MT_ROUTE                  ((apl_uint32_t)0x00000012)
#define CMPP_GET_MT_ROUTE_RESP             ((apl_uint32_t)0x80000012)
#define CMPP_MT_ROUTE_UPDATE               ((apl_uint32_t)0x00000013)
#define CMPP_MT_ROUTE_UPDATE_RESP          ((apl_uint32_t)0x80000013)
#define CMPP_MO_ROUTE_UPDATE               ((apl_uint32_t)0x00000014)
#define CMPP_MO_ROUTE_UPDATE_RESP          ((apl_uint32_t)0x80000014)
#define CMPP_PUSH_MT_ROUTE_UPDATE          ((apl_uint32_t)0x00000015)
#define CMPP_PUSH_MT_ROUTE_UPDATE_RESP     ((apl_uint32_t)0x80000015)
#define CMPP_PUSH_MO_ROUTE_UPDATE          ((apl_uint32_t)0x00000016)
#define CMPP_PUSH_MO_ROUTE_UPDATE_RESP     ((apl_uint32_t)0x80000016)
#define CMPP_GET_MO_ROUTE                  ((apl_uint32_t)0x00000017)
#define CMPP_GET_MO_ROUTE_RESP             ((apl_uint32_t)0x80000017)

#define RETURN_ERR_IF(conditon, errno) \
do \
{ \
    if(conditon) \
    { \
        return errno; \
    } \
}while(0);

enum EPDUErr
{
    ERR_CMPP_CONNECT_ENCODE         = -401,
    ERR_CMPP_CONNECT_DECODE         = -402,
    ERR_CMPP_CONNECT_RESP_ENCODE    = -403,
    ERR_CMPP_CONNECT_RESP_DECODE    = -404,
    ERR_CMPP_SUBMIT_ENCODE          = -405,
    ERR_CMPP_SUBMIT_DECODE          = -406,
    ERR_CMPP_SUBMIT_RESP_ENCODE     = -407,
    ERR_CMPP_SUBMIT_RESP_DECODE     = -408,
    ERR_CMPP_DELIVER_ENCODE         = -409,
    ERR_CMPP_DELIVER_DECODE         = -410,
    ERR_CMPP_DELIVER_RESP_ENCODE    = -411,
    ERR_CMPP_DELIVER_RESP_DECODE    = -412,
    ERR_CMPP_QUERY_ENCODE           = -413,
    ERR_CMPP_QUERY_DECODE           = -414,
    ERR_CMPP_QUERY_RESP_ENCODE      = -415,
    ERR_CMPP_QUERY_RESP_DECODE      = -416,
    ERR_CMPP_CANCEL_ENCODE          = -417,
    ERR_CMPP_CANCEL_DECODE          = -418,
    ERR_CMPP_CANCEL_RESP_ENCODE     = -419,
    ERR_CMPP_CANCEL_RESP_DECODE     = -420,
    ERR_CMPP_ACTIVETEST_RESP_ENCODE = -421,
    ERR_CMPP_ACTIVETEST_RESP_DECODE = -422,
    ERR_CMPP_FWD_ENCODE             = -423,
    ERR_CMPP_FWD_DECODE             = -424,
    ERR_CMPP_FWD_RESP_ENCODE        = -425,
    ERR_CMPP_FWD_RESP_DECODE        = -426,
};
///////////////////////////////////////////////////////////////////////////

/**
 * @class CCMPPHeader
 *
 * @brief CMPP Message Header
 */
class CCMPPHeader
{
public:
    CCMPPHeader(void);

    apl_ssize_t Encode(void* apPtr, apl_size_t aiLen) const;

    apl_ssize_t Decode(void const* apPtr, apl_size_t aiLen);

public:
    BUILD_GETTER_SETTER_MU32(TotalLength);
    BUILD_GETTER_SETTER_MU32(CommandID);
    BUILD_GETTER_SETTER_MU32(SequenceID);

private:
    apl_uint32_t    mu32TotalLength;
    apl_uint32_t    mu32CommandID;
    apl_uint32_t    mu32SequenceID;
};

/**
 * @class ICMPPPDUBase
 *
 * @brief the interface of all message body PDU
 */
class ICMPPPDUBase
{
public:
    virtual ~ICMPPPDUBase() {};

public:
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t auLen) const = 0;

    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t auLen) = 0;
};

/**
 * @class CCMPPConnectPDU
 *
 * @brief CMPP_CONNECT (SP->ISMG)
 */
class CCMPPConnectPDU : public ICMPPPDUBase
{
public:
    CCMPPConnectPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

    apl_uint32_t CalcAuthenticatorSource(
            char* apcAuthenticatorSource,
            const char* apcSourceAddr,
            const char* apcSharedSecret,
            apl_uint32_t  au32Timestamp = 0);

public:
    BUILD_GETTER_SETTER_CSTR(SourceAddr);
    BUILD_GETTER_SETTER_VARSTR(AuthenticatorSource);
    BUILD_GETTER_SETTER_MU8(Version);
    BUILD_GETTER_SETTER_MU32(Timestamp);

private:
    char         macSourceAddr[6+1];        //SP_ID
    char         macAuthenticatorSource[16];
    apl_uint8_t  mu8Version;
    apl_uint32_t mu32Timestamp;

    //char         macSharedSecret[CMPP_SHARED_SECRET_LEN +1];
};

class CCMPP20ConnectPDU : public CCMPPConnectPDU
{
};

class CCMPP30ConnectPDU : public CCMPPConnectPDU
{
};

/**
 * @class CCMPPConnectRespPDU
 *
 * @brief CMPP_CONNECT_RESP (ISMG->SP)
 */
class CCMPPConnectRespPDU : public ICMPPPDUBase
{
public:
    CCMPPConnectRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const {return 0;};

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen) {return 0;};

public:
    BUILD_GETTER_SETTER_VARSTR(AuthenticatorISMG);
    BUILD_GETTER_SETTER_MU8(Version);

protected:
    char         macAuthenticatorISMG[CMPP_AUTHENTICATOR_LEN];
    apl_uint8_t  mu8Version;
};

/**
 * @class CCMPP20ConnectRespPDU
 */
class CCMPP20ConnectRespPDU : public CCMPPConnectRespPDU
{
public:
    CCMPP20ConnectRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

    apl_int_t CalcAuthenticatorISMG(
            char* apcAuthenticatorISMG,
            apl_uint8_t au8Status,
            const char* apcAuthenticatorSource,
            const char* apcSharedSecret);

public:
    BUILD_GETTER_SETTER_MU8(Status);

private:
    apl_uint8_t mu8Status;
};

/**
 * @class CCMPP30ConnectRespPDU
 */
class CCMPP30ConnectRespPDU : public CCMPPConnectRespPDU
{
public:
    CCMPP30ConnectRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

    apl_int_t CalcAuthenticatorISMG(
            char* apcAuthenticatorISMG,
            apl_uint32_t au32Status,
            const char* apcAuthenticatorSource,
            const char* apcSharedSecret);

public:
    BUILD_GETTER_SETTER_MU32(Status);

private:
    apl_uint32_t mu32Status;
};

/**
 * @class CCMPPTerminatePDU
 *
 * @brief CMPP_TERMINATE (SP<->ISMG)
 */
class CCMPPTerminatePDU : public ICMPPPDUBase
{
public:
    CCMPPTerminatePDU(void) {};

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const {return 0;};

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen) {return 0;};
};

/**
 * @class CCMPPTerminateRespPDU
 *
 * @brief CMPP_TERMINATE_RESP (SP<->ISMG)
 */
class CCMPPTerminateRespPDU: public ICMPPPDUBase
{
public:
    CCMPPTerminateRespPDU(void) {};

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const {return 0;};

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen) {return 0;};
};

/**
 * @class CCMPPSubmitPDU
 *
 * @brief CMPP_SUBMIT (SP->ISMG)
 */
class CCMPPSubmitPDU : public ICMPPPDUBase
{
public:
    CCMPPSubmitPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const {return 0;};

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen) {return 0;};

public:
    BUILD_GETTER_SETTER_MU64(MsgID);
    BUILD_GETTER_SETTER_MU8(PkTotal);
    BUILD_GETTER_SETTER_MU8(PkNumber);
    BUILD_GETTER_SETTER_MU8(RegisteredDelivery);
    BUILD_GETTER_SETTER_MU8(MsgLevel);
    BUILD_GETTER_SETTER_CSTR(ServiceID);
    BUILD_GETTER_SETTER_MU8(FeeUserType);
    BUILD_GETTER_SETTER_MU8(TpPID);
    BUILD_GETTER_SETTER_MU8(TpUDHI);
    BUILD_GETTER_SETTER_MU8(MsgFmt);
    BUILD_GETTER_SETTER_CSTR(MsgSrc);
    BUILD_GETTER_SETTER_CSTR(FeeType);
    BUILD_GETTER_SETTER_CSTR(FeeCode);
    BUILD_GETTER_SETTER_CSTR(ValidTime);
    BUILD_GETTER_SETTER_CSTR(AtTime);
    BUILD_GETTER_SETTER_CSTR(SrcID);
    BUILD_GETTER_SETTER_MU8(DestUsrTL);

public:
    const char* GetMsgContent() const
    {
        return this->macMsgContent;
    }

    void SetMsgContent(const void* apvMsgContent, apl_size_t auLen)
    {
        this->mu8MsgLength = std::min(auLen, sizeof(this->macMsgContent));
        apl_memcpy(this->macMsgContent, apvMsgContent, this->mu8MsgLength);
    }

    apl_uint8_t GetMsgLength(void)
    {
        return this->mu8MsgLength;
    }

    void SetMsgLength(apl_uint8_t au8MsgLength)
    {
        this->mu8MsgLength = au8MsgLength;
    }

protected:
    apl_uint64_t    mu64MsgID;
    apl_uint8_t     mu8PkTotal;
    apl_uint8_t     mu8PkNumber;
    apl_uint8_t     mu8RegisteredDelivery;
    apl_uint8_t     mu8MsgLevel;
    char            macServiceID[10+1];
    apl_uint8_t     mu8FeeUserType;
    apl_uint8_t     mu8TpPID;
    apl_uint8_t     mu8TpUDHI;
    apl_uint8_t     mu8MsgFmt;
    char            macMsgSrc[6+1];
    char            macFeeType[2+1];
    char            macFeeCode[6+1];
    char            macValidTime[17+1];
    char            macAtTime[17+1];
    char            macSrcID[21+1];
    apl_uint8_t     mu8DestUsrTL;
    apl_uint8_t     mu8MsgLength;
    char            macMsgContent[CMPP_MAX_CONTENT_LEN + 1];
};

/**
 * @class CCMPP20SubmitPDU
 */
class CCMPP20SubmitPDU : public CCMPPSubmitPDU
{
public:
    CCMPP20SubmitPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    BUILD_GETTER_SETTER_CSTR(FeeTerminalID);
    BUILD_GETTER_SETTER_CSTR(Reserve);

public:
    apl_int_t SetDestTerminalID(const char* apcDestTerminalID, apl_size_t luIndex);
    const char* GetDestTerminalID(apl_size_t luIndex);

private:
    char            macFeeTerminalID[21+1];
    char            maacDestTerminalID[CMPP_MAX_DEST_USR][21+1];
    char            macReserve[8+1];
};

/**
 * @class CCMPP30SubmitPDU
 */
class CCMPP30SubmitPDU : public CCMPPSubmitPDU
{
public:
    CCMPP30SubmitPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    BUILD_GETTER_SETTER_CSTR(FeeTerminalID);
    BUILD_GETTER_SETTER_MU8(FeeTerminalType);
    BUILD_GETTER_SETTER_MU8(DestTerminalType);
    BUILD_GETTER_SETTER_CSTR(LinkID);

public:
    apl_int_t SetDestTerminalID(const char* apcDestTerminalID, apl_size_t luIndex);
    const char* GetDestTerminalID(apl_size_t luIndex);

private:
    char            macFeeTerminalID[32+1];
    apl_uint8_t     mu8FeeTerminalType;
    char            maacDestTerminalID[CMPP_MAX_DEST_USR][32+1];
    apl_uint8_t     mu8DestTerminalType;
    char            macLinkID[20+1];
};

/**
 * @class CCMPPSubmitRespPDU
 *
 * @brief CMPP_SUBMIT_RESP (ISMG->SP)
 */
class CCMPPSubmitRespPDU : public ICMPPPDUBase
{
public:
    CCMPPSubmitRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const {return 0;};

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen) {return 0;};

public:
    BUILD_GETTER_SETTER_MU64(MsgID);

protected:
    apl_uint64_t     mu64MsgID;
};

/**
 * @class CCMPP20SubmitRespPDU
 */
class CCMPP20SubmitRespPDU : public CCMPPSubmitRespPDU
{
public:
    CCMPP20SubmitRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    BUILD_GETTER_SETTER_MU8(Result);

private:
    apl_uint8_t   mu8Result;
};

/**
 * @class CCMPP30SubmitRespPDU
 */
class CCMPP30SubmitRespPDU : public CCMPPSubmitRespPDU
{
public:
    CCMPP30SubmitRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    BUILD_GETTER_SETTER_MU32(Result);

private:
    apl_uint32_t   mu32Result;
};

/**
 * @class CCMPPQueryPDU
 *
 * @brief CMPP_QUERY (SP->ISMG)
 */
class CCMPPQueryPDU : public ICMPPPDUBase
{
public:
    CCMPPQueryPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    BUILD_GETTER_SETTER_CSTR(Time);
    BUILD_GETTER_SETTER_MU8(QueryType);
    BUILD_GETTER_SETTER_CSTR(QueryCode);
    BUILD_GETTER_SETTER_CSTR(Reserve);

private:
    char            macTime[8+1];
    apl_uint8_t     mu8QueryType;
    char            macQueryCode[10+1];
    char            macReserve[8+1];
};

/**
 * @class CCMPPQueryRespPDU
 *
 * @brief CMPP_QUERY_RESP (ISMG->SP)
 */
class CCMPPQueryRespPDU : public ICMPPPDUBase
{
public:
    CCMPPQueryRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    BUILD_GETTER_SETTER_CSTR(Time);
    BUILD_GETTER_SETTER_MU8(QueryType);
    BUILD_GETTER_SETTER_CSTR(QueryCode);
    BUILD_GETTER_SETTER_MU32(MTTlMsg);
    BUILD_GETTER_SETTER_MU32(MTTlUsr);
    BUILD_GETTER_SETTER_MU32(MTScs);
    BUILD_GETTER_SETTER_MU32(MTWT);
    BUILD_GETTER_SETTER_MU32(MTFL);
    BUILD_GETTER_SETTER_MU32(MOScs);
    BUILD_GETTER_SETTER_MU32(MOWT);
    BUILD_GETTER_SETTER_MU32(MOFL);

private:
    char            macTime[8+1];
    apl_uint8_t     mu8QueryType;
    char            macQueryCode[10+1];
    apl_uint32_t    mu32MTTlMsg;
    apl_uint32_t    mu32MTTlUsr;
    apl_uint32_t    mu32MTScs;
    apl_uint32_t    mu32MTWT;
    apl_uint32_t    mu32MTFL;
    apl_uint32_t    mu32MOScs;
    apl_uint32_t    mu32MOWT;
    apl_uint32_t    mu32MOFL;
};

/**
 * @class CCMPPCancelPDU
 *
 * @brief CMPP_CANCEL (SP->ISMG)
 */
class CCMPPCancelPDU : public ICMPPPDUBase
{
public:
    CCMPPCancelPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    BUILD_GETTER_SETTER_MU64(MsgID);

private:
    apl_uint64_t    mu64MsgID;

};

/**
 * @class CCMPPCancelRespPDU
 *
 * @brief CMPP_CANCEL_RESP (ISMG->SP)
 */
class CCMPPCancelRespPDU : public ICMPPPDUBase
{
public:
    CCMPPCancelRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    BUILD_GETTER_SETTER_MU32(SuccessID);

private:
    apl_uint32_t        mu32SuccessID;

};

/**
 * @class CCMPPDeliverPDU
 *
 * @brief CMPP_DELIVER (ISMG->SP)
 */
class CCMPPDeliverPDU : public ICMPPPDUBase
{
public:
    CCMPPDeliverPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const {return 0;};

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen) {return 0;};

public:
    BUILD_GETTER_SETTER_MU64(MsgID);
    BUILD_GETTER_SETTER_CSTR(DestID);
    BUILD_GETTER_SETTER_CSTR(ServiceID);
    BUILD_GETTER_SETTER_MU8(TpPID);
    BUILD_GETTER_SETTER_MU8(TpUDHI);
    BUILD_GETTER_SETTER_MU8(MsgFmt);
    BUILD_GETTER_SETTER_MU8(RegisteredDelivery);

    BUILD_GETTER_SETTER_MU64(SRMsgID);
    BUILD_GETTER_SETTER_CSTR(SRStatus);
    BUILD_GETTER_SETTER_CSTR(SRSubmitTime);
    BUILD_GETTER_SETTER_CSTR(SRDoneTime);
    BUILD_GETTER_SETTER_MU32(SRSMSCSequence);

public:
    const char* GetMsgContent() const
    {
        return this->macMsgContent;
    }

    void SetMsgContent(const void* apvContent, apl_size_t auLen)
    {
        this->mu8MsgLength = std::min(auLen, sizeof(this->macMsgContent));
        apl_memcpy(this->macMsgContent, apvContent, this->mu8MsgLength);
    }

    apl_uint8_t GetMsgLength(void)
    {
        return this->mu8MsgLength;
    }

    void SetMsgLength(apl_uint8_t au8MsgLength)
    {
        this->mu8MsgLength = au8MsgLength;
    }

protected:
    apl_uint64_t    mu64MsgID;
    char            macDestID[21+1];
    char            macServiceID[10+1];
    apl_uint8_t     mu8TpPID;
    apl_uint8_t     mu8TpUDHI;
    apl_uint8_t     mu8MsgFmt;
    apl_uint8_t     mu8RegisteredDelivery;
    apl_uint8_t     mu8MsgLength;
    char            macMsgContent[CMPP_MAX_CONTENT_LEN+1];

    // For Stat Report //
    apl_uint64_t    mu64SRMsgID;
    char            macSRStatus[7+1];
    char            macSRSubmitTime[10+1];
    char            macSRDoneTime[10+1];
    apl_uint32_t    mu32SRSMSCSequence;
};
/**
 * @class CCMPP20DeliverPDU
 */
class CCMPP20DeliverPDU : public CCMPPDeliverPDU
{
public:
    CCMPP20DeliverPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    void EncodeSR(void);

    void DecodeSR(void);

public:
    BUILD_GETTER_SETTER_CSTR(SrcTerminalID);
    BUILD_GETTER_SETTER_CSTR(Reserved);

    BUILD_GETTER_SETTER_CSTR(SRDestTerminalID);
private:
    char            macSrcTerminalID[21+1];
    char            macReserved[8+1];

    // For Stat Report //
    char            macSRDestTerminalID[21+1];
};

/**
 * @class CCMPP30DeliverPDU
 */
class CCMPP30DeliverPDU : public CCMPPDeliverPDU
{
public:
    CCMPP30DeliverPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

    void EncodeSR(void);

    void DecodeSR(void);

public:
    BUILD_GETTER_SETTER_VARSTR2(SrcTerminalID);
    BUILD_GETTER_SETTER_MU8(SrcTerminalType);
    BUILD_GETTER_SETTER_CSTR(LinkID);

    BUILD_GETTER_SETTER_CSTR(SRDestTerminalID);
private:
    char            macSrcTerminalID[32+1];
    apl_uint8_t     mu8SrcTerminalType;
    char            macLinkID[20+1];

    // For Stat Report //
    char            macSRDestTerminalID[32+1];
};

/**
 * @class CCMPPDeliverRespPDU
 *
 * @brief CMPP_DELIVER_RESP (SP->ISMG)
 */
class CCMPPDeliverRespPDU : public ICMPPPDUBase
{
public:
    CCMPPDeliverRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const {return 0;};

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen) {return 0;};

public:
    BUILD_GETTER_SETTER_MU64(MsgID);

protected:
    apl_uint64_t   mu64MsgID;
};

/**
 * @class CCMPP20DeliverRespPDU
 */
class CCMPP20DeliverRespPDU : public CCMPPDeliverRespPDU
{
public:
    CCMPP20DeliverRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    BUILD_GETTER_SETTER_MU8(Result);

private:
    apl_uint8_t   mu8Result;
};

/**
 * @class CCMPP30DeliverRespPDU
 */
class CCMPP30DeliverRespPDU : public CCMPPDeliverRespPDU
{
public:
    CCMPP30DeliverRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    BUILD_GETTER_SETTER_MU32(Result);

private:
    apl_uint32_t   mu32Result;
};

/**
 * @class CCMPPActiveTestPDU
 *
 * @brief CMPP_ACTIVE_TEST ((SP<->ISMG))
 */
class CCMPPActiveTestPDU : public ICMPPPDUBase
{
public:
    CCMPPActiveTestPDU(void) {};

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const {return 0;};

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen) {return 0;};
};

/**
 * @class CCMPPActiveTestRespPDU
 *
 * @brief CMPP_ACTIVE_TEST_RESP (SP<->ISMG)
 */
class CCMPPActiveTestRespPDU : public ICMPPPDUBase
{
public:
    CCMPPActiveTestRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    BUILD_GETTER_SETTER_MU8(Reserved);

private:
    apl_uint8_t mu8Reserved;
};

/**
 * @class CCMPPFwdPDU
 *
 * @brief CMPP_FWD (ISMG->ISMG)
 */
class CCMPPFwdPDU : public ICMPPPDUBase
{
public:
    CCMPPFwdPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const {return 0;};

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen) {return 0;};

public:
    BUILD_GETTER_SETTER_CSTR(SourceID);
    BUILD_GETTER_SETTER_CSTR(DestinationID);
    BUILD_GETTER_SETTER_MU8(NodeCount);
    BUILD_GETTER_SETTER_MU8(MsgFwdType);
    BUILD_GETTER_SETTER_MU64(MsgID);
    BUILD_GETTER_SETTER_MU8(PkTotal);
    BUILD_GETTER_SETTER_MU8(PkNumber);
    BUILD_GETTER_SETTER_MU8(RegisteredDelivery);
    BUILD_GETTER_SETTER_MU8(MsgLevel);
    BUILD_GETTER_SETTER_CSTR(ServiceID);
    BUILD_GETTER_SETTER_MU8(FeeUserType);
    BUILD_GETTER_SETTER_CSTR(FeeTerminalID);
    BUILD_GETTER_SETTER_MU8(TpPID);
    BUILD_GETTER_SETTER_MU8(TpUDHI);
    BUILD_GETTER_SETTER_MU8(MsgFmt);
    BUILD_GETTER_SETTER_CSTR(MsgSrc);
    BUILD_GETTER_SETTER_CSTR(FeeType);
    BUILD_GETTER_SETTER_CSTR(FeeCode);
    BUILD_GETTER_SETTER_CSTR(ValidTime);
    BUILD_GETTER_SETTER_CSTR(AtTime);
    BUILD_GETTER_SETTER_CSTR(SrcTerminalID);
    BUILD_GETTER_SETTER_MU8(DestUsrTL);
    BUILD_GETTER_SETTER_CSTR(DestTerminalID);

    // For Stat Report //
    BUILD_GETTER_SETTER_MU64(SRMsgID);
    BUILD_GETTER_SETTER_CSTR(SRStatus);
    BUILD_GETTER_SETTER_CSTR(SRSubmitTime);
    BUILD_GETTER_SETTER_CSTR(SRDoneTime);
    BUILD_GETTER_SETTER_MU32(SRSMSCSequence);

public:
    const char* GetMsgContent() const
    {
        return this->macMsgContent;
    }

    void SetMsgContent(const void* apvMsgContent, apl_size_t auLen)
    {
        this->mu8MsgLength = std::min(auLen, sizeof(this->macMsgContent));
        apl_memcpy(this->macMsgContent, apvMsgContent, this->mu8MsgLength);
    }

    apl_uint8_t GetMsgLength(void)
    {
        return this->mu8MsgLength;
    }

    void SetMsgLength(apl_uint8_t au8MsgLength)
    {
        this->mu8MsgLength = au8MsgLength;
    }

protected:
    char            macSourceID[6+1];
    char            macDestinationID[6+1];
    apl_uint64_t    mu64MsgID;
    apl_uint8_t     mu8NodeCount;
    apl_uint8_t     mu8MsgFwdType;
    apl_uint8_t     mu8PkTotal;
    apl_uint8_t     mu8PkNumber;
    apl_uint8_t     mu8RegisteredDelivery;
    apl_uint8_t     mu8MsgLevel;
    char            macServiceID[10+1];
    apl_uint8_t     mu8FeeUserType;
    char            macFeeTerminalID[21+1];
    apl_uint8_t     mu8TpPID;
    apl_uint8_t     mu8TpUDHI;
    apl_uint8_t     mu8MsgFmt;
    char            macMsgSrc[6+1];
    char            macFeeType[2+1];
    char            macFeeCode[6+1];
    char            macValidTime[17+1];
    char            macAtTime[17+1];
    char            macSrcTerminalID[21+1];
    apl_uint8_t     mu8DestUsrTL;
    char            macDestTerminalID[21+1];
    apl_uint8_t     mu8MsgLength;
    char            macMsgContent[CMPP_MAX_CONTENT_LEN+1];

    // For Stat Report //
    apl_uint64_t    mu64SRMsgID;
    char            macSRStatus[7+1];
    char            macSRSubmitTime[10+11];
    char            macSRDoneTime[10+11];
    apl_uint32_t    mu32SRSMSCSequence;
};

/**
 * @class CCMPP20FwdPDU
 */
class CCMPP20FwdPDU : public CCMPPFwdPDU
{
public:
    CCMPP20FwdPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

    void EncodeMOSR(void);

    void DecodeMOSR(void);

    void EncodeMTSR(void);

    void DecodeMTSR(void);

public:
    BUILD_GETTER_SETTER_CSTR(Reserved);
    BUILD_GETTER_SETTER_CSTR(SRDestTerminalID);

private:
    char            macReserved[8+1];

    // For Stat Report //
    char            macSRDestTerminalID[21+1];
};

/**
 * @class CCMPP30FwdPDU
 */
class CCMPP30FwdPDU : public CCMPPFwdPDU
{
public:
    CCMPP30FwdPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

    void EncodeMOSR(void);

    void DecodeMOSR(void);

    void EncodeMTSR(void);

    void DecodeMTSR(void);

public:
    BUILD_GETTER_SETTER_VARSTR2(FeeTerminalPseudo);
    BUILD_GETTER_SETTER_MU8(FeeTerminalUserType);
    BUILD_GETTER_SETTER_VARSTR2(SrcTerminalPseudo);
    BUILD_GETTER_SETTER_MU8(SrcTerminalUserType);
    BUILD_GETTER_SETTER_MU8(SrcTerminalType);
    BUILD_GETTER_SETTER_VARSTR2(DestTerminalPseudo);
    BUILD_GETTER_SETTER_MU8(DestTerminalUserType);
    BUILD_GETTER_SETTER_CSTR(LinkID);
    BUILD_GETTER_SETTER_CSTR(SRDestTerminalID);

private:
    char            macFeeTerminalPseudo[32+1];
    apl_uint8_t     mu8FeeTerminalUserType;
    char            macSrcTerminalPseudo[32+1];
    apl_uint8_t     mu8SrcTerminalUserType;
    apl_uint8_t     mu8SrcTerminalType;
    char            macDestTerminalPseudo[32+1];
    apl_uint8_t     mu8DestTerminalUserType;
    char            macLinkID[20+1];

    // For Stat Report //
    char            macSRDestTerminalID[32+1];
};

/**
 * @class CCMPPFwdRespPDU
 *
 * @brief CMPP_FWD_RESP (ISMG->ISMG)
 */
class CCMPPFwdRespPDU : public ICMPPPDUBase
{
public:
    CCMPPFwdRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const {return 0;};

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen) {return 0;};

public:
    BUILD_GETTER_SETTER_MU64(MsgID);
    BUILD_GETTER_SETTER_MU8(PkTotal);
    BUILD_GETTER_SETTER_MU8(PkNumber);

protected:
    apl_uint64_t mu64MsgID;
    apl_uint8_t  mu8PkTotal;
    apl_uint8_t  mu8PkNumber;
};

/**
 * @class CCMPP20FwdRespPDU
 */
class CCMPP20FwdRespPDU : public CCMPPFwdRespPDU
{
public:
    CCMPP20FwdRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    BUILD_GETTER_SETTER_MU8(Result);

private:
    apl_uint8_t mu8Result;
};

/**
 * @class CCMPP30FwdRespPDU
 */
class CCMPP30FwdRespPDU : public CCMPPFwdRespPDU
{
public:
    CCMPP30FwdRespPDU(void);

    virtual apl_ssize_t Encode(void* apvPtr, apl_size_t auLen) const;

    virtual apl_ssize_t Decode(const void* apvPtr, apl_size_t auLen);

public:
    BUILD_GETTER_SETTER_MU32(Result);

private:
    apl_uint32_t mu32Result;
};

/**
 * @class CCMPPMessage
 */
class CCMPPMessage
{
public:
    CCMPPMessage(void);

    virtual ~CCMPPMessage(void);

public:
    apl_uint32_t GetTotalLength(void) const;
    void SetTotalLength(apl_uint32_t au32TotalLength);

    apl_uint32_t GetCommandID(void) const;
    void SetCommandID(apl_uint32_t au32CommandID);

    apl_uint32_t GetSequenceID(void) const;
    void SetSequenceID(apl_uint32_t au32SequenceID);

    apl_uint32_t GetMessageID(void) const { return GetSequenceID(); };
    void SetMessageID(apl_uint32_t au32SequenceID) { SetSequenceID(au32SequenceID); };

    ICMPPPDUBase* GetPDU(void);
    void SetPDU(ICMPPPDUBase* apoPDU);

    const char* GetCMPPMsgCmdStr(void);

protected:
    ICMPPPDUBase*   mpoPDU;
    CCMPPHeader moHeader;
};

class CCMPP20Message : public CCMPPMessage
{
public:
    virtual ~CCMPP20Message(void) { }
    apl_ssize_t Encode(void* apvPtr, apl_size_t auLen);
    apl_ssize_t Decode(void const* apvPtr, apl_size_t auLen);

public:
    apl_int_t AllocPDU(void);
    bool CheckLength(apl_size_t auLen);
};

class CCMPP30Message : public CCMPPMessage
{
public:
    virtual ~CCMPP30Message(void) { }
    apl_ssize_t Encode(void* apvPtr, apl_size_t auLen);
    apl_ssize_t Decode(void const* apvPtr, apl_size_t auLen);

public:
    apl_int_t AllocPDU(void);
};

} //namespace cmpp

ANF_NAMESPACE_END

#endif //CMPPMESSAGE_H
