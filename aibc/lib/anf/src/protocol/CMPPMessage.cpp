 
/**
 * @file CMPPMessage.cpp
 */

#include "anf/protocol/CMPPMessage.h"

ANF_NAMESPACE_START

namespace cmpp {

// ---------------- public functions : DecodeField ---------------- //
apl_ssize_t DecodeFieldStr(char* const apDest, const char* const apcSrc, apl_size_t aiLen)
{
    apl_memcpy(apDest, apcSrc, aiLen);
    apDest[aiLen] = '\0';

    return aiLen;
}

inline apl_ssize_t DecodeField(apl_uint8_t* const aiDest, const void* const apcSrc)
{
    *aiDest = *(apl_uint8_t *)apcSrc;

    return 1;   // 1 byte decoded
}


inline apl_ssize_t DecodeField(apl_uint16_t* const aiDest, const void* const apcSrc)
{
    apl_uint16_t lu16;

    apl_memcpy(&lu16, apcSrc, 2);

    *aiDest = apl_ntoh16(lu16);

    return 2;   // 2 bytes decoded
}


inline apl_ssize_t DecodeField(apl_uint32_t* const aiDest, const void* const apcSrc)
{
    apl_uint32_t lu32;

    apl_memcpy(&lu32, apcSrc, 4);

    *aiDest = apl_ntoh32(lu32);

    return 4;   // 4 bytes decoded
}

inline apl_ssize_t DecodeField(apl_uint64_t* const aiDest, const void* const apcSrc)
{
    apl_uint64_t lu64;

    apl_memcpy(&lu64, apcSrc, 8);

    *aiDest = apl_ntoh64(lu64);

    return 8;   // 8 bytes decoded
}

inline apl_ssize_t DecodeFieldMsgId(apl_uint64_t* const aiDest, const void* const apcSrc)
{
    apl_uint64_t lu64;

    apl_memcpy(&lu64, apcSrc, 8);

    *aiDest = lu64;

    return 8;   // 8 bytes decoded
}
// ---------------- public functions : EncodeField ---------------- //

apl_ssize_t EncodeFieldStr(char* const apDest, const char* const apcSrc, apl_size_t aiLen)
{
    apl_memcpy(apDest, apcSrc, aiLen);
    apDest[aiLen] = '\0';

    return aiLen;
}

inline apl_ssize_t EncodeField( void* const apDest, apl_uint8_t const aiSrc)
{
    *(apl_uint8_t *)apDest = aiSrc;

    return 1;   // 1 byte encoded
}

inline apl_ssize_t EncodeField(void* const apcDest, apl_uint16_t const aiSrc)
{
    apl_uint16_t lu16;

    lu16 = apl_hton16(aiSrc);

    apl_memcpy(apcDest, &lu16, 2);

    return 2;   // 2 bytes encoded
}

inline apl_ssize_t EncodeField(void* const apcDest, apl_uint32_t const aiSrc)
{
    apl_uint32_t lu32;

    lu32 = apl_hton32(aiSrc);

    apl_memcpy(apcDest, &lu32, 4);

    return 4;   // 4 bytes encoded
}

inline apl_ssize_t EncodeField(void* const apcDest, apl_uint64_t const aiSrc)
{
    apl_uint64_t lu64;

    lu64 = apl_hton64(aiSrc);

    apl_memcpy(apcDest, &lu64, 8);

    return 8;   // 8 bytes encoded
}

inline apl_ssize_t EncodeFieldMsgId(void* const apcDest, apl_uint64_t const aiSrc)
{
    apl_memcpy(apcDest, &aiSrc, 8);

    return 8;   // 8 bytes encoded
}

// ---------------- CCMPPHeader ---------------- //
CCMPPHeader::CCMPPHeader()
    : mu32TotalLength(0)
    , mu32CommandID(0)
    , mu32SequenceID(0)
{
}

apl_ssize_t CCMPPHeader::Decode(const void* apvPtr, apl_size_t auLen)
{
    if( auLen < CMPP_HEADER_LEN)
    {
        return -1;
    }

    const char* lpcPtr = (const char *)apvPtr;

    lpcPtr += DecodeField(&this->mu32TotalLength, lpcPtr);
    lpcPtr += DecodeField(&this->mu32CommandID, lpcPtr);
    lpcPtr += DecodeField(&this->mu32SequenceID, lpcPtr);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPPHeader::Encode(void* apvPtr, apl_size_t auLen) const
{
    if( auLen < CMPP_HEADER_LEN)
    {
        return -1;
    }

    char* lpcPtr = (char *) apvPtr;

    lpcPtr += EncodeField(lpcPtr, this->mu32TotalLength);
    lpcPtr += EncodeField(lpcPtr, this->mu32CommandID);
    lpcPtr += EncodeField(lpcPtr, this->mu32SequenceID);

    return lpcPtr - (char *)apvPtr;
}

// ---------------- CCMPPConnectPDU ---------------- //
CCMPPConnectPDU::CCMPPConnectPDU(void)
    : mu8Version(0)
    , mu32Timestamp(0)
{
    apl_memset(macSourceAddr, 0, sizeof(macSourceAddr));
    apl_memset(macAuthenticatorSource, 0, sizeof(macAuthenticatorSource));
}

apl_uint32_t CCMPPConnectPDU::CalcAuthenticatorSource(
        char* apcAuthenticatorSource,
        const char* apcSourceAddr,
        const char* apcSharedSecret,
        apl_uint32_t  au32Timestamp)
{
    ACL_ASSERT(apcSourceAddr != APL_NULL && apcSharedSecret != APL_NULL);

    const char *lpcTimestamp = APL_NULL;
    char lacTimestamp[10+1] = {0};
    apl_uint32_t lu32Ret = 0;

    if(au32Timestamp == 0)
    {
        acl::CDateTime loDataTime;
        loDataTime.Update();
        lpcTimestamp = loDataTime.Format(CMPP_DEFAULT_DATE_FORMATE);
        lu32Ret = apl_strtou32(lpcTimestamp, NULL, 10);
    }
    else
    {
        apl_snprintf(lacTimestamp, sizeof(lacTimestamp), "%010"APL_PRIu32, au32Timestamp);
        lpcTimestamp = lacTimestamp;
    }

    acl::CMD5  loMD5;
    char       lacPadding[9] = {0};

    loMD5.Update(apcSourceAddr);
    loMD5.Update(lacPadding, 9);
    loMD5.Update(apcSharedSecret);
    loMD5.Update(lpcTimestamp);

    apl_memcpy(apcAuthenticatorSource, loMD5.Final().GetUstr(), 16);

    return lu32Ret;
}

apl_ssize_t CCMPPConnectPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < apl_size_t(27)), ERR_CMPP_CONNECT_ENCODE);

    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldStr(lpcPtr, this->macSourceAddr, CMPP_SOURCE_ADDR_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macAuthenticatorSource, CMPP_AUTHENTICATOR_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8Version);
    lpcPtr += EncodeField(lpcPtr, this->mu32Timestamp);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPPConnectPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen != (apl_size_t)27), ERR_CMPP_CONNECT_DECODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldStr(this->macSourceAddr, lpcPtr, CMPP_SOURCE_ADDR_LEN);
    lpcPtr += DecodeFieldStr(this->macAuthenticatorSource, lpcPtr, CMPP_AUTHENTICATOR_LEN);
    lpcPtr += DecodeField(&this->mu8Version, lpcPtr);
    lpcPtr += DecodeField(&this->mu32Timestamp, lpcPtr);

    return lpcPtr - (char *)apvPtr;
}

// ---------------- CCMPPConnectRespPDU ---------------- //
CCMPPConnectRespPDU::CCMPPConnectRespPDU(void)
    : mu8Version(0)
{
    apl_memset(macAuthenticatorISMG, 0, sizeof(macAuthenticatorISMG));
}

// ---------------- CCMPP20ConnectRespPDU ---------------- //
CCMPP20ConnectRespPDU::CCMPP20ConnectRespPDU(void)
    : mu8Status(0)
{
}

apl_int_t CCMPP20ConnectRespPDU::CalcAuthenticatorISMG(
        char* apcAuthenticatorISMG,
        apl_uint8_t au8Status,
        const char* apcAuthenticatorSource,
        const char* apcSharedSecret)
{
    ACL_ASSERT(apcAuthenticatorSource != APL_NULL && apcSharedSecret != APL_NULL);

    char      lacTemp[16] = {0};
    apl_snprintf(lacTemp, sizeof(lacTemp), "%"APL_PRIu8, au8Status);

    acl::CMD5 loMD5;
    loMD5.Update(lacTemp);
    loMD5.Update(apcAuthenticatorSource, 16);
    loMD5.Update(apcSharedSecret);

    apl_memcpy(apcAuthenticatorISMG, loMD5.Final().GetUstr(), 16);

    return 0;
}

apl_ssize_t CCMPP20ConnectRespPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < (apl_size_t)18), ERR_CMPP_CONNECT_RESP_ENCODE);

    char* lpcPtr = (char*)apvPtr;

    lpcPtr += EncodeField(lpcPtr, this->mu8Status);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macAuthenticatorISMG, CMPP_AUTHENTICATOR_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8Version);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPP20ConnectRespPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen != (apl_size_t)18), ERR_CMPP_CONNECT_RESP_DECODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeField(&this->mu8Status, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macAuthenticatorISMG, lpcPtr, CMPP_AUTHENTICATOR_LEN);
    lpcPtr += DecodeField(&this->mu8Version, lpcPtr);

    return lpcPtr - (char *)apvPtr;
}

// ---------------- CCMPP30ConnectRespPDU ---------------- //
CCMPP30ConnectRespPDU::CCMPP30ConnectRespPDU(void)
    : mu32Status(0)
{
}

apl_int_t CCMPP30ConnectRespPDU::CalcAuthenticatorISMG(
        char* apcAuthenticatorISMG,
        apl_uint32_t au32Status,
        const char* apcAuthenticatorSource,
        const char* apcSharedSecret)
{
    ACL_ASSERT(apcAuthenticatorSource != APL_NULL && apcSharedSecret != APL_NULL);

    char      lacTemp[16] = {0};
    apl_snprintf(lacTemp, sizeof(lacTemp), "%"APL_PRIu32, au32Status);

    acl::CMD5 loMD5;
    loMD5.Update(lacTemp);
    loMD5.Update(apcAuthenticatorSource, 16);
    loMD5.Update(apcSharedSecret);

    apl_memcpy(apcAuthenticatorISMG, loMD5.Final().GetUstr(), 16);

    return 0;
}

