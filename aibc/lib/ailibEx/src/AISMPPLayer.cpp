#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "AITime.h"
#include "AIString.h"
#include "AISocket.h"
#include "AISMPPLayer.h"
#include "AILogSys.h"

///start namespace
AIBC_NAMESPACE_START

#define _CLOSE_RETURN_IF(ret, expr, sock)   if (expr) { ai_socket_close(sock); return ret; }         

#define _StringCopy(dest, src)  StringCopy(dest, src, sizeof(dest))

#define _SMPP_ERR_TOOBIG    (-1)
#define _SMPP_ERR_SEND      (-2)
#define _SMPP_ERR_RECV      (-3)
#define _SMPP_ERR_HEAD      (-4)
#define _SMPP_ERR_PDU       (-5)

static unsigned char NokiaCodeTable[128] =
{
    0x40, 0x9C, 0x24, 0x9D, 0x8A, 0x82, 0x97, 0x8D,
    0x95, 0x80, 0x0A, 0x20, 0x20, 0x0D, 0x8F, 0x86,
    0x20, 0x5F, 0xE8, 0xE2, 0x20, 0xEA, 0x20, 0x20,
    0xE4, 0xE9, 0x20, 0x20, 0x92, 0x91, 0xE1, 0x90,
    0x20, 0x21, 0x22, 0x23, 0x20, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0xAD, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x8E, 0x99, 0xA5, 0x9A, 0x20,
    0xA8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x84, 0x94, 0xA4, 0x81, 0x85
};


static int DefaultToASCII(char *pIN, int iInSize, char *pOUT)
{
    short   iCodeIndex = 0;
    char    *lpOutput = pOUT;


    for( int liIt = 0; liIt < iInSize; liIt++ )
    {
        iCodeIndex = (*(pIN+liIt))&0x7F;
        *lpOutput = NokiaCodeTable[iCodeIndex];
        lpOutput += 1;
    }

    *lpOutput = '\0';
    return  strlen( pOUT );
}


static int CheckConcatenated( char *apcMessage, char aiEsmClass )
{
    if( (aiEsmClass & (1<<6)) )
    {
        if( apcMessage[0] == 0x05 && apcMessage[1] == 0x00 && apcMessage[2] == 0x03 )
        {
            return  (6);
        }
        else if( apcMessage[0] == 0x06 && apcMessage[1] == 0x08 && apcMessage[2] == 0x04 )
        {
            return  (7);
        }
        else
        {
            return  (1);
        }
    }

    return  (0);
}


// return recv size or error
ssize_t _RecvSMPPPacket(int aiSocket, SMPP_HEADER* apoHeader, void* apPdu, size_t aiLen)
{
    char    lsHeader[16];
    ssize_t liPduSize;
    ssize_t liHdrSize;

    // recv header
    liHdrSize = ai_socket_recvdata(aiSocket, lsHeader, sizeof(lsHeader), 5);
    AI_RETURN_IF(_SMPP_ERR_RECV, (liHdrSize < 0));
    AI_RETURN_IF(0, (0 == liHdrSize));

    //AIWriteHexLOG("PACKET.log", "RECV SMPP_HEADER", lsHeader, liHdrSize);

    AI_RETURN_IF(_SMPP_ERR_RECV, (sizeof(lsHeader) != liHdrSize));

    // decode header
    liHdrSize = apoHeader->Decode(lsHeader, sizeof(lsHeader));
    AI_RETURN_IF(_SMPP_ERR_HEAD, (liHdrSize != sizeof(lsHeader)));

    // get pdu size
    liPduSize = apoHeader->command_length - sizeof(lsHeader);
    AI_RETURN_IF(_SMPP_ERR_PDU, (liPduSize < 0)); 
    AI_RETURN_IF(_SMPP_ERR_TOOBIG, (aiLen < (size_t)liPduSize));
    AI_RETURN_IF(liHdrSize, (0 == liPduSize));

    // recv pdu
    liPduSize = ai_socket_recvdata(aiSocket, (char*)apPdu, liPduSize, 5);
    AI_RETURN_IF(_SMPP_ERR_RECV, (liPduSize <= 0)); // timeout here means error

    //AIWriteHexLOG("PACKET.log", "RECV SMPP_PDU", (char*)apPdu, liPduSize);

    // return all recv size
    return liHdrSize + liPduSize;
}


// return send size or error
ssize_t _SendSMPPPacket(int aiSocket, SMPP_HEADER* apoHeader, void const* apPdu, size_t aiLen)
{
    char    lsBuffer[1024];
    ssize_t liSize;
    ssize_t liRetSize;

    // header
    apoHeader->command_length = 16 + aiLen;
    liRetSize = apoHeader->Encode(lsBuffer, 16);
    AI_RETURN_IF(_SMPP_ERR_HEAD, (liRetSize != 16));

    // pdu
    AI_RETURN_IF(_SMPP_ERR_TOOBIG, (aiLen > sizeof(lsBuffer) - 16));
    memcpy(lsBuffer + 16, apPdu, aiLen);

    // send all 
    liSize = aiLen + 16;
    liRetSize = ai_socket_senddata(aiSocket, lsBuffer, liSize, 5);
    AI_RETURN_IF(_SMPP_ERR_SEND, (liSize != liRetSize));

    // return all recv size
    return liRetSize;
}




