
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "AIMemAlloc.h"
#include "AICMPPStru.h"
#include "AISynch.h"
#include "AIString.h"
#include "AIConfig.h"
#include "AIChunkEx.h"

class AIChannel
{
public:
    virtual ~AIChannel() {};
    virtual int SendRequest( int aiSocket, int aiTimeout )        = 0;
    virtual int RecvRequest( int aiSocket, int aiTimeout )        = 0;
};

class AIProtocol
{
public:
    enum{ 
        ERROR_SOCKET_BROKEN = -1, 
        ERROR_TIMEOUT = -2, 
        ERROR_SEND = -3, 
        ERROR_RECV = -4,
        ERROR_REQUEST = -5 };
    
public:
    virtual ~AIProtocol() {};
    virtual int Initialize() = 0;
    virtual AIChannel* CreateChannel() = 0;
    virtual void ReleaseChannel( AIChannel* apoChannel ) = 0;
    virtual int SendConnectRequest( int aiSocket, AIChannel* apoChannel ) = 0;
    virtual int RecvConnectRequest( int aiSocket, AIChannel* apoChannel ) = 0;
};

///////////////////////////////////////////////////////////////////////////////
class AICmppProtocol;
class AICmppChannel : public AIChannel
{
public:
    void Initialize( AICmppProtocol* apoProtocol, int ciVer );
    virtual int SendRequest( int aiSocket, int aiTimeout );
    virtual int RecvRequest( int aiSocket, int aiTimeout );

protected:
    int             ciVersion;
    AICmppProtocol* cpoProtocol;
};
    
class AICmppProtocol : public AIProtocol
{
public:
    AICmppProtocol();
    virtual ~AICmppProtocol();
    
    virtual int Initialize();
    virtual AIChannel* CreateChannel();
    virtual void ReleaseChannel( AIChannel* apoChannel );
    virtual int SendConnectRequest( int aiSocket, AIChannel* apoChannel );
    virtual int RecvConnectRequest( int aiSocket, AIChannel* apoChannel );
    int SendRequest( int aiSocket, int aiTimeout );
    int RecvRequest( int aiSocket, int aiTimeout );
    
protected:
    void SetPackage( CMPPSubmit* apoData, AIConfig* apoIni, char const* apcSection );
    void SetPackage( CMPPSubmitRep* apoData, AIConfig* apoIni, char const* apcSection );
    void SetPackage( CMPPDeliver* apoData, AIConfig* apoIni, char const* apcSection );
    void SetPackage( CMPPDeliverRep* apoData, AIConfig* apoIni, char const* apcSection );
    void SetPackage( CMPPFwd* apoData, AIConfig* apoIni, char const* apcSection );
    void SetPackage( CMPPFwdRep* apoData, AIConfig* apoIni, char const* apcSection );
    void PaserMsgId(char* apcMsgId, int* apiSegNo, int* apiTime, char* apcIsmgNo );
    
    void EncodePackage( CMPPSubmit& aoData, AIChunkEx& aoChunk );
    void EncodePackage( CMPPSubmitRep& aoData, AIChunkEx& aoChunk );
    void EncodePackage( CMPPDeliver& aoData, AIChunkEx& aoChunk );
    void EncodePackage( CMPPDeliverRep& aoData, AIChunkEx& aoChunk );
    void EncodePackage( CMPPFwd& aoData, AIChunkEx& aoChunk );
    void EncodePackage( CMPPFwdRep& aoData, AIChunkEx& aoChunk );
    
    template< class T >
    void MakeRequest( T& aoData, AIChunkEx& aoChunk )
    {
        int  liMsgID;
        char lsTime[32];
            
        if ( ciTime < 0 )
        {
            AIFormatLocalTime(lsTime, sizeof(lsTime), "%m%d%H%M%S", AICurTime());
        }
        else
        {
            AIFormatLocalTime(lsTime, sizeof(lsTime), "%m%d%H%M%S", ciTime );
        }

        AIFormatLocalTime(lsTime, sizeof(lsTime), "%m%d%H%M%S", AICurTime());
        
        {
            AISmartLock loLock(this->coLock);
            
            if ( ciSeqNo < 0 )
            {
                liMsgID = ciCount++;
            }
            else
            {
                liMsgID = ciSeqNo;
            }
        
            EncodeCMPPMsgID( aoData.csMsgID, liMsgID, lsTime, csIsmgNo );
            this->EncodePackage( aoData, aoChunk );
        }
    }

protected:
    CMPPSubmit     coCmppSubmit;
    CMPPDeliver    coCmppDeliver;
    CMPPFwd        coCmppFwd;
    CMPPSubmitRep  coCmppSubmitRep;
    CMPPDeliverRep coCmppDeliverRep;
    CMPPFwdRep     coCmppFwdRep;
    
    int         ciCommandID;
    int         ciSeqNo;
    char        csIsmgNo[128];
    int         ciTime;
    int         ciCount;
    
    char        csID[128];
    char        csPassword[128];
    int         ciVer;
    
    AIMutexLock coLock;
};

class AICmppProtocolEx : public AICmppProtocol
{
public:
    AICmppProtocolEx();
    virtual ~AICmppProtocolEx();
    
    virtual int Initialize();
    virtual int SendConnectRequest( int aiSocket, AIChannel* apoChannel );
    virtual int RecvConnectRequest( int aiSocket, AIChannel* apoChannel );

protected:
    int LoginCmpp20Gateway(int aiSocket, char const* apcID, char const* apcPasswd, int aiVer, int aiTimeout);

protected:
    char csISMGNo[20];
    char csIP[40];
    int  ciPort;
};

/////////////////////////////////////////////////////////////////////////////////////
class AIDefualtProtocol;
class AIDefualtChannel : public AIChannel
{
public:
    AIDefualtChannel( int aiTranWinCount, int aiUnitSize );
    virtual ~AIDefualtChannel() {};
    virtual int SendRequest( int aiSocket, int aiTimeout );
    virtual int RecvRequest( int aiSocket, int aiTimeout );

protected:
    int ciTranWinCount;
    int ciUnitSize;
};

class AIDefualtProtocol : public AIProtocol
{
public:
    AIDefualtProtocol( int aiTranWinCount, int aiUnitSize );
    virtual ~AIDefualtProtocol() {};
    virtual int Initialize();
    virtual AIChannel* CreateChannel();
    virtual void ReleaseChannel( AIChannel* apoChannel );
    virtual int SendConnectRequest( int aiSocket, AIChannel* apoChannel );
    virtual int RecvConnectRequest( int aiSocket, AIChannel* apoChannel );

protected:
    int ciTranWinCount;
    int ciUnitSize;
};

#endif //
