#include "anf/protocol/SMMCMessage.h"

namespace smmc
{

CSMMCHeader::CSMMCHeader()
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

CBindRespPDU::CBindRespPDU()
{
    apl_memset(macSystem_id, 0, sizeof(macSystem_id));
}

COutBindPDU::COutBindPDU()
{
    apl_memset(macSystem_id, 0, sizeof(macSystem_id));
    apl_memset(macPassword, 0, sizeof(macPassword));
}

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


CSubmitSMRespPDU::CSubmitSMRespPDU()
{
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));
}

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

CDeliverSMRespPDU::CDeliverSMRespPDU()
{
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));
}

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

CDataSMRespPDU::CDataSMRespPDU()
{
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));
}

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

CQuerySMPDU::CQuerySMPDU()
{
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));

    mu8Source_addr_ton = 0;
    mu8Source_addr_npi = 0;
    apl_memset(macSource_addr, 0, sizeof(macSource_addr));
}

CQuerySMRespPDU::CQuerySMRespPDU()
{
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));
    apl_memset(macFinal_date, 0, sizeof(macFinal_date));

    mu8Message_state = 0;
    mu8Error_code = 0;
}

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

CSubmitMultiRespPDU::CSubmitMultiRespPDU()
{
    apl_memset(macMessage_id, 0, sizeof(macMessage_id));
    mu8No_unsuccess = 0;
}

CAlertNotificationPDU::CAlertNotificationPDU()
{
    mu8Source_addr_ton = 0;
    mu8Source_addr_npi = 0;
    apl_memset(macSource_addr, 0, sizeof(macSource_addr));

    mu8Esme_addr_ton = 0;
    mu8Esme_addr_npi = 0;
    apl_memset(macEsme_addr, 0, sizeof(macEsme_addr));
}


CSMMCMessage::CSMMCMessage()
{
    miSMMCErrno = AI_SMMC_ERROR_NOERROR;
    mpoPDU = APL_NULL;
}

CSMMCMessage::~CSMMCMessage()
{
    if(mpoPDU != APL_NULL)
        delete mpoPDU;
}

apl_uint32_t CSMMCMessage::GetLength() const
{
    return moHeader.mu32Command_length;
}

apl_uint32_t CSMMCMessage::GetCommandID() const
{
    return moHeader.mu32Command_id;
}

apl_uint32_t CSMMCMessage::GetSequence() const
{
    return moHeader.mu32Sequence_number;
}

apl_uint32_t CSMMCMessage::GetStatus() const
{
    return moHeader.mu32Command_status;
}

const IPDUBase* CSMMCMessage::GetPDU() const
{
    return mpoPDU;
}

void CSMMCMessage::SetCommandID(apl_uint32_t auCommand)
{
    moHeader.mu32Command_id = auCommand;
}

void CSMMCMessage::SetSequence(apl_uint32_t auSequence)
{
    moHeader.mu32Sequence_number = auSequence;
}

void CSMMCMessage::SetStatus(apl_uint32_t auStatus)
{
    moHeader.mu32Command_status = auStatus;
}

CTLVItem CSMMCMessage::GetTLVItem(apl_uint32_t auIndex)
{
    return moTLVTable[auIndex];
}

apl_size_t CSMMCMessage::GetTLVCount() const
{
    return moTLVTable.size();
}

void CSMMCMessage::RemoveTLVItem(apl_uint32_t auIndex)
{
    std::vector<CTLVItem>::iterator loIter = moTLVTable.begin();
    std::advance(loIter, auIndex);
    moTLVTable.erase(loIter);
}

void CSMMCMessage::AddTLV(CTLVItem aoItem)
{
    moTLVTable.push_back(aoItem);
}

