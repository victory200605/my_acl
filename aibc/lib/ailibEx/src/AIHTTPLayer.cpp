#include <string.h>
#include <ctype.h>

#include "AILib.h"
#include "AIDualLink.h"
#include "AIString.h"
#include "AIChunkEx.h"
#include "AIHTTPLayer.h"

///start namespace
AIBC_NAMESPACE_START

#define _CLOSE_RETURN_IF(ret, exp) if(exp) { Close(); return ret; }


size_t const AI_HTTP_MAX_LINE_LENGTH = 1024;
size_t const AI_HTTP_MAX_HEADER_LENGTH = 4096;

////////////////////////////////////////////////////////////////////////////
struct stHeaderField
{
    LISTHEAD    coFieldNode;
    char*       cpcName;
    char*       cpcValue;
};


static inline stHeaderField* _MallocHeaderField(LISTHEAD* apoList, char const* apcName, char const* apcValue)
{
    stHeaderField*  lpoField;
    size_t          liLen;

    AI_MALLOC_ASSERT(lpoField, stHeaderField, sizeof(stHeaderField));

    liLen = strlen(apcName) + 1;
    AI_MALLOC_ASSERT(lpoField->cpcName, char, strlen(apcName) + 1);
    StringCopy(lpoField->cpcName, apcName, liLen);

    liLen = strlen(apcValue) + 1;
    AI_MALLOC_ASSERT(lpoField->cpcValue, char, strlen(apcValue) + 1);
    StringCopy(lpoField->cpcValue, apcValue, liLen);

    if (apoList)
    {
        ai_list_add_tail(&(lpoField->coFieldNode), apoList);
    }

    return lpoField; 
}


static inline void _AppendHeaderFieldValue(stHeaderField* apoField, char const* apcValueToAppend)
{
    size_t liLen = strlen(apoField->cpcValue);
    size_t liIncLen = strlen(apcValueToAppend);

    if (0 == liLen)
    {
        AI_REALLOC_ASSERT(apoField->cpcValue, char, liIncLen + 1);
        StringCopy(apoField->cpcValue, apcValueToAppend, liIncLen + 1);
    }
    else
    {
        AI_REALLOC_ASSERT(apoField->cpcValue, char, liLen + 1 + liIncLen + 1);
        snprintf(apoField->cpcValue + liLen, 1 + liIncLen + 1, ",%s", apcValueToAppend);
    }
}


static inline int _FreeHeaderField(stHeaderField* apoField)
{
    AI_RETURN_IF(AI_HTTP_ERROR_INVAL, (NULL == apoField));

    ai_list_del_any(&(apoField->coFieldNode));
    AI_FREE(apoField->cpcName);
    AI_FREE(apoField->cpcValue);
    AI_FREE(apoField);

    return 0;
}


static inline stHeaderField* _SearchHeaderField(LISTHEAD const* apoList, char const* apcName)
{
    AI_FOR_EACH_IN_LIST(lpoField, apoList, stHeaderField, coFieldNode)
    {
        AI_RETURN_IF(lpoField, (strcmp(apcName, lpoField->cpcName) == 0));
    }
    return NULL;
}


