#ifndef MM4_MESSAGE_H
#define MM4_MESSAGE_H

#include <string>
#include <list>
#include <map>
#include "anf/Utility.h"
#include "acl/StrAlgo.h"
#include "acl/SString.h"
#include "acl/Singleton.h"
#include "acl/MemoryBlock.h"

ANF_NAMESPACE_START

namespace mm4_smtp{

#define MMS_VERSION                                             "6.3.0"
#define TOKEN_CRLF                                              "\r\n"
#define TOKEN_CRLFCRLF                                          "\r\n\r\n"
#define MAX_LENGTH                                               4096

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

#define ENCODE_STR(message, name, value)                        \
        if(!value.empty())                                      \
        {                                                       \
            message.append(name);                               \
            message.append(": ");                               \
            message.append(value);                              \
            message.append(TOKEN_CRLF);                         \
        }
        
#define RETURN_ERR_IF(conditon, errno)                          \
        do                                                      \
        {                                                       \
            if(conditon)                                        \
            {                                                   \
                return errno;                                   \
            }                                                   \
        }while(0);

enum
{
        MM4_DECODE_ERROR_MMS_VERSION                            = -1,
        MM4_DECODE_ERROR_TRANSACTION_ID                         = -2,
        MM4_DECODE_ERROR_MESSAGE_ID                             = -3,
        MM4_DECODE_ERROR_TO                                     = -4,
        MM4_DECODE_ERROR_FROM                                   = -5,
        MM4_DECODE_ERROR_DELIVERY_REPORT                        = -6,
        MM4_DECODE_ERROR_ORIGINATOR_RS_DELIVERY_REPORT          = -7,
        MM4_DECODE_ERROR_DATE                                   = -8,
        MM4_DECODE_ERROR_CONTENT_TYPE                           = -9,
        MM4_DECODE_ERROR_REQUEST_STATUS_CODE                    = -10,
        MM4_DECODE_ERROR_FORWARD_TO_ORIGINATOR_UA               = -11,
        MM4_DECODE_ERROR_MM_STATUS_CODE                         = -12,
        MM4_DECODE_ERROR_READ_STATUS                            = -13,
        MM4_DECODE_ERROR_EOL                                    = -14,
        MM4_DECODE_ERROR_TOOBIG                                 = -15,
        MM4_DECODE_ERROR_FIELDNAME                              = -16,
        MM4_DECODE_ERROR_FIELDSEP                               = -17
};

inline const char* GetErrorMsg(const apl_int_t aiErrorCode)
{
    switch(aiErrorCode)
    {
        case MM4_DECODE_ERROR_MMS_VERSION:
            return "Field(X-Mms-3GPP-MMS-Version) NULL";
        case MM4_DECODE_ERROR_TRANSACTION_ID:
            return "Field(X-Mms-Transaction-ID) NULL";
        case MM4_DECODE_ERROR_MESSAGE_ID:
            return "Field(X-Mms-Message-ID) NULL";
        case MM4_DECODE_ERROR_TO:
            return "Field(To) NULL";
        case MM4_DECODE_ERROR_FROM:
            return "Field(From) NULL";
        case MM4_DECODE_ERROR_DELIVERY_REPORT:
            return "Field(X-Mms-Delivery-Report) NULL";
        case MM4_DECODE_ERROR_ORIGINATOR_RS_DELIVERY_REPORT:
            return "Field(X-Mms-Originator-R/S-Delivery-Report) NULL";
        case MM4_DECODE_ERROR_DATE:
            return "Field(Date) NULL";
        case MM4_DECODE_ERROR_CONTENT_TYPE:
            return "Field(Content-Type) NULL";
        case MM4_DECODE_ERROR_REQUEST_STATUS_CODE:
            return "Field(X-Mms-Request-Status-Code) NULL";
        case MM4_DECODE_ERROR_FORWARD_TO_ORIGINATOR_UA:
            return "Field(X-Mms-Forward-To-Originator-UA) NULL";
        case MM4_DECODE_ERROR_MM_STATUS_CODE:
            return "Field(X-Mms-MM-Status-Code) NULL";
        case MM4_DECODE_ERROR_READ_STATUS:
            return "Field(X-Mms-Read-Status) NULL";
        case MM4_DECODE_ERROR_EOL:
            return "MM4_DECODE_ERROR_EOL";
        case MM4_DECODE_ERROR_TOOBIG:
            return "MM4_DECODE_ERROR_TOOBIG";
        case MM4_DECODE_ERROR_FIELDNAME:
            return "MM4_DECODE_ERROR_FIELDNAME";
        case MM4_DECODE_ERROR_FIELDSEP:
            return "MM4_DECODE_ERROR_FIELDSEP";
        default:
            break;
    }

    return "Unkown Error";
}

inline bool IsQuote(char ac)  { return (ac == ' ' || ac == '\t' || ac == '\r' || ac == '\n'); }

const apl_int_t MM4_MSG_TYPE_FORWARD_REQ                        = 0x01;
const apl_int_t MM4_MSG_TYPE_FORWARD_RES                        = 0x02;
const apl_int_t MM4_MSG_TYPE_DELIVERY_REPORT_REQ                = 0x03;
const apl_int_t MM4_MSG_TYPE_DELIVERY_REPORT_RES                = 0x04;
const apl_int_t MM4_MSG_TYPE_READ_REPLY_REPORT_REQ              = 0x05;
const apl_int_t MM4_MSG_TYPE_READ_REPLY_REPORT_RES              = 0x06;

const apl_int_t MM4_MMS_VERSION                                 = 0x100;
const apl_int_t MM4_MMS_MESSAGE_TYPE                            = 0x101;
const apl_int_t MM4_MMS_TRANSACTION_ID                          = 0x102;
const apl_int_t MM4_MMS_MESSAGE_ID                              = 0x103;
const apl_int_t MM4_MMS_MESSAGE_CLASS                           = 0x104;
const apl_int_t MM4_MMS_EXPIRY                                  = 0x105;
const apl_int_t MM4_MMS_DELIVERY_REPORT                         = 0x106;
const apl_int_t MM4_MMS_ORIGINATOR_RS_DELIVERY_REPORT           = 0x107;
const apl_int_t MM4_MMS_PRIORITY                                = 0x108;
const apl_int_t MM4_MMS_SENDER_VISIBILITY                       = 0x109;
const apl_int_t MM4_MMS_READ_REPLY                              = 0x10a;
const apl_int_t MM4_MMS_ACK_REQUEST                             = 0x10b;
const apl_int_t MM4_MMS_FORWARD_COUNTER                         = 0x10c;
const apl_int_t MM4_MMS_VASP_ID                                 = 0x10d;
const apl_int_t MM4_MMS_VAS_ID                                  = 0x10f;
const apl_int_t MM4_MMS_SERVICE_CODE                            = 0x110;
const apl_int_t MM4_MMS_CHARGED_PARTY_ID                        = 0x111;
const apl_int_t MM4_MMS_PREVIOUSLY_SENT_BY                      = 0x112;
const apl_int_t MM4_MMS_PREVIOUSLY_SENT_DATE_AND_TIME           = 0x113;
const apl_int_t MM4_MMS_REQUEST_STATUS_CODE                     = 0x114;
const apl_int_t MM4_MMS_STATUS_TEXT                             = 0x115;
const apl_int_t MM4_MMS_FORWARD_TO_ORIGINATOR_UA                = 0x116;
const apl_int_t MM4_MMS_MM_STATUS_CODE                          = 0x117;
const apl_int_t MM4_MMS_MM_STATUS_EXTENSION                     = 0x118;
const apl_int_t MM4_MMS_READ_STATUS                             = 0x119;
const apl_int_t MM4_MMS_ORIGINATOR_SYSTEM                       = 0x11a;
const apl_int_t MM4_MMS_RECP_TO                                 = 0x11b;
const apl_int_t MM4_TO                                          = 0x11c;
const apl_int_t MM4_CC                                          = 0x11d;
const apl_int_t MM4_FROM                                        = 0x11e;
const apl_int_t MM4_DATE                                        = 0x11f;
const apl_int_t MM4_CONTENT_TYPE                                = 0x120;
const apl_int_t MM4_SENDER                                      = 0x121;
const apl_int_t MM4_MESSAGE_ID                                  = 0x122;
const apl_int_t MM4_SUBJECT                                     = 0x123;
const apl_int_t MM4_CONTENT                                     = 0x124;

typedef struct Field
{
    std::string moName;
    std::string moValue;
} stField;

class CMM4Message
{
public:
    CMM4Message();
    virtual ~CMM4Message();

