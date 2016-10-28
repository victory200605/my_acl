#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AILib.h"
#include "AITime.h"
#include "AISocket.h"
#include "AILogSys.h"
#include "AICrypt.h"
#include "AICMPPStru.h"
#include "AICMPPLayer.h"

///start namespace
AIBC_NAMESPACE_START

int RecvCMPPPacket(int aiSocket, CMPPHead* apoCMPPHead, AIChunk* apoAIChunk, int aiTimeout)
{
    size_t      liBodySize;
    ssize_t     liRetCode = 0;

    memset(apoCMPPHead, 0, sizeof(CMPPHead));
    liRetCode = ai_socket_recvdata(aiSocket, (char*)apoCMPPHead, CMPP_HEADER_LEN, aiTimeout);

    if(CMPP_HEADER_LEN != liRetCode)
    {
        if(liRetCode < 0)
        {
            AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "Receive CMPP_HEADER ... fail");
            return  (liRetCode);
        }
        else if(0 == liRetCode)
        {
            return  (ERROR_CL_SOCKET_TIMEOUT);
        }
        else
        {
            return  (ERROR_CL_RECVDATA);
        }    
    }
    
    apoCMPPHead->ciTotalLength = ntohl(apoCMPPHead->ciTotalLength);
    apoCMPPHead->ciCommandID = ntohl(apoCMPPHead->ciCommandID);
    apoCMPPHead->ciInvokeID = ntohl(apoCMPPHead->ciInvokeID);

    if((apoCMPPHead->ciTotalLength < CMPP_HEADER_LEN) || (apoCMPPHead->ciTotalLength > CL_MAX_BODY_LENGTH))
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "Invlid CMPP_HEADER.ciTotalLength = %d -> [%d,%d]", 
            apoCMPPHead->ciTotalLength,
            CMPP_HEADER_LEN,
            CL_MAX_BODY_LENGTH);

        return  (ERROR_CL_INV_LENGTH);
    }

    liBodySize = apoCMPPHead->ciTotalLength - CMPP_HEADER_LEN;
    if(liBodySize > 0)
    {
        if(apoAIChunk->GetSize() < (size_t)liBodySize)
        {
            apoAIChunk->Resize(liBodySize);
        }
        
        liRetCode = ai_socket_recvdata(aiSocket, apoAIChunk->GetPointer(), liBodySize, aiTimeout);

        if((size_t)liRetCode != liBodySize)
        {
            AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "Receive CMPP_BODY ... Fail, body size=%d, return code=%d",
                liBodySize,
                liRetCode);

            return  (ERROR_CL_RECVDATA);
        }
    }
    
    return  (0);//(apoCMPPHead->ciTotalLength);
}

int SendCMPPPacket(int aiSocket, CMPPHead const* apoCMPPHead, void const* apvBody, int aiTimeout)
{
    int         liRetCode = 0;
    size_t      liSendSize = 0;
    char*       lpcSendBuf = NULL;
    CMPPHead*   lpoCMPPHead = NULL;

    liSendSize = apoCMPPHead->ciTotalLength;
    AI_NEW_N_ASSERT(lpcSendBuf, char, liSendSize);
    lpoCMPPHead = (CMPPHead*)lpcSendBuf;
    
    lpoCMPPHead->ciTotalLength = htonl(apoCMPPHead->ciTotalLength);
    lpoCMPPHead->ciCommandID = htonl(apoCMPPHead->ciCommandID);
    lpoCMPPHead->ciInvokeID = htonl(apoCMPPHead->ciInvokeID);

    if(liSendSize > CMPP_HEADER_LEN)
    {
        if(apvBody != NULL)
        {
            memcpy(lpcSendBuf+CMPP_HEADER_LEN, apvBody, liSendSize-CMPP_HEADER_LEN);
        }
        else
        {
            memset(lpcSendBuf+CMPP_HEADER_LEN, 0, liSendSize-CMPP_HEADER_LEN);
        }
    }

    liRetCode = ai_socket_senddata(aiSocket, lpcSendBuf, liSendSize, aiTimeout);
    AI_DELETE_N(lpcSendBuf);

    return  ((size_t)liRetCode == liSendSize ? 0 : ERROR_CL_SENDDATA);
}

