#ifndef SMPPMESSAGE_H 
#define SMPPMESSAGE_H

#include "apl/apl.h"
#include "acl/Utility.h"
#include <vector>
#include <string>


namespace smpp
{

#define BUILD_GETTER_SETTER_MU8(member)                                     \
            apl_uint8_t Get##member() const                                 \
            {                                                               \
                return mu8##member;                                         \
            }                                                               \
                                                                            \
            void Set##member(apl_uint8_t au8Value)                          \
            {                                                               \
                mu8##member = au8Value;                                     \
            }


#define BUILD_GETTER_SETTER_MU16(member)                                    \
            apl_uint16_t Get##member() const                                \
            {                                                               \
                return mu16##member;                                        \
            }                                                               \
                                                                            \
            void Set##member(apl_uint16_t au16Value)                        \
            {                                                               \
                mu16##member = au16Value;                                   \
            }

#define BUILD_GETTER_SETTER_MU32(member)                                    \
            apl_uint32_t Get##member() const                                \
            {                                                               \
                return mu32##member;                                        \
            }                                                               \
                                                                            \
            void Set##member(apl_uint32_t au32Value)                        \
            {                                                               \
                mu32##member = au32Value;                                   \
            }


#define BUILD_GETTER_SETTER_CSTR(member)                                    \
            const char * Get##member() const                                \
            {                                                               \
                return mac##member;                                         \
            }                                                               \
                                                                            \
            void Set##member( const char * apcValue)                        \
            {                                                               \
                apl_strncpy( mac##member, apcValue, sizeof(mac##member));                  \
            }

///////////////////////////////////////////////////////////////////////////
//
enum
{
    AI_SMPP_CMD_GENERIC_NACK            = 0x80000000,
    AI_SMPP_CMD_BIND_RECEIVER           = 0x00000001,
    AI_SMPP_CMD_BIND_RECEIVER_RESP      = 0x80000001,
    AI_SMPP_CMD_BIND_TRANSMITTER        = 0x00000002,
    AI_SMPP_CMD_BIND_TRANSMITTER_RESP   = 0x80000002,
    AI_SMPP_CMD_QUERY_SM                = 0x00000003,
    AI_SMPP_CMD_QUERY_SM_RESP           = 0x80000003,
    AI_SMPP_CMD_SUBMIT_SM               = 0x00000004,
    AI_SMPP_CMD_SUBMIT_SM_RESP          = 0x80000004,
    AI_SMPP_CMD_DELIVER_SM              = 0x00000005,
    AI_SMPP_CMD_DELIVER_SM_RESP         = 0x80000005,
    AI_SMPP_CMD_UNBIND                  = 0x00000006,
    AI_SMPP_CMD_UNBIND_RESP             = 0x80000006,
    AI_SMPP_CMD_REPLACE_SM              = 0x00000007,
    AI_SMPP_CMD_REPLACE_SM_RESP         = 0x80000007,
    AI_SMPP_CMD_CANCEL_SM               = 0x00000008,
    AI_SMPP_CMD_CANCEL_SM_RESP          = 0x80000008,
    AI_SMPP_CMD_BIND_TRANSCEIVER        = 0x00000009,
    AI_SMPP_CMD_BIND_TRANSCEIVER_RESP   = 0x80000009,
    AI_SMPP_CMD_OUTBIND                 = 0x0000000B,
    AI_SMPP_CMD_ENQUIRE_LINK            = 0x00000015,
    AI_SMPP_CMD_ENQUIRE_LINK_RESP       = 0x80000015,
    AI_SMPP_CMD_SUBMIT_MULTI            = 0x00000021,
    AI_SMPP_CMD_SUBMIT_MULTI_RESP       = 0x80000021,
    AI_SMPP_CMD_ALERT_NOTIFICATION      = 0x80000102,
    AI_SMPP_CMD_DATA_SM                 = 0x00000103,
    AI_SMPP_CMD_DATA_SM_RESP            = 0x80000103
};

enum
{
    AI_SMPP_STATUS_ROK              = 0x00000000,   //< No Error
    AI_SMPP_STATUS_RINVMSGLEN       = 0x00000001,   //< Message Length is invalid
    AI_SMPP_STATUS_RINVCMDLEN       = 0x00000002,   //< Command Length is invalid
    AI_SMPP_STATUS_RINVCMDID        = 0x00000003,   //< Invalid Command ID
    AI_SMPP_STATUS_RINVBNDSTS       = 0x00000004,   //< Incorrect BIND Status for given command
    AI_SMPP_STATUS_RALYBND          = 0x00000005,   //< ESME Already in Bound State
    AI_SMPP_STATUS_RINVPRTFLG       = 0x00000006,   //< Invalid Priority Flag
    AI_SMPP_STATUS_RINVREGDLVFLG    = 0x00000007,   //< Invalid Registered Delivery Flag
    AI_SMPP_STATUS_RSYSERR          = 0x00000008,   //< System Error
    AI_SMPP_STATUS_RINVSRCADR       = 0x0000000A,   //< Invalid Source Address
    AI_SMPP_STATUS_RINVDSTADR       = 0x0000000B,   //< Invalid Dest Addr
    AI_SMPP_STATUS_RINVMSGID        = 0x0000000C,   //< Message ID is invalid
    AI_SMPP_STATUS_RBINDFAIL        = 0x0000000D,   //< Bind Failed
    AI_SMPP_STATUS_RINVPASWD        = 0x0000000E,   //< Invalid Password
    AI_SMPP_STATUS_RINVSYSID        = 0x0000000F,   //< Invalid System ID
    AI_SMPP_STATUS_RCANCELFAIL      = 0x00000011,   //< Cancel SM Failed
    AI_SMPP_STATUS_RREPLACEFAIL     = 0x00000013,   //< Replace SM Failed 
    AI_SMPP_STATUS_RMSGQFUL         = 0x00000014,   //< Message Queue Full
    AI_SMPP_STATUS_RINVSERTYP       = 0x00000015,   //< Invalid Service Type
    AI_SMPP_STATUS_RINVNUMDESTS     = 0x00000033,   //< Invalid number of destinations
    AI_SMPP_STATUS_RINVDLNAME       = 0x00000034,   //< Invalid Distribution List name
    AI_SMPP_STATUS_RINVDESTFLAG     = 0x00000040,   //< Destination flag is invalid (submit_multi)
    AI_SMPP_STATUS_RINVSUBREP       = 0x00000042,   //< Invalid 'submit with replace' request 
                                                    //< (i.e. submit_sm with mu8Replace_if_present_flag set)
    AI_SMPP_STATUS_RINVESMCLASS     = 0x00000043,   //< Invalid mu8Esm_class field data
    AI_SMPP_STATUS_RCNTSUBDL        = 0x00000044,   //< Cannot Submit to Distribution List
    AI_SMPP_STATUS_RSUBMITFAIL      = 0x00000045,   //< submit_sm or submit_multi failed
    AI_SMPP_STATUS_RINVSRCTON       = 0x00000048,   //< Invalid Source address TON
    AI_SMPP_STATUS_RINVSRCNPI       = 0x00000049,   //< Invalid Source address NPI
    AI_SMPP_STATUS_RINVDSTTON       = 0x00000050,   //< Invalid Destination address TON
    AI_SMPP_STATUS_RINVDSTNPI       = 0x00000051,   //< Invalid Destination address NPI
    AI_SMPP_STATUS_RINVSYSTYP       = 0x00000053,   //< Invalid macSystem_type field
    AI_SMPP_STATUS_RINVREPFLAG      = 0x00000054,   //< Invalid replace_if_present flag
    AI_SMPP_STATUS_RINVNUMMSGS      = 0x00000055,   //< Invalid number of messages
    AI_SMPP_STATUS_RTHROTTLED       = 0x00000058,   //< Throttling error (ESME has exceeded allowed message limits)
    AI_SMPP_STATUS_RINVSCHED        = 0x00000061,   //< Invalid Scheduled Delivery Time
    AI_SMPP_STATUS_RINVEXPIRY       = 0x00000062,   //< Invalid message validity period (Expiry time)
    AI_SMPP_STATUS_RINVDFTMSGID     = 0x00000063,   //< Predefined Message Invalid or Not Found
    AI_SMPP_STATUS_RX_T_APPN        = 0x00000064,   //< ESME Receiver Temporary App Error Code
    AI_SMPP_STATUS_RX_P_APPN        = 0x00000065,   //< ESME Receiver Permanent App Error Code
    AI_SMPP_STATUS_RX_R_APPN        = 0x00000066,   //< ESME Receiver Reject Message Error Code
    AI_SMPP_STATUS_RQUERYFAIL       = 0x00000067,   //< query_sm request failed
    AI_SMPP_STATUS_RINVOPTPARSTREAM = 0x000000C0,   //< Error in the optional part of the PDU Body.
    AI_SMPP_STATUS_ROPTPARNOTALLWD  = 0x000000C1,   //< Optional Parameter not allowed
    AI_SMPP_STATUS_RINVPARLEN       = 0x000000C2,   //< Invalid Parameter Length.
    AI_SMPP_STATUS_RMISSINGOPTPARAM = 0x000000C3,   //< Expected Optional Parameter missing
    AI_SMPP_STATUS_RINVOPTPARAMVAL  = 0x000000C4,   //< Invalid Optional Parameter Value
    AI_SMPP_STATUS_RDELIVERYFAILURE = 0x000000FE,   //< Delivery Failure (used for data_sm_resp)
    AI_SMPP_STATUS_RUNKNOWNERR      = 0x000000FF    //< Unknown Error
};


enum
{
    AI_SMPP_TON_UNKNOWN             = 0x00, //< 00000000
    AI_SMPP_TON_INTERNATIONAL       = 0x01, //< 00000001
    AI_SMPP_TON_NATIONAL            = 0x02, //< 00000010
    AI_SMPP_TON_NETWORK_SPECIFIC    = 0x03, //< 00000011
    AI_SMPP_TON_SUBSCRIBER_NUMBER   = 0x04, //< 00000100
    AI_SMPP_TON_ALPHANUMERIC        = 0x05, //< 00000101
    AI_SMPP_TON_ABBREVIATED         = 0x06  //< 00000110
};

// TLV mu16tags
enum
{
    AI_SMPP_TAG_DEST_ADDR_SUBUNIT           = 0x0005,
    AI_SMPP_TAG_DEST_NETWORK_TYPE           = 0x0006,
    AI_SMPP_TAG_DEST_BEARER_TYPE            = 0x0007,
    AI_SMPP_TAG_DEST_TELEMATICS_ID          = 0x0008,
    AI_SMPP_TAG_SOURCE_ADDR_SUBUNIT         = 0x000D,
    AI_SMPP_TAG_SOURCE_NETWORK_TYPE         = 0x000E,
    AI_SMPP_TAG_SOURCE_BEARER_TYPE          = 0x000F,
    AI_SMPP_TAG_SOURCE_TELEMATICS_ID        = 0x0010,
    AI_SMPP_TAG_QOS_TIME_TO_LIVE            = 0x0017,
    AI_SMPP_TAG_PAYLOAD_TYPE                = 0x0019,
    AI_SMPP_TAG_ADDITIONAL_STATUS_INFO_TEXT = 0x001D,
    AI_SMPP_TAG_RECEIPTED_MESSAGE_ID        = 0x001E,
    AI_SMPP_TAG_MS_MSG_WAIT_FACILITIES      = 0x0030,
    AI_SMPP_TAG_PRIVACY_INDICATOR           = 0x0201,
    AI_SMPP_TAG_SOURCE_SUBADDRESS           = 0x0202,
    AI_SMPP_TAG_DEST_SUBADDRESS             = 0x0203,
    AI_SMPP_TAG_USER_MESSAGE_REFERENCE      = 0x0204,
    AI_SMPP_TAG_USER_RESPONSE_CODE          = 0x0205,
    AI_SMPP_TAG_SOURCE_PORT                 = 0x020A,
    AI_SMPP_TAG_DESTINATION_PORT            = 0x020B,
    AI_SMPP_TAG_SAR_MSG_REF_NUM             = 0x020C,
    AI_SMPP_TAG_LANGUAGE_INDICATOR          = 0x020D,
    AI_SMPP_TAG_SAR_TOTAL_SEGMENTS          = 0x020E,
    AI_SMPP_TAG_SAR_SEGMENT_SEQNUM          = 0x020F,
    AI_SMPP_TAG_SC_INTERFACE_VERSION        = 0x0210,
    AI_SMPP_TAG_CALLBACK_NUM_PRES_IND       = 0x0302,
    AI_SMPP_TAG_CALLBACK_NUM_ATAG           = 0x0303,
    AI_SMPP_TAG_NUMBER_OF_MESSAGES          = 0x0304,
    AI_SMPP_TAG_CALLBACK_NUM                = 0x0381,
    AI_SMPP_TAG_DPF_RESULT                  = 0x0420,
    AI_SMPP_TAG_SET_DPF                     = 0x0421,
    AI_SMPP_TAG_MS_AVAILABILITY_STATUS      = 0x0422,
    AI_SMPP_TAG_NETWORK_ERROR_CODE          = 0x0423,
    AI_SMPP_TAG_MESSAGE_PAYLOAD             = 0x0424,
    AI_SMPP_TAG_DELIVERY_FAILURE_REASON     = 0x0425,
    AI_SMPP_TAG_MORE_MESSAGES_TO_SEND       = 0x0426,
    AI_SMPP_TAG_MESSAGE_STATE               = 0x0427,
    AI_SMPP_TAG_USSD_SERVICE_OP             = 0x0501,
    AI_SMPP_TAG_DISPLAY_TIME                = 0x1201,
    AI_SMPP_TAG_SMS_SIGNAL                  = 0x1203,
    AI_SMPP_TAG_MS_VALIDITY                 = 0x1204,
    AI_SMPP_TAG_ALERT_ON_MESSAGE_DELIVERY   = 0x130C,
    AI_SMPP_TAG_ITS_REPLY_TYPE              = 0x1380,
    AI_SMPP_TAG_ITS_SESSION_INFO            = 0x1383
};

enum
{
    AI_SMPP_NPI_UNKNOWN         = 0x00, //< 00000000
    AI_SMPP_NPI_ISDN            = 0x01, //< 00000001 (E163/E164)
    AI_SMPP_NPI_DATA            = 0x03, //< 00000011 (X.121)
    AI_SMPP_NPI_TELEX           = 0x04, //< 00000100 (F.69)
    AI_SMPP_NPI_LAND_MOBILE     = 0x06, //< 00000110 (E.121)
    AI_SMPP_NPI_NATIONAL        = 0x08, //< 00001000
    AI_SMPP_NPI_PRIVATE         = 0x09, //< 00001001
    AI_SMPP_NPI_ERMES           = 0x0A, //< 00001010
    AI_SMPP_NPI_INTERNET        = 0x0E, //< 00001110 (IP)
    AI_SMPP_NPI_WAP_CLIENT_ID   = 0x12, //< 00010010
};

enum
{
    AI_SMPP_MULTI_ADDR_SMEADDR = 1,
    AI_SMPP_MULTI_ADDR_DLADDR = 2
};

enum
{
    AI_SMPP_ERROR_NOERROR = 0,
    AI_SMPP_ERROR_INVILD_COMMAND,
    AI_SMPP_ERROR_LEN_MESSAGE,
    AI_SMPP_ERROR_NOT_IMPLEMENT
};

const char* const AI_SMPP_STAT_DELIVRD = "DELIVRD";
const char* const AI_SMPP_STAT_EXPIRED = "EXPIRED";
const char* const AI_SMPP_STAT_DELETED = "DELETED";
const char* const AI_SMPP_STAT_UNDELIV = "UNDELIV";
const char* const AI_SMPP_STAT_ACCEPTD = "ACCEPTD";
const char* const AI_SMPP_STAT_UNKNOWN = "UNKNOWN";
const char* const AI_SMPP_STAT_REJECTD = "REJECTD";

const apl_uint_t AI_SMPP_HEADER_LEN = 16;

const apl_size_t AI_SMPP_MAXLEN_SYSTEM_ID = 16;
const apl_size_t AI_SMPP_MAXLEN_PASSWORD = 9;
const apl_size_t AI_SMPP_MAXLEN_SYSTEM_TYPE = 13;
const apl_size_t AI_SMPP_MAXLEN_ADDRESS_RANGE = 41;
const apl_size_t AI_SMPP_MAXLEN_SERVICE_TYPE = 6;
const apl_size_t AI_SMPP_MAXLEN_SOURCE_ADDR = 21;
const apl_size_t AI_SMPP_MAXLEN_DEST_ADDR = 21;
const apl_size_t AI_SMPP_MAXLEN_SCHEDULE_TIME = 17;
const apl_size_t AI_SMPP_MAXLEN_VALIDITY_PERIOD = 17;
const apl_size_t AI_SMPP_MAXLEN_MESSAGE_ID = 65;
const apl_size_t AI_SMPP_MAXLEN_FINAL_DATE = 17;
const apl_size_t AI_SMPP_MAXLEN_ESME_ADDR = 65;
const apl_size_t AI_SMPP_MAXLEN_SHORT_MESSAGE = 255;

const apl_size_t AI_SMPP_PACKAGE_MAXLEN = 10*1024; //max smpp package:10k

#define AI_SMPP_MIN(v1, v2) ((v1) < (v2) ? (v1) : (v2))

#define RETURN_ERR_IF(conditon, errno) \
do \
{ \
    if(conditon) \
    { \
        return errno; \
    } \
}while(0);

///////////////////////////////////////////////////////////////////////////
//
class SMPPMessage;

struct CSMPPHeader
{
    apl_uint32_t    mu32Command_length;
    apl_uint32_t    mu32Command_id; 
    apl_uint32_t    mu32Command_status;
    apl_uint32_t    mu32Sequence_number;

    CSMPPHeader();
    /*
     *@brief Encode a SMPP header
     *
     *This function shall encode a SMPP header into binary.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary header.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     *
     */
    apl_ssize_t Encode(void* apPtr, apl_size_t aiLen) const;
    
     /*
     *@brief Decode a SMPP header
     *
     *This function shall decode a binary SMPP header.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary header.
     *
     *@param[in] aiLen   The number of bytes of the header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    apl_ssize_t Decode(void const* apPtr, apl_size_t aiLen);
};

class CTLVItem
{
    friend class CSMPPMessage;

    apl_uint16_t mu16Tag;
    apl_uint16_t mu16Len;
    std::vector<apl_uint8_t> moValue;
public:
    CTLVItem();
    BUILD_GETTER_SETTER_MU16(Tag)

    apl_uint16_t GetLen() const
    {
        return mu16Len;
    }
    apl_int32_t GetValueInt32() const
    {
        apl_int32_t* lpu32 = (apl_int32_t *)&moValue[0];

        return apl_ntoh32(*lpu32);
    }
    apl_int16_t GetValueInt16() const
    {
        apl_int16_t* lpu16 = (apl_int16_t *)&moValue[0];

        return apl_ntoh16(*lpu16);
    }
    apl_int8_t GetValueInt8() const
    {
        return *(apl_int8_t *)&moValue[0];
    }
    std::string GetValueString() const
    {
        return std::string(moValue.begin(), moValue.end() );
    }

    void SetValueInt32(apl_int32_t au32Value)
    {
        apl_uint32_t lu32 = apl_hton32(au32Value);
        apl_uint8_t* lpu8 =(apl_uint8_t *)&lu32;

        moValue.clear();
        moValue.push_back(*lpu8++);
        moValue.push_back(*lpu8++);
        moValue.push_back(*lpu8++);
        moValue.push_back(*lpu8++);

        mu16Len = moValue.size();
    }
    void SetValueInt16(apl_int16_t au16Value)
    {
        apl_uint32_t lpu16 = apl_hton16(au16Value);
        apl_uint8_t* lpu8 =(apl_uint8_t *)&lpu16;

        moValue.clear();
        moValue.push_back(*lpu8++);
        moValue.push_back(*lpu8++);

        mu16Len = moValue.size();
    }
    void SetValueInt8(apl_int8_t au8Value)
    {
        moValue.clear();
        moValue.push_back(au8Value);

        mu16Len = moValue.size();
    }
    void SetValueString(std::string aoValue)
    {
        std::string::iterator loIter;

        moValue.clear();
        loIter = aoValue.begin();

        while( loIter != aoValue.end())
        {
            moValue.push_back( *loIter );
            ++loIter;
        }
        mu16Len = moValue.size();
    }
};

/*
 * SME_address and dest_address is used by submit_mulit 
 */
class CSMEAddress
{
    friend class CSubmitMultiPDU;

    apl_uint8_t mu8Dest_addr_ton;
    apl_uint8_t mu8Dest_addr_npi;
    char macDestination_addr[21];
public:
     BUILD_GETTER_SETTER_MU8(Dest_addr_ton)
     BUILD_GETTER_SETTER_MU8(Dest_addr_npi)
     BUILD_GETTER_SETTER_CSTR(Destination_addr)
};


class CAddress
{
    friend class CSubmitMultiPDU;

    apl_uint8_t mu8Dest_flag;
    union
    {
        CSMEAddress moSME_dest_address;
        char macDl_name[21];
    };

public:
    BUILD_GETTER_SETTER_MU8(Dest_flag)
    BUILD_GETTER_SETTER_CSTR(Dl_name);
    
    CSMEAddress GetSEMAddress() const
    {
        return moSME_dest_address;
    }

    void SetSEMAddress(CSMEAddress aoValue)
    {
        moSME_dest_address = aoValue;
    }
        
};

/*
 * CSMEUnsuccess is used by submit_mulit_resp
 * prevents a failed submited destination address list
 */
class CSMEUnsuccess
{
    friend class CSubmitMultiRespPDU;

    apl_uint8_t mu8Dest_addr_ton;
    apl_uint8_t mu8Dest_addr_npi;
    char macDestination_addr[21];
    apl_uint32_t mu32Error_status_code;

public:
    BUILD_GETTER_SETTER_MU8(Dest_addr_ton)
    BUILD_GETTER_SETTER_MU8(Dest_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Destination_addr)
    BUILD_GETTER_SETTER_MU32(Error_status_code)
};


struct IPDUBase
{
    // 20111206
    // -- begin
public:
    IPDUBase()
        : mbIsValidMsg(true)
    {
    }

    bool IsValidMsg(void)
    {
        return this->mbIsValidMsg;
    }

protected:
    void SetValidFlag(bool abIsValidMsg)
    {
        this->mbIsValidMsg = abIsValidMsg;
    }

    apl_ssize_t DecodeFieldCStr(char* const apcDest, const char* const apcSrc, const apl_int_t aiLen, apl_size_t &aiOff);
    apl_ssize_t DecodeFieldStr(char* const apcDest, const char* const apcSrc, const apl_int_t aiLen, apl_size_t &aiOff);
    apl_ssize_t DecodeField(apl_uint8_t* const aiDest, const void* const apcSrc, const apl_int_t aiLen, apl_size_t &aiOff);
    apl_ssize_t DecodeField(apl_uint32_t* const aiDest, const void* const apcSrc, const apl_int_t aiLen, apl_size_t &aiOff);
        

    bool mbIsValidMsg;

public:
    // -- end

    virtual ~IPDUBase() {};
public:
    /*
     *@brief Encode a SMPP PDU
     *
     *This function shall encode a binary SMPP PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const = 0;

    /*
     *@brief Decode a SMPP PDU
     *
     *This function shall decode a binary SMPP PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader) = 0;

    /** 
     * @brief PDU copy abstract method
     * 
     * @param apoPDU base pdu pointer
     */
    virtual void CopyFrom(IPDUBase* apoPDU) = 0;
};

struct CBindPDU : public IPDUBase
{
    char macSystem_id[16];
    char macPassword[9];
    char macSystem_type[13];
    apl_uint8_t mu8Interface_version;
    apl_uint8_t mu8Addr_ton;
    apl_uint8_t mu8Addr_npi;
    char macAddress_range[41];

public:
    CBindPDU();
    /*
     *@brief Encode a SMPP bind PDU
     *
     *This function shall encode a binary SMPP bind PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary bind PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP bind PDU
     *
     *This function shall decode a binary SMPP bind PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary bind PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);
    
    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(System_id)
    BUILD_GETTER_SETTER_CSTR(Password)
    BUILD_GETTER_SETTER_CSTR(System_type)
    BUILD_GETTER_SETTER_MU8(Interface_version)
    BUILD_GETTER_SETTER_MU8(Addr_ton)
    BUILD_GETTER_SETTER_MU8(Addr_npi)
    BUILD_GETTER_SETTER_CSTR(Address_range)
};

struct CBindRespPDU : public IPDUBase
{
    char macSystem_id[16];

public:
    CBindRespPDU();
    /*
     *@brief Encode a SMPP bind_resp PDU
     *
     *This function shall Encode a binary SMPP PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary bind_resp PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP bind_resp PDU
     *
     *This function shall Decode a binary SMPP bind_resp PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary bind_resp PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);
    
    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(System_id)
};

struct COutBindPDU : public IPDUBase
{
    char macSystem_id[16];
    char macPassword[9];

public:
    COutBindPDU();
    /*
     *@brief Encode a SMPP out_bind PDU
     *
     *This function shall Encode a binary SMPP PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes Encoded.
     *        Otherwise, the value -1 returned, Encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP out_bind PDU
     *
     *This function shall Decode a binary SMPP out_bind PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary out_bind PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes Decoded.
     *        Otherwise, the value -1 returned, Decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(System_id)
    BUILD_GETTER_SETTER_CSTR(Password)
};
    

struct CSubmitSMPDU : public IPDUBase
{
    char macService_type[6];

    apl_uint8_t mu8Source_addr_ton;
    apl_uint8_t mu8Source_addr_npi;
    char macSource_addr[21];
    apl_uint8_t mu8Dest_addr_ton;
    apl_uint8_t mu8Dest_addr_npi;
    char macDestination_addr[21];

    apl_uint8_t mu8Esm_class;
    apl_uint8_t mu8Protocol_id;
    apl_uint8_t mu8Priority_flag;
    char macSchedule_delivery_time[17];
    char macValidity_period[17];
    apl_uint8_t mu8Registered_delivery;
    apl_uint8_t mu8Replace_if_present_flag;
    apl_uint8_t mu8Data_coding;
    apl_uint8_t mu8SM_default_msg_id;
    apl_uint8_t mu8SM_length;
    char macShort_message[255];

public:
    CSubmitSMPDU();

    /*
     *@brief Encode a SMPP submit_sm PDU
     *
     *This function shall Encode a binary SMPP submit_sm PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary submit_sm PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, Encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP submit_sm PDU
     *
     *This function shall Decode a binary SMPP submit_sm PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary submit_sm PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(Service_type)

    BUILD_GETTER_SETTER_MU8(Source_addr_ton)
    BUILD_GETTER_SETTER_MU8(Source_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Source_addr)
    BUILD_GETTER_SETTER_MU8(Dest_addr_ton)
    BUILD_GETTER_SETTER_MU8(Dest_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Destination_addr)

    BUILD_GETTER_SETTER_MU8(Esm_class)
    BUILD_GETTER_SETTER_MU8(Protocol_id)
    BUILD_GETTER_SETTER_MU8(Priority_flag)
    BUILD_GETTER_SETTER_CSTR(Schedule_delivery_time)
    BUILD_GETTER_SETTER_CSTR(Validity_period)
    BUILD_GETTER_SETTER_MU8(Registered_delivery)
    BUILD_GETTER_SETTER_MU8(Replace_if_present_flag)
    BUILD_GETTER_SETTER_MU8(Data_coding)
    BUILD_GETTER_SETTER_MU8(SM_default_msg_id)
    BUILD_GETTER_SETTER_MU8(SM_length)

    const char* GetShort_message() const
    {
        return macShort_message;
    }

    void SetShort_message(const char* apcValue, apl_size_t aiLen)
    {
        mu8SM_length = std::min(AI_SMPP_MAXLEN_SHORT_MESSAGE, aiLen);

        apl_memcpy(macShort_message, apcValue, mu8SM_length);
    }
};


struct CSubmitSMRespPDU : public IPDUBase
{
    char macMessage_id[65];

public:
    CSubmitSMRespPDU();
    /*
     *@brief Encode a SMPP submit_sm_resp PDU
     *
     *This function shall Encode a binary SMPP submit_sm_resp PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary submit_sm_resp PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP submit_sm_resp PDU
     *
     *This function shall Decode a binary SMPP submit_sm_resp PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary submit_sm_resp PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(Message_id)
};


struct CDeliverSMPDU : public IPDUBase
{
    char macService_type[6];

    apl_uint8_t mu8Source_addr_ton;
    apl_uint8_t mu8Source_addr_npi;
    char macSource_addr[21];
    apl_uint8_t mu8Dest_addr_ton;
    apl_uint8_t mu8Dest_addr_npi;
    char macDestination_addr[21];

    apl_uint8_t mu8Esm_class;
    apl_uint8_t mu8Protocol_id;
    apl_uint8_t mu8Priority_flag;
    char macSchedule_delivery_time[17];
    char macValidity_period[17];
    apl_uint8_t mu8Registered_delivery;
    apl_uint8_t mu8Replace_if_present_flag;
    apl_uint8_t mu8Data_coding;
    apl_uint8_t mu8SM_default_msg_id;
    apl_uint8_t mu8SM_length;
    char macShort_message[255];

public:
    CDeliverSMPDU();
    /*
     *@brief Encode a SMPP deliver_sm PDU
     *
     *This function shall encode a binary SMPP deliver_sm PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary deliver_sm PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP deliver_sm PDU
     *
     *This function shall Decode a binary SMPP deliver_sm PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary deliver_sm PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(Service_type)

    BUILD_GETTER_SETTER_MU8(Source_addr_ton)
    BUILD_GETTER_SETTER_MU8(Source_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Source_addr)
    BUILD_GETTER_SETTER_MU8(Dest_addr_ton)
    BUILD_GETTER_SETTER_MU8(Dest_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Destination_addr)

    BUILD_GETTER_SETTER_MU8(Esm_class)
    BUILD_GETTER_SETTER_MU8(Protocol_id)
    BUILD_GETTER_SETTER_MU8(Priority_flag)
    BUILD_GETTER_SETTER_CSTR(Schedule_delivery_time)
    BUILD_GETTER_SETTER_CSTR(Validity_period)
    BUILD_GETTER_SETTER_MU8(Registered_delivery)
    BUILD_GETTER_SETTER_MU8(Replace_if_present_flag)
    BUILD_GETTER_SETTER_MU8(Data_coding)
    BUILD_GETTER_SETTER_MU8(SM_default_msg_id)
    BUILD_GETTER_SETTER_MU8(SM_length)

    const char* GetShort_message() const
    {
        return macShort_message;
    }

    void SetShort_message(const char* apcValue, apl_size_t aiLen)
    {
        mu8SM_length = std::min(AI_SMPP_MAXLEN_SHORT_MESSAGE, aiLen);

        apl_memcpy(macShort_message, apcValue, mu8SM_length);
    }
};

struct CDeliverSMRespPDU : public IPDUBase
{
    char macMessage_id[65];

public:
    CDeliverSMRespPDU();
    /*
     *@brief Encode a SMPP deliver_sm_resp PDU
     *
     *This function shall Encode a binary SMPP deliver_sm_resp PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary deliver_sm_resp PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP deliver_sm_resp PDU
     *
     *This function shall Decode a binary SMPP deliver_sm_resp PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary deliver_sm_resp PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(Message_id)
};

struct CDataSMPDU : public IPDUBase
{
    char macService_type[6];

    apl_uint8_t mu8Source_addr_ton;
    apl_uint8_t mu8Source_addr_npi;
    char macSource_addr[21];
    apl_uint8_t mu8Dest_addr_ton;
    apl_uint8_t mu8Dest_addr_npi;
    char macDestination_addr[21];

    apl_uint8_t mu8Esm_class;
    apl_uint8_t mu8Registered_delivery;
    apl_uint8_t mu8Data_coding;

public:
    CDataSMPDU();
    /*
     *@brief Encode a SMPP data_sm PDU
     *
     *This function shall Encode a binary SMPP data_sm PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary data_sm PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP data_sm PDU
     *
     *This function shall Decode a binary SMPP data_sm PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary data_sm PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(Service_type)

    BUILD_GETTER_SETTER_MU8(Source_addr_ton)
    BUILD_GETTER_SETTER_MU8(Source_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Source_addr)
    BUILD_GETTER_SETTER_MU8(Dest_addr_ton)
    BUILD_GETTER_SETTER_MU8(Dest_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Destination_addr)

    BUILD_GETTER_SETTER_MU8(Esm_class)
    BUILD_GETTER_SETTER_MU8(Registered_delivery)
    BUILD_GETTER_SETTER_MU8(Data_coding)
};

struct CDataSMRespPDU : public IPDUBase
{
    char macMessage_id[65];

public:
    CDataSMRespPDU();
    /*
     *@brief Encode a SMPP data_sm_resp PDU
     *
     *This function shall Encode a binary SMPP data_sm_resp PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary data_sm_resp PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes Encoded.
     *        Otherwise, the value -1 returned, Encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP data_sm_resp PDU
     *
     *This function shall Decode a binary SMPP data_sm_resp PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary data_sm_resp PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(Message_id)
};

struct CCancelSMPDU : public IPDUBase
{
    char macService_type[6];
    char macMessage_id[65];

    apl_uint8_t mu8Source_addr_ton;
    apl_uint8_t mu8Source_addr_npi;
    char macSource_addr[21];
    apl_uint8_t mu8Dest_addr_ton;
    apl_uint8_t mu8Dest_addr_npi;
    char macDestination_addr[21];

public:
    CCancelSMPDU();
    /*
     *@brief Encode a SMPP cancel_sm PDU
     *
     *This function shall Encode a binary SMPP cancel_sm PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary cancel_sm PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP cancel_sm PDU
     *
     *This function shall Decode a binary SMPP cancel_sm PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary cancel_sm PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(Service_type)
    BUILD_GETTER_SETTER_CSTR(Message_id)

    BUILD_GETTER_SETTER_MU8(Source_addr_ton)
    BUILD_GETTER_SETTER_MU8(Source_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Source_addr)
    BUILD_GETTER_SETTER_MU8(Dest_addr_ton)
    BUILD_GETTER_SETTER_MU8(Dest_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Destination_addr)
};

struct CReplaceSMPDU : public IPDUBase
{
    char macMessage_id[65];

    apl_uint8_t mu8Source_addr_ton;
    apl_uint8_t mu8Source_addr_npi;
    char macSource_addr[21];

    char macSchedule_delivery_time[17];
    char macValidity_period[17];
    apl_uint8_t mu8Registered_delivery;
    apl_uint8_t mu8SM_default_msg_id;
    apl_uint8_t mu8SM_length;
    char macShort_message[255];

public:
    CReplaceSMPDU();
    /*
     *@brief Encode a SMPP replace_sm PDU
     *
     *This function shall Encode a binary SMPP replace_sm PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary replace_sm PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP replace_sm PDU
     *
     *This function shall Decode a binary SMPP replace_sm PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary replace_sm PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(Message_id) 
    
    BUILD_GETTER_SETTER_MU8(Source_addr_ton)
    BUILD_GETTER_SETTER_MU8(Source_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Source_addr)

    BUILD_GETTER_SETTER_CSTR(Schedule_delivery_time)
    BUILD_GETTER_SETTER_CSTR(Validity_period)
    BUILD_GETTER_SETTER_MU8(Registered_delivery)
    BUILD_GETTER_SETTER_MU8(SM_default_msg_id)
    BUILD_GETTER_SETTER_MU8(SM_length)

    const char* GetShort_message() const
    {
        return macShort_message;
    }

    void SetShort_message(const char* apcValue, apl_size_t aiLen)
    {
        mu8SM_length = std::min(AI_SMPP_MAXLEN_SHORT_MESSAGE, aiLen);

        apl_memcpy(macShort_message, apcValue, mu8SM_length);
    }
};

struct CQuerySMPDU : public IPDUBase
{
    char macMessage_id[65];

    apl_uint8_t mu8Source_addr_ton;
    apl_uint8_t mu8Source_addr_npi;
    char macSource_addr[21];

public:
    CQuerySMPDU();
    /*
     *@brief Encode a SMPP query_sm PDU
     *
     *This function shall Encode a binary SMPP query_sm PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary query_sm PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP query_sm PDU
     *
     *This function shall Decode a binary SMPP query_sm PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary query_sm PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(Message_id) 

    BUILD_GETTER_SETTER_MU8(Source_addr_ton)
    BUILD_GETTER_SETTER_MU8(Source_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Source_addr)
};

struct CQuerySMRespPDU : public IPDUBase
{
    char macMessage_id[65];
    char macFinal_date[17];

    apl_uint8_t mu8Message_state;
    apl_uint8_t mu8Error_code;

public:
    CQuerySMRespPDU();
    /*
     *@brief Encode a SMPP query_sm_resp PDU
     *
     *This function shall Encode a binary SMPP query_sm_resp PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary query_sm_resp PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP query_sm_resp PDU
     *
     *This function shall Decode a binary SMPP query_sm_resp PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary query_sm_resp PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(Message_id) 
    BUILD_GETTER_SETTER_CSTR(Final_date)

    BUILD_GETTER_SETTER_MU8(Message_state)
    BUILD_GETTER_SETTER_MU8(Error_code)
};

struct CSubmitMultiPDU : public IPDUBase
{
    char macService_type[6];

    apl_uint8_t mu8Source_addr_ton;
    apl_uint8_t mu8Source_addr_npi;
    char macSource_addr[21];
    apl_uint8_t mu8Number_of_dests;
    std::vector<CAddress> moDest_address;

    apl_uint8_t mu8Esm_class;
    apl_uint8_t mu8Protocol_id;
    apl_uint8_t mu8Priority_flag;
    char macSchedule_delivery_time[17];
    char macValidity_period[17];
    apl_uint8_t mu8Registered_delivery;
    apl_uint8_t mu8Replace_if_present_flag;
    apl_uint8_t mu8Data_coding;
    apl_uint8_t mu8SM_default_msg_id;
    apl_uint8_t mu8SM_length;
    char macShort_message[255];

public:
    CSubmitMultiPDU();
    /*
     *@brief Encode a SMPP submit_multi PDU
     *
     *This function shall Encode a binary SMPP submit_multi PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary submit_multi PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP submit_multi PDU
     *
     *This function shall Decode a binary SMPP submit_multi PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary submit_multi PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(Service_type)

    BUILD_GETTER_SETTER_MU8(Source_addr_ton)
    BUILD_GETTER_SETTER_MU8(Source_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Source_addr)

    BUILD_GETTER_SETTER_MU8(Esm_class)
    BUILD_GETTER_SETTER_MU8(Protocol_id)
    BUILD_GETTER_SETTER_MU8(Priority_flag)
    BUILD_GETTER_SETTER_CSTR(Schedule_delivery_time)
    BUILD_GETTER_SETTER_CSTR(Validity_period)
    BUILD_GETTER_SETTER_MU8(Registered_delivery)
    BUILD_GETTER_SETTER_MU8(Replace_if_present_flag)
    BUILD_GETTER_SETTER_MU8(Data_coding)
    BUILD_GETTER_SETTER_MU8(SM_default_msg_id)
    BUILD_GETTER_SETTER_MU8(SM_length)

    CAddress GetDest_address(apl_uint_t aiIndex) const
    {
        return moDest_address[aiIndex];
    }

    void RemoveDest_address(apl_uint_t aiIndex)
    {
        std::vector<CAddress>::iterator loIter = moDest_address.begin();

        std::advance(loIter, aiIndex);
        moDest_address.erase(loIter); 
        mu8Number_of_dests = moDest_address.size();
    }
    
    void AddDest_address(CAddress aoValue)
    {
        moDest_address.push_back(aoValue);
        mu8Number_of_dests = moDest_address.size();
    }

    apl_uint8_t GetNumber_of_dests() const
    {
        return mu8Number_of_dests;
    }

    const char* GetShort_message() const
    {
        return macShort_message;
    }

    void SetShort_message(const char* apcValue, apl_size_t aiLen)
    {
        mu8SM_length = std::min(AI_SMPP_MAXLEN_SHORT_MESSAGE, aiLen);

        apl_memcpy(macShort_message, apcValue, mu8SM_length);
    }
};

struct CSubmitMultiRespPDU : public IPDUBase
{
    char macMessage_id[65];

    apl_uint8_t mu8No_unsuccess;
    std::vector<CSMEUnsuccess> moUnsuccess_sme;
public:
    CSubmitMultiRespPDU();
    /*
     *@brief Encode a SMPP submit_multi_resp PDU
     *
     *This function shall Encode a binary SMPP submit_multi_resp PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary submit_multi_resp PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP submit_multi_resp PDU
     *
     *This function shall Decode a binary SMPP submit_multi_resp PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary submit_multi_resp PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_CSTR(Message_id)

    apl_uint8_t GetNo_unsuccess() const
    {
        return mu8No_unsuccess;
    }

    CSMEUnsuccess GetUnsuccess_sme(apl_uint_t aiIndex) const
    {
        return moUnsuccess_sme[aiIndex];
    }

    void RemoveUnsuccess_sme(apl_uint_t aiIndex)
    {
        std::vector<CSMEUnsuccess>::iterator loIter = moUnsuccess_sme.begin();

        std::advance(loIter, aiIndex);
        moUnsuccess_sme.erase(loIter);
        mu8No_unsuccess = moUnsuccess_sme.size();
    }
    
    void AddUnsuccess_sme(CSMEUnsuccess aoValue)
    {
        moUnsuccess_sme.push_back(aoValue);
        mu8No_unsuccess = moUnsuccess_sme.size();
    }
};

struct CAlertNotificationPDU : public IPDUBase
{
    apl_uint8_t mu8Source_addr_ton;
    apl_uint8_t mu8Source_addr_npi;
    char macSource_addr[21];

    apl_uint8_t mu8Esme_addr_ton;
    apl_uint8_t mu8Esme_addr_npi;
    char macEsme_addr[65];
public:
    CAlertNotificationPDU();
    /*
     *@brief Encode a SMPP alert_notification PDU
     *
     *This function shall Encode a binary SMPP alert_notification PDU.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary alert_notification PDU.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    virtual apl_ssize_t Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP alert_notification PDU
     *
     *This function shall Decode a binary SMPP alert_notification PDU.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary alert_notification PDU.
     *
     *@param[in] aiLen   The number of bytes of the PDU.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    virtual apl_ssize_t Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader);

    virtual void CopyFrom(IPDUBase* apoPDU);

    BUILD_GETTER_SETTER_MU8(Source_addr_ton)
    BUILD_GETTER_SETTER_MU8(Source_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Source_addr)
    BUILD_GETTER_SETTER_MU8(Esme_addr_ton)
    BUILD_GETTER_SETTER_MU8(Esme_addr_npi)
    BUILD_GETTER_SETTER_CSTR(Esme_addr)
};


class CSMPPMessage
{
public:
    CSMPPMessage();
    ~CSMPPMessage();
public:
    /*
     *@brief Encode a SMPP packet
     *
     *This function shall Encode a binary SMPP packet.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary packet.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    apl_ssize_t Encode(void* apPtr, apl_size_t aiLen);

    /*
     *@brief Decode a SMPP packet 
     *
     *This function shall Decode a binary SMPP packet.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary packet.
     *
     *@param[in] aiLen   The number of bytes of the packet.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    apl_ssize_t Decode(void const* apPtr, apl_size_t aiLen);
    
    apl_uint32_t GetLength() const;
    apl_uint32_t GetCommandID() const;
    apl_uint32_t GetStatus() const;
    apl_uint32_t GetSequence() const;
    IPDUBase* GetPDU();

    void SetCommandID(apl_uint32_t auCommand);
    void SetSequence(apl_uint32_t auSequence);
    void SetStatus(apl_uint32_t auStatus);
    void SetPDU(const IPDUBase* apoPDU);
    IPDUBase* AllocPDU();

    void RemoveTLVItem(apl_uint32_t auIndex);
    void AddTLV(CTLVItem aoItem);
    apl_size_t GetTLVCount() const;
    CTLVItem GetTLVItem(apl_uint32_t auIndex);

    void SetMessageID(apl_uint64_t au64MsgID)
    {
        mu64MessageID = au64MsgID;
    }

    apl_uint64_t GetMessageID() const
    {
        return mu64MessageID;
    }

public:
    CSMPPMessage& operator=(const CSMPPMessage& aoMessage);

private:
    /*
     *@brief Encode a SMPP TLV 
     *
     *This function shall Encode a binary SMPP TLV.
     *
     *@param[out] apPtr  Pointer to an memory that receive the binary TLV.
     *
     *@param[in] aiLen   The number of bytes in the memory that will be used for receiving.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes encoded.
     *        Otherwise, the value -1 returned, encode failed.
     */
    apl_ssize_t EncodeTLV(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const;

    /*
     *@brief Decode a SMPP TLV
     *
     *This function shall Decode a binary SMPP TLV.
     *
     *@param[in] apPtr  Pointer to an memory that contain the binary TLV.
     *
     *@param[in] aiLen   The number of bytes of the TLV.
     *
     *@param[in] aoHeader The SMPP header.
     *
     *@retval If no error occurs, returns the number of bytes decoded.
     *        Otherwise, the value -1 returned, decode failed.
     */
    apl_ssize_t DecodeTLV(void const* apPtr, apl_size_t aiLen, const CSMPPHeader& Header);

private:
    IPDUBase* mpoPDU;
    CSMPPHeader moHeader;
    apl_uint64_t mu64MessageID;
    apl_int32_t miSMPPErrno;
    std::vector<CTLVItem> moTLVTable;
};

// public functions
apl_ssize_t DecodeField(char* const apcDest, const char* const apcSrc, apl_size_t aiLen);
apl_ssize_t DecodeField(apl_uint8_t* const aiDest, const void* const apcSrc);
apl_ssize_t DecodeField(apl_uint16_t* const aiDest, const void* const apcSrc);
apl_ssize_t DecodeField(apl_uint32_t* const aiDest, const void* const apcSrc);
apl_ssize_t DecodeFieldCStr(char* const apcDest, const char* const apcSrc, apl_size_t aiLen);
apl_ssize_t DecodeFieldStr(char* const apcDest, const char* const apcSrc, apl_size_t aiLen);

apl_ssize_t EncodeField(char* const apcDest, const char* const apcSrc, apl_size_t aiLen);
apl_ssize_t EncodeField(void* const apcDest, const apl_uint8_t aiSrc);
apl_ssize_t EncodeField(void* const apcDest, const apl_uint16_t aiSrc);
apl_ssize_t EncodeField(void* const apcDest, const apl_uint32_t aiSrc);
apl_ssize_t EncodeFieldCStr(char* const apcDest, const char* const apcSrc, apl_size_t aiLen);
apl_ssize_t EncodeFieldStr(char* const apcDest, const char* const apcSrc, apl_size_t aiLen);

} // namespace smpp

#endif