apl_ssize_t CCMPP30ConnectRespPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < (apl_size_t)21), ERR_CMPP_CONNECT_RESP_ENCODE);
    char* lpcPtr = (char*)apvPtr;

    lpcPtr += EncodeField(lpcPtr, this->mu32Status);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macAuthenticatorISMG, CMPP_AUTHENTICATOR_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8Version);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPP30ConnectRespPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen != (apl_size_t)21), ERR_CMPP_CONNECT_RESP_DECODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeField(&this->mu32Status, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macAuthenticatorISMG, lpcPtr, CMPP_AUTHENTICATOR_LEN);
    lpcPtr += DecodeField(&this->mu8Version, lpcPtr);

    return lpcPtr - (char *)apvPtr;
}

// ---------------- CCMPPSubmitPDU ---------------- //
CCMPPSubmitPDU::CCMPPSubmitPDU(void)
    : mu64MsgID(0)
    , mu8PkTotal(0)
    , mu8PkNumber(0)
    , mu8RegisteredDelivery(0)
    , mu8MsgLevel(0)
    , mu8FeeUserType(0)
    , mu8TpPID(0)
    , mu8TpUDHI(0)
    , mu8MsgFmt(0)
    , mu8DestUsrTL(0)
    , mu8MsgLength(0)
{
    apl_memset(macServiceID, 0, sizeof(macServiceID));
    apl_memset(macMsgSrc, 0, sizeof(macMsgSrc));
    apl_memset(macFeeType, 0, sizeof(macFeeType));
    apl_memset(macFeeCode, 0, sizeof(macFeeCode));
    apl_memset(macValidTime, 0, sizeof(macValidTime));
    apl_memset(macAtTime, 0, sizeof(macAtTime));
    apl_memset(macSrcID, 0, sizeof(macSrcID));
    apl_memset(macMsgContent, 0, sizeof(macMsgContent));
}

// ---------------- CCMPP20SubmitPDU ---------------- //
CCMPP20SubmitPDU::CCMPP20SubmitPDU(void)
{
    apl_memset(macFeeTerminalID, 0, sizeof(macFeeTerminalID));
    for(apl_size_t liN=0; liN<sizeof(maacDestTerminalID)/sizeof(maacDestTerminalID[0]); ++liN)
    {
        apl_memset(maacDestTerminalID[liN], 0, sizeof(maacDestTerminalID[liN]));
    }
    apl_memset(macReserve, 0, sizeof(macReserve));
}

apl_int_t CCMPP20SubmitPDU::SetDestTerminalID(const char* apcDestTerminalID, apl_size_t luIndex)
{
    if(luIndex >= CMPP_MAX_DEST_USR)
    {
        return -1;
    }

    apl_strncpy(this->maacDestTerminalID[luIndex], apcDestTerminalID, sizeof(this->maacDestTerminalID[luIndex]));

    return 0;
}

const char* CCMPP20SubmitPDU::GetDestTerminalID(apl_size_t luIndex)
{
    return this->maacDestTerminalID[luIndex];
}

apl_ssize_t CCMPP20SubmitPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < apl_size_t(126+21*this->mu8DestUsrTL+this->mu8MsgLength)), ERR_CMPP_SUBMIT_ENCODE);

    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64MsgID);
    lpcPtr += EncodeField(lpcPtr, this->mu8PkTotal);
    lpcPtr += EncodeField(lpcPtr, this->mu8PkNumber);
    lpcPtr += EncodeField(lpcPtr, this->mu8RegisteredDelivery);
    lpcPtr += EncodeField(lpcPtr, this->mu8MsgLevel);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macServiceID, CMPP_SERVICE_ID_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8FeeUserType);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macFeeTerminalID, 21);
    lpcPtr += EncodeField(lpcPtr, this->mu8TpPID);
    lpcPtr += EncodeField(lpcPtr, this->mu8TpUDHI);
    lpcPtr += EncodeField(lpcPtr, this->mu8MsgFmt);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macMsgSrc, CMPP_MSG_SRC_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macFeeType, CMPP_FEE_TYPE_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macFeeCode, CMPP_FEE_CODE_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macValidTime, CMPP_VALID_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macAtTime, CMPP_AT_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSrcID, CMPP_SRC_ID_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8DestUsrTL);

    for(apl_size_t luN=0; luN<this->mu8DestUsrTL&&luN<CMPP_MAX_DEST_USR; ++luN)
    {
        lpcPtr += EncodeFieldStr(lpcPtr, this->maacDestTerminalID[luN], 21);
    }

    lpcPtr += EncodeField(lpcPtr, this->mu8MsgLength);
    lpcPtr += EncodeFieldStr( lpcPtr, this->macMsgContent, this->mu8MsgLength);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macReserve, 8);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPP20SubmitPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    bool lbHasReserved = false;

    //RETURN_ERR_IF((auLen < (apl_size_t)147), ERR_CMPP_SUBMIT_DECODE); //117+21+1+8
    RETURN_ERR_IF((auLen < (apl_size_t)139), ERR_CMPP_SUBMIT_DECODE); //117+21+1
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldMsgId(&this->mu64MsgID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8PkTotal, lpcPtr);
    lpcPtr += DecodeField(&this->mu8PkNumber, lpcPtr);
    lpcPtr += DecodeField(&this->mu8RegisteredDelivery, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgLevel, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macServiceID, lpcPtr, CMPP_SERVICE_ID_LEN);
    lpcPtr += DecodeField(&this->mu8FeeUserType, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macFeeTerminalID, lpcPtr, 21);
    lpcPtr += DecodeField(&this->mu8TpPID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8TpUDHI, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgFmt, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macMsgSrc, lpcPtr, CMPP_MSG_SRC_LEN);
    lpcPtr += DecodeFieldStr(this->macFeeType, lpcPtr, CMPP_FEE_TYPE_LEN);
    lpcPtr += DecodeFieldStr(this->macFeeCode, lpcPtr, CMPP_FEE_CODE_LEN);
    lpcPtr += DecodeFieldStr(this->macValidTime, lpcPtr, CMPP_VALID_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macAtTime, lpcPtr, CMPP_AT_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macSrcID, lpcPtr, CMPP_SRC_ID_LEN);
    lpcPtr += DecodeField(&this->mu8DestUsrTL, lpcPtr);

    apl_uint8_t lu8DestUsrTL = 0;
    apl_uint8_t lu8Escape    = 0;
    lu8DestUsrTL = this->mu8DestUsrTL;
    if(this->mu8DestUsrTL > CMPP_MAX_DEST_USR)
    {
        this->mu8DestUsrTL = CMPP_MAX_DEST_USR;
        lu8Escape = lu8DestUsrTL - CMPP_MAX_DEST_USR;
    }

    //RETURN_ERR_IF((auLen < apl_size_t(117+21*lu8DestUsrTL+9)), ERR_CMPP_SUBMIT_DECODE);
    RETURN_ERR_IF((auLen < apl_size_t(117+21*lu8DestUsrTL+1)), ERR_CMPP_SUBMIT_DECODE);
    for(apl_size_t luN=0; luN<this->mu8DestUsrTL; ++luN)
    {
        lpcPtr += DecodeFieldStr(this->maacDestTerminalID[luN], lpcPtr, 21);
    }
    lpcPtr += 21*lu8Escape;

    lpcPtr += DecodeField(&this->mu8MsgLength, lpcPtr);

    //RETURN_ERR_IF((auLen != apl_size_t(117+21*lu8DestUsrTL+this->mu8MsgLength+9)), ERR_CMPP_SUBMIT_DECODE);
    /*
    if(auLen == apl_size_t(117+21*lu8DestUsrTL+this->mu8MsgLength+9))
    {
        lbHasReserved = true;
    }
    else if(auLen == apl_size_t(117+21*lu8DestUsrTL+this->mu8MsgLength+1))
    {
        lbHasReserved = false;
    }
    else
    {
        return ERR_CMPP_SUBMIT_DECODE;
    }
    */

    RETURN_ERR_IF((auLen < apl_size_t(117+21*lu8DestUsrTL+this->mu8MsgLength+1)), ERR_CMPP_SUBMIT_DECODE);

    DecodeFieldStr( this->macMsgContent, lpcPtr, this->mu8MsgLength);
    lpcPtr += this->mu8MsgLength;
    //lpcPtr += DecodeFieldStr(this->macReserve, lpcPtr, 8);
    /*
    if(lbHasReserved)
    {
        lpcPtr += DecodeFieldStr(this->macReserve, lpcPtr, 8);
    }
    */

    //return lpcPtr - (char*)apvPtr;
    return auLen;
}

// ---------------- CCMPP30SubmitPDU ---------------- //
CCMPP30SubmitPDU::CCMPP30SubmitPDU(void)
    : mu8FeeTerminalType(0)
    , mu8DestTerminalType(0)
{
    apl_memset(macFeeTerminalID, 0, sizeof(macFeeTerminalID));
    for(apl_size_t liN=0; liN<sizeof(maacDestTerminalID)/sizeof(maacDestTerminalID[0]); ++liN)
    {
        apl_memset(maacDestTerminalID[liN], 0, sizeof(maacDestTerminalID[liN]));
    }
    apl_memset(macLinkID, 0, sizeof(macLinkID));
}

apl_int_t CCMPP30SubmitPDU::SetDestTerminalID(const char* apcDestTerminalID, apl_size_t luIndex)
{
    if(luIndex >= CMPP_MAX_DEST_USR)
    {
        return -1;
    }

    apl_strncpy(this->maacDestTerminalID[luIndex], apcDestTerminalID, sizeof(this->maacDestTerminalID[luIndex]));
    return 0;
}

const char* CCMPP30SubmitPDU::GetDestTerminalID(apl_size_t luIndex)
{
    return this->maacDestTerminalID[luIndex];
}