int LoginCmpp20Gateway(int aiSocket, char const* apcID, char const* apcPasswd, int aiVer, int aiTimeout)
{
    int             liSize;
    int             liTime = 0;
    unsigned char   liResult = 0;
    char            lsKey[128] = {0,};
    char            lcVersion = aiVer;
    char            lsTimeStamp[24] = {0};
    char            lsAuthenticator[24];
    char            lsSendBuffer[128];
    AIChunk         loRecvChunk;
    CMPPHead        lstCMPPHead;
    
    memset(lsKey, 0, sizeof(lsKey));

    GetCurShortTime(lsTimeStamp);
    liTime = htonl(atoi(lsTimeStamp+4));
    
    strcpy(lsKey, apcID);
    liSize = strlen(apcID) + 9;
    
    strcpy(lsKey+liSize, apcPasswd);
    liSize = liSize + strlen(apcPasswd);
    
    strcpy(lsKey+liSize, lsTimeStamp+4);
    liSize = liSize + strlen(lsTimeStamp+4);

    MD5Encode(lsKey, liSize, lsAuthenticator);

    memcpy(lsSendBuffer, apcID, 6);
    memcpy(lsSendBuffer + 6, lsAuthenticator, 16);
    memcpy(lsSendBuffer + 22, &lcVersion, 1);
    memcpy(lsSendBuffer + 23, &liTime, 4);

    lstCMPPHead.ciTotalLength = CMPP_HEADER_LEN+CMPP20_CONNECT_REQ_BODY_LEN;
    lstCMPPHead.ciCommandID = CMPP_CONNECT;
    lstCMPPHead.ciInvokeID = 0;
    
    if(SendCMPPPacket(aiSocket, &lstCMPPHead, lsSendBuffer, aiTimeout) < 0)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Corporation(%s) send CMPP_CONNECT fail",
            __FUNCTION__,
            apcID);

        return  (ERROR_CL_SENDDATA);
    }
    
    if(RecvCMPPPacket(aiSocket, &lstCMPPHead, &loRecvChunk, aiTimeout) < 0)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Corporation(%s) recv CMPP_CONNECT_RESP fail",
            __FUNCTION__,
            apcID);

        return  (ERROR_CL_RECVDATA);
    }
    
    if(lstCMPPHead.ciCommandID != CMPP_CONNECT_REP)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Corporation(%s) recv Invalid CommandID(0x%08x)", 
            __FUNCTION__, 
            apcID, 
            lstCMPPHead.ciCommandID);

        return  (ERROR_CL_INV_COMMAND);
    }
    
    if(lstCMPPHead.ciTotalLength != CMPP_HEADER_LEN+CMPP20_CONNECT_REP_BODY_LEN)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Corporation(%s) recv Invalid packet length(%d)",
            __FUNCTION__,
            apcID,
            lstCMPPHead.ciTotalLength);

        return  (ERROR_CL_INV_LENGTH);
    }
    
    memcpy(&liResult, loRecvChunk.GetPointer(), 1);
    if(liResult)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Corporation(%s) recv CMPP_CONNECT_RESP.Result(%d)", 
            __FUNCTION__,
            apcID,
            (int)liResult);
        return  (liResult);
    }
    
    return  (0);
}

