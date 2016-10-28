#include <assert.h>
#include <inttypes.h>

#include "AISocket.h"
#include "AITime.h"
#include "AIChunkEx.h"
#include "AIModule.h"
#include "AITcpSocket.h"
#include "AIString.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////
LISTHEAD clsBaseSocket::coSocketHead = { &coSocketHead, &coSocketHead };
AIMutexLock clsBaseSocket::coMutexLock;

int clsBaseSocket::GetAllConnectionInfo(AIChunkEx* apoXml)
{
    apoXml->Resize(0);
    AIChunkExOut    loWriter(*apoXml);
    AISmartLock loSmartLock(clsBaseSocket::coMutexLock);
    char    lsBuf[1024];
    int     liRetCode;

    liRetCode = snprintf(
        lsBuf, 
        sizeof(lsBuf), 
        "\t<Module>\r\n"
        "\t\t<ModuleID>%s|%"PRIdMAX"</ModuleID>\r\n"
        "\t\t<SocketList>\r\n",
        gsGlobalModuleID,
        (intmax_t)getpid()
    );
    assert(liRetCode > 0);
    loWriter.PutMem(lsBuf, liRetCode);

    AI_FOR_EACH_IN_LIST(lpoSock, &clsBaseSocket::coSocketHead, clsBaseSocket, coSocketChild)
    {
        char lsTime[32] = { '\0' };
        AIFormatLocalTime(lsTime, sizeof(lsTime), "%Y-%m-%d %H:%M:%S", lpoSock->ciCreateTime);

        liRetCode = snprintf(
            lsBuf, 
            sizeof(lsBuf),
            "\t\t\t<SocketInfo>\r\n"
            "\t\t\t\t<Name>%s</Name>\r\n"
            "\t\t\t\t<Type>%s</Type>\r\n"
            "\t\t\t\t<HandleID>%d</HandleID>\r\n"
            "\t\t\t\t<LocalIP>%s</LocalIP>\r\n"
            "\t\t\t\t<LocalPort>%u</LocalPort>\r\n"
            "\t\t\t\t<RemoteIP>%s</RemoteIP>\r\n"
            "\t\t\t\t<RemotePort>%u</RemotePort>\r\n"
            "\t\t\t\t<CreateTime>%s</CreateTime>\r\n"
            "\t\t\t\t<IdleTime>%"PRIuMAX"</IdleTime>\r\n"
            "\t\t\t\t<SendCount>%"PRIuMAX"</SendCount>\r\n"
            "\t\t\t\t<RecvCount>%"PRIuMAX"</RecvCount>\r\n"
            "\t\t\t</SocketInfo>\r\n",
            lpoSock->csName,
            lpoSock->csType,
            lpoSock->ciSocket,
            lpoSock->csLocalIp,
            lpoSock->ciLocalPort,
            lpoSock->csRemoteIp,
            lpoSock->ciRemotePort,
            lsTime, 
            (uintmax_t)((AICurTime() - lpoSock->ciActiveTime)/AI_TIME_SEC),
            (uintmax_t)lpoSock->ciSendCount,
            (uintmax_t)lpoSock->ciRecvCount
        );
        assert(liRetCode > 0);
        loWriter.PutMem(lsBuf, liRetCode);
    }

    liRetCode = snprintf(
        lsBuf, 
        sizeof(lsBuf), 
        "\t\t</SocketList>\r\n"
        "\t</Module>\r\n"
    );
    assert(liRetCode > 0);
    loWriter.PutMem(lsBuf, liRetCode);

    return 0;
}


clsBaseSocket::clsBaseSocket(int aiSocket, char const* apcType, char const* apcName)
    :ciSocket(aiSocket),
    ciExUsed(0),
    ciActiveTime(AICurTime()),
    ciCreateTime(AICurTime()),
    ciSendCount(0),
    ciRecvCount(0)
{
    ai_socket_local_addr(ciSocket, csLocalIp, ciLocalPort);
    ai_socket_remote_addr(ciSocket, csRemoteIp, ciRemotePort);

    SetType(apcType);
    SetName(apcName);

    AISmartLock loSmartLock(clsBaseSocket::coMutexLock);

    ai_list_add_tail(&coSocketChild, &clsBaseSocket::coSocketHead);
}


clsBaseSocket::~clsBaseSocket()
{
    AISmartLock loSmartLock(clsBaseSocket::coMutexLock);

    ai_list_del_any(&coSocketChild);
}


size_t clsBaseSocket::IncRef(void)
{
    AISmartLock loSmartLock(coSocketLock);

    return ++ciExUsed;
}


size_t clsBaseSocket::DecRef(void)
{
    AISmartLock loSmartLock(coSocketLock);

    return --ciExUsed;
}


size_t clsBaseSocket::GetRef(void) const
{
    return ciExUsed;
}


