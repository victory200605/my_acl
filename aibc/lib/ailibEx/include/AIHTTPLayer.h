/** 
 * @file AIHTTPLayer.h
 */

#ifndef __AILIBEX__AIHTTPLAYER_H__
#define __AILIBEX__AIHTTPLAYER_H__

#include "AILib.h"
#include "AIDualLink.h"
#include "AITcpSocket.h"

///start namespace
AIBC_NAMESPACE_START

/// errors in return value
#define AI_HTTP_ERROR_INVAL         (-90601)    ///< invalid args, (e.g. NULL pointer)
#define AI_HTTP_ERROR_NOTFOUND      (-90602)    ///< header field not found (clsHTTPMessage)
#define AI_HTTP_ERROR_TOOBIG        (-90603)    ///< buffer is too small
#define AI_HTTP_ERROR_EOL           (-90604)    ///< End-Of-Line not found
#define AI_HTTP_ERROR_STARTLINE     (-90605)    ///< other error in start-line
#define AI_HTTP_ERROR_BODY          (-90606)    ///< don't know how to recv message-body
#define AI_HTTP_ERROR_SIZE          (-90607)    ///< invalid Content-Length or chunk-size
#define AI_HTTP_ERROR_SOCKET        (-90608)    ///< socket error

#define AI_HTTP_ERROR_VERSION       (-90611)    ///< invalid version in start-line
#define AI_HTTP_ERROR_METHOD        (-90612)    ///< invalid method in request-line 
#define AI_HTTP_ERROR_URI           (-90613)    ///< invalid uri in request-line
#define AI_HTTP_ERROR_STATUS        (-90614)    ///< invalid statut in response-line
#define AI_HTTP_ERROR_FIELDNAME     (-90615)    ///< invalid field name
#define AI_HTTP_ERROR_FIELDVALUE    (-90616)    ///< invalid field value
#define AI_HTTP_ERROR_FIELDSEP      (-90617)    ///< ':' not found in feild line
#define AI_HTTP_ERROR_LOCATION      (-90618)    ///< Location not found to redirect
#define AI_HTTP_ERROR_REDIRECT      (-90619)    ///< too many redirection


/****************************************************************************
 * http methods (RFC 2616)
 ****************************************************************************/
#define AI_HTTP_METHOD_GET      "GET"
#define AI_HTTP_METHOD_HEAD     "HEAD"
#define AI_HTTP_METHOD_POST     "POST"
#define AI_HTTP_METHOD_PUT      "PUT"
#define AI_HTTP_METHOD_DELETE   "DELETE"
#define AI_HTTP_METHOD_TRACE    "TRACE"
#define AI_HTTP_METHOD_CONNECT  "CONNECT"

/****************************************************************************
 * general header fields (RFC 2616)
 ****************************************************************************/
#define AI_HTTP_FIELD_CACHE_CONTROL         "Cache-Control"
#define AI_HTTP_FIELD_CONNECTION            "Connection"
#define AI_HTTP_FIELD_DATE                  "Date"
#define AI_HTTP_FIELD_PRAGMA                "Pragma"
#define AI_HTTP_FIELD_TRAILER               "Trailer"
#define AI_HTTP_FIELD_TRANSFER_ENCODING     "Transfer-Encoding"
#define AI_HTTP_FIELD_UPGRADE               "Upgrade"
#define AI_HTTP_FIELD_VIA                   "Via"
#define AI_HTTP_FIELD_WARNING               "Warning"

/****************************************************************************
 * request header fields (RFC 2616)
 ****************************************************************************/
