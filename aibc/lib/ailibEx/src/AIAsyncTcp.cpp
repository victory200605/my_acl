#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include "AIString.h"
#include "AILogSys.h"
#include "AISocket.h"
#include "AITime.h"
#include "AIAsyncTcp.h"

///start namespace
AIBC_NAMESPACE_START

#define _StringCopy(dst, src) StringCopy(dst, src, sizeof(dst))
#define ASYNC_IDLE_INTERVAL         (3)  
#define ASYNC_INACTIVITY_INTERVAL   (15)
#define ASYNC_HEART_BEAT_MSG_ID     (-1)

AITime_t const AI_ASYNCTCP_WAIT_DELTA = AI_TIME_MSEC * 100;

////////////////////////////////////////////////////////////////////
//
int clsAsyncClient::siNextMsgID = 0;
LISTHEAD clsAsyncClient::soAsyncHead = { &(clsAsyncClient::soAsyncHead), &(clsAsyncClient::soAsyncHead) };
LISTHEAD clsAsyncServer::soAsyncHead = { &(clsAsyncServer::soAsyncHead), &(clsAsyncServer::soAsyncHead) };
AIMutexLock clsAsyncClient::soAsyncHeadLock;
AIMutexLock clsAsyncServer::soAsyncHeadLock;

////////////////////////////////////////////////////////////////////
//
stASYNCSOCKETCTL::stASYNCSOCKETCTL() :
    ciSocket(-1),
    ciLocalPort(0),
    ciRemotePort(0),
    ciExUsed(0),
    cpoAsyncObj(NULL),
    ciActiveTime(0),
    ctRecvThreadID(0)
{
    memset( csLocalIp,      0, sizeof(csLocalIp) );
    memset( csRemoteIp,     0, sizeof(csRemoteIp) );
    memset( &coSocketChild, 0, sizeof(coSocketChild) );
}

stMatchNode::stMatchNode() :
    ciMsgID(0),
    ciDataSize(0),
    cpiResult(NULL),
    cpcExData(NULL),
    ciEnterTime(0),
    ctPendThreadID(0),
    cpcDescription(NULL)
{
    memset( &coTimeChild,     0, sizeof(coTimeChild) );
    memset( &coMsgIDChild,    0, sizeof(coMsgIDChild) );
    memset( &coThreadIDChild, 0, sizeof(coThreadIDChild) );
}

////////////////////////////////////////////////////////////////////
//
void *async_c_recv_thread(void *avOpt)
{
    int                 liSize = 0;
    int                 liRecvSize;
    int                 liLocalPort;
    int                 liSocket = -1;
    int                 liOffset = 0;
    char                lsLocalIp[32];
    char                *lpcData = NULL;
    time_t              liLastCheckTime;
    char                *lpcBuffer = NULL;
    ASYNCSOCK           *lpoSocket = NULL;
    clsAsyncClient      *lpoMainClass = NULL;
    
    lpoSocket = (ASYNCSOCK *)avOpt;
    lpoMainClass = (clsAsyncClient *)lpoSocket->cpoAsyncObj;
    AI_NEW_N_ASSERT(lpcBuffer, char, ASYNC_RECV_BUFF_SIZE);

    lpoSocket->ctRecvThreadID = pthread_self();
    pthread_detach(pthread_self());
    liLastCheckTime = time(NULL);
    
    while(lpoMainClass->ciShutDown == 0)
    {
        if (time(NULL) >= (liLastCheckTime + ASYNC_C_INTERVAL))
        {
            liLastCheckTime = time(NULL);
            lpoMainClass->CheckTimeoutMsg(lpoMainClass->ciTimeout);
        }
        
        if (!IS_VALID_SOCKET(lpoSocket->ciSocket))
        {
            liSocket = ai_tcp_connect(lpoMainClass->csServerIp, lpoMainClass->ciServerPort, ASYNC_NET_TIMEOUT);
            if (!IS_VALID_SOCKET(liSocket))
            {
                AIWriteLOG(
                    AILIB_LOGFILE,
                    AILOG_LEVEL_TRACE,
                    "[%s] ai_tcp_connect(%s:%d) ... Fail = %d",
                    __FUNCTION__, 
                    lpoMainClass->csServerIp,
                    lpoMainClass->ciServerPort, 
                    liSocket
                );
                
                sleep(1);
                continue;
            }
            
            ai_socket_local_addr(liSocket, lsLocalIp, liLocalPort);

            lpoMainClass->coSocketHeadLock.Lock();

            _StringCopy(lpoSocket->csLocalIp, lsLocalIp);
            lpoSocket->ciLocalPort = liLocalPort;
            lpoSocket->ciSocket = liSocket;
            lpoSocket->ciActiveTime = time(NULL);

            lpoMainClass->coSocketHeadLock.Unlock();
        }
		
        if ((time(NULL) - lpoSocket->ciActiveTime) >= ASYNC_INACTIVITY_INTERVAL)
        {
            // close socket
            AISmartLock loSmartLock(lpoMainClass->coSocketHeadLock);
            ai_socket_close( lpoSocket->ciSocket );
            liSize = 0;

            continue;
        } // end of close socket
 
        if (time(NULL) >= (lpoSocket->ciActiveTime + ASYNC_IDLE_INTERVAL)) 
        {
            ASYNCHEAD   loAsyncHead;

            loAsyncHead.ciLength = ntohl(0);
            loAsyncHead.ciResult = ntohl(0);
            loAsyncHead.ciMsgID = ntohl(ASYNC_HEART_BEAT_MSG_ID);

            lpoSocket->ctMutexLock.Lock();
            if(ai_socket_senddata(lpoSocket->ciSocket, (char*)&loAsyncHead, sizeof(loAsyncHead), ASYNC_NET_TIMEOUT)
                    != sizeof(loAsyncHead)) 
            {
                AISmartLock loSmartLock(lpoMainClass->coSocketHeadLock);

                ai_socket_close(lpoSocket->ciSocket);
                liSize = 0;

                lpoSocket->ctMutexLock.Unlock();
                continue;
            }

            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE,
                "[%s] send heart beat (%s:%d)",
                __FUNCTION__,
                lpoMainClass->csServerIp,
                lpoMainClass->ciServerPort
            );


            lpoSocket->ctMutexLock.Unlock();
        }

       
        if((liRecvSize = ai_socket_recvdata(lpoSocket->ciSocket, lpcBuffer+liSize, ASYNC_RECV_BUFF_SIZE-liSize)) <= 0)
        {
            if(liRecvSize < 0)
            {
                AISmartLock loSmartLock(lpoMainClass->coSocketHeadLock);
                ai_socket_close(lpoSocket->ciSocket);
                
                liSize = 0;
            }
            
            continue;
        }
        
        liOffset = 0;
        liSize += liRecvSize;
        
        lpoSocket->ciActiveTime = time(NULL);

        while((liSize-liOffset) >= (int)sizeof(ASYNCHEAD))
        {
            ASYNCHEAD   loAsyncHead;
            
            memcpy(&loAsyncHead, lpcBuffer+liOffset, sizeof(ASYNCHEAD));
            loAsyncHead.ciLength = ntohl(loAsyncHead.ciLength);
            loAsyncHead.ciResult = ntohl(loAsyncHead.ciResult);
            loAsyncHead.ciMsgID = ntohl(loAsyncHead.ciMsgID);

            if(loAsyncHead.ciMsgID == ASYNC_HEART_BEAT_MSG_ID) 
            {
                AIWriteLOG(
                    AILIB_LOGFILE,
                    AILOG_LEVEL_TRACE,
                    "[%s] recv heart beat (%s:%d)",
                    __FUNCTION__,
                    lpoMainClass->csServerIp,
                    lpoMainClass->ciServerPort
                );

                liOffset += sizeof(ASYNCHEAD);
                continue;
            }
            
            // Check ASYNCHEAD validity
            if ( !IS_VALID_ASYNCHEAD(loAsyncHead) )
            {
                AIWriteLOG(
                    AILIB_LOGFILE,
                    AILOG_LEVEL_ERROR,
                    "[%s]:  ASYNCHEAD invalid and close connection to [%s:%d] now ..., MsgID = %d, Result = %d, Length = %d", 
                    __FUNCTION__, 
                    lpoSocket->csRemoteIp,
                    lpoSocket->ciRemotePort,
                    loAsyncHead.ciMsgID, 
                    loAsyncHead.ciResult,
                    loAsyncHead.ciLength
                );
                
                AISmartLock loSmartLock(lpoMainClass->coSocketHeadLock);
                ai_socket_close(lpoSocket->ciSocket);
                
                liSize = 0;
                
                break;
            }// pass
        
            if(loAsyncHead.ciResult == ASYNC_ERROR_QUEUEFULL)
            {
                AISmartLock loSmartLock(lpoMainClass->coSocketHeadLock);
                lpoSocket->ciActiveTime += ASYNC_INVALID_TIME;
            }
            
            if(loAsyncHead.ciLength <= 0)
            {
                lpoMainClass->WakeupPendMsg(loAsyncHead.ciMsgID, loAsyncHead.ciResult, NULL, 0);
                liOffset += sizeof(ASYNCHEAD);
                continue;
            }
            
            liRecvSize = (int)(sizeof(ASYNCHEAD) + loAsyncHead.ciLength);
            if((liSize-liOffset) >= liRecvSize)
            {
                int     liTempSize;
                    
                liTempSize = loAsyncHead.ciLength;
                lpcData = lpcBuffer+(liOffset+sizeof(ASYNCHEAD));
                    
                lpoMainClass->WakeupPendMsg(loAsyncHead.ciMsgID, loAsyncHead.ciResult, lpcData, liTempSize);
                liOffset += liRecvSize;
                continue;
            }
                
            break;
        }
        
        if(liOffset > 0)
        {
            if((liSize = liSize - liOffset) > 0)
            {
                memmove(lpcBuffer, lpcBuffer+liOffset, liSize);
            }
        }
    }
    
    lpoSocket->ctRecvThreadID = 0;
    AI_DELETE_N(lpcBuffer);
    
    return  (NULL);
}