apl_ssize_t CCMPP30SubmitPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < apl_size_t(151+32*this->mu8DestUsrTL+this->mu8MsgLength)), ERR_CMPP_SUBMIT_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64MsgID);
    lpcPtr += EncodeField(lpcPtr, this->mu8PkTotal);
    lpcPtr += EncodeField(lpcPtr, this->mu8PkNumber);
    lpcPtr += EncodeField(lpcPtr, this->mu8RegisteredDelivery);
    lpcPtr += EncodeField(lpcPtr, this->mu8MsgLevel);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macServiceID, CMPP_SERVICE_ID_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8FeeUserType);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macFeeTerminalID, CMPP_TERMINAL_ID_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8FeeTerminalType);
    lpcPtr += EncodeField(lpcPtr, this->mu8TpPID);
    lpcPtr += EncodeField(lpcPtr, this->mu8TpUDHI);
    lpcPtr += EncodeField(lpcPtr, this->mu8MsgFmt);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macMsgSrc, CMPP_MSG_SRC_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macFeeType, CMPP_FEE_TYPE_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macFeeCode, CMPP_FEE_CODE_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macValidTime, CMPP_VALID_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macAtTime, CMPP_AT_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSrcID, CMPP_SRC_ID_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8DestUsrTL);

    for(apl_size_t luN=0; luN<this->mu8DestUsrTL&&luN<CMPP_MAX_DEST_USR; ++luN)
    {
        lpcPtr += EncodeFieldStr(lpcPtr, this->maacDestTerminalID[luN], CMPP_TERMINAL_ID_LEN);
    }

    lpcPtr += EncodeField(lpcPtr, this->mu8DestTerminalType);
    lpcPtr += EncodeField(lpcPtr, this->mu8MsgLength);
    lpcPtr += EncodeFieldStr( lpcPtr, this->macMsgContent, this->mu8MsgLength); 
    lpcPtr += EncodeFieldStr(lpcPtr, this->macLinkID, CMPP_LINK_ID_LEN);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPP30SubmitPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen < (apl_size_t)151), ERR_CMPP_SUBMIT_DECODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldMsgId(&this->mu64MsgID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8PkTotal, lpcPtr);
    lpcPtr += DecodeField(&this->mu8PkNumber, lpcPtr);
    lpcPtr += DecodeField(&this->mu8RegisteredDelivery, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgLevel, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macServiceID, lpcPtr, CMPP_SERVICE_ID_LEN);
    lpcPtr += DecodeField(&this->mu8FeeUserType, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macFeeTerminalID, lpcPtr, CMPP_TERMINAL_ID_LEN);
    lpcPtr += DecodeField(&this->mu8FeeTerminalType, lpcPtr);
    lpcPtr += DecodeField(&this->mu8TpPID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8TpUDHI, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgFmt, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macMsgSrc, lpcPtr, CMPP_MSG_SRC_LEN);
    lpcPtr += DecodeFieldStr(this->macFeeType, lpcPtr, CMPP_FEE_TYPE_LEN);
    lpcPtr += DecodeFieldStr(this->macFeeCode, lpcPtr, CMPP_FEE_CODE_LEN);
    lpcPtr += DecodeFieldStr(this->macValidTime, lpcPtr, CMPP_VALID_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macAtTime, lpcPtr, CMPP_AT_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macSrcID, lpcPtr, CMPP_SRC_ID_LEN);
    lpcPtr += DecodeField(&this->mu8DestUsrTL, lpcPtr);

    apl_uint8_t lu8DestUsrTL = 0;
    apl_uint8_t lu8Escape    = 0;
    lu8DestUsrTL = this->mu8DestUsrTL;
    if(this->mu8DestUsrTL > CMPP_MAX_DEST_USR)
    {
        this->mu8DestUsrTL = CMPP_MAX_DEST_USR;
        lu8Escape = lu8DestUsrTL - CMPP_MAX_DEST_USR;
    }

    RETURN_ERR_IF((auLen < apl_size_t(151+32*lu8DestUsrTL)), ERR_CMPP_SUBMIT_DECODE);
    for(apl_size_t luN=0; luN<this->mu8DestUsrTL; ++luN)
    {
        lpcPtr += DecodeFieldStr(this->maacDestTerminalID[luN], lpcPtr, CMPP_TERMINAL_ID_LEN);
    }
    lpcPtr += 32*lu8Escape;

    lpcPtr += DecodeField(&this->mu8DestTerminalType, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgLength, lpcPtr);

    RETURN_ERR_IF((auLen != apl_size_t(151+32*lu8DestUsrTL+this->mu8MsgLength)), ERR_CMPP_SUBMIT_DECODE);
    DecodeFieldStr( this->macMsgContent, lpcPtr, this->mu8MsgLength); // this->mu8MsgLength wount large than CMPP_MAX_CONTENT_LEN
    lpcPtr += this->mu8MsgLength;
    lpcPtr += DecodeFieldStr(this->macLinkID, lpcPtr, CMPP_LINK_ID_LEN);

    return lpcPtr - (char*)apvPtr;
}

// ---------------- CCMPPSubmitRespPDU ---------------- //
CCMPPSubmitRespPDU::CCMPPSubmitRespPDU(void)
    : mu64MsgID(0)
{
}

// ---------------- CCMPP20SubmitRespPDU ---------------- //
CCMPP20SubmitRespPDU::CCMPP20SubmitRespPDU(void)
    : mu8Result(0)
{
}

apl_ssize_t CCMPP20SubmitRespPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < (apl_size_t)9), ERR_CMPP_SUBMIT_RESP_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64MsgID);
    lpcPtr += EncodeField(lpcPtr, this->mu8Result);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPP20SubmitRespPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen != (apl_size_t)9), ERR_CMPP_SUBMIT_RESP_DECODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldMsgId(&this->mu64MsgID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8Result, lpcPtr);

    return lpcPtr - (const char *)apvPtr;
}

// ---------------- CCMPP30SubmitRespPDU ---------------- //
CCMPP30SubmitRespPDU::CCMPP30SubmitRespPDU(void)
    : mu32Result(0)
{
}

apl_ssize_t CCMPP30SubmitRespPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < (apl_size_t)12), ERR_CMPP_SUBMIT_RESP_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64MsgID);
    lpcPtr += EncodeField(lpcPtr, this->mu32Result);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPP30SubmitRespPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen != (apl_size_t)12), ERR_CMPP_SUBMIT_RESP_DECODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldMsgId(&this->mu64MsgID, lpcPtr);
    lpcPtr += DecodeField(&this->mu32Result, lpcPtr);

    return lpcPtr - (const char *)apvPtr;
}

// ---------------- CCMPPDeliverPDU ---------------- //
CCMPPDeliverPDU::CCMPPDeliverPDU(void)
    : mu64MsgID(0)
    , mu8TpPID(0)
    , mu8TpUDHI(0)
    , mu8MsgFmt(0)
    , mu8RegisteredDelivery(0)
    , mu8MsgLength(0)
    , mu64SRMsgID(0)
    , mu32SRSMSCSequence(0)
{
    apl_memset(macDestID, 0, sizeof(macDestID));
    apl_memset(macServiceID, 0, sizeof(macServiceID));
    apl_memset(macMsgContent, 0, sizeof(macMsgContent));

    apl_memset(macSRStatus, 0, sizeof(macSRStatus));
    apl_memset(macSRSubmitTime, 0, sizeof(macSRSubmitTime));
    apl_memset(macSRDoneTime, 0, sizeof(macSRDoneTime));
}

// ---------------- CCMPP20DeliverPDU ---------------- //
CCMPP20DeliverPDU::CCMPP20DeliverPDU(void)
{
    apl_memset(macSrcTerminalID, 0, sizeof(macSrcTerminalID));
    apl_memset(macReserved, 0, sizeof(macReserved));
    apl_memset(macSRDestTerminalID, 0, sizeof(macSRDestTerminalID));
}

void CCMPP20DeliverPDU::EncodeSR(void)
{
    apl_memset(this->macMsgContent, 0, sizeof(this->macMsgContent));
    char* lpcPtr = this->macMsgContent;

    this->SetMsgLength((apl_uint8_t)60);
    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64SRMsgID);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRStatus, CMPP_SR_STATUS_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRSubmitTime, CMPP_SR_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDoneTime, CMPP_SR_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDestTerminalID, 21);
    lpcPtr += EncodeField(lpcPtr, this->mu32SRSMSCSequence);
}

apl_ssize_t CCMPP20DeliverPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < apl_size_t(73+this->mu8MsgLength)), ERR_CMPP_DELIVER_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64MsgID);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macDestID, CMPP_DEST_ID_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macServiceID, CMPP_SERVICE_ID_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8TpPID);
    lpcPtr += EncodeField(lpcPtr, this->mu8TpUDHI);
    lpcPtr += EncodeField(lpcPtr, this->mu8MsgFmt);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSrcTerminalID, 21);
    lpcPtr += EncodeField(lpcPtr, this->mu8RegisteredDelivery);

    /*
    if(this->mu8RegisteredDelivery == 1)
    {
        //this->SetMsgLength((apl_uint8_t)60);
        //lpcPtr += EncodeField(lpcPtr, this->mu8MsgLength);
        lpcPtr += EncodeField(lpcPtr, (apl_uint8_t)60);
        lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64SRMsgID);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRStatus, CMPP_SR_STATUS_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRSubmitTime, CMPP_SR_TIME_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDoneTime, CMPP_SR_TIME_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDestTerminalID, 21);
        lpcPtr += EncodeField(lpcPtr, this->mu32SRSMSCSequence);
    }
    else
    */
    {
        lpcPtr += EncodeField(lpcPtr, this->mu8MsgLength);

        lpcPtr += EncodeFieldStr( lpcPtr, this->macMsgContent, this->mu8MsgLength);
    }

    lpcPtr += EncodeFieldStr(lpcPtr, this->macReserved, 8);

    return lpcPtr - (char *)apvPtr;
}

void CCMPP20DeliverPDU::DecodeSR(void)
{
    const char* lpcPtr = this->macMsgContent;

    lpcPtr += DecodeFieldMsgId(&this->mu64SRMsgID, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macSRStatus, lpcPtr, CMPP_SR_STATUS_LEN);
    lpcPtr += DecodeFieldStr(this->macSRSubmitTime, lpcPtr, CMPP_SR_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macSRDoneTime, lpcPtr, CMPP_SR_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macSRDestTerminalID, lpcPtr, 21);
    lpcPtr += DecodeField(&this->mu32SRSMSCSequence, lpcPtr);
}

