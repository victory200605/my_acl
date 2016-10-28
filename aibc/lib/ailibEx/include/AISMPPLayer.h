#ifndef __AILIBEX__AISMPPLAYER_H__
#define __AILIBEX__AISMPPLAYER_H__

#include "AIDualLink.h"
#include "AISMPPStru.h"
#include "AISynch.h"

///start namespace
AIBC_NAMESPACE_START

//////////////////////////////////////////////////////////////////////
//
#define   SCT_FLUSH_INIT        (0)
#define   SCT_FLUSH_EXIST       (1)
#define   SCT_FLUSH_NEWADD      (2)
#define   SCT_SOCK_TIMEOUT      (5)
#define   SCT_MAX_ACC_COUNT     (128)
//#define   SCT_MAX_HASH_SIZE     (97)
#define   SCT_DEFAULT_WEIGHT    (32)
#define   SCT_MASK_UDHIFLAG     (0x40)
#define   SCT_MASK_STATREPORT   (0x04)

//////////////////////////////////////////////////////////////////////
//
#define   SCT_ERROR_RECVFAIL        (-90300)
#define   SCT_ERROR_SENDFAIL        (-90301)
#define   SCT_ERROR_TYPESTATUS      (-90310)
#define   SCT_ERROR_DUPCONNINFO     (-90311)
#define   SCT_ERROR_INV_CONNNUM     (-90320)
#define   SCT_ERROR_INV_SCALLBAK    (-90321)
#define   SCT_ERROR_INV_RCALLBAK    (-90322)
#define   SCT_ERROR_INV_DCALLBAK    (-90323)

//////////////////////////////////////////////////////////////////////
#define     AI_SMPP_ROUTE_AUTO      (-1)
#define     AI_SMPP_ROUTE_FAILURE   (-2)


//////////////////////////////////////////////////////////////////////
//
typedef struct stSmppAccount
{
    int                 ciGroupNo;
    int                 ciAccountId;
    char                csSystemID[24];
    char                csRemoteIp[20];
    int                 ciRemotePort;
    int                 ciBindType;
} SmppAccount;

typedef struct stSmppRouteInfo
{
    char                csSrcTermID[21+1];
    char                csServiceID[10+1];
    char                csDestTermID[21+1];
    int                 ciBindType;
} SmppRouteInfo;

typedef struct StatREPORT
{
    char                ciFakeFlag;
    char                csMsgID[65+1];
    char                csSub[3+1];
    char                csDlvrd[3+1];
    char                csSubmitTime[10+1];
    char                csDoneTime[10+1];
    char                csStat[7+1];
    char                csErr[3+1];
} StatREPORT;

//////////////////////////////////////////////////////////////////////
// sizeof(csIDPrefix) <= 32 Bytes
typedef struct stSmppConfig
{
    int                 ciGroupNo;
    int                 ciAccountId;
    int                 ciWeight;
    int                 ciIDFormat;
    int                 ciBindType;
    size_t              ciConnCount;
    char                csPassword[24];
    char                csIDPrefix[32];
    int                 ciRemotePort;
    char                csRemoteIp[20];
    char                csSystemID[24];
    int                 ciVendorType;
} SmppConfig;

typedef struct stSmppClientCfg
{
    stSmppClientCfg()
        :ciGroupNo(-1),
        ciAccountId(-1),
        ciWeight(0), 
        ciIDFormat(0), 
        ciBindType(0), 
        ciConnCount(0), 
        ciRemotePort(0), 
        ciVendorType(0),
        ciShutDown(0),
        ciFlushFlag(0),
        ciSendCount(0)
    {
        memset(csPassword, 0, sizeof(csPassword));
        memset(csIDPrefix, 0, sizeof(csIDPrefix));
        memset(csRemoteIp, 0, sizeof(csRemoteIp));
        memset(csSystemID, 0, sizeof(csSystemID));
        ai_init_list_head(&coSocketHead);
        memset(&coClientChild, 0, sizeof(coClientChild));
    }

    LISTHEAD            coClientChild;
    int                 ciGroupNo;
    int                 ciAccountId;
    int                 ciWeight;
    int                 ciIDFormat;
    int                 ciBindType;
    size_t              ciConnCount;
    char                csPassword[24];
    char                csIDPrefix[32];
    int                 ciRemotePort;
    char                csRemoteIp[20];
    char                csSystemID[24];
    int                 ciVendorType;
        
    int                 ciShutDown;
    int                 ciFlushFlag;
    int                 ciSendCount;
    LISTHEAD            coSocketHead;
    //LISTHEAD            coIndexChild;
    AIMutexLock         coSocketHeadLock;
} SmppClientInfo;

typedef struct stSmppSocketInfo
{
    stSmppSocketInfo()
        :ciExUsed(0),
        ciSocket(-1),
        ciLocalPort(0),
        ctActiveTime(0),
        ciEnquireLinkCount(0),
        cpoSmppClass(NULL),
        ctRecvThreadID(0),
        cpoClientInfo(NULL),
        ciSeqNum(0)
    {
        memset(csLocalIp, 0, sizeof(csLocalIp));
        memset(&coSocketChild, 0, sizeof(coSocketChild));
    }

    int GetNextSeq()    
    {        
        AISmartLock loSmartLock(coSeqMutexLock);        
        if (ciSeqNum <= 0)        
        {            
            ciSeqNum = 1;        
        }        

        return ciSeqNum++;    
    }

    LISTHEAD            coSocketChild;
    int                 ciExUsed;
    int                 ciSocket;
    int                 ciLocalPort;
    char                csLocalIp[20];
    time_t              ctActiveTime;
    size_t              ciEnquireLinkCount;

    void*               cpoSmppClass;
    pthread_t           ctRecvThreadID;
    SmppClientInfo*     cpoClientInfo;
    AIMutexLock         coWriteMutexLock;
    AIMutexLock         coSeqMutexLock;    
    int                 ciSeqNum;
} SmppSocketInfo;

