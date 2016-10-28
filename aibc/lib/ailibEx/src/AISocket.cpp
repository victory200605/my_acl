#include "AILib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stdint.h>

#include "AIString.h"
#include "AISocket.h"

///start namespace
AIBC_NAMESPACE_START

#define  AI_DEFAULT_TIMEOUT       ((time_t)2)
#define  AI_SOCKET_MAX_BUFFER     (8192)

int ai_set_sockaddr_in(struct sockaddr_in* apoSA, char const* apcIPAddr, int aiPort) 
{
    memset(apoSA, 0, sizeof(*apoSA));

    apoSA->sin_family = AF_INET;

    if (NULL == apcIPAddr || 0 == apcIPAddr[0])
    {
        apoSA->sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
#if defined(AF_INET6)
        AI_RETURN_IF(AI_SOCK_ERROR_INVAL, inet_pton(AF_INET, apcIPAddr, &apoSA->sin_addr) < 0);
#else
        apoSA->sin_addr.s_addr = inet_addr(apcIPAddr);
#endif
    }

    AI_RETURN_IF(AI_SOCK_ERROR_INVAL, aiPort > (int)UINT16_MAX || aiPort < 0);
    
    apoSA->sin_port = htons(aiPort);

    return 0;
}


int ai_get_sockaddr_in(struct sockaddr_in const* apoSA, char* apcIPAddr, size_t aiLen, int* apiPort)
{
    if (apiPort)
    {
        *apiPort = ntohs(apoSA->sin_port);
    }

    if (apcIPAddr)
    {
        AI_RETURN_IF(AI_SOCK_ERROR_INVAL, aiLen < INET_ADDRSTRLEN);
#if defined(AF_INET6)
        AI_RETURN_IF(AI_SOCK_ERROR_INVAL, inet_ntop(AF_INET, &apoSA->sin_addr.s_addr, apcIPAddr, aiLen) == NULL);
#else
        StringCopy(apcIPAddr, (char*)inet_ntoa(apoSA->sin_addr), aiLen);
#endif
    }

    return 0;
}


int ai_socket_create(int aiFamily, int aiType, int aiProto)
{
    int liSocket;
    
    liSocket = socket(aiFamily, aiType, aiProto);

    AI_RETURN_IF(AI_SOCK_ERROR_SOCKET, liSocket < 0);

    return liSocket;
}


void ai_socket_close(int &aiSocket)
{
    if(IS_VALID_SOCKET(aiSocket))
    {
        close(aiSocket);
    }

    aiSocket = -1;
}


int ai_socket_bind(int aiSocket, char const* apcBindAddr, int aiPort)
{
    int liRetCode;
    struct sockaddr_in  loInetAddr;

    liRetCode = ai_set_sockaddr_in(&loInetAddr, apcBindAddr, aiPort);

    AI_RETURN_IF(AI_SOCK_ERROR_INVAL, liRetCode < 0); 

    liRetCode = bind(aiSocket, (struct sockaddr*)&loInetAddr, sizeof(loInetAddr));

    AI_RETURN_IF(AI_SOCK_ERROR_BIND, liRetCode < 0);

    return 0;
}


int ai_udp_bind_remote(int aiSocket, char const* apcConnAddr, int aiPort)
{
    int liRetCode;
    struct sockaddr_in  loInetAddr;
    
    liRetCode = ai_set_sockaddr_in(&loInetAddr, apcConnAddr, aiPort);

    AI_RETURN_IF(AI_SOCK_ERROR_INVAL, liRetCode < 0);

    liRetCode = connect(aiSocket, (struct sockaddr*)&loInetAddr, sizeof(loInetAddr));

    AI_RETURN_IF(AI_SOCK_ERROR_CONNECT, liRetCode < 0);

    return 0;
}