static inline stHeaderField* _GetHeaderField(LISTHEAD const* apoList, size_t aiIndex)
{
    size_t liCount = 0;

    AI_FOR_EACH_IN_LIST(lpoField, apoList, stHeaderField, coFieldNode)
    {
        AI_RETURN_IF(lpoField, (liCount == aiIndex));

        ++liCount;
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////
struct stHTTPEntity
{
    LISTHEAD    coEntityNode;
    size_t      ciLength;
    char*       cpcEntity;
};


static inline stHTTPEntity* _MallocHTTPEntity(LISTHEAD* apoList, void const* apBuf, size_t aiBufSize)
{
    stHTTPEntity* lpoEntity;

    AI_MALLOC_ASSERT(lpoEntity, stHTTPEntity, sizeof(stHTTPEntity));
    AI_MALLOC_ASSERT(lpoEntity->cpcEntity, char, aiBufSize + 1);

    lpoEntity->ciLength = aiBufSize;
    memcpy(lpoEntity->cpcEntity, apBuf, aiBufSize);
    lpoEntity->cpcEntity[aiBufSize] = '\0';

    if (apoList)
    {
        ai_list_add_tail(&(lpoEntity->coEntityNode), apoList);
    }

    return lpoEntity; 
}


static inline void _ReallocHTTPEntity(stHTTPEntity* apoEntity, void const* apBuf, size_t aiBufSize)
{
    AI_REALLOC_ASSERT(apoEntity->cpcEntity, char, aiBufSize + 1);
    memcpy(apoEntity->cpcEntity, apBuf, aiBufSize);
    apoEntity->cpcEntity[aiBufSize] = '\0';
    apoEntity->ciLength = aiBufSize;
}

static inline int _FreeHTTPEntity(stHTTPEntity* apoEntity)
{
    AI_RETURN_IF(AI_HTTP_ERROR_INVAL, (NULL == apoEntity));

    ai_list_del_any(&(apoEntity->coEntityNode));
    AI_FREE(apoEntity->cpcEntity);
    AI_FREE(apoEntity);

    return 0;
}


static inline stHTTPEntity* _GetHTTPEntity(LISTHEAD const* apoList, size_t aiIndex)
{
    size_t liCount = 0;

    AI_FOR_EACH_IN_LIST(lpoEntity, apoList, stHTTPEntity, coEntityNode)
    {
        AI_RETURN_IF(lpoEntity, (liCount == aiIndex));

        ++liCount;
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////
clsHTTPMessage::clsHTTPMessage()
    :ciVersion(11)
{
    ai_init_list_head(&coFieldList);
    ai_init_list_head(&coEntityList);
}


clsHTTPMessage::~clsHTTPMessage()
{
    Reset();
}


void clsHTTPMessage::Reset()
{
    while (!ai_list_is_empty(&coFieldList))
    {
        _FreeHeaderField(
            AI_FIRST_OF_LIST(&coFieldList, stHeaderField, coFieldNode)
        );
    }

    while (!ai_list_is_empty(&coEntityList))
    {
        _FreeHTTPEntity(
            AI_FIRST_OF_LIST(&coEntityList, stHTTPEntity, coEntityNode)
        );
    }
}


unsigned clsHTTPMessage::GetVersion() const
{
    return ciVersion;
}


void clsHTTPMessage::SetVersion(unsigned aiVersion)
{
    ciVersion = aiVersion % 100;
}


int clsHTTPMessage::AddField(char const* apcName, char const* apcValue)
{
    AI_RETURN_IF(AI_HTTP_ERROR_INVAL, (NULL == apcName || NULL == apcValue));

    stHeaderField* lpoField = _SearchHeaderField(&coFieldList, apcName);

    if (NULL == lpoField)
    {
        lpoField = _MallocHeaderField(&coFieldList, apcName, apcValue);
    }
    else
    {
        _AppendHeaderFieldValue(lpoField, apcValue);
    }

    return 0;
}


int clsHTTPMessage::DelField(char const* apcName)
{
    AI_RETURN_IF(AI_HTTP_ERROR_INVAL, (NULL == apcName));

    return _FreeHeaderField(
        _SearchHeaderField(&coFieldList, apcName)
    );    
}


size_t clsHTTPMessage::GetFieldCount() const
{
    size_t liCount = 0;

    AI_FOR_EACH_IN_LIST(lpoField, &coFieldList, stHeaderField, coFieldNode)
    {
        ++liCount;
    }

    return liCount;
}


char const* clsHTTPMessage::GetFieldName(size_t aiIndex) const
{
    stHeaderField* lpoField = _GetHeaderField(&coFieldList, aiIndex);

    if (NULL == lpoField)
    {
        return NULL;
    }

    return lpoField->cpcName;
}


char const* clsHTTPMessage::GetFieldValue(size_t aiIndex) const
{
    stHeaderField* lpoField = _GetHeaderField(&coFieldList, aiIndex);

    AI_RETURN_IF(NULL, (NULL == lpoField));

    return lpoField->cpcValue;
}


char const* clsHTTPMessage::GetFieldValue(char const* apcName) const
{
    stHeaderField* lpoField = _SearchHeaderField(&coFieldList, apcName);

    AI_RETURN_IF(NULL, (NULL == lpoField));

    return lpoField->cpcValue;
}


ssize_t clsHTTPMessage::BuildHeaderFieldString(char* apcBuf, size_t aiBufSize) const
{
    char* lpcCur = apcBuf;
    char* lpcEnd = lpcCur + aiBufSize;
    ssize_t liRet;

    AI_FOR_EACH_IN_LIST(lpoField, &coFieldList, stHeaderField, coFieldNode)
    {
        liRet = snprintf(lpcCur, lpcEnd - lpcCur, "%s:%s\r\n", lpoField->cpcName, lpoField->cpcValue);

        AI_RETURN_IF(AI_HTTP_ERROR_TOOBIG, (liRet < 0));
        
        AI_RETURN_IF(AI_HTTP_ERROR_TOOBIG, (liRet > (lpcEnd - lpcCur - 1)));
       
        lpcCur += liRet; 
    }

    AI_RETURN_IF(AI_HTTP_ERROR_TOOBIG, ((lpcEnd - lpcCur) < 3));

    memcpy(lpcCur, "\r\n\0", 3);

    lpcCur += 2;

    return lpcCur - apcBuf;
}


ssize_t clsHTTPMessage::ParseHeaderFieldString(char const* apcBuf)
{
    char const* lpcBeginOfLine = apcBuf;
    char const* lpcEndOfLine;
    char const* lpcSep;
    char lsName[AI_HTTP_MAX_LINE_LENGTH + 1];
    char lsValue[AI_HTTP_MAX_LINE_LENGTH + 1]; 
    char lsTmp[AI_HTTP_MAX_LINE_LENGTH + 1];

    memset(lsName, 0, sizeof(lsName));
    memset(lsValue, 0, sizeof(lsValue));
    memset(lsTmp, 0, sizeof(lsTmp));

    while (true)
    {
        lpcEndOfLine = strstr(lpcBeginOfLine, "\r\n");

        AI_RETURN_IF(AI_HTTP_ERROR_EOL, (NULL == lpcEndOfLine));

        if (lpcEndOfLine == lpcBeginOfLine)
        {
            lpcBeginOfLine = lpcEndOfLine + strlen("\r\n");
            break; // reach end of buffer, break and return; 
        }

        assert(lpcEndOfLine > lpcBeginOfLine);

        AI_RETURN_IF(AI_HTTP_ERROR_TOOBIG, (size_t(lpcEndOfLine - lpcBeginOfLine) > AI_HTTP_MAX_LINE_LENGTH));

        if (isspace(lpcBeginOfLine[0]))
        { // LWS
            AI_RETURN_IF(AI_HTTP_ERROR_FIELDNAME, (strlen(lsName) == 0));

            memset(lsTmp, 0, sizeof(lsTmp));
            memcpy(lsTmp, lpcBeginOfLine, lpcEndOfLine - lpcBeginOfLine);
            StringTrim(lsTmp);
           
            AI_RETURN_IF(AI_HTTP_ERROR_FIELDVALUE, (strlen(lsTmp) == 0));

            memset(lsValue, 0, sizeof(lsValue));
            StringCopy(lsValue, GetFieldValue(lsName), sizeof(lsValue));
           
            size_t liValueLen = strlen(lsValue);
            size_t liAddLen = strlen(lsTmp);

            AI_RETURN_IF(AI_HTTP_ERROR_FIELDVALUE, (liValueLen + 1 + liAddLen + 1 > AI_HTTP_MAX_LINE_LENGTH));
           
            snprintf(lsValue + liValueLen, sizeof(lsValue) - liValueLen, " %s", lsTmp); 

            DelField(lsName);
            AddField(lsName, lsValue);
        }
        else
        {
            lpcSep = (char const*)memchr(lpcBeginOfLine, ':', lpcEndOfLine - lpcBeginOfLine);

            AI_RETURN_IF(AI_HTTP_ERROR_FIELDSEP, (NULL == lpcSep));
           
            memset(lsName, 0, sizeof(lsName));
            memcpy(lsName, lpcBeginOfLine, lpcSep - lpcBeginOfLine);
            StringTrim(lsName);
             
            memset(lsValue, 0, sizeof(lsValue));
            memcpy(lsValue, lpcSep + 1, lpcEndOfLine - (lpcSep + 1));
            StringTrim(lsValue);
           
            AI_RETURN_IF(AI_HTTP_ERROR_FIELDNAME, (strlen(lsName) == 0));
        
            AddField(lsName, lsValue); 
        }

        lpcBeginOfLine = lpcEndOfLine + strlen("\r\n");
    }

    return lpcBeginOfLine - apcBuf;
}


int clsHTTPMessage::AddEntity(void const* apBuf, size_t aiBufSize)
{
    AI_RETURN_IF(AI_HTTP_ERROR_INVAL, (NULL == apBuf));

    _MallocHTTPEntity(&coEntityList, apBuf, aiBufSize);
    
    return 0;
}


int clsHTTPMessage::SetEntity(size_t aiIndex, void const* apBuf, size_t aiBufSize)
{
    stHTTPEntity* lpoEntity = _GetHTTPEntity(&coEntityList, aiIndex);

    AI_RETURN_IF(AI_HTTP_ERROR_NOTFOUND, (NULL == lpoEntity));

    _ReallocHTTPEntity(lpoEntity, apBuf, aiBufSize);

    return 0;
}


int clsHTTPMessage::DelEntity(size_t aiIndex)
{
    return _FreeHTTPEntity(
        _GetHTTPEntity(&coEntityList, aiIndex)
    );
}


char const* clsHTTPMessage::GetEntityPtr(size_t aiIndex) const
{
    stHTTPEntity* lpoEntity = _GetHTTPEntity(&coEntityList, aiIndex);

    AI_RETURN_IF(NULL, (NULL == lpoEntity));

    return lpoEntity->cpcEntity;
}


ssize_t clsHTTPMessage::GetEntitySize(size_t aiIndex) const
{
    stHTTPEntity* lpoEntity = _GetHTTPEntity(&coEntityList, aiIndex);

    AI_RETURN_IF(AI_HTTP_ERROR_NOTFOUND, (NULL == lpoEntity));

    return lpoEntity->ciLength;
}


size_t clsHTTPMessage::GetEntityCount() const
{
    size_t liCount = 0;

    AI_FOR_EACH_IN_LIST(lpoEntity, &coEntityList, stHTTPEntity, coEntityNode)
    {
        ++liCount;
    }

    return liCount;
}


size_t clsHTTPMessage::CalcContentLength() const
{
    size_t liTotal = 0;

    AI_FOR_EACH_IN_LIST(lpoEntity, &coEntityList, stHTTPEntity, coEntityNode)
    {
        liTotal += lpoEntity->ciLength; 
    }

    return liTotal;
}
////////////////////////////////////////////////////////////////////////////
clsHTTPRequest::clsHTTPRequest()
{
    AI_MALLOC_ASSERT(cpcMethod, char, 1);
    AI_MALLOC_ASSERT(cpcUri, char, 1);
    *cpcMethod = '\0';
    *cpcUri = '\0';
}


clsHTTPRequest::~clsHTTPRequest()
{
    AI_FREE(cpcUri);
    AI_FREE(cpcMethod);
}


char const* clsHTTPRequest::GetMethod() const
{
    return cpcMethod;
}


void clsHTTPRequest::SetMethod(char const* apcMethod)
{
    size_t liLen = strlen(apcMethod) + 1;

    AI_REALLOC_ASSERT(cpcMethod, char, liLen);
    StringCopy(cpcMethod, apcMethod, liLen);
}

char const* clsHTTPRequest::GetUri() const
{
    return cpcUri;
}


void clsHTTPRequest::SetUri(char const* apcUri)
{
    size_t liLen = strlen(apcUri) + 1;
    
    AI_REALLOC_ASSERT(cpcUri, char, liLen);
    StringCopy(cpcUri, apcUri, liLen);
}


ssize_t clsHTTPRequest::BuildStartLineString(char* apcBuf, size_t aiBufSize) const
{
    AI_RETURN_IF(AI_HTTP_ERROR_INVAL, (NULL == apcBuf));

    ssize_t liRet = snprintf(apcBuf, aiBufSize, "%s %s HTTP/%u.%u\r\n", cpcMethod, cpcUri, (ciVersion%100)/10, ciVersion%10);     
    AI_RETURN_IF(AI_HTTP_ERROR_TOOBIG, (liRet < 0 || size_t(liRet + 1) > aiBufSize));

    return liRet;
}


ssize_t clsHTTPRequest::ParseStartLineString(char const* apcBuf)
{
    char const* lpcSep;
    char const* lpcLast = apcBuf;

    lpcSep = strchr(lpcLast, ' ');

    AI_RETURN_IF(AI_HTTP_ERROR_METHOD, (NULL == lpcSep || lpcSep == lpcLast));

    AI_REALLOC_ASSERT(cpcMethod, char, lpcSep - lpcLast + 1);

    memcpy(cpcMethod, lpcLast, lpcSep - lpcLast);
    cpcMethod[lpcSep - lpcLast] = '\0';

    lpcLast = lpcSep + 1;
    lpcSep = strchr(lpcLast, ' ');

    AI_RETURN_IF(AI_HTTP_ERROR_URI, (NULL == lpcSep || lpcSep == lpcLast));

    AI_REALLOC_ASSERT(cpcUri, char, lpcSep - lpcLast + 1);
    memcpy(cpcUri, lpcLast, lpcSep - lpcLast);
    cpcUri[lpcSep - lpcLast] = '\0';

    ++lpcSep; 

    if (memcmp(lpcSep, "HTTP/", 5) != 0
        || !isdigit(lpcSep[5])
        || '.' != lpcSep[6]
        || !isdigit(lpcSep[7])
        || memcmp(lpcSep + 8, "\r\n", 2) != 0)
    {
        return AI_HTTP_ERROR_VERSION;
    }

    ciVersion = strtoul(lpcSep + 5, NULL, 10) * 10 + strtoul(lpcSep + 7, NULL, 10);

    return lpcSep - apcBuf + 10;
}


////////////////////////////////////////////////////////////////////////////
clsHTTPResponse::clsHTTPResponse()
    :ciStatus(200)
{
    AI_MALLOC_ASSERT(cpcReason, char, 1);
    *cpcReason = '\0';
}


clsHTTPResponse::~clsHTTPResponse()
{
    AI_FREE(cpcReason);
}


unsigned clsHTTPResponse::GetStatus() const
{
    return ciStatus;
}


void clsHTTPResponse::SetStatus(unsigned aiStatus)
{
    ciStatus = aiStatus;
}


char const* clsHTTPResponse::GetReason() const
{
    return cpcReason;
}

void clsHTTPResponse::SetReason(char const* apcReason)
{
    size_t liLen = strlen(apcReason) + 1;

    AI_REALLOC_ASSERT(cpcReason, char, liLen);
    StringCopy(cpcReason, apcReason, liLen);
}


ssize_t clsHTTPResponse::BuildStartLineString(char* apcBuf, size_t aiBufSize) const
{
    AI_RETURN_IF(AI_HTTP_ERROR_INVAL, (NULL == apcBuf));

    ssize_t liRet = snprintf(
        apcBuf,
        aiBufSize,
        "HTTP/%u.%u %03u %s\r\n",
        (ciVersion % 100) / 10, 
        ciVersion % 10, 
        ciStatus % 1000, 
        cpcReason
    );

    AI_RETURN_IF(AI_HTTP_ERROR_TOOBIG, (liRet < 0 || size_t(liRet + 1) > aiBufSize));

    return liRet;
}


ssize_t clsHTTPResponse::ParseStartLineString(char const* apcBuf)
{
    AI_RETURN_IF(AI_HTTP_ERROR_STARTLINE, (strlen(apcBuf)  - strlen("HTTP/x.y abc \r\n") < 0));

    if (memcmp(apcBuf, "HTTP/", 5) != 0
        || !isdigit(apcBuf[5])
        || '.' != apcBuf[6]
        || !isdigit(apcBuf[7])
        || ' ' != apcBuf[8])
    {
        return AI_HTTP_ERROR_VERSION;
    }

    ciVersion = strtoul(apcBuf + 5, NULL, 10) * 10 + strtoul(apcBuf + 7, NULL, 10);
    
    if (!isdigit(apcBuf[9])
        || !isdigit(apcBuf[10])
        || !isdigit(apcBuf[11])
        || ' ' != apcBuf[12])
    {
        return AI_HTTP_ERROR_STATUS;
    }

    ciStatus = strtoul(apcBuf + 9, NULL, 10);

    char const* lpcEnd = strstr(apcBuf, "\r\n");

    AI_RETURN_IF(AI_HTTP_ERROR_EOL, (NULL == lpcEnd || lpcEnd < apcBuf + 13));

    ssize_t liReasonSize = lpcEnd - (apcBuf + 13);
    AI_REALLOC_ASSERT(cpcReason, char, liReasonSize + 1);
    memcpy(cpcReason, apcBuf + 13, liReasonSize);
    cpcReason[liReasonSize] = '\0'; 
    
    return lpcEnd - apcBuf + strlen("\r\n"); 
}

////////////////////////////////////////////////////////////////////////////

clsHTTPConnection::clsHTTPConnection(int aiSocket, char const* apcType, char const* apcName)
    :clsTcpSocket(aiSocket, apcType, apcName)
{
}


clsHTTPConnection::~clsHTTPConnection()
{
}


ssize_t clsHTTPConnection::RecvUntilEmptyLine(char* apcBuf, size_t aiBufSize, AITime_t aiTimeout)
{
    size_t liRecvSize = 0;
    ssize_t liRet = 0;

    memset(apcBuf, 0, aiBufSize);

    while (2 != liRet)
    {
        liRet = RecvToken("\r\n", apcBuf + liRecvSize, aiBufSize - liRecvSize, aiTimeout);

        AI_RETURN_IF(AI_HTTP_ERROR_SOCKET, (liRet <= 0));
        assert(liRet >= 2);

        liRecvSize += liRet;
    }

    return liRecvSize;
}


ssize_t clsHTTPConnection::SendHeader(clsHTTPMessage* apoMsg, bool abAutoCalc, AITime_t aiTimeout)
{
    char lsBuf[AI_HTTP_MAX_HEADER_LENGTH + 1];
    size_t liBufSize = 0;
    ssize_t liRet;

    if (abAutoCalc && apoMsg->GetEntityCount() > 0)
    {
        size_t liSize = 0;
        for (size_t i=0; i < apoMsg->GetEntityCount(); ++i)
        {
            liSize += apoMsg->GetEntitySize(i);
        }

        memset(lsBuf, 0, sizeof(lsBuf));
        snprintf(lsBuf, sizeof(lsBuf), "%lu", liSize);
        apoMsg->DelField(AI_HTTP_FIELD_CONTENT_LENGTH);
        liRet = apoMsg->AddField(AI_HTTP_FIELD_CONTENT_LENGTH, lsBuf);
        _CLOSE_RETURN_IF(liRet, (liRet < 0));
    }

    memset(lsBuf, 0, sizeof(lsBuf));
    liRet = apoMsg->BuildStartLineString(lsBuf, AI_HTTP_MAX_HEADER_LENGTH);

    _CLOSE_RETURN_IF(liRet, (liRet < 0));

    liBufSize += liRet;

    liRet = apoMsg->BuildHeaderFieldString(lsBuf + liRet, AI_HTTP_MAX_HEADER_LENGTH - liRet);
    
    _CLOSE_RETURN_IF(liRet, (liRet < 0));

    liBufSize += liRet;

    liRet = SendData(lsBuf, liBufSize, aiTimeout);

    _CLOSE_RETURN_IF(AI_HTTP_ERROR_SOCKET, (liRet < 0));

    return liBufSize;
}


ssize_t clsHTTPConnection::RecvHeader(clsHTTPMessage* apoMsg, AITime_t aiTimeout)
{
    char lsBuf[AI_HTTP_MAX_HEADER_LENGTH + 1];
    size_t liBufSize = 0;
    ssize_t liRet;

    apoMsg->Reset();

    memset(lsBuf, 0, sizeof(lsBuf));
    liRet = RecvUntilEmptyLine(lsBuf, AI_HTTP_MAX_HEADER_LENGTH, aiTimeout);
    
    _CLOSE_RETURN_IF(liRet, (liRet < 0));
    
    liBufSize = liRet;

    liRet = apoMsg->ParseStartLineString(lsBuf);

    _CLOSE_RETURN_IF(liRet, (liRet < 0));

    liRet = apoMsg->ParseHeaderFieldString(lsBuf + liRet);

    _CLOSE_RETURN_IF(liRet, (liRet < 0));

    return liBufSize;
}


ssize_t clsHTTPConnection::SendBody(clsHTTPMessage* apoMsg, AITime_t aiTimeout)
{
    size_t liSendSize = 0;

    for(size_t i=0; i < apoMsg->GetEntityCount(); ++i)
    {
        char const* lpcData = apoMsg->GetEntityPtr(i);
        size_t liSize = apoMsg->GetEntitySize(i);
        ssize_t liRet = SendData(lpcData, liSize, aiTimeout);

        _CLOSE_RETURN_IF(AI_HTTP_ERROR_SOCKET, (liRet < 0));

        liSendSize += liRet;
    }

    return liSendSize;
}


ssize_t clsHTTPConnection::RecvChunks(clsHTTPMessage* apoMsg, AITime_t aiTimeout)
{
    size_t liRecvSize = 0;
    ssize_t liEntitySize;
    ssize_t liRet;
    char lsBuf[AI_HTTP_MAX_HEADER_LENGTH+ 1];
    AIChunkEx loEntity;

    while (true)
    {
        // recv chunk size
        memset(lsBuf, 0, sizeof(lsBuf));
        liRet = RecvToken("\r\n", lsBuf, AI_HTTP_MAX_LINE_LENGTH, aiTimeout);

        _CLOSE_RETURN_IF(AI_HTTP_ERROR_SOCKET, (liRet < 0));
        
        liRecvSize += liRet;

        // handle chunk size
        liEntitySize = strtol(lsBuf, NULL, 16);

        _CLOSE_RETURN_IF(AI_HTTP_ERROR_SIZE, (liEntitySize < 0));

        if (0 == liEntitySize)
        { // last chunk
            // recv trailer
            liRet = RecvUntilEmptyLine(lsBuf, AI_HTTP_MAX_HEADER_LENGTH, aiTimeout);

            _CLOSE_RETURN_IF(liRet, (liRet < 0));

            liRecvSize += liRet;
          
            // handle the header??
            liRet = apoMsg->ParseHeaderFieldString(lsBuf); 

            _CLOSE_RETURN_IF(liRet, (liRet < 0));

            break;
        }

        loEntity.Resize(liEntitySize);
       
        // recv entity 
        liRet = RecvData(loEntity.GetPointer(), liEntitySize, aiTimeout);

        _CLOSE_RETURN_IF(AI_HTTP_ERROR_SOCKET, (liRet < 0));

        liRecvSize += liEntitySize;
        liRet = apoMsg->AddEntity(loEntity.GetPointer(), liEntitySize);

        _CLOSE_RETURN_IF(liRet, (liRet < 0));

        // recv end of entity
        liRet = RecvToken("\r\n", lsBuf, AI_HTTP_MAX_LINE_LENGTH, aiTimeout);

        _CLOSE_RETURN_IF(AI_HTTP_ERROR_SOCKET, (liRet < 0));
        assert(liRet >= 2);

        liRecvSize += liRet;

    }

    return liRecvSize;
}


ssize_t clsHTTPConnection::RecvBody(clsHTTPMessage* apoMsg, AITime_t aiTimeout)
{
    size_t liRecvSize = 0;
    ssize_t liRet;
    char const* lpcValue;
    ssize_t liEntitySize;
    AIChunkEx loEntity;

    lpcValue = apoMsg->GetFieldValue(AI_HTTP_FIELD_TRANSFER_ENCODING);
    if (NULL != lpcValue)
    { // chunk encoding
        _CLOSE_RETURN_IF(AI_HTTP_ERROR_FIELDVALUE, (strcasecmp(lpcValue, "chunked") != 0));
       
        liRet = RecvChunks(apoMsg, aiTimeout);
         
        _CLOSE_RETURN_IF(liRet, (liRet < 0));

        liRecvSize += liRet;
        return liRecvSize; // read chunks ok
    }

    lpcValue = apoMsg->GetFieldValue(AI_HTTP_FIELD_CONTENT_LENGTH);
    if (NULL != lpcValue)
    {
        liEntitySize = strtol(lpcValue, NULL, 10);
        
        _CLOSE_RETURN_IF(AI_HTTP_ERROR_SIZE, (liEntitySize < 0));
       
        loEntity.Resize(liEntitySize);
        liRet = RecvData(loEntity.GetPointer(), liEntitySize, aiTimeout);

        _CLOSE_RETURN_IF(AI_HTTP_ERROR_SOCKET, (liRet < 0));

        liRet = apoMsg->AddEntity(loEntity.GetPointer(), liEntitySize);

        _CLOSE_RETURN_IF(liRet, (liRet < 0));
    
        liRecvSize += liEntitySize;

        return liRecvSize;
    }

    return 0;
}


int clsHTTPConnection::HandleClientRequest(clsHTTPRequest* apoReq, clsHTTPResponse* apoResp, size_t aiMaxRedir, AITime_t aiTimeout)
{
    ssize_t liRet;

    liRet = SendHeader(apoReq, true, aiTimeout);

    AI_RETURN_IF(liRet, (liRet < 0));

    liRet = SendBody(apoReq, aiTimeout);

    AI_RETURN_IF(liRet, (liRet < 0));

    // handle response-header
    do 
    {
        apoResp->Reset();

        liRet = RecvHeader(apoResp, aiTimeout);

        AI_RETURN_IF(liRet, (liRet < 0));

    }while(apoResp->GetStatus() % 100 == 1); // ignore 100-continue

    // handle response-body
    if (strcasecmp(apoReq->GetMethod(), AI_HTTP_METHOD_HEAD) != 0
        && apoResp->GetStatus() != AI_HTTP_STATUS_NO_CONTENT
        && apoResp->GetStatus() != AI_HTTP_STATUS_NOT_MODIFIED
        && apoResp->GetStatus() != AI_HTTP_STATUS_RESET_CONTENT)
    {
        liRet = RecvBody(apoResp, aiTimeout);

        AI_RETURN_IF(liRet, (liRet < 0));
    }

    // handle redirect
    if (aiMaxRedir)
    { 
        if (strcasecmp(apoReq->GetMethod(), AI_HTTP_METHOD_HEAD) == 0
            || strcasecmp(apoReq->GetMethod(), AI_HTTP_METHOD_GET) == 0)
        {
            if (apoResp->GetStatus() == AI_HTTP_STATUS_MOVED_PERMANENTLY
                || apoResp->GetStatus() == AI_HTTP_STATUS_FOUND
                || apoResp->GetStatus() == AI_HTTP_STATUS_TEMPORARY_REDIRECT)
            { 
                char const* lpcLocation = apoResp->GetFieldValue(AI_HTTP_FIELD_LOCATION);
                
                AI_RETURN_IF(AI_HTTP_ERROR_LOCATION, (NULL == lpcLocation));
                
                apoReq->DelField(AI_HTTP_FIELD_REFFER);
                apoReq->AddField(AI_HTTP_FIELD_REFFER, apoReq->GetUri()); 
                apoReq->SetUri(lpcLocation);

                return HandleClientRequest(apoReq, apoResp, aiMaxRedir - 1, aiTimeout);
            }
        }

        if (apoResp->GetStatus() == AI_HTTP_STATUS_SEE_OTHER)
        { 
            char const* lpcLocation = apoResp->GetFieldValue(AI_HTTP_FIELD_LOCATION);
            
            AI_RETURN_IF(AI_HTTP_ERROR_LOCATION, (NULL == lpcLocation));
            
            apoReq->DelField(AI_HTTP_FIELD_REFFER);
            apoReq->AddField(AI_HTTP_FIELD_REFFER, apoReq->GetUri()); 
            apoReq->SetUri(lpcLocation);
            apoReq->SetMethod(AI_HTTP_METHOD_GET); // must change to GET

            return HandleClientRequest(apoReq, apoResp, aiMaxRedir - 1, aiTimeout);
        }

    } 
    else
    {
        return AI_HTTP_ERROR_REDIRECT;
    }

    return 0;
}

///end namespace
AIBC_NAMESPACE_END
