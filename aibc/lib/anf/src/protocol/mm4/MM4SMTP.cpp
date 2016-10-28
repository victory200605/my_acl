#include "anf/protocol/mm4/MM4SMTP.h"
#include "aaf/LogService.h"

ANF_NAMESPACE_START

namespace mm4_smtp{

/*
MM4Message
*/
CMM4Message::CMM4Message()
{}

CMM4Message::~CMM4Message()
{}

/*
MM4_Forward.REQ
*/
CMM4ForwardREQ::CMM4ForwardREQ()
{
    miMessageType = MM4_MSG_TYPE_FORWARD_REQ;
    moMessageType = "MM4_forward.REQ";
}

CMM4ForwardREQ::~CMM4ForwardREQ()
{}

apl_ssize_t CMM4ForwardREQ::Encode(acl::CMemoryBlock& aoMemoryBlock)
{
    //check
    RETURN_ERR_IF((this->moTransactionID.empty()),                  MM4_DECODE_ERROR_TRANSACTION_ID);
    RETURN_ERR_IF((this->moMessageID.empty()),                      MM4_DECODE_ERROR_MESSAGE_ID);
    RETURN_ERR_IF((this->moRecipientAddress.empty()),               MM4_DECODE_ERROR_TO);
    RETURN_ERR_IF((this->moSenderAddress.empty()),                  MM4_DECODE_ERROR_FROM);
    RETURN_ERR_IF((this->moContentType.empty()),                    MM4_DECODE_ERROR_CONTENT_TYPE);
    RETURN_ERR_IF((this->moDate.empty()),                           MM4_DECODE_ERROR_DATE);
    
    std::string loMessage;
    ENCODE_STR(loMessage, "X-Mms-Message-Type",                     this->moMessageType);
    ENCODE_STR(loMessage, "X-Mms-Transaction-ID",                   this->moTransactionID);
    ENCODE_STR(loMessage, "X-Mms-3GPP-MMS-Version",                 std::string(MMS_VERSION));
    ENCODE_STR(loMessage, "X-Mms-Message-ID",                       this->moMessageID);
    ENCODE_STR(loMessage, "From",                                   this->moSenderAddress);
    ENCODE_STR(loMessage, "To",                                     this->moRecipientAddress);
    ENCODE_STR(loMessage, "X-Mms-Message-Class",                    this->moMessageClass);
    ENCODE_STR(loMessage, "Date",                                   Timestamp2HttpDate(this->moDate));
    //ENCODE_STR(loMessage, "Date",                                   this->moDate);
    ENCODE_STR(loMessage, "X-Mms-Expiry",                           Timestamp2HttpDate(this->moExpiry));
    //ENCODE_STR(loMessage, "X-Mms-Expiry",                           this->moExpiry);
    ENCODE_STR(loMessage, "X-Mms-Delivery-Report",                  std::string("Yes"));
    ENCODE_STR(loMessage, "X-Mms-Originator-R/S-Delivery-Report",   std::string("Yes"));
    ENCODE_STR(loMessage, "X-Mms-Priority",                         this->moPriority);
    ENCODE_STR(loMessage, "X-Mms-Sender-Visibility",                this->moSenderVisibility);
    ENCODE_STR(loMessage, "X-Mms-Read-Reply",                       std::string("No"));
    ENCODE_STR(loMessage, "Subject",                                this->moSubject);
    ENCODE_STR(loMessage, "X-Mms-Ack-Request",                      this->moAckRequest);
    ENCODE_STR(loMessage, "X-Mms-Forward-Counter",                  std::string("1"));
    ENCODE_STR(loMessage, "X-Mms-VASP-ID",                          this->moVASPID);
    ENCODE_STR(loMessage, "X-Mms-VAS-ID",                           this->moVASID);
    ENCODE_STR(loMessage, "X-Mms-Service-Code",                     this->moServiceCode);
    ENCODE_STR(loMessage, "X-Mms-Charged-Party-ID",                 this->moChargedPartyID);
    ENCODE_STR(loMessage, "X-Mms-Previously-sent-by",               this->moPreviouslySentBy);
    ENCODE_STR(loMessage, "X-Mms-Previously-sent-date-and-time",    this->moPreviouslySentDateAndTime);
    ENCODE_STR(loMessage, "Sender",                                 this->moSTDSender);
    ENCODE_STR(loMessage, "X-Mms-Originator-System",                this->moSTDOriSystem);
    ENCODE_STR(loMessage, "Message-ID",                             this->moSTDMessageID);

    apl_uint32_t lu32Pos = 0;
    if((lu32Pos = this->moContentType.find("application/vnd.wap.multipart.")) != (apl_uint32_t)std::string::npos)
    {
        this->moContentType.replace(lu32Pos, apl_strlen("application/vnd.wap.multipart."), "multipart/");
    }
    ENCODE_STR(loMessage, "Content-Type",                           this->moContentType);
    loMessage += TOKEN_CRLF;

    ReplaceMultipartContentType(this->moContent);

    aoMemoryBlock.Resize(loMessage.size()+this->moContent.size());
    aoMemoryBlock.Write(loMessage.data(), loMessage.size());
    aoMemoryBlock.Write(this->moContent.data(), this->moContent.size());

    return aoMemoryBlock.GetLength();
}

apl_ssize_t CMM4ForwardREQ::Decode(std::list<stField> aoFieldList)
{   
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch(acl::Instance<CTokenMap>()->Map(loIter->moName))
        {
            //MM4
            case MM4_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;

            case MM4_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
            
            case MM4_MMS_MESSAGE_ID:
                this->moMessageID = loIter->moValue;
                break;

            case MM4_TO:
            case MM4_CC:
                this->moRecipientAddress = loIter->moValue;
                break;
            
            case MM4_FROM:
                this->moSenderAddress = loIter->moValue;
                break;
            
            case MM4_CONTENT_TYPE:
                this->moContentType = loIter->moValue;
                break;
            
            case MM4_MMS_MESSAGE_CLASS:
                this->moMessageClass = loIter->moValue;
                break;
            
            case MM4_DATE:
                this->moDate = HttpDate2Timestamp(loIter->moValue);
                //this->moDate = loIter->moValue;
                break;
            
            case MM4_MMS_EXPIRY:
                this->moExpiry = HttpDate2Timestamp(loIter->moValue);
                //this->moExpiry = loIter->moValue;
                break;
            
            case MM4_MMS_DELIVERY_REPORT:
                this->moDeliveryReport = loIter->moValue;
                break;
            
            case MM4_MMS_ORIGINATOR_RS_DELIVERY_REPORT:
                this->moOriginatorDeliveryReport = loIter->moValue;
                break;
            
            case MM4_MMS_PRIORITY:
                this->moPriority = loIter->moValue;
                break;
            
            case MM4_MMS_SENDER_VISIBILITY:
                this->moSenderVisibility = loIter->moValue;
                break;
            
            case MM4_MMS_READ_REPLY:
                this->moReadReply = loIter->moValue;
                break;
            
            case MM4_SUBJECT:
                this->moSubject = loIter->moValue;
                break;
                
            case MM4_CONTENT:
                this->moContent = loIter->moValue;
                break;
            
            case MM4_MMS_ACK_REQUEST:
                this->moAckRequest = loIter->moValue;
                break;
            
            case MM4_MMS_FORWARD_COUNTER:
                this->moForwardCounter = loIter->moValue;
                break;
            
            case MM4_MMS_PREVIOUSLY_SENT_BY:
                this->moPreviouslySentBy = loIter->moValue;
                break;
            
            case MM4_MMS_PREVIOUSLY_SENT_DATE_AND_TIME:
                this->moPreviouslySentDateAndTime = loIter->moValue;
                break;
            
            case MM4_MMS_VASP_ID:
                this->moVASPID = loIter->moValue;
                break;
            
            case MM4_MMS_VAS_ID:
                this->moVASID = loIter->moValue;
                break;
            
            case MM4_MMS_SERVICE_CODE:
                this->moServiceCode = loIter->moValue;
                break;
            
            case MM4_MMS_CHARGED_PARTY_ID:
                this->moChargedPartyID = loIter->moValue;
                break;

            //STD
            case MM4_SENDER:
                this->moSTDSender = loIter->moValue;
                break;
            
            case MM4_MESSAGE_ID:
                this->moSTDMessageID = loIter->moValue;
                break;
            
            case MM4_MMS_ORIGINATOR_SYSTEM:
                this->moSTDOriSystem = loIter->moValue;
                break;
                
            default:
                break;
        }
    }
    
