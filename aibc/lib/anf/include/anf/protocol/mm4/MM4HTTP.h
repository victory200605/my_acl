#ifndef MM4_WSP_H
#define MM4_WSP_H

#include <string>
#include <list>
#include "anf/Utility.h"
#include "acl/StrAlgo.h"
#include "acl/SString.h"
#include "acl/MemoryBlock.h"
#include "acl/Codecs.h"

ANF_NAMESPACE_START

namespace mm4_http{

#define MMS_VERSION                                     "6.3.0"
#define TOKEN_CRLF                                      "\r\n"
#define TOKEN_CRLFCRLF                                  "\r\n\r\n"
#define BOUNDARY_NEXT_PART                              "--NextPart_0_2817_24856"
#define BOUNDARY_SUB_PART                               "SubPart_0_2817_24856"
#define MAX_LENGTH                                      4096

#define BUILD_GETTER_SETTER_INT(member)                         \
        apl_int_t const& Get##member()                          \
        {                                                       \
            return mi##member;                                  \
        }                                                       \
                                                                \
        void Set##member(apl_int_t const& aiValue)              \
        {                                                       \
            mi##member = aiValue;                               \
        }  

#define BUILD_GETTER_SETTER_STR(member)                         \
        std::string const& Get##member()                        \
        {                                                       \
            return mo##member;                                  \
        }                                                       \
                                                                \
        void Set##member(std::string const& aoValue)            \
        {                                                       \
            mo##member = aoValue;                               \
        }                                                       
                                                                
#define BUILD_GETTER_SETTER_LIST(member)                        \
        std::list<std::string> const& Get##member()             \
        {                                                       \
            return mo##member;                                  \
        }                                                       \
                                                                \
        void Set##member(std::string const& aoValue)            \
        {                                                       \
            mo##member.push_back(aoValue);                      \
        }

#define RETURN_ERR_IF(conditon, errno)                          \
        do                                                      \
        {                                                       \
            if(conditon)                                        \
            {                                                   \
                return errno;                                   \
            }                                                   \
        }while(0);

#define RESIZE_IF(memoryblock, length)                          \
        if(memoryblock.GetSpace() < length)                     \
        {                                                       \
            memoryblock.Resize(memoryblock.GetLength()+length+1);\
        }

/*message type*/
enum
{
    MESSAGE_TYPE_CAPABILIT_REQ          = 0x50, //0x50|0x80 = 208
    MESSAGE_TYPE_CAPABILIT_RES          = 0x51, //0x51|0x80 = 209
    MESSAGE_TYPE_FORWARD_REQ            = 0x52, //0x52|0x80 = 210
    MESSAGE_TYPE_FORWARD_RES            = 0x53, //0x53|0x80 = 211
    MESSAGE_TYPE_DELIVERY_REPORT_REQ    = 0x54, //0x54|0x80 = 212
    MESSAGE_TYPE_DELIVERY_REPORT_RES    = 0x55, //0x55|0x80 = 213
    MESSAGE_TYPE_READ_REPLY_REPORT_REQ  = 0x56, //0x56|0x80 = 214
    MESSAGE_TYPE_READ_REPLY_REPORT_RES  = 0x57  //0x57|0x80 = 215
};

/*yes, no*/
enum
{
    TYPE_YES    = 0x00, //0x00|0x80 = 128
    TYPE_NO     = 0x01  //0x01|0x80 = 129
};

/*message class*/
enum
{
    MESSAGE_CLASS_PERSONAL              = 0x00, //0x00|0x80 = 128
    MESSAGE_CLASS_ADVERTISEMENT         = 0x01, //0x01|0x80 = 129
    MESSAGE_CLASS_INFOMATIONAL          = 0x02, //0x02|0x80 = 130
    MESSAGE_CLASS_ATUO                  = 0x03, //0x03|0x80 = 131 
    MESSAGE_CLASS_TEST                  = 0x47  //0x47|0x80 = 199
};

/*priority*/
enum
{
    MESSAGE_PRIORITY_LOW                = 0x00, //0x00|0x80 = 128
    MESSAGE_PRIORITY_NORMAL             = 0x01, //0x01|0x80 = 129
    MESSAGE_PRIORITY_HIGH               = 0x02  //0x02|0x80 = 130
};

/*sender visibility*/
enum
{
    SENDER_VISIBILIT_HIDE               = 0x00, //0x00|0x80 = 128
    SENDER_VISIBILIT_SHOW               = 0x01  //0x01|0x80 = 129
};

/*request status code*/
enum
{
    REQ_STATUS_CODE_OK                          = 0x00, //0x00|0x80 = 128
    REQ_STATUS_CODE_UNSPECIFIED                 = 0x01, //0x01|0x80 = 129
    REQ_STATUS_CODE_SERVICE_DENIED              = 0x02, //0x02|0x80 = 130
    REQ_STATUS_CODE_MESSAGE_FORMAT_CORRUPT      = 0x03, //0x03|0x80 = 131
    REQ_STATUS_CODE_SENDING_ADDRESS_UNRESOLVED  = 0x04, //0x04|0x80 = 132
    REQ_STATUS_CODE_MESSAGE_NOT_FOUND           = 0x05, //0x05|0x80 = 133
    REQ_STATUS_CODE_NETWORK_PROBLEM             = 0x06, //0x06|0x80 = 134
    REQ_STATUS_CODE_CONTENT_NOT_ACCEPTED        = 0x07, //0x07|0x80 = 135
    REQ_STATUS_CODE_UNSUPPORTED_MESSAGE         = 0x08, //0x08|0x80 = 136
    REQ_STATUS_CODE_TEST_FAILED                 = 0x47  //0x47|0x80 = 199               
};

/*mm staus code*/
enum
{
    MM_STATUS_CODE_EXPIRED              = 0x00, //0x00|0x80 = 128
    MM_STATUS_CODE_RETRIEVED            = 0x01, //0x01|0x80 = 129
    MM_STATUS_CODE_REJECTED             = 0x02, //0x02|0x80 = 130
    MM_STATUS_CODE_DEFERRED             = 0x03, //0x03|0x80 = 131
    MM_STATUS_CODE_UNRECOGNIZED         = 0x04, //0x04|0x80 = 132
    MM_STATUS_CODE_INDETERMINATE        = 0x05, //0x05|0x80 = 133
    MM_STATUS_CODE_FORWARDED            = 0x06, //0x06|0x80 = 134
    MM_STATUS_CODE_FORWARDED_TO_MMBOX   = 0x07  //0x07|0x80 = 135
};

/*mm status extension*/
enum
{
    MM_STATUS_EXTENSION_REJECTED_BY_RECIPIENT       = 0x00, //0x00|0x80 = 128
    MM_STATUS_EXTENSION_REJECTED_BY_OTHER_MMSC      = 0x01, //0x01|0x80 = 129
    MM_STATUS_EXTENSION_REJECTED_BY_VAS_AREA_LIMIT  = 0x02  //0x02|0x80 = 130
};

/*read status*/
enum
{
    READ_STATUS_READ                        = 0x00, //0x00|0x80 = 128
    READ_STATUS_DELETED_WITHOUT_BEING_READ  = 0x01  //0x01|0x80 = 129
};

/*error code*/
enum
{
        MM4_ERROR_MMS_VERSION                            = -1,
        MM4_ERROR_TRANSACTION_ID                         = -2,
        MM4_ERROR_MESSAGE_ID                             = -3,
        MM4_ERROR_TO                                     = -4,
        MM4_ERROR_FROM                                   = -5,
        MM4_ERROR_DELIVERY_REPORT                        = -6,
        MM4_ERROR_ORIGINATOR_RS_DELIVERY_REPORT          = -7,
        MM4_ERROR_DATE                                   = -8,
        MM4_ERROR_CONTENT_TYPE                           = -9,
        MM4_ERROR_REQUEST_STATUS_CODE                    = -10,
        MM4_ERROR_FORWARD_TO_ORIGINATOR_UA               = -11,
        MM4_ERROR_MM_STATUS_CODE                         = -12,
        MM4_ERROR_READ_STATUS                            = -13,
        MM4_ERROR_ORIGINATOR_SYSTEM                      = -14,
        MM4_ERROR_RCPT_To                                = -15,
        MM4_ERROR_REPLY_CHARGING_SUPPORTING              = -16,
        MM4_ERROR_READ_CHARGING_SUPPORTING               = -17,