int LoginCmpp30Gateway(int aiSocket, char const* apcID, char const* apcPasswd, int aiVer, int aiTimeout)
{
    int             liSize;
    int             liTime = 0;
    unsigned int    liResult = 0;
    char            lsKey[128] = {0,};
    char            lcVersion = aiVer;
    char            lsTimeStamp[24] = {0};
    char            lsAuthenticator[24];
    char            lsSendBuffer[128];
    AIChunk         loRecvChunk;
    CMPPHead        lstCMPPHead;
    
    memset(lsKey, 0, sizeof(lsKey));

    GetCurShortTime(lsTimeStamp);
    liTime = htonl(atoi(lsTimeStamp+4));
    
    strcpy(lsKey, apcID);
    liSize = strlen(apcID) + 9;
    
    strcpy(lsKey+liSize, apcPasswd);
    liSize = liSize + strlen(apcPasswd);
    
    strcpy(lsKey+liSize, lsTimeStamp+4);
    liSize = liSize + strlen(lsTimeStamp+4);

    MD5Encode(lsKey, liSize, lsAuthenticator);

    memcpy(lsSendBuffer, apcID, 6);
    memcpy(lsSendBuffer + 6, lsAuthenticator, 16);
    memcpy(lsSendBuffer + 22, &lcVersion, 1);
    memcpy(lsSendBuffer + 23, &liTime, 4);

    lstCMPPHead.ciTotalLength = CMPP_HEADER_LEN+CMPP30_CONNECT_REQ_BODY_LEN;
    lstCMPPHead.ciCommandID = CMPP_CONNECT;
    lstCMPPHead.ciInvokeID = 0;
    
    if(SendCMPPPacket(aiSocket, &lstCMPPHead, lsSendBuffer, aiTimeout) < 0)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Corporation(%s) send CMPP_CONNECT fail", 
            __FUNCTION__,
             apcID);

        return  (ERROR_CL_SENDDATA);
    }
    
    if(RecvCMPPPacket(aiSocket, &lstCMPPHead, &loRecvChunk, aiTimeout) < 0)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Corporation(%s) recv CMPP_CONNECT_RESP fail", 
            __FUNCTION__,
            apcID);
        return  (ERROR_CL_RECVDATA);
    }
    
    if(lstCMPPHead.ciCommandID != CMPP_CONNECT_REP)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Corporation(%s) recv Invalid CommandID(0x%08x)", 
            __FUNCTION__,
            apcID,
            lstCMPPHead.ciCommandID);
        return  (ERROR_CL_INV_COMMAND);
    }
    
    if(lstCMPPHead.ciTotalLength != CMPP_HEADER_LEN+CMPP30_CONNECT_REP_BODY_LEN)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Corporation(%s) recv Invalid packet length(%d)", 
            __FUNCTION__,
            apcID,
            lstCMPPHead.ciTotalLength);

        return  (ERROR_CL_INV_LENGTH);
    }
    
    memcpy(&liResult, loRecvChunk.GetPointer(), 4);
    liResult = ntohl(liResult);
    
    if(liResult)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Corporation(%s) recv CMPP_CONNECT_RESP.Result(%u)", 
            __FUNCTION__,
            apcID,
            liResult);
        return  (liResult);
    }
    
    return  (0);
}

int CheckCMPP20_SUBMIT(CMPPHead* apoCMPPHead, char* apcBody, CMPPSubmit* apoSubmit)
{
    int                 liTrueSize = 0;
    int                 liMsgLength = 0;
    int                 liNodeCount = 0;
    char*               lpcNextPtr = NULL;
    uint8_t             lcTempValue = 0;
    
    assert(apoSubmit);
    
    lpcNextPtr = apcBody;
    memset(&apoSubmit, 0, sizeof(CMPPSubmit));
    liTrueSize = apoCMPPHead->ciTotalLength - CMPP_HEADER_LEN;
    
    if(liTrueSize < CMPP20_SUBMIT_MIN_BODY_LEN)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Invalid Body Size = %d < %d",
            __FUNCTION__, liTrueSize, CMPP20_SUBMIT_MIN_BODY_LEN);
        return  (ERROR_CODE_INVALID_PACKET);
    }

    memcpy(&lcTempValue, lpcNextPtr + 116, 1 );
    apoSubmit->ciDestUsrTL = lcTempValue;
    liNodeCount = (lcTempValue & 0xFF);
    
    if(liNodeCount <= 0 || liNodeCount > CMPP_MAX_DEST_USR_COUNT)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Invalid DestUsr_Tl = %d",
            __FUNCTION__,
             liNodeCount);

        return  (ERROR_CODE_DEST_USR_TL_FAIL);
    }
    
    if(liTrueSize < (CMPP20_SUBMIT_MIN_BODY_LEN + (liNodeCount-1)*21))
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Invalid Body Size = %d/%d",
            __FUNCTION__,
            liTrueSize,
             liNodeCount);

        return  (ERROR_CODE_DEST_USR_TL_FAIL);
    }
    
    lpcNextPtr = (apcBody + 117 + (liNodeCount*21));
    memcpy(&lcTempValue, lpcNextPtr, 1);
    apoSubmit->ciMsgLength = lcTempValue;
    liMsgLength = (lcTempValue & 0xFF);
    
    if(liMsgLength < 0 || liMsgLength > MAX_CONTENT_LEN)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Invalid MsgLength = %d",
            __FUNCTION__,
            liMsgLength);

        return  (ERROR_CODE_INVALID_LEN);
    }
    
    memcpy(&(apoSubmit->ciPkTotal),            apcBody +   8, 1 );
    memcpy(&(apoSubmit->ciPkNumber),           apcBody +   9, 1 );
    memcpy(&(apoSubmit->ciRegisteredDelivery), apcBody +  10, 1 );
    memcpy(&(apoSubmit->ciMsgLevel),           apcBody +  11, 1 );
    memcpy(&(apoSubmit->csServiceID),          apcBody +  12, 10);
    memcpy(&(apoSubmit->ciFeeUserType),        apcBody +  22, 1 );
    memcpy(&(apoSubmit->csFeeTerminalID),      apcBody +  23, 21);
    memcpy(&(apoSubmit->ciTpPID),              apcBody +  44, 1 );
    memcpy(&(apoSubmit->ciTpUDHI),             apcBody +  45, 1 );
    memcpy(&(apoSubmit->ciMsgFmt),             apcBody +  46, 1 );
    memcpy(&(apoSubmit->csMsgSrc),             apcBody +  47, 6 );
    memcpy(&(apoSubmit->csFeeType),            apcBody +  53, 2 );
    memcpy(&(apoSubmit->csFeeCode),            apcBody +  55, 6 );
    memcpy(&(apoSubmit->csValidTime),          apcBody +  61, 17);
    memcpy(&(apoSubmit->csAtTime),             apcBody +  78, 17);
    memcpy(&(apoSubmit->csSrcID),              apcBody +  95, 21);
    
    return  (0);
}