void *smpp_c_recv_thread(void *avOpt)
{
    ssize_t             liSize;
    int                 liRetCode;
    int                 liLocalPort;
    char                lsLocalIp[20];
    char                lsPdu[1024];
    time_t              ltLastActTime;
    time_t              ltReConnTime;
    SMPP_HEADER         loSmppHeader;
    clsSmppClient*      lpoMainClass;
    SmppClientInfo*     lpoClientInfo;
    SmppSocketInfo*     lpoSocketInfo;
    
    pthread_detach(pthread_self());
    lpoSocketInfo = (SmppSocketInfo *)avOpt;
    lpoClientInfo = lpoSocketInfo->cpoClientInfo;

    lpoSocketInfo->ctRecvThreadID = pthread_self();
    lpoMainClass = (clsSmppClient *)lpoSocketInfo->cpoSmppClass;
    
    ltReConnTime = 0;
    ltLastActTime = time(NULL);
    
    while(lpoClientInfo->ciShutDown == 0)
    {
        if (lpoSocketInfo->ciSocket < 0)
        {
            int     liSocket = 0;
            
            if (time(NULL) < (ltReConnTime + lpoMainClass->ciReConnTime))
            {
                sleep(1);
                continue;
            }
           
            if (lpoSocketInfo->ciExUsed > 0)
            {
                sleep(1);
                continue;
            }

            ltReConnTime = time(NULL);

            liSocket = ai_tcp_connect(lpoClientInfo->csRemoteIp, lpoClientInfo->ciRemotePort, SCT_SOCK_TIMEOUT);

            if (liSocket < 0)
            {
                AIWriteLOG(
                    NULL,
                    AILOG_LEVEL_ERROR,
                    "[%s] ai_tcp_connect(%s:%d) ... Fail = %d", 
                    __FUNCTION__, 
                    lpoClientInfo->csRemoteIp, 
                    lpoClientInfo->ciRemotePort, 
                    liSocket
                );
                continue;
            }
           
            lpoSocketInfo->ctActiveTime = time(NULL);
            lpoSocketInfo->ciEnquireLinkCount = 0; 

            // handle bind 
            if ((liRetCode = lpoMainClass->DealBIND(liSocket, lpoClientInfo)))
            {
                AIWriteLOG(
                    NULL,
                    AILOG_LEVEL_ERROR,
                    "[%s] DealBIND(%s:%d) ... Fail, err=%d", 
                    __FUNCTION__, 
                    lpoClientInfo->csRemoteIp, 
                    lpoClientInfo->ciRemotePort,
                    liRetCode
                );
                ai_socket_close(liSocket);
                continue;
            }
            
            ai_socket_local_addr(liSocket, lsLocalIp, liLocalPort);

            lpoClientInfo->coSocketHeadLock.Lock();

            _StringCopy(lpoSocketInfo->csLocalIp, lsLocalIp);
            lpoSocketInfo->ciLocalPort = liLocalPort;
            lpoSocketInfo->ciSocket = liSocket;

            lpoClientInfo->coSocketHeadLock.Unlock();
            
            if (lpoMainClass->cpfConnLoginCallback)
            {
                lpoMainClass->cpfConnLoginCallback(lpoSocketInfo);
            }
        }
      
        liSize = _RecvSMPPPacket(lpoSocketInfo->ciSocket, &loSmppHeader, lsPdu, sizeof(lsPdu));

        // handle error
        if (liSize < 0)
        {
            AIWriteLOG(
                NULL,
                AILOG_LEVEL_ERROR,
                "[%s] recv SMPP packdet (%s:%d) ... Fail",
                __FUNCTION__, 
                lpoSocketInfo->csLocalIp,
                lpoSocketInfo->ciLocalPort
            );
            goto  CONNECTION_BREAK;
        }

        // handle timeout 
        if (0 == liSize)
        {
            if ((time(NULL) - ltLastActTime) >= lpoMainClass->ciEnquireTime)
            {
                if (lpoSocketInfo->ciEnquireLinkCount > 3)
                {
                    goto  CONNECTION_BREAK;
                }

                ltLastActTime = time(NULL);
                
                loSmppHeader.command_id = AI_SMPP_CMD_ENQUIRE_LINK;
                loSmppHeader.command_status = 0;
                loSmppHeader.sequence_number = lpoSocketInfo->GetNextSeq();
               
                { 
                    AISmartLock loSmartLock(lpoSocketInfo->coWriteMutexLock);
                    liSize = _SendSMPPPacket(lpoSocketInfo->ciSocket, &loSmppHeader, NULL, 0);
                }

                if (liSize < 0)
                {
                    AIWriteLOG(
                        NULL,
                        AILOG_LEVEL_ERROR,
                        "[%s] send SMPP packet(%s:%d) ... Fail, err=%d", 
                        __FUNCTION__, 
                        lpoSocketInfo->csLocalIp, 
                        lpoSocketInfo->ciLocalPort,
                        liSize
                    );
                    goto  CONNECTION_BREAK;
                }
                ++lpoSocketInfo->ciEnquireLinkCount;
            }
            
            continue;
        } 
        
        lpoSocketInfo->ctActiveTime = time(NULL);
        lpoSocketInfo->ciEnquireLinkCount = 0; 

        liSize -= 16;
        assert(liSize >= 0);

        if (loSmppHeader.command_id == AI_SMPP_CMD_SUBMIT_SM_RESP)
        {
            liRetCode = lpoMainClass->DealSubmitRESP(lpoSocketInfo, &loSmppHeader, lsPdu, liSize);
            continue;
        }
        else if (loSmppHeader.command_id == AI_SMPP_CMD_DELIVER_SM)
        {
            int liType = lpoMainClass->CheckStatReport(lsPdu, liSize);

            if (liType < 0)
            {
                //error pdu, ignore    
                break;            
            }            
            else if (liType > 0)
            {
                liRetCode = lpoMainClass->DealStatREPORT(lpoSocketInfo, &loSmppHeader, lsPdu, liSize);
            }
            else
            {
                liRetCode = lpoMainClass->DealDELIVER(lpoSocketInfo, &loSmppHeader, lsPdu, liSize);
            }
            continue;
        }
        else if (loSmppHeader.command_id == AI_SMPP_CMD_UNBIND)
        {
            AIWriteLOG(
                NULL,
                AILOG_LEVEL_DEBUG, 
                "[%s] (%s:%d) Receive UNBIND", 
                __FUNCTION__, 
                lpoSocketInfo->csLocalIp, 
                lpoSocketInfo->ciLocalPort
            );
            goto  CONNECTION_BREAK;
        }
        else if (loSmppHeader.command_id == AI_SMPP_CMD_ENQUIRE_LINK)
        {
            AIWriteLOG(
                NULL, 
                AILOG_LEVEL_DEBUG,
                "[%s] (%s:%d) Receive ENQUIRE_LINK", 
                __FUNCTION__, 
                lpoSocketInfo->csLocalIp, 
                lpoSocketInfo->ciLocalPort
            );

            loSmppHeader.command_id = AI_SMPP_CMD_ENQUIRE_LINK_RESP;
            loSmppHeader.command_status = 0;
           
            { 
                AISmartLock loSmartWriteMutexLock(lpoSocketInfo->coWriteMutexLock);
                liSize = _SendSMPPPacket(lpoSocketInfo->ciSocket, &loSmppHeader, NULL, 0);
            }

            if (liSize < 0)
            {
                AIWriteLOG(
                    NULL,
                    AILOG_LEVEL_ERROR,
                    "[%s] send SMPP packet(%s:%d) ... Fail, err=%d", 
                    __FUNCTION__, 
                    lpoSocketInfo->csLocalIp, 
                    lpoSocketInfo->ciLocalPort,
                    liSize
                );
                goto  CONNECTION_BREAK;
            }               
            
            continue;
        }
        else if (loSmppHeader.command_id == AI_SMPP_CMD_ENQUIRE_LINK_RESP)
        {
            AIWriteLOG(
                NULL,
                AILOG_LEVEL_DEBUG,
                "[%s] (%s:%d) Receive ENQUIRE_LINK_RESP", 
                __FUNCTION__, 
                lpoSocketInfo->csLocalIp, 
                lpoSocketInfo->ciLocalPort
            );
            continue;
        }
        else if (loSmppHeader.command_id == AI_SMPP_CMD_GENERIC_NACK)
        {
            AIWriteLOG(
                NULL,
                AILOG_LEVEL_DEBUG,
                "[%s] (%s:%d) Receive GENERIC_NACK", 
                __FUNCTION__, 
                lpoSocketInfo->csLocalIp, 
                lpoSocketInfo->ciLocalPort
            );
            continue;
        }
        else
        {
            AIWriteLOG(
                NULL,
                AILOG_LEVEL_DEBUG,
                "[%s] (%s:%d) Receive CommandID = 0x%08X", 
                __FUNCTION__, 
                lpoSocketInfo->csLocalIp, 
                lpoSocketInfo->ciLocalPort, 
                loSmppHeader.command_id
            );
            continue;
        }
        
CONNECTION_BREAK:
        if (lpoMainClass->cpfConnBreakCallback)
        {
            lpoMainClass->cpfConnBreakCallback(lpoSocketInfo);
        }

        ai_socket_close(lpoSocketInfo->ciSocket);
        continue;
    }
    
    ai_socket_close(lpoSocketInfo->ciSocket);
    lpoSocketInfo->ctRecvThreadID = 0;
    
    return  (NULL);
}