apl_ssize_t CCMPP20DeliverPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    bool lbHasReserved = false;
    //RETURN_ERR_IF((auLen < (apl_size_t)73), ERR_CMPP_DELIVER_DECODE);
    RETURN_ERR_IF((auLen < (apl_size_t)65), ERR_CMPP_DELIVER_DECODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldMsgId(&this->mu64MsgID, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macDestID, lpcPtr, CMPP_DEST_ID_LEN);
    lpcPtr += DecodeFieldStr(this->macServiceID, lpcPtr, CMPP_SERVICE_ID_LEN);
    lpcPtr += DecodeField(&this->mu8TpPID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8TpUDHI, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgFmt, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macSrcTerminalID, lpcPtr, 21);
    lpcPtr += DecodeField(&this->mu8RegisteredDelivery, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgLength, lpcPtr);

    /*
    if(this->mu8RegisteredDelivery == 1)
    {
        lpcPtr += DecodeFieldMsgId(&this->mu64SRMsgID, lpcPtr);
        lpcPtr += DecodeFieldStr(this->macSRStatus, lpcPtr, CMPP_SR_STATUS_LEN);
        lpcPtr += DecodeFieldStr(this->macSRSubmitTime, lpcPtr, CMPP_SR_TIME_LEN);
        lpcPtr += DecodeFieldStr(this->macSRDoneTime, lpcPtr, CMPP_SR_TIME_LEN);
        lpcPtr += DecodeFieldStr(this->macSRDestTerminalID, lpcPtr, 21);
        lpcPtr += DecodeField(&this->mu32SRSMSCSequence, lpcPtr);
    }
    else
    */
    {
        //RETURN_ERR_IF((auLen != (apl_size_t)(73+this->mu8MsgLength)), ERR_CMPP_DELIVER_DECODE);
        if(auLen == (apl_size_t)(73+this->mu8MsgLength))
        {
            lbHasReserved = true;
        }
        else if(auLen == (apl_size_t)(65+this->mu8MsgLength))
        {
            lbHasReserved = false;
        }
        else
        {
            return ERR_CMPP_DELIVER_DECODE;
        }

        DecodeFieldStr( this->macMsgContent, lpcPtr, this->mu8MsgLength);
        lpcPtr += this->mu8MsgLength;
    }

    //lpcPtr += DecodeFieldStr(this->macReserved, lpcPtr, 8);
    if(lbHasReserved)
    {
        lpcPtr += DecodeFieldStr(this->macReserved, lpcPtr, 8);
    }

    return lpcPtr - (char *)apvPtr;
}

// ---------------- CCMPP30DeliverPDU ---------------- //
CCMPP30DeliverPDU::CCMPP30DeliverPDU(void)
    : mu8SrcTerminalType(0)
{
    apl_memset(macSrcTerminalID, 0, sizeof(macSrcTerminalID));
    apl_memset(macLinkID, 0, sizeof(macLinkID));
    apl_memset(macSRDestTerminalID, 0, sizeof(macSRDestTerminalID));

}

void CCMPP30DeliverPDU::EncodeSR(void)
{
    char* lpcPtr = this->macMsgContent;

    this->SetMsgLength((apl_uint8_t)71);
    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64SRMsgID);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRStatus, CMPP_SR_STATUS_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRSubmitTime, CMPP_SR_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDoneTime, CMPP_SR_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDestTerminalID, 32);
    lpcPtr += EncodeField(lpcPtr, this->mu32SRSMSCSequence);
}

apl_ssize_t CCMPP30DeliverPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < apl_size_t(97+this->mu8MsgLength)), ERR_CMPP_DELIVER_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64MsgID);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macDestID, CMPP_DEST_ID_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macServiceID, CMPP_SERVICE_ID_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8TpPID);
    lpcPtr += EncodeField(lpcPtr, this->mu8TpUDHI);
    lpcPtr += EncodeField(lpcPtr, this->mu8MsgFmt);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSrcTerminalID, CMPP_TERMINAL_ID_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8SrcTerminalType);
    lpcPtr += EncodeField(lpcPtr, this->mu8RegisteredDelivery);

    /*
    if(this->mu8RegisteredDelivery == 1)
    {
        //this->SetMsgLength((apl_uint8_t)71);
        //lpcPtr += EncodeField(lpcPtr, this->mu8MsgLength);
        lpcPtr += EncodeField(lpcPtr, (apl_uint8_t)71);
        lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64SRMsgID);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRStatus, CMPP_SR_STATUS_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRSubmitTime, CMPP_SR_TIME_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDoneTime, CMPP_SR_TIME_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDestTerminalID, 32);
        lpcPtr += EncodeField(lpcPtr, this->mu32SRSMSCSequence);
    }
    else
    */
    {
        lpcPtr += EncodeField(lpcPtr, this->mu8MsgLength);
        lpcPtr += EncodeFieldStr( lpcPtr, this->macMsgContent, this->mu8MsgLength);
    }

    lpcPtr += EncodeFieldStr(lpcPtr, this->macLinkID, CMPP_LINK_ID_LEN);

    return lpcPtr - (char *)apvPtr;
}

void CCMPP30DeliverPDU::DecodeSR(void)
{
    const char* lpcPtr = this->macMsgContent;

    lpcPtr += DecodeFieldMsgId(&this->mu64SRMsgID, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macSRStatus, lpcPtr, CMPP_SR_STATUS_LEN);
    lpcPtr += DecodeFieldStr(this->macSRSubmitTime, lpcPtr, CMPP_SR_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macSRDoneTime, lpcPtr, CMPP_SR_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macSRDestTerminalID, lpcPtr, CMPP_TERMINAL_ID_LEN);
    lpcPtr += DecodeField(&this->mu32SRSMSCSequence, lpcPtr);
}

apl_ssize_t CCMPP30DeliverPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen < (apl_size_t)97), ERR_CMPP_DELIVER_DECODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldMsgId(&this->mu64MsgID, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macDestID, lpcPtr, CMPP_DEST_ID_LEN);
    lpcPtr += DecodeFieldStr(this->macServiceID, lpcPtr, CMPP_SERVICE_ID_LEN);
    lpcPtr += DecodeField(&this->mu8TpPID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8TpUDHI, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgFmt, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macSrcTerminalID, lpcPtr, CMPP_TERMINAL_ID_LEN);
    lpcPtr += DecodeField(&this->mu8SrcTerminalType, lpcPtr);
    lpcPtr += DecodeField(&this->mu8RegisteredDelivery, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgLength, lpcPtr);

    /*
    if(this->mu8RegisteredDelivery == 1)
    {
        lpcPtr += DecodeFieldMsgId(&this->mu64SRMsgID, lpcPtr);
        lpcPtr += DecodeFieldStr(this->macSRStatus, lpcPtr, CMPP_SR_STATUS_LEN);
        lpcPtr += DecodeFieldStr(this->macSRSubmitTime, lpcPtr, CMPP_SR_TIME_LEN);
        lpcPtr += DecodeFieldStr(this->macSRDoneTime, lpcPtr, CMPP_SR_TIME_LEN);
        lpcPtr += DecodeFieldStr(this->macSRDestTerminalID, lpcPtr, CMPP_TERMINAL_ID_LEN);
        lpcPtr += DecodeField(&this->mu32SRSMSCSequence, lpcPtr);
    }
    else
    */
    {
        RETURN_ERR_IF((auLen != (apl_size_t)(97+this->mu8MsgLength)), ERR_CMPP_DELIVER_DECODE);
        DecodeFieldStr( this->macMsgContent, lpcPtr, this->mu8MsgLength );
        lpcPtr += this->mu8MsgLength;
    }
    lpcPtr += DecodeFieldStr(this->macLinkID, lpcPtr, CMPP_LINK_ID_LEN);

    return lpcPtr - (char *)apvPtr;
}

// ---------------- CCMPPDeliverRespPDU ---------------- //
CCMPPDeliverRespPDU::CCMPPDeliverRespPDU(void)
    : mu64MsgID(0)
{
}

// ---------------- CCMPP20DeliverRespPDU ---------------- //
CCMPP20DeliverRespPDU::CCMPP20DeliverRespPDU(void)
    : mu8Result(0)
{
}

apl_ssize_t CCMPP20DeliverRespPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < (apl_size_t)9), ERR_CMPP_DELIVER_RESP_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64MsgID);
    lpcPtr += EncodeField(lpcPtr, this->mu8Result);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPP20DeliverRespPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen != (apl_size_t)9), ERR_CMPP_DELIVER_RESP_DECODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldMsgId(&this->mu64MsgID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8Result, lpcPtr);

    return lpcPtr - (const char *)apvPtr;
}

// ---------------- CCMPP30DeliverRespPDU ---------------- //
CCMPP30DeliverRespPDU::CCMPP30DeliverRespPDU(void)
    : mu32Result(0)
{
}

apl_ssize_t CCMPP30DeliverRespPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < (apl_size_t)12), ERR_CMPP_DELIVER_RESP_ENCODE); 
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64MsgID);
    lpcPtr += EncodeField(lpcPtr, this->mu32Result);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPP30DeliverRespPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen != (apl_size_t)12), ERR_CMPP_DELIVER_RESP_DECODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldMsgId(&this->mu64MsgID, lpcPtr);
    lpcPtr += DecodeField(&this->mu32Result, lpcPtr);

    return lpcPtr - (const char *)apvPtr;
}

// ---------------- CCMPPQueryPDU ---------------- //
CCMPPQueryPDU::CCMPPQueryPDU(void)
    : mu8QueryType(0)
{
    apl_memset(macTime, 0, sizeof(macTime));
    apl_memset(macQueryCode, 0, sizeof(macQueryCode));
    apl_memset(macReserve, 0, sizeof(macReserve));
}

apl_ssize_t CCMPPQueryPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < (apl_size_t)27), ERR_CMPP_QUERY_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldStr(lpcPtr, this->macTime, 8);
    lpcPtr += EncodeField(lpcPtr, this->mu8QueryType);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macQueryCode, 10);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macReserve, 8);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPPQueryPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    bool lbHasReserved = false;

    //RETURN_ERR_IF((auLen != (apl_size_t)27), ERR_CMPP_QUERY_DECODE);
    if(auLen == (apl_size_t)27)
    {
        lbHasReserved = true;
    }
    else if(auLen == (apl_size_t)19)
    {
        lbHasReserved = false;
    }
    else
    {
        return ERR_CMPP_QUERY_DECODE;
    }

    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldStr(this->macTime, lpcPtr, 8);
    lpcPtr += DecodeField(&this->mu8QueryType, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macQueryCode, lpcPtr, 10);
    //lpcPtr += DecodeFieldStr(this->macReserve, lpcPtr, 8);

    if(lbHasReserved)
    {
        lpcPtr += DecodeFieldStr(this->macReserve, lpcPtr, 8);
    }

    return lpcPtr - (const char *)apvPtr;
}

// ---------------- CCMPPQueryRespPDU ---------------- //
CCMPPQueryRespPDU::CCMPPQueryRespPDU(void)
    : mu8QueryType(0)
    , mu32MTTlMsg(0)
    , mu32MTTlUsr(0)
    , mu32MTScs(0)
    , mu32MTWT(0)
    , mu32MTFL(0)
    , mu32MOScs(0)
    , mu32MOWT(0)
    , mu32MOFL(0)
{
    apl_memset(macTime, 0, sizeof(macTime));
    apl_memset(macQueryCode, 0, sizeof(macQueryCode));
}

