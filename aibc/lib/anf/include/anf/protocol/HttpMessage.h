/**
 * @file HttpMessage.h 
 * @author  $Author: hezk $
 * @date    $Date: 2009/12/15 03:40:15 $
 * @version $Revision: 1.1 $
 */
#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#include <string>
#include <list>
#include "acl/stl/map.h"
#include "acl/Utility.h"
#include "acl/MemoryBlock.h"
#include "acl/StrAlgo.h"
#include "anf/Utility.h"


ANF_NAMESPACE_START

namespace http{


/**
 * typedef std::map<std::string, std::string> StrStrMMap;
 */
typedef std::map<std::string, std::string> StrStrMMap;

apl_size_t const ANF_HTTP_MAX_LINE_LENGTH = 1024;
apl_size_t const ANF_HTTP_MAX_HEADER_LENGTH = 4096;

//define message type string
const apl_int_t MESSAGE_TYPE_HTTP_INVALID           = 1;
const apl_int_t MESSAGE_TYPE_HTTP_REQUEST           = 2;
const apl_int_t MESSAGE_TYPE_HTTP_RESPONSE          = 3;

/// errors in return value
#define ANF_HTTP_ERROR_INVAL         (-90601)    ///< invalid args, (e.g. NULL pointer)
#define ANF_HTTP_ERROR_NOTFOUND      (-90602)    ///< header field not found (CHttpMessage)
#define ANF_HTTP_ERROR_TOOBIG        (-90603)    ///< buffer is too small
#define ANF_HTTP_ERROR_EOL           (-90604)    ///< End-Of-Line not found
#define ANF_HTTP_ERROR_STARTLINE     (-90605)    ///< other error in start-line
#define ANF_HTTP_ERROR_BODY          (-90606)    ///< don't know how to recv message-body
#define ANF_HTTP_ERROR_SIZE          (-90607)    ///< invalid Content-Length or chunk-size
#define ANF_HTTP_ERROR_SOCKET        (-90608)    ///< socket error

#define ANF_HTTP_ERROR_VERSION       (-90611)    ///< invalid version in start-line
#define ANF_HTTP_ERROR_METHOD        (-90612)    ///< invalid method in request-line 
#define ANF_HTTP_ERROR_URI           (-90613)    ///< invalid uri in request-line
#define ANF_HTTP_ERROR_STATUS        (-90614)    ///< invalid statut in response-line
#define ANF_HTTP_ERROR_FIELDNAME     (-90615)    ///< invalid field name
#define ANF_HTTP_ERROR_FIELDVALUE    (-90616)    ///< invalid field value
#define ANF_HTTP_ERROR_FIELDSEP      (-90617)    ///< ':' not found in feild line
#define ANF_HTTP_ERROR_LOCATION      (-90618)    ///< Location not found to redirect
#define ANF_HTTP_ERROR_REDIRECT      (-90619)    ///< too many redirection
#define ANF_HTTP_ERROR_TYPE          (-90620)    ///< message type is incorrect
#define ANF_HTTP_ERROR_BUF_NOT_ENOUGH (-90621)    ///< message type is incorrect
#define ANF_HTTP_ERROR_CONTENT_LENGTH (-90622)    ///< invalid content length


/****************************************************************************
 * http methods (RFC 2616)
 ****************************************************************************/
#define ANF_HTTP_METHOD_GET      "GET"
#define ANF_HTTP_METHOD_HEAD     "HEAD"
#define ANF_HTTP_METHOD_POST     "POST"
#define ANF_HTTP_METHOD_PUT      "PUT"
#define ANF_HTTP_METHOD_DELETE   "DELETE"
#define ANF_HTTP_METHOD_TRACE    "TRACE"
#define ANF_HTTP_METHOD_CONNECT  "CONNECT"

/****************************************************************************
 * general header fields (RFC 2616)
 ****************************************************************************/
#define ANF_HTTP_FIELD_CACHE_CONTROL         "Cache-Control"
#define ANF_HTTP_FIELD_CONNECTION            "Connection"
#define ANF_HTTP_FIELD_DATE                  "Date"
#define ANF_HTTP_FIELD_PRAGMA                "Pragma"
#define ANF_HTTP_FIELD_TRAILER               "Trailer"
#define ANF_HTTP_FIELD_TRANSFER_ENCODING     "Transfer-Encoding"
#define ANF_HTTP_FIELD_UPGRADE               "Upgrade"
#define ANF_HTTP_FIELD_VIA                   "Via"
#define ANF_HTTP_FIELD_WARNING               "Warning"

/****************************************************************************
 * request header fields (RFC 2616)
 ****************************************************************************/
#define ANF_HTTP_FIELD_ACCEPT                "Accept"
#define ANF_HTTP_FIELD_ACCEPT_CHARSET        "Accept-Charset"
#define ANF_HTTP_FIELD_ACCEPT_ENCODING       "Accept-Encoding"
#define ANF_HTTP_FIELD_ACCEPT_LANGUAGE       "Accept-Language"
#define ANF_HTTP_FIELD_AUTHORIZATION         "Authorization"
#define ANF_HTTP_FIELD_EXPECT                "Expect"
#define ANF_HTTP_FIELD_FROM                  "From"
#define ANF_HTTP_FIELD_HOST                  "Host"
#define ANF_HTTP_FIELD_IF_MATCH              "If-Match"
#define ANF_HTTP_FIELD_IF_MODIFIED_SINCE     "If-Modified-Sinc"
#define ANF_HTTP_FIELD_IF_NONE_MATCH         "If-None-Match"
#define ANF_HTTP_FIELD_IF_RANGE              "If-Range"
#define ANF_HTTP_FIELD_IF_UNMODIFIED_SINCE   "If-Unmodified-Sinc"
#define ANF_HTTP_FIELD_MAX_FORWARDS          "Max-Forwards"
#define ANF_HTTP_FIELD_PROXY_AUTHORIZATION   "Proxy-Authorization"
#define ANF_HTTP_FIELD_RANGE                 "Range"
#define ANF_HTTP_FIELD_REFFER                "Reffer"
#define ANF_HTTP_FIELD_TE                    "TE"
#define ANF_HTTP_FIELD_USER_AGENT            "User-Agent"

/****************************************************************************
 * response header fields (RFC 2616)
 ****************************************************************************/
#define ANF_HTTP_FIELD_ACCEPT_RANGES         "Accept-Ranges"
#define ANF_HTTP_FIELD_AGE                   "Age"
#define ANF_HTTP_FIELD_ETAG                  "ETag"
#define ANF_HTTP_FIELD_LOCATION              "Location"
#define ANF_HTTP_FIELD_PROXY_AUTHENTICATE    "Proxy-Authenticate"
#define ANF_HTTP_FIELD_RETRY_AFTER           "Retry-After"
#define ANF_HTTP_FIELD_SERVER                "Server"
#define ANF_HTTP_FIELD_VARY                  "Vary"
#define ANF_HTTP_FIELD_WWW_AUTHENTICATE      "WWW-Authenticate"

/****************************************************************************
 * entity header fields (RFC 2616)
 ****************************************************************************/
#define ANF_HTTP_FIELD_ALLOW                 "Allow"
#define ANF_HTTP_FIELD_CONTENT_ENCODING      "Content-Encoding"
#define ANF_HTTP_FIELD_CONTENT_LANGUAGE      "Content-Language"
#define ANF_HTTP_FIELD_CONTENT_LENGTH        "Content-Length"
#define ANF_HTTP_FIELD_CONTENT_LOCATION      "Content-Location"
#define ANF_HTTP_FIELD_CONTENT_MD5           "Content-MD5"
#define ANF_HTTP_FIELD_CONTENT_RANGE         "Content-Range"
#define ANF_HTTP_FIELD_CONTENT_TYPE          "Content-Type"
#define ANF_HTTP_FIELD_EXPIRES               "Expires"
#define ANF_HTTP_FIELD_LAST_MODIFIED         "Last-Modified"


/****************************************************************************
 * status codes (RFC 2616)
 ****************************************************************************/
enum
{
    ANF_HTTP_STATUS_CONTINUE                         = 100,
    ANF_HTTP_STATUS_SWITCHING_PROTOCOLS              = 101,