        MM4_DECODE_ERROR_TYPE_VALUE                      = -100,
        MM4_DECODE_ERROR_NULL_STRING                     = -101,
        MM4_DECODE_ERROR_CONTENT_TYPE                    = -102,
        MM4_DECODE_ERROR_FROM                            = -103,
        MM4_DECODE_ERROR_NOT_NULL_STRING                 = -104,
        MM4_DECODE_ERROR_NAME                            = -105,
        MM4_DECODE_ERROR_ENCODE_STRING                   = -106,
        MM4_DECODE_ERROR_MESSAGE_TYPE                    = -107,
        MM4_DECODE_ERROR_LONG_INT_LENGTH                 = -108,
        MM4_DECODE_ERROR_PREVIOUSLY_SENT_BY              = -109,
        MM4_DECODE_ERROR_PREVIOUSLY_SENT_DATE_AND_TIME   = -110,
        MM4_DECODE_ERROR_CONTENT                         = -111,
};

//value type
enum
{
    VALUE_TYPE_OCTETS_WITH_LENGTH   = 0x01, //octet string defined by length
    VALUE_TYPE_SHORT_INT            = 0x02, //short integer, range 0-127
    VALUE_TYPE_NULL_END_STRING      = 0x03, //0-terminated string
    VALUE_TYPE_NONE                 = 0x04
};

//boundary type
enum
{
    BOUNDARY_TYPE_NEXT   = 0x01,
    BOUNDARY_TYPE_SUB    = 0x02
};


//well-known-field:name
const apl_int_t MM4_BCC                                 = 0x01;
const apl_int_t MM4_CC                                  = 0x02;
const apl_int_t MM4_CONTENT_TYPE                        = 0x04;
const apl_int_t MM4_DATE                                = 0x05;
const apl_int_t MM4_X_MMS_DELIVERY_REPORT               = 0x06;
const apl_int_t MM4_X_MMS_EXPIRY                        = 0x08; 
const apl_int_t MM4_FROM                                = 0x09;
const apl_int_t MM4_X_MMS_MESSAGE_CLASS                 = 0x0A;
const apl_int_t MM4_X_MMS_MESSAGE_ID                    = 0x0B;
const apl_int_t MM4_X_MMS_MESSAGE_TYPE                  = 0x0C;
const apl_int_t MM4_X_MMS_PRIORITY                      = 0x0F;
const apl_int_t MM4_X_MMS_READ_REPLY                    = 0x10;
const apl_int_t MM4_X_MMS_REQUEST_STATUS_CODE           = 0x12;
const apl_int_t MM4_X_MMS_STATUS_TEXT                   = 0x13;
const apl_int_t MM4_X_MMS_SENDER_VISIBILITY             = 0x14;
const apl_int_t MM4_X_MMS_MM_STATUS_CODE                = 0x15;
const apl_int_t MM4_SUBJECT                             = 0x16;
const apl_int_t MM4_TO                                  = 0x17;
const apl_int_t MM4_X_MMS_TRANSACTION_ID                = 0x18;
const apl_int_t MM4_X_MMS_READ_STATUS                   = 0x1B;
const apl_int_t MM4_X_MMS_PREVIOUSLY_SENT_BY            = 0x20; 
const apl_int_t MM4_X_MMS_PREVIOUSLY_SENT_DATE_AND_TIME = 0x21;
const apl_int_t MM4_X_MMS_VERSION                       = 0x50;
const apl_int_t MM4_X_MMS_RCPT_TO                       = 0x51;
const apl_int_t MM4_X_MMS_ACK_REQUEST                   = 0x52;
const apl_int_t MM4_X_MMS_FORWARD_COUNTER               = 0x53;
const apl_int_t MM4_X_MMS_LINKED_ID                     = 0x54;
const apl_int_t MM4_X_MMS_FEE_TYPE                      = 0x55;
const apl_int_t MM4_X_MMS_FEE_CODE                      = 0x56;
const apl_int_t MM4_X_MMS_SERVICE_CODE                  = 0x57;
const apl_int_t MM4_X_MMS_VASP_ID                       = 0x58;
const apl_int_t MM4_X_MMS_VAS_ID                        = 0x59;
const apl_int_t MM4_X_MMS_CHARGED_PARTY_ID              = 0x5A;
const apl_int_t MM4_X_MMS_ORIGINATOR_SYSTEM             = 0x5B;
const apl_int_t MM4_X_MMS_ORIGINATOR_RS_DELIVERY_REPORT = 0x5C;
const apl_int_t MM4_X_MMS_FORWARD_TO_ORIGINATOR_UA      = 0x5D;
const apl_int_t MM4_X_MMS_MM_STATUS_EXTENSION           = 0x5E;
const apl_int_t MM4_X_MMS_RERLY_CHARGING_SUPPORT        = 0x60;
const apl_int_t MM4_X_MMS_PDU_READ_REPORT_SUPPORT       = 0x61;
const apl_int_t MM4_CONTENT                             = 0x99;

inline const char* GetFieldName(const apl_int_t aiFieldValue)
{
    switch(aiFieldValue)
    {
        case MM4_BCC:
            return "MM4_BCC";
        case MM4_CC:
            return "MM4_CC";
        case MM4_CONTENT_TYPE:
            return "MM4_CONTENT_TYPE";
        case MM4_DATE:
            return "MM4_DATE";
        case MM4_X_MMS_DELIVERY_REPORT:
            return "MM4_X_MMS_DELIVERY_REPORT";
        case MM4_X_MMS_EXPIRY:
            return "MM4_X_MMS_EXPIRY";
        case MM4_FROM:
            return "MM4_FROM";
        case MM4_X_MMS_MESSAGE_CLASS:
            return "MM4_X_MMS_MESSAGE_CLASS";
        case MM4_X_MMS_MESSAGE_ID:
            return "MM4_X_MMS_MESSAGE_ID";
        case MM4_X_MMS_MESSAGE_TYPE:
            return "MM4_X_MMS_MESSAGE_TYPE";
        case MM4_X_MMS_PRIORITY:
            return "MM4_X_MMS_PRIORITY";
        case MM4_X_MMS_READ_REPLY:
            return "MM4_X_MMS_READ_REPLY";
        case MM4_X_MMS_REQUEST_STATUS_CODE:
            return "MM4_X_MMS_REQUEST_STATUS_CODE";
        case MM4_X_MMS_STATUS_TEXT:
            return "MM4_X_MMS_STATUS_TEXT";
        case MM4_X_MMS_SENDER_VISIBILITY:
            return "MM4_X_MMS_SENDER_VISIBILITY";
        case MM4_X_MMS_MM_STATUS_CODE:
            return "MM4_X_MMS_MM_STATUS_CODE";
        case MM4_SUBJECT:
            return "MM4_SUBJECT";
        case MM4_TO:
            return "MM4_TO";
        case MM4_X_MMS_TRANSACTION_ID:
            return "MM4_X_MMS_TRANSACTION_ID";
        case MM4_X_MMS_READ_STATUS:
            return "MM4_X_MMS_READ_STATUS";
        case MM4_X_MMS_PREVIOUSLY_SENT_BY:
            return "MM4_X_MMS_PREVIOUSLY_SENT_BY";
        case MM4_X_MMS_PREVIOUSLY_SENT_DATE_AND_TIME:
            return "MM4_X_MMS_PREVIOUSLY_SENT_DATE_AND_TIME";
        case MM4_X_MMS_VERSION:
            return "MM4_X_MMS_VERSION";
        case MM4_X_MMS_RCPT_TO:
            return "MM4_X_MMS_RCPT_TO";
        case MM4_X_MMS_ACK_REQUEST:
            return "MM4_X_MMS_ACK_REQUEST";
        case MM4_X_MMS_FORWARD_COUNTER:
            return "MM4_X_MMS_FORWARD_COUNTER";
        case MM4_X_MMS_LINKED_ID:
            return "MM4_X_MMS_LINKED_ID";
        case MM4_X_MMS_FEE_TYPE:
            return "MM4_X_MMS_FEE_TYPE";
        case MM4_X_MMS_FEE_CODE:
            return "MM4_X_MMS_FEE_CODE";
        case MM4_X_MMS_SERVICE_CODE:
            return "MM4_X_MMS_SERVICE_CODE";
        case MM4_X_MMS_VASP_ID:
            return "MM4_X_MMS_VASP_ID";
        case MM4_X_MMS_VAS_ID:
            return "MM4_X_MMS_VAS_ID";
        case MM4_X_MMS_CHARGED_PARTY_ID:
            return "MM4_X_MMS_CHARGED_PARTY_ID";
        case MM4_X_MMS_ORIGINATOR_SYSTEM:
            return "MM4_X_MMS_ORIGINATOR_SYSTEM";
        case MM4_X_MMS_ORIGINATOR_RS_DELIVERY_REPORT:
            return "MM4_X_MMS_ORIGINATOR_RS_DELIVERY_REPORT";
        case MM4_X_MMS_FORWARD_TO_ORIGINATOR_UA:
            return "MM4_X_MMS_FORWARD_TO_ORIGINATOR_UA";
        case MM4_X_MMS_RERLY_CHARGING_SUPPORT:
            return "MM4_X_MMS_RERLY_CHARGING_SUPPORT";
        case MM4_X_MMS_PDU_READ_REPORT_SUPPORT:
            return "MM4_X_MMS_PDU_READ_REPORT_SUPPORT";
        case MM4_CONTENT:
            return "MM4_CONTENT";
        default:
            break;
    }

    return "Unkown Field";
}


inline bool IsQuote(char ac)  { return (ac == ' ' || ac == '\t' || ac == '\r' || ac == '\n' || ac == '"'); }

/*struct define*/
typedef struct table
{
    apl_int32_t mi32Value;
    char const* macValue;
} stTable;

typedef struct Field
{
    apl_int_t   miName;
    std::string moValue;
} stField;

typedef struct HeaderField
{
    std::string moName;
    std::string moValue;
} stHeaderField;

typedef struct MIMEEntity
{
    std::list<stHeaderField>        moHeaderList;
    std::string                     moBody;
    std::list<struct MIMEEntity>    moMultipartList;
}stMIMEEntity;

inline char const* Get(const stTable aoTable[], const apl_int32_t ai32Value)
{
    apl_int32_t i = 0;
    while(aoTable[i].mi32Value != -1)
    {
        if(aoTable[i].mi32Value == ai32Value)
        {
            return aoTable[i].macValue;
        }
        i++;
    }

    return "";
}

inline apl_int32_t Get(const stTable aoTable[], const char* apcValue)
{
    apl_int32_t i = 0;
    while(aoTable[i].mi32Value != -1) 
    {   
        if(apl_strcasecmp(aoTable[i].macValue, apcValue) == 0)
        {
            return aoTable[i].mi32Value;
        }
        i++;
    }

    return -1;
}


/*all kinds of MM4Message define*/
class CMM4Message
{
public:
    CMM4Message();
    virtual ~CMM4Message();

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock) const = 0;
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList) = 0;
    virtual void DumpMessage() const = 0;

    BUILD_GETTER_SETTER_INT(MessageType);
    BUILD_GETTER_SETTER_STR(TransactionID);
    BUILD_GETTER_SETTER_STR(MMSVersion);