apl_ssize_t CCMPPQueryRespPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < (apl_size_t)51), ERR_CMPP_QUERY_RESP_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldStr(lpcPtr, this->macTime, 8);
    lpcPtr += EncodeField(lpcPtr, this->mu8QueryType);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macQueryCode, 10);
    lpcPtr += EncodeField(lpcPtr, this->mu32MTTlMsg);
    lpcPtr += EncodeField(lpcPtr, this->mu32MTTlUsr);
    lpcPtr += EncodeField(lpcPtr, this->mu32MTScs);
    lpcPtr += EncodeField(lpcPtr, this->mu32MTWT);
    lpcPtr += EncodeField(lpcPtr, this->mu32MTFL);
    lpcPtr += EncodeField(lpcPtr, this->mu32MOScs);
    lpcPtr += EncodeField(lpcPtr, this->mu32MOWT);
    lpcPtr += EncodeField(lpcPtr, this->mu32MOFL);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPPQueryRespPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen != (apl_size_t)51), ERR_CMPP_QUERY_RESP_ENCODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldStr(this->macTime, lpcPtr, 8);
    lpcPtr += DecodeField(&this->mu8QueryType, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macQueryCode, lpcPtr, 10);
    lpcPtr += DecodeField(&this->mu32MTTlMsg, lpcPtr);
    lpcPtr += DecodeField(&this->mu32MTTlUsr, lpcPtr);
    lpcPtr += DecodeField(&this->mu32MTScs, lpcPtr);
    lpcPtr += DecodeField(&this->mu32MTWT, lpcPtr);
    lpcPtr += DecodeField(&this->mu32MTFL, lpcPtr);
    lpcPtr += DecodeField(&this->mu32MOScs, lpcPtr);
    lpcPtr += DecodeField(&this->mu32MOWT, lpcPtr);
    lpcPtr += DecodeField(&this->mu32MOFL, lpcPtr);

    return lpcPtr - (const char *)apvPtr;
}

// ---------------- CCMPPCancelPDU ---------------- //
CCMPPCancelPDU::CCMPPCancelPDU(void)
    : mu64MsgID(0)
{
}

apl_ssize_t CCMPPCancelPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < (apl_size_t)8), ERR_CMPP_CANCEL_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64MsgID);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPPCancelPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen != (apl_size_t)8), ERR_CMPP_CANCEL_ENCODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldMsgId(&this->mu64MsgID, lpcPtr);

    return lpcPtr - (const char *)apvPtr;
}

// ---------------- CCMPPCancelRespPDU ---------------- //
CCMPPCancelRespPDU::CCMPPCancelRespPDU(void)
    : mu32SuccessID(0)
{
}

apl_ssize_t CCMPPCancelRespPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < (apl_size_t)4), ERR_CMPP_CANCEL_RESP_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeField(lpcPtr, this->mu32SuccessID);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPPCancelRespPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen != (apl_size_t)4), ERR_CMPP_CANCEL_RESP_ENCODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeField(&this->mu32SuccessID, lpcPtr);

    return lpcPtr - (const char *)apvPtr;
}

// ---------------- CCMPPActiveTestRespPDU ---------------- //
CCMPPActiveTestRespPDU::CCMPPActiveTestRespPDU(void)
    : mu8Reserved(0)
{
}

apl_ssize_t CCMPPActiveTestRespPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < (apl_size_t)1), ERR_CMPP_ACTIVETEST_RESP_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeField(lpcPtr, this->mu8Reserved);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPPActiveTestRespPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    bool lbHasReserved = false;

    //RETURN_ERR_IF((auLen != (apl_size_t)1), ERR_CMPP_ACTIVETEST_RESP_ENCODE);
    if(auLen == (apl_size_t)1)
    {
        lbHasReserved = true; 
    }
    else if(auLen == (apl_size_t)0)
    {
        lbHasReserved = false;
    }
    else
    {
        return ERR_CMPP_ACTIVETEST_RESP_ENCODE;
    }

    const char* lpcPtr = (const char*)apvPtr;

    //lpcPtr += DecodeField(&this->mu8Reserved, lpcPtr);
    if(lbHasReserved)
    {
        lpcPtr += DecodeField(&this->mu8Reserved, lpcPtr);
    }

    return lpcPtr - (const char *)apvPtr;
}

// ---------------- CCMPPFwdPDU ---------------- //
CCMPPFwdPDU::CCMPPFwdPDU(void)
    : mu64MsgID(0)
    , mu8NodeCount(0)
    , mu8MsgFwdType(0)
    , mu8PkTotal(0)
    , mu8PkNumber(0)
    , mu8RegisteredDelivery(0)
    , mu8MsgLevel(0)
    , mu8FeeUserType(0)
    , mu8TpPID(0)
    , mu8TpUDHI(0)
    , mu8MsgFmt(0)
    , mu8DestUsrTL(1) //must be 1
    , mu8MsgLength(0)
    , mu64SRMsgID(0)
    , mu32SRSMSCSequence(0)
{
    apl_memset(macSourceID, 0, sizeof(macSourceID));
    apl_memset(macDestinationID, 0, sizeof(macDestinationID));
    apl_memset(macServiceID, 0, sizeof(macServiceID));
    apl_memset(macFeeTerminalID, 0, sizeof(macFeeTerminalID));
    apl_memset(macMsgSrc, 0, sizeof(macMsgSrc));
    apl_memset(macFeeType, 0, sizeof(macFeeType));
    apl_memset(macFeeCode, 0, sizeof(macFeeCode));
    apl_memset(macValidTime, 0, sizeof(macValidTime));
    apl_memset(macAtTime, 0, sizeof(macAtTime));
    apl_memset(macSrcTerminalID, 0, sizeof(macSrcTerminalID));
    apl_memset(macDestTerminalID, 0, sizeof(macDestTerminalID));
    apl_memset(macMsgContent, 0, sizeof(macMsgContent));

    apl_memset(macSRStatus, 0, sizeof(macSRStatus));
    apl_memset(macSRSubmitTime, 0, sizeof(macSRSubmitTime));
    apl_memset(macSRDoneTime, 0, sizeof(macSRDoneTime));
}

// ---------------- CCMPP20FwdPDU ---------------- //
CCMPP20FwdPDU::CCMPP20FwdPDU(void)
{
    apl_memset(macReserved, 0, sizeof(macReserved));
    apl_memset(macSRDestTerminalID, 0, sizeof(macSRDestTerminalID));
}

void CCMPP20FwdPDU::EncodeMOSR(void)
{
    char* lpcPtr = this->macMsgContent;
    this->SetMsgLength((apl_uint8_t)60);
    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64SRMsgID);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRStatus, CMPP_SR_STATUS_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRSubmitTime, CMPP_SR_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDoneTime, CMPP_SR_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDestTerminalID, 21);
    lpcPtr += EncodeField(lpcPtr, this->mu32SRSMSCSequence);
}

void CCMPP20FwdPDU::EncodeMTSR(void)
{
    this->EncodeMOSR();
}

apl_ssize_t CCMPP20FwdPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < apl_size_t(161+this->mu8MsgLength)), ERR_CMPP_FWD_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldStr(lpcPtr, this->macSourceID, CMPP_ISMG_ID_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macDestinationID, CMPP_ISMG_ID_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8NodeCount);
    lpcPtr += EncodeField(lpcPtr, this->mu8MsgFwdType);
    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64MsgID);
    lpcPtr += EncodeField(lpcPtr, this->mu8PkTotal);
    lpcPtr += EncodeField(lpcPtr, this->mu8PkNumber);
    lpcPtr += EncodeField(
            lpcPtr,
            this->mu8RegisteredDelivery<=(apl_uint8_t)2 ? this->mu8RegisteredDelivery: (apl_uint8_t)1);
    lpcPtr += EncodeField(lpcPtr, this->mu8MsgLevel);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macServiceID, CMPP_SERVICE_ID_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8FeeUserType);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macFeeTerminalID, 21);
    lpcPtr += EncodeField(lpcPtr, this->mu8TpPID);
    lpcPtr += EncodeField(lpcPtr, this->mu8TpUDHI);
    lpcPtr += EncodeField(lpcPtr, this->mu8MsgFmt);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macMsgSrc, CMPP_MSG_SRC_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macFeeType, CMPP_FEE_TYPE_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macFeeCode, CMPP_FEE_CODE_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macValidTime, CMPP_VALID_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macAtTime, CMPP_AT_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSrcTerminalID, 21);
    lpcPtr += EncodeField(lpcPtr, (apl_uint8_t)1);  //set destusr_tl
    lpcPtr += EncodeFieldStr(lpcPtr, this->macDestTerminalID, 21);
    /*
    if(this->mu8MsgFwdType == FWD_TYPE_MO_STATUS)
    {
        //this->SetMsgLength((apl_uint8_t)60);
        //lpcPtr += EncodeField(lpcPtr, this->mu8MsgLength);
        lpcPtr += EncodeField(lpcPtr, (apl_uint8_t)60);
        lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64SRMsgID);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRStatus, CMPP_SR_STATUS_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRSubmitTime, CMPP_SR_TIME_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDoneTime, CMPP_SR_TIME_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDestTerminalID, 21);
        lpcPtr += EncodeField(lpcPtr, this->mu32SRSMSCSequence);
    }
    else if(this->mu8MsgFwdType == FWD_TYPE_MT_STATUS)
    {
        //this->SetMsgLength((apl_uint8_t)71);
        //lpcPtr += EncodeField(lpcPtr, this->mu8MsgLength);
        lpcPtr += EncodeField(lpcPtr, (apl_uint8_t)71);
        lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64SRMsgID);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRStatus, CMPP_SR_STATUS_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRSubmitTime, CMPP_SR_TIME_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDoneTime, CMPP_SR_TIME_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDestTerminalID, 21);
        lpcPtr += EncodeField(lpcPtr, this->mu32SRSMSCSequence);
    }
    else
    */
    {
        lpcPtr += EncodeField(lpcPtr, this->mu8MsgLength);
        lpcPtr += EncodeFieldStr( lpcPtr, this->macMsgContent, this->mu8MsgLength);
    }
    lpcPtr += EncodeFieldStr(lpcPtr, this->macReserved, 8);

    return lpcPtr - (char *)apvPtr;
}

void CCMPP20FwdPDU::DecodeMOSR(void)
{
    const char* lpcPtr = this->macMsgContent;
    lpcPtr += DecodeFieldMsgId(&this->mu64SRMsgID, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macSRStatus, lpcPtr, CMPP_SR_STATUS_LEN);
    lpcPtr += DecodeFieldStr(this->macSRSubmitTime, lpcPtr, CMPP_SR_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macSRDoneTime, lpcPtr, CMPP_SR_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macSRDestTerminalID, lpcPtr, 21);
    lpcPtr += DecodeField(&this->mu32SRSMSCSequence, lpcPtr);
}