clsSmppClient::clsSmppClient(const int aiEnquireTime, const int aiReConnectTime)
{
    ai_init_list_head(&coClientHead);

    ciReConnTime = ((aiReConnectTime<3)?3:aiReConnectTime);
    ciEnquireTime = ((aiEnquireTime<5)?5:aiEnquireTime);
    
    cpfGetRoute = NULL;
    cpfDeliverCallback = NULL;
    cpfSubmitRespCallback = NULL;
    cpfStatReportCallback = NULL;
    cpfConnBreakCallback = NULL;
    cpfConnLoginCallback = NULL;

    //for(int liIt = 0; liIt < SCT_MAX_HASH_SIZE; liIt++)
    //{
    //    ai_init_list_head(&(coIndexHead[liIt]));
    //}
}


void clsSmppClient::ShutDown(SmppClientInfo *apoClientInfo)
{
    apoClientInfo->coSocketHeadLock.Lock();
    apoClientInfo->ciShutDown = 1;

    AI_FOR_EACH_IN_LIST(lpoSocketInfo, &(apoClientInfo->coSocketHead), SmppSocketInfo, coSocketChild)
    {
        if (lpoSocketInfo->ciSocket >= 0)
        {
            ai_socket_close(lpoSocketInfo->ciSocket);
        }
    }

    apoClientInfo->coSocketHeadLock.Unlock();

AGAIN:  
    apoClientInfo->coSocketHeadLock.Lock();

    AI_FOR_EACH_IN_LIST(lpoSocketInfo, &(apoClientInfo->coSocketHead), SmppSocketInfo, coSocketChild)
    {
        if (lpoSocketInfo->ctRecvThreadID == 0)
        {
            if (lpoSocketInfo->ciExUsed == 0)
            {
                ai_list_del_any(&(lpoSocketInfo->coSocketChild));
                AI_DELETE(lpoSocketInfo);

                apoClientInfo->coSocketHeadLock.Unlock();
                
                //ai_socket_close(lpoSocketInfo->ciSocket);
                goto  AGAIN;
            }
        }   
    }

    if (!ai_list_is_empty(&(apoClientInfo->coSocketHead)))
    {
        apoClientInfo->coSocketHeadLock.Unlock();
        sleep(1);
        
        goto  AGAIN;
    }   

    apoClientInfo->coSocketHeadLock.Unlock();
}


void clsSmppClient::ShutDown(void)
{
    LISTHEAD*       lpoTemp;
    SmppClientInfo* lpoClientInfo;

    coMutexLock.Lock();

    while(!ai_list_is_empty(&coClientHead))
    {
        ai_list_del_head(lpoTemp, &coClientHead);
        lpoClientInfo = AI_GET_STRUCT_PTR(lpoTemp, SmppClientInfo, coClientChild);

        coMutexLock.Unlock();
        
        clsSmppClient::ShutDown(lpoClientInfo);
        AI_DELETE(lpoClientInfo);     
        
        coMutexLock.Lock();
    }
    
    coMutexLock.Unlock();
}


void clsSmppClient::CheckThread(void)
{
AGAIN:
    coMutexLock.Lock();
    
    AI_FOR_EACH_IN_LIST(lpoClientInfo, &coClientHead, SmppClientInfo, coClientChild)
    {
        lpoClientInfo->coSocketHeadLock.Lock();

        AI_FOR_EACH_IN_LIST(lpoSocketInfo, &(lpoClientInfo->coSocketHead), SmppSocketInfo, coSocketChild)
        {
            if (lpoSocketInfo->ctRecvThreadID == 0)
            {
                lpoClientInfo->coSocketHeadLock.Unlock();
                coMutexLock.Unlock();
                
                sleep(1);
                goto  AGAIN;
            }
        }

        lpoClientInfo->coSocketHeadLock.Unlock();
    }
    
   coMutexLock.Unlock();
}


void clsSmppClient::CreateThread(SmppClientInfo *apoClientInfo)
{
    pthread_t           ltThreadID;
    SmppSocketInfo*     lpoSocketInfo;
    
    AISmartLock loSmartSocketHeadLock(apoClientInfo->coSocketHeadLock);

    for(size_t liIt = 0; liIt < apoClientInfo->ciConnCount; liIt++)
    {
        AI_NEW_ASSERT(lpoSocketInfo, SmppSocketInfo);
                
        lpoSocketInfo->cpoSmppClass = (void *)this;
        lpoSocketInfo->cpoClientInfo = apoClientInfo;
                
        if (pthread_create(&ltThreadID, NULL, smpp_c_recv_thread, lpoSocketInfo))
        {
            AIWriteLOG(
                NULL,
                AILOG_LEVEL_ERROR, 
                "[%s] ALERT:  pthread_create(smpp_c_recv_thread) ... Fail",
                __FUNCTION__
            );
            AI_DELETE(lpoSocketInfo);
            break;
        }

        ai_list_add_tail(&(lpoSocketInfo->coSocketChild), &(apoClientInfo->coSocketHead));
    }
}


int clsSmppClient::AjustConnections(SmppClientInfo* apoClientInfo, size_t aiNewConnCount)
{
    if (apoClientInfo->ciConnCount < aiNewConnCount)
    { // to add
        AISmartLock loSmartLock(apoClientInfo->coSocketHeadLock);

        for (size_t i = apoClientInfo->ciConnCount; i < aiNewConnCount; ++i)
        {
            SmppSocketInfo* lpoSocketInfo;
            pthread_t       ltThreadID;

            AI_NEW_ASSERT(lpoSocketInfo, SmppSocketInfo);
                    
            lpoSocketInfo->cpoSmppClass = (void *)this;
            lpoSocketInfo->cpoClientInfo = apoClientInfo;
                    
            if (pthread_create(&ltThreadID, NULL, smpp_c_recv_thread, lpoSocketInfo))
            {
                AIWriteLOG(
                    NULL,
                    AILOG_LEVEL_ERROR, 
                    "[%s] ALERT:  pthread_create(smpp_c_recv_thread) ... Fail",
                    __FUNCTION__
                );

                AI_DELETE(lpoSocketInfo);
                return -1;
            }
            
            ai_list_add_tail(&(lpoSocketInfo->coSocketChild), &(apoClientInfo->coSocketHead));
            ++(apoClientInfo->ciConnCount);
        }
    }
    else if (apoClientInfo->ciConnCount > aiNewConnCount)
    { // to remove
        // TODO: make sure threads exit before closing sockets...
    }

    return 0;
}