    return 0;
}

void CMM4ForwardREQ::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }
    
    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4ForwardREQ/SMTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",                   this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",               this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",                 this->moTransactionID.c_str(),
        "X-Mms-Message-ID",                     this->moMessageID.c_str(),
        "To",                                   this->moRecipientAddress.c_str(),
        "From",                                 this->moSenderAddress.c_str(),
        "X-Mms-Message-Class",                  this->moMessageClass.c_str(),
        "Date",                                 this->moDate.c_str(),
        "X-Mms-Expiry",                         this->moExpiry.c_str(),
        "X-Mms-Delivery-Report",                this->moDeliveryReport.c_str(),
        "X-Mms-Originator-R/S-Delivery-Report", this->moOriginatorDeliveryReport.c_str(),
        "X-Mms-Priority",                       this->moPriority.c_str(),
        "X-Mms-Sender-Visibility",              this->moSenderVisibility.c_str(),
        "X-Mms-Read-Reply",                     this->moReadReply.c_str(),
        "X-Mms-Ack-Request",                    this->moAckRequest.c_str(),
        "X-Mms-Forward-Counter",                this->moForwardCounter.c_str(),
        "X-Mms-Previously-sent-by",             this->moPreviouslySentBy.c_str(),
        "X-Mms-Previously-sent-date-and-time",  this->moPreviouslySentDateAndTime.c_str(),
        "X-Mms-VASP-ID",                        this->moVASPID.c_str(),
        "X-Mms-VAS-ID",                         this->moVASID.c_str(),
        "X-Mms-Service-Code",                   this->moServiceCode.c_str(),
        "X-Mms-Charged-Party-ID",               this->moChargedPartyID.c_str(),
        "STD Sender",                           this->moSTDSender.c_str(),
        "STD X-Mms-Originator-System",          this->moSTDOriSystem.c_str(),
        "STD Message-ID",                       this->moSTDMessageID.c_str(),
        "Content-Type",                         this->moContentType.c_str(),
        "Subject",                              this->moSubject.c_str()
        );
    
    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}


/*
MM4_Forward.RES
*/
CMM4ForwardRES::CMM4ForwardRES()
{
    miMessageType = MM4_MSG_TYPE_FORWARD_RES;
    moMessageType = "MM4_forward.RES";
}

CMM4ForwardRES::~CMM4ForwardRES()
{}

apl_ssize_t CMM4ForwardRES::Encode(acl::CMemoryBlock& aoMemoryBlock)
{
    //check
    RETURN_ERR_IF((this->moTransactionID.empty()),                  MM4_DECODE_ERROR_TRANSACTION_ID);
    RETURN_ERR_IF((this->moMessageID.empty()),                      MM4_DECODE_ERROR_MESSAGE_ID);
    RETURN_ERR_IF((this->moRequestStatusCode.empty()),              MM4_DECODE_ERROR_REQUEST_STATUS_CODE);

    std::string loMessage;
    ENCODE_STR(loMessage, "X-Mms-Message-Type",                     this->moMessageType);
    ENCODE_STR(loMessage, "X-Mms-Transaction-ID",                   this->moTransactionID);
    ENCODE_STR(loMessage, "X-Mms-3GPP-MMS-Version",                 std::string(MMS_VERSION));
    ENCODE_STR(loMessage, "X-Mms-Message-ID",                       this->moMessageID);
    ENCODE_STR(loMessage, "X-Mms-Request-Status-Code",              this->moRequestStatusCode);
    ENCODE_STR(loMessage, "X-Mms-Status-Text",                      this->moStatusText);
    ENCODE_STR(loMessage, "Sender",                                 this->moSTDSender);
    ENCODE_STR(loMessage, "To",                                     this->moSTDTo);
    ENCODE_STR(loMessage, "Message-ID",                             this->moSTDMessageID);
    ENCODE_STR(loMessage, "Date",                                   Timestamp2HttpDate(this->moSTDDate));
    //ENCODE_STR(loMessage, "Date",                                   this->moSTDDate);
    //loMessage += TOKEN_CRLF;

    aoMemoryBlock.Resize(loMessage.size());
    aoMemoryBlock.Write(loMessage.data(), loMessage.size());

    return aoMemoryBlock.GetLength();
}

