#include "LogServerProtocol.h"


CLogServerProtocol::CLogServerProtocol(CLogServerMsg* apoLogServerMsg)
    :muContentLen(0)
    ,muSeq(0)
    ,muMod(0)
    ,miStatus(-1)
    ,muMsgType(LOG_SERVER_PROTOCOL_TYPE_INVALID)
    ,mpoLogServerMsg(apoLogServerMsg)
{
    this->muContentLen = this->CalContentLen(); 
}

CLogServerProtocol::~CLogServerProtocol()
{}

apl_uint32_t CLogServerProtocol::GetLen()
{
    return (this->muContentLen + LOG_SERVER_PROTOCOL_HEADER_LEN);
}

void CLogServerProtocol::SetSeq(apl_uint32_t auSeq)
{
    this->muSeq = auSeq;
}

apl_uint32_t CLogServerProtocol::GetSeq()
{
    return this->muSeq;
}

void CLogServerProtocol::SetType(apl_uint32_t auType)
{
    this->muMsgType = auType;
}

apl_uint32_t CLogServerProtocol::GetType()
{
    return this->muMsgType;
}

void CLogServerProtocol::SetMode(apl_uint32_t auMode)
{
    this->muMod = auMode;
}

apl_uint32_t CLogServerProtocol::GetMode()
{
    return this->muMod;
}

void CLogServerProtocol::SetStatus(apl_int32_t aiStatus)
{
    this->miStatus = aiStatus;
}

apl_int32_t CLogServerProtocol::GetStatus()
{
    return this->miStatus;
}

CLogServerMsg* CLogServerProtocol::GetLogServerMsg()
{
    return  this->mpoLogServerMsg;
}

apl_int_t CLogServerProtocol::Encode(char* apcBuf, apl_uint32_t auLen)
{
    char* lpcBuf = apcBuf;
    apl_uint32_t luTotalLen = this->muContentLen + LOG_SERVER_PROTOCOL_HEADER_LEN;
    apl_uint32_t luContentLen;

    if (auLen < luTotalLen)
        return -1;

    apl_memcpy(lpcBuf, &luTotalLen, sizeof(luTotalLen));
    lpcBuf += sizeof(luTotalLen);

    apl_memcpy(lpcBuf, &this->muSeq, sizeof(this->muSeq));
    lpcBuf += sizeof(this->muSeq);

    apl_memcpy(lpcBuf, &this->muMsgType, sizeof(this->muMsgType));
    lpcBuf += sizeof(this->muMsgType);

    //response message encode
    if ((this->muMsgType & LOG_SERVER_PROTOCOL_TYPE_RSP_MASK) != 0) 
    {
        apl_memcpy(lpcBuf, &this->miStatus, sizeof(this->miStatus));
        lpcBuf += sizeof(this->miStatus);
    }
    //request message encode or other
    else
    {
        apl_memcpy(lpcBuf, &this->muMod, sizeof(this->muMod));
        lpcBuf += sizeof(this->muMod);
    }

    luContentLen = CopyContentToBuf(lpcBuf, auLen - LOG_SERVER_PROTOCOL_HEADER_LEN);

    //some error happen while copying content
    if (luContentLen != this->muContentLen)
        return -1;

    //return how many bytes we encoded
    return (lpcBuf - apcBuf + luContentLen);
}