void clsBaseSocket::IncSendCount()
{
    AISmartLock loSmartLock(coSocketLock);

    ++ciSendCount;
    ciActiveTime = AICurTime();
}


void clsBaseSocket::IncRecvCount()
{
    AISmartLock loSmartLock(coSocketLock);

    ++ciRecvCount;
    ciActiveTime = AICurTime();
}


void clsBaseSocket::SetType(char const* apcType)
{
    if (apcType)
    {
        StringCopy(csType, apcType, sizeof(csType));
    }
    else
    {
        memset(csType, 0, sizeof(csType));
    }
}


void clsBaseSocket::SetName(char const* apcName)
{
    if (apcName)
    {
        StringCopy(csName, apcName, sizeof(csName));
    }
    else
    {
        memset(csName, 0, sizeof(csName));
    }
}
////////////////////////////////////////////////////////////////////////////

clsTcpSocket::clsTcpSocket(int aiSocket, char const* apcType, char const* apcName)
    :clsBaseSocket(aiSocket, apcType, apcName)
{
}


clsTcpSocket::~clsTcpSocket()
{
    Close();
}


int clsTcpSocket::ConnectTo(char const* apcIP, int aiPort, AITime_t aiTimeout)
{
    if (ciSocket >= 0)
    {
        return -10;
    }

    ciSocket = ai_tcp_connect(apcIP, aiPort, aiTimeout/AI_TIME_SEC);

    ciActiveTime = AICurTime();

    return ciSocket >= 0 ? 0 : ciSocket;
}


int clsTcpSocket::ListenOn(char const* apcIP, int aiPort, int aiBacklog)
{
    if (ciSocket >= 0)
    {
        return -10;
    }

    ciSocket = ai_socket_listen(apcIP, aiPort, aiBacklog);

    ciActiveTime = AICurTime();

    return ciSocket >= 0 ? 0 : ciSocket;
}


int clsTcpSocket::Accept(char* apcFromAddr, int* apiFromPort, AITime_t aiTimeout)
{

    ciActiveTime = AICurTime();

    return ai_socket_accept(ciSocket, apcFromAddr, apiFromPort);
}


int clsTcpSocket::WaitEvent(int aiEvents, int* apiRevents, AITime_t atTimeout)
{
    return ai_socket_wait_events(ciSocket, aiEvents, apiRevents, atTimeout / AI_TIME_SEC);
}


ssize_t clsTcpSocket::SendData(void const* apBuf, size_t aiBufLen, AITime_t aiTimeout)
{

    ciActiveTime = AICurTime();

    return ai_socket_senddata(ciSocket, (char const*)apBuf, aiBufLen, aiTimeout/AI_TIME_SEC);
}


ssize_t clsTcpSocket::RecvData(void* apBuf, size_t aiBufLen, AITime_t aiTimeout)
{

    ciActiveTime = AICurTime();

    return ai_socket_recvdata(ciSocket, (char*)apBuf, aiBufLen, aiTimeout/AI_TIME_SEC);
}


ssize_t clsTcpSocket::RecvToken(char const* apcToken, void* apBuf, size_t aiBufLen, AITime_t aiTimeout)
{

    ciActiveTime = AICurTime();

    return ai_socket_recvtoken(ciSocket, apcToken, (char*)apBuf, aiBufLen, aiTimeout/AI_TIME_SEC);
}

ssize_t clsTcpSocket::SendFile(int aiFd, AITime_t aiTimeout, size_t aiBlockSize, ssize_t aiMaxLength)
{
    AIChunkEx loSendBlock(aiBlockSize);
    size_t liRecvSize = 0;
    size_t liHandleSize = 0;
    ssize_t liRet = -1;

    assert(aiFd >= 0);

    while (aiMaxLength < 0 || liHandleSize < (size_t)aiMaxLength)
    {
        if (aiMaxLength < 0)
        {
            liRecvSize = aiBlockSize;
        }
        else
        {
            liRecvSize = (size_t)aiMaxLength < aiBlockSize ? (size_t)aiMaxLength : aiBlockSize;
        }

        liRet = read(aiFd, loSendBlock.GetPointer(), liRecvSize);
       
        AI_RETURN_IF(-10, (liRet < 0));
        
        if (0 == liRet)
        { // EOF
            break;
        }

        assert((size_t)liRet <= liRecvSize);
       
        liHandleSize += (size_t)liRet; 

        liRet = ai_socket_senddata(ciSocket, loSendBlock.GetPointer(), liRet, aiTimeout);

        AI_RETURN_IF(liRet, (liRet < 0));
    }

    ciActiveTime = AICurTime();

    return liHandleSize; 
}



void clsTcpSocket::Close()
{
    ciActiveTime = AICurTime();

    ai_socket_close(ciSocket);
}

///end namespace
AIBC_NAMESPACE_END