void clsSmppClient::StartDaemon(void)
{
    coMutexLock.Lock();
    
    AI_FOR_EACH_IN_LIST(lpoClientInfo, &coClientHead, SmppClientInfo, coClientChild)
    {
        CreateThread(lpoClientInfo);
    }
    
    coMutexLock.Unlock();

    CheckThread();
}


void clsSmppClient::ListConnection(char *apcInformation)
{
    int         liTempVal = 0;
    char*       lpcPtr = NULL;

    lpcPtr = apcInformation;
    if (lpcPtr)  *lpcPtr = (char)0;

    AISmartLock loSmartMutexLock(coMutexLock);

    AI_FOR_EACH_IN_LIST(lpoClientInfo, &coClientHead, SmppClientInfo, coClientChild)
    {
        AISmartLock loSmartSocketHeadLock(lpoClientInfo->coSocketHeadLock);

        AI_FOR_EACH_IN_LIST(lpoSocketInfo, &(lpoClientInfo->coSocketHead), SmppSocketInfo, coSocketChild)
        {
            if (lpoSocketInfo->ciSocket >= 0)
            {
                if ((liTempVal = lpoSocketInfo->ctActiveTime - time(NULL)) <= 0)
                {
                     liTempVal = 0;
                }

                if (apcInformation)
                {           
                    sprintf(
                        lpcPtr, 
                        "%s(%3d):  %18s:%d -> %18s:%d  [Wait = %2d]\r\n", 
                        __FUNCTION__, 
                        lpoSocketInfo->ciSocket, 
                        lpoSocketInfo->csLocalIp, 
                        lpoSocketInfo->ciLocalPort, 
                        lpoClientInfo->csRemoteIp, 
                        lpoClientInfo->ciRemotePort, 
                        liTempVal
                    );

                    lpcPtr += strlen(lpcPtr);                   
                }
                else
                {
                    AIWriteLOG(
                        NULL,
                        AILOG_LEVEL_DEBUG, 
                        "[%s] (%d):  %s:%d -> %s:%d  [Wait = %d]",
                        __FUNCTION__, 
                        lpoSocketInfo->ciSocket, 
                        lpoSocketInfo->csLocalIp, 
                        lpoSocketInfo->ciLocalPort, 
                        lpoClientInfo->csRemoteIp,
                        lpoClientInfo->ciRemotePort, 
                        liTempVal
                    );
                }
            }
        }
    }
}


SmppClientInfo* clsSmppClient::SearchConnection(const char *apcIp, const int aiPort, const char *apcSystemID, int aiBindType)
{
    //int             liHashKey;
    //char            lsString[64];

    //sprintf(lsString, "%s%s", apcIp, apcSystemID);
    //liHashKey = GetStringHashKey1(lsString, SCT_MAX_HASH_SIZE);

    //AI_FOR_EACH_IN_LIST(lpoClientInfo, &(coIndexHead[liHashKey]), SmppClientInfo, coIndexChild)
    AI_FOR_EACH_IN_LIST(lpoClientInfo, &coClientHead, SmppClientInfo, coClientChild)
    {
        if (strcmp(lpoClientInfo->csRemoteIp, apcIp) == 0
            && strcmp(lpoClientInfo->csSystemID, apcSystemID) == 0
            && lpoClientInfo->ciRemotePort == aiPort
            && (aiBindType == 0 || lpoClientInfo->ciBindType == aiBindType))
        {
            return  (lpoClientInfo);
        }
    }
    
    return  (NULL);
}


SmppClientInfo* clsSmppClient::SearchConnection(int aiGroupNo, int aiAccountId, int aiBindType)
{
    AI_FOR_EACH_IN_LIST(lpoClientInfo, &coClientHead, SmppClientInfo, coClientChild)
    {
        if (aiGroupNo == lpoClientInfo->ciGroupNo
            && aiAccountId == lpoClientInfo->ciAccountId
            && (aiBindType == 0 || aiBindType == lpoClientInfo->ciBindType))
        {
            return lpoClientInfo;
        }
    }

    return NULL;
}


int clsSmppClient::_AddConnection(const SmppConfig *apoClientInfo)
{
    //int             liHashKey;
    //char            lsString[64];
    SmppClientInfo*     lpoClientInfo;
    
    if (apoClientInfo->ciConnCount > 0)
    {
        AI_NEW_ASSERT(lpoClientInfo, SmppClientInfo);

        lpoClientInfo->ciGroupNo    = apoClientInfo->ciGroupNo;
        lpoClientInfo->ciAccountId  = apoClientInfo->ciAccountId;
        lpoClientInfo->ciIDFormat   = apoClientInfo->ciIDFormat;
        lpoClientInfo->ciBindType   = apoClientInfo->ciBindType;
        lpoClientInfo->ciConnCount  = apoClientInfo->ciConnCount;
        lpoClientInfo->ciRemotePort = apoClientInfo->ciRemotePort;
        lpoClientInfo->ciWeight     = apoClientInfo->ciWeight;

        if (lpoClientInfo->ciWeight <= 0)  
        {
            lpoClientInfo->ciWeight = SCT_DEFAULT_WEIGHT;
        }
 
        StringCopy(lpoClientInfo->csRemoteIp, apoClientInfo->csRemoteIp, sizeof(lpoClientInfo->csRemoteIp)); 
        StringCopy(lpoClientInfo->csSystemID, apoClientInfo->csSystemID, sizeof(lpoClientInfo->csSystemID)); 
        StringCopy(lpoClientInfo->csPassword, apoClientInfo->csPassword, sizeof(lpoClientInfo->csPassword)); 
        StringCopy(lpoClientInfo->csIDPrefix, apoClientInfo->csIDPrefix, sizeof(lpoClientInfo->csIDPrefix)); 
        
        lpoClientInfo->ciFlushFlag = SCT_FLUSH_NEWADD;

        ai_init_list_head(&(lpoClientInfo->coSocketHead));
        ai_list_add_tail(&(lpoClientInfo->coClientChild), &coClientHead);
       
        //sprintf(lsString, "%s%s", apoClientInfo->csRemoteIp, apoClientInfo->csSystemID);
        //liHashKey = GetStringHashKey1(lsString, SCT_MAX_HASH_SIZE);
        //ai_list_add_tail(&(lpoClientInfo->coIndexChild), &(coIndexHead[liHashKey]));
        
        return  (0);
    }

    return  (SCT_ERROR_INV_CONNNUM);
}


