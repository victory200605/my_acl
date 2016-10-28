#ifndef __AILIBEX__AISMPPSTRU_H__
#define __AILIBEX__AISMPPSTRU_H__

//#include <stdint.h>

#include "AITime.h"
#include "AIChunkEx.h"

///start namespace
AIBC_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////
//
enum
{
    AI_SMPP_CMD_GENERIC_NACK            = 0x80000000,
    AI_SMPP_CMD_BIND_RECEIVER           = 0x00000001,
    AI_SMPP_CMD_BIND_RECEIVER_RESP      = 0x80000001,
    AI_SMPP_CMD_BIND_TRANSMITTER        = 0x00000002,
    AI_SMPP_CMD_BIND_TRANSMITTER_RESP   = 0x80000002,
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
    AI_SMPP_CMD_OUTBIND                 = 0x8000000B,
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
    AI_SMPP_STATUS_RINVSUBREP       = 0x00000042,   //< Invalid 'submit with replace' request (i.e. submit_sm with replace_if_present_flag set)
    AI_SMPP_STATUS_RINVESMCLASS     = 0x00000043,   //< Invalid esm_class field data
    AI_SMPP_STATUS_RCNTSUBDL        = 0x00000044,   //< Cannot Submit to Distribution List
    AI_SMPP_STATUS_RSUBMITFAIL      = 0x00000045,   //< submit_sm or submit_multi failed
    AI_SMPP_STATUS_RINVSRCTON       = 0x00000048,   //< Invalid Source address TON
    AI_SMPP_STATUS_RINVSRCNPI       = 0x00000049,   //< Invalid Source address NPI
    AI_SMPP_STATUS_RINVDSTTON       = 0x00000050,   //< Invalid Destination address TON
    AI_SMPP_STATUS_RINVDSTNPI       = 0x00000051,   //< Invalid Destination address NPI
    AI_SMPP_STATUS_RINVSYSTYP       = 0x00000053,   //< Invalid system_type field
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

#define AI_SMPP_STAT_DELIVRD    "DELIVRD"
#define AI_SMPP_STAT_EXPIRED    "EXPIRED"
#define AI_SMPP_STAT_DELETED    "DELETED"
#define AI_SMPP_STAT_UNDELIV    "UNDELIV"
#define AI_SMPP_STAT_ACCEPTD    "ACCEPTD"
#define AI_SMPP_STAT_UNKNOWN    "UNKNOWN"
#define AI_SMPP_STAT_REJECTD    "REJECTD"


///////////////////////////////////////////////////////////////////////////
//
struct SMPP_HEADER
{
    uint32_t    command_length;
    uint32_t    command_id;
    uint32_t    command_status;
    uint32_t    sequence_number;

    ssize_t Encode(void* apPtr, size_t aiLen) const;
    ssize_t Decode(void const* apPtr, size_t aiLen);
};

struct SMPP_BIND
{
    SMPP_HEADER     coHeader;

    char            system_id[16];
    char            password[9];
    char            system_type[13];
    uint8_t         interface_version;
    uint8_t         addr_ton;
    uint8_t         addr_npi;
    char            address_range[41];

    ssize_t Encode(void* apPtr, size_t aiLen) const;
    ssize_t Decode(void const* apPtr, size_t aiLen);
};


struct SMPP_BIND_RESP
{
    SMPP_HEADER     coHeader;

    char            system_id[16];

    ssize_t Encode(void* apPtr, size_t aiLen) const;
    ssize_t Decode(void const* apPtr, size_t aiLen);
};


typedef SMPP_BIND SMPP_BIND_RECEIVER;
typedef SMPP_BIND SMPP_BIND_TRANSMITTER;
typedef SMPP_BIND SMPP_BIND_TRANSCEIVER;

typedef SMPP_BIND_RESP SMPP_BIND_RECEIVER_RESP;
typedef SMPP_BIND_RESP SMPP_BIND_TRANSMITTER_RESP;
typedef SMPP_BIND_RESP SMPP_BIND_TRANSCEIVER_RESP;


struct SMPP_SM
{
    SMPP_HEADER     coHeader;
    
    char            service_type[6];
    uint8_t         source_addr_ton;
    uint8_t         source_addr_npi;
    char            source_addr[21];
    uint8_t         dest_addr_ton;
    uint8_t         dest_addr_npi;
    char            destination_addr[21];
    uint8_t         esm_class;
    uint8_t         protocol_id;
    uint8_t         priority_flag;
    char            schedule_delivery_time[17];
    char            validity_period[17];
    uint8_t         registered_delivery; 
    uint8_t         replace_if_present_flag;
    uint8_t         data_coding;
    uint8_t         sm_default_msg_id;
    uint8_t         sm_length;
    char            short_message[256];

    ssize_t Encode(void* apPtr, size_t aiLen) const;
    ssize_t Decode(void const* apPtr, size_t aiLen);
};

typedef SMPP_SM SMPP_SUBMIT_SM;
typedef SMPP_SM SMPP_DELIVER_SM;


struct SMPP_SUBMIT_SM_RESP
{
    SMPP_HEADER     coHeader;

    char            message_id[65];

    ssize_t Encode(void* apPtr, size_t aiLen) const;
    ssize_t Decode(void const* apPtr, size_t aiLen);
};


struct SMPP_DELIVER_SM_RESP
{
    SMPP_HEADER     coHeader;

    char            message_id[65];

    ssize_t Encode(void* apPtr, size_t aiLen) const;
    ssize_t Decode(void const* apPtr, size_t aiLen);
};


/**
 * parse smpp time (both absolute time and relative time) 
 * @param apcSMPPTime
 * @param aiCurTime current time when handle smpp relative
 * @return -1        - error time format
 *         otherwise - parsed absolute time 
 */
extern AITime_t AIParseSMPPTime(char const* apcSMPPTime, AITime_t aiCurTime = AICurTime());


extern int AIFormatAbsSMPPTime(char* apcSMPPTime, size_t aiSize, AITime_t aiTime, AITime_t aiTimeZone = AIGetTimeZone());

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AISMPPSTRU_H__