////////////////////////////////////////////////////////////////////
//
clsAsyncClient::clsAsyncClient(char const* apcIpAddr, int aiServerPort, int aiMaxConn, time_t aiTimeout)
{
    ciShutDown = 0;
    ciServerPort = aiServerPort;
    _StringCopy(csServerIp, (apcIpAddr?apcIpAddr:"127.0.0.1"));
    ciTimeout = AIMax(aiTimeout, (time_t)5);

    ciMaxConn = AIMin(AIMax(aiMaxConn, 1), ASYNC_MAX_CONN_NUM);

    ai_init_list_head(&coSocketHead);

    ai_init_list_head(&coFreeHead);
    ai_init_list_head(&coTimeHead);
    for(int liIt = 0; liIt < ASYNC_MAX_HASH_SIZE; liIt++)
    {
        ai_init_list_head(&coMsgIDHead[liIt]);
        ai_init_list_head(&coThreadIDHead[liIt]);
    }

    soAsyncHeadLock.Lock();
    ai_list_add_tail(&coAsyncChld, &soAsyncHead);
    soAsyncHeadLock.Unlock();
}

clsAsyncClient::~clsAsyncClient()
{
    AISmartLock loSmartLock(soAsyncHeadLock);
    ai_list_del_any(&coAsyncChld);
}

////////////////////////////////////////////////////////////////////
//
void clsAsyncClient::ShutDown(void)
{
    int             liIt = 0;
    LISTHEAD        *lpoHead = NULL;

    ciShutDown = 1;
    lpoHead = &coSocketHead;
    
    coSocketHeadLock.Lock();
    AI_FOR_EACH_IN_LIST(lpoSocket, lpoHead, ASYNCSOCK, coSocketChild)
    {
        if (IS_VALID_SOCKET(lpoSocket->ciSocket))  
        {
            ai_socket_close(lpoSocket->ciSocket);
        }
    }
    coSocketHeadLock.Unlock();
    sleep(1);
    
AGAIN1:
    coSocketHeadLock.Lock();
    AI_FOR_EACH_IN_LIST(lpoSocket, lpoHead, ASYNCSOCK, coSocketChild)
    {
        if(lpoSocket->ctRecvThreadID == 0)
        {
            if(lpoSocket->ciExUsed == 0)
            {
                ai_list_del_any(&lpoSocket->coSocketChild);
                coSocketHeadLock.Unlock();
                
                AI_DELETE(lpoSocket);
                goto  AGAIN1;
            }
        }
    }
    
    if(!ai_list_is_empty(lpoHead))
    {
        coSocketHeadLock.Unlock();
        sleep(1);
        
        goto  AGAIN1;
    }
    
    coSocketHeadLock.Unlock();
    clsAsyncClient::CancelPendMsg();

AGAIN2:
    coMatchHeadLock.Lock();
    for(liIt = 0; liIt < ASYNC_MAX_HASH_SIZE; liIt++)
    {
        lpoHead = &coThreadIDHead[liIt];
        if(!ai_list_is_empty(lpoHead))
        {
            coMatchHeadLock.Unlock();
            sleep(1);

            goto  AGAIN2;
        }
    }
    
    lpoHead = &coFreeHead;
    while(!ai_list_is_empty(lpoHead))
    {
        MatchNode* lpoMatchNode = AI_FIRST_OF_LIST(lpoHead, MatchNode, coTimeChild);

        ai_list_del_any(&lpoMatchNode->coTimeChild);
        AI_DELETE(lpoMatchNode);
    }
    coMatchHeadLock.Unlock();
}

int clsAsyncClient::StartDaemon(time_t aiTimeout, int aiTryFlag)
{
    pthread_t       ltThreadID;
    ASYNCSOCK       *lpoSocket = NULL;

    if(aiTryFlag)
    {
        int             liSocket;
        time_t          liTimeout;
        
        liTimeout = AIMax(aiTimeout, (time_t)3);
        liSocket = ai_tcp_connect(csServerIp, ciServerPort, liTimeout);
        if (!IS_VALID_SOCKET(liSocket))
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE,
                "[%s] ai_tcp_connect(%s:%d) ... fail", 
                __FUNCTION__,
                csServerIp, 
                ciServerPort
            );

            return  (liSocket);
        }
        
        ai_socket_close(liSocket);
    }
    
    for(int liIt = 0; liIt < ciMaxConn; liIt++)
    {
        AI_NEW_ASSERT(lpoSocket, ASYNCSOCK);
        
        lpoSocket->ciRemotePort = ciServerPort;
        _StringCopy(lpoSocket->csRemoteIp, csServerIp);

        lpoSocket->cpoAsyncObj = (void *)this;

        if(pthread_create(&ltThreadID, NULL, async_c_recv_thread, lpoSocket))
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE,
                "[%s] pthread_create(async_c_recv_thread) ... Fail", 
                __FUNCTION__
            );
            
            AI_DELETE(lpoSocket);
            CheckThread();

            return  (liIt);
        }

        coSocketHeadLock.Lock();
        ai_list_add_tail(&(lpoSocket->coSocketChild), &coSocketHead);
        coSocketHeadLock.Unlock();
    }

    CheckThread();  
    return  (ciMaxConn);
}

