#include "AIFTPLayer.h"
#include "AITcpSocket.h"
#include "AIFile.h"
#include "AIString.h"

///start namespace
AIBC_NAMESPACE_START

#define _CLOSE_RETURN_IF(ret, expr)  if(expr) {Close(); return ret;}


clsFTPConnection::clsFTPConnection(int aiSocket)
    :clsTcpSocket(aiSocket),
    ciReplyCode(0)
{
    memset(csReplyMsg, 0, sizeof(csReplyMsg));
}


clsFTPConnection::~clsFTPConnection()
{
}


int clsFTPConnection::GetReplyCode() const
{
    return ciReplyCode;
}


char const* clsFTPConnection::GetReplyMsg() const
{
    return csReplyMsg;
}


int clsFTPConnection::ConnectTo(char const* apcIP, int aiPort, AITime_t aiTimeout)
{
    int liRetCode;

    liRetCode = clsTcpSocket::ConnectTo(apcIP, aiPort, aiTimeout);

    AI_RETURN_IF(AI_FTP_ERROR_SOCKET, (liRetCode < 0));

    liRetCode = RecvResponse(aiTimeout);

    _CLOSE_RETURN_IF(liRetCode, (liRetCode < 0));

    return 0;
}


int clsFTPConnection::SendCommand(char const* apcCommand, char const* apcArgs, AITime_t aiTimeout)
{
    char lsBuf[1024];
    int liRetCode;

    assert(apcCommand && apcArgs);

    ciReplyCode = 0;
    memset(csReplyMsg, 0, sizeof(csReplyMsg));

    liRetCode = snprintf(lsBuf, sizeof(lsBuf), "%s%s%s\r\n", apcCommand, '\0' == apcArgs[0] ? "" : " ", apcArgs);

    _CLOSE_RETURN_IF(AI_FTP_ERROR_TOOBIG, (liRetCode < 0));
    _CLOSE_RETURN_IF(AI_FTP_ERROR_TOOBIG, ((size_t)liRetCode > sizeof(lsBuf) - 1));
    
    liRetCode = SendData(lsBuf, liRetCode, aiTimeout);

    _CLOSE_RETURN_IF(AI_FTP_ERROR_SOCKET, (liRetCode < 0));

    return 0;
}


int clsFTPConnection::RecvResponse(AITime_t aiTimeout)
{
    char lsBuf[sizeof(csReplyMsg) + 6];
    int liRetCode;
    char lsToken[5];
    char *lpcPtr = NULL;
    memset(lsBuf, 0, sizeof(lsBuf));

    liRetCode = RecvToken("\r\n", lsBuf, sizeof(lsBuf) - 1, aiTimeout);
    _CLOSE_RETURN_IF(AI_FTP_ERROR_SOCKET, (liRetCode < 0));
    _CLOSE_RETURN_IF(AI_FTP_ERROR_RESPLEN, (liRetCode < 6)); // "DDD \r\n"

    if (!isdigit(lsBuf[0])
        || !isdigit(lsBuf[1])
        || !isdigit(lsBuf[2]) )
    {
        return AI_FTP_ERROR_REPLYCODE;
    }

    if (lsBuf[3] != ' ')
    {// multi-lines is  supported now
	
        if (lsBuf[3] == '-')
        {
            StringCopy(lsToken,lsBuf,sizeof(lsToken));
            lsToken[3] = ' ';

            while(true)
            {
                liRetCode = RecvToken("\r\n", lsBuf, sizeof(lsBuf) - 1, aiTimeout);
                _CLOSE_RETURN_IF(AI_FTP_ERROR_SOCKET, (liRetCode < 0));
                _CLOSE_RETURN_IF(AI_FTP_ERROR_RESPLEN, (liRetCode < 6)); // "DDD \r\n"
 
                lpcPtr = strstr(lsBuf,lsToken);

                if (lpcPtr != NULL)
                {
                    break;
                }

            }
        }
        else 
        {
            return AI_FTP_ERROR_REPLYMSG;
        }
    }
    ciReplyCode = strtoul(lsBuf, NULL, 10);
    memset(csReplyMsg, 0, sizeof(csReplyMsg));
    memcpy(csReplyMsg, lsBuf + 4, liRetCode - 6);

    return 0;
}


int clsFTPConnection::HandleCommand(char const* apcCommand, char const* apcArgs, AITime_t aiTimeout)
{
    int liRetCode;

    liRetCode = SendCommand(apcCommand, apcArgs, aiTimeout);

    _CLOSE_RETURN_IF(liRetCode, (liRetCode < 0));

    liRetCode = RecvResponse(aiTimeout);

    _CLOSE_RETURN_IF(liRetCode, (liRetCode < 0));

    return 0;
}