int ai_socket_listen(const char *apcBindIpAddr, int aiPort, int aiBackLog)
{
    int     liFlag = 1;
    int     liSocket = -1;


    liSocket = ai_socket_create(AF_INET, SOCK_STREAM, 0);

    AI_RETURN_IF(AI_SOCK_ERROR_SOCKET, !IS_VALID_SOCKET(liSocket));

    setsockopt(liSocket, SOL_SOCKET, SO_REUSEADDR, &liFlag, sizeof(int));
    fcntl(liSocket, F_SETFL, O_NONBLOCK|fcntl(liSocket, F_GETFL, 0));

    if(ai_socket_bind(liSocket, apcBindIpAddr, aiPort) < 0)
    {
        ai_socket_close(liSocket);
        return  (AI_SOCK_ERROR_BIND);
    }

    if(listen(liSocket, aiBackLog) < 0)
    {
        ai_socket_close(liSocket);
        return  (AI_SOCK_ERROR_LISTEN);
    }
    
    return  (liSocket);
}


int ai_socket_accept(int aiSocket, char *apcIpAddr, int *apiPort)
{
    struct  sockaddr_in fromAddrIn;
    socklen_t           liSockAddrLen;
    int                 liRetCode = 0;
    int                 liNewSocket = 0;

    fd_set              rd_set;
    struct timeval      time_val;

    while(1)
    {
        FD_ZERO(&rd_set);
        FD_SET((unsigned int)aiSocket, &rd_set);

        liSockAddrLen = sizeof(fromAddrIn);
        memset(&fromAddrIn, 0, liSockAddrLen);
        
        time_val.tv_sec = 1;
        time_val.tv_usec = 0;

        if((liRetCode = select(aiSocket+1, &rd_set, NULL, NULL, &time_val)) == 0)
        {
            continue;
        }
        else if(liRetCode < 0)
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }

            return  (AI_SOCK_ERROR_WAIT);
        }
        else if(!FD_ISSET(aiSocket, &rd_set))
        {
            continue;
        }

        liNewSocket = accept(aiSocket, (struct sockaddr *)&fromAddrIn, &liSockAddrLen);
        if (!IS_VALID_SOCKET(liNewSocket))
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }

            return  (AI_SOCK_ERROR_ACCEPT);
        }

        ai_get_sockaddr_in(&fromAddrIn, apcIpAddr, INET_ADDRSTRLEN, apiPort);

        return  (liNewSocket);
    }

    return  (AI_SOCK_ERROR_ACCEPT);
}


