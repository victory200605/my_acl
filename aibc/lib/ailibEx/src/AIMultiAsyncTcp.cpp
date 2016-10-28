#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "AISocket.h"
#include "AIAsyncTcp.h"
#include "AIMultiAsyncTcp.h"

///start namespace
AIBC_NAMESPACE_START

clsMultiAsyncClient::clsMultiAsyncClient(int aiTimeout)
{
    srand(time(NULL));
    
    ai_init_list_head(&coClientHead);
    ai_init_list_head(&coShutDownHead);

    ciTimeout = (aiTimeout<3?3:aiTimeout);
}

clsMultiAsyncClient::~clsMultiAsyncClient()
{
    clsMultiAsyncClient::ShutDown();
}

void clsMultiAsyncClient::ListConnection(void)
{
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    AsyncSvrNode    *lpoClientNode = NULL;
    clsAsyncClient  *lpoAsyncClient = NULL;
    
    lpoHead = &coClientHead;
    coClientMutexLock.Lock();
    
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoClientNode = AI_GET_STRUCT_PTR(lpoTemp, AsyncSvrNode, coClientChild);
        
        if((lpoAsyncClient = lpoClientNode->cpoAsyncClient))
        { 
#ifdef  DEBUG
            fprintf(stderr, "%20s: Server = %s:%d\r\n", __FUNCTION__, \
                lpoClientNode->csServerIp, lpoClientNode->ciServerPort);
#endif          
            lpoAsyncClient->ListConnection();
        }
    }
    
    coClientMutexLock.Unlock();
}

int clsMultiAsyncClient::GetConnCount(void)
{
    int             liCount = 0;
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    AsyncSvrNode    *lpoClientNode = NULL;
    clsAsyncClient  *lpoAsyncClient = NULL;
    
    lpoHead = &coClientHead;
    
    coClientMutexLock.Lock();

    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoClientNode = AI_GET_STRUCT_PTR(lpoTemp, AsyncSvrNode, coClientChild);
        
        if((lpoAsyncClient = lpoClientNode->cpoAsyncClient))
        {
            liCount += lpoAsyncClient->GetConnCount();
        }
    }

    coClientMutexLock.Unlock();
    return  (liCount);
}

void clsMultiAsyncClient::ShutDown(void)
{
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    AsyncSvrNode    *lpoClientNode = NULL;
    clsAsyncClient  *lpoAsyncClient = NULL;
    
    lpoHead = &coClientHead;
    
    coClientMutexLock.Lock();
    
    while(!ai_list_is_empty(lpoHead))
    {
        ai_list_del_head(lpoTemp, lpoHead);
        ai_list_add_tail(lpoTemp, &coShutDownHead);
    
        lpoClientNode = AI_GET_STRUCT_PTR(lpoTemp, AsyncSvrNode, coClientChild);

        if((lpoAsyncClient = lpoClientNode->cpoAsyncClient))
        {
            lpoAsyncClient->ShutDown();
        }
    }
    
    coClientMutexLock.Unlock();

AGAIN:
    lpoHead = &coShutDownHead;
    
    coClientMutexLock.Lock();
    
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoClientNode = AI_GET_STRUCT_PTR(lpoTemp, AsyncSvrNode, coClientChild);
            
        if(lpoClientNode->ciExUsed == 0)
        {       
            ai_list_del_any(&(lpoClientNode->coClientChild));
            coClientMutexLock.Unlock();
            
            AI_DELETE(lpoClientNode->cpoAsyncClient);
            AI_DELETE(lpoClientNode);
            
            goto  AGAIN;
        }
    }

    if(!ai_list_is_empty(lpoHead))
    {
        coClientMutexLock.Unlock();
        sleep(1);
        
        goto  AGAIN;
    }

    coClientMutexLock.Unlock();
}