void clsAsyncClient::ListConnection(void)
{
    time_t          liTempVal = 0;
    LISTHEAD        *lpoHead = NULL;

    lpoHead = &coSocketHead;
    AISmartLock loSmartLock(coSocketHeadLock);
    AI_FOR_EACH_IN_LIST(lpoSocket, lpoHead, ASYNCSOCK, coSocketChild)
    {
        liTempVal = AIMax((lpoSocket->ciActiveTime - time(NULL)), (time_t)0);

        AIWriteLOG(
            AILIB_LOGFILE,
            AILOG_LEVEL_TRACE,
            "[%s](%d):  %18s:%d -> %18s:%d  [Wait = %2"PRIdMAX"]", 
            __FUNCTION__, 
            lpoSocket->ciSocket, 
            lpoSocket->csLocalIp, 
            lpoSocket->ciLocalPort, 
            lpoSocket->csRemoteIp, 
            lpoSocket->ciRemotePort, 
            (intmax_t)liTempVal
        );
    }
        
}

////////////////////////////////////////////////////////////////////
//
void clsAsyncClient::CheckThread(void)
{
    LISTHEAD        *lpoHead = NULL;

AGAIN:
    lpoHead = &coSocketHead;
    coSocketHeadLock.Lock();

    AI_FOR_EACH_IN_LIST(lpoSocket, lpoHead, ASYNCSOCK, coSocketChild)
    {
        if(lpoSocket->ctRecvThreadID == 0)
        {
            coSocketHeadLock.Unlock();
            sleep(1);
            
            goto  AGAIN;
        }
    }
    coSocketHeadLock.Unlock();
}

void clsAsyncClient::FreeMatchNode(MatchNode *apoMatchNode)
{
    if(apoMatchNode->cpiResult)
    {
        AI_DELETE(apoMatchNode->cpiResult);
        apoMatchNode->cpiResult = NULL;
    }
    
    if(apoMatchNode->cpcDescription)
    {
        free(apoMatchNode->cpcDescription);
        apoMatchNode->cpcDescription = NULL;
    }
    
    if(apoMatchNode->cpcExData)
    {
        AI_DELETE_N(apoMatchNode->cpcExData);
        apoMatchNode->cpcExData = NULL;
        apoMatchNode->ciDataSize = 0;
    }

    ai_list_add_tail(&(apoMatchNode->coTimeChild), &coFreeHead);
}

MatchNode *clsAsyncClient::FindMatchNode(void)
{
    int             liIndex;
    pthread_t       ltThreadID;
    LISTHEAD        *lpoHead = NULL;
    
    ltThreadID = pthread_self();
    liIndex = ((intptr_t)ltThreadID) % ASYNC_MAX_HASH_SIZE;
    lpoHead = &(coThreadIDHead[ liIndex ]);

    AISmartLock loSmartLock(coMatchHeadLock);
    AI_FOR_EACH_IN_LIST(lpoMatchNode, lpoHead, MatchNode, coThreadIDChild)
    {
        if(lpoMatchNode->ctPendThreadID == ltThreadID)
        {
            return  (lpoMatchNode);
        }
    }
    
    return  (NULL);
}

MatchNode *clsAsyncClient::GetMatchNode(void)
{
    int             liNextMsgID;
    time_t          liTimeStamp;
    LISTHEAD        *lpoHead = NULL;
    LISTHEAD        *lpoHead1 = NULL;
    LISTHEAD        *lpoHead2 = NULL;
    pthread_t       ltCurrThreadID = 0;
    MatchNode       *lpoMatchNode = NULL;

    liNextMsgID = GetNextMsgID();
    lpoHead1 = &coMsgIDHead[liNextMsgID%ASYNC_MAX_HASH_SIZE];

    liTimeStamp = time(NULL);
    ltCurrThreadID = pthread_self();
    lpoHead2 = &coThreadIDHead[((intptr_t)ltCurrThreadID)%ASYNC_MAX_HASH_SIZE];
    
    lpoHead = &coFreeHead;
    AISmartLock loSmartLock(coMatchHeadLock);
    if (!ai_list_is_empty(lpoHead))
    {
        lpoMatchNode = AI_FIRST_OF_LIST(lpoHead, MatchNode, coTimeChild);

        ai_list_del_any(&lpoMatchNode->coTimeChild);

        lpoMatchNode->ctPendThreadID = ltCurrThreadID;
        lpoMatchNode->ciEnterTime = liTimeStamp;
        lpoMatchNode->ciMsgID = liNextMsgID;

        ai_list_add_tail(&(lpoMatchNode->coMsgIDChild), lpoHead1);
        ai_list_add_tail(&(lpoMatchNode->coTimeChild), &coTimeHead);
        ai_list_add_tail(&(lpoMatchNode->coThreadIDChild), lpoHead2);
        return  (lpoMatchNode);
    }
    
    AI_NEW_ASSERT(lpoMatchNode, MatchNode);
    
    lpoMatchNode->ciMsgID = liNextMsgID;
    lpoMatchNode->ciEnterTime = liTimeStamp;
    lpoMatchNode->ctPendThreadID = ltCurrThreadID;
    
    AIWriteLOG(
        AILIB_LOGFILE,
        AILOG_LEVEL_TRACE,
        "[%s]: Create New Node ptr = 0x%p / %"PRIuMAX, 
        __FUNCTION__,
        lpoMatchNode, 
        (uintmax_t)lpoMatchNode->ctPendThreadID
    );

    ai_list_add_tail(&(lpoMatchNode->coMsgIDChild), lpoHead1);
    ai_list_add_tail(&(lpoMatchNode->coTimeChild), &coTimeHead);
    ai_list_add_tail(&(lpoMatchNode->coThreadIDChild), lpoHead2);
    return  (lpoMatchNode);
}

int clsAsyncClient::GetNextMsgID(void)
{
    int         liRetCode;
    
    AISmartLock loSmartLock(soAsyncHeadLock);
    liRetCode = siNextMsgID;

    siNextMsgID = (siNextMsgID+1)%ASYNC_MSGID_MAX;

    return  (liRetCode);
}

int clsAsyncClient::GetConnCount(void)
{
    int             liCount = 0;
    time_t          liCurrTime;
    LISTHEAD        *lpoHead = NULL;
    
    liCurrTime = time(NULL);
    lpoHead = &coSocketHead;
    
    AISmartLock loSmartLock(coSocketHeadLock);
    AI_FOR_EACH_IN_LIST(lpoSocket, lpoHead, ASYNCSOCK, coSocketChild)
    {
        if (liCurrTime >= lpoSocket->ciActiveTime)
        {
            if (IS_VALID_SOCKET(lpoSocket->ciSocket))
            {
                liCount += 1;
            }
        }
    }
    
    return  (liCount);
}

ASYNCSOCK *clsAsyncClient::GetValidConn(void)
{
    time_t          liCurrTime;
    LISTHEAD        *lpoHead = NULL;
    
    liCurrTime = time(NULL);
    lpoHead = &coSocketHead;
    
    AISmartLock loSmartLock(coSocketHeadLock);
    AI_FOR_EACH_IN_LIST(lpoSocket, lpoHead, ASYNCSOCK, coSocketChild)
    {
        if(liCurrTime >= lpoSocket->ciActiveTime)
        {
            if (IS_VALID_SOCKET(lpoSocket->ciSocket))
            {
                ai_list_del_any(&lpoSocket->coSocketChild);
                ai_list_add_tail(&(lpoSocket->coSocketChild), lpoHead);
                
                lpoSocket->ciExUsed += 1;
                
                return  (lpoSocket);
            }
        }
    }
    
    return  (NULL);
}