protected:
    apl_int_t   miMessageType;
    std::string moMessageType;
    std::string moTransactionID;
    std::string moMMSVersion;
};

class CMM4CapabilityNegotiationREQ: public CMM4Message
{
public:
    CMM4CapabilityNegotiationREQ();
    virtual ~CMM4CapabilityNegotiationREQ();

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock) const;
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

    BUILD_GETTER_SETTER_STR(ReplyChargingSupporting);
    BUILD_GETTER_SETTER_STR(ReadReportSupporting);

private:
    std::string moReplyChargingSupporting;
    std::string moReadReportSupporting;
};

class CMM4CapabilityNegotiationRES: public CMM4Message
{
public:
    CMM4CapabilityNegotiationRES();
    virtual ~CMM4CapabilityNegotiationRES();

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock) const;
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

    BUILD_GETTER_SETTER_STR(ReplyChargingSupporting);
    BUILD_GETTER_SETTER_STR(ReadReportSupporting);

private:
    std::string moReplyChargingSupporting;
    std::string moReadReportSupporting;
};

class CMM4ForwardREQ: public CMM4Message
{
public:
    CMM4ForwardREQ();
    virtual ~CMM4ForwardREQ();

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock) const;
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

    BUILD_GETTER_SETTER_STR(MessageID);
    BUILD_GETTER_SETTER_STR(RecipientAddress);
    BUILD_GETTER_SETTER_STR(SenderAddress);
    BUILD_GETTER_SETTER_STR(MessageClass);
    BUILD_GETTER_SETTER_STR(Date);
    BUILD_GETTER_SETTER_STR(Expiry);
    BUILD_GETTER_SETTER_STR(DeliveryReport);
    BUILD_GETTER_SETTER_STR(OriginatorDeliveryReport);
    BUILD_GETTER_SETTER_STR(Priority);
    BUILD_GETTER_SETTER_STR(SenderVisibility);
    BUILD_GETTER_SETTER_STR(ReadReply);
    BUILD_GETTER_SETTER_STR(Subject);
    BUILD_GETTER_SETTER_STR(AckRequest);
    BUILD_GETTER_SETTER_STR(ForwardCounter);
    BUILD_GETTER_SETTER_STR(PreviouslySentBy);
    BUILD_GETTER_SETTER_STR(PreviouslySentDateAndTime);
    BUILD_GETTER_SETTER_STR(VASPID);
    BUILD_GETTER_SETTER_STR(VASID);
    BUILD_GETTER_SETTER_STR(ServiceCode);
    BUILD_GETTER_SETTER_STR(ChargedPartyID);
    BUILD_GETTER_SETTER_STR(LinkedID);
    BUILD_GETTER_SETTER_STR(FeeType);
    BUILD_GETTER_SETTER_STR(FeeCode);
    BUILD_GETTER_SETTER_STR(OriginatorSystem);
    BUILD_GETTER_SETTER_STR(RcptTo);
    BUILD_GETTER_SETTER_STR(ContentType);
    BUILD_GETTER_SETTER_STR(Content);