apl_ssize_t CMM4ForwardRES::Decode(std::list<stField> aoFieldList)
{
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch(acl::Instance<CTokenMap>()->Map(loIter->moName))
        {
            //MM4
            case MM4_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;

            case MM4_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
            
            case MM4_MMS_MESSAGE_ID:
                this->moMessageID = loIter->moValue;
                break;
            
            case MM4_MMS_REQUEST_STATUS_CODE:
                this->moRequestStatusCode = loIter->moValue;
                break;
            
            case MM4_MMS_STATUS_TEXT:
                this->moStatusText = loIter->moValue;
                break;

            //STD
            case MM4_SENDER:
                this->moSTDSender = loIter->moValue;
                break;
            
            case MM4_TO:
                this->moSTDTo = loIter->moValue;
                break;
            
            case MM4_MESSAGE_ID:
                this->moSTDMessageID = loIter->moValue;
                break;
            
            case MM4_DATE:
                this->moSTDDate = HttpDate2Timestamp(loIter->moValue);
                //this->moSTDDate = loIter->moValue;
                break;
                
            default:
                break;
        }
    }
    
    return 0;
}

void CMM4ForwardRES::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }
    
    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4ForwardRES/SMTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",                   this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",               this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",                 this->moTransactionID.c_str(),
        "X-Mms-Message-ID",                     this->moMessageID.c_str(),
        "X-Mms-Request-Status-Code",            this->moRequestStatusCode.c_str(),
        "X-Mms-Status-Text",                    this->moStatusText.c_str(),
        "STD Message-ID",                       this->moSTDMessageID.c_str(),
        "STD Sender",                           this->moSTDSender.c_str(),
        "STD To",                               this->moSTDTo.c_str(),
        "STD Date",                             this->moSTDDate.c_str()
        );

    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}


/*
MM4_Delivery_report.REQ
*/
CMM4DeliveryReportREQ::CMM4DeliveryReportREQ()
{
    miMessageType = MM4_MSG_TYPE_DELIVERY_REPORT_REQ;
    moMessageType = "MM4_delivery_report.REQ";
}

CMM4DeliveryReportREQ::~CMM4DeliveryReportREQ()
{}

apl_ssize_t CMM4DeliveryReportREQ::Encode(acl::CMemoryBlock& aoMemoryBlock)
{
    //check
    RETURN_ERR_IF((this->moTransactionID.empty()),                  MM4_DECODE_ERROR_TRANSACTION_ID);
    RETURN_ERR_IF((this->moMessageID.empty()),                      MM4_DECODE_ERROR_MESSAGE_ID);
    RETURN_ERR_IF((this->moRecipientAddress.empty()),               MM4_DECODE_ERROR_TO);
    RETURN_ERR_IF((this->moSenderAddress.empty()),                  MM4_DECODE_ERROR_FROM);
    RETURN_ERR_IF((this->moDate.empty()),                           MM4_DECODE_ERROR_DATE);
    RETURN_ERR_IF((this->moForwardToOriginatorUA.empty()),          MM4_DECODE_ERROR_FORWARD_TO_ORIGINATOR_UA);
    RETURN_ERR_IF((this->moMMStatusCode.empty()),                   MM4_DECODE_ERROR_MM_STATUS_CODE);

    std::string loMessage;
    ENCODE_STR(loMessage, "X-Mms-Message-Type",                     this->moMessageType);
    ENCODE_STR(loMessage, "X-Mms-Transaction-ID",                   this->moTransactionID);
    ENCODE_STR(loMessage, "X-Mms-3GPP-MMS-Version",                 std::string(MMS_VERSION));
    ENCODE_STR(loMessage, "X-Mms-Message-ID",                       this->moMessageID);
    ENCODE_STR(loMessage, "To",                                     this->moRecipientAddress);
    ENCODE_STR(loMessage, "From",                                   this->moSenderAddress);
    ENCODE_STR(loMessage, "Date",                                   Timestamp2HttpDate(this->moDate));
	//ENCODE_STR(loMessage, "Date",                                   this->moDate);
    ENCODE_STR(loMessage, "X-Mms-Ack-Request",                      this->moAckRequest);
    ENCODE_STR(loMessage, "X-Mms-Forward-To-Originator-UA",         this->moForwardToOriginatorUA);
    ENCODE_STR(loMessage, "X-Mms-MM-Status-Code",                   this->moMMStatusCode);
    ENCODE_STR(loMessage, "X-Mms-MM-Status-Extension",              this->moMMStatusExtension);
    ENCODE_STR(loMessage, "X-Mms-Status-Text",                      this->moStatusText);
    ENCODE_STR(loMessage, "Sender",                                 this->moSTDSender);
    ENCODE_STR(loMessage, "Message-ID",                             this->moSTDMessageID);
    //loMessage += TOKEN_CRLF;

    aoMemoryBlock.Resize(loMessage.size());
    aoMemoryBlock.Write(loMessage.data(), loMessage.size());

    return aoMemoryBlock.GetLength();
}

apl_ssize_t CMM4DeliveryReportREQ::Decode(std::list<stField> aoFieldList)
{
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch(acl::Instance<CTokenMap>()->Map(loIter->moName))
        {
            //MM4
            case MM4_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;

            case MM4_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
            
            case MM4_MMS_MESSAGE_ID:
                this->moMessageID = loIter->moValue;
                break;
                        
            case MM4_FROM:
                this->moSenderAddress = loIter->moValue;
                break;
            
            case MM4_TO:
            case MM4_CC:
                this->moRecipientAddress = loIter->moValue;
                break;
            
            case MM4_DATE:
                this->moDate = HttpDate2Timestamp(loIter->moValue);
                //this->moDate = loIter->moValue;
                break;
            
            case MM4_MMS_ACK_REQUEST:
                this->moAckRequest = loIter->moValue;
                break;
            
            case MM4_MMS_FORWARD_TO_ORIGINATOR_UA:
                this->moForwardToOriginatorUA = loIter->moValue;
                break;
            
            case MM4_MMS_MM_STATUS_CODE:
                this->moMMStatusCode = loIter->moValue;
                break;
            
            case MM4_MMS_MM_STATUS_EXTENSION:
                this->moMMStatusExtension = loIter->moValue;
                break;

            case MM4_MMS_STATUS_TEXT:
                this->moStatusText = loIter->moValue;
                break;

            //STD
            case MM4_SENDER:
                this->moSTDSender = loIter->moValue;
                break;
            
            case MM4_MESSAGE_ID:
                this->moSTDMessageID = loIter->moValue;
                break;
            
            default:
                break;
        }
    }
 
    return 0;
}

