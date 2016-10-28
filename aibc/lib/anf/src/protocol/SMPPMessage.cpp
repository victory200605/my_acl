#include "anf/protocol/SMPPMessage.h"

namespace smpp
{

#define SMPP_PDU_COPY_IMPL(type) \
    void type::CopyFrom(IPDUBase* apoPDU) \
    { \
        type* lpoTemp = dynamic_cast<type*>(apoPDU); \
        ACL_ASSERT(lpoTemp != APL_NULL); \
        *this = *lpoTemp; \
    }

CSMPPHeader::CSMPPHeader()
{
    mu32Command_length = 0;
    mu32Command_id = 0;
    mu32Command_status = 0;
    mu32Sequence_number = 0;
}

CTLVItem::CTLVItem()
{
    mu16Tag = 0;
    mu16Len = 0;
}

CBindPDU::CBindPDU()
{
    apl_memset(macSystem_id, 0, sizeof(macSystem_id));
    apl_memset(macPassword, 0, sizeof(macPassword));
    apl_memset(macSystem_type, 0, sizeof(macSystem_type));
    mu8Interface_version = 0;
    mu8Addr_ton = 0;
    mu8Addr_npi = 0;
    apl_memset(macAddress_range, 0, sizeof(macAddress_range));
}

SMPP_PDU_COPY_IMPL(CBindPDU);

CBindRespPDU::CBindRespPDU()
{
    apl_memset(macSystem_id, 0, sizeof(macSystem_id));
}

SMPP_PDU_COPY_IMPL(CBindRespPDU);

COutBindPDU::COutBindPDU()
{
    apl_memset(macSystem_id, 0, sizeof(macSystem_id));
    apl_memset(macPassword, 0, sizeof(macPassword));
}

SMPP_PDU_COPY_IMPL(COutBindPDU);

CSubmitSMPDU::CSubmitSMPDU()
{
    mu8Source_addr_ton = 0;
    mu8Source_addr_npi = 0;
    mu8Dest_addr_ton = 0;
    mu8Dest_addr_npi = 0;
    mu8Esm_class = 0;
    mu8Protocol_id = 0;
    mu8Priority_flag = 0;
    mu8Registered_delivery = 0;
    mu8Replace_if_present_flag = 0;
    mu8Data_coding = 0;
    mu8SM_default_msg_id = 0;
    mu8SM_length = 0;

    apl_memset(macService_type, 0, sizeof(macService_type));
    apl_memset(macSource_addr, 0, sizeof(macSource_addr));
    apl_memset(macDestination_addr, 0, sizeof(macDestination_addr));
    apl_memset(macSchedule_delivery_time, 0, sizeof(macSchedule_delivery_time));
    apl_memset(macValidity_period, 0, sizeof(macValidity_period));
    apl_memset(macShort_message, 0, sizeof(macShort_message));
}

SMPP_PDU_COPY_IMPL(CSubmitSMPDU);

CSubmitSMRespPDU::CSubmitSMRespPDU()
{
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));
}

SMPP_PDU_COPY_IMPL(CSubmitSMRespPDU);

CDeliverSMPDU::CDeliverSMPDU()
{
    mu8Source_addr_ton = 0;
    mu8Source_addr_npi = 0;
    mu8Dest_addr_ton = 0;
    mu8Dest_addr_npi = 0;
    mu8Esm_class = 0;
    mu8Protocol_id = 0;
    mu8Priority_flag = 0;
    mu8Registered_delivery = 0;
    mu8Replace_if_present_flag = 0;
    mu8Data_coding = 0;
    mu8SM_default_msg_id = 0;
    mu8SM_length = 0;

    apl_memset(macService_type, 0, sizeof(macService_type));
    apl_memset(macSource_addr, 0, sizeof(macSource_addr));
    apl_memset(macDestination_addr, 0, sizeof(macDestination_addr));
    apl_memset(macSchedule_delivery_time, 0, sizeof(macSchedule_delivery_time));
    apl_memset(macValidity_period, 0, sizeof(macValidity_period));
    apl_memset(macShort_message, 0, sizeof(macShort_message));
}

SMPP_PDU_COPY_IMPL(CDeliverSMPDU);

CDeliverSMRespPDU::CDeliverSMRespPDU()
{
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));
}

SMPP_PDU_COPY_IMPL(CDeliverSMRespPDU);

CDataSMPDU::CDataSMPDU()
{
    apl_memset(macService_type, 0, sizeof(macService_type));
    mu8Source_addr_ton = 0;
    mu8Source_addr_npi = 0;
    apl_memset(macSource_addr, 0, sizeof(macSource_addr));
    mu8Dest_addr_ton = 0;
    mu8Dest_addr_npi = 0;
    apl_memset(macDestination_addr, 0, sizeof(macDestination_addr));

    mu8Esm_class = 0;
    mu8Registered_delivery = 0;
    mu8Data_coding = 0;
}

SMPP_PDU_COPY_IMPL(CDataSMPDU);

CDataSMRespPDU::CDataSMRespPDU()
{
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));
}

SMPP_PDU_COPY_IMPL(CDataSMRespPDU);

CCancelSMPDU::CCancelSMPDU()
{
    apl_memset(macService_type, 0, sizeof(macService_type));
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));

    mu8Source_addr_ton = 0;
    mu8Source_addr_npi = 0;
    apl_memset(macSource_addr, 0, sizeof(macSource_addr));
    mu8Dest_addr_ton = 0;
    mu8Dest_addr_npi = 0;
    apl_memset(macDestination_addr, 0, sizeof(macDestination_addr));
}

SMPP_PDU_COPY_IMPL(CCancelSMPDU);

CReplaceSMPDU::CReplaceSMPDU()
{
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));

    mu8Source_addr_ton = 0;
    mu8Source_addr_npi = 0;
    apl_memset(macSource_addr, 0, sizeof(macSource_addr));

    apl_memset(macSchedule_delivery_time, 0, sizeof(macSchedule_delivery_time));
    apl_memset(macValidity_period, 0, sizeof(macValidity_period));
    mu8Registered_delivery = 0;
    mu8SM_default_msg_id = 0;
    mu8SM_length = 0;
    apl_memset(macShort_message, 0, sizeof(macShort_message));
}