    BUILD_GETTER_SETTER_INT(MessageType);
    BUILD_GETTER_SETTER_STR(TransactionID);
    BUILD_GETTER_SETTER_STR(MMSVersion);

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock) = 0;
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList) = 0;
    virtual void DumpMessage() const = 0;
    
protected:
    apl_int_t   miMessageType;
    std::string moMessageType;
    std::string moTransactionID;
    std::string moMMSVersion;
};

class CMM4ForwardREQ: public CMM4Message
{
public:
    CMM4ForwardREQ();
    virtual ~CMM4ForwardREQ();

    //MM4
    BUILD_GETTER_SETTER_STR(MessageID);
    BUILD_GETTER_SETTER_STR(RecipientAddress);
    BUILD_GETTER_SETTER_STR(SenderAddress);
    BUILD_GETTER_SETTER_STR(ContentType);
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
    BUILD_GETTER_SETTER_STR(VASPID);
    BUILD_GETTER_SETTER_STR(VASID);
    BUILD_GETTER_SETTER_STR(ServiceCode);
    BUILD_GETTER_SETTER_STR(ChargedPartyID);
    BUILD_GETTER_SETTER_STR(PreviouslySentBy);
    BUILD_GETTER_SETTER_STR(PreviouslySentDateAndTime);
    BUILD_GETTER_SETTER_STR(Content);