int clsSmppClient::AddConnection(const SmppConfig *apoClientInfo)
{
    int             liRetCode;
    SmppClientInfo  *lpoClientInfo;
    
    AISmartLock loSmartMutexLock(coMutexLock);

    lpoClientInfo = SearchConnection(
        apoClientInfo->csRemoteIp, 
        apoClientInfo->ciRemotePort, 
        apoClientInfo->csSystemID,
        apoClientInfo->ciBindType
    );

    if (lpoClientInfo)
    {
        AIWriteLOG(
            NULL,
            AILOG_LEVEL_ERROR, 
            "[%s] Duplicate %s@%s:%d|%X", 
            __FUNCTION__,
            apoClientInfo->csSystemID,
            apoClientInfo->csRemoteIp, 
            apoClientInfo->ciRemotePort,
            apoClientInfo->ciBindType
        );

        return  (SCT_ERROR_DUPCONNINFO);
    }
    
    liRetCode = _AddConnection(apoClientInfo);

    return  (liRetCode);
}


void clsSmppClient::CheckThread(SmppClientInfo *apoClientInfo)
{
AGAIN:
    apoClientInfo->coSocketHeadLock.Lock();

    AI_FOR_EACH_IN_LIST(lpoSocketInfo, &(apoClientInfo->coSocketHead), SmppSocketInfo, coSocketChild)
    {
        if (lpoSocketInfo->ctRecvThreadID == 0)
        {
            apoClientInfo->coSocketHeadLock.Unlock();
            sleep(1);
            
            goto  AGAIN;
        }
    }
    apoClientInfo->coSocketHeadLock.Unlock();
}


int clsSmppClient::RefreshConnection(SmppConfig *apoCltCfg, size_t aiArraySize)
{
    SmppConfig      *lpoSmppCltCfg = NULL;
    
    coMutexLock.Lock();

    AI_FOR_EACH_IN_LIST(lpoClientInfo, &coClientHead, SmppClientInfo, coClientChild)
    {
        lpoClientInfo->ciFlushFlag = SCT_FLUSH_INIT;
    }
    
    for(size_t liIt = 0; liIt < aiArraySize; liIt++)
    {
        lpoSmppCltCfg = &(apoCltCfg[liIt]);

        SmppClientInfo* lpoClientInfo = SearchConnection(
            lpoSmppCltCfg->csRemoteIp,
            lpoSmppCltCfg->ciRemotePort, 
            lpoSmppCltCfg->csSystemID,
            lpoSmppCltCfg->ciBindType
        );
        
        if (NULL != lpoClientInfo)
        { // FIXME: need locking ?
            lpoClientInfo->ciFlushFlag  = SCT_FLUSH_EXIST;
            lpoClientInfo->ciVendorType = lpoSmppCltCfg->ciVendorType;
            lpoClientInfo->ciIDFormat   = lpoSmppCltCfg->ciIDFormat;
            lpoClientInfo->ciWeight     = lpoSmppCltCfg->ciWeight;
            lpoClientInfo->ciBindType   = lpoSmppCltCfg->ciBindType;
            lpoClientInfo->ciRemotePort = lpoSmppCltCfg->ciRemotePort;
            _StringCopy(lpoClientInfo->csRemoteIp, lpoSmppCltCfg->csRemoteIp);
            _StringCopy(lpoClientInfo->csSystemID, lpoSmppCltCfg->csSystemID);
            _StringCopy(lpoClientInfo->csPassword, lpoSmppCltCfg->csPassword);
            _StringCopy(lpoClientInfo->csIDPrefix, lpoSmppCltCfg->csIDPrefix);

            AjustConnections(lpoClientInfo, lpoSmppCltCfg->ciConnCount);
        }
        else
        {
            _AddConnection(lpoSmppCltCfg);
        }
    }

DELAGAIN:
    AI_FOR_EACH_IN_LIST(lpoClientInfo, &coClientHead, SmppClientInfo, coClientChild)
    {
        if (lpoClientInfo->ciFlushFlag == SCT_FLUSH_INIT)
        {
            ai_list_del_any(&(lpoClientInfo->coClientChild));
            coMutexLock.Unlock();
            
            ShutDown(lpoClientInfo);
            AI_DELETE(lpoClientInfo);
            
            coMutexLock.Lock();
            goto  DELAGAIN;
        }
    }
        
    AI_FOR_EACH_IN_LIST(lpoClientInfo, &coClientHead, SmppClientInfo, coClientChild)
    {
        if (lpoClientInfo->ciFlushFlag == SCT_FLUSH_NEWADD)
        {
            CreateThread(lpoClientInfo);
            CheckThread(lpoClientInfo);
        }
    }
        
    coMutexLock.Unlock();
    return  (0);
}


void clsSmppClient::SetRouteCallback(smppGetRouteCallback apfCallback)
{
    cpfGetRoute = apfCallback;
}


void clsSmppClient::SetDeliverCallback(smppDeliverCallback apfCallback)
{
    cpfDeliverCallback = apfCallback;
}


void clsSmppClient::SetSubmitRespCallback(smppSubmitRespCallback apfCallback)
{
    cpfSubmitRespCallback = apfCallback;
}


void clsSmppClient::SetStatReportCallback(smppStatReportCallback apfCallback)
{
    cpfStatReportCallback = apfCallback;
}


void clsSmppClient::SetConnectionBreak(smppConnBreakCallback apfCallback)
{
    cpfConnBreakCallback = apfCallback;
}


void clsSmppClient::SetConnectionLogin(smppConnLoginCallback apfCallback)
{
    cpfConnLoginCallback = apfCallback;
}


int clsSmppClient::GetConnectionCount(SmppClientInfo *apoClientInfo)
{
    int             liCount = 0;

    AISmartLock loSmartSocketHeadLock(apoClientInfo->coSocketHeadLock);

    AI_FOR_EACH_IN_LIST(lpoSocketInfo, &(apoClientInfo->coSocketHead), SmppSocketInfo, coSocketChild)
    {
        if (lpoSocketInfo->ciSocket >= 0)
        {
            liCount += 1;
        }
    }
    
    return  (liCount);
}


