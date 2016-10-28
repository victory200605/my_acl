/** @file AISocket.h
*/

#ifndef __AILIBEX__AISOCKET_H__
#define __AILIBEX__AISOCKET_H__
#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

#define IS_VALID_SOCKET(s)  (s >= 0)

enum 
{
    AI_SOCK_ERROR_INVAL     = -90101, ///< invalid input arguments
    AI_SOCK_ERROR_REFUSE    = -90102, ///< connect() refuse
    AI_SOCK_ERROR_PARTIAL   = -90103, ///< (timeout and) partial send or recv
    AI_SOCK_ERROR_BROKEN    = -90104, ///< connection is broken

    AI_SOCK_ERROR_WAIT      = -90110, ///< failed to select() or poll()
    AI_SOCK_ERROR_SEND      = -90111, ///< failed to send()
    AI_SOCK_ERROR_RECV      = -90112, ///< failed to recv()
    AI_SOCK_ERROR_SOCKET    = -90113, ///< failed to socket()
    AI_SOCK_ERROR_BIND      = -90114, ///< failed to bind()
    AI_SOCK_ERROR_LISTEN    = -90115, ///< failed to listen()
    AI_SOCK_ERROR_ACCEPT    = -90116, ///< failed to accept()
    AI_SOCK_ERROR_CONNECT   = -90117, ///< failed to connect()
    AI_SOCK_ERROR_SOCKOPT   = -90118, ///< failed to getsockopt() or setsockopt()
    AI_SOCK_ERROR_GETNAME   = -90119, ///< failed to getsockname() or getpeername()
    AI_SOCK_ERROR_SENDTO    = -90120, ///< failed to sendto()
    AI_SOCK_ERROR_RECVFROM  = -90121, ///< failed to recvfrom()
    AI_SOCK_ERROR_POLL      = -90122, ///< failed to poll()

    AI_SOCK_ERROR_UNKNOWN   = -90199  ///< unknown error
};

/**
 * @brief Set the struct of sockaddr_in from the specified IP Address and port.
 * 
 * @param apoSA         OUT - the output struct of sockaddr_in
 * @param apcIPAddr     IN - the input IP Address
 * @param aiPort        IN - the port
 *
 * @return Upon successful completion,0 shall be returned.Otherwise,an errno -90101 shall be returned to indicate invalid input arguments.
 */ 
extern int ai_set_sockaddr_in(struct sockaddr_in* apoSA, char const* apcIPAddr, int aiPort);

/**
 * @brief Get IP Address and port from the struct of sockaddr_in.
 * 
 * @param apoSA         IN - the input struct of sockaddr_in
 * @param apcIPAddr     OUT - the output IP Address 
 * @param aiLen         IN - the length of IP Address
 * @param apiPort       OUT - the output port
 *
 * @return Upon successful completion,0 shall be returned.Otherwise,an errno -90101 shall be returned to indicate invalid input arguments.
 */ 
extern int ai_get_sockaddr_in(struct sockaddr_in const* apoSA, char* apcIPAddr, size_t aiLen, int* apiPort);

/**
 * @brief Create an endpoint for communication.
 *
 * @param aiFamily    IN - the specified the communications domain in which a socket is to be created.
 * @param aiType      IN - the specified the type of socket to be created. 
 * @param aiProto     IN - the specified a particular protocol to be used with the socket.
 * 
 * @return Upon successful completion, ai_socket_create() shall return a non-negative integer, the socket file descriptor. Otherwise, an errno -90113 shall be returned to indicate failed to socket().
 */ 
extern int ai_socket_create(int aiFamily, int aiType, int aiProto);

/** 
 * @brief Close the socket(aiSocket) and set it -1.
 *
 * @param aiSocket     IN/OUT - the socket to close
 */
extern void ai_socket_close(int& aiSocket);

/**
 * @brief Bind to socket.
 *
 * @param aiSocket     IN - the specified socket
 * @param apcBindAddr  IN - the bind IP Address
 * @param aiPort       IN - the bind port
 *
 * @return 0:bind successfully; <0:failed to bind
 */ 
extern int ai_socket_bind(int aiSocket, char const* apcBindAddr, int aiPort);

/**
 * @brief Connect socket to remote address
 *
 * @param aiSocket     IN - the specified socket
 * @param apcConnAddr  IN - the remote address
 * @param aiPort       IN - the remote port
 *
 * @return 0:bind successfully; <0:failed to connect
 */ 
extern int ai_udp_bind_remote(int aiSocket, char const* apcConnAddr, int aiPort);

/**
 * @brief Server to listen port.
 *
 * @param apcBindIpAddr      IN - bind IP Address
 * @param aiPort             IN - listen port
 * @param aiBackLog          IN - the max number of outstanding connections in the socket's listen queue
 *
 * @return <0 error code, >=0 connected socket.
 */