//////////////////////////////////////////////////////////////////////
//
typedef int (*smppConnBreakCallback)(SmppSocketInfo*);
typedef int (*smppConnLoginCallback)(SmppSocketInfo*);
typedef int (*smppDeliverCallback)(SmppSocketInfo*, SMPP_DELIVER_SM*);
typedef int (*smppSubmitRespCallback)(SmppSocketInfo*, SMPP_SUBMIT_SM_RESP*);
typedef int (*smppStatReportCallback)(SmppSocketInfo*, SMPP_DELIVER_SM*, StatREPORT*);
typedef int (*smppGetRouteCallback)(SmppRouteInfo const*, /*SmppAccount*, size_t,*/ void*);

//////////////////////////////////////////////////////////////////////
//
class clsSmppClient
{
public:
    clsSmppClient(int aiEnquireTime, int aiReConnectTime);

private:
    smppGetRouteCallback cpfGetRoute;
    smppConnBreakCallback cpfConnBreakCallback;
    smppConnLoginCallback cpfConnLoginCallback;
    smppDeliverCallback cpfDeliverCallback;
    smppSubmitRespCallback cpfSubmitRespCallback;
    smppStatReportCallback cpfStatReportCallback;
    
private:
    int             ciReConnTime;
    int             ciEnquireTime;
    AIMutexLock     coMutexLock;

private:
    LISTHEAD        coClientHead;
    //LISTHEAD        coIndexHead[SCT_MAX_HASH_SIZE];

private:
    int CheckStatReport(char* apcData, size_t aiSize);    
    int GetUdhiFlag(char* apcData, size_t aiSize);
    int _AddConnection(SmppConfig const* apoClientInfo);
    int GetConnectionCount(SmppClientInfo* apoClientInfo);

private:
    void CheckThread(void);
    void CheckThread(SmppClientInfo* apoClientInfo);
    void CreateThread(SmppClientInfo* apoClientInfo);
    void ShutDown(SmppClientInfo* apoClientInfo);
    int AjustConnections(SmppClientInfo* apoClientInfo, size_t aiNewConnNum); 

private:
    SmppClientInfo* SearchConnection(char const* apcIp, int aiPort, char const* apcSystemID, int aiBindType = AI_SMPP_CMD_BIND_TRANSMITTER);
    SmppClientInfo* SearchConnection(int aiGroupNo, int aiAccoutId, int aiBindType = AI_SMPP_CMD_BIND_TRANSMITTER);

private:        
    int GetValueOfStatReport(char const* apcData, char const* apcLabel, char* apcRetValue, int aiMaxSize);
    int ExtractNormalStatReport(SmppSocketInfo const* apoSocketInfo, char const* apcContent, StatREPORT* apoStat);
    int ExtractEspecialStatReport(SmppSocketInfo const* apoSocketInfo, char const* apcContent, StatREPORT* apoStat);
    int FormatStatReport(SmppSocketInfo const* apoSocketInfo, char const* apcContent, StatREPORT* apoStat);
        
private:
    int DealBIND(int aiSocket, SmppClientInfo* apoClientInfo);
    int DealDELIVER(SmppSocketInfo* apoSocketInfo, SMPP_HEADER* apoHead, void const* apcBuffer, size_t aiLen);
    int DealSubmitRESP(SmppSocketInfo* apoSocketInfo, SMPP_HEADER* apoHead, void const* apcBuffer, size_t aiLen);
    int DealStatREPORT(SmppSocketInfo* apoSocketInfo, SMPP_HEADER* apoHead, void const* apcBuffer, size_t aiLen);
        
public:     
    void ShutDown(void);
    void StartDaemon(void);
    void ListConnection(char* apcInformation);
    int AddConnection(SmppConfig const* apoClientInfo);
    int RefreshConnection(SmppConfig* apoClientInfo, size_t aiArraySize);

public:
    void FreeConnection(SmppSocketInfo const* apoSocketInfo);
    //int GetConnectionCount(SmppRouteInfo const* apoRouteInfo, bool abIsSplit, void* apExtraArg);
    SmppSocketInfo* GetConnection(SmppRouteInfo const* apoRouteInfo, bool abIsSplit, void* apExtraArg);
        
public:
    void SetRouteCallback(smppGetRouteCallback apfCallback);
    void SetDeliverCallback(smppDeliverCallback apfCallback);
    void SetSubmitRespCallback(smppSubmitRespCallback apfCallback);
    void SetStatReportCallback(smppStatReportCallback apfCallback);
    void SetConnectionBreak(smppConnBreakCallback apfCallback);
    void SetConnectionLogin(smppConnLoginCallback apfCallback);

public:
    friend void *smpp_c_recv_thread(void* avOpt);
};

///end namespace
AIBC_NAMESPACE_END

#endif   // __AILIBEX__AISMPPLAYER_H__