    //STD
    BUILD_GETTER_SETTER_STR(STDSender);
    BUILD_GETTER_SETTER_STR(STDOriSystem);
    BUILD_GETTER_SETTER_STR(STDMessageID);

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock);
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

private:
    //MM4
    std::string moMessageID;
    std::string moRecipientAddress;
    std::string moSenderAddress;
    std::string moContentType;
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
    std::string moVASPID;
    std::string moVASID;
    std::string moServiceCode;
    std::string moChargedPartyID;
    std::string moPreviouslySentBy;
    std::string moPreviouslySentDateAndTime;
    std::string moContent;

    //STD
    std::string moSTDSender;
    std::string moSTDOriSystem;
    std::string moSTDMessageID;
};

class CMM4ForwardRES: public CMM4Message
{
public:
    CMM4ForwardRES();
    virtual ~CMM4ForwardRES();

    //MM4
    BUILD_GETTER_SETTER_STR(MessageID);
    BUILD_GETTER_SETTER_STR(RequestStatusCode);
    BUILD_GETTER_SETTER_STR(StatusText);

    //STD
    BUILD_GETTER_SETTER_STR(STDSender);
    BUILD_GETTER_SETTER_STR(STDTo);
    BUILD_GETTER_SETTER_STR(STDMessageID);
    BUILD_GETTER_SETTER_STR(STDDate);

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock);
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

private:
    //MM4
    std::string moMessageID;
    std::string moRequestStatusCode;
    std::string moStatusText;

    //STD
    std::string moSTDSender;
    std::string moSTDTo;
    std::string moSTDMessageID;
    std::string moSTDDate;
};

class CMM4DeliveryReportREQ: public CMM4Message
{
public:
    CMM4DeliveryReportREQ();
    virtual ~CMM4DeliveryReportREQ();

    //MM4
    BUILD_GETTER_SETTER_STR(MessageID);
    BUILD_GETTER_SETTER_STR(SenderAddress);
    BUILD_GETTER_SETTER_STR(RecipientAddress);
    BUILD_GETTER_SETTER_STR(Date);
    BUILD_GETTER_SETTER_STR(AckRequest);
    BUILD_GETTER_SETTER_STR(ForwardToOriginatorUA);
    BUILD_GETTER_SETTER_STR(MMStatusCode);
    BUILD_GETTER_SETTER_STR(MMStatusExtension);
    BUILD_GETTER_SETTER_STR(StatusText);

    //STD   
    BUILD_GETTER_SETTER_STR(STDMessageID);
    BUILD_GETTER_SETTER_STR(STDSender);

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock);
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

