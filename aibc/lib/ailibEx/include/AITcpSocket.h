#ifndef __AILIBEX__AITCPSOCKET_H__
#define __AILIBEX__AITCPSOCKET_H__

#include "AITime.h"
#include "AISynch.h"
#include "AIChunkEx.h"
#include "AIDualLink.h"

///start namespace
AIBC_NAMESPACE_START

class clsBaseSocket
{
protected:
    int         ciSocket;
    size_t      ciExUsed;

public:
    char        csType[32];
    char        csName[32];
    int         ciLocalPort;
    char        csLocalIp[32];
    int         ciRemotePort;
    char        csRemoteIp[32];
    AITime_t    ciActiveTime;
    AITime_t    ciCreateTime;
    size_t      ciSendCount;
    size_t      ciRecvCount;
    LISTHEAD    coSocketChild;
    AIMutexLock coSocketLock;

public:
    static LISTHEAD     coSocketHead;
    static AIMutexLock  coMutexLock;
    static int GetAllConnectionInfo(AIChunkEx* apoXml);

public:
    clsBaseSocket(int aiSocket = -1, char const* apcType = NULL, char const* apcName = NULL);
    virtual ~clsBaseSocket();

    size_t IncRef();
    size_t DecRef();
    size_t GetRef() const;

    void IncSendCount();
    void IncRecvCount();

    void SetType(char const* apcType);
    void SetName(char const* apcName);
};


class clsTcpSocket : public clsBaseSocket
{
public:
    clsTcpSocket(int aiSocket = -1, char const* apcType = NULL, char const* apcName = NULL);
    virtual ~clsTcpSocket();

    int ConnectTo(char const* apcIP, int aiPort, AITime_t aiTimeout = 0);
    int ListenOn(char const* apcIP, int aiPort, int aiBacklog = 10);
    int Accept(char* apcFromAddr, int* apiFromPort, AITime_t aiTimeout = 0);

    int WaitEvent(int aiEvents, int* apiRevents, AITime_t atTimeout = 0);

    ssize_t SendData(void const* apBuf, size_t aiBufLen, AITime_t aiTimeout = 0);
    ssize_t RecvData(void* apBuf, size_t aiBufLen, AITime_t aiTimeout = 0);
    ssize_t RecvToken(char const* apcToken, void* apBuf, size_t aiBufLen, AITime_t aiTimeout = 0);

    ssize_t SendFile(int aiFd, AITime_t aiTimeout = 0, size_t aiBlockSize = 4096, ssize_t aiSendSize = -1);

    void Close();
};

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AITCPSOCKET_H__