private:
    std::string moMessageID;
    std::string moRecipientAddress;
    std::string moSenderAddress;
    std::string moMessageClass;
    std::string moDate;
    std::string moExpiry;
    std::string moDeliveryReport;
    std::string moOriginatorDeliveryReport;
    std::string moPriority;
    std::string moSenderVisibility;
    std::string moReadReply;
    std::string moSubject;
    std::string moAckRequest;
    std::string moForwardCounter;
    std::string moPreviouslySentBy;
    std::string moPreviouslySentDateAndTime;
    std::string moVASPID;
    std::string moVASID;
    std::string moServiceCode;
    std::string moChargedPartyID;
    std::string moLinkedID;
    std::string moFeeType;
    std::string moFeeCode;
    std::string moOriginatorSystem;
    std::string moRcptTo;
    std::string moContentType;
    std::string moContent;
};

class CMM4ForwardRES: public CMM4Message
{
public:
    CMM4ForwardRES();
    virtual ~CMM4ForwardRES();

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock) const;
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

    BUILD_GETTER_SETTER_STR(MessageID);
    BUILD_GETTER_SETTER_STR(RequestStatusCode);
    BUILD_GETTER_SETTER_STR(StatusText);

private:
    std::string moMessageID;
    std::string moRequestStatusCode;
    std::string moStatusText;
};