    ANF_HTTP_STATUS_OK                               = 200,
    ANF_HTTP_STATUS_CREATED                          = 201,
    ANF_HTTP_STATUS_ACCEPTED                         = 202,
    ANF_HTTP_STATUS_NON_AUTHORITATIVE_INFORMATION    = 203,
    ANF_HTTP_STATUS_NO_CONTENT                       = 204,
    ANF_HTTP_STATUS_RESET_CONTENT                    = 205,
    ANF_HTTP_STATUS_PARTIAL_CONTENT                  = 206,

    ANF_HTTP_STATUS_MULTIPLE_CHOICES                 = 300,
    ANF_HTTP_STATUS_MOVED_PERMANENTLY                = 301,
    ANF_HTTP_STATUS_FOUND                            = 302,
    ANF_HTTP_STATUS_SEE_OTHER                        = 303,
    ANF_HTTP_STATUS_NOT_MODIFIED                     = 304,
    ANF_HTTP_STATUS_USE_PROXY                        = 305,
    ANF_HTTP_STATUS_TEMPORARY_REDIRECT               = 307,

    ANF_HTTP_STATUS_BAD_REQUEST                      = 400,
    ANF_HTTP_STATUS_BAD_UNAUTHORIZED                 = 401,
    ANF_HTTP_STATUS_PAYMENT_REQUIRED                 = 402,
    ANF_HTTP_STATUS_FORBIDDEN                        = 403,
    ANF_HTTP_STATUS_NOT_FOUND                        = 404,
    ANF_HTTP_STATUS_METHOD_NOT_ALLOWED               = 405,
    ANF_HTTP_STATUS_NOT_ACCEPTABLE                   = 406,
    ANF_HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED    = 407,
    ANF_HTTP_STATUS_REQUEST_TIMEOUT                  = 408,
    ANF_HTTP_STATUS_CONFLICT                         = 409,
    ANF_HTTP_STATUS_GONE                             = 410,
    ANF_HTTP_STATUS_LENGTH_REQUIRED                  = 411,
    ANF_HTTP_STATUS_PRECONDITION_FAILED              = 412,
    ANF_HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE         = 413,
    ANF_HTTP_STATUS_REQUEST_URI_TOO_LONG             = 414,
    ANF_HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE           = 415,
    ANF_HTTP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE  = 416,
    ANF_HTTP_STATUS_EXPECTATION_FAILED               = 417,