int ai_socket_connect(const char *apcIpAddr, int aiPort, time_t atTimeout)
{
    int                 liSocket = 0;
    struct sockaddr_in  server_addr;
    fd_set              wt_set;
    struct timeval      time_val;

    AI_RETURN_IF(AI_SOCK_ERROR_INVAL, ai_set_sockaddr_in(&server_addr, apcIpAddr, aiPort) < 0);

    liSocket = ai_socket_create(AF_INET, SOCK_STREAM, 0);

    AI_RETURN_IF(AI_SOCK_ERROR_SOCKET, !IS_VALID_SOCKET(liSocket));

    if(connect(liSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        close(liSocket);
        return  (AI_SOCK_ERROR_CONNECT);
    }

AGAIN:
    FD_ZERO(&wt_set);
    FD_SET((unsigned int)liSocket, &wt_set);

    time_val.tv_usec = 0;
    time_val.tv_sec = AIMax(atTimeout, (time_t)5);
    
    errno = 0;    
    if(select(liSocket+1, NULL, &wt_set, NULL, &time_val) <= 0)
    {
        if(errno == EINTR)
        {
            goto    AGAIN;
        }

        ai_socket_close(liSocket);
        return  (AI_SOCK_ERROR_WAIT);
    }

    return  (liSocket);
}


int ai_socket_connect_nonblocking(const char *apcIpAddr, int aiPort, time_t atTimeout)
{
    int                 liFlags;
    int                 liSocket;
    int                 liRetCode;
    int                 liTmpError;
    socklen_t           liLength;
    struct sockaddr_in  server_addr;
    struct timeval      time_val;
    fd_set              rd_set;
    fd_set              wt_set;

    AI_RETURN_IF(AI_SOCK_ERROR_INVAL, ai_set_sockaddr_in(&server_addr, apcIpAddr, aiPort) < 0); 

    liSocket = socket(AF_INET, SOCK_STREAM, 0);

    AI_RETURN_IF(AI_SOCK_ERROR_SOCKET, !IS_VALID_SOCKET(liSocket));

    liTmpError = 0;
    liFlags = fcntl(liSocket, F_GETFL, 0);
    fcntl(liSocket, F_SETFL, liFlags | O_NONBLOCK);

    if((liRetCode = connect(liSocket, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0)
    {
        if(errno != EINPROGRESS)
        {
            close(liSocket);
            return  (AI_SOCK_ERROR_CONNECT);
        }
    }

    if(liRetCode == 0)
    {
        goto CONNECT_OK;
    }

    FD_ZERO(&rd_set);
    FD_SET(liSocket, &rd_set);
    wt_set = rd_set;
    
    time_val.tv_sec = atTimeout;
    time_val.tv_usec = 0;

    liRetCode = select(liSocket+1, &rd_set, &wt_set, NULL, (atTimeout ? &time_val : NULL));
    if (0 == liRetCode) 
    {
        close(liSocket);
        errno = ETIMEDOUT;
        return  (AI_SOCK_ERROR_WAIT);
    }

    if(FD_ISSET(liSocket, &rd_set) || FD_ISSET(liSocket, &wt_set)) 
    {
        liLength = sizeof(liTmpError);

        if (getsockopt(liSocket, SOL_SOCKET, SO_ERROR, &liTmpError, &liLength) < 0)
        {
            close(liSocket);
            return  (AI_SOCK_ERROR_SOCKOPT);
        }
    } 
    else
    {
        close(liSocket);
        return  (AI_SOCK_ERROR_UNKNOWN);
    }

CONNECT_OK:
    fcntl(liSocket, F_SETFL, liFlags);

    if(liTmpError) 
    {
        close(liSocket);
        errno = liTmpError;

        return  (AI_SOCK_ERROR_REFUSE);
    }
    
    return  (liSocket);
}


int ai_socket_canread(int aiSocket, time_t atTimeout)
{
    int         liRetCode = 0;
    time_t      ltTimeout = AIMax(atTimeout, AI_DEFAULT_TIMEOUT);

    fd_set          rd_set;
    struct timeval  time_val;

    while(1)
    {
        FD_ZERO(&rd_set);
        FD_SET((unsigned int)aiSocket, &rd_set);
        
        time_val.tv_usec = 0;
        time_val.tv_sec = ltTimeout;

        errno = 0;
        if((liRetCode = select(aiSocket+1, &rd_set, NULL, NULL, &time_val)) == 0)
        {
            continue;
        }
        else if(liRetCode < 0)
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }

            return  (AI_SOCK_ERROR_WAIT);
        }

        return  (1);
    }

    return  (0);
}


ssize_t ai_socket_senddata(int aiSocket, const char *apcData, size_t aiSize, time_t atTimeout)
{
    int             liRetCode;
    ssize_t         liSendSize;
    size_t          liRemainSize;
    time_t          ltTimeout = AIMax(atTimeout, AI_DEFAULT_TIMEOUT);
    time_t          ltBeginTime;
    const char      *lpcTemp = NULL;

    fd_set          wt_set;
    struct timeval  time_val;
    
    if(!IS_VALID_SOCKET(aiSocket) || apcData == NULL || aiSize <= 0)
    {
        return  (AI_SOCK_ERROR_INVAL);
    }

    lpcTemp = apcData;
    liRemainSize = aiSize;

    ltBeginTime = time(NULL);

    while(liRemainSize > 0)
    {
        if (time(NULL) > ltBeginTime + ltTimeout)
        {
            return AI_SOCK_ERROR_PARTIAL;
        }

        FD_ZERO(&wt_set);
        FD_SET((unsigned int)aiSocket, &wt_set);

        time_val.tv_usec = 0;
        time_val.tv_sec = 1;

        errno = 0;
        if((liRetCode = select(aiSocket+1, NULL, &wt_set, NULL, &time_val)) == 0)
        {
            continue;
        }
        else if(liRetCode < 0)
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }

            return  (AI_SOCK_ERROR_WAIT);
        }

        errno = 0;            
        liSendSize = send(aiSocket, lpcTemp, liRemainSize, 0);
        if (liSendSize < 0)
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }

            return  (AI_SOCK_ERROR_SEND);
        }

        liRemainSize = liRemainSize - liSendSize;
        lpcTemp = lpcTemp + liSendSize;
    }

    return  (aiSize);
}