class CMM4DeliveryReportREQ: public CMM4Message
{
public:
    CMM4DeliveryReportREQ();
    virtual ~CMM4DeliveryReportREQ();

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock) const;
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

    BUILD_GETTER_SETTER_STR(MessageID);
    BUILD_GETTER_SETTER_STR(RecipientAddress);
    BUILD_GETTER_SETTER_STR(SenderAddress);
    BUILD_GETTER_SETTER_STR(Date);
    BUILD_GETTER_SETTER_STR(AckRequest);
    BUILD_GETTER_SETTER_STR(ForwardToOriginatorUA);
    BUILD_GETTER_SETTER_STR(MMStatusCode);
    BUILD_GETTER_SETTER_STR(MMStatusExtension);
    BUILD_GETTER_SETTER_STR(StatusText);

private:
    std::string moMessageID;
    std::string moRecipientAddress;
    std::string moSenderAddress;
    std::string moDate;
    std::string moAckRequest;
    std::string moForwardToOriginatorUA;
    std::string moMMStatusCode;
    std::string moMMStatusExtension;
    std::string moStatusText;
};

class CMM4DeliveryReportRES: public CMM4Message
{
public:
    CMM4DeliveryReportRES();
    virtual ~CMM4DeliveryReportRES();

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock) const;
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

    BUILD_GETTER_SETTER_STR(MessageID);
    BUILD_GETTER_SETTER_STR(RequestStatusCode);
    BUILD_GETTER_SETTER_STR(StatusText);