    ANF_HTTP_STATUS_INTERNAL_SERVER_ERROR            = 500,
    ANF_HTTP_STATUS_NOT_IMPLEMENTED                  = 501,
    ANF_HTTP_STATUS_BAD_GATEWAY                      = 502,
    ANF_HTTP_STATUS_SERVICE_UNAVAILABLE              = 503,
    ANF_HTTP_STATUS_GATEWAY_TIMEOUT                  = 504,
    ANF_HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED       = 505

};


/**
 * Http message class 
 */
class CHttpMessage
{
public:
    /**
     * Constructor
     */
    CHttpMessage(apl_int_t aiType);

    /**
     * Destructor.
     */
    virtual ~CHttpMessage();

    virtual void SetMessageID(apl_uint64_t auMsgId);

    virtual apl_uint64_t GetMessageID();

    /**
     * Add content to a message body.
     *
     * @param[in] aoData  string add to the body.
     *
     * @return the size add to message body.
     */
    apl_ssize_t AddBody(std::string const& aoData);

    /**
     * Add content to message body.
     *
     * @param[in] apcBuf  pointer of the add-content buf.
     * @param[in] aiBufLen  size of the content.
     *
     * @return the size add to message body.
     */
    apl_ssize_t AddBody(const char* apcBuf, apl_size_t aiBufLen);

    /**
     * Get the message body content.
     *
     * @return the message body content in string format.
     */
    std::string const& GetBody();

    /**
     * Set message body content. 
     *
     * @param[in] aoData  string set to the body.
     *
     * @return the size of message body content.
     */
    apl_ssize_t SetBody(std::string const& aoData);

    /**
     * Set message body content. 
     *
     * @param[in] apcBuf  pointer of the add-content buf.
     * @param[in] aiBufLen  size of the content.
     *
     * @return the size of message body content.
     */
    apl_ssize_t SetBody(const char* apcBuf, apl_size_t aiBufLen);

    /**
     * Clear message body content. 
     */
    void ClearBody();

    /**
     * Get the size of message body content.
     */
    apl_uint_t GetBodySize();

    /**
     * Add header filed to message. If the header filed exist, append comma to 
     * the end of the original value, then append the new value.
     *
     * @param[in] aoName  Header filed.
     * @param[in] aoValue  Header Value.
     * @param[in] acSep  Seperater, muti-line header value would need a 
     *                   space. 
     */
    void AddHeader(std::string const& aoName, std::string const& aoValue, char acSep = ',');

    /**
     * Get the value of a specified header filed.
     *
     * @param[in] aoName  Header filed.
     * 
     * @return the value of the header field.
     */
    std::string GetHeader(std::string const& aoName);

    /**
     * Get the list of all the header filed.
     *
     * @return the list of all header field.
     */
    StrStrMMap const& GetHeaderList(void);

    /**
     * Delete a specified header field.
     *
     * @return the list of all header field.
     */
    void DelHeader(std::string const& aoName);