ssize_t ai_socket_writedata(int aiSocket, const char *apcFormat, ...)
{
    va_list         ap;
    int             liSize;
    ssize_t         liRetCode;
    char            *lpcBuffer = NULL;
    
    AI_NEW_N_ASSERT(lpcBuffer, char, AI_SOCKET_MAX_BUFFER);

    va_start(ap, apcFormat);
    liSize = vsnprintf(lpcBuffer, AI_SOCKET_MAX_BUFFER, apcFormat, ap);
    va_end(ap);
    
    liRetCode = ai_socket_senddata(aiSocket, lpcBuffer, liSize, 5);
    AI_DELETE_N(lpcBuffer);
    
    return  (liRetCode == liSize);
}


ssize_t ai_socket_sendline(int aiSocket, const char *apcData, size_t aiSize, time_t atTimeout)
{
    ssize_t liRetSize;
    
    liRetSize = ai_socket_senddata(aiSocket, apcData, aiSize, atTimeout);

    AI_RETURN_IF(liRetSize, (liRetSize <= 0));
    AI_RETURN_IF(AI_SOCK_ERROR_PARTIAL, ((size_t)liRetSize != aiSize));

    return  ai_socket_senddata(aiSocket, "\r\n", 2, atTimeout);
}


ssize_t ai_socket_sendto(int aiSocket, char const* apcToAddr, int aiToPort, char const* apcData, size_t aiSize)
{
    ssize_t liRetSize;
    struct sockaddr_in  loInetAddr;
    
    AI_RETURN_IF(AI_SOCK_ERROR_INVAL, ai_set_sockaddr_in(&loInetAddr, apcToAddr, aiToPort) < 0);
  
    liRetSize = sendto(aiSocket, apcData, aiSize, 0, (struct sockaddr*)&loInetAddr, sizeof(loInetAddr));  

    AI_RETURN_IF(AI_SOCK_ERROR_SENDTO, liRetSize < 0);

    return liRetSize;
}


ssize_t ai_socket_recvdata(int aiSocket, char *apcData, size_t aiNeedSize, time_t atTimeout)
{
    int             liRetCode;
    ssize_t         liRecvSize;
    size_t          liRemainSize;
    time_t          ltTimeout = AIMax(atTimeout, AI_DEFAULT_TIMEOUT);
    char            *lpcTemp = NULL;

    fd_set          rd_set;
    struct timeval  time_val;

    if(!IS_VALID_SOCKET(aiSocket) || apcData == NULL || aiNeedSize <= 0)
    {
        return  (AI_SOCK_ERROR_INVAL);
    }

    lpcTemp = (char *)apcData;
    liRemainSize = aiNeedSize;

    while(liRemainSize > 0)
    {
        FD_ZERO(&rd_set);
        FD_SET((unsigned int)aiSocket, &rd_set);
        
        time_val.tv_usec = 0;
        time_val.tv_sec = ltTimeout;

        errno = 0;
        if((liRetCode = select(aiSocket+1, &rd_set, NULL, NULL, &time_val)) == 0)
        {
            if(lpcTemp != apcData)
            {
                return  (AI_SOCK_ERROR_PARTIAL);
            }    
            return  (0);
        }
        else if(liRetCode < 0)
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }

            return  (AI_SOCK_ERROR_WAIT);
        }
        else if(!FD_ISSET(aiSocket, &rd_set))
        {
            continue;
        }

        errno = 0;
        if((liRecvSize = recv(aiSocket, lpcTemp, liRemainSize, 0)) <= 0)
        {
            if(liRecvSize < 0)
            {
                if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    continue;
                }
                    
                return  (AI_SOCK_ERROR_RECV);
            }

            return  (AI_SOCK_ERROR_BROKEN);
        }

        liRemainSize = liRemainSize - liRecvSize;
        lpcTemp = lpcTemp + liRecvSize;
    }

    return  (aiNeedSize);
}


