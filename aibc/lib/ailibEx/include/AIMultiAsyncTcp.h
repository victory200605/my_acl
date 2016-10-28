#ifndef __AILIBEX__AIMULTIASYNCTCP_H__
#define __AILIBEX__AIMULTIASYNCTCP_H__

#include "AIDualLink.h"
#include "AIAsyncTcp.h"
#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////
//
#define  ASYNC_ERROR_INVPARAM    (-90050)
#define  ASYNC_ERROR_DUPSERVER   (-90051)
#define  ASYNC_ERROR_STARTUP     (-90052)
#define  ASYNC_ERROR_NOSERVER    (-90053)

////////////////////////////////////////////////////////////////////
//
#define  ASYNC_DEFAULT_WEIGHT    (20)

////////////////////////////////////////////////////////////////////
//
typedef struct stAsyncSvrNode
{
    int             ciWeight;
    int             ciExUsed;
    int             ciGroupID;
    int             ciMaxConn;
    int             ciTimeout;
    int             ciCurCount;
    int             ciServerPort;
    char            csServerIp[32];
    
    LISTHEAD        coClientChild;
    clsAsyncClient  *cpoAsyncClient;
} AsyncSvrNode;

////////////////////////////////////////////////////////////////////
//
class clsMultiAsyncClient
{
private:
    int             ciTimeout;
    LISTHEAD        coClientHead;
    LISTHEAD        coShutDownHead;
    AIMutexLock     coClientMutexLock;
        
public:
    clsMultiAsyncClient(int aiTimeout);
    ~clsMultiAsyncClient();

private:
    AsyncSvrNode *SearchClient(int aiGroupID, char const* apcIpAddr, int aiServerPort);
    
public:
    int AddServer(char const* apcIpAddr, int aiServerPort, int aiWeight, int aiMaxConn, int aiGroupID = 0);
    int ChangeServer(char const* apcIpAddr, int aiServerPort, int aiWeight, int aiGroupID = 0);
    int CloseServer(char const* apcIpAddr, int aiServerPort, int aiGroupID = 0);

public:
    clsAsyncClient *GetClient(int aiGroupID = 0);
    int FreeClient(clsAsyncClient const* apoAsyncClient);
    int SendResult(char const* apcInData, int aiInSize, char* apcData, int aiMaxSize, int &aiSize, int aiGroupID = 0);

	//return dest ip and dest port info
	int SendResult(char const* apcInData, int aiInSize, char* apcData, int aiMaxSize, int &aiSize, char* apcDestIP, int &aiDestPort, int aiGroupID = 0);
                
public:     
    void ShutDown(void);
    int GetConnCount(void);
    void ListConnection(void);
};

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AIMULTIASYNCTCP_H__