private:
    std::string moMessageID;
    std::string moSenderAddress;
    std::string moRecipientAddress;
    std::string moDate;
    std::string moAckRequest;
    std::string moForwardToOriginatorUA;
    std::string moMMStatusCode;
    std::string moMMStatusExtension;
    std::string moStatusText;

    //STD
    std::string moSTDSender;
    std::string moSTDMessageID;
};

class CMM4DeliveryReportRES: public CMM4Message
{
public:
    CMM4DeliveryReportRES();
    virtual ~CMM4DeliveryReportRES();

    //MM4
    BUILD_GETTER_SETTER_STR(MessageID);
    BUILD_GETTER_SETTER_STR(RequestStatusCode);
    BUILD_GETTER_SETTER_STR(StatusText);

    //STD
    BUILD_GETTER_SETTER_STR(STDSender);
    BUILD_GETTER_SETTER_STR(STDTo);
    BUILD_GETTER_SETTER_STR(STDMessageID);
    BUILD_GETTER_SETTER_STR(STDDate);

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock);
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

private:
    //MM4
    std::string moMessageID;
    std::string moRequestStatusCode;
    std::string moStatusText;

    //STD
    std::string moSTDSender;
    std::string moSTDTo;
    std::string moSTDMessageID;
    std::string moSTDDate;
};

class CMM4ReadReplyReportREQ: public CMM4Message
{
public:
    CMM4ReadReplyReportREQ();
    virtual ~CMM4ReadReplyReportREQ();

    //MM4
    BUILD_GETTER_SETTER_STR(MessageID);
    BUILD_GETTER_SETTER_STR(SenderAddress);
    BUILD_GETTER_SETTER_STR(RecipientAddress);
    BUILD_GETTER_SETTER_STR(Date);
    BUILD_GETTER_SETTER_STR(AckRequest);
    BUILD_GETTER_SETTER_STR(ReadStatus);
    BUILD_GETTER_SETTER_STR(StatusText);

    //STD
    BUILD_GETTER_SETTER_STR(STDSender);
    BUILD_GETTER_SETTER_STR(STDMessageID);
    BUILD_GETTER_SETTER_STR(STDDate);

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock);
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

private:
    std::string moMessageID;
    std::string moSenderAddress;
    std::string moRecipientAddress;
    std::string moDate;
    std::string moAckRequest;
    std::string moReadStatus;
    std::string moStatusText;

    //STD
    std::string moSTDSender;
    std::string moSTDMessageID; 
    std::string moSTDDate;
};

class CMM4ReadReplyReportRES: public CMM4Message
{
public:
    CMM4ReadReplyReportRES();
    virtual ~CMM4ReadReplyReportRES();

    //MM4
    BUILD_GETTER_SETTER_STR(RequestStatusCode);
    BUILD_GETTER_SETTER_STR(StatusText);

    //STD
    BUILD_GETTER_SETTER_STR(STDSender);
    BUILD_GETTER_SETTER_STR(STDTo);
    BUILD_GETTER_SETTER_STR(STDMessageID);
    BUILD_GETTER_SETTER_STR(STDDate);

    virtual apl_ssize_t Encode(acl::CMemoryBlock& aoMemoryBlock);
    virtual apl_ssize_t Decode(std::list<stField> aoFieldList);
    virtual void DumpMessage() const;

private:
    //MM4
    std::string moRequestStatusCode;
    std::string moStatusText;

    //STD
    std::string moSTDSender;
    std::string moSTDTo;
    std::string moSTDMessageID;
    std::string moSTDDate;
};

class CTokenMap
{
public:
    CTokenMap(void);
    ~CTokenMap();
    
    apl_int_t Map(std::string aoToken);

protected:
    void Insert(std::string const& aoToken, const apl_int_t aiToken);
    
private:
    std::map<std::string, apl_int_t> moMap;
};

apl_ssize_t ParseMessage(char* const apcPtr, const apl_size_t aiLen, std::string &aoMessageType, std::list<stField> &aoFieldList);
CMM4Message* DecodeMessage(void* const apcPtr, const apl_size_t aiLen);

void ReplaceMultipartContentType(std::string &aoMultiPart);
std::string HttpDate2Timestamp(std::string const& aoHttpDate);
std::string Timestamp2HttpDate(std::string const& aoTimestamp);

}

ANF_NAMESPACE_END

#endif