ssize_t ai_socket_peekdata(int aiSocket, char* apcData, size_t aiMaxSize, time_t atTimeout)
{
    int     liRetCode;
    time_t  ltTimeout = AIMax(atTimeout, AI_DEFAULT_TIMEOUT);
    ssize_t liRecvSize;

    fd_set          rd_set;
    struct timeval  time_val;

    if(!IS_VALID_SOCKET(aiSocket) || apcData == NULL || aiMaxSize <= 0)
    {
        return  (AI_SOCK_ERROR_INVAL);
    }

    FD_ZERO(&rd_set);
    FD_SET((unsigned int)aiSocket, &rd_set);
        
    time_val.tv_sec = ltTimeout;
    time_val.tv_usec = 0;

    errno = 0;
    if((liRetCode = select(aiSocket+1, &rd_set, NULL, NULL, &time_val)) == 0)
    {
        return  (0);
    }
    else if(liRetCode < 0)
    {
        if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return  (0);
        }

        return  (AI_SOCK_ERROR_WAIT);
    }
    else if(!FD_ISSET(aiSocket, &rd_set))
    {
        return  (0);
    }

    errno = 0;
    liRecvSize = recv( aiSocket, apcData, aiMaxSize, MSG_PEEK );
    if (liRecvSize <= 0)
    {
        if(liRecvSize < 0)
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return  (0);
            }
            
            return  (AI_SOCK_ERROR_RECV);
        }

        return  (AI_SOCK_ERROR_BROKEN);
    }

    return  (liRecvSize);
}


ssize_t ai_socket_recvdata(int aiSocket, char* apcData, size_t aiMaxSize )
{
    int     liRetCode;
    ssize_t liRecvSize;

    fd_set          rd_set;
    struct timeval  time_val;


    if(!IS_VALID_SOCKET(aiSocket) || apcData == NULL || aiMaxSize <= 0)
    {
        return  (AI_SOCK_ERROR_INVAL);
    }

    FD_ZERO(&rd_set);
    FD_SET((unsigned int)aiSocket, &rd_set);
        
    time_val.tv_sec = 1;
    time_val.tv_usec = 0;

    errno = 0;
    if((liRetCode = select(aiSocket+1, &rd_set, NULL, NULL, &time_val)) == 0)
    {
        return  (0);
    }
    else if(liRetCode < 0)
    {
        if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return  (0);
        }

        return  (AI_SOCK_ERROR_WAIT);
    }
    else if(!FD_ISSET(aiSocket, &rd_set))
    {
        return  (0);
    }

    errno = 0;
    liRecvSize = recv(aiSocket, apcData, aiMaxSize, 0);
    if (liRecvSize <= 0)
    {
        if(liRecvSize < 0)
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return  (0);
            }
            
            return  (AI_SOCK_ERROR_RECV);
        }

        return  (AI_SOCK_ERROR_BROKEN);
    }

    return  (liRecvSize);
}


ssize_t ai_socket_recvline(int aiSocket, char *apcData, size_t aiMaxSize, time_t atTimeout)
{
    int             liRetCode;
    ssize_t         liRecvSize;
    size_t          liRemainSize;
    int             liTailPos = 0;
    time_t          ltTimeout = AIMax(atTimeout, AI_DEFAULT_TIMEOUT);
    char            *lpcEndPtr = NULL;

    fd_set          rd_set;
    struct timeval  time_val;

    if(!IS_VALID_SOCKET(aiSocket) || apcData == NULL || aiMaxSize <= 0)
    {
        return  (AI_SOCK_ERROR_INVAL);
    }

    while(1)
    {
        FD_ZERO(&rd_set);
        FD_SET((unsigned int)aiSocket, &rd_set);
        
        time_val.tv_usec = 0;
        time_val.tv_sec = ltTimeout;

        errno = 0;
        liRetCode = select(aiSocket+1, &rd_set, NULL, NULL, &time_val);
        if (0 == liRetCode)
        {
            if(liTailPos > 0)
            {
                return  (AI_SOCK_ERROR_PARTIAL);
            }    
            return  (0);
        }
        else if(liRetCode < 0)
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }

            return  (AI_SOCK_ERROR_WAIT);
        }
        else if(!FD_ISSET(aiSocket, &rd_set))
        {
            continue;
        }

        liRemainSize = aiMaxSize-liTailPos-1;
       
        errno = 0;
        if((liRecvSize = recv(aiSocket, apcData+liTailPos, liRemainSize, MSG_PEEK)) <= 0)
        {
            if(liRecvSize < 0)
            {
                if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    continue;
                }
                return  (AI_SOCK_ERROR_RECV);
            }
            
            return  (AI_SOCK_ERROR_BROKEN);
        }

        liRemainSize = liRecvSize;
        if((lpcEndPtr = (char *)memchr(apcData+liTailPos, '\n', liRemainSize)))
        {
            lpcEndPtr += 1; /* Include CRLF */
            liRemainSize = lpcEndPtr - (apcData+liTailPos);
        }

        errno = 0;
        while((liRecvSize = recv(aiSocket, apcData+liTailPos, liRemainSize, 0)) < 0)
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                errno = 0;
                continue;
            }
    
            return  (AI_SOCK_ERROR_RECV);
        }
        
        liTailPos += liRecvSize;
        apcData[liTailPos] = (char)0;

        if(lpcEndPtr && (size_t)liRecvSize == liRemainSize)
        {
            return  (liTailPos);
        }
    }

    return  (0);
}