void CCMPP20FwdPDU::DecodeMTSR(void)
{
    this->DecodeMOSR();
}

apl_ssize_t CCMPP20FwdPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    bool lbHasReserved = false;

    //RETURN_ERR_IF((auLen < apl_size_t(161)), ERR_CMPP_FWD_DECODE);
    RETURN_ERR_IF((auLen < apl_size_t(153)), ERR_CMPP_FWD_DECODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldStr(this->macSourceID, lpcPtr, CMPP_ISMG_ID_LEN);
    lpcPtr += DecodeFieldStr(this->macDestinationID, lpcPtr, CMPP_ISMG_ID_LEN);
    lpcPtr += DecodeField(&this->mu8NodeCount, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgFwdType, lpcPtr);
    lpcPtr += DecodeFieldMsgId(&this->mu64MsgID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8PkTotal, lpcPtr);
    lpcPtr += DecodeField(&this->mu8PkNumber, lpcPtr);
    lpcPtr += DecodeField(&this->mu8RegisteredDelivery, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgLevel, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macServiceID, lpcPtr, CMPP_SERVICE_ID_LEN);
    lpcPtr += DecodeField(&this->mu8FeeUserType, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macFeeTerminalID, lpcPtr, 21);
    lpcPtr += DecodeField(&this->mu8TpPID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8TpUDHI, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgFmt, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macMsgSrc, lpcPtr, CMPP_MSG_SRC_LEN);
    lpcPtr += DecodeFieldStr(this->macFeeType, lpcPtr, CMPP_FEE_TYPE_LEN);
    lpcPtr += DecodeFieldStr(this->macFeeCode, lpcPtr, CMPP_FEE_CODE_LEN);
    lpcPtr += DecodeFieldStr(this->macValidTime, lpcPtr, CMPP_VALID_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macAtTime, lpcPtr, CMPP_AT_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macSrcTerminalID, lpcPtr, 21);
    lpcPtr += DecodeField(&this->mu8DestUsrTL, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macDestTerminalID, lpcPtr, 21);
    lpcPtr += DecodeField(&this->mu8MsgLength, lpcPtr);

    //RETURN_ERR_IF((auLen != apl_size_t(161+this->mu8MsgLength)), ERR_CMPP_FWD_DECODE);
    if(auLen == apl_size_t(161+this->mu8MsgLength))
    {
        lbHasReserved = true;
    }
    else if(auLen == apl_size_t(153+this->mu8MsgLength))
    {
        lbHasReserved = false;
    }
    else
    {
        return ERR_CMPP_FWD_DECODE;
    }
    /*
    if(this->mu8MsgFwdType == FWD_TYPE_MT_STATUS || this->mu8MsgFwdType == FWD_TYPE_MO_STATUS)
    {
        lpcPtr += DecodeFieldMsgId(&this->mu64SRMsgID, lpcPtr);
        lpcPtr += DecodeFieldStr(this->macSRStatus, lpcPtr, CMPP_SR_STATUS_LEN);
        lpcPtr += DecodeFieldStr(this->macSRSubmitTime, lpcPtr, CMPP_SR_TIME_LEN);
        lpcPtr += DecodeFieldStr(this->macSRDoneTime, lpcPtr, CMPP_SR_TIME_LEN);
        lpcPtr += DecodeFieldStr(this->macSRDestTerminalID, lpcPtr, 21);
        lpcPtr += DecodeField(&this->mu32SRSMSCSequence, lpcPtr);
    }
    else
    */
    {
        DecodeFieldStr( this->macMsgContent, lpcPtr, this->mu8MsgLength );
        lpcPtr += this->mu8MsgLength;
    }

    //lpcPtr += DecodeFieldStr(this->macReserved, lpcPtr, 8);
    if(lbHasReserved)
    {
        lpcPtr += DecodeFieldStr(this->macReserved, lpcPtr, 8);
    }

    return lpcPtr - (char *)apvPtr;
}

// ---------------- CCMPP30FwdPDU ---------------- //
CCMPP30FwdPDU::CCMPP30FwdPDU(void)
    : mu8FeeTerminalUserType(0)
    , mu8SrcTerminalUserType(0)
    , mu8SrcTerminalType(0)
    , mu8DestTerminalUserType(0)
{
    apl_memset(macDestTerminalPseudo, 0, sizeof(macDestTerminalPseudo));
    apl_memset(macFeeTerminalPseudo, 0, sizeof(macFeeTerminalPseudo));
    apl_memset(macSrcTerminalPseudo, 0, sizeof(macSrcTerminalPseudo));
    apl_memset(macLinkID, 0, sizeof(macLinkID));
    apl_memset(macSRDestTerminalID, 0, sizeof(macSRDestTerminalID));
}

void CCMPP30FwdPDU::EncodeMOSR(void)
{
    char* lpcPtr = this->macMsgContent;
    this->SetMsgLength((apl_uint8_t)60);
    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64SRMsgID);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRStatus, CMPP_SR_STATUS_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRSubmitTime, CMPP_SR_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDoneTime, CMPP_SR_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDestTerminalID, 21);
    lpcPtr += EncodeField(lpcPtr, this->mu32SRSMSCSequence);
}

void CCMPP30FwdPDU::EncodeMTSR(void)
{
    char* lpcPtr = this->macMsgContent;
    this->SetMsgLength((apl_uint8_t)71);
    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64SRMsgID);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRStatus, CMPP_SR_STATUS_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRSubmitTime, CMPP_SR_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDoneTime, CMPP_SR_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDestTerminalID, 32);
    lpcPtr += EncodeField(lpcPtr, this->mu32SRSMSCSequence);
}

apl_ssize_t CCMPP30FwdPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < apl_size_t(273+this->mu8MsgLength)), ERR_CMPP_FWD_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldStr(lpcPtr, this->macSourceID, CMPP_ISMG_ID_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macDestinationID, CMPP_ISMG_ID_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8NodeCount);
    lpcPtr += EncodeField(lpcPtr, this->mu8MsgFwdType);
    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64MsgID);
    lpcPtr += EncodeField(lpcPtr, this->mu8PkTotal);
    lpcPtr += EncodeField(lpcPtr, this->mu8PkNumber);
    lpcPtr += EncodeField(
            lpcPtr,
            this->mu8RegisteredDelivery<=(apl_uint8_t)2 ? this->mu8RegisteredDelivery: (apl_uint8_t)1);
    lpcPtr += EncodeField(lpcPtr, this->mu8MsgLevel);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macServiceID, CMPP_SERVICE_ID_LEN);
    lpcPtr += EncodeField(lpcPtr, this->mu8FeeUserType);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macFeeTerminalID, 21);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macFeeTerminalPseudo, CMPP_TERMINAL_PSEUDO_LEN);
    lpcPtr += EncodeField(
            lpcPtr,
            this->mu8FeeTerminalUserType<=(apl_uint8_t)1 ? this->mu8FeeTerminalUserType : (apl_uint8_t)1);
    lpcPtr += EncodeField(lpcPtr, this->mu8TpPID);
    lpcPtr += EncodeField(lpcPtr, this->mu8TpUDHI);
    lpcPtr += EncodeField(lpcPtr, this->mu8MsgFmt);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macMsgSrc, CMPP_MSG_SRC_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macFeeType, CMPP_FEE_TYPE_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macFeeCode, CMPP_FEE_CODE_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macValidTime, CMPP_VALID_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macAtTime, CMPP_AT_TIME_LEN);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSrcTerminalID, 21);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macSrcTerminalPseudo, CMPP_TERMINAL_PSEUDO_LEN);
    lpcPtr += EncodeField(
            lpcPtr,
            this->mu8SrcTerminalUserType<=(apl_uint8_t)1 ? this->mu8SrcTerminalUserType : (apl_uint8_t)1);
    lpcPtr += EncodeField(lpcPtr, this->mu8SrcTerminalType);
    lpcPtr += EncodeField(lpcPtr, (apl_uint8_t)1);  //set destusr_tl
    lpcPtr += EncodeFieldStr(lpcPtr, this->macDestTerminalID, 21);
    lpcPtr += EncodeFieldStr(lpcPtr, this->macDestTerminalPseudo, CMPP_TERMINAL_PSEUDO_LEN);
    lpcPtr += EncodeField(
            lpcPtr,
            this->mu8DestTerminalUserType<=(apl_uint8_t)1 ? this->mu8DestTerminalUserType : (apl_uint8_t)1);

    /*
    if(this->mu8MsgFwdType == FWD_TYPE_MO_STATUS)
    {
        //this->SetMsgLength((apl_uint8_t)60);
        //lpcPtr += EncodeField(lpcPtr, this->mu8MsgLength);
        lpcPtr += EncodeField(lpcPtr, (apl_uint8_t)60);
        lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64SRMsgID);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRStatus, CMPP_SR_STATUS_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRSubmitTime, CMPP_SR_TIME_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDoneTime, CMPP_SR_TIME_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDestTerminalID, 21);
        lpcPtr += EncodeField(lpcPtr, this->mu32SRSMSCSequence);
    }
    else if(this->mu8MsgFwdType == FWD_TYPE_MT_STATUS)
    {
        //this->SetMsgLength((apl_uint8_t)71);
        //lpcPtr += EncodeField(lpcPtr, this->mu8MsgLength);
        lpcPtr += EncodeField(lpcPtr, (apl_uint8_t)71);
        lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64SRMsgID);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRStatus, CMPP_SR_STATUS_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRSubmitTime, CMPP_SR_TIME_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDoneTime, CMPP_SR_TIME_LEN);
        lpcPtr += EncodeFieldStr(lpcPtr, this->macSRDestTerminalID, 32);
        lpcPtr += EncodeField(lpcPtr, this->mu32SRSMSCSequence);
    }
    else
    */
    {
        lpcPtr += EncodeField(lpcPtr, this->mu8MsgLength);
        lpcPtr += EncodeFieldStr( lpcPtr, this->macMsgContent, this->mu8MsgLength);
    }
    lpcPtr += EncodeFieldStr(lpcPtr, this->macLinkID, CMPP_LINK_ID_LEN);

    return lpcPtr - (char *)apvPtr;
}

void CCMPP30FwdPDU::DecodeMOSR(void)
{
    const char* lpcPtr = this->macMsgContent;
    lpcPtr += DecodeFieldMsgId(&this->mu64SRMsgID, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macSRStatus, lpcPtr, CMPP_SR_STATUS_LEN);
    lpcPtr += DecodeFieldStr(this->macSRSubmitTime, lpcPtr, CMPP_SR_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macSRDoneTime, lpcPtr, CMPP_SR_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macSRDestTerminalID, lpcPtr, 21);
    lpcPtr += DecodeField(&this->mu32SRSMSCSequence, lpcPtr);
}