int clsAsyncClient::WakeupPendMsg(int aiMsgID, int aiResult, char* apcData, int aiSize)
{
    assert( aiMsgID >= 0 ); // Promises it here
    
    LISTHEAD        *lpoHead = NULL;
    
    coMatchHeadLock.Lock(); // LOCK!

    lpoHead = &coMsgIDHead[aiMsgID%ASYNC_MAX_HASH_SIZE];
    AI_FOR_EACH_IN_LIST(lpoMatchNode, lpoHead, MatchNode, coMsgIDChild)
    {
        if(lpoMatchNode->ciMsgID == aiMsgID)
        {
            ai_list_del_any(&(lpoMatchNode->coTimeChild));
            ai_list_del_any(&(lpoMatchNode->coMsgIDChild));
            coMatchHeadLock.Unlock(); // UNLOCK!

            AISmartLock loSmartLock(lpoMatchNode->ctMutexLock);
            AI_NEW_ASSERT(lpoMatchNode->cpiResult, int);
            *(lpoMatchNode->cpiResult) = aiResult;
            
            if(apcData && aiSize > 0)
            {
                lpoMatchNode->ciDataSize = aiSize;
                AI_NEW_N_ASSERT(lpoMatchNode->cpcExData, char, aiSize);
                memcpy(lpoMatchNode->cpcExData, apcData, aiSize);
            }

            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_DEBUG,
                "[%s]: Thread = %"PRIuMAX", MsgID = %d, Result = %d, Time = %"PRIdMAX", Size = %d, Desc:%s",
                __FUNCTION__, 
                (uintmax_t)lpoMatchNode->ctPendThreadID, 
                lpoMatchNode->ciMsgID, 
                *(lpoMatchNode->cpiResult), 
                (intmax_t)(time(NULL) - lpoMatchNode->ciEnterTime),
                lpoMatchNode->ciDataSize,
                (lpoMatchNode->cpcDescription!=NULL)?(lpoMatchNode->cpcDescription):"NULL"
            );
            
            lpoMatchNode->ctCondLock.Signal();
            
            return  (0);                
        }
    }
    
    coMatchHeadLock.Unlock(); // UNLOCK!

    return  (ASYNC_ERROR_INV_MSGID);
}

int clsAsyncClient::CheckTimeoutMsg(time_t aiTimeout)
{
    int             liNum = 0;
    time_t          liTimeout;
    time_t          liCurrTime;
    LISTHEAD        *lpoHead = NULL;
    
    lpoHead = &coTimeHead;
    liCurrTime = time(NULL);
    liTimeout = AIMax(aiTimeout, (time_t)5);

AGAIN:  
    coMatchHeadLock.Lock();
    AI_FOR_EACH_IN_LIST(lpoMatchNode, lpoHead, MatchNode, coTimeChild)
    {
        if (liCurrTime > (lpoMatchNode->ciEnterTime + liTimeout))
        {
            ai_list_del_any(&(lpoMatchNode->coTimeChild));
            ai_list_del_any(&(lpoMatchNode->coMsgIDChild));
            coMatchHeadLock.Unlock();

            AISmartLock loSmartLock(lpoMatchNode->ctMutexLock);
            AI_NEW_ASSERT(lpoMatchNode->cpiResult, int);
            *(lpoMatchNode->cpiResult) = ASYNC_ERROR_TIMEOUT;

            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_ERROR,
                "[%s]: Thread = %"PRIuMAX", MsgID = %d, Result = %d(ASYNC_ERROR_TIMEOUT), CurrTime = %d, EnterTime = %d, Time = %"PRIdMAX", Desc:%s", 
                __FUNCTION__, 
                (uintmax_t)lpoMatchNode->ctPendThreadID, 
                lpoMatchNode->ciMsgID,
                *(lpoMatchNode->cpiResult), 
                liCurrTime,
                lpoMatchNode->ciEnterTime,
                (intmax_t)(liCurrTime - lpoMatchNode->ciEnterTime),
                (lpoMatchNode->cpcDescription!=NULL)?(lpoMatchNode->cpcDescription):"NULL"
            );
            
            lpoMatchNode->ctCondLock.Signal();

            liNum = liNum + 1;
            goto  AGAIN;
        }
        
        break;
    }
    
    coMatchHeadLock.Unlock();
    return  (liNum);
}

int clsAsyncClient::CancelPendMsg(void)
{
    int             liNum = 0;
    time_t          liCurrTime;
    LISTHEAD        *lpoHead = NULL;
    
    lpoHead = &coTimeHead;
    liCurrTime = time(NULL);

AGAIN:  
    coMatchHeadLock.Lock();
    AI_FOR_EACH_IN_LIST(lpoMatchNode, lpoHead, MatchNode, coTimeChild)
    {
        ai_list_del_any(&(lpoMatchNode->coTimeChild));
        ai_list_del_any(&(lpoMatchNode->coMsgIDChild));

        coMatchHeadLock.Unlock();

        AISmartLock loSmartLock(lpoMatchNode->ctMutexLock);
        AI_NEW_ASSERT(lpoMatchNode->cpiResult, int);
        *(lpoMatchNode->cpiResult) = ASYNC_ERROR_SHUTDOWN;

        AIWriteLOG(
            AILIB_LOGFILE,
            AILOG_LEVEL_ERROR,
            "[%s]: Thread = %"PRIuMAX", MsgID = %d, Result = %d(ASYNC_ERROR_SHUTDOWN), Time = %"PRIdMAX", Desc:%s",
            __FUNCTION__, 
            (uintmax_t)lpoMatchNode->ctPendThreadID, 
            lpoMatchNode->ciMsgID, 
            *(lpoMatchNode->cpiResult), 
            (intmax_t)(liCurrTime - lpoMatchNode->ciEnterTime),
            (lpoMatchNode->cpcDescription!=NULL)?(lpoMatchNode->cpcDescription):"NULL"
        );
            
        lpoMatchNode->ctCondLock.Signal();

        liNum = liNum + 1;
        goto  AGAIN;
    }
    
    coMatchHeadLock.Unlock();

    return  (liNum);
}