IPDUBase* CSMMCMessage::AllocPDU()
{
    if(this->mpoPDU != NULL)
        ACL_DELETE(this->mpoPDU);

    switch( this -> moHeader.mu32Command_id )
    {
    case AI_SMMC_CMD_BIND_RECEIVER:
    case AI_SMMC_CMD_BIND_TRANSMITTER:
    case AI_SMMC_CMD_BIND_TRANSCEIVER:
        ACL_NEW_ASSERT(this->mpoPDU, CBindPDU);
        break;
    case AI_SMMC_CMD_BIND_RECEIVER_RESP:
    case AI_SMMC_CMD_BIND_TRANSMITTER_RESP:
    case AI_SMMC_CMD_BIND_TRANSCEIVER_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CBindRespPDU);
        break;
    case AI_SMMC_CMD_QUERY_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CQuerySMPDU);
        break;
    case AI_SMMC_CMD_QUERY_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CQuerySMRespPDU);
        break;
    case AI_SMMC_CMD_OUTBIND:
        ACL_NEW_ASSERT(this->mpoPDU, COutBindPDU);
        break;
    case AI_SMMC_CMD_UNBIND:
        // No PDU
        break;
    case AI_SMMC_CMD_UNBIND_RESP:
        // No PDU
        break;
    case AI_SMMC_CMD_GENERIC_NACK:
        // No PDU
        break;
    case AI_SMMC_CMD_SUBMIT_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitSMPDU);
        break;
    case AI_SMMC_CMD_SUBMIT_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitSMRespPDU);
        break;
    case AI_SMMC_CMD_DELIVER_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CDeliverSMPDU);
        break;
    case AI_SMMC_CMD_DELIVER_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CDeliverSMRespPDU);
        break;
    case AI_SMMC_CMD_SUBMIT_MULTI:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitMultiPDU);
        break;
    case AI_SMMC_CMD_SUBMIT_MULTI_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitMultiRespPDU);
        break;
    case AI_SMMC_CMD_DATA_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CDataSMPDU);
        break;
    case AI_SMMC_CMD_DATA_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CDataSMRespPDU);
        break;
    case AI_SMMC_CMD_REPLACE_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CReplaceSMPDU);
        break;
    case AI_SMMC_CMD_REPLACE_SM_RESP:
        // No PDU
        break;
    case AI_SMMC_CMD_CANCEL_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CCancelSMPDU);
        break;
    case AI_SMMC_CMD_CANCEL_SM_RESP:
        // No PDU
        break;
    case AI_SMMC_CMD_ENQUIRE_LINK:
        // No PDU
        break;
    case AI_SMMC_CMD_ENQUIRE_LINK_RESP:
        // No PDU
        break;
    case AI_SMMC_CMD_ALERT_NOTIFICATION:
        ACL_NEW_ASSERT(this->mpoPDU, CAlertNotificationPDU);
        break;
    default:
        this -> miSMMCErrno = AI_SMMC_ERROR_INVILD_COMMAND;           
        return NULL;
    }

    return this -> mpoPDU;
}


apl_ssize_t CSMMCHeader::Decode(const void* apPtr, apl_size_t aiLen)
{
    if( aiLen < AI_SMMC_HEADER_LEN )
        return -1;

    const char* lpc = (const char *)apPtr;

    lpc += DecodeField(&this -> mu32Command_length, lpc);
    lpc += DecodeField(&this -> mu32Command_id, lpc);
    lpc += DecodeField(&this -> mu32Command_status, lpc);
    lpc += DecodeField(&this -> mu32Sequence_number, lpc);

    return lpc - (char *)apPtr;
}

apl_ssize_t CBindPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macSystem_id, lpc, AI_SMMC_MAXLEN_SYSTEM_ID);
    lpc += DecodeFieldCStr(this -> macPassword, lpc, AI_SMMC_MAXLEN_PASSWORD);
    lpc += DecodeFieldCStr(this -> macSystem_type, lpc, AI_SMMC_MAXLEN_SYSTEM_TYPE);
    lpc += DecodeField(&this -> mu8Interface_version, lpc);  
    lpc += DecodeField(&this -> mu8Addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macAddress_range, lpc, AI_SMMC_MAXLEN_ADDRESS_RANGE);

    return lpc - (char *)apPtr;
}

apl_ssize_t CBindRespPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader) 
{
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macSystem_id, lpc, AI_SMMC_MAXLEN_SYSTEM_ID);

    return lpc - (const char *)apPtr;
}

apl_ssize_t COutBindPDU::Decode(const void* apPtr, apl_size_t aiLen, const  CSMMCHeader& aoHeader) 
{ 
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macSystem_id, lpc, AI_SMMC_MAXLEN_SYSTEM_ID);
    lpc += DecodeFieldCStr(this -> macPassword, lpc, AI_SMMC_MAXLEN_PASSWORD);

    return lpc - (const char *)apPtr;
}

apl_ssize_t CSubmitSMPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader) 
{ 
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macService_type, lpc, AI_SMMC_MAXLEN_SERVICE_TYPE);
    lpc += DecodeField(&this -> mu8Source_addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Source_addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macSource_addr, lpc, AI_SMMC_MAXLEN_SOURCE_ADDR);
    lpc += DecodeField(&this -> mu8Dest_addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Dest_addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macDestination_addr, lpc, AI_SMMC_MAXLEN_DEST_ADDR);
    lpc += DecodeField(&this -> mu8Esm_class, lpc);
    lpc += DecodeField(&this -> mu8Protocol_id, lpc);
    lpc += DecodeField(&this -> mu8Priority_flag, lpc);
    lpc += DecodeFieldCStr(this -> macSchedule_delivery_time, lpc, AI_SMMC_MAXLEN_SCHEDULE_TIME);
    lpc += DecodeFieldCStr(this -> macValidity_period, lpc, AI_SMMC_MAXLEN_VALIDITY_PERIOD);
    lpc += DecodeField(&this -> mu8Registered_delivery, lpc);
    lpc += DecodeField(&this -> mu8Replace_if_present_flag, lpc);
    lpc += DecodeField(&this -> mu8Data_coding, lpc);
    lpc += DecodeField(&this -> mu8SM_default_msg_id, lpc);
    lpc += DecodeField(&this -> mu8SM_length, lpc);
    lpc += DecodeFieldStr(this -> macShort_message, lpc, this -> mu8SM_length);

    return lpc - (const char *)apPtr;
}