int CheckCMPP30_SUBMIT(CMPPHead *apoCMPPHead, char *apcBody, CMPPSubmit *apoSubmit)
{
    int                 liTrueSize = 0;
    int                 liMsgLength = 0;
    int                 liNodeCount = 0;
    char*               lpcNextPtr = NULL;
    unsigned char       lcTempValue = (char)0;
    
    assert(apoSubmit);
    
    lpcNextPtr = apcBody;
    memset(&apoSubmit, 0, sizeof(CMPPSubmit));
    liTrueSize = apoCMPPHead->ciTotalLength - CMPP_HEADER_LEN;
    
    if(liTrueSize < CMPP30_SUBMIT_MIN_BODY_LEN)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Invalid Body Size = %d < %d",
            __FUNCTION__,
            liTrueSize,
            CMPP30_SUBMIT_MIN_BODY_LEN);

        return  (ERROR_CODE_INVALID_PACKET);
    }

    memcpy(&lcTempValue, lpcNextPtr + 128, 1 );
    apoSubmit->ciDestUsrTL = lcTempValue;
    liNodeCount = (lcTempValue & 0xFF);
    
    if(liNodeCount <= 0 || liNodeCount > CMPP_MAX_DEST_USR_COUNT)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Invalid DestUsr_Tl = %d",
            __FUNCTION__,
            liNodeCount);

        return  (ERROR_CODE_DEST_USR_TL_FAIL);
    }
    
    if(liTrueSize < (CMPP30_SUBMIT_MIN_BODY_LEN + (liNodeCount-1)*32))
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Invalid Body Size = %d/%d",
            __FUNCTION__,
            liTrueSize,
            liNodeCount);

        return  (ERROR_CODE_DEST_USR_TL_FAIL);
    }
    
    lpcNextPtr = (apcBody + 129 + (liNodeCount*32) + 1);
    memcpy(&lcTempValue, lpcNextPtr, 1);
    apoSubmit->ciMsgLength = lcTempValue;
    liMsgLength = (lcTempValue & 0xFF);
    
    if(liMsgLength < 0 || liMsgLength > MAX_CONTENT_LEN)
    {
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s Invalid MsgLength = %d",
            __FUNCTION__,
            liMsgLength);

        return  (ERROR_CODE_INVALID_LEN);
    }
    
    memcpy(&(apoSubmit->ciPkTotal),            apcBody +   8, 1 );
    memcpy(&(apoSubmit->ciPkNumber),           apcBody +   9, 1 );
    memcpy(&(apoSubmit->ciRegisteredDelivery), apcBody +  10, 1 );
    memcpy(&(apoSubmit->ciMsgLevel),           apcBody +  11, 1 );
    memcpy(&(apoSubmit->csServiceID),          apcBody +  12, 10);
    memcpy(&(apoSubmit->ciFeeUserType),        apcBody +  22, 1 );
    memcpy(&(apoSubmit->csFeeTerminalID),      apcBody +  23, 32);
    memcpy(&(apoSubmit->ciFeeTerminalType),    apcBody +  55, 1 );
    memcpy(&(apoSubmit->ciTpPID),              apcBody +  56, 1 );
    memcpy(&(apoSubmit->ciTpUDHI),             apcBody +  57, 1 );
    memcpy(&(apoSubmit->ciMsgFmt),             apcBody +  58, 1 );
    memcpy(&(apoSubmit->csMsgSrc),             apcBody +  59, 6 );
    memcpy(&(apoSubmit->csFeeType),            apcBody +  65, 2 );
    memcpy(&(apoSubmit->csFeeCode),            apcBody +  67, 6 );
    memcpy(&(apoSubmit->csValidTime),          apcBody +  73, 17);
    memcpy(&(apoSubmit->csAtTime),             apcBody +  90, 17);
    memcpy(&(apoSubmit->csSrcID),              apcBody + 107, 21);
        
    return  (0);
}