////////////////////////////////////////////////////////////////////
//
int clsAsyncClient::SendRequest(char const* apcData, int aiSize, char* apcDesc)
{
    int             liSize = 0;
    char            *lpcBuf = NULL;
    ASYNCSOCK       *lpoSocket = NULL;
    MatchNode       *lpoMatchNode = NULL;
    ASYNCHEAD       *lpoAsyncHead = NULL;
    
    if( apcData == NULL || aiSize > ASYNC_RECV_BUFF_SIZE || aiSize <= 0 )
    {
        return  (ASYNC_ERROR_INVLENGTH);
    }

    if((lpoSocket = GetValidConn()) == NULL)
    {
        AIWriteLOG(
            AILIB_LOGFILE,
            AILOG_LEVEL_TRACE, 
            "[%s]: GetValidConn() ... Fail", 
            __FUNCTION__
        );

        return  (ASYNC_ERROR_NO_CONNCT);
    }   
    
    lpoMatchNode = GetMatchNode();
    if(apcDesc)
    {
        lpoMatchNode->cpcDescription = strdup(apcDesc);
    }

    AI_NEW_N_ASSERT(lpcBuf, char, aiSize+sizeof(ASYNCHEAD));

    lpoAsyncHead = (ASYNCHEAD *)lpcBuf;
    liSize = aiSize + sizeof(ASYNCHEAD);

    lpoAsyncHead->ciResult = htonl(0);
    lpoAsyncHead->ciLength = htonl(aiSize);
    lpoAsyncHead->ciMsgID = htonl(lpoMatchNode->ciMsgID);
    memcpy(lpcBuf+sizeof(ASYNCHEAD), apcData, aiSize);
    
    lpoSocket->ctMutexLock.Lock();
    if(ai_socket_senddata(lpoSocket->ciSocket, lpcBuf, liSize, ASYNC_NET_TIMEOUT) != liSize)
    {
        lpoSocket->ctMutexLock.Unlock();

        {
            AISmartLock loSmartLock(coSocketHeadLock);

            lpoSocket->ciExUsed -= 1;
            ai_socket_close(lpoSocket->ciSocket);
        }

        AIWriteLOG(
            AILIB_LOGFILE,
            AILOG_LEVEL_TRACE,
            "[%s]: ai_socket_senddata(%s:%d) ... Fail", 
            __FUNCTION__,
            lpoSocket->csRemoteIp, 
            lpoSocket->ciRemotePort
        );
        
        {
            AISmartLock loSmartLock(coMatchHeadLock);
            ai_list_del_any(&(lpoMatchNode->coTimeChild));
            ai_list_del_any(&(lpoMatchNode->coMsgIDChild));
            ai_list_del_any(&(lpoMatchNode->coThreadIDChild));

            FreeMatchNode(lpoMatchNode);
        }

        AI_DELETE_N(lpcBuf);
        return  (ASYNC_ERROR_SENDFAIL);
    }
    
    lpoSocket->ctMutexLock.Unlock();

    coSocketHeadLock.Lock();
        lpoSocket->ciExUsed -= 1;
    coSocketHeadLock.Unlock();

    AI_DELETE_N(lpcBuf);
    return  (0);
}

int clsAsyncClient::RecvResponse(void)
{
    int         liRetCode = 0;
    MatchNode*  lpoMatchNode = NULL;

    if((lpoMatchNode = FindMatchNode()) == NULL)
    {
        AIWriteLOG(
            AILIB_LOGFILE,
            AILOG_LEVEL_TRACE,
            "[%s]:  FindMatchNode(%u) ... Fail", 
            __FUNCTION__, 
            (unsigned int)pthread_self()
        );

        return  (ASYNC_ERROR_LOSTNODE);
    }
    
    lpoMatchNode->ctMutexLock.Lock();
    while(lpoMatchNode->cpiResult == NULL)
    {
        lpoMatchNode->ctCondLock.WaitFor(lpoMatchNode->ctMutexLock, AI_ASYNCTCP_WAIT_DELTA);
    }   
    lpoMatchNode->ctMutexLock.Unlock();
    
    liRetCode = *(lpoMatchNode->cpiResult);

    coMatchHeadLock.Lock();
    ai_list_del_any(&(lpoMatchNode->coThreadIDChild));

    AIWriteLOG(
        AILIB_LOGFILE,
        AILOG_LEVEL_DEBUG, 
        "[%s]: Thread = %"PRIuMAX", MsgID = %d, Result = %d, CurrTime = %d, EnterTime = %d, Time = %d, Size = %"PRIdMAX", Desc:%s", 
        __FUNCTION__, 
        (uintmax_t)lpoMatchNode->ctPendThreadID, 
        lpoMatchNode->ciMsgID, 
        *(lpoMatchNode->cpiResult), 
        time(NULL),
        lpoMatchNode->ciEnterTime,
        (intmax_t)(time(NULL) - lpoMatchNode->ciEnterTime),
        lpoMatchNode->ciDataSize,
        (lpoMatchNode->cpcDescription!=NULL)?(lpoMatchNode->cpcDescription):"NULL"
    );

    FreeMatchNode(lpoMatchNode);
    coMatchHeadLock.Unlock();
    
    return  (liRetCode);
}

int clsAsyncClient::RecvResponse(char *apcData, int &aiSize)
{
    int             liRetCode = 0;
    int             liMaxSize = aiSize;
    MatchNode       *lpoMatchNode = NULL;

    aiSize = 0;
    if((lpoMatchNode = FindMatchNode()) == NULL)
    {
        AIWriteLOG(
            AILIB_LOGFILE,
            AILOG_LEVEL_TRACE,
            "[%s]:  FindMatchNode(%u) ... Fail", 
            __FUNCTION__, 
            (unsigned int)pthread_self()
        );

        return  (ASYNC_ERROR_LOSTNODE);
    }
    
    lpoMatchNode->ctMutexLock.Lock();
    while(lpoMatchNode->cpiResult == NULL)
    {
        lpoMatchNode->ctCondLock.WaitFor(lpoMatchNode->ctMutexLock, AI_ASYNCTCP_WAIT_DELTA);
    }   
    lpoMatchNode->ctMutexLock.Unlock();
    
    liRetCode = *(lpoMatchNode->cpiResult);
    
    if(apcData && liMaxSize > 0)
    {
        if(lpoMatchNode->cpcExData && lpoMatchNode->ciDataSize > 0)
        {
            aiSize = AI_MIN(lpoMatchNode->ciDataSize, liMaxSize);
            memcpy(apcData, lpoMatchNode->cpcExData, aiSize);
        }
    }
    
    AISmartLock loSmartLock(coMatchHeadLock);
    ai_list_del_any(&(lpoMatchNode->coThreadIDChild));

    AIWriteLOG(
        AILIB_LOGFILE,
        AILOG_LEVEL_DEBUG, 
        "[%s]: Thread = %"PRIuMAX", MsgID = %d, Result = %d, CurrTime = %d, EnterTime = %d, Time = %d, Size = %"PRIdMAX", Desc:%s", 
        __FUNCTION__, 
        (uintmax_t)lpoMatchNode->ctPendThreadID, 
        lpoMatchNode->ciMsgID, 
        *(lpoMatchNode->cpiResult), 
        time(NULL),
        lpoMatchNode->ciEnterTime,
        (intmax_t)(time(NULL) - lpoMatchNode->ciEnterTime), 
        lpoMatchNode->ciDataSize,
        (lpoMatchNode->cpcDescription!=NULL)?(lpoMatchNode->cpcDescription):"NULL"
    );

    FreeMatchNode(lpoMatchNode);
    
    return  (liRetCode);
}

int clsAsyncClient::RecvResponse(char* apcData, int aiMaxSize, int &aiSize)
{
    int         liRetCode = 0;
    int         liMaxSize = aiMaxSize;
    
    liRetCode = RecvResponse(apcData, liMaxSize);
    aiSize = liMaxSize;
    
    return  (liRetCode);
}