apl_ssize_t CSubmitSMRespPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader) 
{ 
    return aoHeader.mu32Command_length - 16;
}

apl_ssize_t CDeliverSMPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader) 
{ 
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macService_type, lpc, AI_SMMC_MAXLEN_SERVICE_TYPE);
    lpc += DecodeField(&this -> mu8Source_addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Source_addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macSource_addr, lpc, AI_SMMC_MAXLEN_SOURCE_ADDR);
    lpc += DecodeField(&this -> mu8Dest_addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Dest_addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macDestination_addr, lpc, AI_SMMC_MAXLEN_DEST_ADDR);
    lpc += DecodeField(&this -> mu8Esm_class, lpc);
    lpc += DecodeField(&this -> mu8Protocol_id, lpc);
    lpc += DecodeField(&this -> mu8Priority_flag, lpc);
    lpc += DecodeFieldCStr(this -> macSchedule_delivery_time, lpc, AI_SMMC_MAXLEN_SCHEDULE_TIME);
    lpc += DecodeFieldCStr(this -> macValidity_period, lpc, AI_SMMC_MAXLEN_VALIDITY_PERIOD);
    lpc += DecodeField(&this -> mu8Registered_delivery, lpc);
    lpc += DecodeField(&this -> mu8Replace_if_present_flag, lpc);
    lpc += DecodeField(&this -> mu8Data_coding, lpc);
    lpc += DecodeField(&this -> mu8SM_default_msg_id, lpc);
    lpc += DecodeField(&this -> mu8SM_length, lpc);
    lpc += DecodeFieldStr(this -> macShort_message, lpc, this -> mu8SM_length);

    return lpc - (char *)apPtr;
}

apl_ssize_t CDeliverSMRespPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader) 
{
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macMessage_id, lpc, AI_SMMC_MAXLEN_MESSAGE_ID);

    return lpc - (char *)apPtr;
}

apl_ssize_t CDataSMPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader) 
{ 
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macService_type, lpc, AI_SMMC_MAXLEN_SERVICE_TYPE);
    lpc += DecodeField(&this -> mu8Source_addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Source_addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macSource_addr, lpc, AI_SMMC_MAXLEN_SOURCE_ADDR);
    lpc += DecodeField(&this -> mu8Dest_addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Dest_addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macDestination_addr, lpc, AI_SMMC_MAXLEN_DEST_ADDR);
    lpc += DecodeField(&this -> mu8Esm_class, lpc);
    lpc += DecodeField(&this -> mu8Registered_delivery, lpc);
    lpc += DecodeField(&this -> mu8Data_coding, lpc);

    return lpc - (char *)apPtr;
}

apl_ssize_t CDataSMRespPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader) 
{
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macMessage_id, lpc, AI_SMMC_MAXLEN_MESSAGE_ID);

    return lpc - (char *)apPtr;
}


apl_ssize_t CQuerySMPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macMessage_id, lpc, AI_SMMC_MAXLEN_MESSAGE_ID);
    lpc += DecodeField(&this -> mu8Source_addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Source_addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macSource_addr, lpc, AI_SMMC_MAXLEN_SOURCE_ADDR);

    return lpc - (const char *)apPtr;
}


apl_ssize_t CQuerySMRespPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macMessage_id, lpc, AI_SMMC_MAXLEN_MESSAGE_ID);
    lpc += DecodeFieldCStr(this -> macFinal_date, lpc, AI_SMMC_MAXLEN_FINAL_DATE);
    lpc += DecodeField(&this -> mu8Message_state, lpc);
    lpc += DecodeField(&this -> mu8Error_code, lpc);

    return lpc - (const char *)apPtr;
}

apl_ssize_t CReplaceSMPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macMessage_id, lpc, AI_SMMC_MAXLEN_MESSAGE_ID);
    lpc += DecodeField(&this -> mu8Source_addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Source_addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macSource_addr, lpc, AI_SMMC_MAXLEN_SOURCE_ADDR);

    lpc += DecodeFieldCStr(this -> macSchedule_delivery_time, lpc, AI_SMMC_MAXLEN_SCHEDULE_TIME);
    lpc += DecodeFieldCStr(this -> macValidity_period, lpc, AI_SMMC_MAXLEN_VALIDITY_PERIOD);
    lpc += DecodeField(&this -> mu8Registered_delivery, lpc);
    lpc += DecodeField(&this -> mu8SM_default_msg_id, lpc);
    lpc += DecodeField(&this -> mu8SM_length, lpc);
    lpc += DecodeFieldStr(this -> macShort_message, lpc, this -> mu8SM_length);

    return lpc - (const char *)apPtr;
}