SMPP_PDU_COPY_IMPL(CReplaceSMPDU);

CQuerySMPDU::CQuerySMPDU()
{
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));

    mu8Source_addr_ton = 0;
    mu8Source_addr_npi = 0;
    apl_memset(macSource_addr, 0, sizeof(macSource_addr));
}

SMPP_PDU_COPY_IMPL(CQuerySMPDU);

CQuerySMRespPDU::CQuerySMRespPDU()
{
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));
    apl_memset(macFinal_date, 0, sizeof(macFinal_date));

    mu8Message_state = 0;
    mu8Error_code = 0;
}

SMPP_PDU_COPY_IMPL(CQuerySMRespPDU);

CSubmitMultiPDU::CSubmitMultiPDU()
{
    apl_memset(macService_type, 0, sizeof(macService_type));

    mu8Source_addr_ton = 0;
    mu8Source_addr_npi = 0;
    apl_memset(macSource_addr, 0, sizeof(macSource_addr));

    mu8Number_of_dests = 0;
    mu8Esm_class = 0;
    mu8Protocol_id = 0;
    mu8Priority_flag = 0;
    apl_memset(macSchedule_delivery_time, 0, sizeof(macSchedule_delivery_time));
    apl_memset(macValidity_period, 0, sizeof(macValidity_period));
    mu8Registered_delivery = 0;
    mu8Replace_if_present_flag = 0;
    mu8Data_coding = 0;
    mu8SM_default_msg_id = 0;
    mu8SM_length = 0;
    apl_memset(macShort_message, 0, sizeof(macShort_message));
}

SMPP_PDU_COPY_IMPL(CSubmitMultiPDU);

CSubmitMultiRespPDU::CSubmitMultiRespPDU()
{
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));
    mu8No_unsuccess = 0;
}

SMPP_PDU_COPY_IMPL(CSubmitMultiRespPDU);

CAlertNotificationPDU::CAlertNotificationPDU()
{
    mu8Source_addr_ton = 0;
    mu8Source_addr_npi = 0;
    apl_memset(macSource_addr, 0, sizeof(macSource_addr));

    mu8Esme_addr_ton = 0;
    mu8Esme_addr_npi = 0;
    apl_memset(macEsme_addr, 0, sizeof(macEsme_addr));
}

SMPP_PDU_COPY_IMPL(CAlertNotificationPDU);

CSMPPMessage::CSMPPMessage()
{
    miSMPPErrno = AI_SMPP_ERROR_NOERROR;
    mpoPDU = APL_NULL;
}

CSMPPMessage::~CSMPPMessage()
{
    if(mpoPDU != APL_NULL)
        delete mpoPDU;
}

apl_uint32_t CSMPPMessage::GetLength() const
{
    return moHeader.mu32Command_length;
}

apl_uint32_t CSMPPMessage::GetCommandID() const
{
    return moHeader.mu32Command_id;
}

apl_uint32_t CSMPPMessage::GetSequence() const
{
    return moHeader.mu32Sequence_number;
}

apl_uint32_t CSMPPMessage::GetStatus() const
{
    return moHeader.mu32Command_status;
}

IPDUBase* CSMPPMessage::GetPDU() 
{
    return mpoPDU;
}

void CSMPPMessage::SetPDU(const IPDUBase* apoPDU) 
{
    mpoPDU = const_cast<IPDUBase*>(apoPDU);
}

void CSMPPMessage::SetCommandID(apl_uint32_t auCommand)
{
    moHeader.mu32Command_id = auCommand;
}

void CSMPPMessage::SetSequence(apl_uint32_t auSequence)
{
    moHeader.mu32Sequence_number = auSequence;
}

void CSMPPMessage::SetStatus(apl_uint32_t auStatus)
{
    moHeader.mu32Command_status = auStatus;
}

CTLVItem CSMPPMessage::GetTLVItem(apl_uint32_t auIndex)
{
    return moTLVTable[auIndex];
}

apl_size_t CSMPPMessage::GetTLVCount() const
{
    return moTLVTable.size();
}

void CSMPPMessage::RemoveTLVItem(apl_uint32_t auIndex)
{
    std::vector<CTLVItem>::iterator loIter = moTLVTable.begin();
    std::advance(loIter, auIndex);
    this->moTLVTable.erase(loIter);
}

void CSMPPMessage::AddTLV(CTLVItem aoItem)
{
    this->moTLVTable.push_back(aoItem);
}

CSMPPMessage& CSMPPMessage::operator=(const CSMPPMessage& aoMessage) 
{
    if(this != &aoMessage)
    {
        this->moHeader = aoMessage.moHeader;
        this->mu64MessageID = aoMessage.mu64MessageID;
        this->miSMPPErrno = aoMessage.miSMPPErrno;
        this->moTLVTable = aoMessage.moTLVTable;
    }

    this->AllocPDU();

    this->mpoPDU->CopyFrom(aoMessage.mpoPDU);

    return *this;
}