apl_int_t CLogServerProtocol::Decode(char* apcBuf, apl_uint32_t auLen)
{
    char* lpcBuf = apcBuf;
    apl_uint32_t luTotalLen = 0;
    apl_uint32_t luSeq = 0;
    apl_uint32_t luMod = 0;
    apl_int32_t  liStatus = 0;
    apl_int32_t  luMsgType = 0;

    //not enough header to decode
    if (auLen < LOG_SERVER_PROTOCOL_HEADER_LEN) 
        return 0;

    apl_memcpy(&luTotalLen, lpcBuf, sizeof(luTotalLen));
    lpcBuf += sizeof(luTotalLen);

    //invalid length
    if (luTotalLen < LOG_SERVER_PROTOCOL_HEADER_LEN)
        return -1;

    //not enough content to decode
    if (luTotalLen > auLen)
        return 0;

    this->muContentLen = luTotalLen - LOG_SERVER_PROTOCOL_HEADER_LEN;

    apl_memcpy(&luSeq, lpcBuf, sizeof(luSeq));
    lpcBuf += sizeof(luSeq);
    this->muSeq = luSeq;

    apl_memcpy(&luMsgType, lpcBuf, sizeof(luMsgType));
    lpcBuf += sizeof(luMsgType);
    this->muMsgType = luMsgType;

    //response 
    if ((luMsgType & LOG_SERVER_PROTOCOL_TYPE_RSP_MASK) != 0)
    {
        apl_memcpy(&liStatus, lpcBuf, sizeof(liStatus));
        lpcBuf += sizeof(liStatus);
        this->miStatus = liStatus;
    }
    else //request
    {
        apl_memcpy(&luMod, lpcBuf, sizeof(luMod));
        lpcBuf += sizeof(luMod);
        this->muMod = luMod;
    }

    //decode content to a CLogServerMsg
    if (mpoLogServerMsg != APL_NULL)
    {
        apl_uint16_t luTag = 0;
        apl_uint16_t luLen = 0;

        while (lpcBuf < apcBuf + luTotalLen)
        {
            apl_memcpy(&luTag, lpcBuf, sizeof(luTag));
            lpcBuf += sizeof(luTag);
            apl_memcpy(&luLen, lpcBuf, sizeof(luLen));
            lpcBuf += sizeof(luLen);
            this->mpoLogServerMsg->AddTagValue(luTag, lpcBuf, luLen);
            lpcBuf += luLen;
        }

        //might encount errors 
        if (lpcBuf != apcBuf + luTotalLen)
            return -1;
    }

    //return how many bytes we decoded
    return lpcBuf - apcBuf;
}

apl_uint32_t CLogServerProtocol::CalContentLen()
{
    apl_uint32_t luContentLen = 0;
    apl_uint16_t luTagLen = 0;
    apl_int_t liSize; 
    
    if (mpoLogServerMsg == APL_NULL)
        return 0;

    liSize = mpoLogServerMsg->GetSize();

    for (apl_int_t liIdx = 0; liIdx < liSize; ++liIdx)
    {
        luTagLen = 0;
        mpoLogServerMsg->GetTLV(liIdx, APL_NULL, &luTagLen, APL_NULL);
        luContentLen += luTagLen;
    }

    return luContentLen;
}

apl_uint32_t CLogServerProtocol::CopyContentToBuf(char* apcBuf, apl_uint32_t auLen)
{
    char* lpcBuf = apcBuf;
    apl_uint16_t luTag = 0;
    apl_uint16_t luTagLen = 0;
    apl_int_t liSize;
    char* lpcValue = APL_NULL;

    if (mpoLogServerMsg == APL_NULL)
        return 0;

    //not enough space
    if (auLen < this->muContentLen)
        return -1;

    liSize = mpoLogServerMsg->GetSize();

    for (apl_int_t liIdx = 0; liIdx < liSize; ++liIdx)
    {
        mpoLogServerMsg->GetTLV(liIdx, &luTag, &luTagLen, (void**)&lpcValue);

        apl_memcpy(lpcBuf, &luTag, sizeof(luTag));
        lpcBuf += sizeof(luTag);

        apl_memcpy(lpcBuf, &luTagLen, sizeof(luTagLen));
        lpcBuf += sizeof(luTagLen);

        if (lpcValue != APL_NULL)
        {
            apl_memcpy(lpcBuf, lpcValue, luTagLen);
            lpcBuf += luTagLen;
        } else if (luTagLen != 0)
        {
            //Len is not 0, bug Value is NULL!
            return -1;
        }
    }

    return lpcBuf - apcBuf;
}