void CMM4DeliveryReportREQ::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }
    
    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4DeliveryReportREQ/SMTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",                   this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",               this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",                 this->moTransactionID.c_str(),
        "X-Mms-Message-ID",                     this->moMessageID.c_str(),
        "To",                                   this->moRecipientAddress.c_str(),
        "From",                                 this->moSenderAddress.c_str(),
        "Date",                                 this->moDate.c_str(),
        "X-Mms-Ack-Request",                    this->moAckRequest.c_str(),
        "X-Mms-Forward-To-Originator-UA",       this->moForwardToOriginatorUA.c_str(),
        "X-Mms-MM-Status-Code",                 this->moMMStatusCode.c_str(),
        "X-Mms-MM-Status-Extension",            this->moMMStatusExtension.c_str(),
        "X-Mms-Status-Text",                    this->moStatusText.c_str(),
        "STD Sender",                           this->moSTDSender.c_str(),
        "STD Message-ID",                       this->moSTDMessageID.c_str()
        );

    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}


/*
MM4_Delivery_report.RES
*/
CMM4DeliveryReportRES::CMM4DeliveryReportRES()
{
    miMessageType = MM4_MSG_TYPE_DELIVERY_REPORT_RES;
    moMessageType = "MM4_delivery_report.RES";
}

CMM4DeliveryReportRES::~CMM4DeliveryReportRES()
{}

apl_ssize_t CMM4DeliveryReportRES::Encode(acl::CMemoryBlock& aoMemoryBlock)
{
    //check
    RETURN_ERR_IF((this->moTransactionID.empty()),                  MM4_DECODE_ERROR_TRANSACTION_ID);
    RETURN_ERR_IF((this->moMessageID.empty()),                      MM4_DECODE_ERROR_MESSAGE_ID);
    RETURN_ERR_IF((this->moRequestStatusCode.empty()),              MM4_DECODE_ERROR_REQUEST_STATUS_CODE);
    
    std::string loMessage;
    ENCODE_STR(loMessage, "X-Mms-Message-Type",                     this->moMessageType);
    ENCODE_STR(loMessage, "X-Mms-Transaction-ID",                   this->moTransactionID);
    ENCODE_STR(loMessage, "X-Mms-3GPP-MMS-Version",                 std::string(MMS_VERSION));
    ENCODE_STR(loMessage, "X-Mms-Message-ID",                       this->moMessageID);
    ENCODE_STR(loMessage, "X-Mms-Request-Status-Code",              this->moRequestStatusCode);
    ENCODE_STR(loMessage, "X-Mms-Status-Text",                      this->moStatusText);
    ENCODE_STR(loMessage, "Sender",                                 this->moSTDSender);
    ENCODE_STR(loMessage, "To",                                     this->moSTDTo);
    ENCODE_STR(loMessage, "Message-ID",                             this->moSTDMessageID);
    ENCODE_STR(loMessage, "Date",                                   Timestamp2HttpDate(this->moSTDDate));
    //ENCODE_STR(loMessage, "Date",                                   this->moSTDDate);
    //loMessage += TOKEN_CRLF;

    aoMemoryBlock.Resize(loMessage.size());
    aoMemoryBlock.Write(loMessage.data(), loMessage.size());

    return aoMemoryBlock.GetLength();
}

apl_ssize_t CMM4DeliveryReportRES::Decode(std::list<stField> aoFieldList)
{
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch( acl::Instance<CTokenMap>()->Map(loIter->moName) )
        {
            //MM4
            case MM4_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;

            case MM4_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
            
            case MM4_MMS_MESSAGE_ID:
                this->moMessageID = loIter->moValue;
                break;
                        
            case MM4_MMS_REQUEST_STATUS_CODE:
                this->moRequestStatusCode = loIter->moValue;
                break;
            
            case MM4_MMS_STATUS_TEXT:
                this->moStatusText = loIter->moValue;
                break;

            //STD
            case MM4_SENDER:
                this->moSTDSender = loIter->moValue;
                break;
            
            case MM4_TO:
                this->moSTDTo = loIter->moValue;
                break;
            
            case MM4_MESSAGE_ID:
                this->moSTDMessageID = loIter->moValue;
                break;
            
            case MM4_DATE:
                this->moSTDDate = HttpDate2Timestamp(loIter->moValue);
                //this->moSTDDate = loIter->moValue;
                break;
                
            default:
                break;
        }
    }
        
    return 0;
}

void CMM4DeliveryReportRES::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }
    
    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4DeliveryReportRES/SMTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",                   this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",               this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",                 this->moTransactionID.c_str(),
        "X-Mms-Message-ID",                     this->moMessageID.c_str(),
        "X-Mms-Request-Status-Code",            this->moRequestStatusCode.c_str(),
        "X-Mms-Status-Text",                    this->moStatusText.c_str(),
        "STD Sender",                           this->moSTDSender.c_str(),
        "STD To",                               this->moSTDTo.c_str(),
        "STD Message-ID",                       this->moSTDMessageID.c_str(),
        "STD Date",                             this->moSTDDate.c_str()
        );

    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}


/*
MM4_Read_reply_report.REQ
*/
CMM4ReadReplyReportREQ::CMM4ReadReplyReportREQ()
{
    miMessageType = MM4_MSG_TYPE_READ_REPLY_REPORT_REQ;
    moMessageType = "MM4_read_reply_report.REQ";
}

CMM4ReadReplyReportREQ::~CMM4ReadReplyReportREQ()
{}