void *async_s_recv_thread(void *avOpt)
{
    int                 liSize;
    int                 liRetCode;
    char                *lpcBuffer;
    pthread_t           ltRecvThreadID;
    ASYNCSOCK           *lpoSocket = NULL;
    clsAsyncServer      *lpoMainClass = NULL;
    
    lpoSocket = (ASYNCSOCK *)avOpt;
    lpoMainClass = (clsAsyncServer *)lpoSocket->cpoAsyncObj;

    ltRecvThreadID = pthread_self();
    pthread_detach(ltRecvThreadID);
    lpoSocket->ctRecvThreadID = ltRecvThreadID;
    AI_NEW_N_ASSERT(lpcBuffer, char, ASYNC_RECV_BUFF_SIZE);

    while(lpoMainClass->ciShutDown == 0)
    {
        ASYNCHEAD       loAsyncHead;
        ASYNCMATCH      loAsyncMatch;
        
        liSize = sizeof(ASYNCHEAD);
        if((liRetCode = ai_socket_recvdata(lpoSocket->ciSocket, lpcBuffer, liSize, ASYNC_NET_TIMEOUT)) != liSize)
        {
            if(liRetCode < 0 || liRetCode > 0)
            {
                AIWriteLOG(
                    AILIB_LOGFILE,
                    AILOG_LEVEL_TRACE,
                    "[%s]:  ai_socket_recvdata(%s:%d) ... fail = %d", 
                    __FUNCTION__, 
                    lpoSocket->csRemoteIp, 
                    lpoSocket->ciRemotePort,
                    liRetCode
                );

                AISmartLock loSmartLock(lpoMainClass->coSocketHeadLock);
                ai_socket_close(lpoSocket->ciSocket);

                break;
            }
            else if( time(NULL) - lpoSocket->ciActiveTime > 10*ASYNC_IDLE_INTERVAL ) 
            {
                AIWriteLOG(NULL, AILOG_LEVEL_DEBUG, "[AIAsyncTcp] %s:  ai_socket_recvdata(%s:%d) ... timeout = %d",
                    __FUNCTION__,
                    lpoSocket->csRemoteIp,
                    lpoSocket->ciRemotePort,
                    time(NULL) - lpoSocket->ciActiveTime);

                AISmartLock loSmartLock(lpoMainClass->coSocketHeadLock);
                ai_socket_close( lpoSocket->ciSocket );

                break;
            }
            
            continue;
        }

        lpoSocket->ciActiveTime = time(NULL);
        memcpy(&loAsyncHead, lpcBuffer, liSize);

        loAsyncHead.ciMsgID = ntohl(loAsyncHead.ciMsgID);
        loAsyncHead.ciResult = ntohl(loAsyncHead.ciResult);
        loAsyncHead.ciLength = ntohl(loAsyncHead.ciLength);
    
        if (loAsyncHead.ciMsgID == ASYNC_HEART_BEAT_MSG_ID) 
        {
             AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE,
                "[%s] recv heart beat (%s:%d), and send resp",
                __FUNCTION__,
                lpoMainClass->csServerIp,
                lpoMainClass->ciServerPort
            );

            liRetCode = 0;
            goto SEND_RESP;
        }

        // Check ASYNCHEAD validity
        if ( !IS_VALID_ASYNCHEAD(loAsyncHead) )
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_ERROR,
                "[%s]:  ASYNCHEAD invalid and close connection from [%s:%d] now ..., MsgID = %d, Result = %d, Length = %d", 
                __FUNCTION__, 
                lpoSocket->csRemoteIp,
                lpoSocket->ciRemotePort,
                loAsyncHead.ciMsgID, 
                loAsyncHead.ciResult,
                loAsyncHead.ciLength
            );
            
            AISmartLock loSmartLock(lpoMainClass->coSocketHeadLock);
            ai_socket_close(lpoSocket->ciSocket);

            break;
        }// pass
        
        AIWriteLOG(
            AILIB_LOGFILE,
            AILOG_LEVEL_TRACE,
            "[%s]:  MsgID = %d, Result = %d, Length = %d", 
            __FUNCTION__, 
            loAsyncHead.ciMsgID, 
            loAsyncHead.ciResult,
            loAsyncHead.ciLength
        );
        
        if(loAsyncHead.ciLength > 0)
        {
            liSize = loAsyncHead.ciLength;
            if((liRetCode = ai_socket_recvdata(lpoSocket->ciSocket, lpcBuffer, liSize, ASYNC_NET_TIMEOUT)) != liSize)
            {
                AIWriteLOG(
                    AILIB_LOGFILE,
                    AILOG_LEVEL_TRACE,
                    "[%s]:  ai_socket_recvdata(%s:%d) ... fail = %d", 
                    __FUNCTION__, 
                    lpoSocket->csRemoteIp, 
                    lpoSocket->ciRemotePort,
                    liRetCode
                );

                AISmartLock loSmartLock(lpoMainClass->coSocketHeadLock);
                ai_socket_close(lpoSocket->ciSocket);
                
                break;
            }
            
            if(lpoMainClass->cpfCallbackEx)
            {
                loAsyncMatch.ctThreadID = ltRecvThreadID;
                loAsyncMatch.ciMsgID = loAsyncHead.ciMsgID;
                lpoMainClass->cpfCallbackEx(&loAsyncMatch, lpcBuffer, loAsyncHead.ciLength);
                continue;
            }
            
            liRetCode = 0;
            if(lpoMainClass->cpfCallback)
            {
                liRetCode = lpoMainClass->cpfCallback(lpoSocket, lpcBuffer, loAsyncHead.ciLength);
                goto  SEND_RESP;
            }

            loAsyncMatch.ctThreadID = ltRecvThreadID;
            loAsyncMatch.ciMsgID = loAsyncHead.ciMsgID;
            lpoMainClass->Callback(&loAsyncMatch, lpcBuffer, loAsyncHead.ciLength);
            continue;
        }

        liRetCode = ASYNC_ERROR_INVLENGTH;

SEND_RESP:      
        loAsyncHead.ciMsgID = htonl(loAsyncHead.ciMsgID);
        loAsyncHead.ciResult = htonl(liRetCode);
        loAsyncHead.ciLength = htonl(0);
        liSize = sizeof(ASYNCHEAD);
        
        if(ai_socket_senddata(lpoSocket->ciSocket, (char *)&loAsyncHead, liSize, ASYNC_NET_TIMEOUT) != liSize)
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE,
                "[%s]:  ai_socket_senddata(%s:%d) ... fail",
                __FUNCTION__, 
                lpoSocket->csRemoteIp, 
                lpoSocket->ciRemotePort
            );

            AISmartLock loSmartLock(lpoMainClass->coSocketHeadLock);
            ai_socket_close(lpoSocket->ciSocket);
            
            break;
        }
    }
    
    lpoSocket->ctRecvThreadID = 0;
    AI_DELETE_N(lpcBuffer);
    
    return  (NULL);
}

void *async_accept_thread(void *avOpt)
{
    int                 liClientPort;
    int                 liListenSocket;
    int                 liClientSocket;
    char                lsClientIp[32];
    pthread_t           ltNewRecvThread;
    ASYNCSOCK           *lpoSocket = NULL;
    clsAsyncServer      *lpoMainClass = NULL;
    
    pthread_detach(pthread_self());

    lpoMainClass = (clsAsyncServer *)avOpt;
    liListenSocket = lpoMainClass->ciListenSocket;
    lpoMainClass->ciAcceptThreadID = pthread_self();
    
    while(lpoMainClass->ciShutDown == 0)
    {
        lpoMainClass->ClearInvalid();
        
        liClientSocket = ai_socket_accept(liListenSocket, lsClientIp, &liClientPort);
        if (!IS_VALID_SOCKET(liClientSocket))
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE,
                "[%s]:  accept new connection <%d> ... fail = %d", 
                __FUNCTION__, 
                liListenSocket, 
                liClientSocket
            );

            continue;
        }

        AIWriteLOG(
            AILIB_LOGFILE,
            AILOG_LEVEL_TRACE, 
            "[%s]:  accept new connection <%d> = %s:%d", 
            __FUNCTION__, 
            liListenSocket, 
            lsClientIp, 
            liClientPort
        );

        AI_NEW_ASSERT(lpoSocket, ASYNCSOCK);
        
        if(strlen(lpoMainClass->csServerIp))
        {
            _StringCopy(lpoSocket->csLocalIp, lpoMainClass->csServerIp);
        }
        else
        {
            _StringCopy(lpoSocket->csLocalIp, "localhost");
        }

        lpoSocket->ciLocalPort = lpoMainClass->ciServerPort;

        _StringCopy(lpoSocket->csRemoteIp, lsClientIp);
        lpoSocket->ciRemotePort = liClientPort;

        lpoSocket->cpoAsyncObj = (void *)lpoMainClass;
        lpoSocket->ciActiveTime = time(NULL);
        lpoSocket->ciSocket = liClientSocket;
        
        if(pthread_create(&ltNewRecvThread, NULL, async_s_recv_thread, lpoSocket))
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE, 
                "[%s]:  pthread_create(%s:%d) ... fail", 
                __FUNCTION__,
                lsClientIp, 
                liClientPort
            );

            ai_socket_close(liClientSocket);
            AI_DELETE(lpoSocket);
            
            sleep(1);
            continue;
        }
        
        AISmartLock loSmartLock(lpoMainClass->coSocketHeadLock);
        ai_list_add_tail(&(lpoSocket->coSocketChild), &(lpoMainClass->coSocketHead));
    }
    
    lpoMainClass->ciAcceptThreadID = 0;
    return  (NULL);
}