apl_ssize_t CCancelSMPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macService_type, lpc, AI_SMMC_MAXLEN_SERVICE_TYPE);
    lpc += DecodeFieldCStr(this -> macMessage_id, lpc, AI_SMMC_MAXLEN_MESSAGE_ID);
    lpc += DecodeField(&this -> mu8Source_addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Source_addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macSource_addr, lpc, AI_SMMC_MAXLEN_SOURCE_ADDR);
    lpc += DecodeField(&this -> mu8Dest_addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Dest_addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macDestination_addr, lpc, AI_SMMC_MAXLEN_DEST_ADDR);

    return lpc - (const char *)apPtr;
    
}

apl_ssize_t CSubmitMultiPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macService_type, lpc, AI_SMMC_MAXLEN_SERVICE_TYPE);
    lpc += DecodeField(&this -> mu8Source_addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Source_addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macSource_addr, lpc, AI_SMMC_MAXLEN_SOURCE_ADDR);
    
    lpc += DecodeField(&this -> mu8Number_of_dests, lpc);

    for( apl_uint32_t liIndex = 0; liIndex < this -> mu8Number_of_dests; ++liIndex)
    {
        CAddress loAddress;

        lpc += DecodeField(&loAddress.mu8Dest_flag, lpc);
        if(loAddress.mu8Dest_flag == AI_SMMC_MULTI_ADDR_SMEADDR)
        {
            lpc += DecodeField(&loAddress.moSME_dest_address.mu8Dest_addr_ton, lpc);
            lpc += DecodeField(&loAddress.moSME_dest_address.mu8Dest_addr_npi, lpc);
            lpc += DecodeFieldCStr(loAddress.moSME_dest_address.macDestination_addr, lpc, AI_SMMC_MAXLEN_DEST_ADDR);
        }
        else // AI_SMMC_MULTI_ADDR_DLADDR
        {
            lpc += DecodeFieldCStr(loAddress.macDl_name, lpc, AI_SMMC_MAXLEN_DEST_ADDR);
        }
        this -> moDest_address.push_back(loAddress);
    }

    lpc += DecodeField(&this -> mu8Esm_class, lpc);
    lpc += DecodeField(&this -> mu8Protocol_id, lpc);
    lpc += DecodeField(&this -> mu8Priority_flag, lpc);
    lpc += DecodeFieldCStr(this -> macSchedule_delivery_time, lpc, AI_SMMC_MAXLEN_SCHEDULE_TIME);
    lpc += DecodeFieldCStr(this -> macValidity_period, lpc, AI_SMMC_MAXLEN_VALIDITY_PERIOD);
    lpc += DecodeField(&this -> mu8Registered_delivery, lpc);
    lpc += DecodeField(&this -> mu8Replace_if_present_flag, lpc);
    lpc += DecodeField(&this -> mu8Data_coding, lpc);
    lpc += DecodeField(&this -> mu8SM_default_msg_id, lpc);
    lpc += DecodeField(&this -> mu8SM_length, lpc);
    lpc += DecodeFieldStr(this -> macShort_message, lpc, this -> mu8SM_length);

    return lpc - (const char *)apPtr;
}

apl_ssize_t CSubmitMultiRespPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;

    lpc += DecodeFieldCStr(this -> macMessage_id, lpc, AI_SMMC_MAXLEN_MESSAGE_ID);
    lpc += DecodeField(&this -> mu8No_unsuccess, lpc);

    for(apl_uint32_t liIndex = 0; liIndex < this -> mu8No_unsuccess; ++liIndex)
    {
        CSMEUnsuccess loUnsuccess;

        lpc += DecodeField(&loUnsuccess.mu8Dest_addr_ton, lpc);
        lpc += DecodeField(&loUnsuccess.mu8Dest_addr_npi, lpc);
        lpc += DecodeFieldCStr(loUnsuccess.macDestination_addr, lpc, AI_SMMC_MAXLEN_DEST_ADDR);
        lpc += DecodeField(&loUnsuccess.mu32Error_status_code, lpc);

        this -> moUnsuccess_sme.push_back(loUnsuccess);
    }

    return lpc - (const char *)apPtr;
}