apl_ssize_t CMM4ReadReplyReportREQ::Encode(acl::CMemoryBlock& aoMemoryBlock)
{
    //check
    RETURN_ERR_IF((this->moMMSVersion.empty()),                     MM4_DECODE_ERROR_MMS_VERSION);
    RETURN_ERR_IF((this->moTransactionID.empty()),                  MM4_DECODE_ERROR_TRANSACTION_ID);
    RETURN_ERR_IF((this->moMessageID.empty()),                      MM4_DECODE_ERROR_MESSAGE_ID);
    RETURN_ERR_IF((this->moRecipientAddress.empty()),               MM4_DECODE_ERROR_TO);
    RETURN_ERR_IF((this->moSenderAddress.empty()),                  MM4_DECODE_ERROR_FROM);
    RETURN_ERR_IF((this->moDate.empty()),                           MM4_DECODE_ERROR_DATE);
    RETURN_ERR_IF((this->moReadStatus.empty()),                     MM4_DECODE_ERROR_READ_STATUS);

    std::string loMessage;
    ENCODE_STR(loMessage, "X-Mms-Message-Type",                     this->moMessageType);
    ENCODE_STR(loMessage, "X-Mms-Transaction-ID",                   this->moTransactionID);
    ENCODE_STR(loMessage, "X-Mms-3GPP-MMS-Version",                 std::string(MMS_VERSION));
    ENCODE_STR(loMessage, "From",                                   this->moSenderAddress);
    ENCODE_STR(loMessage, "To",                                     this->moRecipientAddress);
    ENCODE_STR(loMessage, "X-Mms-Message-ID",                       this->moMessageID);
    ENCODE_STR(loMessage, "Date",                                   Timestamp2HttpDate(this->moDate));
	//ENCODE_STR(loMessage, "Date",                                   this->moDate);
    ENCODE_STR(loMessage, "X-Mms-Ack-Request",                      this->moAckRequest);
    ENCODE_STR(loMessage, "X-Mms-Read-Status",                      this->moReadStatus);
    ENCODE_STR(loMessage, "X-Mms-Status-Text",                      this->moStatusText);
    ENCODE_STR(loMessage, "Sender",                                 this->moSTDSender);
    ENCODE_STR(loMessage, "Message-ID",                             this->moSTDMessageID);
    //loMessage += TOKEN_CRLF;

    aoMemoryBlock.Resize(loMessage.size());
    aoMemoryBlock.Write(loMessage.data(), loMessage.size());
 
    return aoMemoryBlock.GetLength();
}

apl_ssize_t CMM4ReadReplyReportREQ::Decode(std::list<stField> aoFieldList)
{
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch( acl::Instance<CTokenMap>()->Map(loIter->moName) )
        {
            //MM4
            case MM4_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;

            case MM4_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
                                    
            case MM4_FROM:
                this->moSenderAddress = loIter->moValue;
                break;
            
            case MM4_TO:
            case MM4_CC:
                this->moRecipientAddress = loIter->moValue;
                break;
            
            case MM4_MMS_MESSAGE_ID:
                this->moMessageID = loIter->moValue;
                break;
            
            case MM4_DATE:
                this->moDate = HttpDate2Timestamp(loIter->moValue);
                //this->moDate = loIter->moValue;
                break;
                        
            case MM4_MMS_ACK_REQUEST:
                this->moAckRequest = loIter->moValue;
                break;
            
            case MM4_MMS_READ_STATUS:
                this->moReadStatus = loIter->moValue;
                break;
            
            case MM4_MMS_STATUS_TEXT:
                this->moStatusText = loIter->moValue;
                break;

            //STD
            case MM4_SENDER:
                this->moSTDSender = loIter->moValue;
                break;
                        
            case MM4_MESSAGE_ID:
                this->moSTDMessageID = loIter->moValue;
                break;
            
            //case MM4_DATE:
            //  this->moSTDDate = HttpDate2Timestamp(loIter->moValue);
            //  break;
                
            default:
                break;
        }
    }
    
    return 0;
}

void CMM4ReadReplyReportREQ::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }
    
    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4ReadReplyReportREQ/SMTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",                   this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",               this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",                 this->moTransactionID.c_str(),
        "X-Mms-Message-ID",                     this->moMessageID.c_str(),
        "To",                                   this->moRecipientAddress.c_str(),
        "From",                                 this->moSenderAddress.c_str(),
        "Date",                                 this->moDate.c_str(),
        "X-Mms-Ack-Request",                    this->moAckRequest.c_str(),
        "X-Mms-Read-Status",                    this->moReadStatus.c_str(),
        "X-Mms-Status-Text",                    this->moStatusText.c_str(),
        "STD Sender",                           this->moSTDSender.c_str(),
        "STD Message-ID",                       this->moSTDMessageID.c_str(),
        "STD Date",                             this->moSTDDate.c_str()
        );

    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}


/*
MM4_Read_reply_report.RES
*/
CMM4ReadReplyReportRES::CMM4ReadReplyReportRES()
{
    miMessageType = MM4_MSG_TYPE_READ_REPLY_REPORT_RES;
    moMessageType = "MM4_read_reply_report.RES";
}

CMM4ReadReplyReportRES::~CMM4ReadReplyReportRES()
{}

apl_ssize_t CMM4ReadReplyReportRES::Encode(acl::CMemoryBlock& aoMemoryBlock)
{
    //check
    RETURN_ERR_IF((this->moMMSVersion.empty()),                     MM4_DECODE_ERROR_MMS_VERSION);
    RETURN_ERR_IF((this->moTransactionID.empty()),                  MM4_DECODE_ERROR_TRANSACTION_ID);
    RETURN_ERR_IF((this->moRequestStatusCode.empty()),              MM4_DECODE_ERROR_REQUEST_STATUS_CODE);

    std::string loMessage;
    ENCODE_STR(loMessage, "X-Mms-Message-Type",                     this->moMessageType);
    ENCODE_STR(loMessage, "X-Mms-Transaction-ID",                   this->moTransactionID);
    ENCODE_STR(loMessage, "X-Mms-3GPP-MMS-Version",                 std::string(MMS_VERSION));
    ENCODE_STR(loMessage, "X-Mms-Request-Status-Code",              this->moRequestStatusCode);
    ENCODE_STR(loMessage, "X-Mms-Status-Text",                      this->moStatusText);
    ENCODE_STR(loMessage, "Sender",                                 this->moSTDSender);
    ENCODE_STR(loMessage, "To",                                     this->moSTDTo);
    ENCODE_STR(loMessage, "Message-ID",                             this->moSTDMessageID);
    ENCODE_STR(loMessage, "Date",                                   Timestamp2HttpDate(this->moSTDDate));
    //ENCODE_STR(loMessage, "Date",                                   this->moSTDDate);
    //loMessage += TOKEN_CRLF;

    aoMemoryBlock.Resize(loMessage.size());
    aoMemoryBlock.Write(loMessage.data(), loMessage.size());

    return aoMemoryBlock.GetLength();
}

