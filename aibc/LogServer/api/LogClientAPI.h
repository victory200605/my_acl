#ifndef LOG_CLIENT_API
#define LOG_CLIENT_API

#include "LogServerMsg.h"
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>

/// no available connection
#define LOG_SERVER_ERRNO_NOCONN            (-1)
/// socket error whild sending request
#define LOG_SERVER_ERRNO_SENDERR           (-2)
/// socket is broken while sending request or receiving response
#define LOG_SERVER_ERRNO_BROKEN            (-3)
/// response timedout
#define LOG_SERVER_ERRNO_TIMEDOUT          (-4)
/// too mandy pending requests
#define LOG_SERVER_ERRNO_TOOMANY           (-5)

/** 
 * Though the LogClient API itself would re-connect when the connection 
 * is closed, a callback specified in Startup() is involved. Here
 * are aiErrno why the connection are reset.
 *
 */
#define LOG_SERVER_CONNTION_CLOSE_RECV_IDLE        1
#define LOG_SERVER_CONNTION_CLOSE_RECV_FAILED      2
#define LOG_SERVER_CONNTION_CLOSE_RECV_INVALID_MSG 3
#define LOG_SERVER_CONNTION_CLOSE_SEND_FAILED      4

/**
 * LogClient CONNECTION callback function.
 * The callback MAY be execute in multithread evirenment, 
 * may sure to use lock when accessing external resources,
 *
 * @param[in] apvArg    the customer apvArg supplied in callback function.
 * @param[in] aiErrno   one of LOG_SERVER_CONNTION_CLOSE_* or 0 to indicate the result status
 * @param[in] apcAddr   the IP Address which connection closed.
 * @param[in] aiPort    the port which connection closed.
 */
typedef void (*LogClientConnCloseCallback)(
        void* apvArg, 
        int aiErrno, 
        const char* apcAddr, 
        uint16_t aiPort);

/**
 * Impl class of Log Client API
 */
class CLogClientApiImpl;

/**
 * Class of Log Client API
 */
class CLogClientApi
{
public:
	/**
     * Read options from config file and constructor the Log Client API
     *
     * @param[in] apcConfigFile path to the config file
     */
	CLogClientApi(const char* apcConfigFile);

	/**
     * Destructor
     */
	~CLogClientApi();

    /**
     * Send Log request (and wait for rsp from Log Server).
     * There are two kinds of requests: 
     *      1. Don't need any response from Log Server. (Logging message)
     *      2. Need response from Log Server. (Sending bill)
     *
     * @param[in] aoReq         contain the message to Log Server
     * @param[out] apiStatus    response status from Log Server, should be 
     *                          not NULL if aiMode is LOG_SERVER_MODE_TWO_WAY
     * @param[in] aiMode        request mode in one of LOG_SERVER_MODE_*
     *
     * @retval 0    request sent successful
     * @retval !=0  request sent failure
     */
	int SendRequest(CLogServerMsg const& aoSeq,
		int32_t* api32Status = NULL,
		uint32_t ai32Mode = LOG_SERVER_MODE_ONE_WAY);

	 /**
     * Start the Log Client API.
     * Threads would be created and start to make connections
     *
     * @retval 0    success
     * @retval !=0  error, see logging for more details
     * @param[in] apfCB     the connction close callback function 
     * @param[in] apvArg    this would become the first argument for apfCB
     */
	int StartUp(LogClientConnCloseCallback apfConnCloseCallback, void* apvArg);

	 /**
     * Stop the Log Client API.
     * Thread would be exit and connections would be close.
     *
     * @retval 0    success
     * @retval !=0  error, see logging for more details
     */
	int ShutDown();

private:
	CLogClientApiImpl* mpoApiImpl;
};

/**
 * Example config file.
 *
 * [LogClientAPI0]
 * ;; server ip
 * server_ip=10.3.3.108
 *
 * ;; server port
 * server_port=9696
 *
 * ;; socket send idle period, 
 * ;; keep-alive should be send when timedout.
 * send_idle_period=5
 *
 * ;; max time to finish request sending
 * request_timeout=5
 *
 * ;; period between request and response,
 * ;; operation should be notified error when timeout
 * response_timeout=5
 *
 * ;; max pending requests 
 * max_pending_requests=10000
 *
 * ;; multi-APIs is supported
 * ;; same as LogClientAPI0, multiterm is supported
 * [LogClientAPI1]
 * server_ip=10.3.3.108
 * server_port=8888
 * send_idle_period=5
 * request_timeout=5
 * response_timeout=5
 * max_pending_requests=10000
 *
 */

#endif
