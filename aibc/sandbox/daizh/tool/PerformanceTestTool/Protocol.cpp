
#include <stdio.h>
#include "AISocket.h"
#include "Protocol.h"
#include "AICMPPLayer.h"
#include "AIAsyncTcp.h"
#include "AICrypt.h"
#include "AILog.h"
#include "AILogSys.h"

#define GET_INT(field) apoData->field = apoIni->GetIniInt(apcSection, #field, apoData->field)
#define GET_STR(field) apoIni->GetIniString(apcSection, #field, apoData->field, sizeof(apoData->field))

//////////////////////////////////////////////////////////////////////////////////////////////
int RecvCMPPPacketEx(int aiSocket, CMPPHead* apoCMPPHead, AIChunk* apoAIChunk, int aiTimeout)
{
    size_t      liBodySize;
    ssize_t     liRetCode = 0;
    stASYNCHEAD loAsyncHead;
    char lacHeader[CMPP_HEADER_LEN + sizeof(stASYNCHEAD)] = {0,};

    liRetCode = ai_socket_recvdata(aiSocket, lacHeader, sizeof(lacHeader), aiTimeout);
    if(sizeof(lacHeader) != liRetCode)
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
    
    memcpy(&loAsyncHead, lacHeader, sizeof(stASYNCHEAD) );
    memcpy(apoCMPPHead, lacHeader + sizeof(stASYNCHEAD), sizeof(CMPPHead) );
    
    loAsyncHead.ciLength = ntohl(loAsyncHead.ciLength);
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

    liBodySize = loAsyncHead.ciLength - CMPP_HEADER_LEN;
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

int SendCMPPPacketEx(int aiSocket, CMPPHead const* apoCMPPHead, void const* apvBody, int aiTimeout)
{
    int         liRetCode = 0;
    size_t      liSendSize = 0;
    char*       lpcSendBuf = NULL;
    CMPPHead    loCMPPHead;
    stASYNCHEAD loAsyncHead;

    liSendSize = apoCMPPHead->ciTotalLength + sizeof(loAsyncHead);
    AI_NEW_N_ASSERT(lpcSendBuf, char, liSendSize);
    
    loAsyncHead.ciMsgID = htonl(1);
    loAsyncHead.ciResult = 0;
    loAsyncHead.ciLength = htonl(apoCMPPHead->ciTotalLength);
    loCMPPHead.ciTotalLength = htonl(apoCMPPHead->ciTotalLength);
    loCMPPHead.ciCommandID = htonl(apoCMPPHead->ciCommandID);
    loCMPPHead.ciInvokeID = htonl(apoCMPPHead->ciInvokeID);

    memcpy(lpcSendBuf, &loAsyncHead, sizeof(loAsyncHead) );
    memcpy(lpcSendBuf + sizeof(loAsyncHead), &loCMPPHead, CMPP_HEADER_LEN);

    if(liSendSize > CMPP_HEADER_LEN)
    {
        if(apvBody != NULL)
        {
            memcpy(lpcSendBuf+CMPP_HEADER_LEN + sizeof(loAsyncHead), apvBody, liSendSize-CMPP_HEADER_LEN);
        }
        else
        {
            memset(lpcSendBuf+CMPP_HEADER_LEN + sizeof(loAsyncHead), 0, liSendSize-CMPP_HEADER_LEN);
        }
    }

    liRetCode = ai_socket_senddata(aiSocket, lpcSendBuf, liSendSize, aiTimeout);
    AI_DELETE_N(lpcSendBuf);

    return  ((size_t)liRetCode == liSendSize ? 0 : ERROR_CL_SENDDATA);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AICmppProtocol::AICmppProtocol() :
    ciCommandID(0),
    ciSeqNo(0),
    ciTime(0),
    ciCount(0),
    ciVer(0)
{
}

AICmppProtocol::~AICmppProtocol()
{
}

int AICmppProtocol::Initialize()
{
    AIConfig* lpoIni = AIGetIniHandler( "../CmppProtocol.ini" );
    if ( lpoIni == NULL )
    {
        printf( "Get CmppProtocol ini fail\n" );
        return -1;
    }

    this->ciCommandID = lpoIni->GetIniInt( "Main", "CommandID", -1 );
    this->ciVer = lpoIni->GetIniInt( "Main", "Version", -1 );
    if ( lpoIni->GetIniString( "Main", "ID", this->csID, sizeof(this->csID) ) <= 0 )
    {
        printf( "Get CmppProtocol Connect ID fail\n" );
        return -1;
    }
    if ( lpoIni->GetIniString( "Main", "Password", this->csPassword, sizeof(this->csPassword) ) <= 0 )
    {
        printf( "Get CmppProtocol Connect Password fail\n" );
        return -1;
    }

    switch( this->ciCommandID )
    {
        case CMPP_SUBMIT:
        {
            this->SetPackage( &coCmppSubmit, lpoIni, "CMPPSubmit" );
            this->SetPackage( &coCmppSubmitRep, lpoIni, "CMPPSubmitRep" );
            break;
        }
        case CMPP_DELIVER:
        {
            this->SetPackage( &coCmppDeliver, lpoIni, "CMPPDeliver" );
            this->SetPackage( &coCmppDeliverRep, lpoIni, "CMPPDeliverRep" );
            break;
        }
        case CMPP_FWD:
        {
            this->SetPackage( &coCmppFwd, lpoIni, "CMPPFwd" );
            this->SetPackage( &coCmppFwdRep, lpoIni, "CMPPFwdRep" );
            break;
        }
        default:
        {
            printf( "ERROR: invalid protocol commmand id %d\n", ciCommandID );
            assert( false );
        }
    };
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
void AICmppChannel::Initialize( AICmppProtocol* apoProtocol, int ciVer )
{
    cpoProtocol = apoProtocol;
    ciVersion   = ciVer;
}
int AICmppChannel::SendRequest( int aiSocket, int aiTimeout )
{
    if ( this->ciVersion == 1 )
        return cpoProtocol->RecvRequest(aiSocket, aiTimeout);
    else
        return cpoProtocol->SendRequest(aiSocket, aiTimeout);
}
int AICmppChannel::RecvRequest( int aiSocket, int aiTimeout )
{
    if ( this->ciVersion == 1 )
        return cpoProtocol->SendRequest(aiSocket, aiTimeout);
    else
        return cpoProtocol->RecvRequest(aiSocket, aiTimeout);
}
    
AIChannel* AICmppProtocol::CreateChannel()
{
    return new AICmppChannel;
}

void AICmppProtocol::ReleaseChannel( AIChannel* apoChannel )
{
    delete apoChannel;
}
    
int AICmppProtocol::SendConnectRequest( int aiSocket, AIChannel* apoChannel )
{
    int liRetCode = ::LoginCmpp20Gateway( aiSocket, this->csID, this->csPassword,  this->ciVer, 30 );
    AILOG_DEBUG( "Login Cmpp20 Gateway, [RetCode=%d]", liRetCode );
    if ( liRetCode == 0 )
    {
        static_cast<AICmppChannel*>(apoChannel)->Initialize( this, this->ciVer );
    }
    return liRetCode;
}

int AICmppProtocol::RecvConnectRequest( int aiSocket, AIChannel* apoChannel )
{
    AIChunk  loChunk;
    AIChunk  loResponse;
    CMPPHead loHeader;
    char     liVersion;
    
    if ( ::RecvCMPPPacketEx( aiSocket, &loHeader, &loChunk, 30 ) != 0 )
    {
        return ERROR_RECV;
    }
    
    if ( loHeader.ciCommandID != CMPP_CONNECT )
    {
        return ERROR_REQUEST;
    }
    
    memcpy( &liVersion, loChunk.GetPointer() + 22, 1 );
    static_cast<AICmppChannel*>(apoChannel)->Initialize( this, liVersion );
    AILOG_INFO( "Recv connection success, [command=0x%X]/[version=0x%X]", loHeader.ciCommandID, liVersion );
    
    loHeader.ciCommandID = CMPP_CONNECT_REP;
    loResponse.Resize(CMPP20_CONNECT_REP_BODY_LEN);
    ::memset( loResponse.GetPointer(), 0, loResponse.GetSize() );
    loHeader.ciTotalLength = CMPP_HEADER_LEN + loResponse.GetSize();
    if ( ::SendCMPPPacket( aiSocket, &loHeader, loResponse.GetPointer(), 30 ) != 0 )
    {
        return ERROR_SEND;
    }
    return 0;
}

int AICmppProtocol::SendRequest( int aiSocket, int aiTimeout )
{
    AIChunkEx loChunk;
    AIChunk   loResponse;
    CMPPHead  loHeader;
    int       liResultPos = 0;
    size_t    liRespCommandID = 0;
    
    memset( &loHeader, 0, sizeof(loHeader) );
    
    switch( this->ciCommandID )
    {
        case CMPP_SUBMIT:
        {
            this->MakeRequest( coCmppSubmit, loChunk );
            liResultPos = 8;
            liRespCommandID = CMPP_SUBMIT_REP;
            break;
        }
        case CMPP_DELIVER:
        {
            this->MakeRequest( coCmppDeliver, loChunk );
            liResultPos = 8;
            liRespCommandID = CMPP_DELIVER_REP;
            break;
        }
        case CMPP_FWD:
        {
            this->MakeRequest( coCmppFwd, loChunk );
            liResultPos = 10;
            liRespCommandID = CMPP_FWD_REP;
            break;
        }
        default:
        {
            printf( "ERROR: invalid protocol commmand id %d\n", ciCommandID );
            assert( false );
        }
    };

    loHeader.ciCommandID = this->ciCommandID;
    loHeader.ciTotalLength = CMPP_HEADER_LEN + loChunk.GetSize();
    
    if ( ::SendCMPPPacket( aiSocket, &loHeader, loChunk.GetPointer(), aiTimeout ) != 0 )
    {
        return ERROR_SEND;
    }
    
    AILOG_DEBUG( "Send CMPP Packet, [Command=0x%X]/[Length=%d]", loHeader.ciCommandID, loHeader.ciTotalLength );
    
    if ( ::RecvCMPPPacketEx( aiSocket, &loHeader, &loResponse, aiTimeout ) != 0 )
    {
        return ERROR_RECV;
    }
    
    char liResult;
    memcpy( &liResult, loResponse.GetPointer() + liResultPos, 1 );
    if ( loHeader.ciCommandID != liRespCommandID || liResult != 0 )
    {
        AILOG_ERROR( "Request command[0x%X]/result [%d]", loHeader.ciCommandID, liResult );
        return ERROR_REQUEST;
    }
    
    AILOG_DEBUG( "Recv CMPP Response Packet, [Command=0x%X]/[Length=%d]", loHeader.ciCommandID, loHeader.ciTotalLength );
    
    return 0;
}

int AICmppProtocol::RecvRequest( int aiSocket, int aiTimeout )
{
    AIChunk   loChunk;
    AIChunkEx loResponse;
    CMPPHead  loHeader;
    int       liRetCode = 0;
    if ( ( liRetCode = ::RecvCMPPPacketEx( aiSocket, &loHeader, &loChunk, aiTimeout ) ) != 0 )
    {
        if ( liRetCode == ERROR_CL_SOCKET_TIMEOUT )
        {
            return ERROR_TIMEOUT;
        }
        else
        {
           return ERROR_RECV;
        }
    }
    
    AILOG_DEBUG( "Recv CMPP Packet, [Command=0x%X]/[Length=%d]", loHeader.ciCommandID, loHeader.ciTotalLength );
    
    switch( loHeader.ciCommandID )
    {
        case CMPP_SUBMIT:
        {
            loHeader.ciCommandID = CMPP_SUBMIT_REP;
            loResponse.Resize(sizeof(CMPPSubmitRep));
            
            CMPPSubmitRep loRep;
            memcpy( &loRep, &coCmppSubmitRep, loResponse.GetSize() );
            memcpy( loRep.csMsgID, loChunk.GetPointer(), 8 );
            this->EncodePackage( loRep, loResponse );
            break;
        }
        case CMPP_DELIVER:
        {
            loHeader.ciCommandID = CMPP_DELIVER_REP;
            loResponse.Resize(sizeof(CMPPDeliverRep));
            
            CMPPDeliverRep loRep;
            memcpy( &loRep, &coCmppDeliverRep, loResponse.GetSize() );
            memcpy( loRep.csMsgID, loChunk.GetPointer(), 8 );
            this->EncodePackage( loRep, loResponse );
            break;
        }
        case CMPP_FWD:
        {
            loHeader.ciCommandID = CMPP_FWD_REP;
            loResponse.Resize(sizeof(CMPPFwdRep));
            
            CMPPFwdRep loRep;
            memcpy( &loRep, &coCmppFwdRep, loResponse.GetSize() );
            memcpy( loRep.csMsgID, loChunk.GetPointer() + 14/*off*/, 8 );
            this->EncodePackage( loRep, loResponse );
            break;
        }
        default:
        {
            AILOG_ERROR( "Invalid protocol command [0x%X]", loHeader.ciCommandID );
            return 0;
        }
    };
    
    loHeader.ciTotalLength = CMPP_HEADER_LEN + loResponse.GetSize();
    if ( ::SendCMPPPacket( aiSocket, &loHeader, loResponse.GetPointer(), aiTimeout ) != 0 )
    {
        return ERROR_SEND;
    }
    
    AILOG_DEBUG( "Response CMPP Packet, [Command=0x%X]/[Length=%d]", loHeader.ciCommandID, loHeader.ciTotalLength );
    
    return 0;
}

void AICmppProtocol::SetPackage( CMPPSubmit* apoData, AIConfig* apoIni, char const* apcSection )
{
    memset(apoData, 0, sizeof(*apoData));
    char lsMsgID[64] = {0};
    apoIni->GetIniString(apcSection, "csMsgID", lsMsgID, sizeof(lsMsgID));
    this->PaserMsgId( lsMsgID, &this->ciSeqNo, &this->ciTime, this->csIsmgNo );


    GET_INT(ciPkTotal);
    GET_INT(ciPkNumber);
    GET_INT(ciRegisteredDelivery);
    GET_INT(ciMsgLevel);
    GET_STR(csServiceID);
    GET_INT(ciFeeUserType);
    GET_STR(csFeeTerminalID);
    GET_INT(ciFeeTerminalType);
    GET_INT(ciTpPID);
    GET_INT(ciTpUDHI);
    GET_INT(ciMsgFmt);
    GET_STR(csMsgSrc);
    GET_STR(csFeeType);
    GET_STR(csFeeCode);
    GET_STR(csValidTime);
    GET_STR(csAtTime);
    GET_STR(csSrcID);
    GET_INT(ciDestUsrTL);
    GET_STR(csDestTerminalID);
    GET_INT(ciDestTerminalType);
    GET_INT(ciMsgLength);
    GET_STR(csMsgContent);
    GET_STR(csLinkID);
    
    GET_STR(csExpandMsgID);
}

void AICmppProtocol::SetPackage( CMPPDeliver* apoData, AIConfig* apoIni, char const* apcSection )
{
    memset(apoData, 0, sizeof(*apoData));
    
    char lsMsgID[64] = {0};
    apoIni->GetIniString(apcSection, "csMsgID", lsMsgID, sizeof(lsMsgID));
    this->PaserMsgId( lsMsgID, &this->ciSeqNo, &this->ciTime, this->csIsmgNo );

    GET_STR(csDestID);
    GET_STR(csServiceID);
    GET_INT(ciTpPID);
    GET_INT(ciTpUDHI);
    GET_INT(ciMsgFmt);
    GET_STR(csSrcTerminalID);
    GET_INT(ciSrcTerminalType);
    GET_INT(ciRegisteredDelivery);
    GET_INT(ciMsgLength);
    apoIni->GetIniString(apcSection, "csMsgContent", (char*)apoData->csMsgContent, sizeof(apoData->csMsgContent));
    GET_STR(csLinkID);
    
    // For Stat Report //
    GET_STR(csSRMsgID);
    GET_STR(csSRStatus);
    GET_STR(csSRSubmitTime);
    GET_STR(csSRDoneTime);
    GET_STR(csSRDestTerminalID);
    GET_INT(ciSRSmscSequence);

    // Extra Information //
    GET_STR(csExpandMsgID);
    GET_STR(csExpandSRMsgID);
}

void AICmppProtocol::SetPackage( CMPPFwd* apoData, AIConfig* apoIni, char const* apcSection )
{
    memset(apoData, 0, sizeof(*apoData));
    
    char lsMsgID[64] = {0};
    apoIni->GetIniString(apcSection, "csMsgID", lsMsgID, sizeof(lsMsgID));
    this->PaserMsgId( lsMsgID, &this->ciSeqNo, &this->ciTime, this->csIsmgNo );
    
    GET_STR(csSourceID);
    GET_STR(csDestinationID);
    GET_INT(ciNodeCount);
    GET_INT(ciFwdType); 
    GET_INT(ciPkTotal);
    GET_INT(ciPkNumber);
    GET_INT(ciRegisteredDelivery);
    GET_INT(ciMsgLevel);
    GET_STR(csServiceID);
    GET_INT(ciFeeUserType);
    GET_STR(csFeeTerminalID);
    GET_STR(csFeeTerminalPseudo);
    GET_INT(ciFeeTerminalUserType);
    GET_INT(ciTpPID);
    GET_INT(ciTpUDHI);
    GET_INT(ciMsgFmt);
    GET_STR(csMsgSrc);
    GET_STR(csFeeType);
    GET_STR(csFeeCode);
    GET_STR(csValidTime);
    GET_STR(csAtTime);
    GET_STR(csSrcTerminalID);
    GET_STR(csSrcTerminalPseudo);
    GET_INT(ciSrcTerminalUserType);
    GET_INT(ciSrcTerminalType);
    GET_INT(ciDestUsrTL);
    GET_STR(csDestTerminalID);
    GET_STR(csDestTerminalPseudo);
    GET_INT(ciDestTerminalUserType);
    GET_INT(ciMsgLength);
    GET_STR(csMsgContent);
    GET_STR(csLinkID);

    // For Stat Report //
    GET_STR(csSRMsgID);
    GET_STR(csSRStatus);
    GET_STR(csSRSubmitTime);
    GET_STR(csSRDoneTime);
    GET_STR(csSRDestTerminalID);
    GET_INT(ciSRSmscSequence);

    // Extra Information //
    GET_STR(csExpandMsgID);
    GET_STR(csExpandSRMsgID);
}

void AICmppProtocol::SetPackage( CMPPSubmitRep* apoData, AIConfig* apoIni, char const* apcSection )
{
    memset(apoData, 0, sizeof(*apoData));
    
    GET_INT(ciResult);
}

void AICmppProtocol::SetPackage( CMPPDeliverRep* apoData, AIConfig* apoIni, char const* apcSection )
{
    memset(apoData, 0, sizeof(*apoData));
    
    GET_INT(ciResult);
}

void AICmppProtocol::SetPackage( CMPPFwdRep* apoData, AIConfig* apoIni, char const* apcSection )
{
    memset(apoData, 0, sizeof(*apoData));
    
    GET_INT(ciPkTotal);
    GET_INT(ciPkNumber);
    GET_INT(ciResult);
}

void AICmppProtocol::PaserMsgId(char* apcMsgId, int* apiSegNo, int* apiTime, char* apcIsmgNo )
{
    char* lpcTime;
    char* lpcIsmgNo;
    char* lpcSeqNo;

    lpcTime = apcMsgId;

    lpcIsmgNo = strchr(lpcTime, ':');
    if (NULL == lpcIsmgNo)
    {
        return;
    }
  
    *lpcIsmgNo = '\0'; 
    ++lpcIsmgNo;

    lpcSeqNo = strchr(lpcIsmgNo, ':');
    if (NULL == lpcSeqNo)
    {
        return;
    }

    *lpcSeqNo = '\0';
    ++lpcSeqNo;
    
    *apiSegNo = StringAtoi(lpcSeqNo);
    *apiTime  = StringAtoi(lpcTime);
    strcpy( apcIsmgNo, lpcIsmgNo );
}

#define EN_STR( name, len ) loOutput.PutMem( aoData.name, len )
#define EN_INT( name ) loOutput.PutInt8( aoData.name )
#define EN_INT32( name ) loOutput.PutNInt32( aoData.name )

void AICmppProtocol::EncodePackage( CMPPSubmit& aoData, AIChunkEx& aoChunk )
{
    AIChunkExOut loOutput(aoChunk);
    EN_STR(csMsgID, 8);
    EN_INT(ciPkTotal);
    EN_INT(ciPkNumber);
    EN_INT(ciRegisteredDelivery);
    EN_INT(ciMsgLevel);
    EN_STR(csServiceID, 10);
    EN_INT(ciFeeUserType);
    EN_STR(csFeeTerminalID, 21);
    EN_INT(ciTpPID);
    EN_INT(ciTpUDHI);
    EN_INT(ciMsgFmt);
    EN_STR(csMsgSrc, 6);
    EN_STR(csFeeType, 2);
    EN_STR(csFeeCode, 6);
    EN_STR(csValidTime, 17);
    EN_STR(csAtTime, 17);
    EN_STR(csSrcID, 21);
    EN_INT(ciDestUsrTL);
    EN_STR(csDestTerminalID, 21);
    EN_INT(ciMsgLength);
    EN_STR(csMsgContent, MAX_CONTENT_LEN);
    char lsReserver[8] = {0};
    loOutput.PutMem(lsReserver, 8);
}

void AICmppProtocol::EncodePackage( CMPPSubmitRep& aoData, AIChunkEx& aoChunk )
{
    AIChunkExOut loOutput(aoChunk);
    EN_STR(csMsgID, 8);
    EN_INT(ciResult);
}

void AICmppProtocol::EncodePackage( CMPPDeliver& aoData, AIChunkEx& aoChunk )
{
    AIChunkExOut loOutput(aoChunk);
    EN_STR(csMsgID, 8);
    EN_STR(csDestID, 21);
    EN_STR(csServiceID, 10);
    EN_INT(ciTpPID);
    EN_INT(ciTpUDHI);
    EN_INT(ciMsgFmt);
    EN_STR(csSrcTerminalID, 21);
    EN_INT(ciRegisteredDelivery);
    EN_INT(ciMsgLength);
    if ( aoData.ciRegisteredDelivery == 1 )
    {
        // For Stat Report //
        EN_STR(csMsgID, 8);
        EN_STR(csSRStatus, 7);
        EN_STR(csSRSubmitTime, 10);
        EN_STR(csSRDoneTime, 10);
        EN_STR(csSRDestTerminalID, 21);
        EN_INT32(ciSRSmscSequence);
        char lsTmp[MAX_CONTENT_LEN - 70] = {0};
        loOutput.PutMem(lsTmp, sizeof(lsTmp));
    }
    else
    {
        EN_STR(csMsgContent, MAX_CONTENT_LEN);
    }
    char lsReserver[8] = {0};
    loOutput.PutMem(lsReserver, 8);
}

void AICmppProtocol::EncodePackage( CMPPDeliverRep& aoData, AIChunkEx& aoChunk )
{
    AIChunkExOut loOutput(aoChunk);
    EN_STR(csMsgID, 8);
    EN_INT(ciResult);
}

void AICmppProtocol::EncodePackage( CMPPFwd& aoData, AIChunkEx& aoChunk )
{
    AIChunkExOut loOutput(aoChunk);
    
    EN_STR(csSourceID, 6);
    EN_STR(csDestinationID, 6);
    EN_INT(ciNodeCount);
    EN_INT(ciFwdType); 
    EN_STR(csMsgID, 8);
    EN_INT(ciPkTotal);
    EN_INT(ciPkNumber);
    EN_INT(ciRegisteredDelivery);
    EN_INT(ciMsgLevel);
    EN_STR(csServiceID, 10);
    EN_INT(ciFeeUserType);
    EN_STR(csFeeTerminalID, 21);
    EN_INT(ciTpPID);
    EN_INT(ciTpUDHI);
    EN_INT(ciMsgFmt);
    EN_STR(csMsgSrc, 6);
    EN_STR(csFeeType, 2);
    EN_STR(csFeeCode, 6);
    EN_STR(csValidTime, 17);
    EN_STR(csAtTime, 17);
    EN_STR(csSrcTerminalID, 21);
    EN_INT(ciDestUsrTL);
    EN_STR(csDestTerminalID, 21);
    //EN_STR(csDestTerminalPseudo);
    //EN_INT(ciDestTerminalUserType);
    EN_INT(ciMsgLength);
    if ( aoData.ciFwdType == 3 || aoData.ciFwdType == 2 )
    {
        // For Stat Report //
        EN_STR(csMsgID, 8);
        EN_STR(csSRStatus, 7);
        EN_STR(csSRSubmitTime, 10);
        EN_STR(csSRDoneTime, 10);
        EN_STR(csSRDestTerminalID, 21);
        EN_INT32(ciSRSmscSequence);
        char lsTmp[MAX_CONTENT_LEN - 70] = {0};
        loOutput.PutMem(lsTmp, sizeof(lsTmp));
    }
    else
    {
        EN_STR(csMsgContent, MAX_CONTENT_LEN);
    }
    char lsReserver[8] = {0};
    loOutput.PutMem(lsReserver, 8);
}

void AICmppProtocol::EncodePackage( CMPPFwdRep& aoData, AIChunkEx& aoChunk )
{
    AIChunkExOut loOutput(aoChunk);
    EN_STR(csMsgID, 8);
    EN_INT(ciPkTotal);
    EN_INT(ciPkNumber);
    EN_INT(ciResult);
}

    
#undef GET_STR
#undef GET_INT

//////////////////////////////////////////////////////////////////////////////////
AICmppProtocolEx::AICmppProtocolEx()
    : ciPort(0)
{
}

AICmppProtocolEx::~AICmppProtocolEx()
{
}

int AICmppProtocolEx::Initialize()
{
    AIConfig* lpoIni = AIGetIniHandler( "../CmppProtocol.ini" );
    if ( lpoIni == NULL )
    {
        printf( "Get CmppProtocol ini fail\n" );
        return -1;
    }

    this->ciPort = lpoIni->GetIniInt( "Main", "Port", -1 );
    
    if ( lpoIni->GetIniString( "Main", "ISMGNo", this->csISMGNo, sizeof(this->csISMGNo) ) <= 0 )
    {
        printf( "Get CmppProtocol Connect ISMGNo fail\n" );
        return -1;
    }
    if ( lpoIni->GetIniString( "Main", "IP", this->csIP, sizeof(this->csIP) ) <= 0 )
    {
        printf( "Get CmppProtocol Connect IP fail\n" );
        return -1;
    }
    
    return AICmppProtocol::Initialize();
}

int AICmppProtocolEx::SendConnectRequest( int aiSocket, AIChannel* apoChannel )
{
    int liRetCode = this->LoginCmpp20Gateway( aiSocket, this->csID, this->csPassword,  this->ciVer, 30 );
    AILOG_DEBUG( "Login Cmpp20 Gateway, [RetCode=%d]", liRetCode );
    if ( liRetCode == 0 )
    {
        static_cast<AICmppChannel*>(apoChannel)->Initialize( this, this->ciVer );
    }
    return liRetCode;
}

int AICmppProtocolEx::RecvConnectRequest( int aiSocket, AIChannel* apoChannel )
{
    AIChunk  loChunk;
    AIChunk  loResponse;
    CMPPHead loHeader;
    char     liVersion;
    
    loChunk.Resize( CMPP20_CONNECT_REQ_BODY_LEN + 42 );
    
    if ( ai_socket_recvdata(
        aiSocket, loChunk.GetPointer(), loChunk.GetSize(), 30 ) != (int)loChunk.GetSize() )
    {
        return ERROR_RECV;
    }
    
    memcpy( &liVersion, loChunk.GetPointer() + 22, 1 );
    static_cast<AICmppChannel*>(apoChannel)->Initialize( this, liVersion );
    AILOG_INFO( "Recv connection success, [command=0x%X]/[version=0x%X]", loHeader.ciCommandID, liVersion );
    
    loResponse.Resize(1 + CMPP20_CONNECT_REP_BODY_LEN);
    ::memset( loResponse.GetPointer(), 0, loResponse.GetSize() );

    if ( ai_socket_senddata( 
        aiSocket, loResponse.GetPointer(), loResponse.GetSize(), 30) != (int)loResponse.GetSize() )
    {
        return ERROR_SEND;
    }
    
    return 0;
}

int AICmppProtocolEx::LoginCmpp20Gateway(int aiSocket, char const* apcID, char const* apcPasswd, int aiVer, int aiTimeout)
{
    int             liSize;
    int             liTime = 0;
    unsigned char   liResult = 0;
    char            lsKey[128] = {0,};
    char            lcVersion = aiVer;
    char            lsTimeStamp[24] = {0};
    char            lsAuthenticator[24];
    char            lsSendBuffer[128];
    char            lsRecvBuffer[1 + CMPP20_CONNECT_REP_BODY_LEN];
    AIChunk         loRecvChunk;
    
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
    memcpy(lsSendBuffer + 27, this->csISMGNo, 6);
    memcpy(lsSendBuffer + 33, this->csIP, 32);
    memcpy(lsSendBuffer + 65, &this->ciPort, 4);

    int liTotalLen = CMPP20_CONNECT_REQ_BODY_LEN + 42;
    
    if ( ai_socket_senddata( aiSocket, lsSendBuffer, liTotalLen, aiTimeout) != liTotalLen )
    {
        return ERROR_SEND;
    }
    
    if ( ai_socket_recvdata(
        aiSocket, lsRecvBuffer, sizeof(lsRecvBuffer), aiTimeout) != (int)sizeof(lsRecvBuffer) )
    {
        return ERROR_RECV;
    }
    
    memcpy(&liResult, lsRecvBuffer, 1);
    if(liResult)
    {
        return liResult;
    }
    
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
AIDefualtChannel::AIDefualtChannel( int aiTranWinCount, int aiUnitSize )
    : ciTranWinCount(aiTranWinCount)
    , ciUnitSize(aiUnitSize)
{
    
}

int AIDefualtChannel::SendRequest( int aiSocket, int aiTimeout )
{
    AIChunkEx lsBuffer(ciTranWinCount*ciUnitSize);
    
    if ( ai_socket_senddata( aiSocket, lsBuffer.BasePtr(), lsBuffer.GetSize() ) != (ssize_t)lsBuffer.GetSize() )
    {
        return AIProtocol::ERROR_SEND;
    }
    
    if ( ai_socket_recvdata( aiSocket, lsBuffer.BasePtr(), lsBuffer.GetSize(), aiTimeout ) != (ssize_t)lsBuffer.GetSize() )
    {
        return AIProtocol::ERROR_RECV;
    }

    return 0;
}

int AIDefualtChannel::RecvRequest( int aiSocket, int aiTimeout )
{
    AIChunkEx lsBuffer(ciTranWinCount*ciUnitSize);

    if ( ai_socket_recvdata( aiSocket, lsBuffer.BasePtr(), lsBuffer.GetSize(), aiTimeout ) != (ssize_t)lsBuffer.GetSize() )
    {
        return AIProtocol::ERROR_RECV;
    }
    
    if ( ai_socket_senddata( aiSocket, lsBuffer.BasePtr(), lsBuffer.GetSize() ) != (ssize_t)lsBuffer.GetSize() )
    {
        return AIProtocol::ERROR_SEND;
    }

    return 0;
}

AIDefualtProtocol::AIDefualtProtocol( int aiTranWinCount, int aiUnitSize )
    : ciTranWinCount(aiTranWinCount)
    , ciUnitSize(aiUnitSize)
{
}

int AIDefualtProtocol::Initialize()
{
    return 0;
}

AIChannel* AIDefualtProtocol::CreateChannel()
{
    return new AIDefualtChannel(ciTranWinCount, ciUnitSize);
}

void AIDefualtProtocol::ReleaseChannel( AIChannel* apoChannel )
{
    delete (AIDefualtChannel*)apoChannel;
}

int AIDefualtProtocol::SendConnectRequest( int aiSocket, AIChannel* apoChannel )
{
    return 0;
}

int AIDefualtProtocol::RecvConnectRequest( int aiSocket, AIChannel* apoChannel )
{
    return 0;
}