apl_ssize_t CMM4ReadReplyReportRES::Decode(std::list<stField> aoFieldList)
{
    std::list<stField>::iterator loIter = aoFieldList.begin();
    for(; loIter != aoFieldList.end(); ++loIter) 
    {
        switch( acl::Instance<CTokenMap>()->Map(loIter->moName) )
        {
            //MM4
            case MM4_MMS_VERSION:
                this->moMMSVersion = loIter->moValue;
                break;

            case MM4_MMS_MESSAGE_TYPE:
                break;
            
            case MM4_MMS_TRANSACTION_ID:
                this->moTransactionID = loIter->moValue;
                break;
                        
            case MM4_MMS_REQUEST_STATUS_CODE:
                this->moRequestStatusCode = loIter->moValue;
                break;
            
            case MM4_MMS_STATUS_TEXT:
                this->moStatusText = loIter->moValue;
                break;

            //STD
            case MM4_SENDER:
                this->moSTDSender = loIter->moValue;
                break;
            
            case MM4_TO:
                this->moSTDTo = loIter->moValue;
                break;
            
            case MM4_MESSAGE_ID:
                this->moSTDMessageID = loIter->moValue;
                break;
            
            case MM4_DATE:
                this->moSTDDate = HttpDate2Timestamp(loIter->moValue);
                //this->moSTDDate = loIter->moValue;
                break;
                
            default:
                break;
        }
    }
        
    return 0;
}

void CMM4ReadReplyReportRES::DumpMessage() const
{
    if(acl::Instance<aaf::CLogService>()->GetLevel() < acl::LOG_LVL_DEBUG)
    {
        return;
    }
    
    char lsBuff[MAX_LENGTH] = {0};
    apl_snprintf(lsBuff, sizeof(lsBuff), 
        "{ MM4ReadReplyReportRES/SMTP } :\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n"
        "\t\t%-40s = %s\n",
        "X-Mms-Message-Type",                   this->moMessageType.c_str(),
        "X-Mms-3GPP-MMS-Version",               this->moMMSVersion.c_str(),
        "X-Mms-Transaction-ID",                 this->moTransactionID.c_str(),
        "X-Mms-Request-Status-Code",            this->moRequestStatusCode.c_str(),
        "X-Mms-Status-Text",                    this->moStatusText.c_str(),
        "STD Sender",                           this->moSTDSender.c_str(),
        "STD To",                               this->moSTDTo.c_str(),
        "STD Message-ID",                       this->moSTDMessageID.c_str(),
        "STD Date",                             this->moSTDDate.c_str()
        );

    AAF_LOG_DEBUG("%s", lsBuff);

    return;
}


CTokenMap::CTokenMap(void)
{ 
    Insert("MM4_forward.REQ",                       MM4_MSG_TYPE_FORWARD_REQ);
    Insert("MM4_forward.RES",                       MM4_MSG_TYPE_FORWARD_RES);
    Insert("MM4_delivery_report.REQ",               MM4_MSG_TYPE_DELIVERY_REPORT_REQ);
    Insert("MM4_delivery_report.RES",               MM4_MSG_TYPE_DELIVERY_REPORT_RES);
    Insert("MM4_read_reply_report.REQ",             MM4_MSG_TYPE_READ_REPLY_REPORT_REQ);
    Insert("MM4_read_reply_report.RES",             MM4_MSG_TYPE_READ_REPLY_REPORT_RES);

    Insert("X-Mms-3GPP-MMS-Version",                MM4_MMS_VERSION);
    Insert("X-Mms-Message-Type",                    MM4_MMS_MESSAGE_TYPE);
    Insert("X-Mms-Transaction-ID",                  MM4_MMS_TRANSACTION_ID);
    Insert("X-Mms-Message-ID",                      MM4_MMS_MESSAGE_ID);
    Insert("X-Mms-Message-Class",                   MM4_MMS_MESSAGE_CLASS);
    Insert("X-Mms-Expiry",                          MM4_MMS_EXPIRY);
    Insert("X-Mms-Delivery-Report",                 MM4_MMS_DELIVERY_REPORT);
    Insert("X-Mms-Originator-R/S-Delivery-Report",  MM4_MMS_ORIGINATOR_RS_DELIVERY_REPORT);
    Insert("X-Mms-Priority",                        MM4_MMS_PRIORITY);
    Insert("X-Mms-Sender-Visibility",               MM4_MMS_SENDER_VISIBILITY);
    Insert("X-Mms-Read-Reply",                      MM4_MMS_READ_REPLY);
    Insert("X-Mms-Ack-Request",                     MM4_MMS_ACK_REQUEST);
    Insert("X-Mms-Forward-Counter",                 MM4_MMS_FORWARD_COUNTER);
    Insert("X-Mms-VASP-ID",                         MM4_MMS_VASP_ID);
    Insert("X-Mms-VAS-ID",                          MM4_MMS_VAS_ID);
    Insert("X-Mms-Service-Code",                    MM4_MMS_SERVICE_CODE);  
    Insert("X-Mms-Charged-Party-ID",                MM4_MMS_CHARGED_PARTY_ID);
    Insert("X-Mms-Previously-sent-by",              MM4_MMS_PREVIOUSLY_SENT_BY);
    Insert("X-Mms-Previously-sent-date-and-time",   MM4_MMS_PREVIOUSLY_SENT_DATE_AND_TIME);
    Insert("X-Mms-Request-Status-Code",             MM4_MMS_REQUEST_STATUS_CODE);
    Insert("X-Mms-Status-Text",                     MM4_MMS_STATUS_TEXT);
    Insert("X-Mms-Forward-To-Originator-UA",        MM4_MMS_FORWARD_TO_ORIGINATOR_UA);
    Insert("X-Mms-MM-Status-Code",                  MM4_MMS_MM_STATUS_CODE);
    Insert("X-Mms-MM-Status-Extension",             MM4_MMS_MM_STATUS_EXTENSION);
    Insert("X-Mms-Read-Status",                     MM4_MMS_READ_STATUS);
    Insert("X-Mms-Originator-System",               MM4_MMS_ORIGINATOR_SYSTEM);
    Insert("X-Mms-Rcpt-To",                         MM4_MMS_RECP_TO);
    Insert("To",                                    MM4_TO);
    Insert("CC",                                    MM4_CC);
    Insert("From",                                  MM4_FROM);
    Insert("Date",                                  MM4_DATE);
    Insert("Content-Type",                          MM4_CONTENT_TYPE);
    Insert("Sender",                                MM4_SENDER);
    Insert("Message-ID",                            MM4_MESSAGE_ID);
    Insert("Subject",                               MM4_SUBJECT);
    Insert("Content",                               MM4_CONTENT);
}
    