    /**
     * Set a specified header field's value
     *
     * @param[in] aoName  Header filed.
     * @param[in] aoValue  Header Value.
     *
     */
    void SetHeader(std::string const& aoName, std::string const& aoValue);

    /**
     * Clear the header list.
     *
     */
    void ClearHeader();

    /**
     * Set the Http version.
     *
     * @param[in] aoVersion  Http version.
     */
    void SetHttpVersion(std::string const& aoVersion);

    /**
     * Get the Http version.
     *
     * @return  Http version.
     */
    std::string const& GetHttpVersion();

    
////////////////////////////////////////////////////////////////////////////////
    /**
     * Encode the Http header from CHttpMessage to memory block, 
     * Content-Length will be filled automatically when 
     * mbContentLengthFlag is true.
     *
     * @param[in] apoBuffer  pointer to a MemoryBlock. Header field in 
     *                       this message will encode and append to 
     *                       memory block.
     *
     * @retval =0 success
     * @retval <0 failure, indicate the error type.
     */
	apl_int_t EncodeHeader(acl::CMemoryBlock* apoBuffer);

    /**
     * Encode the Http body from CHttpMessage to memory block. 
     *
     * @param[in] apoMsg  pointer to a CHttpMessage. Body in CHttpMessage 
     *                    will encode and append to memory block. 
     *
     * @retval =0 success
     * @retval <0 failure, indicate the error type.
     */
	apl_int_t EncodeBody(acl::CMemoryBlock* apoBuffer);

    /**
     * Encode the Http request line from CHttpMessage to memory block.
     *
     * @param[in] apoMsg  pointer to a CHttpMessage. Body in CHttpMessage
     *                    will encode and append to memory block. 
     *
     * @retval =0 success
     * @retval <0 failure, indicate the error type.
     */
	virtual apl_int_t EncodeStartLine(acl::CMemoryBlock* apoBuffer) = 0;

    /**
     * Set the flag to calculate the content length when encoding header.
     *
     * @param[in] abFlag, bool type to set the flag. 
     */
    void SetAutoCalcContentLength(bool abFlag);

    /**
     * Set the flag to chunk the entity when encoding body.
     *
     * @param[in] abFlag, bool type to set the flag. 
     */
    void SetAutoTransferEncoding(bool abFlag);

    /**
     * Get the flag to calculate the content length when encoding header.
     *
     * @return true of false.
     */
    bool GetAutoCalcContentLengthFlag();

    /**
     * Get the flag to chunk the entity when encoding body.
     *
     * @return true of false.
     */
    bool GetAutoTransferEncodingFlag();

    /**
     * Encode a Http message.
     *
     * The encoded bytes should be appended to apoBuffer.
     *
     * @param[in] apoBuffer  pointer to a Memoryblock which to be filled.
     *
     * @retval  success, 
     * @retval <0 failure, the return value is difference from protocols.
     */
    apl_int_t EncodeMessage(acl::CMemoryBlock* apoBuffer);

    /**
     * Save the write  pointer of memory block in case of the encode fail.
     */
    void SaveMBWritePtr(acl::CMemoryBlock* apoBuffer);

    /**
     * Restore the write pointer of memory block when the encode fail.
     */
    void RestoreMBWritePtr(acl::CMemoryBlock* apoBuffer);

////////////////////////////////////////////////////////////////////////////////
//  Decode

    /**
     * Decode the Http header from memory block to CHttpMessage.
     *
     * @param[in] apoBuffer  pointer to a Memoryblock which contain the
     *                       data to decode.
     *
     * @retval =0 success
     * @retval <0 failure, indicate the error type.
     */
    apl_int_t DecodeHeader(acl::CMemoryBlock* apoBuffer);

    /**
     * Decode the Http body from memory block to CHttpMessage.
     *
     * @param[in] apoBuffer  pointer to a Memoryblock which contain the
     *                       data to decode.
     *
     * @retval =0 success
     * @retval <0 failure, indicate the error type.
     */
    apl_int_t DecodeBody(acl::CMemoryBlock* apoBuffer);

    /**
     * Decode the chunked Http body from memory block to CHttpMessage.
     *
     * @param[in] apoBuffer  pointer to a Memoryblock which contain the
     *                       data to decode.
     *
     * @retval =0 success
     * @retval <0 failure, indicate the error type.
     */
    apl_int_t DecodeBodyChunked(acl::CMemoryBlock* apoBuffer);