apl_ssize_t CAlertNotificationPDU::Decode(const void* apPtr, apl_size_t aiLen, const CSMMCHeader& aoHeader)
{
    const char* lpc = (const char *)apPtr;

    lpc += DecodeField(&this -> mu8Source_addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Source_addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macSource_addr, lpc, AI_SMMC_MAXLEN_SOURCE_ADDR);

    lpc += DecodeField(&this -> mu8Esme_addr_ton, lpc);
    lpc += DecodeField(&this -> mu8Esme_addr_npi, lpc);
    lpc += DecodeFieldCStr(this -> macEsme_addr, lpc, AI_SMMC_MAXLEN_ESME_ADDR);

    return lpc - (const char *)apPtr;
    
}

apl_ssize_t CSMMCMessage::Decode(const void* apPtr, apl_size_t aiLen)
{
    const char* lpc = (const char *)apPtr;
    apl_int_t liRet = 0;
    apl_size_t luRemainLen;

    liRet = this -> moHeader.Decode(apPtr, aiLen);

    if( liRet < 0 )
    {
        this -> miSMMCErrno = AI_SMMC_ERROR_LEN_MESSAGE;
        return -1;
    }
    
    if( this -> GetLength() > aiLen )
    {
        this -> miSMMCErrno = AI_SMMC_ERROR_LEN_MESSAGE;
        return -1;
    }
    
    luRemainLen = this -> GetLength();


    lpc += liRet;
    luRemainLen -= liRet;

    if( mpoPDU != APL_NULL )
        ACL_DELETE(mpoPDU);

    switch( this -> moHeader.mu32Command_id )
    {
    case AI_SMMC_CMD_BIND_RECEIVER:
    case AI_SMMC_CMD_BIND_TRANSMITTER:
    case AI_SMMC_CMD_BIND_TRANSCEIVER:
        ACL_NEW_ASSERT(this->mpoPDU, CBindPDU);
        break;
    case AI_SMMC_CMD_BIND_RECEIVER_RESP:
    case AI_SMMC_CMD_BIND_TRANSMITTER_RESP:
    case AI_SMMC_CMD_BIND_TRANSCEIVER_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CBindRespPDU);
        break;
    case AI_SMMC_CMD_QUERY_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CQuerySMPDU);
        break;
    case AI_SMMC_CMD_QUERY_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CQuerySMRespPDU);
        break;
    case AI_SMMC_CMD_OUTBIND:
        ACL_NEW_ASSERT(this->mpoPDU, COutBindPDU);
        break;
    case AI_SMMC_CMD_UNBIND:
        // No PDU
        break;
    case AI_SMMC_CMD_UNBIND_RESP:
        // No PDU
        break;
    case AI_SMMC_CMD_GENERIC_NACK:
        // No PDU
        break;
    case AI_SMMC_CMD_SUBMIT_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitSMPDU);
        break;
    case AI_SMMC_CMD_SUBMIT_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitSMRespPDU);
        break;
    case AI_SMMC_CMD_DELIVER_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CDeliverSMPDU);
        break;
    case AI_SMMC_CMD_DELIVER_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CDeliverSMRespPDU);
        break;
    case AI_SMMC_CMD_SUBMIT_MULTI:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitMultiPDU);
        break;
    case AI_SMMC_CMD_SUBMIT_MULTI_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CSubmitMultiRespPDU);
        break;
    case AI_SMMC_CMD_DATA_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CDataSMPDU);
        break;
    case AI_SMMC_CMD_DATA_SM_RESP:
        ACL_NEW_ASSERT(this->mpoPDU, CDataSMRespPDU);
        break;
    case AI_SMMC_CMD_REPLACE_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CReplaceSMPDU);
        break;
    case AI_SMMC_CMD_REPLACE_SM_RESP:
        // No PDU
        break;
    case AI_SMMC_CMD_CANCEL_SM:
        ACL_NEW_ASSERT(this->mpoPDU, CCancelSMPDU);
        break;
    case AI_SMMC_CMD_CANCEL_SM_RESP:
        // No PDU
        break;
    case AI_SMMC_CMD_ENQUIRE_LINK:
        // No PDU
        break;
    case AI_SMMC_CMD_ENQUIRE_LINK_RESP:
        // No PDU
        break;
    case AI_SMMC_CMD_ALERT_NOTIFICATION:
        ACL_NEW_ASSERT(this->mpoPDU, CAlertNotificationPDU);
        break;
    default:
        this -> miSMMCErrno = AI_SMMC_ERROR_INVILD_COMMAND;           
        return -1;
    }

    liRet = 0;

    if(mpoPDU != APL_NULL)
        liRet = mpoPDU -> Decode(lpc, luRemainLen, moHeader);


    if( liRet < 0 )
    {
        this -> miSMMCErrno = AI_SMMC_ERROR_LEN_MESSAGE;
        return -1;
    }

    lpc += liRet;
    luRemainLen -= liRet;

    //printf("Before TLV: %d\n", luRemainLen);
    liRet = this -> DecodeTLV(lpc, luRemainLen, moHeader);

    if( liRet < 0 )
    {
        this -> miSMMCErrno = AI_SMMC_ERROR_LEN_MESSAGE;
        return -1;
    }

    lpc += liRet;
    luRemainLen -= liRet;

    return lpc - (char *)apPtr;
}