apl_int_t CTokenMap::Map(std::string aoToken)
{
    std::map<std::string, apl_int_t>::iterator loIter = this->moMap.find(aoToken);
    
    return loIter != this->moMap.end() ? loIter->second : -1;
}

void CTokenMap::Insert(std::string const& aoToken, const apl_int_t aiToken)
{
    std::string loToken = aoToken;
    transform(loToken.begin(), loToken.end(), loToken.begin(), apl_tolower);

    this->moMap[loToken] = aiToken;
}

apl_ssize_t ParseMessage(char* const apcPtr, const apl_size_t aiLen, std::string &aoMessageType, std::list<stField> &aoFieldList)
{   
    char const* lpcStart = apcPtr;
    char const* lpcBeginOfLine = apcPtr;
    char const* lpcEndOfLine;
    char const* lpcSep;
    char lsName[MAX_LENGTH + 1];
    char lsValue[MAX_LENGTH + 1]; 

    apl_memset(lsName, 0, sizeof(lsName));
    apl_memset(lsValue, 0, sizeof(lsValue));

    while (true)
    {
        lpcEndOfLine = apl_strstr(lpcBeginOfLine, TOKEN_CRLF);

        RETURN_ERR_IF((NULL == lpcEndOfLine || lpcEndOfLine > lpcStart + aiLen), MM4_DECODE_ERROR_EOL);

        if (lpcEndOfLine == lpcBeginOfLine)
        {
            lpcBeginOfLine = lpcEndOfLine + apl_strlen(TOKEN_CRLF);

            if( apl_size_t(lpcEndOfLine - lpcStart) < aiLen )
            {
                stField loField;
                loField.moName = std::string("content");
                loField.moValue.append(lpcBeginOfLine, aiLen-(lpcBeginOfLine-lpcStart));
                aoFieldList.push_back(loField);
            }
            break; // reach end of buffer, break and return; 
        }

        if(apl_size_t((lpcEndOfLine + apl_strlen(TOKEN_CRLF))- lpcStart) >= aiLen)
        {
            break;
        }

        assert(lpcEndOfLine > lpcBeginOfLine);

        RETURN_ERR_IF(((apl_size_t)(lpcEndOfLine - lpcBeginOfLine) > MAX_LENGTH), MM4_DECODE_ERROR_TOOBIG);

        if (apl_isspace(lpcBeginOfLine[0]))
        { // LWS
            RETURN_ERR_IF((apl_strlen(lsName) == 0), MM4_DECODE_ERROR_FIELDNAME);

            apl_memset(lsValue, 0, sizeof(lsValue));
            apl_memcpy(lsValue, lpcBeginOfLine, lpcEndOfLine - lpcBeginOfLine);
            acl::stralgo::TrimIf(lsValue, IsQuote);
            
            RETURN_ERR_IF((apl_strlen(lsValue) == 0), MM4_DECODE_ERROR_FIELDNAME);

            RETURN_ERR_IF((aoFieldList.empty()), MM4_DECODE_ERROR_FIELDNAME);

            std::list<stField>::reverse_iterator loRIter = aoFieldList.rbegin();
            RETURN_ERR_IF((apl_strcasecmp(loRIter->moName.c_str(),lsName) != 0), MM4_DECODE_ERROR_FIELDNAME);

            loRIter->moValue.append(lsValue);
        }
        else
        {
            lpcSep = (char const*)apl_memchr(lpcBeginOfLine, ':', lpcEndOfLine - lpcBeginOfLine);

            RETURN_ERR_IF((NULL == lpcSep), MM4_DECODE_ERROR_FIELDSEP);
           
            apl_memset(lsName, 0, sizeof(lsName));
            apl_memcpy(lsName, lpcBeginOfLine, lpcSep - lpcBeginOfLine);
            acl::stralgo::TrimIf(lsName, IsQuote);
             
            apl_memset(lsValue, 0, sizeof(lsValue));
            apl_memcpy(lsValue, lpcSep + 1, lpcEndOfLine - (lpcSep + 1));
            acl::stralgo::TrimIf(lsValue, IsQuote);
           
            RETURN_ERR_IF((apl_strlen(lsName) == 0), MM4_DECODE_ERROR_FIELDNAME);
            
            stField loField;
            loField.moName = lsName;
            loField.moValue = lsValue;
            transform(loField.moName.begin(), loField.moName.end(), loField.moName.begin(), apl_tolower);
            if( apl_strcasecmp(lsName, "X-Mms-Message-Type") == 0 )
            {
                transform(loField.moValue.begin(), loField.moValue.end(), loField.moValue.begin(), apl_tolower);
                aoMessageType = loField.moValue;
            }
            
            aoFieldList.push_back(loField);
        }

        lpcBeginOfLine = lpcEndOfLine + apl_strlen(TOKEN_CRLF);
    }
    
    return 0;
}

CMM4Message* DecodeMessage(void* const apcPtr, const apl_size_t aiLen)
{
    std::list<stField>  loFieldList;
    CMM4Message*        lpoMessage = NULL;
    std::string         loMessageType;
    apl_ssize_t         liErrorCode = 0;

    if((liErrorCode = ParseMessage((char* const)apcPtr, aiLen, loMessageType, loFieldList)) != 0 || loMessageType.empty())
    {
        apl_errprintf("error decode, %s\n", liErrorCode!=0?GetErrorMsg(liErrorCode):"Filed(X-Mms-Message-Type) NULL");
        return NULL;
    }
    
    switch( acl::Instance<CTokenMap>()->Map(loMessageType) )
    {
        case MM4_MSG_TYPE_FORWARD_REQ: 
            ACL_NEW_ASSERT(lpoMessage, CMM4ForwardREQ);
            break;
        
        case MM4_MSG_TYPE_FORWARD_RES: 
            ACL_NEW_ASSERT(lpoMessage, CMM4ForwardRES); 
            break;
        
        case MM4_MSG_TYPE_DELIVERY_REPORT_REQ: 
            ACL_NEW_ASSERT(lpoMessage, CMM4DeliveryReportREQ); 
            break;
        
        case MM4_MSG_TYPE_DELIVERY_REPORT_RES: 
            ACL_NEW_ASSERT(lpoMessage, CMM4DeliveryReportRES); 
            break;
        
        case MM4_MSG_TYPE_READ_REPLY_REPORT_REQ: 
            ACL_NEW_ASSERT(lpoMessage, CMM4ReadReplyReportREQ); 
            break;
        
        case MM4_MSG_TYPE_READ_REPLY_REPORT_RES: 
            ACL_NEW_ASSERT(lpoMessage, CMM4ReadReplyReportRES); 
            break;
    
        default:
            apl_errprintf("error decode, X-Mms-Message-Type(%s) not support!\n", loMessageType.c_str());
            return NULL;
    }

    if( (liErrorCode = lpoMessage->Decode(loFieldList)) != 0 )
    {
        apl_errprintf("[MessageType: %s] error decode, %s\n", loMessageType.c_str(), GetErrorMsg(liErrorCode));
        ACL_DELETE(lpoMessage);
        return NULL;
    }
    
    return lpoMessage;
}