private:
    std::string moMessageID;
    std::string moRequestStatusCode;
    std::string moStatusText;
};

class CMM4ReadReplyReportREQ: public CMM4Message
{
public:
    CMM4ReadReplyReportREQ();
    virtual ~CMM4ReadReplyReportREQ();

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock) const;
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

    BUILD_GETTER_SETTER_STR(RecipientAddress);
    BUILD_GETTER_SETTER_STR(SenderAddress);
    BUILD_GETTER_SETTER_STR(MessageID);
    BUILD_GETTER_SETTER_STR(Date);
    BUILD_GETTER_SETTER_STR(AckRequest);
    BUILD_GETTER_SETTER_STR(ReadStatus);
    BUILD_GETTER_SETTER_STR(StatusText);

private:
    std::string moRecipientAddress;
    std::string moSenderAddress;
    std::string moMessageID;
    std::string moDate;
    std::string moAckRequest;
    std::string moReadStatus;
    std::string moStatusText;
};

class CMM4ReadReplyReportRES: public CMM4Message
{
public:
    CMM4ReadReplyReportRES();
    virtual ~CMM4ReadReplyReportRES();

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock) const;
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

    BUILD_GETTER_SETTER_STR(RequestStatusCode);
    BUILD_GETTER_SETTER_STR(StatusText);