apl_ssize_t CSMMCMessage::DecodeTLV(void const* apPtr, apl_size_t aiLen, const CSMMCHeader& Header)
{
    apl_int32_t luRemainLen = aiLen;
    
    const char* lpc = (const char *)apPtr;
    const char* lpcOld = lpc;

    if( luRemainLen == 0 )
        return 0;

    if( luRemainLen < 4 )
        return -1;

    while( luRemainLen > 4 )
    {
        CTLVItem loItem;
        
        lpc += DecodeField(&loItem.mu16Tag, lpc);
        lpc += DecodeField(&loItem.mu16Len, lpc);
        
        loItem.moValue = std::vector<apl_uint8_t>(lpc, lpc + loItem.mu16Len);
        lpc += loItem.mu16Len;

        moTLVTable.push_back(loItem);
        luRemainLen -= lpc - lpcOld;
        lpcOld = lpc;
    }

    if( luRemainLen != 0)
        return -1;

    return lpc - (const char *)apPtr;
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

apl_ssize_t CSMMCHeader::Encode(void* apPtr, apl_size_t aiLen) const
{
    if( aiLen < AI_SMMC_HEADER_LEN )
        return -1;

    apl_uint32_t* lpu = (apl_uint32_t *)apPtr;

    *lpu++ = apl_hton32(this -> mu32Command_length);
    *lpu++ = apl_hton32(this -> mu32Command_id);
    *lpu++ = apl_hton32(this -> mu32Command_status);
    *lpu++ = apl_hton32(this -> mu32Sequence_number);

    return (char *)lpu - (char *)apPtr;
}

apl_ssize_t CBindPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macSystem_id, AI_SMMC_MAXLEN_SYSTEM_ID);
    lpc += EncodeFieldCStr(lpc, this -> macPassword, AI_SMMC_MAXLEN_PASSWORD);
    lpc += EncodeFieldCStr(lpc, this -> macSystem_type, AI_SMMC_MAXLEN_SYSTEM_TYPE);
    lpc += EncodeField(lpc, this -> mu8Interface_version);  
    lpc += EncodeField(lpc, this -> mu8Addr_ton);
    lpc += EncodeField(lpc, this -> mu8Addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macAddress_range, AI_SMMC_MAXLEN_ADDRESS_RANGE);

    return lpc - (char *)apPtr;
}

apl_ssize_t CBindRespPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macSystem_id, AI_SMMC_MAXLEN_SYSTEM_ID);

    return lpc - (char *)apPtr;
}

apl_ssize_t COutBindPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{ 
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macSystem_id, AI_SMMC_MAXLEN_SYSTEM_ID);
    lpc += EncodeFieldCStr(lpc, this -> macPassword, AI_SMMC_MAXLEN_PASSWORD);

    return lpc - (char *)apPtr;
}

apl_ssize_t CSubmitSMPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{ 
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macService_type, AI_SMMC_MAXLEN_SERVICE_TYPE);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMMC_MAXLEN_SOURCE_ADDR);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macDestination_addr, AI_SMMC_MAXLEN_DEST_ADDR);
    lpc += EncodeField(lpc, this -> mu8Esm_class);
    lpc += EncodeField(lpc, this -> mu8Protocol_id);
    lpc += EncodeField(lpc, this -> mu8Priority_flag);
    lpc += EncodeFieldCStr(lpc, this -> macSchedule_delivery_time, AI_SMMC_MAXLEN_SCHEDULE_TIME);
    lpc += EncodeFieldCStr(lpc, this -> macValidity_period, AI_SMMC_MAXLEN_VALIDITY_PERIOD);
    lpc += EncodeField(lpc, this -> mu8Registered_delivery);
    lpc += EncodeField(lpc, this -> mu8Replace_if_present_flag);
    lpc += EncodeField(lpc, this -> mu8Data_coding);
    lpc += EncodeField(lpc, this -> mu8SM_default_msg_id);
    lpc += EncodeField(lpc, this -> mu8SM_length);
    lpc += EncodeFieldStr(lpc, this -> macShort_message, this -> mu8SM_length);

    return lpc - (char *)apPtr;
}

apl_ssize_t CSubmitSMRespPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{ 
    return 0;
}