void CCMPP30FwdPDU::DecodeMTSR(void)
{
    const char* lpcPtr = this->macMsgContent;

    lpcPtr += DecodeFieldMsgId(&this->mu64SRMsgID, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macSRStatus, lpcPtr, CMPP_SR_STATUS_LEN);
    lpcPtr += DecodeFieldStr(this->macSRSubmitTime, lpcPtr, CMPP_SR_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macSRDoneTime, lpcPtr, CMPP_SR_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macSRDestTerminalID, lpcPtr, 32);
    lpcPtr += DecodeField(&this->mu32SRSMSCSequence, lpcPtr);
}

apl_ssize_t CCMPP30FwdPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen < apl_size_t(273)), ERR_CMPP_FWD_DECODE);
    const char* lpcPtr = (const char*)apvPtr;

    lpcPtr += DecodeFieldStr(this->macSourceID, lpcPtr, CMPP_ISMG_ID_LEN);
    lpcPtr += DecodeFieldStr(this->macDestinationID, lpcPtr, CMPP_ISMG_ID_LEN);
    lpcPtr += DecodeField(&this->mu8NodeCount, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgFwdType, lpcPtr);
    lpcPtr += DecodeFieldMsgId(&this->mu64MsgID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8PkTotal, lpcPtr);
    lpcPtr += DecodeField(&this->mu8PkNumber, lpcPtr);
    lpcPtr += DecodeField(&this->mu8RegisteredDelivery, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgLevel, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macServiceID, lpcPtr, CMPP_SERVICE_ID_LEN);
    lpcPtr += DecodeField(&this->mu8FeeUserType, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macFeeTerminalID, lpcPtr, 21);
    lpcPtr += DecodeFieldStr(this->macFeeTerminalPseudo, lpcPtr, CMPP_TERMINAL_PSEUDO_LEN);
    lpcPtr += DecodeField(&this->mu8FeeTerminalUserType, lpcPtr);
    lpcPtr += DecodeField(&this->mu8TpPID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8TpUDHI, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgFmt, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macMsgSrc, lpcPtr, CMPP_MSG_SRC_LEN);
    lpcPtr += DecodeFieldStr(this->macFeeType, lpcPtr, CMPP_FEE_TYPE_LEN);
    lpcPtr += DecodeFieldStr(this->macFeeCode, lpcPtr, CMPP_FEE_CODE_LEN);
    lpcPtr += DecodeFieldStr(this->macValidTime, lpcPtr, CMPP_VALID_TIME_LEN);
    lpcPtr += DecodeFieldStr(this->macAtTime, lpcPtr, CMPP_AT_TIME_LEN);

    lpcPtr += DecodeFieldStr(this->macSrcTerminalID, lpcPtr, 21);
    lpcPtr += DecodeFieldStr(this->macSrcTerminalPseudo, lpcPtr, CMPP_TERMINAL_PSEUDO_LEN);
    lpcPtr += DecodeField(&this->mu8SrcTerminalUserType, lpcPtr);
    lpcPtr += DecodeField(&this->mu8SrcTerminalType, lpcPtr);
    lpcPtr += DecodeField(&this->mu8DestUsrTL, lpcPtr);
    lpcPtr += DecodeFieldStr(this->macDestTerminalID, lpcPtr, 21);
    lpcPtr += DecodeFieldStr(this->macDestTerminalPseudo, lpcPtr, CMPP_TERMINAL_PSEUDO_LEN);
    lpcPtr += DecodeField(&this->mu8DestTerminalUserType, lpcPtr);
    lpcPtr += DecodeField(&this->mu8MsgLength, lpcPtr);

    /*
    if(this->mu8MsgFwdType == FWD_TYPE_MT_STATUS )
    {
        lpcPtr += DecodeFieldMsgId(&this->mu64SRMsgID, lpcPtr);
        lpcPtr += DecodeFieldStr(this->macSRStatus, lpcPtr, CMPP_SR_STATUS_LEN);
        lpcPtr += DecodeFieldStr(this->macSRSubmitTime, lpcPtr, CMPP_SR_TIME_LEN);
        lpcPtr += DecodeFieldStr(this->macSRDoneTime, lpcPtr, CMPP_SR_TIME_LEN);
        lpcPtr += DecodeFieldStr(this->macSRDestTerminalID, lpcPtr, 32);
        lpcPtr += DecodeField(&this->mu32SRSMSCSequence, lpcPtr);
    }
    else if(this->mu8MsgFwdType == FWD_TYPE_MO_STATUS)
    {
        lpcPtr += DecodeFieldMsgId(&this->mu64SRMsgID, lpcPtr);
        lpcPtr += DecodeFieldStr(this->macSRStatus, lpcPtr, CMPP_SR_STATUS_LEN);
        lpcPtr += DecodeFieldStr(this->macSRSubmitTime, lpcPtr, CMPP_SR_TIME_LEN);
        lpcPtr += DecodeFieldStr(this->macSRDoneTime, lpcPtr, CMPP_SR_TIME_LEN);
        lpcPtr += DecodeFieldStr(this->macSRDestTerminalID, lpcPtr, 21);
        lpcPtr += DecodeField(&this->mu32SRSMSCSequence, lpcPtr);
    }
    else
    */
    {
        RETURN_ERR_IF((auLen != apl_size_t(273+this->mu8MsgLength)), ERR_CMPP_FWD_DECODE);
        DecodeFieldStr( this->macMsgContent, lpcPtr, this->mu8MsgLength);
        lpcPtr += this->mu8MsgLength;
    }

    lpcPtr += DecodeFieldStr(this->macLinkID, lpcPtr, CMPP_LINK_ID_LEN);

    return lpcPtr - (char *)apvPtr;
}

// ---------------- CCMPPFwdRespPDU ---------------- //
CCMPPFwdRespPDU::CCMPPFwdRespPDU(void)
    : mu64MsgID(0)
    , mu8PkTotal(0)
    , mu8PkNumber(0)
{
}

// ---------------- CCMPP20FwdRespPDU ---------------- //
CCMPP20FwdRespPDU::CCMPP20FwdRespPDU(void)
    : mu8Result(0)
{
}

apl_ssize_t CCMPP20FwdRespPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < (apl_size_t)11), ERR_CMPP_FWD_RESP_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64MsgID);
    lpcPtr += EncodeField(lpcPtr, this->mu8PkTotal);
    lpcPtr += EncodeField(lpcPtr, this->mu8PkNumber);
    lpcPtr += EncodeField(lpcPtr, this->mu8Result);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPP20FwdRespPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen != (apl_size_t)11), ERR_CMPP_FWD_RESP_DECODE);
    const char* lpcPtr = (const char *)apvPtr;

    lpcPtr += DecodeFieldMsgId(&this->mu64MsgID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8PkTotal, lpcPtr);
    lpcPtr += DecodeField(&this->mu8PkNumber, lpcPtr);
    lpcPtr += DecodeField(&this->mu8Result, lpcPtr);

    return lpcPtr - (char *)apvPtr;
}

// ---------------- CCMPP30FwdRespPDU ---------------- //
CCMPP30FwdRespPDU::CCMPP30FwdRespPDU(void)
    : mu32Result(0)
{
}