IPDUBase* CSMPPMessage::AllocPDU()
{
    if(this->mpoPDU != NULL)
        ACL_DELETE(this->mpoPDU);

    switch( this -> moHeader.mu32Command_id )
    {
    case AI_SMPP_CMD_BIND_RECEIVER:
    case AI_SMPP_CMD_BIND_TRANSMITTER:
    case AI_SMPP_CMD_BIND_TRANSCEIVER:
        ACL_NEW_ASSERT(this->mpoPDU, CBindPDU);
        break;
    case AI_SMPP_CMD_BIND_RECEIVER_RESP:
    case AI_SMPP_CMD_BIND_TRANSMITTER_RESP:
    case AI_SMPP_CMD_BIND_TRANSCEIVER_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CBindRespPDU);
        break;
    case AI_SMPP_CMD_QUERY_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CQuerySMPDU);
        break;
    case AI_SMPP_CMD_QUERY_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CQuerySMRespPDU);
        break;
    case AI_SMPP_CMD_OUTBIND:
        ACL_NEW_ASSERT(this->mpoPDU, COutBindPDU);
        break;
    case AI_SMPP_CMD_UNBIND:
        // No PDU
        break;
    case AI_SMPP_CMD_UNBIND_RESP:
        // No PDU
        break;
    case AI_SMPP_CMD_GENERIC_NACK:
        // No PDU
        break;
    case AI_SMPP_CMD_SUBMIT_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitSMPDU);
        break;
    case AI_SMPP_CMD_SUBMIT_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitSMRespPDU);
        break;
    case AI_SMPP_CMD_DELIVER_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CDeliverSMPDU);
        break;
    case AI_SMPP_CMD_DELIVER_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CDeliverSMRespPDU);
        break;
    case AI_SMPP_CMD_SUBMIT_MULTI:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitMultiPDU);
        break;
    case AI_SMPP_CMD_SUBMIT_MULTI_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitMultiRespPDU);
        break;
    case AI_SMPP_CMD_DATA_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CDataSMPDU);
        break;
    case AI_SMPP_CMD_DATA_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CDataSMRespPDU);
        break;
    case AI_SMPP_CMD_REPLACE_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CReplaceSMPDU);
        break;
    case AI_SMPP_CMD_REPLACE_SM_RESP:
        // No PDU
        break;
    case AI_SMPP_CMD_CANCEL_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CCancelSMPDU);
        break;
    case AI_SMPP_CMD_CANCEL_SM_RESP:
        // No PDU
        break;
    case AI_SMPP_CMD_ENQUIRE_LINK:
        // No PDU
        break;
    case AI_SMPP_CMD_ENQUIRE_LINK_RESP:
        // No PDU
        break;
    case AI_SMPP_CMD_ALERT_NOTIFICATION:
        ACL_NEW_ASSERT(this->mpoPDU, CAlertNotificationPDU);
        break;
    default:
        this -> miSMPPErrno = AI_SMPP_ERROR_INVILD_COMMAND;           
        return NULL;
    }

    return this -> mpoPDU;
}



apl_ssize_t CSMPPHeader::Decode(const void* apPtr, apl_size_t aiLen)
{
    if( aiLen < AI_SMPP_HEADER_LEN )
        return -1;

    const char* lpc = (const char *)apPtr;

    lpc += DecodeField(&this -> mu32Command_length, lpc);
    lpc += DecodeField(&this -> mu32Command_id, lpc);
    lpc += DecodeField(&this -> mu32Command_status, lpc);
    lpc += DecodeField(&this -> mu32Sequence_number, lpc);

    return lpc - (char *)apPtr;
}

apl_ssize_t CBindPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macSystem_id, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSystem_id), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macPassword, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macPassword), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macSystem_type, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSystem_type), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Interface_version, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Addr_ton, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macAddress_range, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macAddress_range), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
}

apl_ssize_t CBindRespPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader) 
{
    if(aoHeader.mu32Command_status != 0 && aiLen == 0)
    {
        return 0;
    }

    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macSystem_id, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSystem_id), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
}

apl_ssize_t COutBindPDU::Decode(const void* apPtr, apl_size_t aiLen, const  CSMPPHeader& aoHeader) 
{ 
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macSystem_id, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSystem_id), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macPassword, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macPassword), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
}

apl_ssize_t CSubmitSMPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader) 
{ 
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macService_type, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macService_type), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_ton, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macSource_addr, lpc+liOff, \
        AI_SMPP_MIN(sizeof(this -> macSource_addr), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Dest_addr_ton, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Dest_addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macDestination_addr, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macDestination_addr), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Esm_class, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Protocol_id, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Priority_flag, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macSchedule_delivery_time, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSchedule_delivery_time), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macValidity_period, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macValidity_period), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Registered_delivery, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Replace_if_present_flag, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Data_coding, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8SM_default_msg_id, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8SM_length, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldStr(this -> macShort_message, lpc+liOff, \
                AI_SMPP_MIN(this -> mu8SM_length, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
}

apl_ssize_t CSubmitSMRespPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader) 
{ 
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    if( aoHeader.mu32Command_status == AI_SMPP_STATUS_ROK )
    {
        liRetCode = this->DecodeFieldCStr(this->macMessage_id, lpc+liOff, \
                    AI_SMPP_MIN(sizeof(this -> macMessage_id), aiLen-liOff), liOff);
        RETURN_ERR_IF(liRetCode < 0, -1);
    }

    return liOff;
}

apl_ssize_t CDeliverSMPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader) 
{ 
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macService_type, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macService_type), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_ton, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macSource_addr, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSource_addr), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Dest_addr_ton, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Dest_addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macDestination_addr, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macDestination_addr), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Esm_class, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Protocol_id, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Priority_flag, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macSchedule_delivery_time, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSchedule_delivery_time), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macValidity_period, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macValidity_period), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Registered_delivery, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Replace_if_present_flag, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Data_coding, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8SM_default_msg_id, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8SM_length, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldStr(this -> macShort_message, lpc+liOff, \
                AI_SMPP_MIN(this -> mu8SM_length, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
}

apl_ssize_t CDeliverSMRespPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader) 
{
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macMessage_id, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macMessage_id), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
}

apl_ssize_t CDataSMPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader) 
{ 
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macService_type, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macService_type), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_ton, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macSource_addr, lpc+liOff, 
                AI_SMPP_MIN(sizeof(this -> macSource_addr), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Dest_addr_ton, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Dest_addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macDestination_addr, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macDestination_addr), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Esm_class, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Registered_delivery, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Data_coding, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
}

apl_ssize_t CDataSMRespPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader) 
{
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macMessage_id, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macMessage_id), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
}


apl_ssize_t CQuerySMPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macMessage_id, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macMessage_id), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_ton, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macSource_addr, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSource_addr), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
}


apl_ssize_t CQuerySMRespPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macMessage_id, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macMessage_id), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macFinal_date, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macFinal_date), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Message_state, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Error_code, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
}

