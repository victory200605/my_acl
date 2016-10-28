#include <inttypes.h>

#include "AILib.h"
#include "AISynch.h"
#include "AIString.h"
#include "AIDualLink.h"
#include "AIRegExpr.h"
#include "AISocket.h"
#include "AILogSys.h"

#include "AIPacketTrace.h"

///start namespace
AIBC_NAMESPACE_START

#define _PF_MAX_CONDS       (32)

#define _PF_MAX_TYPE_LENGTH (32)
#define _PF_MAX_FIELD_LENGTH (32)
#define _PF_MAX_DATA_LENGTH (1024)

#define _StringCopy(dest, src)  StringCopy(dest, src, sizeof(dest))

#define AI_PT_DEF_TIMEOUT  (10)
#define AI_PT_MAX_PENDING   (1000)

////////////////////////////////////////////////////////////////////////////
struct _FilterData
{
    LISTHEAD    coListNode;

    char        csType[_PF_MAX_TYPE_LENGTH]; 
    
    size_t      ciSize; 
    char        csData[_PF_MAX_DATA_LENGTH];
};


struct _FilterCond
{
    LISTHEAD    coListNode;

    char        csField[_PF_MAX_FIELD_LENGTH];
    char        csExpr[256];
    AIRegExpr   coRE;
};


struct _FilterExpr
{
    LISTHEAD    coCondList; 
};


static AIMutexLock  soLock;
static LISTHEAD     soPendingList = {&soPendingList, &soPendingList};
static size_t       siListSize = 0;

static AIRWLock     soRWLock;
static _FilterExpr* spoFE    = NULL;


////////////////////////////////////////////////////////////////////////////

static inline _FilterData* _NewData(char const* apcType, void const* apData, size_t aiLen)
{
    _FilterData*    lpoData;

    AI_NEW_ASSERT(lpoData, _FilterData);

    _StringCopy(lpoData->csType, apcType);
    memcpy(lpoData->csData, apData, AI_MIN(aiLen, sizeof(lpoData->csData)));
    lpoData->ciSize = aiLen;

    return lpoData;
}


////////////////////////////////////////////////////////////////////////////

static inline _FilterCond* _NewCond(char const* apcField, char const* apcCond)
{
    _FilterCond*    lpoCond;

    AI_NEW_ASSERT(lpoCond, _FilterCond);

    _StringCopy(lpoCond->csField, apcField);
    _StringCopy(lpoCond->csExpr, apcCond);
    StringTrim(lpoCond->csExpr);
    if (lpoCond->coRE.Compile(lpoCond->csExpr) < 0)
    {
        AI_DELETE(lpoCond);
    } 

    return lpoCond;
}


////////////////////////////////////////////////////////////////////////////

static inline void _UpdateExpr(char const* apcExpr)
{
    char*           lppcStrs[_PF_MAX_CONDS];
    int             liRetCode;
    _FilterExpr*    lpoFE = NULL;
    
    // create new one 
    if (apcExpr || strlen(apcExpr) == 0)
    {
        AI_NEW_ASSERT(lpoFE, _FilterExpr);
        ai_init_list_head(&lpoFE->coCondList); 
          
        liRetCode = ExtractStringColumns(apcExpr, ',', "", lppcStrs, AI_ARRAY_SIZE(lppcStrs));
        
        for (int i = 0; i < liRetCode; ++i)
        {
            _FilterCond*    lpoCond;
            char*           lpcSep = strchr(lppcStrs[i], '=');

            if (NULL == lpcSep)
            {
                continue;
            }

            *lpcSep = '\0';

            lpoCond = _NewCond(lppcStrs[i], lpcSep + 1);
            
            if (NULL != lpoCond)
            {
                ai_list_add_tail(&lpoCond->coListNode, &lpoFE->coCondList);
            }
        }

        ReleaseStringColumns(lppcStrs, liRetCode);   
    }

    // delete old one
    if (spoFE)
    {
        while (!ai_list_is_empty(&spoFE->coCondList))
        {
            _FilterCond* lpoCond    = AI_GET_STRUCT_PTR(spoFE->coCondList.cpNext, _FilterCond, coListNode);
            ai_list_del_any(&lpoCond->coListNode);
            AI_DELETE(lpoCond);
        }

        AI_DELETE(spoFE);
    }

    // update pointer
    spoFE = lpoFE;
}


////////////////////////////////////////////////////////////////////////////
static inline void _PurgePending()
{
    while (!ai_list_is_empty(&soPendingList))
    {
        _FilterData* lpoData    = AI_GET_STRUCT_PTR(soPendingList.cpNext, _FilterData, coListNode);
        ai_list_del_any(&lpoData->coListNode);
        AI_DELETE(lpoData);
    }

    siListSize = 0;
}

////////////////////////////////////////////////////////////////////////////
void AIPacketFilterUpdate(char const* apcExpr)
{
    AISmartWLock    loSmartLock(soRWLock);

    _UpdateExpr(apcExpr);
    
}


////////////////////////////////////////////////////////////////////////////

void _AIPacketFilterAcquireRLock()
{
    soRWLock.WLock();
}


////////////////////////////////////////////////////////////////////////////

void _AIPacketFilterReleaseRLock()
{
    soRWLock.Unlock();
}


////////////////////////////////////////////////////////////////////////////