SmppSocketInfo *clsSmppClient::GetConnection(SmppRouteInfo const* apoRouteInfo, bool abIsSplit, void* apExtraArg)
{
    SmppClientInfo* lpoClientInfoEx = NULL;
    size_t          liAvailable = 0;
    size_t          liFull = 0;
    SmppClientInfo* lpoClientInfoArray[SCT_MAX_ACC_COUNT];
    int             liSMCSeq;

    liSMCSeq = cpfGetRoute(apoRouteInfo, apExtraArg);

    if (AI_SMPP_ROUTE_FAILURE == liSMCSeq) 
    {
        AIWriteLOG(
            NULL,
            AILOG_LEVEL_ERROR, 
            "[%s] cpfGetRoute(%s->%s/%s/%d) ... Fail", 
            __FUNCTION__,
            apoRouteInfo->csSrcTermID, 
            apoRouteInfo->csDestTermID,
            apoRouteInfo->csServiceID,
            apoRouteInfo->ciBindType
        );
        return  (NULL);
    }
    
    AISmartLock loSmartMutexLock(coMutexLock);

    if (AI_SMPP_ROUTE_AUTO == liSMCSeq)
    {
        AI_FOR_EACH_IN_LIST(lpoClientInfo, &coClientHead, SmppClientInfo, coClientChild)
        {
            assert(lpoClientInfo);
           
            if (AI_SMPP_CMD_BIND_TRANSMITTER != lpoClientInfo->ciBindType)
            {
                continue;
            }

            if (((intptr_t)apExtraArg) != lpoClientInfo->ciGroupNo)
            {
                continue;
            }

            if (GetConnectionCount(lpoClientInfo) <= 0)
            {
                continue;
            }

            if (abIsSplit)
            {
                lpoClientInfoEx = lpoClientInfo;
                goto END_SEARCH;
            }

            if (lpoClientInfo->ciSendCount >= lpoClientInfo->ciWeight)
            {
                ++liFull;
            }
            else if (NULL == lpoClientInfoEx || lpoClientInfo->ciSendCount < lpoClientInfoEx->ciSendCount)
            {
                lpoClientInfoEx = lpoClientInfo;
            }

            lpoClientInfoArray[liAvailable++] = lpoClientInfo;

            if (liAvailable >= SCT_MAX_ACC_COUNT)
            {
                break;
            }
        }

        if (liFull == liAvailable && liAvailable > 0)
        {
            for (size_t i = 0; i < liAvailable; ++i)
            {
                lpoClientInfoArray[i]->ciSendCount = 0;
            }

            lpoClientInfoEx = lpoClientInfoArray[0];
        }
    }
    else
    {
        lpoClientInfoEx = SearchConnection(
                (int)apExtraArg,
                liSMCSeq
        );
    }

END_SEARCH:
    if (NULL == lpoClientInfoEx || AI_SMPP_CMD_BIND_TRANSMITTER != lpoClientInfoEx->ciBindType)
    {
        AIWriteLOG(
            NULL,
            AILOG_LEVEL_ERROR, 
            "[%s] SearchConnection(%s->%s/%s/0x%X) ... Fail",
            __FUNCTION__,
            apoRouteInfo->csSrcTermID,
            apoRouteInfo->csDestTermID, 
            apoRouteInfo->csServiceID,
            apoRouteInfo->ciBindType
        );   
        
        return  (NULL);
    }

    lpoClientInfoEx->ciSendCount += 1;

    AISmartLock loSmartSocketHeadLock(lpoClientInfoEx->coSocketHeadLock);

    AI_FOR_EACH_IN_LIST(lpoSocketInfo, &(lpoClientInfoEx->coSocketHead), SmppSocketInfo, coSocketChild)
    {
        if (lpoSocketInfo->ciSocket >= 0)
        {   
            ai_list_del_any(&(lpoSocketInfo->coSocketChild));
            ai_list_add_tail(&(lpoSocketInfo->coSocketChild), &(lpoClientInfoEx->coSocketHead));

            lpoSocketInfo->ciExUsed += 1;

            return  (lpoSocketInfo);
        }
    }
    
    return  (NULL);
}


void clsSmppClient::FreeConnection(const SmppSocketInfo *apoSocketInfo)
{
    AISmartLock loSmartSocketHeadLock(apoSocketInfo->cpoClientInfo->coSocketHeadLock);

    AI_FOR_EACH_IN_LIST(lpoSocketInfo, &(apoSocketInfo->cpoClientInfo->coSocketHead), SmppSocketInfo, coSocketChild)
    {
        if (lpoSocketInfo == apoSocketInfo)
        {
            lpoSocketInfo->ciExUsed -= 1;
            break;
        }       
    }
}


int clsSmppClient::DealBIND(int aiSocket, SmppClientInfo *apoClientInfo)
{
    ssize_t         liPduSize;
    int             liRetSize;
    char            lsBuffer[512];
    SMPP_BIND       loSmppBind;
    SMPP_BIND_RESP  loSmppBindResp;
  
    // bind
    loSmppBind.coHeader.command_id      = apoClientInfo->ciBindType;
    loSmppBind.coHeader.command_status  = 0;
    loSmppBind.coHeader.sequence_number = 0x7FFFFFFF;
  
    _StringCopy(loSmppBind.system_id, apoClientInfo->csSystemID);
    _StringCopy(loSmppBind.password, apoClientInfo->csPassword);
    _StringCopy(loSmppBind.system_type, "");
    loSmppBind.interface_version = 0x34;
    loSmppBind.addr_ton = 0x02;
    loSmppBind.addr_npi = 0x08;
    _StringCopy(loSmppBind.address_range, "");
 
    liPduSize = loSmppBind.Encode(lsBuffer, sizeof(lsBuffer));
    
    AI_RETURN_IF(liPduSize, (liPduSize < 0));
   
    // send bind   
    liRetSize = _SendSMPPPacket(aiSocket, &loSmppBind.coHeader, lsBuffer, liPduSize);
    _CLOSE_RETURN_IF(SCT_ERROR_SENDFAIL, (liRetSize < 0), aiSocket);
  
    // recv bind resp 
    liRetSize = _RecvSMPPPacket(aiSocket, &loSmppBindResp.coHeader, lsBuffer, sizeof(lsBuffer));
    _CLOSE_RETURN_IF(SCT_ERROR_RECVFAIL, (liRetSize <= 0), aiSocket);

    liRetSize = loSmppBindResp.Decode(lsBuffer, liRetSize - 16);
    _CLOSE_RETURN_IF(-1, (liRetSize  < 0), aiSocket);

    if (loSmppBind.coHeader.sequence_number == loSmppBindResp.coHeader.sequence_number 
        && 0 == loSmppBindResp.coHeader.command_status
        && loSmppBindResp.coHeader.command_id == (apoClientInfo->ciBindType | 0x80000000)) 
    {
        if (AI_SMPP_CMD_BIND_TRANSMITTER == apoClientInfo->ciBindType)
        {
            AIWriteLOG(
                NULL,
                AILOG_LEVEL_DEBUG, 
                "%s: BIND_TRANSMITTER ... Ok", 
                __FUNCTION__
            );
            return  (0);
        }
        else if (AI_SMPP_CMD_BIND_RECEIVER == apoClientInfo->ciBindType)
        {
            AIWriteLOG(
                NULL,
                AILOG_LEVEL_DEBUG, 
                "[%s] BIND_RECEIVER ... Ok", 
                __FUNCTION__
            );
            return  (0);
        }
    }         

    ai_socket_close(aiSocket);
    return  (SCT_ERROR_TYPESTATUS);
}