private:
    std::string moRequestStatusCode;
    std::string moStatusText;
};

CMM4Message* DecodeMessage(void* const apcPtr, const apl_size_t aiLen);
apl_ssize_t ParseMessage(acl::CMemoryBlock& aoMemoryBlock, apl_uint32_t &au32MessageType, std::list<stField> &aoFieldList, apl_uint8_t& au8ErrField);

/*encode*/
apl_ssize_t EncodeShortint(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Value);
apl_ssize_t EncodeShortint(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const apl_uint8_t au8Value);
apl_ssize_t EncodeInt(acl::CMemoryBlock& aoMemoryBlock, const apl_uint32_t au32Value);
apl_ssize_t EncodeInt(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const apl_uint32_t au32Value);
apl_ssize_t EncodeLongint(acl::CMemoryBlock& aoMemoryBlock, const apl_uint32_t au32Value);
apl_ssize_t EncodeLongint(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const apl_uint32_t au32Value);
apl_ssize_t EncodeUintvar(acl::CMemoryBlock& aoMemoryBlock, const apl_uint32_t au32Value);
apl_ssize_t EncodeUintvar(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const apl_uint32_t au32Value);
apl_ssize_t EncodeValueLength(acl::CMemoryBlock& aoMemoryBlock, const apl_uint32_t au32Value);
apl_ssize_t EncodeValueLength(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const apl_uint32_t au32Value);
apl_ssize_t EncodeTextString(acl::CMemoryBlock& aoMemoryBlock, const char* apcText, const apl_size_t aiSize = 0);
apl_ssize_t EncodeTextString(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const char* apcText, const apl_size_t aiSize = 0);
apl_ssize_t EncodeQuoteString(acl::CMemoryBlock& aoMemoryBlock, const char* apcText, const apl_size_t aiSize = 0);
apl_ssize_t EncodeQuoteString(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const char* apcText, const apl_size_t aiSize = 0);
apl_ssize_t EncodeFieldName(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Value);
apl_ssize_t EncodeEncodeString(acl::CMemoryBlock& aoMemoryBlock, const apl_int_t aiCharSet, const char* apcText);
apl_ssize_t EncodeEncodeString(acl::CMemoryBlock& aoMemoryBlock, const char* apcCharSet, const char* apcText);
apl_ssize_t EncodeEncodeString(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const apl_int_t aiCharSet, const char* apcText);
apl_ssize_t EncodeEncodeString(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const char* apcCharSet, const char* apcText);
apl_ssize_t EncodePreviouslySentDateAndTime(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name);
apl_ssize_t EncodePreviouslySentBy(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const char* apcPreviouslySentBy);
apl_ssize_t EncodeFrom(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, const char* apcFrom);
apl_ssize_t EncodeContentType(acl::CMemoryBlock& aoMemoryBlock, std::string const& aoContentType);
apl_ssize_t EncodeContentType(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Name, std::string const& aoContentType);
apl_ssize_t EncodeMultipartHeader(acl::CMemoryBlock& aoMemoryBlock, std::list<stHeaderField> aoHeaderList, std::string& aoContentType);
apl_ssize_t EncodeMultipart(acl::CMemoryBlock& aoMemoryBlock, std::list<stMIMEEntity> aoMIMEEntityList);

