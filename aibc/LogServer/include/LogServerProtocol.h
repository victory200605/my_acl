#ifndef LOG_SERVER_PROTOCOL
#define LOG_SERVER_PROTOCOL

#include "apl/inttypes.h"
#include "apl/mem.h"
#include "LogServerMsg.h"


#define LOG_SERVER_PROTOCOL_TYPE_INVALID    0
#define LOG_SERVER_PROTOCOL_TYPE_RSP_MASK   (0x80000000) 
///common request
#define LOG_SERVER_PROTOCOL_TYPE_DATA_REQ        1
///common response
#define LOG_SERVER_PROTOCOL_TYPE_DATA_RSP        (LOG_SERVER_PROTOCOL_TYPE_DATA_REQ | LOG_SERVER_PROTOCOL_TYPE_RSP_MASK)
///handshake request
#define LOG_SERVER_PROTOCOL_TYPE_HEARTBEAT_REQ      2
///handshake response
#define LOG_SERVER_PROTOCOL_TYPE_HEARTBEAT_RSP      (LOG_SERVER_PROTOCOL_TYPE_HEARTBEAT_REQ | LOG_SERVER_PROTOCOL_TYPE_RSP_MASK) 


///fixed header length (bytes)
///LENGTH(4) + SEQUENCE(4) + TYPE(4) + MODE|STATUS(4)
const apl_uint32_t LOG_SERVER_PROTOCOL_HEADER_LEN = 16;


class CLogServerProtocol {
public:


    /** 
     * Constructor
     *
     * @param[in] apoLogServerMsg pointer to a CLogServerMsg
     *                            setting to NULL always use in two cases:
     *                            1.Decode header only. Content will be 
     *                            ignored, e.g., API would expect only a 
     *                            header rsp from Log Server.
     *                            2.encode header only. e.g. LogServer 
     *                            would only reply a header to API.
     */
	CLogServerProtocol(CLogServerMsg* apoLogServerMsg);

	~CLogServerProtocol();

    /**
     * Get the len of whole protocol message
     *
     * @retval  >0 the whole length of protocol message
     * @retval <=0 get failture
     */
    apl_uint32_t GetLen();

    /**
     * Set the sequence of protocol message
     *
     * @param[in] auSeq     the sequence to set
     */
    void SetSeq(apl_uint32_t auSeq);

    /**
     * Get the sequence of protocol message
     *
     * @return  the sequence of protocol message
     */
    apl_uint32_t GetSeq();

    /**
     * Set the type of protocol message
     *
     * @param[in] auType     the message type to set, should be one in 
     *                       LOG_SERVER_PROTOCOL_TYPE_*
     *
     */
    void SetType(apl_uint32_t auType);

    /**
     * Get the type of protocol message
     *
     * @retval  should be one in LOG_SERVER_PROTOCOL_TYPE_*
     *
     */
    apl_uint32_t GetType();

    /**
     * Set the mode of protocol message
     *
     * @param[in] auMode     the message type to set, should be one in 
     *                       LOG_SERVER_MODE_*
     *
     */
    void SetMode(apl_uint32_t auMode);

    /**
     * Get the mode of protocol message
     *
     * @retval  should be one in LOG_SERVER_MODE_*
     *
     */
    apl_uint32_t GetMode();

    /**
     * Set the response status of protocol message
     *
     * @param[in] aiStatus   The reponse status.
     *
     */
    void SetStatus(apl_int32_t aiStatus);

    /**
     * Get the response status of protocol message
     *
     * @retval  Any value, the status of response
     *
     */
    apl_int32_t GetStatus();

    /**
     * Get the LogServer message
     * Usally get this after decoded.
     *
     * @retval  The logserver message which binded this protocol
     *
     */
    CLogServerMsg* GetLogServerMsg();

    /**
     * Encode the header and mpoLogServerMsg(TLV) body to apcBuf.
     *
     * @param[out] apcBuf   The buffer to store the encode content
     * @param[in]  aiLen    The length of the buffer
     *
     * @retval >0 Encode success, return how many bytes we encoded.
     * @retval <0 Encode failture
     */
    apl_int_t Encode(char* apcBuf, apl_uint32_t auLen);

    /**
     * Dncode the header and TLV body from apcBuf.
     *
     * @param[in] apcBuf   The buffer stored the content which ready for decode
     * @param[in]  aiLen   The content length of the buffer
     *
     * @retval >0 Decode success, return how many bytes we decoded
     * @retval =0 Not enough content to decode.
     * @retval <0 Decode failture
     */
    apl_int_t Decode(char* apcBuf, apl_uint32_t auLen);

private:
    apl_uint32_t CalContentLen();
    apl_uint32_t CopyContentToBuf(char* apcBuf, apl_uint32_t auLen);

    apl_uint32_t muContentLen;
    apl_uint32_t muSeq;
    apl_uint32_t muMod;
    apl_int32_t  miStatus;
    apl_uint32_t muMsgType;

    CLogServerMsg* mpoLogServerMsg;

};

#endif // LOG_SERVER_PROTOCOL