int clsSmppClient::DealDELIVER(SmppSocketInfo *apoSocketInfo, SMPP_HEADER* apoHead, void const* apcBuffer, size_t aiLen)
{
    SmppClientInfo*     lpoClientInfo;
    SMPP_DELIVER_SM     loDeliverSM;
    ssize_t             liRetSize;
     
    if (cpfDeliverCallback == NULL)
    {
        AIWriteLOG(
            NULL,
            AILOG_LEVEL_ERROR, 
            "[%s] Invlaid cpfDeliverCallback",
            __FUNCTION__
        );
        return  (SCT_ERROR_INV_DCALLBAK);
    }
    
    lpoClientInfo = apoSocketInfo->cpoClientInfo;

    memset(&loDeliverSM, 0, sizeof(SMPP_DELIVER_SM));
    memcpy(&(loDeliverSM.coHeader), apoHead, sizeof(SMPP_HEADER));

    liRetSize = loDeliverSM.Decode(apcBuffer, aiLen);
    if (liRetSize < 0)
    {
        AIWriteLOG(
            NULL,
            AILOG_LEVEL_ERROR, 
            "[%s] DELIVER_SM decode error, offset=%ld",
            __FUNCTION__,
            (long)-liRetSize
        );
        return SCT_ERROR_INV_DCALLBAK;
    }
    
    SMPP_DELIVER_SM*    lpoDeliverSM;
    AI_NEW_ASSERT(lpoDeliverSM, SMPP_DELIVER_SM);
    memcpy(lpoDeliverSM, &loDeliverSM, sizeof(SMPP_DELIVER_SM));
  
    if (apoSocketInfo->cpoClientInfo->ciVendorType != 0 && loDeliverSM.data_coding == 0) 
    { // nokia default coding
        int liSize = CheckConcatenated(loDeliverSM.short_message, loDeliverSM.esm_class);

        DefaultToASCII(
            loDeliverSM.short_message + liSize, 
            loDeliverSM.sm_length - liSize,
            lpoDeliverSM->short_message + liSize
        );
    }
     
    lpoClientInfo->coSocketHeadLock.Lock();
    apoSocketInfo->ciExUsed += 1;
    lpoClientInfo->coSocketHeadLock.Unlock();
  
    return  cpfDeliverCallback(apoSocketInfo, lpoDeliverSM);
}


int clsSmppClient::DealSubmitRESP(SmppSocketInfo *apoSocketInfo, SMPP_HEADER* apoHead, void const* apcBuffer, size_t aiLen)
{
    SmppClientInfo*         lpoClientInfo;
    SMPP_SUBMIT_SM_RESP     loSubmitResp;
    unsigned long long      liMsgId;
    ssize_t                 liRetSize;

    if (cpfSubmitRespCallback == NULL)
    {
        AIWriteLOG(
            NULL,
            AILOG_LEVEL_ERROR, 
            "[%s] Invlaid cpfSubmitRespCallback", 
            __FUNCTION__
        );
        return  (SCT_ERROR_INV_RCALLBAK);
    }

    lpoClientInfo = apoSocketInfo->cpoClientInfo;

    memcpy(&(loSubmitResp.coHeader), apoHead, sizeof(SMPP_HEADER));
  
    liRetSize = loSubmitResp.Decode(apcBuffer, aiLen);
    if (liRetSize < 0)
    {
        AIWriteLOG(
            NULL,
            AILOG_LEVEL_ERROR, 
            "[%s] SUBMIT_SM_RESP decode error, offset=%ld",
            __FUNCTION__,
            (long)-liRetSize
        );
        return -1;
    }

    SMPP_SUBMIT_SM_RESP*    lpoSubmitResp;
    AI_NEW_ASSERT(lpoSubmitResp, SMPP_SUBMIT_SM_RESP);
    memcpy(lpoSubmitResp, &loSubmitResp, sizeof(*lpoSubmitResp));

    int liBase = (lpoClientInfo->ciIDFormat & 0xFF00) >> 8;
    if (liBase >= 2 && liBase <= 36)
    {
        liMsgId = strtoull(loSubmitResp.message_id, NULL, liBase);
        snprintf(
            lpoSubmitResp->message_id,
            sizeof(lpoSubmitResp->message_id),
            "%s.%llu",
            lpoClientInfo->csIDPrefix,
            liMsgId
        );
    }
    else
    {
        snprintf(
            lpoSubmitResp->message_id,
            sizeof(lpoSubmitResp->message_id),
            "%s.%s",
            lpoClientInfo->csIDPrefix,
            loSubmitResp.message_id 
        );
    }

    lpoClientInfo->coSocketHeadLock.Lock();
    apoSocketInfo->ciExUsed += 1;
    lpoClientInfo->coSocketHeadLock.Unlock();
   
    return  cpfSubmitRespCallback(apoSocketInfo, lpoSubmitResp);
}


int clsSmppClient::DealStatREPORT(SmppSocketInfo *apoSocketInfo, SMPP_HEADER* apoHead, void const* apcBuffer, size_t aiLen)
{
    SmppClientInfo*     lpoClientInfo;
    SMPP_DELIVER_SM     loDeliverSM;
    ssize_t             liRetSize;
        
    if (cpfStatReportCallback == NULL)
    {
        AIWriteLOG(
            NULL,
            AILOG_LEVEL_ERROR,
            "[%s] Invlaid cpfStatReportCallback", 
            __FUNCTION__
        );
        return  (SCT_ERROR_INV_SCALLBAK);
    }
    
    lpoClientInfo = apoSocketInfo->cpoClientInfo;

    memset(&loDeliverSM, 0, sizeof(SMPP_DELIVER_SM));
    memcpy(&(loDeliverSM.coHeader), apoHead, sizeof(loDeliverSM.coHeader));

    liRetSize = loDeliverSM.Decode(apcBuffer, aiLen);
    if (liRetSize < 0)
    {
        AIWriteLOG(
            NULL,
            AILOG_LEVEL_ERROR, 
            "[%s] DELIVER_SM(StatusReport) decode error, offset=%ld",
            __FUNCTION__,
            (long)-liRetSize
        );
        return SCT_ERROR_INV_SCALLBAK;
    }

    SMPP_DELIVER_SM*    lpoDeliverSM;
    AI_NEW_ASSERT(lpoDeliverSM, SMPP_DELIVER_SM);
    memcpy(lpoDeliverSM, &loDeliverSM, sizeof(SMPP_DELIVER_SM));

    if (apoSocketInfo->cpoClientInfo->ciVendorType != 0 && loDeliverSM.data_coding == 0) 
    { // nokia default coding
        DefaultToASCII(
            loDeliverSM.short_message, 
            loDeliverSM.sm_length,
            lpoDeliverSM->short_message
        );
    }

    StatREPORT*     lpoStatReport;
    AI_NEW_ASSERT(lpoStatReport, StatREPORT);
    
    FormatStatReport(apoSocketInfo, lpoDeliverSM->short_message, lpoStatReport);

    lpoClientInfo->coSocketHeadLock.Lock();
    apoSocketInfo->ciExUsed += 1;
    lpoClientInfo->coSocketHeadLock.Unlock();

    return  cpfStatReportCallback(apoSocketInfo, lpoDeliverSM, lpoStatReport);
}