bool _AIPacketFilterTestFieldS(char const* apcField, char const* apcValue)
{
    //AIWriteLOG(NULL, AILOG_LEVEL_DEBUG, "[%s] %s=%s", __FUNCTION__, apcField, apcValue);

    AI_FOR_EACH_IN_LIST(lpoCond, &spoFE->coCondList, _FilterCond, coListNode)
    {
        if (strcmp(lpoCond->csField, apcField) == 0
            && lpoCond->coRE.IsReady())
        {
            //AIWriteLOG(NULL, AILOG_LEVEL_DEBUG, "[%s] test with [%s]", __FUNCTION__, lpoCond->csExpr);
            if (lpoCond->coRE.Match(apcValue) == 0)
            {
                //AIWriteLOG(NULL, AILOG_LEVEL_DEBUG, "[%s] match!", __FUNCTION__);
                return true;
            }
        }
    }

    //AIWriteLOG(NULL, AILOG_LEVEL_DEBUG, "[%s] not match!", __FUNCTION__);
    return false;
}


////////////////////////////////////////////////////////////////////////////

bool _AIPacketFilterTestFieldI(char const* apcField, intmax_t aiValue)
{
    char lsBuf[64] = { '\0' };

    snprintf(lsBuf, sizeof(lsBuf), "%"PRIdMAX"\n", aiValue);

    return _AIPacketFilterTestFieldS(apcField, lsBuf);
}


////////////////////////////////////////////////////////////////////////////

void _AIPacketFilterAddPacket(char const* apcType, void const* apData, size_t aiLen)
{
    AISmartLock loSmartLock(soLock);

    if (siListSize >= AI_PT_MAX_PENDING)
    {
        return;
    }

    _FilterData*    lpoData = _NewData(apcType, apData, aiLen);

    ai_list_add_tail(&lpoData->coListNode, &soPendingList);
    ++siListSize;
    //AIWriteLOG(NULL, AILOG_LEVEL_DEBUG, "[%s] add [%p] %u", __FUNCTION__, lpoData->csData, lpoData->ciSize);
}

////////////////////////////////////////////////////////////////////////////
int AIPacketFilterFlushPending(int aiSock)
{
    _FilterData*    lpoData = NULL;
    int             liCount = 0;
    int             liRetCode = 0;

    while (true)
    {
        char lsBuf[64];
        
        {
            AISmartLock loSmartLock(soLock);

            if (ai_list_is_empty(&soPendingList))
            {
                break;
            }

            lpoData = AI_GET_STRUCT_PTR(soPendingList.cpNext, _FilterData, coListNode); 
            assert(lpoData);  

            ai_list_del_any(&lpoData->coListNode);
            --siListSize;
        }

        liRetCode = snprintf(lsBuf, sizeof(lsBuf), "TRACE %lu %s\r\n", lpoData->ciSize, lpoData->csType);
        assert(liRetCode > 0);

        //AIWriteLOG(NULL, AILOG_LEVEL_DEBUG, "[%s] header %s!", __FUNCTION__, lsBuf);
        liRetCode = ai_socket_senddata(aiSock, lsBuf, liRetCode, AI_PT_DEF_TIMEOUT);
        if (liRetCode < 0)
        {
            //AIWriteLOG(NULL, AILOG_LEVEL_DEBUG, "[%s] senddata error %d, %s!", __FUNCTION__, liRetCode, strerror(errno));
            break;
        }

        //AIWriteLOG(NULL, AILOG_LEVEL_DEBUG, "[%s] data [%p] %u!", __FUNCTION__, lpoData->csData, lpoData->ciSize);
        liRetCode = ai_socket_senddata(aiSock, lpoData->csData, lpoData->ciSize, AI_PT_DEF_TIMEOUT);
        if (liRetCode < 0)
        {
            //AIWriteLOG(NULL, AILOG_LEVEL_DEBUG, "[%s] senddata error2 %d, %s!", __FUNCTION__, liRetCode, strerror(errno));
            break;
        }

        AI_DELETE(lpoData);
        ++liCount;
    }

    AI_DELETE(lpoData);
    return  liRetCode < 0 ? -1 : liCount;
}

AISmartDataPtr AIPacketFilterGetData(void)
{
    _FilterData*   lpoData = NULL;
    
    AISmartLock loSmartLock(soLock);

    if (ai_list_is_empty(&soPendingList))
    {
        AISmartDataPtr loRetData(NULL);
        return loRetData;
    }

    lpoData = AI_GET_STRUCT_PTR(soPendingList.cpNext, _FilterData, coListNode); 
    assert(lpoData);
    AISmartDataPtr loRetData(lpoData); 

    ai_list_del_any(&lpoData->coListNode);
      --siListSize;
        
    return loRetData;
}

AISmartDataPtr::AISmartDataPtr( _FilterData* apoData ) : cpoData(apoData)
{
}

AISmartDataPtr::AISmartDataPtr( AISmartDataPtr& aoRhs ) : cpoData(aoRhs.cpoData)
{
    aoRhs.cpoData = NULL;
}

AISmartDataPtr::~AISmartDataPtr()
{
    AI_DELETE(cpoData);
}
    
bool AISmartDataPtr::operator == ( void* apvRhs )
{ 
    return cpoData == apvRhs;
}

const char* AISmartDataPtr::GetData()
{
    return cpoData->csData;
}

const char* AISmartDataPtr::GetType()
{
    return cpoData->csType;
}

size_t AISmartDataPtr::GetSize()
{
    return cpoData->ciSize;
}

AISmartDataPtr::operator _FilterData* ()
{
    _FilterData* lpoTmp = cpoData;
    cpoData = NULL;
    return lpoTmp;
}

///end namespace
AIBC_NAMESPACE_END