ssize_t ai_socket_recvtoken(int aiSocket, const char *apcToken, char *apcData, size_t aiMaxSize, time_t atTimeout)
{
    ssize_t     liOffset;
    ssize_t     liRecvSize;
    ssize_t     liTokenSize;
    ssize_t     liRemainSize;
    ssize_t     liTailPos = 0;
    time_t      liTimeout = AIMax(atTimeout, AI_DEFAULT_TIMEOUT);
    char*       lpcEndPtr = NULL;

    fd_set          rd_set;
    struct timeval  time_val;

    if(!IS_VALID_SOCKET(aiSocket) || apcToken == NULL || apcData == NULL || aiMaxSize <= 0)
    {
        return  (AI_SOCK_ERROR_INVAL);
    }

    liTokenSize = strlen(apcToken);

    while(1)
    {
        FD_ZERO(&rd_set);
        FD_SET((unsigned int)aiSocket, &rd_set);
        
        time_val.tv_usec = 0;
        time_val.tv_sec = liTimeout;

        errno = 0;
        if((liRecvSize = select(aiSocket+1, &rd_set, NULL, NULL, &time_val)) == 0)
        {
            if(liTailPos > 0)
            {
                return  (AI_SOCK_ERROR_PARTIAL);
            }    
            return  (0);
        }
        else if(liRecvSize < 0)
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }

            return  (AI_SOCK_ERROR_WAIT);
        }
        else if(!FD_ISSET(aiSocket, &rd_set))
        {
            continue;
        }

        liRemainSize = aiMaxSize-liTailPos-1;
       
        errno = 0;
        if((liRecvSize = recv(aiSocket, apcData+liTailPos, liRemainSize, MSG_PEEK)) <= 0)
        {
            if(liRecvSize < 0)
            {
                if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    continue;
                }
                return  (AI_SOCK_ERROR_RECV);
            }
            
            return  (AI_SOCK_ERROR_BROKEN);
        }

        liRemainSize = liRecvSize;

        apcData[liTailPos+liRemainSize] = (char)0;
        liOffset = (liTailPos < liTokenSize) ? 0 : (liTailPos-liTokenSize);
        if((lpcEndPtr = strstr(apcData + liOffset, apcToken)))
        {
            lpcEndPtr += liTokenSize; /* Include TOKEN */
            liRemainSize = lpcEndPtr - (apcData+liTailPos);
        }

        errno = 0;
        while((liRecvSize = recv(aiSocket, apcData+liTailPos, liRemainSize, 0)) < 0)
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                errno = 0;
                continue;
            }
    
            return  (AI_SOCK_ERROR_RECV);
        }
        
        liTailPos += liRecvSize;
        apcData[liTailPos] = (char)0;

        if(lpcEndPtr && liRecvSize == liRemainSize)
        {
            return  (liTailPos);
        }
    }

    return  (0);
}


