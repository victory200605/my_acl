#ifndef  __AILIBEX__AIASYNCTCP_H__
#define  __AILIBEX__AIASYNCTCP_H__

#include "AILib.h"
#include "AISynch.h"
#include "AIDualLink.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////
//
#define  ASYNC_ERROR_TIMEOUT     (-90000)
#define  ASYNC_ERROR_SHUTDOWN    (-90001)
#define  ASYNC_ERROR_LOSTNODE    (-90002)
#define  ASYNC_ERROR_INVLENGTH   (-90003)
#define  ASYNC_ERROR_INV_MSGID   (-90004)
#define  ASYNC_ERROR_NO_CONNCT   (-90005)
#define  ASYNC_ERROR_LOCALBUSY   (-90010)
#define  ASYNC_ERROR_QUEUEFULL   (-90010)
#define  ASYNC_ERROR_SENDFAIL    (-90020)
#define  ASYNC_ERROR_NEWTHREAD   (-90021)
#define  ASYNC_ERROR_LISTENFAIL  (-90022)
#define  ASYNC_ERROR_NO_SOCKET   (-90023)

////////////////////////////////////////////////////////////////////
//
#define  ASYNC_C_INTERVAL        (2)
#define  ASYNC_NET_TIMEOUT       (5)
#define  ASYNC_INVALID_TIME      (10)
#define  ASYNC_MAX_CONN_NUM      (64)
#define  ASYNC_MAX_HASH_SIZE     (97)
#define  ASYNC_RECV_BUFF_SIZE    (128*1024)
#define  ASYNC_MSGID_MAX         (1000000000)

////////////////////////////////////////////////////////////////////
//
#define IS_VALID_ASYNCHEAD( head ) \
    ( head.ciMsgID >= 0  \
     && head.ciMsgID < ASYNC_MSGID_MAX \
     && head.ciLength >= 0 \
     && head.ciLength <= ASYNC_RECV_BUFF_SIZE )
    
////////////////////////////////////////////////////////////////////
//
typedef struct stASYNCHEAD
{
    int     ciMsgID;
    int     ciResult;
    int     ciLength;
} ASYNCHEAD;

typedef struct stASYNCMATCH
{
    int         ciMsgID;
    pthread_t   ctThreadID;
} ASYNCMATCH;

typedef struct stASYNCSOCKETCTL
{
    stASYNCSOCKETCTL();
    
    int         ciSocket;
    int         ciLocalPort;
    char        csLocalIp[32];
    int         ciRemotePort;
    char        csRemoteIp[32];
    
    int         ciExUsed;
    void*       cpoAsyncObj;
    time_t      ciActiveTime;
    LISTHEAD    coSocketChild;  
    pthread_t   ctRecvThreadID;
    AIMutexLock ctMutexLock;
} ASYNCSOCK;

typedef struct stMatchNode
{
    stMatchNode();
    
    int         ciMsgID;
    int         ciDataSize;
    int*        cpiResult;
    char*       cpcExData;
    time_t      ciEnterTime;
    pthread_t   ctPendThreadID;
    char*       cpcDescription;
    
    LISTHEAD    coTimeChild;
    LISTHEAD    coMsgIDChild;
    LISTHEAD    coThreadIDChild;
    
    AICondition ctCondLock;
    AIMutexLock ctMutexLock;
} MatchNode;