apl_ssize_t CReplaceSMPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macMessage_id, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macMessage_id), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_ton, lpc+liOff, \
        AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macSource_addr, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSource_addr), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    liRetCode = this->DecodeFieldCStr(this -> macSchedule_delivery_time, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSchedule_delivery_time), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macValidity_period, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macValidity_period), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Registered_delivery, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8SM_default_msg_id, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8SM_length, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldStr(this -> macShort_message, lpc+liOff, \
                AI_SMPP_MIN(this -> mu8SM_length, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
}

apl_ssize_t CCancelSMPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macService_type, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macService_type), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macMessage_id, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macMessage_id), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_ton, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macSource_addr, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSource_addr), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Dest_addr_ton, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Dest_addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macDestination_addr, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macDestination_addr), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
    
}

apl_ssize_t CSubmitMultiPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macService_type, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macService_type), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_ton, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macSource_addr, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSource_addr), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Number_of_dests, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    for( apl_uint32_t liIndex = 0; liIndex < this -> mu8Number_of_dests; ++liIndex)
    {
        CAddress loAddress;

        liRetCode = this->DecodeField(&loAddress.mu8Dest_flag, lpc+liOff, \
                    AI_SMPP_MIN(1, aiLen-liOff), liOff);
        RETURN_ERR_IF(liRetCode < 0, -1);
        
        if(loAddress.mu8Dest_flag == AI_SMPP_MULTI_ADDR_SMEADDR)
        {
            liRetCode = this->DecodeField(&loAddress.moSME_dest_address.mu8Dest_addr_ton, lpc+liOff, \
                        AI_SMPP_MIN(1, aiLen-liOff), liOff);
            RETURN_ERR_IF(liRetCode < 0, -1);
            
            liRetCode = this->DecodeField(&loAddress.moSME_dest_address.mu8Dest_addr_npi, lpc+liOff, \
                        AI_SMPP_MIN(1, aiLen-liOff), liOff);
            RETURN_ERR_IF(liRetCode < 0, -1);
            
            liRetCode = this->DecodeFieldCStr(loAddress.moSME_dest_address.macDestination_addr, lpc+liOff, \
                        AI_SMPP_MIN(sizeof(loAddress.moSME_dest_address.macDestination_addr), aiLen-liOff), liOff);
            RETURN_ERR_IF(liRetCode < 0, -1);
        }
        else // AI_SMPP_MULTI_ADDR_DLADDR
        {
            liRetCode = this->DecodeFieldCStr(loAddress.macDl_name, lpc+liOff, \
                        AI_SMPP_MIN(sizeof(loAddress.macDl_name), aiLen-liOff), liOff);
            RETURN_ERR_IF(liRetCode < 0, -1);
        }
        this -> moDest_address.push_back(loAddress);
    }

    liRetCode = this->DecodeField(&this -> mu8Esm_class, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Protocol_id, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Priority_flag, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macSchedule_delivery_time, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSchedule_delivery_time), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macValidity_period, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macValidity_period), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Registered_delivery, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Replace_if_present_flag, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Data_coding, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8SM_default_msg_id, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8SM_length, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldStr(this -> macShort_message, lpc+liOff, \
                AI_SMPP_MIN(this -> mu8SM_length, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
}

apl_ssize_t CSubmitMultiRespPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeFieldCStr(this -> macMessage_id, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macMessage_id), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8No_unsuccess, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    for(apl_uint32_t liIndex = 0; liIndex < this -> mu8No_unsuccess; ++liIndex)
    {
        CSMEUnsuccess loUnsuccess;

        liRetCode = this->DecodeField(&loUnsuccess.mu8Dest_addr_ton, lpc+liOff, \
                    AI_SMPP_MIN(1, aiLen-liOff), liOff);
        RETURN_ERR_IF(liRetCode < 0, -1);
        
        liRetCode = this->DecodeField(&loUnsuccess.mu8Dest_addr_npi, lpc+liOff, \
                    AI_SMPP_MIN(1, aiLen-liOff), liOff);
        RETURN_ERR_IF(liRetCode < 0, -1);
        
        liRetCode = this->DecodeFieldCStr(loUnsuccess.macDestination_addr, lpc+liOff, \
                    AI_SMPP_MIN(sizeof(loUnsuccess.macDestination_addr), aiLen-liOff), liOff);
        RETURN_ERR_IF(liRetCode < 0, -1);
        
        liRetCode = this->DecodeField(&loUnsuccess.mu32Error_status_code, lpc+liOff, \
                    AI_SMPP_MIN(4, aiLen-liOff), liOff);
        RETURN_ERR_IF(liRetCode < 0, -1);

        this -> moUnsuccess_sme.push_back(loUnsuccess);
    }

    return liOff;
}