ssize_t ai_socket_recvfrom(int aiSocket, char* apcIP, size_t aiIPLen, int* apiPort, char* apcData, size_t aiMaxSize, time_t atTimeout)
{
    ssize_t             liRetSize;
    struct sockaddr_in  loFromAddr;
    socklen_t           liSockLen = sizeof(loFromAddr);
    int                 liReady;

    liReady = ai_socket_wait_events(aiSocket, POLLIN, NULL, atTimeout);

    AI_RETURN_IF(AI_SOCK_ERROR_WAIT, liReady < 0);
    AI_RETURN_IF(0, 0 == liReady);

    liRetSize = recvfrom(aiSocket, apcData, aiMaxSize, 0, (struct sockaddr*)&loFromAddr, &liSockLen); 

    AI_RETURN_IF(AI_SOCK_ERROR_RECVFROM, liRetSize < 0);

    ai_get_sockaddr_in(&loFromAddr, apcIP, aiIPLen, apiPort);

    return liRetSize;
}


int ai_socket_get_errno(int aiSocket, int &aiErrorCode)
{
    socklen_t       liLength;
    
    liLength = sizeof(aiErrorCode);
    return  getsockopt(aiSocket, SOL_SOCKET, SO_ERROR, &aiErrorCode, &liLength);
}


int ai_socket_get_events(int aiSocket, int &aiEvents)
{
    int             liRetCode;
    pollfd          loPollFd;
    
    loPollFd.fd = aiSocket;
    loPollFd.events = ~0;
    loPollFd.revents = 0;
    
    do
    {
        liRetCode = poll(&loPollFd, 1, 0);
    } while(liRetCode < 0 && errno == EINTR);
    
    aiEvents = loPollFd.revents;
    return  (liRetCode);
}


int ai_socket_wait_events(int aiSocket, int aiEvents, int* apiRevents, time_t aiTimeout)
{
    int     liRetCode;
    pollfd  loPollFd;
    time_t  liEndTime;
    time_t  liCurrTime;
    
    loPollFd.fd = aiSocket;
    loPollFd.events = aiEvents;
    loPollFd.revents = 0;
   
    if (aiTimeout <= 0)
    {
        do
        {
            liRetCode = poll(&loPollFd, 1, aiTimeout < 0 ? -1 : 0);
        } while(liRetCode < 0 && errno == EINTR);
    }
    else
    {
        liCurrTime = time(NULL);
        liEndTime = liCurrTime + aiTimeout;
        do{
            liRetCode = poll(&loPollFd, 1, (liEndTime - liCurrTime) * 1000);
            
            liCurrTime = time(NULL);
        }while(liCurrTime < liEndTime && liRetCode < 0 && errno == EINTR);
    }
 
    AI_RETURN_IF(AI_SOCK_ERROR_POLL, liRetCode < 0);
     
    if (apiRevents)
    {
        *apiRevents = loPollFd.revents;
    } 

    return liRetCode;
}


int ai_socket_set_nonblocking(int aiSocket, bool abNonBlocking)
{
    int             liValue;
    
    liValue = fcntl(aiSocket, F_GETFL);
    
    if(abNonBlocking)
    {
        liValue |= O_NONBLOCK;
    }
    else
    {
        liValue &= ~O_NONBLOCK;
    }
    
    return  fcntl(aiSocket, F_SETFL, liValue);
}


int ai_socket_local_addr(int aiSocket, char *apcIpAddr, int &aiPort)
{
    int             liRetCode;
    socklen_t       liLength;
    sockaddr_in     loSockAddr;
    
    liLength = sizeof(sockaddr_in);
    liRetCode = getsockname(aiSocket, reinterpret_cast<sockaddr*>(&loSockAddr), &liLength);
    
    if(liRetCode)
    {
        return  (AI_SOCK_ERROR_GETNAME);
    }

    return ai_get_sockaddr_in(&loSockAddr, apcIpAddr, INET_ADDRSTRLEN, &aiPort);
}


int ai_socket_remote_addr(int aiSocket, char *apcIpAddr, int &aiPort)
{
    int             liRetCode;
    socklen_t       liLength;
    sockaddr_in     loSockAddr;
    
    liLength = sizeof(loSockAddr);
    liRetCode = getpeername(aiSocket, reinterpret_cast<sockaddr*>(&loSockAddr), &liLength);
    
    if(liRetCode)
    {
        return  (AI_SOCK_ERROR_GETNAME);
    }
    
    return ai_get_sockaddr_in(&loSockAddr, apcIpAddr, INET_ADDRSTRLEN, &aiPort);
}

///end namespace
AIBC_NAMESPACE_END