void *async_accept_thread_ex(void *avOpt)
{
    int                 liClientPort;
    int                 liServerPort;
    int                 liClientSocket;
    char                lsClientIp[32];
    char                lsServerIp[32];
    char                lsListenName[64];
    pthread_t           ltNewRecvThread;
    ASYNCSOCK           *lpoSocket = NULL;
    clsAsyncServer      *lpoMainClass = NULL;
    
    pthread_detach(pthread_self());

    lpoMainClass = (clsAsyncServer *)avOpt;
    _StringCopy(lsListenName, lpoMainClass->csListenName);
    lpoMainClass->ciAcceptThreadID = pthread_self();
    
    while(lpoMainClass->ciShutDown == 0)
    {
        lpoMainClass->ClearInvalid();
        
        liClientSocket = lpoMainClass->cpfGetConnection(lsListenName, lsClientIp, &liClientPort);
        if (!IS_VALID_SOCKET(liClientSocket))
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE,
                "[%s]:  accept new connection <%s> ... fail = %d",
                __FUNCTION__, 
                lsListenName, 
                liClientSocket
            );

            continue;
        }

        AIWriteLOG(
            AILIB_LOGFILE,
            AILOG_LEVEL_TRACE, 
            "[%s]:  accept new connection <%s> = %s:%d",
            __FUNCTION__, 
            lsListenName, 
            lsClientIp, 
            liClientPort
        );

        AI_NEW_ASSERT(lpoSocket, ASYNCSOCK);
        
        ai_socket_local_addr( liClientSocket, lsServerIp, liServerPort );
        _StringCopy(lpoSocket->csLocalIp, lsServerIp);
        lpoSocket->ciLocalPort = liServerPort;
        
        _StringCopy(lpoSocket->csRemoteIp, lsClientIp);
        lpoSocket->ciRemotePort = liClientPort;

        lpoSocket->cpoAsyncObj = (void *)lpoMainClass;
        lpoSocket->ciActiveTime = time(NULL);
        lpoSocket->ciSocket = liClientSocket;
        
        if(pthread_create(&ltNewRecvThread, NULL, async_s_recv_thread, lpoSocket))
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE,
                "[%s]:  pthread_create(%s:%d) ... fail", 
                __FUNCTION__, 
                lsClientIp, 
                liClientPort
            );

            ai_socket_close(liClientSocket);
            AI_DELETE(lpoSocket);
            
            sleep(1);
            continue;
        }
        
        AISmartLock loSmartLock(lpoMainClass->coSocketHeadLock);
        ai_list_add_tail(&(lpoSocket->coSocketChild), &(lpoMainClass->coSocketHead));
    }
    
    lpoMainClass->ciAcceptThreadID = 0;
    return  (NULL);
}

clsAsyncServer::clsAsyncServer(char const* apcIpAddr, int aiServerPort)
{
    ciShutDown = 0;
    cpfCallback = NULL;
    cpfCallbackEx = NULL;
    ciAcceptThreadID = 0;
    ciServerPort = aiServerPort;
    _StringCopy(csServerIp, (apcIpAddr?apcIpAddr:""));
    
    memset(csListenName, 0, sizeof(csListenName));
    cpfGetConnection = NULL;

    ai_init_list_head(&coSocketHead);
    
    AISmartLock loSmartLock(soAsyncHeadLock);
    ai_list_add_tail(&coAsyncChld, &soAsyncHead);
}

clsAsyncServer::~clsAsyncServer()
{
    AISmartLock loSmartLock(soAsyncHeadLock);
    ai_list_del_any(&coAsyncChld);
}

void clsAsyncServer::SetCallback(asyncCallback apfCallBack)
{
    cpfCallback = apfCallBack;
}

void clsAsyncServer::SetCallbackEx(asyncCallbackEx apfCallBack)
{
    cpfCallbackEx = apfCallBack;
}


void clsAsyncServer::ClearInvalid(void)
{
    LISTHEAD        *lpoHead = NULL;
    
AGAIN:  
    lpoHead = &coSocketHead;
    coSocketHeadLock.Lock();
    AI_FOR_EACH_IN_LIST(lpoSocket, lpoHead, ASYNCSOCK, coSocketChild)
    {
        if (!IS_VALID_SOCKET(lpoSocket->ciSocket))
        {
            if(lpoSocket->ctRecvThreadID == 0)
            {
                if(lpoSocket->ciExUsed == 0)
                {
                    ai_list_del_any(&lpoSocket->coSocketChild);
                    coSocketHeadLock.Unlock();
                    
                    AI_DELETE(lpoSocket);
                    goto  AGAIN;
                }
            }
        }       
    }
    coSocketHeadLock.Unlock();
}

void clsAsyncServer::ShutDown(void)
{
    LISTHEAD        *lpoHead = NULL;

    ciShutDown = 1;
    ai_socket_close(ciListenSocket);

    while(ciAcceptThreadID)
    {
        sleep(1);
    }
    
    lpoHead = &coSocketHead;
    coSocketHeadLock.Lock();
    AI_FOR_EACH_IN_LIST(lpoSocket, lpoHead, ASYNCSOCK, coSocketChild)
    {
        if (IS_VALID_SOCKET(lpoSocket->ciSocket))
        {
            ai_socket_close(lpoSocket->ciSocket);
        }
    }
    
    coSocketHeadLock.Unlock();
    sleep(1);
    
AGAIN:
    coSocketHeadLock.Lock();
    AI_FOR_EACH_IN_LIST(lpoSocket, lpoHead, ASYNCSOCK, coSocketChild)
    {
        if(lpoSocket->ctRecvThreadID == 0)
        {
            if(lpoSocket->ciExUsed == 0)
            {
                ai_list_del_any(&lpoSocket->coSocketChild);
                coSocketHeadLock.Unlock();
                
                AI_DELETE(lpoSocket);
                goto  AGAIN;
            }
        }
    }
    
    if(!ai_list_is_empty(lpoHead))
    {
        coSocketHeadLock.Unlock();
        sleep(1);
        
        goto  AGAIN;
    }
    
    coSocketHeadLock.Unlock();
}