apl_ssize_t CDeliverSMPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{ 
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macService_type, AI_SMMC_MAXLEN_SERVICE_TYPE);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMMC_MAXLEN_SOURCE_ADDR);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_npi);
    lpc += EncodeFieldCStr(lpc , this -> macDestination_addr, AI_SMMC_MAXLEN_DEST_ADDR);
    lpc += EncodeField(lpc, this -> mu8Esm_class);
    lpc += EncodeField(lpc, this -> mu8Protocol_id);
    lpc += EncodeField(lpc, this -> mu8Priority_flag);
    lpc += EncodeFieldCStr(lpc, this -> macSchedule_delivery_time, AI_SMMC_MAXLEN_SCHEDULE_TIME);
    lpc += EncodeFieldCStr(lpc, this -> macValidity_period, AI_SMMC_MAXLEN_VALIDITY_PERIOD);
    lpc += EncodeField(lpc, this -> mu8Registered_delivery);
    lpc += EncodeField(lpc, this -> mu8Replace_if_present_flag);
    lpc += EncodeField(lpc, this -> mu8Data_coding);
    lpc += EncodeField(lpc, this -> mu8SM_default_msg_id);
    lpc += EncodeField(lpc, this -> mu8SM_length);
    lpc += EncodeFieldStr(lpc, this -> macShort_message, this -> mu8SM_length);

    return lpc - (char *)apPtr;
}

apl_ssize_t CDeliverSMRespPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const 
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMMC_MAXLEN_MESSAGE_ID);

    return lpc - (char *)apPtr;
}

apl_ssize_t CDataSMPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{ 
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macService_type, AI_SMMC_MAXLEN_SERVICE_TYPE);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMMC_MAXLEN_SOURCE_ADDR);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_npi);
    lpc += EncodeFieldCStr(lpc , this -> macDestination_addr, AI_SMMC_MAXLEN_DEST_ADDR);
    lpc += EncodeField(lpc, this -> mu8Esm_class);
    lpc += EncodeField(lpc, this -> mu8Registered_delivery);
    lpc += EncodeField(lpc, this -> mu8Data_coding);

    return lpc - (char *)apPtr;
}

apl_ssize_t CDataSMRespPDU::Encode(void * const apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const 
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMMC_MAXLEN_MESSAGE_ID);

    return lpc - (char *)apPtr;
}


apl_ssize_t CQuerySMPDU::Encode(void* const apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMMC_MAXLEN_MESSAGE_ID);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMMC_MAXLEN_SOURCE_ADDR);

    return lpc - (char *)apPtr;
}


apl_ssize_t CQuerySMRespPDU::Encode(void* const apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMMC_MAXLEN_MESSAGE_ID);
    lpc += EncodeFieldCStr(lpc, this -> macFinal_date, AI_SMMC_MAXLEN_FINAL_DATE);
    lpc += EncodeField(lpc, this -> mu8Message_state);
    lpc += EncodeField(lpc, this -> mu8Error_code);

    return lpc - (char *)apPtr;
}

apl_ssize_t CReplaceSMPDU::Encode(void* const apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMMC_MAXLEN_MESSAGE_ID);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMMC_MAXLEN_SOURCE_ADDR);

    lpc += EncodeFieldCStr(lpc, this -> macSchedule_delivery_time, AI_SMMC_MAXLEN_SCHEDULE_TIME);
    lpc += EncodeFieldCStr(lpc, this -> macValidity_period, AI_SMMC_MAXLEN_VALIDITY_PERIOD);
    lpc += EncodeField(lpc, this -> mu8Registered_delivery);
    lpc += EncodeField(lpc, this -> mu8SM_default_msg_id);
    lpc += EncodeField(lpc, this -> mu8SM_length);
    lpc += EncodeFieldStr(lpc, this -> macShort_message, this -> mu8SM_length);

    return lpc - (char *)apPtr;
}

apl_ssize_t CCancelSMPDU::Encode(void* const apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macService_type, AI_SMMC_MAXLEN_SERVICE_TYPE);
    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMMC_MAXLEN_MESSAGE_ID);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMMC_MAXLEN_SOURCE_ADDR);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Dest_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macDestination_addr, AI_SMMC_MAXLEN_DEST_ADDR);

    return lpc - (char *)apPtr;
}