    /**
     * Decode the Http request line from memory block to CHttpMessage.
     *
     * @param[in] apoBuffer  pointer to a Memoryblock which contain the
     *                       data to decode.
     *
     * @retval =0 success
     * @retval <0 failure, indicate the error type.
     */
    virtual apl_int_t DecodeStartLine(acl::CMemoryBlock* apoBuffer) = 0;

    /**
     * Save the read pointer of memory block in case of the decode fail.
     */
    void SaveMBReadPtr(acl::CMemoryBlock* apoBuffer);

    /**
     * Restore the read pointer of memory block when the decode fail.
     */
    void RestoreMBReadPtr(acl::CMemoryBlock* apoBuffer);

    /**
     * Decode a Http message.
     *
     * This message will be fill by decode content
     *
     * @param[in] apoBuffer  pointer to a Memoryblock which to be filled.
     *
     * @retval >0 decode ok. how many bytes are read.
     * @retval =0 not enough content to decode
     * @retval <0 failure, indicate the error type.
     */
    apl_int_t DecodeMessage(acl::CMemoryBlock* apoBuffer);

    apl_int_t GetMsgType();

    virtual void SetErrMsg(std::string const & aoErrMsg);

    virtual const char* GetErrMsg();

    virtual void SetErrCode(apl_int_t aiErrCode);

    virtual apl_int_t GetErrCode();

///////////////////////////////////////////////////////////////////////////////


protected:     
    StrStrMMap      moFieldList; //header field list
    std::string     moEntity; //body string
    std::string     moVersion; //http version

    bool            mbContentLengthFlag; //default true
    bool            mbTransferEncodeFlag; //default false
    char*           mpcMBWritePtr; //memory block write pt
    char*           mpcMBReadPtr; //memory block read ptr

    apl_int_t       miMsgType;
};


class CHttpRequest : public CHttpMessage
{

public:
    /**
     * Constructor.
     */
    CHttpRequest();

    /**
     * Destructor.
     */
    virtual ~CHttpRequest();

    /**
     * Get request method.
     */
    std::string const& GetMethod();

    /**
     * Get request URI.
     */
    std::string const& GetRequestURI();

    /**
     * Set request method.
     *
     * @param[in] aoMethod  request method.
     */
    void SetMethod(std::string const& aoMethod);

    /**
     * Set request URI.
     *
     * @param[in] aoUri  request URI.
     */
    void SetRequestURI(std::string const& aoUri);

	virtual apl_int_t EncodeStartLine(acl::CMemoryBlock* apoBuffer);

    virtual apl_int_t DecodeStartLine(acl::CMemoryBlock* apoBuffer);

protected:
    std::string moMethod; //request method
    std::string moUri; //request URI

};



class CHttpResponse : public CHttpMessage
{
public:
    /**
     * Constructor.
     */
    CHttpResponse();

    /**
     * Destructor.
     */
    virtual ~CHttpResponse();

    /**
     * Get response reason phrase.
     *
     * @return response reason phrase.
     */
    std::string const& GetReasonPhrase();

    /**
     * Get response status code.
     *
     * @return response status code.
     */
    apl_uint_t GetStatusCode();

    /**
     * Set response reason phrase.
     *
     * @param[in] aoReasonPhrase  response reason phrase.
     */
    void SetReasonPhrase(std::string const& aoReasonPhrase);

    /**
     * Set response status code.
     *
     * @param[in] aiStatus  response status code.
     */
    void SetStatusCode(apl_uint32_t aiStatus);

	virtual apl_int_t EncodeStartLine(acl::CMemoryBlock* apoBuffer);

    virtual apl_int_t DecodeStartLine(acl::CMemoryBlock* apoBuffer);

protected:
    apl_uint32_t    muStatus; //response status code
    std::string     moReason; //response reason phrase
};


class CHttpInvalid : public CHttpMessage
{
public:

    CHttpInvalid();

	virtual apl_int_t EncodeStartLine(acl::CMemoryBlock* apoBuffer);

    virtual apl_int_t DecodeStartLine(acl::CMemoryBlock* apoBuffer);

    virtual void SetErrMsg(std::string const & aoErrMsg);

    virtual const char* GetErrMsg();

    virtual void SetErrCode(apl_int_t aiErrCode);

    virtual apl_int_t GetErrCode();

private:
    std::string moErrMsg;
    apl_int_t miErrCode;
};


CHttpMessage* DecodeStartLine(acl::CMemoryBlock* apoBuffer, apl_size_t* apuPkgLen);

}

ANF_NAMESPACE_END



#endif//HTTP_MESSAGE_H