extern  int ai_socket_listen(char const* apcBindIpAddr, int aiPort, int aiBackLog = 5);


/** 
 * @brief Server to accept one client.
 *
 * @param aiSocket      IN - a socket in listening
 * @param apcIpAddr     IN - pointer to the start of the buffer to save client ip
 * @param aiPort        IN - pointer to the start of the buffer to save port
 *
 * @return <0  error code, >=0 a new that accepted.
 */
extern  int ai_socket_accept(int aiSocket, char* apcClientAddr, int* apiPort);


/** 
 * @brief Connect to server for write.
 *
 * @param apcIpAddr     IN - server ip
 * @param aiPort        IN - server listen port
 * @param aiTimeout     IN - connect timeout in seconds
 *
 * @return <0 error code, >=0 a new socket that connected
 */
extern  int ai_socket_connect(char const* apcIpAddr, int aiPort, time_t atTimeout = 3);

/**
 * @brief Connect to server for write via nonblocking.
 *
 * @param apcIpAddr    IN - server IP address
 * @param aiPort       IN - server listen port
 * @param aiTimeout    IN - connect timeout in seconds
 *
 * @return Return the non-negative file descriptor of the socket if no error.Otherwise a negative number shall be returned to indicate the error.
 */ 
extern  int ai_socket_connect_nonblocking(char const *apcIpAddr, int aiPort, time_t atTimeout = 3);

/**
 * @brief Examine the socket whether is ready for reading.
 *
 * @param aiSocket      IN - the socket to examine
 * @param atNewTimeout  IN - select timeout in second 
 *
 * @return If the socket can read, 1 shall be returned.Otherwise,-90110 shall be returned to indicate failed to select() or poll().
 */ 
extern  int ai_socket_canread(int aiSocket, time_t atNewTimeout = 0);

#define ai_tcp_connect  ai_socket_connect_nonblocking

/** 
 * @brief Send data to socket.
 *
 * @param aiSocket      IN - the specified socket file descriptor  
 * @param apcData       IN - pointer to the buffer containing the message to send
 * @param aiSize        IN - the specified length of the message in bytes
 * @param aiTimeout     IN - timeout
 *
 * @return <0 error code, 0 timeout, >0 a new socket that connected
 */
extern  ssize_t ai_socket_senddata(int aiSocket, char const* apcData, size_t aiSize, time_t atTimeout = 0);

/**
 * @brief Format write data to socket.
 * 
 * @param aiSocket    IN - the specified socket file descriptor
 * @param apcFormat   IN - the data format
 * @param ...         IN - variable-length arguments
 * 
 * @return >0 shall be returned if successfully write data.
 */ 
extern  ssize_t ai_socket_writedata(int aiSocket, char const* apcFormat, ...);

/**
 * @brief Send a line of text to the socket.
 *
 * @param aiSocket    IN - the specified socket file descriptor
 * @param apcData     IN - pointer to the buffer containing the message to send
 * @param aiSize      IN - the specified length of the message in bytes
 * @param aiTimeout   IN - timeout
 *
 * @return >0 shall be returned if send successfully.
 */ 
extern  ssize_t ai_socket_sendline(int aiSocket, char const* apcData, size_t aiSize, time_t atTimeout);

/**
 * @brief Send data to a socket.
 *
 * @param aiSocket    IN - the specified socket file descriptor. 
 * @param apcIP       IN - the sendto IP Address
 * @param aiPort      IN - the sendto port
 * @param apcData     IN - the pointer to a buffer containing the data to be sent. 
 * @param aiSize      IN - the size of the data in bytes 
 *
 * @return Upon successful completion,ai_socket_sendto() shall return the number of bytes sent. Otherwise, an errno shall be returned.
 */ 
extern  ssize_t ai_socket_sendto(int aiSocket, char const* apcIP, int aiPort, char const * apcData, size_t aiSize);


/** 
 * @brief Recv data from socket.
 *
 * @param aiSocket      IN - send data to the specific socket file descriptor 
 * @param apcData       OUT - buffer to save data
 * @param aiNeedSize    IN - needed data size 
 * @param aiTimeout     IN - timeout
 *
 * @return <0 error code, 0 timeout and no data received, >0 the size of received data.
 */
extern  ssize_t ai_socket_recvdata(int aiSocket, char* apcData, size_t aiNeedSize, time_t atTimeout);

/**
 * @brief Peeks at an incoming message.
 *
 * @param aiSocket     IN - the specified socket file descriptor
 * @param apcData      OUT - buffer to save data
 * @param aiMaxSize    IN - the max buffer size
 * @param atTimeout    IN - timeout
 *
 * @return the peek size
 */ 
extern  ssize_t ai_socket_peekdata(int aiSocket, char* apcData, size_t aiMaxSize, time_t atTimeout);