apl_ssize_t CCMPP30FwdRespPDU::Encode(void* apvPtr, apl_size_t auLen) const
{
    RETURN_ERR_IF((auLen < (apl_size_t)14), ERR_CMPP_FWD_RESP_ENCODE);
    char* lpcPtr = (char *)apvPtr;

    lpcPtr += EncodeFieldMsgId(lpcPtr, this->mu64MsgID);
    lpcPtr += EncodeField(lpcPtr, this->mu8PkTotal);
    lpcPtr += EncodeField(lpcPtr, this->mu8PkNumber);
    lpcPtr += EncodeField(lpcPtr, this->mu32Result);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPP30FwdRespPDU::Decode(const void* apvPtr, apl_size_t auLen)
{
    RETURN_ERR_IF((auLen != (apl_size_t)14), ERR_CMPP_FWD_RESP_DECODE);
    const char* lpcPtr = (const char *)apvPtr;

    lpcPtr += DecodeFieldMsgId(&this->mu64MsgID, lpcPtr);
    lpcPtr += DecodeField(&this->mu8PkTotal, lpcPtr);
    lpcPtr += DecodeField(&this->mu8PkNumber, lpcPtr);
    lpcPtr += DecodeField(&this->mu32Result, lpcPtr);

    return lpcPtr - (char *)apvPtr;
}

// ---------------- CCMPPMessage ---------------- //
CCMPPMessage::CCMPPMessage(void)
    : mpoPDU(APL_NULL)
{
}

CCMPPMessage::~CCMPPMessage(void)
{
    if(mpoPDU != APL_NULL)
    {
        ACL_DELETE(mpoPDU);
    }
}

apl_uint32_t CCMPPMessage::GetTotalLength(void) const
{
    return this->moHeader.GetTotalLength();
}

void CCMPPMessage::SetTotalLength(apl_uint32_t au32TotalLength)
{
    this->moHeader.SetTotalLength(au32TotalLength);
}

apl_uint32_t CCMPPMessage::GetCommandID(void) const
{
    return this->moHeader.GetCommandID();
}

void CCMPPMessage::SetCommandID(apl_uint32_t au32CommandID)
{
    this->moHeader.SetCommandID( au32CommandID );
}

apl_uint32_t CCMPPMessage::GetSequenceID(void) const
{
    return this->moHeader.GetSequenceID();
}

void CCMPPMessage::SetSequenceID(apl_uint32_t au32SequenceID)
{
    this->moHeader.SetSequenceID( au32SequenceID );
}

ICMPPPDUBase* CCMPPMessage::GetPDU(void)
{
    return this->mpoPDU;
}

void CCMPPMessage::SetPDU(ICMPPPDUBase* apoPDU)
{
    this->mpoPDU = apoPDU;
}

const char* CCMPPMessage::GetCMPPMsgCmdStr(void)
{
    const char* lpcPtr = APL_NULL;
    switch(this->moHeader.GetCommandID())
    {
    case CMPP_CONNECT:
        lpcPtr = "CMPP_CONNECT"; break;
    case CMPP_CONNECT_RESP:
        lpcPtr = "CMPP_CONNECT_RESP"; break;
    case CMPP_TERMINATE:
        lpcPtr = "CMPP_TERMINATE"; break;
    case CMPP_TERMINATE_RESP:
        lpcPtr = "CMPP_TERMINATE_RESP"; break;
    case CMPP_SUBMIT:
        lpcPtr = "CMPP_SUBMIT"; break;
    case CMPP_SUBMIT_RESP:
        lpcPtr = "CMPP_SUBMIT_RESP"; break;
    case CMPP_DELIVER:
        lpcPtr = "CMPP_DELIVER"; break;
    case CMPP_DELIVER_RESP:
        lpcPtr = "CMPP_DELIVER_RESP"; break;
    case CMPP_ACTIVE_TEST:
        lpcPtr = "CMPP_ACTIVE_TEST"; break;
    case CMPP_ACTIVE_TEST_RESP:
        lpcPtr = "CMPP_ACTIVE_TEST_RESP"; break;
    case CMPP_FWD:
        lpcPtr = "CMPP_FWD"; break;
    case CMPP_FWD_RESP:
        lpcPtr = "CMPP_FWD_RESP"; break;
    case CMPP_QUERY:
        lpcPtr = "CMPP_QUERY"; break;
    case CMPP_QUERY_RESP:
        lpcPtr = "CMPP_QUERY_RESP"; break;
    case CMPP_CANCEL:
        lpcPtr = "CMPP_CANCEL"; break;
    case CMPP_CANCEL_RESP:
        lpcPtr = "CMPP_CANCEL_RESP"; break;
    default:
        lpcPtr = "unknow cmpp type";
    }

    return lpcPtr;
}

// ---------------- CCMPP20Message ---------------- //
apl_int_t CCMPP20Message::AllocPDU(void)
{
    if(this->mpoPDU != APL_NULL)
    {
        ACL_DELETE(this->mpoPDU);
    }

    switch(this->GetCommandID())
    {
    case CMPP_CONNECT:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP20ConnectPDU);
        break;
    case CMPP_CONNECT_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP20ConnectRespPDU);
        break;
    case CMPP_TERMINATE:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPTerminatePDU);
        break;
    case CMPP_TERMINATE_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPTerminateRespPDU);
        break;
    case CMPP_SUBMIT:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP20SubmitPDU);
        break;
    case CMPP_SUBMIT_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP20SubmitRespPDU);
        break;
    case CMPP_DELIVER:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP20DeliverPDU);
        break;
    case CMPP_DELIVER_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP20DeliverRespPDU);
        break;
    case CMPP_QUERY:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPQueryPDU);
        break;
    case CMPP_QUERY_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPQueryRespPDU);
        break;
    case CMPP_CANCEL:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPCancelPDU);
        break;
    case CMPP_CANCEL_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPCancelRespPDU);
        break;
    case CMPP_ACTIVE_TEST:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPActiveTestPDU);
        break;
    case CMPP_ACTIVE_TEST_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPActiveTestRespPDU);
        break;
    case CMPP_FWD:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP20FwdPDU);
        break;
    case CMPP_FWD_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP20FwdRespPDU);
        break;
    /*
    case CMPP_MT_ROUTE:
    case CMPP_MT_ROUTE_RESP:
    case CMPP_MO_ROUTE:
    case CMPP_MO_ROUTE_RESP:
    case CMPP_GET_MT_ROUTE:
    case CMPP_GET_MT_ROUTE_RESP:
    case CMPP_MT_ROUTE_UPDATE:
    case CMPP_MT_ROUTE_UPDATE_RESP:
    case CMPP_MO_ROUTE_UPDATE:
    case CMPP_MO_ROUTE_UPDATE_RESP:
    case CMPP_PUSH_MT_ROUTE_UPDATE:
    case CMPP_PUSH_MT_ROUTE_UPDATE_RESP:
    case CMPP_PUSH_MO_ROUTE_UPDATE:
    case CMPP_PUSH_MO_ROUTE_UPDATE_RESP:
    case CMPP_GET_MO_ROUTE:
    case CMPP_GET_MO_ROUTE_RESP:
    */
    default:
        return -1;
    }

    return 0;
}

apl_ssize_t CCMPP20Message::Encode(void* apvPtr, apl_size_t auLen)
{
    char* lpcPtr = (char *)apvPtr;

    apl_int_t  liResult = 0;
    apl_size_t luRemainLen = auLen;

    liResult = this->moHeader.Encode(apvPtr, luRemainLen);
    if(liResult < 0)
    {
        return -1;
    }

    lpcPtr      += liResult;
    luRemainLen -= liResult;

    if(this->mpoPDU != APL_NULL)
    {
        liResult = this->mpoPDU->Encode(lpcPtr, luRemainLen);
    }
    else
    {
        return -1;
    }

    lpcPtr      += liResult;
    luRemainLen -= liResult;

    this->SetTotalLength( static_cast<apl_uint32_t>(lpcPtr - (char *)apvPtr) );
    if(this->GetTotalLength() > auLen)
    {
        return -1;
    }

    this->moHeader.Encode(apvPtr, auLen);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPP20Message::Decode(void const* apvPtr, apl_size_t auLen)
{
    const char* lpcPtr = (const char*)apvPtr;
    apl_int_t liResult = 0;
    apl_size_t luTotalLen = 0; 
    apl_size_t luRemainLen = 0;

    liResult = this->moHeader.Decode(apvPtr, auLen);
    if(liResult < 0)
    {
        //auLen < CMPP_HEADER_LEN
        return 0;
    }
	
    if( this->moHeader.GetTotalLength() > CMPP_MAX_PACKAGE )
    {
        //field TotalLength is abnormal
        return -1;
    }

    if( this->moHeader.GetTotalLength() > auLen )
    {
        //haven't finish receive
        return 0;
    }

    luTotalLen = this->GetTotalLength();
    luRemainLen = luTotalLen;

    lpcPtr      += liResult;
    luRemainLen -= liResult;

    liResult = this->AllocPDU();
    if(liResult != 0)
    {
        return -1;
    }

    liResult = this->mpoPDU->Decode(lpcPtr, luRemainLen);
    if(liResult < 0)
    {
        return liResult;
    }

    lpcPtr      += liResult;
    luRemainLen -= liResult;

    return lpcPtr - (char *)apvPtr;
}

// ---------------- CCMPP30Message ---------------- //
apl_int_t CCMPP30Message::AllocPDU(void)
{
    if(this->mpoPDU != APL_NULL)
    {
        ACL_DELETE(this->mpoPDU);
    }

    switch(this->GetCommandID())
    {
    case CMPP_CONNECT:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP30ConnectPDU);
        break;
    case CMPP_CONNECT_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP30ConnectRespPDU);
        break;
    case CMPP_TERMINATE:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPTerminatePDU);
        break;
    case CMPP_TERMINATE_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPTerminateRespPDU);
        break;
    case CMPP_SUBMIT:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP30SubmitPDU);
        break;
    case CMPP_SUBMIT_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP30SubmitRespPDU);
        break;
    case CMPP_DELIVER:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP30DeliverPDU);
        break;
    case CMPP_DELIVER_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP30DeliverRespPDU);
        break;
    case CMPP_QUERY:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPQueryPDU);
        break;
    case CMPP_QUERY_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPQueryRespPDU);
        break;
    case CMPP_CANCEL:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPCancelPDU);
        break;
    case CMPP_CANCEL_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPCancelRespPDU);
        break;
    case CMPP_ACTIVE_TEST:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPActiveTestPDU);
        break;
    case CMPP_ACTIVE_TEST_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPPActiveTestRespPDU);
        break;
    case CMPP_FWD:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP30FwdPDU);
        break;
    case CMPP_FWD_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CCMPP30FwdRespPDU);
        break;
    /*
    case CMPP_MT_ROUTE:
    case CMPP_MT_ROUTE_RESP:
    case CMPP_MO_ROUTE:
    case CMPP_MO_ROUTE_RESP:
    case CMPP_GET_MT_ROUTE:
    case CMPP_GET_MT_ROUTE_RESP:
    case CMPP_MT_ROUTE_UPDATE:
    case CMPP_MT_ROUTE_UPDATE_RESP:
    case CMPP_MO_ROUTE_UPDATE:
    case CMPP_MO_ROUTE_UPDATE_RESP:
    case CMPP_PUSH_MT_ROUTE_UPDATE:
    case CMPP_PUSH_MT_ROUTE_UPDATE_RESP:
    case CMPP_PUSH_MO_ROUTE_UPDATE:
    case CMPP_PUSH_MO_ROUTE_UPDATE_RESP:
    case CMPP_GET_MO_ROUTE:
    case CMPP_GET_MO_ROUTE_RESP:
    */
    default:
        return -1;
    }

    return 0;
}

apl_ssize_t CCMPP30Message::Encode(void* apvPtr, apl_size_t auLen)
{
    char* lpcPtr = (char *)apvPtr;

    apl_int_t  liResult = 0;
    apl_size_t luRemainLen = auLen;

    liResult = this->moHeader.Encode(apvPtr, luRemainLen);
    if(liResult < 0)
    {
        return -1;
    }

    lpcPtr      += liResult;
    luRemainLen -= liResult;

    if(this->mpoPDU != APL_NULL)
    {
        liResult = this->mpoPDU->Encode(lpcPtr, luRemainLen);
    }
    else
    {
        return -1;
    }

    lpcPtr      += liResult;
    luRemainLen -= liResult;

    this->SetTotalLength( static_cast<apl_uint32_t>(lpcPtr - (char *)apvPtr) );
    if(this->GetTotalLength() > auLen)
    {
        return -1;
    }

    this->moHeader.Encode(apvPtr, auLen);

    return lpcPtr - (char *)apvPtr;
}

apl_ssize_t CCMPP30Message::Decode(void const* apvPtr, apl_size_t auLen)
{
    const char* lpcPtr = (const char*)apvPtr;
    apl_int_t liResult = 0;
    apl_size_t luTotalLen = 0;
    apl_size_t luRemainLen = 0;

    liResult = this->moHeader.Decode(apvPtr, auLen);
    if(liResult < 0)
    {
        //auLen < CMPP_HEADER_LEN
        return 0;
    }

    if( this->moHeader.GetTotalLength() > CMPP_MAX_PACKAGE )
    {
        //field TotalLength is abnormal
        return -1;
    }

    if( this->moHeader.GetTotalLength() > auLen )
    {
        return 0;
    }

    luTotalLen = this->GetTotalLength();
    luRemainLen = luTotalLen;

    lpcPtr      += liResult;
    luRemainLen -= liResult;

    liResult = this->AllocPDU();
    if(liResult != 0)
    {
        return -1;
    }

    liResult = this->mpoPDU->Decode(lpcPtr, luRemainLen);
    if(liResult < 0)
    {
        return liResult;
    }

    lpcPtr      += liResult;
    luRemainLen -= liResult;

    return lpcPtr - (char *)apvPtr;
}

} //namespace cmpp

ANF_NAMESPACE_END