apl_ssize_t ParseContentType(std::string const& aoContentType, std::string& aoType, std::list<stHeaderField>& aoPararmList);
apl_ssize_t ParseCharSetText(std::string const& aoCharSetTextIn, std::string& aoCharSet, std::string& aoCharSetTextOut);
bool IsMultipart(std::string const& aoContentType,  std::string& aoBoundary);
apl_ssize_t ParseMultipartHeader(std::string const& aoHeader, std::list<stHeaderField>& aoHeaderList);
apl_ssize_t ParseMultipart(std::string const& aoMultipart, const char* apcBoundary, std::list<stMIMEEntity>& aoMIMEEntityList);

/*decode*/
apl_ssize_t DecodeMultipartHeader(acl::CMemoryBlock& aoMemoryBlock, std::list<stHeaderField>& aoHeaderList);
apl_ssize_t DecodeMultipart(acl::CMemoryBlock& aoMemoryBlock, std::string& aoContent);
apl_ssize_t DecodeMultipart(acl::CMemoryBlock& aoMemoryBlock, const std::string aoLastBoundary, apl_uint32_t& au32BoundarySeq, std::string& aoContent);
apl_ssize_t DecodeContentType(acl::CMemoryBlock& aoMemoryBlock, const apl_uint32_t au32Type, const apl_uint32_t au32Length, std::string& aoContentType);
apl_ssize_t DecodePreviouslySentBy(acl::CMemoryBlock& aoMemoryBlock, const apl_int_t aiType, const apl_uint8_t au8Length, std::string& aoPreviouslySentBy);
apl_ssize_t DecodePreviouslySentDateAndTime(acl::CMemoryBlock& aoMemoryBlock, const apl_int_t aiType, const apl_uint8_t au8Length, std::string& aoPreviouslySentDateAndTime);
apl_ssize_t DecodeFrom(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Length, std::string& aoFrom);
apl_uint32_t DecodeLongint(acl::CMemoryBlock& aoMemoryBlock, const apl_uint8_t au8Length);
apl_uint32_t DecodeInt(acl::CMemoryBlock& aoMemoryBlock, const apl_int_t aiType, const apl_uint8_t au8Length);
apl_uint32_t DecodeUintvar(acl::CMemoryBlock& aoMemoryBlock);
apl_ssize_t DecodeNullEndString(acl::CMemoryBlock& aoMemoryBlock, std::string& aoText);
apl_ssize_t DecodeEncodeString(acl::CMemoryBlock& aoMemoryBlock, const apl_uint32_t au32Type, const apl_uint32_t au32Length, std::string& aoCharSet, std::string& aoText);
apl_uint32_t DecodeTypeValue(acl::CMemoryBlock& aoMemoryBlock, apl_uint32_t& au32Value);

apl_int_t QPDecode(const char* apcIn, char* apcOut, apl_uint32_t* apcOutLen);

}

ANF_NAMESPACE_END

#endif