AsyncSvrNode *clsMultiAsyncClient::SearchClient(int aiGroupID, char const* apcIpAddr, int aiServerPort)
{
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    AsyncSvrNode    *lpoClientNode = NULL;
    
    lpoHead = &coClientHead;
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoClientNode = AI_GET_STRUCT_PTR(lpoTemp, AsyncSvrNode, coClientChild);
        
        if(strcmp(lpoClientNode->csServerIp, apcIpAddr) == 0 && \
            lpoClientNode->ciServerPort == aiServerPort && \
            lpoClientNode->ciGroupID == aiGroupID)
        {
            return  (lpoClientNode);
        }
    }
    
    return  (NULL);
}

int clsMultiAsyncClient::AddServer(char const* apcIpAddr, int aiServerPort, int aiWeight, int aiMaxConn, int aiGroupID)
{
    AsyncSvrNode    *lpoClientNode = NULL;
    clsAsyncClient  *lpoAsyncClient = NULL;
    
    if(apcIpAddr == NULL || aiServerPort <= 0 || aiMaxConn <= 0)
        return  (ASYNC_ERROR_INVPARAM);

    coClientMutexLock.Lock();
    if((lpoClientNode = SearchClient(aiGroupID, apcIpAddr, aiServerPort)))
    {
#ifdef  DEBUG
        fprintf(stderr, "%20s: Duplicate ServerInfo(%s:%d / %d)\r\n", __FUNCTION__, \
            apcIpAddr, aiServerPort, aiGroupID);
#endif
        coClientMutexLock.Unlock();
        return  (ASYNC_ERROR_DUPSERVER);
    }

    AI_NEW_ASSERT(lpoClientNode, AsyncSvrNode);
    memset(lpoClientNode, 0, sizeof(AsyncSvrNode));   

    lpoClientNode->ciWeight = aiWeight;
    lpoClientNode->ciGroupID = aiGroupID;
    lpoClientNode->ciMaxConn = aiMaxConn;
    lpoClientNode->ciTimeout = ciTimeout;
    lpoClientNode->ciServerPort = aiServerPort;
    strcpy(lpoClientNode->csServerIp, apcIpAddr); 
    
    if(lpoClientNode->ciWeight <= 0)
    {
        lpoClientNode->ciWeight = ASYNC_DEFAULT_WEIGHT;
    }
    
    AI_NEW_ASSERT(lpoAsyncClient, clsAsyncClient(apcIpAddr, aiServerPort, aiMaxConn, ciTimeout));
    if(lpoAsyncClient->StartDaemon(3) <= 0)
    {
#ifdef  DEBUG
        fprintf(stderr, "%20s: StartDaemon(%s:%d / %d) ... Fail\r\n", __FUNCTION__, \
            apcIpAddr, aiServerPort, aiGroupID);
#endif      

        AI_DELETE(lpoAsyncClient);
        AI_DELETE(lpoClientNode);
        
        coClientMutexLock.Unlock();
        return  (ASYNC_ERROR_STARTUP);
    }
    
    lpoClientNode->cpoAsyncClient = lpoAsyncClient;
    ai_list_add_tail(&(lpoClientNode->coClientChild), &coClientHead);
    
    coClientMutexLock.Unlock();
    return  (0);
}

int clsMultiAsyncClient::CloseServer(char const* apcIpAddr, int aiServerPort, int aiGroupID)
{
    AsyncSvrNode    *lpoClientNode = NULL;
    clsAsyncClient  *lpoAsyncClient = NULL;
    
    if(apcIpAddr == NULL || aiServerPort <= 0)
        return  (ASYNC_ERROR_INVPARAM);

    coClientMutexLock.Lock();
    if((lpoClientNode = SearchClient(aiGroupID, apcIpAddr, aiServerPort)))
    {
        ai_list_del_any(&(lpoClientNode->coClientChild));
        ai_list_add_tail(&(lpoClientNode->coClientChild), &coShutDownHead);
        
        if((lpoAsyncClient = lpoClientNode->cpoAsyncClient))
        {
            lpoAsyncClient->ShutDown();
        }

        while(lpoClientNode->ciExUsed)
        {       
            coClientMutexLock.Unlock();

            sleep(1);

            coClientMutexLock.Lock();
        }

        ai_list_del_any(&(lpoClientNode->coClientChild));
        AI_DELETE(lpoAsyncClient);
        AI_DELETE(lpoClientNode);

        coClientMutexLock.Unlock();
        return  (0);
    }

    coClientMutexLock.Unlock();

    return  (ASYNC_ERROR_NOSERVER);
}