int clsSmppClient::GetValueOfStatReport(const char *apcData, const char *apcLabel, char *apcRetValue, const int aiMaxSize)
{
    int         liMaxSize = 0;
    char*       lpcPtr1 = NULL;
    char*       lpcPtr2 = NULL;

    if ((lpcPtr1 = (char*)strstr(apcData, apcLabel)) == NULL)
    {
        apcRetValue[0] = '\0';
        return  (-1);
    }

    lpcPtr1 = lpcPtr1 + strlen(apcLabel);
    while(*lpcPtr1 == ' ')  lpcPtr1 += 1;

    if ((lpcPtr2 = strchr(lpcPtr1, ' ')) == NULL)
        lpcPtr2 = lpcPtr1 + strlen(lpcPtr1);

    liMaxSize = (lpcPtr2 - lpcPtr1) + 1;
    if (liMaxSize > aiMaxSize)  liMaxSize = aiMaxSize;

    strncpy(apcRetValue, lpcPtr1, liMaxSize-1);
    apcRetValue[ liMaxSize-1 ] = (char)0;
    
    return  (liMaxSize-1);
}


int clsSmppClient::ExtractNormalStatReport(const SmppSocketInfo *apoSocketInfo, const char *apcContent, StatREPORT *apoStat)
{   
    unsigned long long  liMsgID = 0;
    char                lsNewMsgID[128];
    SmppClientInfo*     lpoClientInfo;

    GetValueOfStatReport(apcContent, "err:", apoStat->csErr, sizeof(apoStat->csErr));
    GetValueOfStatReport(apcContent, "sub:", apoStat->csSub, sizeof(apoStat->csSub));
    GetValueOfStatReport(apcContent, "id:", apoStat->csMsgID, sizeof(apoStat->csMsgID));
    GetValueOfStatReport(apcContent, "stat:", apoStat->csStat, sizeof(apoStat->csStat));
    GetValueOfStatReport(apcContent, "dlvrd:", apoStat->csDlvrd, sizeof(apoStat->csDlvrd));
    GetValueOfStatReport(apcContent, "done date:", apoStat->csDoneTime, sizeof(apoStat->csDoneTime));
    GetValueOfStatReport(apcContent, "submit date:", apoStat->csSubmitTime, sizeof(apoStat->csSubmitTime));

    lpoClientInfo = apoSocketInfo->cpoClientInfo;

    int liBase = lpoClientInfo->ciIDFormat & 0xFF;
    if (liBase >= 2 && liBase <= 36)
    {
        liMsgID = strtoull(apoStat->csMsgID, NULL, liBase);
        snprintf(apoStat->csMsgID, sizeof(apoStat->csMsgID), "%s.%llu", lpoClientInfo->csIDPrefix, liMsgID);
    }
    else
    {
        snprintf(lsNewMsgID, sizeof(lsNewMsgID), "%s.%s", lpoClientInfo->csIDPrefix, apoStat->csMsgID);
        _StringCopy(apoStat->csMsgID, lsNewMsgID);
    }

    return  (0);
}


int clsSmppClient::ExtractEspecialStatReport(const SmppSocketInfo *apoSocketInfo, const char *apcContent, StatREPORT *apoStat)
{
    unsigned long long  liMsgID = 0;
    char*               lpcPtr = NULL;
    char*               lpcEnd = NULL;
    char                lsNewMsgID[128];
    SmppClientInfo*     lpoClientInfo;
    int                 liStat;
     
    lpcPtr = (char *)apcContent;

    strncpy(apoStat->csMsgID, lpcPtr, 10);
    lpcPtr += 10;
    strncpy(apoStat->csSub, lpcPtr, 3);
    lpcPtr += 3;
    strncpy(apoStat->csDlvrd, lpcPtr, 3);
    lpcPtr += 3;
    strncpy(apoStat->csSubmitTime, lpcPtr, 10);
    lpcPtr += 10;
    strncpy(apoStat->csDoneTime, lpcPtr, 10);
    lpcPtr += 10;
    strncpy(apoStat->csStat, lpcPtr, 7);
    lpcPtr += 7;
    strncpy(apoStat->csErr, lpcPtr, 3);
    lpcPtr += 3;

    errno = 0;
    liStat = strtol(apoStat->csStat, &lpcEnd, 10);

    if (NULL != lpcEnd && apoStat->csStat != lpcEnd)
    {
        switch(liStat)
        {
            case 2:
                _StringCopy(apoStat->csStat, AI_SMPP_STAT_ACCEPTD);
                break;

            case 3:
                _StringCopy(apoStat->csStat, AI_SMPP_STAT_EXPIRED);
                break;

            case 4:
                _StringCopy(apoStat->csStat, AI_SMPP_STAT_DELETED);
                break;

            case 5:
                _StringCopy(apoStat->csStat, AI_SMPP_STAT_UNDELIV);
                break;

            case 6:
                _StringCopy(apoStat->csStat, AI_SMPP_STAT_DELIVRD);
                break;

            default:
                _StringCopy(apoStat->csStat, AI_SMPP_STAT_UNDELIV);
                break;  
        }
    }

    lpoClientInfo = apoSocketInfo->cpoClientInfo;

    int liBase = lpoClientInfo->ciIDFormat & 0xFF ;
    if (liBase >= 2 && liBase <= 36)
    {
        liMsgID = strtoull(apoStat->csMsgID, NULL, liBase);
        snprintf(apoStat->csMsgID, sizeof(apoStat->csMsgID), "%s.%llu", lpoClientInfo->csIDPrefix, liMsgID);
    }
    else
    {
        snprintf(lsNewMsgID, sizeof(lsNewMsgID), "%s.%s", lpoClientInfo->csIDPrefix, apoStat->csMsgID);
        _StringCopy(apoStat->csMsgID, lsNewMsgID);
    }
    
    return  (0);
}


int clsSmppClient::FormatStatReport(const SmppSocketInfo *apoSocketInfo, const char *apcContent, StatREPORT *apoStat)
{
    memset(apoStat, 0, sizeof(StatREPORT));

    if (strstr(apcContent, "id:") && strstr(apcContent, "stat:"))
    {
        return ExtractNormalStatReport(apoSocketInfo, apcContent, apoStat);
    }

    return ExtractEspecialStatReport(apoSocketInfo, apcContent, apoStat);
}


int clsSmppClient::CheckStatReport(char *apcData, size_t aiSize)
{    
    SMPP_DELIVER_SM loDeliver;        
    
    if (loDeliver.Decode(apcData, aiSize) < 0)    
    {        
        return -1;    
    }    

    return  (loDeliver.esm_class & SCT_MASK_STATREPORT) ? 1 : 0;
}


int clsSmppClient::GetUdhiFlag(char *apcData, size_t aiSize)
{    
    SMPP_DELIVER_SM loDeliver;        
    
    if (loDeliver.Decode(apcData, aiSize) < 0)    
    {        
        return -1;    
    }    
    
    return  (loDeliver.esm_class & SCT_MASK_UDHIFLAG) ? 1 : 0;
}

///end namespace
AIBC_NAMESPACE_END