apl_ssize_t CSubmitMultiPDU::Encode(void* apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macService_type, AI_SMMC_MAXLEN_SERVICE_TYPE);
    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMMC_MAXLEN_SOURCE_ADDR);
    
    lpc += EncodeField(lpc, this -> mu8Number_of_dests);

    for( apl_uint32_t liIndex = 0; liIndex < this -> mu8Number_of_dests; ++liIndex)
    {
        lpc += EncodeField(lpc, moDest_address[liIndex].mu8Dest_flag);
        if(moDest_address[liIndex].mu8Dest_flag == AI_SMMC_MULTI_ADDR_SMEADDR)
        {
            lpc += EncodeField(lpc, moDest_address[liIndex].moSME_dest_address.mu8Dest_addr_ton);
            lpc += EncodeField(lpc, moDest_address[liIndex].moSME_dest_address.mu8Dest_addr_npi);
            lpc += EncodeFieldCStr(lpc, moDest_address[liIndex].moSME_dest_address.macDestination_addr, AI_SMMC_MAXLEN_DEST_ADDR);
        }
        else // AI_SMMC_MULTI_ADDR_DLADDR
        {
            lpc += EncodeFieldCStr(lpc, moDest_address[liIndex].macDl_name, AI_SMMC_MAXLEN_DEST_ADDR);
        }
    }

    lpc += EncodeField(lpc, this -> mu8Esm_class);
    lpc += EncodeField(lpc, this -> mu8Protocol_id);
    lpc += EncodeField(lpc, this -> mu8Priority_flag);
    lpc += EncodeFieldCStr(lpc, this -> macSchedule_delivery_time, AI_SMMC_MAXLEN_SCHEDULE_TIME);
    lpc += EncodeFieldCStr(lpc, this -> macValidity_period, AI_SMMC_MAXLEN_VALIDITY_PERIOD);
    lpc += EncodeField(lpc, this -> mu8Registered_delivery);
    lpc += EncodeField(lpc, this -> mu8Replace_if_present_flag);
    lpc += EncodeField(lpc, this -> mu8Data_coding);
    lpc += EncodeField(lpc, this -> mu8SM_default_msg_id);
    lpc += EncodeField(lpc, this -> mu8SM_length);
    lpc += EncodeFieldStr(lpc, this -> macShort_message, this -> mu8SM_length);

    return lpc - (char *)apPtr;
}

apl_ssize_t CSubmitMultiRespPDU::Encode(void* apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeFieldCStr(lpc, this -> macMessage_id, AI_SMMC_MAXLEN_MESSAGE_ID);
    lpc += EncodeField(lpc, this -> mu8No_unsuccess);

    for(apl_uint32_t liIndex = 0; liIndex < this -> mu8No_unsuccess; ++liIndex)
    {
        lpc += EncodeField(lpc, moUnsuccess_sme[liIndex].mu8Dest_addr_ton);
        lpc += EncodeField(lpc, moUnsuccess_sme[liIndex].mu8Dest_addr_npi);
        lpc += EncodeFieldCStr(lpc, moUnsuccess_sme[liIndex].macDestination_addr, AI_SMMC_MAXLEN_DEST_ADDR);
        lpc += EncodeField(lpc, moUnsuccess_sme[liIndex].mu32Error_status_code);
    }

    return lpc - (char *)apPtr;
}

apl_ssize_t CAlertNotificationPDU::Encode(void* apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
{
    char* lpc = (char *)apPtr;

    lpc += EncodeField(lpc, this -> mu8Source_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Source_addr_npi);
    lpc += EncodeFieldCStr(lpc, this -> macSource_addr, AI_SMMC_MAXLEN_SOURCE_ADDR);

    lpc += EncodeField(lpc, this -> mu8Esme_addr_ton);
    lpc += EncodeField(lpc, this -> mu8Esme_addr_npi);
    lpc += EncodeFieldCStr(lpc , this -> macEsme_addr, AI_SMMC_MAXLEN_ESME_ADDR);

    return lpc - (char *)apPtr;
}


apl_ssize_t CSMMCMessage::Encode(void* apPtr, apl_size_t aiLen)
{
    char* lpc = (char *)apPtr;
    apl_int_t liRet = 0;
    apl_size_t luRemainLen = aiLen;

    liRet = this -> moHeader.Encode(apPtr, luRemainLen);

    if( liRet < 0 )
    {
        this -> miSMMCErrno = AI_SMMC_ERROR_LEN_MESSAGE;
        return -1;
    }

    lpc += liRet;
    luRemainLen -= liRet;

    if( this -> GetLength() > aiLen )
    {
        this -> miSMMCErrno = AI_SMMC_ERROR_LEN_MESSAGE;
        return -1;
    }

    liRet = 0;

    if(mpoPDU != NULL)
        liRet = mpoPDU -> Encode(lpc, luRemainLen, moHeader);

    
    if( liRet < 0 )
    {
        this -> miSMMCErrno = AI_SMMC_ERROR_LEN_MESSAGE;
        return -1;
    }

    lpc += liRet;
    luRemainLen -= liRet;

    liRet = this -> EncodeTLV(lpc, luRemainLen, moHeader);

    if( liRet < 0 )
    {
        this -> miSMMCErrno = AI_SMMC_ERROR_LEN_MESSAGE;
        return -1;
    }

    lpc += liRet;
    luRemainLen -= liRet;

    // Encode Header again to set the mu32Command_length field
    this -> moHeader.mu32Command_length = lpc - (char *) apPtr;
    moHeader.Encode(apPtr, aiLen);

    return lpc - (char *)apPtr;
}

apl_ssize_t CSMMCMessage::EncodeTLV(void* apPtr, apl_size_t aiLen, CSMMCHeader& aoHeader) const
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


} // namespace smmc