apl_ssize_t CAlertNotificationPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMPPHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;
    apl_size_t liOff = 0;
    apl_int_t liRetCode = 0;

    liRetCode = this->DecodeField(&this -> mu8Source_addr_ton, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Source_addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macSource_addr, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macSource_addr), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    liRetCode = this->DecodeField(&this -> mu8Esme_addr_ton, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeField(&this -> mu8Esme_addr_npi, lpc+liOff, \
                AI_SMPP_MIN(1, aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);
    
    liRetCode = this->DecodeFieldCStr(this -> macEsme_addr, lpc+liOff, \
                AI_SMPP_MIN(sizeof(this -> macEsme_addr), aiLen-liOff), liOff);
    RETURN_ERR_IF(liRetCode < 0, -1);

    return liOff;
    
}

apl_ssize_t CSMPPMessage::Decode(const void* apPtr, apl_size_t aiLen)
{
    const char* lpc = (const char *)apPtr;
    apl_int_t liRet = 0;
    apl_size_t luRemainLen;

    liRet = this -> moHeader.Decode(apPtr, aiLen);

    if( liRet < 0 )
    {
        //this -> miSMPPErrno = AI_SMPP_ERROR_LEN_MESSAGE;
        return 0;
    }
    
    if( this -> GetLength() > AI_SMPP_PACKAGE_MAXLEN )
    {
        //field TotalLength is abnormal
        this -> miSMPPErrno = AI_SMPP_ERROR_LEN_MESSAGE;
        return -1;
    }
    
    if( this -> GetLength() > aiLen )
    {
        //haven't finish receive
        return 0;
    }
    
    luRemainLen = this -> GetLength();


    lpc += liRet;
    luRemainLen -= liRet;

    if( mpoPDU != APL_NULL )
        ACL_DELETE(mpoPDU);

    switch( this -> moHeader.mu32Command_id )
    {
    case AI_SMPP_CMD_BIND_RECEIVER:
    case AI_SMPP_CMD_BIND_TRANSMITTER:
    case AI_SMPP_CMD_BIND_TRANSCEIVER:
        ACL_NEW_ASSERT(this->mpoPDU, CBindPDU);
        break;
    case AI_SMPP_CMD_BIND_RECEIVER_RESP:
    case AI_SMPP_CMD_BIND_TRANSMITTER_RESP:
    case AI_SMPP_CMD_BIND_TRANSCEIVER_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CBindRespPDU);
        break;
    case AI_SMPP_CMD_QUERY_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CQuerySMPDU);
        break;
    case AI_SMPP_CMD_QUERY_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CQuerySMRespPDU);
        break;
    case AI_SMPP_CMD_OUTBIND:
        ACL_NEW_ASSERT(this->mpoPDU, COutBindPDU);
        break;
    case AI_SMPP_CMD_UNBIND:
        // No PDU
        break;
    case AI_SMPP_CMD_UNBIND_RESP:
        // No PDU
        break;
    case AI_SMPP_CMD_GENERIC_NACK:
        // No PDU
        break;
    case AI_SMPP_CMD_SUBMIT_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitSMPDU);
        break;
    case AI_SMPP_CMD_SUBMIT_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitSMRespPDU);
        break;
    case AI_SMPP_CMD_DELIVER_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CDeliverSMPDU);
        break;
    case AI_SMPP_CMD_DELIVER_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CDeliverSMRespPDU);
        break;
    case AI_SMPP_CMD_SUBMIT_MULTI:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitMultiPDU);
        break;
    case AI_SMPP_CMD_SUBMIT_MULTI_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitMultiRespPDU);
        break;
    case AI_SMPP_CMD_DATA_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CDataSMPDU);
        break;
    case AI_SMPP_CMD_DATA_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CDataSMRespPDU);
        break;
    case AI_SMPP_CMD_REPLACE_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CReplaceSMPDU);
        break;
    case AI_SMPP_CMD_REPLACE_SM_RESP:
        // No PDU
        break;
    case AI_SMPP_CMD_CANCEL_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CCancelSMPDU);
        break;
    case AI_SMPP_CMD_CANCEL_SM_RESP:
        // No PDU
        break;
    case AI_SMPP_CMD_ENQUIRE_LINK:
        // No PDU
        break;
    case AI_SMPP_CMD_ENQUIRE_LINK_RESP:
        // No PDU
        break;
    case AI_SMPP_CMD_ALERT_NOTIFICATION:
        ACL_NEW_ASSERT(this->mpoPDU, CAlertNotificationPDU);
        break;
    default:
        this -> miSMPPErrno = AI_SMPP_ERROR_INVILD_COMMAND;           
        return -1;
    }

    liRet = 0;

    if(mpoPDU != APL_NULL)
        liRet = mpoPDU -> Decode(lpc, luRemainLen, moHeader);


    if( liRet < 0 )
    {
        this -> miSMPPErrno = AI_SMPP_ERROR_LEN_MESSAGE;
        return -1;
    }
/*
    lpc += liRet;
    luRemainLen -= liRet;

    //printf("Before TLV: %d\n", luRemainLen);
    liRet = this -> DecodeTLV(lpc, luRemainLen, moHeader);

    if( liRet < 0 )
    {
        this -> miSMPPErrno = AI_SMPP_ERROR_LEN_MESSAGE;
        return -1;
    }

    lpc += liRet;
    luRemainLen -= liRet;
*/
    return this->GetLength();
}


apl_ssize_t CSMPPMessage::DecodeTLV(void const* apPtr, apl_size_t aiLen, const CSMPPHeader& Header)
{
    apl_int32_t luRemainLen = aiLen;
    
    const char* lpc = (const char *)apPtr;
    const char* lpcOld = lpc;

    while( luRemainLen > 4 )
    {
        CTLVItem loItem;
        
        lpc += DecodeField(&loItem.mu16Tag, lpc);
        lpc += DecodeField(&loItem.mu16Len, lpc);
       
        if (luRemainLen < 4 /*length of the T & L*/ + loItem.mu16Len /*length of the V*/)
        {
            break;
        }
 
        loItem.moValue = std::vector<apl_uint8_t>(lpc, lpc + loItem.mu16Len);
        lpc += loItem.mu16Len;

        moTLVTable.push_back(loItem);
        luRemainLen -= lpc - lpcOld;
        lpcOld = lpc;
    }
    
    return lpcOld - (const char *)apPtr;
}


// public functions
apl_ssize_t DecodeFieldCStr(char* const apDest, const char* const apcSrc, apl_size_t aiLen)
{
    const char *lpc = (const char *)apcSrc;
    apl_uint_t liIndex = 0;

    while( liIndex != aiLen && *lpc != 0 )
    {
        ++liIndex;
        ++lpc;
    }

    apl_memcpy(apDest, apcSrc, liIndex + 1);

    if(*lpc != 0)
    {
        apDest[liIndex] = 0;
    }

    return  liIndex + 1;
}

inline apl_ssize_t DecodeFieldStr(char* const apcDest, const char* const apcSrc, apl_size_t aiLen)
{
    apl_memcpy(apcDest, apcSrc, aiLen);
    
    return  aiLen;
}

inline apl_ssize_t DecodeField(apl_uint8_t* const aiDest, const void* const apcSrc)
{
    *aiDest = *(apl_uint8_t *)apcSrc;

    return 1;   // 1 byte decoded
}


inline apl_ssize_t DecodeField(apl_uint16_t* const aiDest, const void* const apcSrc)
{
    apl_uint16_t lu16;

    apl_memcpy(&lu16, apcSrc, 2);

    *aiDest = apl_ntoh16(lu16);

    return 2;   // 2 bytes decoded
}