#define AI_HTTP_FIELD_ACCEPT                "Accept"
#define AI_HTTP_FIELD_ACCEPT_CHARSET        "Accept-Charset"
#define AI_HTTP_FIELD_ACCEPT_ENCODING       "Accept-Encoding"
#define AI_HTTP_FIELD_ACCEPT_LANGUAGE       "Accept-Language"
#define AI_HTTP_FIELD_AUTHORIZATION         "Authorization"
#define AI_HTTP_FIELD_EXPECT                "Expect"
#define AI_HTTP_FIELD_FROM                  "From"
#define AI_HTTP_FIELD_HOST                  "Host"
#define AI_HTTP_FIELD_IF_MATCH              "If-Match"
#define AI_HTTP_FIELD_IF_MODIFIED_SINCE     "If-Modified-Sinc"
#define AI_HTTP_FIELD_IF_NONE_MATCH         "If-None-Match"
#define AI_HTTP_FIELD_IF_RANGE              "If-Range"
#define AI_HTTP_FIELD_IF_UNMODIFIED_SINCE   "If-Unmodified-Sinc"
#define AI_HTTP_FIELD_MAX_FORWARDS          "Max-Forwards"
#define AI_HTTP_FIELD_PROXY_AUTHORIZATION   "Proxy-Authorization"
#define AI_HTTP_FIELD_RANGE                 "Range"
#define AI_HTTP_FIELD_REFFER                "Reffer"
#define AI_HTTP_FIELD_TE                    "TE"
#define AI_HTTP_FIELD_USER_AGENT            "User-Agent"

/****************************************************************************
 * response header fields (RFC 2616)
 ****************************************************************************/
#define AI_HTTP_FIELD_ACCEPT_RANGES         "Accept-Ranges"
#define AI_HTTP_FIELD_AGE                   "Age"
#define AI_HTTP_FIELD_ETAG                  "ETag"
#define AI_HTTP_FIELD_LOCATION              "Location"
#define AI_HTTP_FIELD_PROXY_AUTHENTICATE    "Proxy-Authenticate"
#define AI_HTTP_FIELD_RETRY_AFTER           "Retry-After"
#define AI_HTTP_FIELD_SERVER                "Server"
#define AI_HTTP_FIELD_VARY                  "Vary"
#define AI_HTTP_FIELD_WWW_AUTHENTICATE      "WWW-Authenticate"

/****************************************************************************
 * entity header fields (RFC 2616)
 ****************************************************************************/
#define AI_HTTP_FIELD_ALLOW                 "Allow"
#define AI_HTTP_FIELD_CONTENT_ENCODING      "Content-Encoding"
#define AI_HTTP_FIELD_CONTENT_LANGUAGE      "Content-Language"
#define AI_HTTP_FIELD_CONTENT_LENGTH        "Content-Length"
#define AI_HTTP_FIELD_CONTENT_LOCATION      "Content-Location"
#define AI_HTTP_FIELD_CONTENT_MD5           "Content-MD5"
#define AI_HTTP_FIELD_CONTENT_RANGE         "Content-Range"
#define AI_HTTP_FIELD_CONTENT_TYPE          "Content-Type"
#define AI_HTTP_FIELD_EXPIRES               "Expires"
#define AI_HTTP_FIELD_LAST_MODIFIED         "Last-Modified"


/****************************************************************************
 * status codes (RFC 2616)
 ****************************************************************************/
typedef enum
{
    AI_HTTP_STATUS_CONTINUE                         = 100,
    AI_HTTP_STATUS_SWITCHING_PROTOCOLS              = 101,

    AI_HTTP_STATUS_OK                               = 200,
    AI_HTTP_STATUS_CREATED                          = 201,
    AI_HTTP_STATUS_ACCEPTED                         = 202,
    AI_HTTP_STATUS_NON_AUTHORITATIVE_INFORMATION    = 203,
    AI_HTTP_STATUS_NO_CONTENT                       = 204,
    AI_HTTP_STATUS_RESET_CONTENT                    = 205,
    AI_HTTP_STATUS_PARTIAL_CONTENT                  = 206,

    AI_HTTP_STATUS_MULTIPLE_CHOICES                 = 300,
    AI_HTTP_STATUS_MOVED_PERMANENTLY                = 301,
    AI_HTTP_STATUS_FOUND                            = 302,
    AI_HTTP_STATUS_SEE_OTHER                        = 303,
    AI_HTTP_STATUS_NOT_MODIFIED                     = 304,
    AI_HTTP_STATUS_USE_PROXY                        = 305,
    AI_HTTP_STATUS_TEMPORARY_REDIRECT               = 307,

    AI_HTTP_STATUS_BAD_REQUEST                      = 400,
    AI_HTTP_STATUS_BAD_UNAUTHORIZED                 = 401,
    AI_HTTP_STATUS_PAYMENT_REQUIRED                 = 402,
    AI_HTTP_STATUS_FORBIDDEN                        = 403,
    AI_HTTP_STATUS_NOT_FOUND                        = 404,
    AI_HTTP_STATUS_METHOD_NOT_ALLOWED               = 405,
    AI_HTTP_STATUS_NOT_ACCEPTABLE                   = 406,
    AI_HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED    = 407,
    AI_HTTP_STATUS_REQUEST_TIMEOUT                  = 408,
    AI_HTTP_STATUS_CONFLICT                         = 409,
    AI_HTTP_STATUS_GONE                             = 410,
    AI_HTTP_STATUS_LENGTH_REQUIRED                  = 411,
    AI_HTTP_STATUS_PRECONDITION_FAILED              = 412,
    AI_HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE         = 413,
    AI_HTTP_STATUS_REQUEST_URI_TOO_LONG             = 414,
    AI_HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE           = 415,
    AI_HTTP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE  = 416,
    AI_HTTP_STATUS_EXPECTATION_FAILED               = 417,

    AI_HTTP_STATUS_INTERNAL_SERVER_ERROR            = 500,
    AI_HTTP_STATUS_NOT_IMPLEMENTED                  = 501,
    AI_HTTP_STATUS_BAD_GATEWAY                      = 502,
    AI_HTTP_STATUS_SERVICE_UNAVAILABLE              = 503,
    AI_HTTP_STATUS_GATEWAY_TIMEOUT                  = 504,
    AI_HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED       = 505

};

