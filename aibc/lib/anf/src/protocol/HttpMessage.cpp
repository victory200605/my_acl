#include "anf/protocol/HttpMessage.h"

ANF_NAMESPACE_START

namespace http {

bool checkSPHT_CRLF_COMMA(char c)
{ 
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n'|| c == ',') 
        return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////
CHttpMessage::CHttpMessage(apl_int_t aiType)
    :moVersion("1.1")
    ,mbContentLengthFlag(true)
    ,mbTransferEncodeFlag(false)
    ,miMsgType(aiType)
{
}



CHttpMessage::~CHttpMessage(){

}

void CHttpMessage::SetMessageID(apl_uint64_t auMsgId)
{
    //FIXME  what would be the message id of http?
    //return 0;
}

apl_uint64_t CHttpMessage::GetMessageID()
{
    //FIXME  what would be the message id of http?
    return 0;
}

void CHttpMessage::AddHeader(std::string const& aoName, std::string const& aoValue, char acSep)
{
    // Search in the filedlist 
    StrStrMMap::iterator lpPos;

    if (aoName.empty())
        return;

    lpPos = moFieldList.find(aoName);

    // the filed exists
    if (lpPos != moFieldList.end())
    {   
        //add nothing
        if (aoValue.empty())
            return;

        //if the exist filed value not null
        if (lpPos->second.size()) 
        {
            lpPos->second += acSep;
            lpPos->second += aoValue;
            return;
        }
    }

    // the filed is new or the value is NULL
    moFieldList[aoName] = aoValue;
}


void CHttpMessage::DelHeader(std::string const& aoName)
{
    // Search in the filedlist 
    StrStrMMap::iterator lpPos;

    if (aoName.empty())
        return;

    lpPos = moFieldList.find(aoName);

    // erase the header if found
    if (lpPos != moFieldList.end())
    {
        moFieldList.erase(aoName);
    }

}

std::string CHttpMessage::GetHeader(std::string const& aoName)
{
    StrStrMMap::iterator lpPos;
    std::string loStr; //the empty string ready to return

    if (aoName.empty())
    {   
        return loStr;
    }

    lpPos = moFieldList.find(aoName);

    //return the value if found the head
    if (lpPos != moFieldList.end())
    {
        return lpPos->second;
    }

    return loStr;
}

StrStrMMap const& CHttpMessage::GetHeaderList(void)
{
    return moFieldList;
}

void CHttpMessage::SetHeader(std::string const& aoName, std::string const& aoValue)
{

    if (aoName.empty())
    {   
        return;
    }

    moFieldList[aoName] = aoValue;
}

void CHttpMessage::ClearHeader()
{
    moFieldList.clear();
}

apl_ssize_t CHttpMessage::AddBody(std::string const& aoData)
{
    apl_ssize_t liLen = moEntity.size();

    moEntity += aoData;
    liLen = moEntity.size() - liLen;

    return liLen;
}


apl_ssize_t CHttpMessage::AddBody(const char* apcBuf, apl_size_t aiBufLen)
{
    apl_ssize_t liLen = moEntity.size();

    if (apcBuf == APL_NULL)
        return -1;

    moEntity.append(apcBuf, apcBuf+aiBufLen);

    liLen = moEntity.size() - liLen;

    return liLen;
}

std::string const& CHttpMessage::GetBody()
{
        return  moEntity;
}


apl_ssize_t CHttpMessage::SetBody(std::string const& aoData)
{
    apl_ssize_t liLen = 0;

    moEntity = aoData;
    liLen = moEntity.size();

    return liLen;
}


apl_ssize_t CHttpMessage::SetBody(const char* apcBuf, apl_size_t aiBufLen)
{
    apl_ssize_t liLen = 0;

    ClearBody();
    liLen = AddBody(apcBuf, aiBufLen);

    return liLen; 
}

void CHttpMessage::ClearBody()
{
    moEntity.erase();
}


apl_size_t CHttpMessage::GetBodySize()
{
    return moEntity.size();
}


std::string const& CHttpMessage::GetHttpVersion(){
    return moVersion;
}


void CHttpMessage::SetHttpVersion(std::string const& aoVersion){
    moVersion = aoVersion;
}


/////////////////////////////Encoder
apl_int_t CHttpMessage::EncodeHeader(acl::CMemoryBlock* apoBuffer)
{
    StrStrMMap::iterator loIter;
    char * lpcCur = APL_NULL;
    char * lpcStart = APL_NULL;
    char* lpcEnd = APL_NULL;
    apl_size_t liBufSize = 0;
    apl_ssize_t liRet;

    lpcStart = apoBuffer->GetWritePtr();
    lpcCur = lpcStart;

    if (NULL == lpcCur)
        return ANF_HTTP_ERROR_INVAL;

    liBufSize = apoBuffer->GetSpace();
    lpcEnd = lpcCur + liBufSize;

    //calc the content len automatically
    if (GetAutoCalcContentLengthFlag())
    {
        apl_size_t liSize = this->GetBodySize();
        char lcaBuf[128]; //128bit for len should enough

        this->DelHeader(ANF_HTTP_FIELD_CONTENT_LENGTH);

        if (liSize > 0) 
        {
            apl_memset(lcaBuf, 0, sizeof(lcaBuf));
            apl_snprintf(lcaBuf, sizeof(lcaBuf), "%lu", liSize);
            this->AddHeader(ANF_HTTP_FIELD_CONTENT_LENGTH, lcaBuf);
        }
    }

    for (loIter = this->moFieldList.begin(); loIter != this->moFieldList.end(); ++loIter) 
    {
        liRet = apl_snprintf(lpcCur, lpcEnd - lpcCur, "%s:%s\r\n", loIter->first.c_str(), loIter->second.c_str());

        if (liRet > (lpcEnd - lpcCur - 1)) 
            return ANF_HTTP_ERROR_TOOBIG;

        lpcCur += liRet; 
    }

    if ((lpcEnd - lpcCur) < 3)
        return ANF_HTTP_ERROR_TOOBIG;

    apl_memcpy(lpcCur, "\r\n\0", 3);

    lpcCur += 2;

    apoBuffer->SetWritePtr(lpcCur);

    return 0;
}

apl_int_t CHttpMessage::EncodeBody(acl::CMemoryBlock* apoBuffer)
{
    apl_ssize_t liRet;
    char * lpcBuf = APL_NULL;
    //apl_size_t liBufSize = 0;

    lpcBuf = apoBuffer->GetWritePtr();

    if (NULL == lpcBuf)
        return ANF_HTTP_ERROR_INVAL;

    return apoBuffer->Write(this->GetBody().data(), this->GetBody().size());

    //liBufSize = apoBuffer->GetSpace();

    //build the body with chunk encoding
#if 0 //currently not support 
    if (GetAutoTransferEncoding() && 
            this->GetHeader(ANF_HTTP_FIELD_TRANSFER_ENCODING) == "chunked")
    {

    }
    else 
    {   
        liRet = apl_snprintf(lpcBuf, liBufSize, this->GetBody().c_str());

        if (liRet < 0 || apl_size_t(liRet + 1) > liBufSize)
            return ANF_HTTP_ERROR_TOOBIG;

    }   

    apoBuffer->SetWritePtr(lpcBuf + liRet);

    return 0;
#endif
}

void CHttpMessage::SetAutoCalcContentLength(bool abFlag)
{
    mbContentLengthFlag = abFlag;
}

void CHttpMessage::SetAutoTransferEncoding(bool abFlag)
{
    mbTransferEncodeFlag = abFlag;
}

bool CHttpMessage::GetAutoCalcContentLengthFlag()
{
    return mbContentLengthFlag;
}

bool CHttpMessage::GetAutoTransferEncodingFlag()
{
    return mbTransferEncodeFlag;
}

apl_int_t CHttpMessage::EncodeMessage(acl::CMemoryBlock* apoBuffer)
{
    char* lpcStart = apoBuffer->GetWritePtr();
    apl_int_t liRet = 0;

    this->SaveMBWritePtr(apoBuffer);

    if (lpcStart == APL_NULL)
        return ANF_HTTP_ERROR_INVAL;

    liRet = EncodeStartLine(apoBuffer);
    if (liRet <0) goto ENCODE_FAILED; 

    liRet = EncodeHeader(apoBuffer);
    if (liRet <0) goto ENCODE_FAILED; 

    liRet = EncodeBody(apoBuffer);
    if (liRet <0) goto ENCODE_FAILED; 

    return apoBuffer->GetWritePtr() - lpcStart;

ENCODE_FAILED:
    this->RestoreMBWritePtr(apoBuffer);
    return liRet;
}

void CHttpMessage::SaveMBWritePtr(acl::CMemoryBlock* apoBuffer)
{
    this->mpcMBWritePtr = apoBuffer->GetWritePtr();
}

void CHttpMessage::RestoreMBWritePtr(acl::CMemoryBlock* apoBuffer)
{
    apoBuffer->SetWritePtr(this->mpcMBWritePtr);
}


/////////////////////////////Decoder
apl_int_t CHttpMessage::DecodeHeader(acl::CMemoryBlock* apoBuffer)
{
    const char* lpcStart = apoBuffer->GetReadPtr();
    apl_size_t liBufSize = apoBuffer->GetLength();
    const char* lpcBeginOfLine = lpcStart;
    const char* lpcEndOfLine;
    const char* lpcSep;
    char lsName[ANF_HTTP_MAX_LINE_LENGTH + 1];
    char lsValue[ANF_HTTP_MAX_LINE_LENGTH + 1]; 
    bool lbIsEnd = false;

    if (lpcStart == APL_NULL)
        return ANF_HTTP_ERROR_INVAL;

    apl_memset(lsName, 0, sizeof(lsName));
    apl_memset(lsValue, 0, sizeof(lsValue));

    while (true)
    {
        lpcEndOfLine = apl_strstr(lpcBeginOfLine, "\r\n");

        if (lpcEndOfLine == NULL || lpcEndOfLine > lpcStart + liBufSize)
        {
            //not enough content
            return 0; 
        }

        if (lpcEndOfLine == lpcBeginOfLine)
        {
            lpcBeginOfLine = lpcEndOfLine + strlen("\r\n");

            if (lpcEndOfLine != lpcStart)
            {
                lbIsEnd = true;
            }

            break; // reach end of buffer, break and return; 
        }

        assert(lpcEndOfLine > lpcBeginOfLine);

        if (apl_size_t(lpcEndOfLine - lpcBeginOfLine) > ANF_HTTP_MAX_LINE_LENGTH)
            return ANF_HTTP_ERROR_TOOBIG; 

        //LWS = [CRLF] 1*( SP | HT ) 
        if (apl_isspace(lpcBeginOfLine[0])) 
        { // LWS
            if (strlen(lsName) == 0)
                return ANF_HTTP_ERROR_FIELDNAME;

            apl_memset(lsValue, 0, sizeof(lsValue));
            apl_memcpy(lsValue, lpcBeginOfLine, lpcEndOfLine - lpcBeginOfLine);
            acl::stralgo::TrimIf(lsValue, checkSPHT_CRLF_COMMA);
           
            apl_size_t liValueLen = this->GetHeader(lsName).size();
            apl_size_t liAddLen = apl_strlen(lsValue);

            if (liAddLen == 0)
                return ANF_HTTP_ERROR_FIELDVALUE;

            if (liValueLen + 1 + liAddLen + 1 > ANF_HTTP_MAX_LINE_LENGTH)
                return ANF_HTTP_ERROR_FIELDVALUE;
           
            this->AddHeader(lsName, lsValue, ' ');
        }
        else
        {
            lpcSep = (const char*)apl_memchr(lpcBeginOfLine, ':', lpcEndOfLine - lpcBeginOfLine);

            if (NULL == lpcSep)
                return ANF_HTTP_ERROR_FIELDSEP;
           
            apl_memset(lsName, 0, sizeof(lsName));
            apl_memcpy(lsName, lpcBeginOfLine, lpcSep - lpcBeginOfLine);
            acl::stralgo::TrimIf(lsName, checkSPHT_CRLF_COMMA);

            apl_memset(lsValue, 0, sizeof(lsValue));
            apl_memcpy(lsValue, lpcSep + 1, lpcEndOfLine - (lpcSep + 1));
            acl::stralgo::TrimIf(lsValue, checkSPHT_CRLF_COMMA);
           
            if (apl_strlen(lsName) == 0)
                return ANF_HTTP_ERROR_FIELDNAME;

            if (apl_strlen(lsValue) == 0)
                return ANF_HTTP_ERROR_FIELDVALUE;

            this->AddHeader(lsName, lsValue); 
        }

        lpcBeginOfLine = lpcEndOfLine + strlen("\r\n");
    }

    if (!lbIsEnd) 
    {
        return 0;
    }

    liBufSize = lpcBeginOfLine - lpcStart;
    apoBuffer->SetReadPtr(liBufSize);

    return liBufSize;
}

apl_int_t CHttpMessage::DecodeBody(acl::CMemoryBlock* apoBuffer)
{
    const char* lpcStart = apoBuffer->GetReadPtr();
    apl_size_t liBufSize = apoBuffer->GetLength();
    const char* lpcCur = lpcStart;
    std::string lpoValue;
    apl_int32_t liEntitySize;
    apl_ssize_t liRet;
    apl_int_t liErrno = 0;

    lpoValue = this->GetHeader(ANF_HTTP_FIELD_TRANSFER_ENCODING);

    if (!lpoValue.empty())
    { // chunk encoding
        if (lpoValue.compare("chunked") != 0)
            return ANF_HTTP_ERROR_FIELDVALUE;
       
        liRet = DecodeBodyChunked(apoBuffer);

        return liRet; 
    }

    lpoValue = this->GetHeader(ANF_HTTP_FIELD_CONTENT_LENGTH);

    if (!lpoValue.empty())
    {
        liEntitySize = apl_strtoi32(lpoValue.c_str(), NULL, 10);
        liErrno = apl_get_errno();

        if (liEntitySize < 0
                || liErrno == EINVAL 
                || liErrno == ERANGE)
            return ANF_HTTP_ERROR_SIZE;

        //not all message received yet
        if (apl_size_t(liEntitySize) > liBufSize)
            return 0;
       
        liRet = this->SetBody(lpcCur, liEntitySize);

        if (liRet != liEntitySize)
        {
            //DelBody(); ? 
            assert(false);
        }
    
        apoBuffer->SetReadPtr(liRet);

        return liRet;
    }
    //
    else 
        return ANF_HTTP_ERROR_CONTENT_LENGTH;
}

apl_int_t CHttpMessage::DecodeBodyChunked(acl::CMemoryBlock* apoBuffer)
{
    const char* lpcStart = apoBuffer->GetReadPtr();
    apl_size_t liBufSize = apoBuffer->GetLength();
    const char* lpcCur = lpcStart;
    const char* lpcEndOfLine = APL_NULL;
    apl_int32_t liChunkSize = 0;
    apl_ssize_t liRet;
    apl_ssize_t liContentSize = 0;
    char lcaBuf[64] = {0};
    apl_int_t liErrno = 0;

    while (true)
    {
        // recv chunk size
        lpcEndOfLine = apl_strstr(lpcCur, "\r\n");

        if (lpcEndOfLine == NULL || lpcEndOfLine > lpcStart + liBufSize)
            return 0; //not enough content to decode

        // handle chunk size
        liChunkSize = apl_strtoi32(lpcCur, NULL, 16);
        liErrno = apl_get_errno();

        if (liChunkSize < 0
                || liErrno == EINVAL 
                || liErrno == ERANGE)
            return ANF_HTTP_ERROR_SIZE;

        //go to next line
        lpcCur = lpcEndOfLine + 2;

        if (0 == liChunkSize)
        { // last chunk
            //set the memory block for decodeheader 
            apoBuffer->SetReadPtr(lpcCur-lpcStart);
            //handle the trailer header
            liRet = DecodeHeader(apoBuffer); 

            if (liRet < 0) 
                return liRet;

            break;
        }

        // recv entity 
        lpcEndOfLine = lpcCur + liChunkSize; 

        if (0 != apl_memcmp(lpcEndOfLine, "\r\n", 2) 
                || lpcEndOfLine > lpcStart + liBufSize)
            return ANF_HTTP_ERROR_EOL;

        liRet = this->AddBody(lpcCur, liChunkSize);

        if (liRet != liChunkSize)
        {
            //this->ClearBody();
            assert(false); //add body failed
        }

        liContentSize += liRet;
        
        //go to next line
        lpcCur = lpcEndOfLine + 2;
    }

    //AddHeader(Content-Length := length
    apl_snprintf(lcaBuf, sizeof(lcaBuf), APL_PRIu32, liContentSize);
    this->AddHeader(ANF_HTTP_FIELD_CONTENT_LENGTH, lcaBuf);
    this->DelHeader(ANF_HTTP_FIELD_TRANSFER_ENCODING);

    //how many bytes we read
    liRet = lpcCur + liRet - lpcStart;

    //the memory block read ptr had set while decode header
    return liRet;
}

void CHttpMessage::SaveMBReadPtr(acl::CMemoryBlock* apoBuffer)
{
    this->mpcMBReadPtr = apoBuffer->GetReadPtr();
}

void CHttpMessage::RestoreMBReadPtr(acl::CMemoryBlock* apoBuffer)
{
    apoBuffer->SetReadPtr(this->mpcMBReadPtr);
}

apl_int_t CHttpMessage::DecodeMessage(acl::CMemoryBlock* apoBuffer)
{
    //char* lpcStart = apoBuffer->GetReadPtr();
    //char* lpcEnd = APL_NULL;
    char* lpcStart = APL_NULL;
    apl_int_t liRet = 0;

    this->SaveMBReadPtr(apoBuffer);

    liRet = DecodeStartLine(apoBuffer);
    if (liRet <= 0) 
        goto DECODE_FAILED;

    liRet = DecodeHeader(apoBuffer);
    if (liRet <= 0) 
        goto DECODE_FAILED;

    liRet = DecodeBody(apoBuffer);
    if (liRet <= 0) 
        goto DECODE_FAILED;

    //lpcEnd = apoBuffer->GetReadPtr();
    //liRet = lpcEnd - lpcStart;
    
    lpcStart = apoBuffer->GetReadPtr();

    //how many bytes we read 
    liRet = lpcStart - this->mpcMBReadPtr;
    return liRet;

DECODE_FAILED:
    this->RestoreMBReadPtr(apoBuffer);

    if (liRet == ANF_HTTP_ERROR_BUF_NOT_ENOUGH)
        return 0;

    return liRet;
}

apl_int_t CHttpMessage::GetMsgType()
{
    return this->miMsgType;
}

void CHttpMessage::SetErrMsg(std::string const & aoErrMsg)
{
}

const char* CHttpMessage::GetErrMsg()
{
    return APL_NULL;
}

void CHttpMessage::SetErrCode(apl_int_t aiErrCode)
{
}

apl_int_t CHttpMessage::GetErrCode()
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

CHttpRequest::CHttpRequest()
    :CHttpMessage(MESSAGE_TYPE_HTTP_REQUEST)
{
}

CHttpRequest::~CHttpRequest()
{
}

std::string const& CHttpRequest::GetMethod()
{
    return  moMethod;
}

std::string const& CHttpRequest::GetRequestURI()
{
    return  moUri;
}

void CHttpRequest::SetMethod(std::string const& aoMethod)
{
    moMethod = aoMethod;
}

void CHttpRequest::SetRequestURI(std::string const& aoUri)
{
    moUri = aoUri;
}

apl_int_t CHttpRequest::EncodeStartLine(acl::CMemoryBlock* apoBuffer)
{
    apl_ssize_t liRet;
    char * lpcBuf = APL_NULL;
    apl_size_t liBufSize = 0;

    lpcBuf = apoBuffer->GetWritePtr();

    if (NULL == lpcBuf)
        return ANF_HTTP_ERROR_INVAL;

    liBufSize = apoBuffer->GetSpace();

    liRet = apl_snprintf(lpcBuf, liBufSize, "%s %s HTTP/%s\r\n", 
            this->GetMethod().c_str(), this->GetRequestURI().c_str(), this->GetHttpVersion().c_str());

    if (liRet < 0 || apl_size_t(liRet + 1) > liBufSize)
        return ANF_HTTP_ERROR_TOOBIG;

    apoBuffer->SetWritePtr(lpcBuf + liRet);

    return 0;
}

apl_int_t CHttpRequest::DecodeStartLine(acl::CMemoryBlock* apoBuffer)
{
    const char* lpcStart = apoBuffer->GetReadPtr();
    apl_size_t liBufSize = apoBuffer->GetLength();
    const char* lpcCur = lpcStart;
    const char* lpcSep;
    std::string loStr;

    if (liBufSize < apl_strlen("xxx x HTTP/x.y\r\n"))
        return 0; //no error, but 0 bytes are decode

    lpcSep = apl_strchr(lpcCur, ' ');

    if (NULL == lpcSep || lpcSep == lpcCur)
        return ANF_HTTP_ERROR_METHOD;

    //set the method 
    loStr.assign(lpcCur, lpcSep);
    this->SetMethod(loStr);

    lpcCur = lpcSep + 1;
    lpcSep = apl_strchr(lpcCur, ' ');

    if (NULL == lpcSep || lpcSep == lpcCur)
        return ANF_HTTP_ERROR_URI;

    loStr.assign(lpcCur, lpcSep);
    this->SetRequestURI(loStr);

    lpcCur = lpcSep + 1;

    if (apl_memcmp(lpcCur, "HTTP/", 5) != 0
        || !apl_isdigit(lpcCur[5])
        || '.' != lpcCur[6]
        || !apl_isdigit(lpcCur[7])
        || apl_memcmp(lpcCur + 8, "\r\n", 2) != 0)
    {
        return ANF_HTTP_ERROR_VERSION;
    }

    loStr.assign(lpcCur + 5, lpcCur+8);
    this->SetHttpVersion(loStr);

    //10 is the "HTTP/x.x\r\n" 's length
    liBufSize  = lpcCur - lpcStart + 10;
    apoBuffer->SetReadPtr(liBufSize);

    return liBufSize; 
}

///////////////////////////////////////////////////////////////////////////////

CHttpResponse::CHttpResponse()
    :CHttpMessage(MESSAGE_TYPE_HTTP_RESPONSE)
    ,muStatus(0)
{
}

CHttpResponse::~CHttpResponse(){

}

std::string const& CHttpResponse::GetReasonPhrase()
{
    return  moReason;
}

apl_size_t CHttpResponse::GetStatusCode()
{
    return  muStatus;
}

void CHttpResponse::SetReasonPhrase(std::string const& aoReasonPhrase)
{
    moReason = aoReasonPhrase;
}

void CHttpResponse::SetStatusCode(apl_uint32_t aiStatus)
{
    muStatus = aiStatus;
}

apl_int_t CHttpResponse::EncodeStartLine(acl::CMemoryBlock* apoBuffer)
{
    apl_ssize_t liRet;
    char * lpcBuf = APL_NULL;
    apl_size_t liBufSize = 0;

    lpcBuf = apoBuffer->GetWritePtr();

    if (NULL == lpcBuf)
        return ANF_HTTP_ERROR_INVAL;

    liBufSize = apoBuffer->GetSpace();

    liRet = apl_snprintf(
        lpcBuf,
        liBufSize,
        "HTTP/%s %03u %s\r\n",
        this->GetHttpVersion().c_str(),
        this->GetStatusCode() % 1000, 
        this->GetReasonPhrase().c_str()
    );

    if (liRet < 0 || apl_size_t(liRet + 1) > liBufSize)
        return ANF_HTTP_ERROR_TOOBIG;

    apoBuffer->SetWritePtr(lpcBuf + liRet);

    return 0;
}

apl_int_t CHttpResponse::DecodeStartLine(acl::CMemoryBlock* apoBuffer)
{
    const char* lpcStart = apoBuffer->GetReadPtr();
    apl_size_t liBufSize = apoBuffer->GetLength();
    const char* lpcCur = lpcStart;
    const char* lpcEnd = APL_NULL;
    std::string loStr;
    apl_ssize_t liReasonSize = 0;

    if (liBufSize < apl_strlen("HTTP/x.y abc \r\n")) 
        return 0; //no error, but 0 bytes are decode

    if (apl_memcmp(lpcCur, "HTTP/", 5) != 0
        || !apl_isdigit(lpcCur[5])
        || '.' != lpcCur[6]
        || !apl_isdigit(lpcCur[7])
        || ' ' != lpcCur[8])
    {
        return ANF_HTTP_ERROR_VERSION;
    }

    loStr.assign(lpcCur + 5, lpcCur+8);
    this->SetHttpVersion(loStr);

    if (!apl_isdigit(lpcCur[9])
        || !apl_isdigit(lpcCur[10])
        || !apl_isdigit(lpcCur[11])
        || ' ' != lpcCur[12])
    {
        return ANF_HTTP_ERROR_STATUS;
    }

    this->SetStatusCode(apl_strtou32(lpcCur + 9, NULL, 10));

    lpcEnd = apl_strstr(lpcCur, "\r\n");

    if (NULL == lpcEnd || lpcEnd < lpcCur + 13)
        return  ANF_HTTP_ERROR_EOL; 

    liReasonSize = lpcEnd - (lpcCur + 13);

    loStr.assign(lpcCur + 13, lpcCur+13+liReasonSize);
    this->SetReasonPhrase(loStr);

    liBufSize = lpcEnd - lpcCur + apl_strlen("\r\n"); 
    apoBuffer->SetReadPtr(liBufSize);

    return liBufSize;
}

////////////////////////////////////////////////////////////////////////////////

CHttpInvalid::CHttpInvalid()
    :CHttpMessage(MESSAGE_TYPE_HTTP_INVALID)
{
}

apl_int_t CHttpInvalid::EncodeStartLine(acl::CMemoryBlock* apoBuffer)
{
    return ANF_HTTP_ERROR_INVAL;
}

apl_int_t CHttpInvalid::DecodeStartLine(acl::CMemoryBlock* apoBuffer)
{
    return ANF_HTTP_ERROR_INVAL;
}

void CHttpInvalid::SetErrMsg(std::string const & aoErrMsg)
{
    this->moErrMsg = aoErrMsg;
}

const char* CHttpInvalid::GetErrMsg()
{
    return this->moErrMsg.c_str();
}

void CHttpInvalid::SetErrCode(apl_int_t aiErrCode)
{
    this->miErrCode = aiErrCode;
}

apl_int_t CHttpInvalid::GetErrCode()
{
    return this->miErrCode;
}

////////////////////////////////////////////////////////////////////////////////

CHttpMessage* DecodeStartLine(acl::CMemoryBlock* apoBuffer, apl_size_t* apuPkgLen)
{
    const char* lpcStart = apoBuffer->GetReadPtr();
    apl_size_t liBufSize = apoBuffer->GetLength();
    CHttpMessage* lpoHttpMsg = APL_NULL;
    apl_int_t liStatus = 0;

    if (liBufSize < apl_strlen("HTTP/x.y abc \r\n")) 
    {
        //not enough content
        return APL_NULL;
    }

    if (apl_memcmp(lpcStart, "HTTP/", 5) != 0
        || !apl_isdigit(lpcStart[5])
        || '.' != lpcStart[6]
        || !apl_isdigit(lpcStart[7])
        || ' ' != lpcStart[8])
    {
        //req
        ACL_NEW_ASSERT(lpoHttpMsg, CHttpRequest);
    }
    else 
    {
        //resp
        ACL_NEW_ASSERT(lpoHttpMsg, CHttpResponse);
    }

    liStatus = lpoHttpMsg->DecodeStartLine(apoBuffer);

    if (liStatus < 0)
    {
        ACL_DELETE(lpoHttpMsg);
        ACL_NEW_ASSERT(lpoHttpMsg, CHttpInvalid);
        lpoHttpMsg->SetErrMsg("Invalid message");
        lpoHttpMsg->SetErrCode(liStatus);
    }
    else if (liStatus == 0)
    {
        ACL_DELETE(lpoHttpMsg);
        return APL_NULL;
    }

    return lpoHttpMsg; 
}

}

ANF_NAMESPACE_END