////////////////////////////////////////////////////////////////////
//
class clsAsyncClient
{
private:
    int                     ciMaxConn;
    time_t                  ciTimeout;
    int                     ciShutDown;
    int                     ciServerPort;
    char                    csServerIp[32];

private:
    AIMutexLock             coMatchHeadLock;
    AIMutexLock             coSocketHeadLock;

private:
    LISTHEAD                coFreeHead;
    LISTHEAD                coTimeHead;
    LISTHEAD                coSocketHead;
    LISTHEAD                coMsgIDHead[ASYNC_MAX_HASH_SIZE];
    LISTHEAD                coThreadIDHead[ASYNC_MAX_HASH_SIZE];
    
private:
    LISTHEAD                coAsyncChld;
    static int              siNextMsgID;
    static LISTHEAD         soAsyncHead;
    static AIMutexLock      soAsyncHeadLock;
    
public:
    clsAsyncClient(char const* apcIpAddr, int aiServerPort, int aiMaxConn, time_t aiTimeout);
    ~clsAsyncClient();

public:
    void ShutDown(void);
    int StartDaemon(time_t aiTimeout, int aiTryFlag = 0);
    void ListConnection(void);
    int SendRequest(char const* apcData, int aiSize, char* apcDesc);
    int RecvResponse(void);
    int RecvResponse(char* apcData, int& aiSize);
    int RecvResponse(char* apcData, int aiMaxSize, int &aiSize);
    int GetConnCount(void);
    char* GetServerIp() { return csServerIp; }
    int GetServerPort() { return ciServerPort; }
        
private:
    void CheckThread(void);
    void FreeMatchNode(MatchNode* apoMatchNode);

private:
    int GetNextMsgID(void);
    int WakeupPendMsg(int aiMsgID, int aiResult, char* apcData, int aiSize);
    int CheckTimeoutMsg(time_t aiTimeout);
    int CancelPendMsg(void);

private:    
    MatchNode* FindMatchNode(void);
    MatchNode* GetMatchNode(void);
    ASYNCSOCK* GetValidConn(void);

public:
    friend void* async_c_recv_thread(void *avOpt);    
};

////////////////////////////////////////////////////////////////////
//
typedef int (*asyncCallback)(ASYNCSOCK const*, char*, int);
typedef void (*asyncCallbackEx)(ASYNCMATCH const*, char*, int);
typedef int (*gpfGetListenSocket)( char const* );
typedef int (*gpfGetConnection)( char const*, char*, int *);

////////////////////////////////////////////////////////////////////
//
class clsAsyncServer
{
private:
    int                     ciShutDown;
    int                     ciServerPort;
    char                    csServerIp[32];
    int                     ciListenSocket;
    asyncCallback           cpfCallback;
    asyncCallbackEx         cpfCallbackEx;
    pthread_t               ciAcceptThreadID;

private:
    LISTHEAD                coSocketHead;
    AIMutexLock             coSocketHeadLock;
    
private:
    LISTHEAD                coAsyncChld;
    static LISTHEAD         soAsyncHead;
    static AIMutexLock      soAsyncHeadLock;

private:
    char                    csListenName[64];
    gpfGetConnection        cpfGetConnection;
    
public:
    clsAsyncServer(char const* apcIpAddr, int aiServerPort);
    virtual ~clsAsyncServer();

public:
    void ShutDown(void);
    void ListConnection(void);
    void SetCallback(asyncCallback apfCallBack);
    void SetCallbackEx(asyncCallbackEx apfCallBack);
    int StartDaemon(char const*apcListenName = NULL, gpfGetConnection apfFunc = NULL, gpfGetListenSocket apfListen = NULL);

public:
    int SendResult(ASYNCMATCH const* apoMatch, int aiResult);       
    int SendResult(ASYNCMATCH const* apoMatch, char const* apcData, int aiSize, int aiResult);

public:
    virtual void Callback(ASYNCMATCH const* apoMatch, char* apcData, int aiSize) {;}
        
private:
    void ClearInvalid(void);
    ASYNCSOCK* FindSocket(ASYNCMATCH const* apoMatch);

public:
    friend void* async_accept_thread(void* avOpt);
    friend void* async_s_recv_thread(void* avOpt);    
    friend void* async_accept_thread_ex(void* avOpt);
};

///end namespace
AIBC_NAMESPACE_END

#endif   // __AILIBEX__AIASYNCTCP_H__