inline apl_ssize_t DecodeField(apl_uint32_t* const aiDest, const void* const apcSrc)
{
    apl_uint32_t lu32;

    apl_memcpy(&lu32, apcSrc, 4);

    *aiDest = apl_ntoh32(lu32);

    return 4;   // 4 bytes decoded
}

apl_ssize_t CSMPPHeader::Encode(void* apPtr, apl_size_t aiLen) const
{
    if( aiLen < AI_SMPP_HEADER_LEN )
        return -1;

    apl_uint32_t* lpu = (apl_uint32_t *)apPtr;

    *lpu++ = apl_hton32(this -> mu32Command_length);
    *lpu++ = apl_hton32(this -> mu32Command_id);
    *lpu++ = apl_hton32(this -> mu32Command_status);
    *lpu++ = apl_hton32(this -> mu32Sequence_number);

    return (char *)lpu - (char *)apPtr;
}

apl_ssize_t CBindPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macSystem_id, AI_SMPP_MAXLEN_SYSTEM_ID);
    lpc += EncodeFieldCStr(lpc, this -> macPassword, AI_SMPP_MAXLEN_PASSWORD);
    lpc += EncodeFieldCStr(lpc, this -> macSystem_type, AI_SMPP_MAXLEN_SYSTEM_TYPE);
    lpc += EncodeField(lpc, this -> mu8Interface_version);  
    lpc += EncodeField(lpc, this -> mu8Addr_ton);
    lpc += EncodeField(lpc, this -> mu8Addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macAddress_range, AI_SMPP_MAXLEN_ADDRESS_RANGE);

    return lpc - (char *)apPtr;
}

apl_ssize_t CBindRespPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macSystem_id, AI_SMPP_MAXLEN_SYSTEM_ID);

    return lpc - (char *)apPtr;
}

apl_ssize_t COutBindPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{ 
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macSystem_id, AI_SMPP_MAXLEN_SYSTEM_ID);
    lpc += EncodeFieldCStr(lpc, this -> macPassword, AI_SMPP_MAXLEN_PASSWORD);

    return lpc - (char *)apPtr;
}

apl_ssize_t CSubmitSMPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{ 
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macService_type, AI_SMPP_MAXLEN_SERVICE_TYPE);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMPP_MAXLEN_SOURCE_ADDR);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macDestination_addr, AI_SMPP_MAXLEN_DEST_ADDR);
    lpc += EncodeField(lpc, this -> mu8Esm_class);
    lpc += EncodeField(lpc, this -> mu8Protocol_id);
    lpc += EncodeField(lpc, this -> mu8Priority_flag);
    lpc += EncodeFieldCStr(lpc, this -> macSchedule_delivery_time, AI_SMPP_MAXLEN_SCHEDULE_TIME);
    lpc += EncodeFieldCStr(lpc, this -> macValidity_period, AI_SMPP_MAXLEN_VALIDITY_PERIOD);
    lpc += EncodeField(lpc, this -> mu8Registered_delivery);
    lpc += EncodeField(lpc, this -> mu8Replace_if_present_flag);
    lpc += EncodeField(lpc, this -> mu8Data_coding);
    lpc += EncodeField(lpc, this -> mu8SM_default_msg_id);
    lpc += EncodeField(lpc, this -> mu8SM_length);
    lpc += EncodeFieldStr(lpc, this -> macShort_message, this -> mu8SM_length);

    return lpc - (char *)apPtr;
}

apl_ssize_t CSubmitSMRespPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{ 
    char* lpc = (char *)apPtr;

    if( aoHeader.mu32Command_status == AI_SMPP_STATUS_ROK )
    {
        lpc += EncodeFieldCStr(lpc, this->macMessage_id, AI_SMPP_MAXLEN_MESSAGE_ID);
    }

    return lpc - (char *)apPtr;
}

apl_ssize_t CDeliverSMPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{ 
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macService_type, AI_SMPP_MAXLEN_SERVICE_TYPE);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMPP_MAXLEN_SOURCE_ADDR);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_npi);
    lpc += EncodeFieldCStr(lpc , this -> macDestination_addr, AI_SMPP_MAXLEN_DEST_ADDR);
    lpc += EncodeField(lpc, this -> mu8Esm_class);
    lpc += EncodeField(lpc, this -> mu8Protocol_id);
    lpc += EncodeField(lpc, this -> mu8Priority_flag);
    lpc += EncodeFieldCStr(lpc, this -> macSchedule_delivery_time, AI_SMPP_MAXLEN_SCHEDULE_TIME);
    lpc += EncodeFieldCStr(lpc, this -> macValidity_period, AI_SMPP_MAXLEN_VALIDITY_PERIOD);
    lpc += EncodeField(lpc, this -> mu8Registered_delivery);
    lpc += EncodeField(lpc, this -> mu8Replace_if_present_flag);
    lpc += EncodeField(lpc, this -> mu8Data_coding);
    lpc += EncodeField(lpc, this -> mu8SM_default_msg_id);
    lpc += EncodeField(lpc, this -> mu8SM_length);
    lpc += EncodeFieldStr(lpc, this -> macShort_message, this -> mu8SM_length);

    return lpc - (char *)apPtr;
}

apl_ssize_t CDeliverSMRespPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const 
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMPP_MAXLEN_MESSAGE_ID);

    return lpc - (char *)apPtr;
}

apl_ssize_t CDataSMPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{ 
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macService_type, AI_SMPP_MAXLEN_SERVICE_TYPE);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMPP_MAXLEN_SOURCE_ADDR);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_npi);
    lpc += EncodeFieldCStr(lpc , this -> macDestination_addr, AI_SMPP_MAXLEN_DEST_ADDR);
    lpc += EncodeField(lpc, this -> mu8Esm_class);
    lpc += EncodeField(lpc, this -> mu8Registered_delivery);
    lpc += EncodeField(lpc, this -> mu8Data_coding);

    return lpc - (char *)apPtr;
}

apl_ssize_t CDataSMRespPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const 
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMPP_MAXLEN_MESSAGE_ID);

    return lpc - (char *)apPtr;
}