//replace "application/vnd.wap.multipart." to "multipart/"
void ReplaceMultipartContentType(std::string &aoMultiPart)
{
    apl_uint32_t lu32Pos1 = 0, lu32Pos2 = 0, lu32Pos3 = 0, lu32Pos4 = 0;

    while((lu32Pos2 = aoMultiPart.find("Content-Type", lu32Pos1)) != (apl_uint32_t)std::string::npos)
    {		
        lu32Pos2 += apl_strlen("Content-Type");

        if((lu32Pos3 = aoMultiPart.find(TOKEN_CRLF, lu32Pos2)) == (apl_uint32_t)std::string::npos)
        {
            break;
        }

        if((lu32Pos4 = aoMultiPart.find("application/vnd.wap.multipart.", lu32Pos2)) != (apl_uint32_t)std::string::npos)
        {
            if(lu32Pos4 < lu32Pos3)
            {
                //replace
                aoMultiPart.replace(lu32Pos4, apl_strlen("application/vnd.wap.multipart."), "multipart/");

                lu32Pos3 -= apl_strlen("application/vnd.wap.multipart.") - apl_strlen("multipart/");
            }
        }

        lu32Pos1 = lu32Pos3 + apl_strlen(TOKEN_CRLF);
    }

    return;
}

static char const WeekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static char const Month[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

//HttpDate: Week, Day Mon Year HH:MM:SS GMT(Fri, 31 Dec 1999 23:59:59 GMT)
std::string HttpDate2Timestamp(std::string const& aoHttpDate)
{
    std::string loTimestamp = aoHttpDate;
    
    apl_uint32_t lu32Start = 0;
    apl_uint32_t lu32End = 0;
    if((lu32End = aoHttpDate.find(",", lu32Start)) != (apl_uint32_t)std::string::npos)
    {
        std::string  loDate, loWeekDay, loDay, loMon, loYear, loHHMMSS;
        apl_uint32_t lu32WeekDay = 0, lu32Day = 0, lu32Mon = 0;
        
        loWeekDay = aoHttpDate.substr(lu32Start, lu32End-lu32Start);
        for(apl_uint32_t lu32Index = 0; lu32Index < 7; lu32Index++)
        {   
            if(apl_strcasecmp(loWeekDay.c_str(), WeekDay[lu32Index]) == 0)
            {
                lu32WeekDay = lu32Index+1;
                break;
            }
        }

        if(lu32WeekDay != 0 && (lu32End+1 < aoHttpDate.size()))
        {
            loDate.assign(aoHttpDate.c_str()+lu32End+1);
            
            acl::CTokenizer loTokenizer(" ");
            if(loTokenizer.Parse((char*)loDate.c_str()) == 5)
            {
                loDay    = loTokenizer.GetField(0);
                loMon    = loTokenizer.GetField(1);
                loYear   = loTokenizer.GetField(2);
                loHHMMSS = loTokenizer.GetField(3);

                lu32Day = apl_strtou32(loDay.c_str(), NULL, 10);
            
                for(apl_uint32_t lu32Index = 0; lu32Index < 12; lu32Index++)
                {   
                    if(apl_strcasecmp(loMon.c_str(), Month[lu32Index]) == 0)
                    {
                        lu32Mon = lu32Index+1;
                        break;
                    }
                }
            
                if(lu32Day != 0 && lu32Mon != 0)
                {
                    char lsDate[64] = {0};
                    apl_snprintf(lsDate, sizeof(lsDate), "%s%02d%02d%s", loYear.c_str(), lu32Mon, lu32Day, loHHMMSS.c_str());
            
                    if(apl_strlen(lsDate) == apl_strlen("yyyymmddhh:mm:ss"))
                    {
                        struct apl_tm_t loTm; 
                        if(apl_strptime(lsDate, "%Y%m%d%H:%M:%S", &loTm) != NULL)
                        {
                            apl_time_t liTime = apl_mktime(&loTm);
                            if(liTime > 0)
                            {
                                apl_snprintf(lsDate, sizeof(lsDate), "%"APL_PRIi64, liTime/APL_TIME_SEC);
                                loTimestamp = lsDate;
                            }
                        }
                    }
                }
            }
        }
    }

    return loTimestamp;
}

std::string Timestamp2HttpDate(std::string const& aoTimestamp)
{
    std::string  loHttpDate = aoTimestamp;
    bool lbTimestamp = true;
    apl_uint32_t lu32Length = aoTimestamp.size();

    if(lu32Length == 10)//timestamp len, exp: 1399353289
    {
        for(apl_uint32_t lu32Index = 0; lu32Index < lu32Length; lu32Index++)
        {   
            if(!apl_isdigit((apl_uint8_t)aoTimestamp[lu32Index]))
            {
                lbTimestamp = false;
                break;
            }
        }
        
        if(lbTimestamp)
        {
            struct apl_tm_t loTm;
            if(apl_localtime(apl_strtoi64(aoTimestamp.c_str(), NULL, 10)*APL_TIME_SEC, &loTm) != NULL)
            {
                char lsHttpDate[64] = {0};
                snprintf(lsHttpDate, sizeof(lsHttpDate), "%.3s, %02d %.3s %d %.2d:%.2d:%.2d GMT",
                    WeekDay[loTm.tm_wday],
                    loTm.tm_mday, 
                    Month[loTm.tm_mon],
                    1900 + loTm.tm_year,
                    loTm.tm_hour,
                    loTm.tm_min, 
                    loTm.tm_sec);
                
                loHttpDate = lsHttpDate;
            }
        }
    }

    return loHttpDate;
}

}

ANF_NAMESPACE_END