int LoadCMPP20_SUBMIT(CMPPHead *apoCMPPHead, char *apcBody, clsSubmitList &aoList)
{
    int                 liNodeCount = 0;
    int                 liMsgLength = 0;
    char*               lpcNextPtr = NULL;
    CMPPSubmit          loGlobalPartSubmit;
    CMPP_SUBMIT_NODE    loDynamicSubmitNode;
    
    aoList.ClearAll();
    lpcNextPtr = apcBody;
    memset(&loGlobalPartSubmit, 0, sizeof(CMPPSubmit));

    memcpy(&(loGlobalPartSubmit.ciPkTotal),            lpcNextPtr +   8, 1 );
    memcpy(&(loGlobalPartSubmit.ciPkNumber),           lpcNextPtr +   9, 1 );
    memcpy(&(loGlobalPartSubmit.ciRegisteredDelivery), lpcNextPtr +  10, 1 );
    memcpy(&(loGlobalPartSubmit.ciMsgLevel),           lpcNextPtr +  11, 1 );
    memcpy(&(loGlobalPartSubmit.csServiceID),          lpcNextPtr +  12, 10);
    memcpy(&(loGlobalPartSubmit.ciFeeUserType),        lpcNextPtr +  22, 1 );
    memcpy(&(loGlobalPartSubmit.csFeeTerminalID),      lpcNextPtr +  23, 21);
    memcpy(&(loGlobalPartSubmit.ciTpPID),              lpcNextPtr +  44, 1 );
    memcpy(&(loGlobalPartSubmit.ciTpUDHI),             lpcNextPtr +  45, 1 );
    memcpy(&(loGlobalPartSubmit.ciMsgFmt),             lpcNextPtr +  46, 1 );
    memcpy(&(loGlobalPartSubmit.csMsgSrc),             lpcNextPtr +  47, 6 );
    memcpy(&(loGlobalPartSubmit.csFeeType),            lpcNextPtr +  53, 2 );
    memcpy(&(loGlobalPartSubmit.csFeeCode),            lpcNextPtr +  55, 6 );
    memcpy(&(loGlobalPartSubmit.csValidTime),          lpcNextPtr +  61, 17);
    memcpy(&(loGlobalPartSubmit.csAtTime),             lpcNextPtr +  78, 17);
    memcpy(&(loGlobalPartSubmit.csSrcID),              lpcNextPtr +  95, 21);
    memcpy(&(loGlobalPartSubmit.ciDestUsrTL),          lpcNextPtr + 116, 1 );
    
    liNodeCount = (loGlobalPartSubmit.ciDestUsrTL & 0xFF);
    lpcNextPtr = lpcNextPtr + (117 + liNodeCount * 21);

    memcpy(&(loGlobalPartSubmit.ciMsgLength),          lpcNextPtr +   0, 1 );

    liMsgLength = (loGlobalPartSubmit.ciMsgLength & 0xFF);
    strcpy(loGlobalPartSubmit.csMsgContent, " ");
    loGlobalPartSubmit.ciMsgLength = 1;

    if(liMsgLength > 0)
    {
        loGlobalPartSubmit.ciMsgLength = (uint8_t)liMsgLength;
        memcpy(&(loGlobalPartSubmit.csMsgContent), lpcNextPtr + 2, liMsgLength);
    }
    
    lpcNextPtr = apcBody + 117;
    loGlobalPartSubmit.ciDestUsrTL = 1;
    memcpy(&(loDynamicSubmitNode.coSubmit), &loGlobalPartSubmit, sizeof(CMPPSubmit));

    for(int liIt = 0; liIt < liNodeCount; liIt++)
    {
        memcpy(loDynamicSubmitNode.coSubmit.csDestTerminalID, lpcNextPtr + (liIt*32), 32);
        aoList.AddNewNode(&loDynamicSubmitNode);
    }
    
    return  (liNodeCount);
}