int clsAsyncServer::StartDaemon( const char *apcListenName, gpfGetConnection apfFunc, gpfGetListenSocket apfListen )
{
    pthread_t       ltThreadID;
    
    if( apcListenName && apfFunc )
    {
        cpfGetConnection = apfFunc;
        _StringCopy(csListenName, apcListenName);

        ciListenSocket = apfListen( csListenName );
        if (!IS_VALID_SOCKET(ciListenSocket))
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE,
                "[%s]:  ai_socket_listen(%s) ... fail",
                __FUNCTION__, 
                csListenName
            );

            return  (ASYNC_ERROR_LISTENFAIL);
        }

        if(pthread_create(&ltThreadID, NULL, async_accept_thread_ex, this))
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE, 
                "[%s]:  pthread_create(%s) ... fail", 
                __FUNCTION__, 
                apcListenName
            );

            return  (ASYNC_ERROR_NEWTHREAD);
        }

        while(ciAcceptThreadID == 0)  sleep(1);
        return  (0);
    }
    
    ciListenSocket = ai_socket_listen((csServerIp[0]?csServerIp:NULL), ciServerPort, 5);
    if (!IS_VALID_SOCKET(ciListenSocket))
    {
        AIWriteLOG(
            AILIB_LOGFILE,
            AILOG_LEVEL_TRACE,
            "[%s]:  ai_socket_listen(%s:%d) ... fail", 
            __FUNCTION__, 
            csServerIp, 
            ciServerPort
        );

        return  (ASYNC_ERROR_LISTENFAIL);
    }   
    
    if(pthread_create(&ltThreadID, NULL, async_accept_thread, this))
    {
        AIWriteLOG(
            AILIB_LOGFILE,
            AILOG_LEVEL_TRACE,
            "[%s]:  pthread_create(%s:%d) ... fail", 
            __FUNCTION__, 
            csServerIp, 
            ciServerPort
        );

        ai_socket_close(ciListenSocket);
        return  (ASYNC_ERROR_NEWTHREAD);
    }
    
    while(ciAcceptThreadID == 0)
    {
        sleep(1);
    }

    return  (0);
}

void clsAsyncServer::ListConnection(void)
{
    LISTHEAD*   lpoHead = &coSocketHead;

    AISmartLock loSmartLock(coSocketHeadLock);
    AI_FOR_EACH_IN_LIST(lpoSocket, lpoHead, ASYNCSOCK, coSocketChild)
    {
        if (IS_VALID_SOCKET(lpoSocket->ciSocket))
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE,
                "[%s]:  %3d> %18s:%d -> %18s:%d  [Idle = %2"PRIdMAX"]", 
                __FUNCTION__, 
                lpoSocket->ciSocket, 
                lpoSocket->csRemoteIp, 
                lpoSocket->ciRemotePort,
                lpoSocket->csLocalIp, 
                lpoSocket->ciLocalPort, 
                (intmax_t)(time(NULL) - lpoSocket->ciActiveTime)
            );
        }
    }
        
    coSocketHeadLock.Unlock();
}

ASYNCSOCK *clsAsyncServer::FindSocket(ASYNCMATCH const* apoMatch)
{
    LISTHEAD*   lpoHead = &coSocketHead;

    AI_FOR_EACH_IN_LIST(lpoSocket, lpoHead, ASYNCSOCK, coSocketChild)
    {
        if(apoMatch->ctThreadID == lpoSocket->ctRecvThreadID)
        {
            if (IS_VALID_SOCKET(lpoSocket->ciSocket))
            {
                lpoSocket->ciExUsed += 1;
                return  (lpoSocket);
            }
        }
    }
    
    return  (NULL);
}

int clsAsyncServer::SendResult(ASYNCMATCH const* apoMatch, int aiResult)
{
    int             liSize = 0;
    ASYNCHEAD       loAsyncHead;
    ASYNCSOCK       *lpoSocket = NULL;

    coSocketHeadLock.Lock();    
    if((lpoSocket = FindSocket(apoMatch)))
    {       
        coSocketHeadLock.Unlock();
                
        liSize = sizeof(ASYNCHEAD);
        loAsyncHead.ciLength = htonl(0);
        loAsyncHead.ciResult = htonl(aiResult);
        loAsyncHead.ciMsgID = htonl(apoMatch->ciMsgID);
    
        lpoSocket->ctMutexLock.Lock();
        if(ai_socket_senddata(lpoSocket->ciSocket, (char *)&loAsyncHead, liSize, ASYNC_NET_TIMEOUT) != liSize)
        {
            lpoSocket->ctMutexLock.Unlock();
                    
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE,
                "[%s]:  ai_socket_senddata(%s:%d) ... fail", 
                __FUNCTION__, 
                lpoSocket->csRemoteIp, 
                lpoSocket->ciRemotePort
            );

            coSocketHeadLock.Lock();
            lpoSocket->ciExUsed -= 1;
            ai_socket_close(lpoSocket->ciSocket);
            coSocketHeadLock.Unlock();
                    
            return  (ASYNC_ERROR_SENDFAIL);
        }

        lpoSocket->ctMutexLock.Unlock();

        coSocketHeadLock.Lock();
        lpoSocket->ciExUsed -= 1;
        coSocketHeadLock.Unlock();
                
        return  (0);            
    }

    coSocketHeadLock.Unlock();
    return  (ASYNC_ERROR_NO_SOCKET);
}

int clsAsyncServer::SendResult(ASYNCMATCH const* apoMatch, char const* apcData, int aiSize, int aiResult)
{
    int             liSize = 0;
    int             liTrueSize;
    char            *lpcBuf = NULL;
    ASYNCSOCK       *lpoSocket = NULL;
    ASYNCHEAD       *lpoAsyncHead = NULL;
        
    coSocketHeadLock.Lock();
    if((lpoSocket = FindSocket(apoMatch)))
    {
        coSocketHeadLock.Unlock();
                
        liTrueSize = (aiSize>0?aiSize:0);
        liSize = liTrueSize + sizeof(ASYNCHEAD);

        AI_NEW_N_ASSERT(lpcBuf, char, liSize);
        lpoAsyncHead = (ASYNCHEAD *)lpcBuf;
                
        lpoAsyncHead->ciResult = htonl(aiResult);
        lpoAsyncHead->ciLength = htonl(liTrueSize);
        lpoAsyncHead->ciMsgID = htonl(apoMatch->ciMsgID);
                
        if(apcData && liTrueSize > 0)
        {
            memcpy(lpcBuf+sizeof(ASYNCHEAD), apcData, liTrueSize);
        }
                    
        lpoSocket->ctMutexLock.Lock();
        if(ai_socket_senddata(lpoSocket->ciSocket, lpcBuf, liSize, ASYNC_NET_TIMEOUT) != liSize)
        {
            lpoSocket->ctMutexLock.Unlock();
            AI_DELETE_N(lpcBuf);
                    
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_TRACE,
                "[%s]:  ai_socket_senddata(%s:%d) ... fail",
                __FUNCTION__, 
                lpoSocket->csRemoteIp, 
                lpoSocket->ciRemotePort
            );

            coSocketHeadLock.Lock();
            lpoSocket->ciExUsed -= 1;
            ai_socket_close(lpoSocket->ciSocket);
            coSocketHeadLock.Unlock();
                    
            return  (ASYNC_ERROR_SENDFAIL);
        }

        lpoSocket->ctMutexLock.Unlock();
        AI_DELETE_N(lpcBuf);

        coSocketHeadLock.Lock();
        lpoSocket->ciExUsed -= 1;
        coSocketHeadLock.Unlock();
                
        return  (0);            
    }

    coSocketHeadLock.Unlock();  
    return  (ASYNC_ERROR_NO_SOCKET);
}

///end namespace
AIBC_NAMESPACE_END