int clsFTPConnection::Login(char const* apcUser, char const* apcPass, AITime_t aiTimeout)
{
    int liRetCode;

    liRetCode = HandleCommand(AI_FTP_CMD_USER, apcUser, aiTimeout);

    _CLOSE_RETURN_IF(liRetCode, (liRetCode < 0));
    _CLOSE_RETURN_IF(AI_FTP_ERROR_USER, (GetReplyCode() != 331));

    liRetCode = HandleCommand(AI_FTP_CMD_PASS, apcPass, aiTimeout);

    _CLOSE_RETURN_IF(liRetCode, (liRetCode < 0));
    _CLOSE_RETURN_IF(AI_FTP_ERROR_PASS, (GetReplyCode() != 230));

    return 0;
}


int clsFTPConnection::PutFile(char const* apcRemoteDir, char const* apcLocalFile, AITime_t aiTimeout)
{
    clsTcpSocket loSocket;
    AIFile  loFile;
    int lpiAddrs[6];
    char lsAddr[20];
    int liPort;
    int liRetCode;
    char const* lpcFilename;
    char const* lpcAddress;

    liRetCode = HandleCommand(AI_FTP_CMD_CWD, apcRemoteDir, aiTimeout);

    _CLOSE_RETURN_IF(liRetCode, (liRetCode < 0));
    _CLOSE_RETURN_IF(AI_FTP_ERROR_CWD, (GetReplyCode() != 250));

    liRetCode = HandleCommand(AI_FTP_CMD_TYPE, "I", aiTimeout);

    _CLOSE_RETURN_IF(liRetCode, (liRetCode < 0));
    _CLOSE_RETURN_IF(AI_FTP_ERROR_TYPE, (GetReplyCode() != 200));

    liRetCode = HandleCommand(AI_FTP_CMD_PASV, "", aiTimeout);

    _CLOSE_RETURN_IF(liRetCode, (liRetCode < 0));
    _CLOSE_RETURN_IF(AI_FTP_ERROR_PASV, (GetReplyCode() != 227));

    lpcAddress = strchr(GetReplyMsg(), '(');
    
    _CLOSE_RETURN_IF(AI_FTP_ERROR_PASV, (NULL == lpcAddress));

    liRetCode = sscanf(
        lpcAddress + 1,
        "%d,%d,%d,%d,%d,%d",
        lpiAddrs,
        lpiAddrs + 1,
        lpiAddrs + 2,
        lpiAddrs + 3,
        lpiAddrs + 4,
        lpiAddrs + 5);

    _CLOSE_RETURN_IF(AI_FTP_ERROR_ADDRESS, (liRetCode < 0));

    for (size_t i=0; i < AI_ARRAY_SIZE(lpiAddrs); ++i)
    {
        if (lpiAddrs[i] < 0 || lpiAddrs[i] > 255)
        {
            return AI_FTP_ERROR_ADDRESS;
        }
    } 

    snprintf(lsAddr, sizeof(lsAddr), "%d.%d.%d.%d", lpiAddrs[0], lpiAddrs[1], lpiAddrs[2], lpiAddrs[3]);
    liPort = lpiAddrs[4] * 256 + lpiAddrs[5];

    liRetCode = loSocket.ConnectTo(lsAddr, liPort, aiTimeout);

    _CLOSE_RETURN_IF(AI_FTP_ERROR_DATASOCK, (liRetCode < 0));
 
    liRetCode = loFile.Open(apcLocalFile);

    _CLOSE_RETURN_IF(AI_FTP_ERROR_OPENFILE, (liRetCode < 0));

    lpcFilename = strrchr(apcLocalFile, '/');
    if (NULL == lpcFilename)
    {
        lpcFilename = apcLocalFile;
    }

    liRetCode = HandleCommand(AI_FTP_CMD_STOR, lpcFilename, aiTimeout);

    _CLOSE_RETURN_IF(liRetCode, (liRetCode < 0));
    _CLOSE_RETURN_IF(AI_FTP_ERROR_STOR, (GetReplyCode() != 150));

    liRetCode = loSocket.SendFile(loFile.GetHandle(), aiTimeout);

    _CLOSE_RETURN_IF(AI_FTP_ERROR_SENDFILE, (liRetCode < 0));

    loSocket.Close();

    liRetCode = RecvResponse(aiTimeout);

    _CLOSE_RETURN_IF(liRetCode, (liRetCode < 0));
    _CLOSE_RETURN_IF(AI_FTP_ERROR_SENDFILE, (GetReplyCode() != 226));

    return 0;
}

///end namespace
AIBC_NAMESPACE_END