/**
 * @brief Base class for HTTP message
 */
class clsHTTPMessage
{
public:
    clsHTTPMessage();
    virtual ~clsHTTPMessage();

    /**
     * @brief Reset object to initial state
     */
    void Reset();

    /**
     * @brief Get HTTP version, default is 11
     */
    unsigned GetVersion() const;

    /**
     * @brief Set HTTP version in message, only the lowest 2 digits is used
     *
     * @param aiVersion          IN - the HTTP version to set
     */
    void SetVersion(unsigned aiVersion);

    /**
     * @brief Add a name/value pair of header field, or join the new value with existed one with a comma (following the #rule in RFC 2616)
     *
     * @param  apName       IN - field name
     * @param  apValue      IN - field value
     *
     * @return Return 0 if successfully add
     */
    int AddField(char const* apName, char const* apValue);

    /**
     * @brief Delete the specified header field by the field name
     *
     * @param apName        IN - the name of field to del
     *
     * @return Return 0 if successfully del
     */
    int DelField(char const* apName);

    /**
     * @brief Return how many header fields in the message
     *
     * @return Return the num of field
     */
    size_t GetFieldCount() const;

    /**
     * @brief Return the header field name by index (order that you call AddField)
     *
     * @param aiIndex       IN - the specified index of field
     *
     * @return Get the header field name if success.
     */
    char const* GetFieldName(size_t aiIndex) const;

    /**
     * @brief Return the header field value by index (order that you call AddField)
     *
     * @param aiIndex       IN - the specified index of field
     *
     * @return Get the header field value if success.
     */
    char const* GetFieldValue(size_t aiIndex) const;


    /**
     * @brief Return the header field value by field name
     *
     * @param apcName      IN - the specified field name
     *
     * @return Get the header field value if success.
     */
    char const* GetFieldValue(char const* apcName) const;

    /**
     * @brief Write the header fields to c string buffer (with null char)
     *
     * @param apcBuf      OUT - the output buffer to store header field
     * @param aiBufSize   IN - the output buffer size
     *
     * @return Return the length of output buffer.
     */
    ssize_t BuildHeaderFieldString(char* apcBuf, size_t aiBufSize) const;

    /**
     * @brief Read header fields from c string buffer (the buffer must be terminated by a NULL char)
     * 
     * @param  apcBuf     IN - the input string
     *
     * @return Return the length of input string
     */
    ssize_t ParseHeaderFieldString(char const* apcBuf);

    /**
     * @breif Add new entity to message
     *
     * @param apBuf      IN - the buffer to add
     * @param aiBufSize  IN - buffer size
     * 
     * @return Return 0 if success.
     */
    int AddEntity(void const* apBuf, size_t aiBufSize);

    /**
     * @brief Replace the N-th Entity with a new one
     *
     * @param aiIndex     IN - the index of old entity
     * @param apBuf       IN - the new entity buffer
     * @param aiBufSize   IN - the buffer size
     *
     * @return Return 0 if success.
     */
    int SetEntity(size_t aiIndex, void const* apBuf, size_t aiBufSize);