int LoadCMPP30_SUBMIT(CMPPHead* apoCMPPHead, char* apcBody, clsSubmitList& aoList)
{
    int                 liNodeCount = 0;
    int                 liMsgLength = 0;
    char*               lpcNextPtr = NULL;
    CMPPSubmit          loGlobalPartSubmit;
    CMPP_SUBMIT_NODE    loDynamicSubmitNode;
    
    aoList.ClearAll();
    lpcNextPtr = apcBody;
    memset(&loGlobalPartSubmit, 0, sizeof(CMPPSubmit));

    memcpy(&(loGlobalPartSubmit.ciPkTotal),            lpcNextPtr +   8, 1 );
    memcpy(&(loGlobalPartSubmit.ciPkNumber),           lpcNextPtr +   9, 1 );
    memcpy(&(loGlobalPartSubmit.ciRegisteredDelivery), lpcNextPtr +  10, 1 );
    memcpy(&(loGlobalPartSubmit.ciMsgLevel),           lpcNextPtr +  11, 1 );
    memcpy(&(loGlobalPartSubmit.csServiceID),          lpcNextPtr +  12, 10);
    memcpy(&(loGlobalPartSubmit.ciFeeUserType),        lpcNextPtr +  22, 1 );
    memcpy(&(loGlobalPartSubmit.csFeeTerminalID),      lpcNextPtr +  23, 32);
    memcpy(&(loGlobalPartSubmit.ciFeeTerminalType),    lpcNextPtr +  55, 1 );
    memcpy(&(loGlobalPartSubmit.ciTpPID),              lpcNextPtr +  56, 1 );
    memcpy(&(loGlobalPartSubmit.ciTpUDHI),             lpcNextPtr +  57, 1 );
    memcpy(&(loGlobalPartSubmit.ciMsgFmt),             lpcNextPtr +  58, 1 );
    memcpy(&(loGlobalPartSubmit.csMsgSrc),             lpcNextPtr +  59, 6 );
    memcpy(&(loGlobalPartSubmit.csFeeType),            lpcNextPtr +  65, 2 );
    memcpy(&(loGlobalPartSubmit.csFeeCode),            lpcNextPtr +  67, 6 );
    memcpy(&(loGlobalPartSubmit.csValidTime),          lpcNextPtr +  73, 17);
    memcpy(&(loGlobalPartSubmit.csAtTime),             lpcNextPtr +  90, 17);
    memcpy(&(loGlobalPartSubmit.csSrcID),              lpcNextPtr + 107, 21);
    memcpy(&(loGlobalPartSubmit.ciDestUsrTL),          lpcNextPtr + 128, 1 );
    
    liNodeCount = (loGlobalPartSubmit.ciDestUsrTL & 0xFF);
    lpcNextPtr = lpcNextPtr + (129 + liNodeCount * 32);

    memcpy(&(loGlobalPartSubmit.ciDestTerminalType),   lpcNextPtr +   0, 1 );
    memcpy(&(loGlobalPartSubmit.ciMsgLength),          lpcNextPtr +   1, 1 );

    liMsgLength = (loGlobalPartSubmit.ciMsgLength & 0xFF);
    strcpy(loGlobalPartSubmit.csMsgContent, " ");
    loGlobalPartSubmit.ciMsgLength = 1;

    if(liMsgLength > 0)
    {
        loGlobalPartSubmit.ciMsgLength = (uint8_t)liMsgLength;
        memcpy(&(loGlobalPartSubmit.csMsgContent),     lpcNextPtr +   2, liMsgLength);
    }
    
    lpcNextPtr = lpcNextPtr + (2 + liMsgLength);
    memcpy(&(loGlobalPartSubmit.csLinkID),             lpcNextPtr +   0, 20 );

    lpcNextPtr = apcBody + 129;
    loGlobalPartSubmit.ciDestUsrTL = 1;
    memcpy(&(loDynamicSubmitNode.coSubmit), &loGlobalPartSubmit, sizeof(CMPPSubmit));

    for(int liIt = 0; liIt < liNodeCount; liIt++)
    {
        memcpy(loDynamicSubmitNode.coSubmit.csDestTerminalID, lpcNextPtr + (liIt*32), 32);
        aoList.AddNewNode(&loDynamicSubmitNode);
    }
    
    return  (liNodeCount);
}

///end namespace
AIBC_NAMESPACE_END