apl_ssize_t CQuerySMPDU::Encode(void* const apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMPP_MAXLEN_MESSAGE_ID);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMPP_MAXLEN_SOURCE_ADDR);

    return lpc - (char *)apPtr;
}


apl_ssize_t CQuerySMRespPDU::Encode(void* const apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMPP_MAXLEN_MESSAGE_ID);
    lpc += EncodeFieldCStr(lpc, this -> macFinal_date, AI_SMPP_MAXLEN_FINAL_DATE);
    lpc += EncodeField(lpc, this -> mu8Message_state);
    lpc += EncodeField(lpc, this -> mu8Error_code);

    return lpc - (char *)apPtr;
}

apl_ssize_t CReplaceSMPDU::Encode(void* const apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMPP_MAXLEN_MESSAGE_ID);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMPP_MAXLEN_SOURCE_ADDR);

    lpc += EncodeFieldCStr(lpc, this -> macSchedule_delivery_time, AI_SMPP_MAXLEN_SCHEDULE_TIME);
    lpc += EncodeFieldCStr(lpc, this -> macValidity_period, AI_SMPP_MAXLEN_VALIDITY_PERIOD);
    lpc += EncodeField(lpc, this -> mu8Registered_delivery);
    lpc += EncodeField(lpc, this -> mu8SM_default_msg_id);
    lpc += EncodeField(lpc, this -> mu8SM_length);
    lpc += EncodeFieldStr(lpc, this -> macShort_message, this -> mu8SM_length);

    return lpc - (char *)apPtr;
}

apl_ssize_t CCancelSMPDU::Encode(void* const apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macService_type, AI_SMPP_MAXLEN_SERVICE_TYPE);
    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMPP_MAXLEN_MESSAGE_ID);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMPP_MAXLEN_SOURCE_ADDR);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macDestination_addr, AI_SMPP_MAXLEN_DEST_ADDR);

    return lpc - (char *)apPtr;
}

apl_ssize_t CSubmitMultiPDU::Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macService_type, AI_SMPP_MAXLEN_SERVICE_TYPE);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMPP_MAXLEN_SOURCE_ADDR);
    
    lpc += EncodeField(lpc, this -> mu8Number_of_dests);

    for( apl_uint32_t liIndex = 0; liIndex < this -> mu8Number_of_dests; ++liIndex)
    {
        lpc += EncodeField(lpc, moDest_address[liIndex].mu8Dest_flag);
        if(moDest_address[liIndex].mu8Dest_flag == AI_SMPP_MULTI_ADDR_SMEADDR)
        {
            lpc += EncodeField(lpc, moDest_address[liIndex].moSME_dest_address.mu8Dest_addr_ton);
            lpc += EncodeField(lpc, moDest_address[liIndex].moSME_dest_address.mu8Dest_addr_npi);
            lpc += EncodeFieldCStr(lpc, moDest_address[liIndex].moSME_dest_address.macDestination_addr, AI_SMPP_MAXLEN_DEST_ADDR);
        }
        else // AI_SMPP_MULTI_ADDR_DLADDR
        {
            lpc += EncodeFieldCStr(lpc, moDest_address[liIndex].macDl_name, AI_SMPP_MAXLEN_DEST_ADDR);
        }
    }

    lpc += EncodeField(lpc, this -> mu8Esm_class);
    lpc += EncodeField(lpc, this -> mu8Protocol_id);
    lpc += EncodeField(lpc, this -> mu8Priority_flag);
    lpc += EncodeFieldCStr(lpc, this -> macSchedule_delivery_time, AI_SMPP_MAXLEN_SCHEDULE_TIME);
    lpc += EncodeFieldCStr(lpc, this -> macValidity_period, AI_SMPP_MAXLEN_VALIDITY_PERIOD);
    lpc += EncodeField(lpc, this -> mu8Registered_delivery);
    lpc += EncodeField(lpc, this -> mu8Replace_if_present_flag);
    lpc += EncodeField(lpc, this -> mu8Data_coding);
    lpc += EncodeField(lpc, this -> mu8SM_default_msg_id);
    lpc += EncodeField(lpc, this -> mu8SM_length);
    lpc += EncodeFieldStr(lpc, this -> macShort_message, this -> mu8SM_length);

    return lpc - (char *)apPtr;
}

apl_ssize_t CSubmitMultiRespPDU::Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMPP_MAXLEN_MESSAGE_ID);
    lpc += EncodeField(lpc, this -> mu8No_unsuccess);

    for(apl_uint32_t liIndex = 0; liIndex < this -> mu8No_unsuccess; ++liIndex)
    {
        lpc += EncodeField(lpc, moUnsuccess_sme[liIndex].mu8Dest_addr_ton);
        lpc += EncodeField(lpc, moUnsuccess_sme[liIndex].mu8Dest_addr_npi);
        lpc += EncodeFieldCStr(lpc, moUnsuccess_sme[liIndex].macDestination_addr, AI_SMPP_MAXLEN_DEST_ADDR);
        lpc += EncodeField(lpc, moUnsuccess_sme[liIndex].mu32Error_status_code);
    }

    return lpc - (char *)apPtr;
}

apl_ssize_t CAlertNotificationPDU::Encode(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMPP_MAXLEN_SOURCE_ADDR);

    lpc += EncodeField(lpc, this -> mu8Esme_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Esme_addr_npi);
    lpc += EncodeFieldCStr(lpc , this -> macEsme_addr, AI_SMPP_MAXLEN_ESME_ADDR);

    return lpc - (char *)apPtr;
}