    /**
     * @brief Delete the N-th Entity
     *
     * @param aiIndex     IN - the index of entity to del
     *
     * @return Return 0 if success.
     */
    int DelEntity(size_t aiIndex);

    /**
     * @brief Return the pointer of the N-th entity
     *
     * @param aiIndex      IN - the index of entity
     *
     * @return Return the pointer of the specified entity
     */
    char const* GetEntityPtr(size_t aiIndex) const;

    /**
     * @brief Return the size of the N-th entity
     *
     * @param aiIndex       IN - the index of entity
     *
     * @return Return the size of the specified entity.
     */
    ssize_t GetEntitySize(size_t aiIndex) const;

    /**
     * @brief Return how many entities in the message
     *
     * @return Return the num of entities
     */
    size_t GetEntityCount() const;

    /**
     * @brief Return total length of all entities
     *
     * @return Return the total length.
     */
    size_t CalcContentLength() const;

    /**
     * @brief Build start line 
     *
     * @param apcBuf         OUT - the output buffer to store start line
     * @param aiBufSize      IN - the output buffer size
     *
     * @return >0 if success,otherwise,errno shall be returned to indicated the error
     */ 
    virtual ssize_t BuildStartLineString(char* apcBuf, size_t aiBufSize) const = 0;
    
    /**
     * @brief Parse start line 
     *
     * @param apcBuf         IN - the input buffer to parse
     *
     * @return Return the length of apcBuf if success.
     */ 
    virtual ssize_t ParseStartLineString(char const* apcBuf) = 0;

protected:     
    unsigned    ciVersion;
    LISTHEAD    coFieldList;
    LISTHEAD    coEntityList;
};


/**
 * @brief Class for HTTP request message
 */
class clsHTTPRequest
    :public clsHTTPMessage
{
public:
    clsHTTPRequest();
    virtual ~clsHTTPRequest();

    /**
     * @brief Return the HTTP method
     *
     * @return Return the HTTP method
     */
    char const* GetMethod() const;

    /**
     * @brief Set the HTTP method
     *
     * @param apcMethod        IN - the method to set
     */
    void SetMethod(char const* apcMethod);

    /**
     * @brief Return the URI
     *
     * @return Get the uri
     */
    char const* GetUri() const;

    /**
     * @brief Set the URI (you have to encode the URI by you hand)
     *
     * @param apcUri           IN - the uri to set
     */
    void SetUri(char const* apcUri);

    /**
     * @brief Write the HTTP request-line to the buffer (with NULL char)
     *
     * @param apcBuf          OUT - the output buffer to store HTTP request-line
     * @param aiBufSize       IN - the output buffer size
     *
     * @return >0 if success,otherwise,errno shall be returned to indicated the error
     */
    virtual ssize_t BuildStartLineString(char* apcBuf, size_t aiBufSize) const;

    /**
     * @brief Read the HTTP request-line from the buffer (buffer must be terminated with NULL char)
     *
     * @param apcBuf           IN - the input buffer to parse
     *
     * @return Return the length of apcBuf if success.
     */
    virtual ssize_t ParseStartLineString(char const* apcBuf);

protected:
    char* cpcMethod;
    char* cpcUri;
};


/**
 * @brief Class for HTTP response message
 */
class clsHTTPResponse
    :public clsHTTPMessage
{
public:
    clsHTTPResponse();
    virtual ~clsHTTPResponse();

    /**
     * @brief Return the response status code 
     *
     * @return Get the response status code
     */
    unsigned GetStatus() const;

    /**
     * @brief Set the response status code 
     *
     * @param aiStatus        IN - the response status code to set
     */
    void SetStatus(unsigned aiStatus);

    /**
     * @brief Return the reason pharse
     *
     * @return Get the reson pharse
     */
    char const* GetReason() const;

    /**
     * @brief Set the reason pharse
     *
     * @param apcReason        IN - the reason pharse to set
     */
    void SetReason(char const* apcReason);

    /**
     * @brief Write the HTTP response-line to the buffer (with NULL char)
     * 
     * @param apcBuf          OUT - the output buffer to store HTTP response-line
     * @param aiBufSize       IN - the output buffer size
     *
     * @return >0 if success,otherwise,errno shall be returned to indicated the error
     */
    virtual ssize_t BuildStartLineString(char* apcBuf, size_t aiBufSize) const;

    /**
     * @brief Read the HTTP response-line from the buffer (buffer must be terminated with NULL char)
     *
     * @param apcBuf           IN - the input buffer to parse
     *
     * @return Return the length of apcBuf if success.
     */
    virtual ssize_t ParseStartLineString(char const* apcBuf);

protected:
    unsigned    ciStatus;
    char*       cpcReason;
};