int clsMultiAsyncClient::ChangeServer(char const* apcIpAddr, int aiServerPort, int aiWeight, int aiGroupID)
{
    AsyncSvrNode    *lpoClientNode = NULL;
    
    if(apcIpAddr == NULL || aiServerPort <= 0)
        return  (ASYNC_ERROR_INVPARAM);
        
    coClientMutexLock.Lock();
    if((lpoClientNode = SearchClient(aiGroupID, apcIpAddr, aiServerPort)))
    {
        lpoClientNode->ciCurCount = 0;
        lpoClientNode->ciWeight = aiWeight;

        if(lpoClientNode->ciWeight <= 0)
        {
            lpoClientNode->ciWeight = ASYNC_DEFAULT_WEIGHT;
        }
        
        coClientMutexLock.Unlock();
        return  (0);
    }

#ifdef  DEBUG
    fprintf(stderr, "%20s: SearchClient(%s:%d / %d) ... Fail\r\n", __FUNCTION__, \
        apcIpAddr, aiServerPort, aiGroupID);
#endif

    coClientMutexLock.Unlock();
    return  (ASYNC_ERROR_NOSERVER);
}

clsAsyncClient *clsMultiAsyncClient::GetClient(int aiGroupID)
{
    int             liIndex = 0;
    int             liGrpCount = 0;
    int             liArrCount = 0;
    LISTHEAD        *lpoHead = NULL;
    LISTHEAD        *lpoTemp = NULL;
    AsyncSvrNode    *lpoClientArray[128];
    AsyncSvrNode    *lpoClientNode = NULL;
    clsAsyncClient  *lpoAsyncClient = NULL;
        
    lpoHead = &coClientHead;

    coClientMutexLock.Lock();
    
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoClientNode = AI_GET_STRUCT_PTR(lpoTemp, AsyncSvrNode, coClientChild);
        
        if(lpoClientNode->ciGroupID == aiGroupID)
        {
            liGrpCount += 1;
            
            if((lpoAsyncClient = lpoClientNode->cpoAsyncClient))
            {
                if(lpoAsyncClient->GetConnCount() > 0)
                {
                    if(lpoClientNode->ciCurCount < lpoClientNode->ciWeight)
                    {
                        lpoClientArray[liArrCount++] = lpoClientNode;
                    }
                }
            }
        }
    }   
    
    if(liGrpCount <= 0)
    {
        coClientMutexLock.Unlock();
        return  (NULL);
    }
    
    if(liArrCount <= 0)
    {
        for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
        {
            lpoClientNode = AI_GET_STRUCT_PTR(lpoTemp, AsyncSvrNode, coClientChild);
            lpoClientNode->ciCurCount = 0;
        }
        
        for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
        {
            lpoClientNode = AI_GET_STRUCT_PTR(lpoTemp, AsyncSvrNode, coClientChild);
                
            if(lpoClientNode->ciGroupID == aiGroupID)
            {
                if((lpoAsyncClient = lpoClientNode->cpoAsyncClient))
                {
                    if(lpoAsyncClient->GetConnCount() > 0)
                    {
                        if(lpoClientNode->ciCurCount < lpoClientNode->ciWeight)
                        {
                            lpoClientArray[liArrCount++] = lpoClientNode;
                        }
                    }
                }
            }
        }   

        if(liArrCount <= 0)
        {
            coClientMutexLock.Unlock();
            return  (NULL);
        }
    }
        
    liIndex = rand() % liArrCount;
    lpoClientNode = lpoClientArray[ liIndex ];
    
    lpoClientNode->ciCurCount += 1;
    lpoClientNode->ciExUsed += 1;

    coClientMutexLock.Unlock();
    return  (lpoClientNode->cpoAsyncClient);
}