apl_ssize_t CSMPPMessage::Encode(void* apPtr, apl_size_t aiLen)
{
    char* lpc = (char *)apPtr;
    apl_int_t liRet = 0;
    apl_size_t luRemainLen = aiLen;

    liRet = this -> moHeader.Encode(apPtr, luRemainLen);

    if( liRet < 0 )
    {
        this -> miSMPPErrno = AI_SMPP_ERROR_LEN_MESSAGE;
        return -1;
    }

    lpc += liRet;
    luRemainLen -= liRet;

    if( this -> GetLength() > aiLen )
    {
        this -> miSMPPErrno = AI_SMPP_ERROR_LEN_MESSAGE;
        return -1;
    }

    liRet = 0;

    if(mpoPDU != NULL)
        liRet = mpoPDU -> Encode(lpc, luRemainLen, moHeader);

    
    if( liRet < 0 )
    {
        this -> miSMPPErrno = AI_SMPP_ERROR_LEN_MESSAGE;
        return -1;
    }

    lpc += liRet;
    luRemainLen -= liRet;

    liRet = this -> EncodeTLV(lpc, luRemainLen, moHeader);

    if( liRet < 0 )
    {
        this -> miSMPPErrno = AI_SMPP_ERROR_LEN_MESSAGE;
        return -1;
    }

    lpc += liRet;
    luRemainLen -= liRet;

    // Encode Header again to set the mu32Command_length field
    this -> moHeader.mu32Command_length = lpc - (char *) apPtr;
    moHeader.Encode(apPtr, aiLen);

    return lpc - (char *)apPtr;
}

apl_ssize_t CSMPPMessage::EncodeTLV(void* apPtr, apl_size_t aiLen, CSMPPHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;
    char* lpcOld = lpc;
    apl_uint32_t luRemainLen = aiLen;
    std::vector<CTLVItem>::const_iterator loIter;
    
    if( moTLVTable.size() == 0 )
    {
        return 0;
    }

    if( luRemainLen < 4 )
        return -1;

    for( loIter = moTLVTable.begin(); loIter != moTLVTable.end() && luRemainLen > 4; ++loIter)
    {
        lpc += EncodeField(lpc, (*loIter).mu16Tag);
        lpc += EncodeField(lpc, (*loIter).mu16Len);
        copy((*loIter).moValue.begin(), (*loIter).moValue.begin() + (*loIter).mu16Len, lpc);
        lpc += (*loIter).mu16Len;

        luRemainLen -= lpc - lpcOld;
        lpcOld = lpc;
    }

    if(loIter != moTLVTable.end())
        return -1;

    return lpc - (const char *)apPtr;
}


// public functions
apl_ssize_t EncodeFieldCStr(char* const apDest, const char* const apcSrc, apl_size_t aiLen)
{
    char *lpc = (char *)apcSrc;
    apl_uint_t liIndex = 0;

    while( liIndex != aiLen && *lpc != 0 )
    {
        ++liIndex;
        ++lpc;
    }

    apl_memcpy(apDest, apcSrc, liIndex + 1);

    return  liIndex + 1;
}


inline apl_ssize_t EncodeFieldStr(char* const apDest, const char* const apcSrc, apl_size_t aiLen)
{
    apl_memcpy(apDest, apcSrc, aiLen);
    
    return  aiLen;
}

inline apl_ssize_t EncodeField( void* const apDest, apl_uint8_t const aiSrc)
{
    *(apl_uint8_t *)apDest = aiSrc;

    return 1;   // 1 byte decoded
}

inline apl_ssize_t EncodeField(void* const apcDest, apl_uint16_t const aiSrc)
{
    apl_uint16_t lu16;

    lu16 = apl_hton16(aiSrc);

    apl_memcpy(apcDest, &lu16, 2);

    return 2;   // 2 bytes decoded
}


inline apl_ssize_t EncodeField(void* const apcDest, apl_uint32_t const aiSrc)
{
    apl_uint32_t lu32;

    lu32 = apl_hton32(aiSrc);

    apl_memcpy(apcDest, &lu32, 4);

    return 4;   // 4 bytes decoded
}

// -- 20111206
apl_ssize_t IPDUBase::DecodeFieldCStr(char* const apDest, const char* const apcSrc, const apl_int_t aiLen, apl_size_t &aiOff)
{
/*    
    const char *lpc = (const char *)apcSrc;
    int liIndex = 0;

    while( liIndex != aiLen - 1 && *lpc != 0 )
    {
        ++liIndex;
        ++lpc;
    }

    if(liIndex == aiLen - 1)
    {
        if(*lpc != '\0')
        {
            this->SetValidFlag(false);
        }
    }

    memcpy(apDest, apcSrc, liIndex + 1);

    if(*lpc != 0)
    {
        apDest[liIndex] = 0; 
    }    

    return  liIndex + 1; 
*/
    if( aiLen < 0 ) return -1;

    apl_size_t liOff = 0;
    const char *lpSrc =  (const char*)apcSrc;
    
    const char *lpc = (const char*)apl_memchr( lpSrc, '\0', aiLen );
    if( APL_NULL == lpc )
    {
        this->SetValidFlag(false);
        return -1;
    }

    apl_strncpy(apDest, lpSrc, aiLen);

    liOff = lpc - lpSrc + 1;
    aiOff += liOff;
    
    return liOff;
}

apl_ssize_t IPDUBase::DecodeFieldStr(char* const apcDest, const char* const apcSrc, const apl_int_t aiLen, apl_size_t &aiOff)
{
    if( aiLen < 0 )
    {
        this->SetValidFlag(false);
        return -1;
    }

    if( aiLen == 0 ) return 0;
    
    apl_memcpy(apcDest, apcSrc, aiLen);

    aiOff += aiLen;
    
    return  aiLen;
}

apl_ssize_t IPDUBase::DecodeField(apl_uint8_t* const aiDest, const void* const apcSrc, const apl_int_t aiLen, apl_size_t &aiOff)
{
    if( aiLen < 1 )
    {
        this->SetValidFlag(false);
        return -1;
    }
    
    *aiDest = *(apl_uint8_t *)apcSrc;

    aiOff += 1;

    return 1;   // 1 byte decoded
}

apl_ssize_t IPDUBase::DecodeField(apl_uint32_t* const aiDest, const void* const apcSrc, const apl_int_t aiLen, apl_size_t &aiOff)
{
    if( aiLen < 4 )
    {
        this->SetValidFlag(false);
        return -1;
    }
    
    apl_uint32_t lu32;

    apl_memcpy(&lu32, apcSrc, 4);

    *aiDest = apl_ntoh32(lu32);
    
    aiOff += 4;

    return 4;   // 4 bytes decoded
}


} // namespace smpp