class clsHTTPConnection
    :public clsTcpSocket
{
public:
    clsHTTPConnection(int aiSocket=-1, char const* apcType= "HTTP", char const* apcName = NULL);
    virtual ~clsHTTPConnection();

    /**
     * @brief receive data until empty line
     *
     * @param apcBuf         OUT - the buffer to store received data
     * @param aiBufSize      IN - the buffer size
     * @param aiTimeout      IN - timeout,default 5s
     *
     * @return Return the received size.
     */ 
    ssize_t RecvUntilEmptyLine(char* apcBuf, size_t aiBufSize, AITime_t aiTimeout = 5 * AI_TIME_SEC);

    /**
     * @brief The method would perform the following steps:
     *  1. set "Content-Length" to the length of entities and delete "Transfer-Encoding",
     *     if abAutoCalc is true
     *  2. build start-line and header fields, and send them
     *
     * @param apoMsg         IN - the HTTP Message
     * @param abAutoCalc     IN - whether set "Content-Length" to the length of entities and delete "Transfer-Encoding" or not
     * @param aiTimeout      IN - timeout,default 5s
     *
     * @return Return sended header buffer size.Otherwise,errno shall be returned.
     */
    ssize_t SendHeader(clsHTTPMessage* apoMsg, bool abAutoCalc=true, AITime_t aiTimeout = 5 * AI_TIME_SEC);

    /**
     * @brief The method would perform the following steps
     *  1. receive start-line (end with "\r\n"), and parse it
     *  2. receive header fields (end with "\r\n\r\n"), and parse them
     * 
     * @param apoMsg         OUT - the struct to store HTTP header
     * @param aiTimeout      IN - timeout,default 5s
     *
     * @return Return the received header buffer size.Otherwise,the negative number shall be returned.
     */
    ssize_t RecvHeader(clsHTTPMessage* apoMsg, AITime_t aiTimeout = 5 * AI_TIME_SEC);
    
    /**
     * @brief The method would call SendHeader(aoMsg, true), and then send all the entities
     *
     * @param apoMsg         IN - the HTTP message
     * @param aiTimeout      IN - timeout,default 5s
     *
     * @return Return sended body size.Otherwise,the negative number shall be returned to indicate the error. 
     */
    ssize_t SendBody(clsHTTPMessage* apoMsg, AITime_t aiTimeout = 5 * AI_TIME_SEC);

    /**
     * @brief The method would perform the following steps
     *  1. search for "Transfer-Encoding" or "Content-Length" to decide how to receive entities
     *  2. receive all the entities
     *  3. if chunked encoding is used, remove "Transfer-Encoding" and calculate "Content-Length")
     *
     * @param apoResp        OUT - the struct to store HTTP message
     * @param aiTimeout      IN - timeout,default 5s
     *
     * @return Return received size.
     */
    ssize_t RecvBody(clsHTTPMessage* apoResp, AITime_t aiTimeout = 5 * AI_TIME_SEC);

    /**
     * @brief Read chunked message-body 
     *
     * @param apoResp       OUT - the object to store received chunks
     * @param aiTimeout     IN - timeout,default 5s
     *
     * @return Return the received size.
     */ 
    ssize_t RecvChunks(clsHTTPMessage* apoResp, AITime_t aiTimeout = 5 * AI_TIME_SEC);

    /**
     * @brief Handle HTTP client request
     *
     * @param apoReq         IN - the HTTP request
     * @param apoReq         OUT - the HTTP response
     * @param aiMaxRedir     IN - max redirect count, default 5 by RFC2616
     * @param aiTimeout      IN - timeout,default 5s
     *
     * @retrun Return 0 if success.Otherwise,the negative number shall be returned.
     */ 
    int HandleClientRequest(clsHTTPRequest* apoReq, clsHTTPResponse* apoResp, size_t aiMaxRedir = 5, AITime_t aiTimeout = 5 * AI_TIME_SEC);

};

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AIHTTPLAYER_H__