int clsMultiAsyncClient::FreeClient(const clsAsyncClient *apoAsyncClient)
{
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    AsyncSvrNode    *lpoClientNode = NULL;
    
    lpoHead = &coClientHead;
    coClientMutexLock.Lock();

    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoClientNode = AI_GET_STRUCT_PTR(lpoTemp, AsyncSvrNode, coClientChild);
        if(lpoClientNode->cpoAsyncClient == apoAsyncClient)
        {
            lpoClientNode->ciExUsed -= 1;
            coClientMutexLock.Unlock();
            return  (0);
        }
    }

    lpoHead = &coShutDownHead;
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoClientNode = AI_GET_STRUCT_PTR(lpoTemp, AsyncSvrNode, coClientChild);
        if(lpoClientNode->cpoAsyncClient == apoAsyncClient)
        {
            lpoClientNode->ciExUsed -= 1;
            coClientMutexLock.Unlock();
            return  (0);
        }
    }

    coClientMutexLock.Unlock();
    return  (ASYNC_ERROR_NOSERVER);
}

int clsMultiAsyncClient::SendResult(char const* apcInData, int aiInSize, char* apcData, int aiMaxSize, int& aiSize, int aiGroupID)
{
    int                 liRetCode = 0;
    clsAsyncClient      *lpoAsyncClient = NULL;
    char                lsDesc[128] = {0};
    
    if((lpoAsyncClient = GetClient(aiGroupID)) == NULL)
        return  (ASYNC_ERROR_NOSERVER);

    snprintf(lsDesc, sizeof(lsDesc), "Remote=%s:%d", 
        lpoAsyncClient->GetServerIp(), lpoAsyncClient->GetServerPort());
	
    if((liRetCode = lpoAsyncClient->SendRequest(apcInData, aiInSize, lsDesc)))
    {
        FreeClient(lpoAsyncClient);   
        return  (liRetCode);
    }
        
    liRetCode = lpoAsyncClient->RecvResponse(apcData, aiMaxSize, aiSize);
    FreeClient(lpoAsyncClient);   

    return  (liRetCode);
}

int clsMultiAsyncClient::SendResult(char const* apcInData, int aiInSize, char* apcData, int aiMaxSize, int& aiSize, char* apcDestIP, int& aiDestPort, int aiGroupID)
{
    int                 liRetCode = 0;
    clsAsyncClient      *lpoAsyncClient = NULL;
    char                lsDesc[128] = {0};
    
    if((lpoAsyncClient = GetClient(aiGroupID)) == NULL)
        return  (ASYNC_ERROR_NOSERVER);

    snprintf(lsDesc, sizeof(lsDesc), "Remote=%s:%d", 
        lpoAsyncClient->GetServerIp(), lpoAsyncClient->GetServerPort());

    //return dest ip and dest port info
    strcpy(apcDestIP, lpoAsyncClient->GetServerIp());
    aiDestPort = lpoAsyncClient->GetServerPort();
	
    if((liRetCode = lpoAsyncClient->SendRequest(apcInData, aiInSize, lsDesc)))
    {
        FreeClient(lpoAsyncClient);   
        return  (liRetCode);
    }
        
    liRetCode = lpoAsyncClient->RecvResponse(apcData, aiMaxSize, aiSize);
    FreeClient(lpoAsyncClient);   

    return  (liRetCode);
}


///end namespace
AIBC_NAMESPACE_END