/**
 * @brief Recv data from socket.
 *
 * @param aiSocket    IN - the specified socket file descriptor
 * @param apcData     OUT - pointer to the start of the buffer where the message should be storted
 * @param aiMaxSize   IN - the specified length in bytes of the buffer pointed to by the buffer argument
 *
 * @return <0 error code, 0 timeout and no data received, >0 the size of received data.
 */ 
extern  ssize_t ai_socket_recvdata(int aiSocket, char* apcData, size_t aiMaxSize );

/**
 * @brief Recv data from socket.
 *
 * @param aiSocket    IN - the specified socket file descriptor
 * @param apcData     OUT - pointer to the start of the buffer where the message should be storted
 * @param aiMaxSize   IN - the specified length in bytes of apcData buffer
 * @param aiTimeout   IN - timeout
 *
 * @return Return the tail position.Otherwise, a negative number shall be returned to indicate the error.
 */ 
extern  ssize_t ai_socket_recvline(int aiSocket, char* apcData, size_t aiMaxSize, time_t atTimeout);

/**
 * @brief Receive data before token.
 *
 * @param aiSocket    IN - the specified socket file descriptor
 * @param apcToken    IN - the token
 * @param apcData     OUT - pointer to the start of the buffer where the message should be storted
 * @param aiMaxSize   IN - the specified length in bytes of apcData buffer
 * @param aiTimeout   IN - timeout
 *
 * @return Return the tail position.Otherwise, a negative number shall be returned to indicate the error.
 */ 
extern  ssize_t ai_socket_recvtoken(int aiSocket, const char *apcToken, char *apcData, size_t aiMaxSize, time_t atTimeout);

/**
 * @brief Receive a message from a socket.
 * 
 * @param aiSocket    IN - the specified socket file descriptor
 * @param apcIP       OUT - the remote IP Address
 * @param aiIPLen     IN - the IP length
 * @param apiPort     OUT - the remote port
 * @param apcData     OUT - pointer to the start of the buffer where the message should be storted
 * @param aiMaxSize   IN - the max output buffer size
 * @param atTimeout   IN - timeout
 *
 * @return the received size,otherwise, errno shall be returned.
 */ 
extern  ssize_t ai_socket_recvfrom(int aiSocket, char* apcIP, size_t aiIPLen, int* apiPort, char* apcData, size_t aiMaxSize, time_t atTimeout);


/**
 * @breif Set nonblocking or not.
 * 
 * @param aiSocket        IN - the specified socket file descriptor
 * @param abNonBlocking   IN - whether set or not
 *
 * @return Upon successful completion,value other than -1 shall be returned.
 */
extern  int ai_socket_set_nonblocking(int aiSocket, bool abNonBlocking);

/**
 * @brief Get the socket errno.
 *
 * @param aiSocket    IN - the specified socket file descriptor
 * @param aiErrno     OUT - socket errno
 * 
 * @return 0:on successful completion.otherwise, -1 shall be returned and errno set to indicate the error.
 */ 
extern  int ai_socket_get_errno(int aiSocket, int& aiErrno );

/**
 * @brief Get the socket events.
 *
 * @param aiSocket    IN - the specified socket file descriptor
 * @param aiEvents    OUT - socket events
 *
 * @return Upon successful completion,a positive value indicates the total number of file descriptors that have been selected;0:timeout;-1:error
 */ 
extern  int ai_socket_get_events(int aiSocket, int& aiEvents );

/**
 * @brief Wait the given socket file descriptor for the event(s) specified in events.
 *
 * @param aiSocket      IN - the specified socket file descriptor
 * @param aiEvents      IN - the events of interest on socket
 * @param apiRevents    OUT - the events that occurred on socket
 * @param atTimeout     IN - timeout
 *
 * @return >0:success; =0:timeout and no file descriptors have been selected; <0: failure
 */ 
extern  int ai_socket_wait_events(int aiSocket, int aiEvents, int* apiRevents, time_t atTimeout);

/**
 * @brief Get the local IP-Address and local port through the specified socket.
 * 
 * @param aiSocket    IN - the specified socket
 * @param apcIP       OUT - local IP-Address
 * @param aiPort      OUT - local port
 *
 * @return 0 on successful completion. Otherwise,AI_SOCK_ERROR_GETNAME shall be returned.
 */ 
extern  int ai_socket_local_addr(int aiSocket, char* apcIP, int& aiPort);

/**
 * @brief Get the remote IP-Address and port through the specified socket.
 *
 * @param aiSocket    IN - the specified socket
 * @param apcIP       OUT - remote IP-Address
 * @param aiPort      OUT - remote port
 * 
 * @return 0 on successful completion.Otherwise,AI_SOCK_ERROR_GETNAME shall be returned.
 */ 
extern  int ai_socket_remote_addr(int aiSocket, char* apcIP, int& aiPort);

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AISOCKET_H__

