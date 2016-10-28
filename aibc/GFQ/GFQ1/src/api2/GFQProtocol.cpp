
#include "gfq/GFQProtocol.h"
#include "acl/MemoryStream.h"
#include "apl/sys.h"
// start namespace
AIBC_GFQ_NAMESPACE_START

#define STREAM_RETURN_IF( ret, opt, val ) \
    APL_RETURN_IF(ret, (opt(val, sizeof(val) ) != (apl_ssize_t)sizeof(val) ) ) 

#define STREAM_N_RETURN_IF( ret, opt, val ) \
    APL_RETURN_IF(ret, (opt(val) != (apl_ssize_t)sizeof(val) ) ) 

//------------------------CHead------------------------//
CMessageHeader::CMessageHeader(void) 
    : miCmd(0)
    , mpoBody(NULL)
{
}
CMessageHeader::CMessageHeader( apl_int_t aiCmd, CMessageBody* apoBody )
    : miCmd(aiCmd)
    , mpoBody(apoBody)
{
}

CMessageHeader::~CMessageHeader(void)
{
}

apl_int_t CMessageHeader::Decode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miCmd);

	if (this->mpoBody != NULL)
	{
		APL_RETURN_IF(-1, (this->mpoBody->Decode(aoBuffer) != 0 ) );
	}

	return 0;
}

apl_int_t CMessageHeader::Encode(acl::CMemoryBlock& aoBuffer)
{
    aoBuffer.Resize(this->GetSize() );
    
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miCmd);
	
    if (this->mpoBody != NULL)
	{
		APL_RETURN_IF(-1, (this->mpoBody->Encode(aoBuffer) != 0) );
	}

	return 0;
}

apl_size_t CMessageHeader::GetSize(void)
{
	apl_size_t luBodySize = 0;

    if (this->mpoBody != NULL )
	{
		luBodySize = this->mpoBody->GetSize();
	}

	return sizeof(this->miCmd) + luBodySize;
}

apl_int_t CMessageHeader::GetCmd(void)
{
	return this->miCmd;
}

//------------------------CUserRequest------------------------//
CUserRequest::CUserRequest(void)
{
    apl_memset(this->macUserName, 0, sizeof(this->macUserName) );
}

CUserRequest::~CUserRequest(void)
{
}

apl_int_t CUserRequest::Decode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_RETURN_IF(-1, loIn.Read, this->macUserName);
	
	return 0;
}

apl_int_t CUserRequest::Encode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_RETURN_IF(-1, loOut.Write, this->macUserName);
	
	return 0;
}

void CUserRequest::SetUserName( const char* apcUserName )
{
	apl_strncpy(this->macUserName, apcUserName, sizeof(this->macUserName) );
}

const char* CUserRequest::GetUserName(void)
{
	return this->macUserName;
}

apl_size_t CUserRequest::GetSize(void)
{
	return sizeof(*this);
}

//----------------------------------CResponse--------------------------------//
CResponse::CResponse(void)
    : miStat(-1)
{
}

CResponse::~CResponse(void)
{
}

apl_int_t CResponse::Decode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miStat);
	
	return 0;
}

apl_int_t CResponse::Encode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miStat);
	
	return 0;
}

void CResponse::SetStat(apl_int_t aiStat)
{
	this->miStat = aiStat;
}

apl_int_t CResponse::GetStat(void)
{
	return this->miStat;
}

apl_size_t CResponse::GetSize(void)
{
	return sizeof(this->miStat);
}

//-------------------------------CRequestAddModule--------------------------------//
CRequestAddModule::CRequestAddModule(void)
    : miPort(-1)
{
    apl_memset(this->macModuleName, 0, sizeof(this->macModuleName) );
    apl_memset(this->macIpAddr, 0, sizeof(this->macIpAddr) );
}

CRequestAddModule::~CRequestAddModule(void)
{
}

apl_int_t CRequestAddModule::Decode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_RETURN_IF(-1, loIn.Read, this->macModuleName);
	STREAM_RETURN_IF(-1, loIn.Read, this->macIpAddr);
	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miPort);
	
	return 0;
}

apl_int_t CRequestAddModule::Encode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_RETURN_IF(-1,   loOut.Write, this->macModuleName);
	STREAM_RETURN_IF(-1,   loOut.Write, this->macIpAddr);
	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miPort);
	
	return 0;
}

void CRequestAddModule::SetModuleName(const char* apcModuleName)
{
	apl_strncpy(this->macModuleName, apcModuleName, sizeof(this->macModuleName) );
}

const char* CRequestAddModule::GetModuleName(void)
{
	return this->macModuleName;
}

void CRequestAddModule::SetIpAddr(const char* apcIpAddr)
{
	apl_strncpy(this->macIpAddr, apcIpAddr, IP_ADDR_LEN );
}

const char* CRequestAddModule::GetIpAddr(void)
{
	return this->macIpAddr;
}

void CRequestAddModule::SetPort(apl_int_t aiPort)
{
	this->miPort = aiPort;
}

apl_int_t CRequestAddModule::GetPort(void)
{
	return this->miPort;
}

apl_size_t CRequestAddModule::GetSize(void)
{
	return sizeof(*this);
}

//-------------------------------CRequestDelModule--------------------------------//
CRequestDelModule::CRequestDelModule(void)
{
    apl_memset(this->macModuleName, 0, sizeof(this->macModuleName) );
}

CRequestDelModule::~CRequestDelModule(void)
{
}

apl_int_t CRequestDelModule::Decode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loIn(&aoBuffer);

    STREAM_RETURN_IF(-1, loIn.Read, this->macModuleName);
	
	return 0;
}

apl_int_t CRequestDelModule::Encode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loOut(&aoBuffer);

    STREAM_RETURN_IF(-1, loOut.Write, this->macModuleName);
	
	return 0;
}

void CRequestDelModule::SetModuleName(const char* apcModuleName)
{
	apl_strncpy(this->macModuleName, apcModuleName, sizeof(macModuleName) );
}

const char* CRequestDelModule::GetModuleName(void)
{
	return this->macModuleName;
}

apl_size_t CRequestDelModule::GetSize(void)
{
	return sizeof(*this);
}

//--------------------------------CRequestAddSeg---------------------------------//
CRequestAddSeg::CRequestAddSeg(void)
{
}

CRequestAddSeg::~CRequestAddSeg(void)
{
}

apl_int_t CRequestAddSeg::Decode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loIn(&aoBuffer);

    STREAM_RETURN_IF(-1, loIn.Read, this->macModuleName);
    STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miSegmentID);
    STREAM_RETURN_IF(-1, loIn.Read, this->macQueueName);
    STREAM_RETURN_IF(-1, loIn.Read, this->macQueueGlobalID);
    STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miSegmentSize);
    STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miSegmentCapacity);
	
	return 0;
}

apl_int_t CRequestAddSeg::Encode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loOut(&aoBuffer);

    STREAM_RETURN_IF(-1, loOut.Write, this->macModuleName);
    STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miSegmentID);
    STREAM_RETURN_IF(-1, loOut.Write, this->macQueueName);
    STREAM_RETURN_IF(-1, loOut.Write, this->macQueueGlobalID);
    STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miSegmentSize);
    STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miSegmentCapacity);
	
	return 0;
}

void CRequestAddSeg::SetModuleName(const char* apcModuleName)
{
	apl_strncpy(this->macModuleName, apcModuleName, sizeof(this->macModuleName) );
}

const char* CRequestAddSeg::GetModuleName(void)
{
	return this->macModuleName;
}

void CRequestAddSeg::SetSegmentID(apl_int_t aiSegmentID)
{
	this->miSegmentID = aiSegmentID;
}

apl_int_t CRequestAddSeg::GetSegmentID(void)
{
	return this->miSegmentID;
}

void CRequestAddSeg::SetQueueName(const char* apcQueueName)
{
	apl_strncpy(this->macQueueName, apcQueueName, sizeof(this->macQueueName) );
}

const char* CRequestAddSeg::GetQueueName(void)
{
	return this->macQueueName;
}

void CRequestAddSeg::SetQueueGlobalID(apl_int64_t aiQueueGlobalID)
{
    apl_snprintf(this->macQueueGlobalID, sizeof(this->macQueueGlobalID), "%"APL_PRId64, aiQueueGlobalID );
}

apl_int64_t CRequestAddSeg::GetQueueGlobalID(void)
{
	return apl_strtoi64(this->macQueueGlobalID, NULL, 10);
}

void CRequestAddSeg::SetSegmentSize( apl_int_t aiSize )
{
	this->miSegmentSize = aiSize;
}

apl_int_t CRequestAddSeg::GetSegmentSize()
{
	return this->miSegmentSize;
}
	
void CRequestAddSeg::SetSegmentCapacity( apl_int_t aiCapacity )
{
	this->miSegmentCapacity = aiCapacity;
}

apl_int_t CRequestAddSeg::GetSegmentCapacity(void)
{
	return this->miSegmentCapacity;
}
	
apl_size_t CRequestAddSeg::GetSize(void)
{
	return sizeof(*this);
}

//------------------------------CRequestWriteSeg------------------------------------//
CRequestWriteSeg::CRequestWriteSeg(void)
{
}

CRequestWriteSeg::~CRequestWriteSeg(void)
{
}

apl_int_t CRequestWriteSeg::Decode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, (CUserRequest::Decode(aoBuffer) != 0) );
    
	acl::CMemoryStream loIn(&aoBuffer);

    STREAM_RETURN_IF( -1, loIn.Read, this->macQueueName);
	
    return 0;
}

apl_int_t CRequestWriteSeg::Encode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, (CUserRequest::Encode(aoBuffer) != 0) );
        
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_RETURN_IF(-1, loOut.Write, this->macQueueName);

	return 0;
}

void CRequestWriteSeg::SetQueueName(const char* apcQueueName)
{
	apl_strncpy(this->macQueueName, apcQueueName, MAX_NAME_LEN );
}

const char* CRequestWriteSeg::GetQueueName(void)
{
	return this->macQueueName;
}

apl_size_t CRequestWriteSeg::GetSize(void)
{
	return sizeof(*this);
}

//-----------------------------------CRequestWriteSegSecond-----------------------------------//
CRequestWriteSegSecond::CRequestWriteSegSecond(void) 
{
}

CRequestWriteSegSecond::~CRequestWriteSegSecond(void) 
{
}

apl_int_t CRequestWriteSegSecond::Decode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, (CUserRequest::Decode(aoBuffer) != 0) );
        
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_RETURN_IF(-1, loIn.Read, this->macQueueName);
	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miSegmentID);
	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miModuleID);
	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miRetcode);
	
    return 0;
}

apl_int_t CRequestWriteSegSecond::Encode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, (CUserRequest::Encode(aoBuffer) != 0) );
        
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_RETURN_IF(-1,   loOut.Write, this->macQueueName);
	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miSegmentID);
	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miModuleID);
	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miRetcode);
	
	return 0;
}

void CRequestWriteSegSecond::SetQueueName(const char* apcQueueName)
{
	apl_strncpy(this->macQueueName, apcQueueName, MAX_NAME_LEN);
}

const char* CRequestWriteSegSecond::GetQueueName(void)
{
	return this->macQueueName;
}

void CRequestWriteSegSecond::SetSegmentID(apl_int_t aiSegmentID)
{
	this->miSegmentID = aiSegmentID;
}

apl_int_t CRequestWriteSegSecond::GetSegmentID(void)
{
	return this->miSegmentID;
}

void CRequestWriteSegSecond::SetModuleID(apl_int_t aiModuleID)
{
	this->miModuleID = aiModuleID;
}

apl_int_t CRequestWriteSegSecond::GetModuleID()
{
	return this->miModuleID;
}

void CRequestWriteSegSecond::SetRetcode(apl_int_t aiRetcode)
{
	this->miRetcode = aiRetcode;
}

apl_int_t CRequestWriteSegSecond::GetRetcode(void)
{
	return this->miRetcode;
}

apl_size_t CRequestWriteSegSecond::GetSize(void)
{
	return sizeof(*this);
}

//------------------------------CRequestQueueStat------------------------------------//
CRequestQueueStat::CRequestQueueStat(void)
{
}

CRequestQueueStat::~CRequestQueueStat(void)
{
}

apl_int_t CRequestQueueStat::Decode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, (CUserRequest::Decode(aoBuffer) != 0) );
        
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_RETURN_IF(-1, loIn.Read, this->macQueueName);
	
    return 0;
}

apl_int_t CRequestQueueStat::Encode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, (CUserRequest::Encode(aoBuffer) != 0) );
        
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_RETURN_IF(-1, loOut.Write, this->macQueueName);
	
    return 0;
}

void CRequestQueueStat::SetQueueName(const char* apcQueueName)
{
	apl_strncpy(this->macQueueName, apcQueueName, MAX_NAME_LEN );
}

const char* CRequestQueueStat::GetQueueName(void)
{
	return this->macQueueName;
}

apl_size_t CRequestQueueStat::GetSize(void)
{
	return sizeof(*this);
}

//------------------------------CRequestCommad------------------------------------//
CRequestCommad::CRequestCommad(void)
{
}

CRequestCommad::~CRequestCommad(void)
{
}

apl_int_t CRequestCommad::Decode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, (CUserRequest::Decode(aoBuffer) != 0) );
	return 0;
}

apl_int_t CRequestCommad::Encode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, (CUserRequest::Encode(aoBuffer) != 0) );
	return 0;
}

apl_size_t CRequestCommad::GetSize()
{
	return sizeof(*this);
}

//------------------------CRequestAlloc------------------------//
CRequestAlloc::CRequestAlloc(void) 
{
}

CRequestAlloc::~CRequestAlloc(void) 
{
}

apl_int_t CRequestAlloc::Decode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miSegmentID);
	STREAM_RETURN_IF(-1, loIn.Read, this->macQueueName);
	STREAM_RETURN_IF(-1, loIn.Read, this->macQueueGlobalID);
	
	return 0;
}

apl_int_t CRequestAlloc::Encode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miSegmentID);
	STREAM_RETURN_IF(-1, loOut.Write, this->macQueueName);
	STREAM_RETURN_IF(-1, loOut.Write, this->macQueueGlobalID);
	
	return 0;
}

void CRequestAlloc::SetSegmentID(apl_int_t aiSegmentID)
{
	this->miSegmentID = aiSegmentID;
}

apl_int_t CRequestAlloc::GetSegmentID(void)
{
	return this->miSegmentID;
}

void CRequestAlloc::SetQueueName(const char* apcQueueName)
{
	apl_strncpy(this->macQueueName, apcQueueName, MAX_NAME_LEN);
}

const char* CRequestAlloc::GetQueueName(void)
{
	return this->macQueueName;
}

void CRequestAlloc::SetQueueGlobalID(apl_int64_t aiQueueGlobalID)
{
    apl_snprintf(this->macQueueGlobalID, sizeof(this->macQueueGlobalID), "%"APL_PRId64, aiQueueGlobalID );
}

apl_int64_t CRequestAlloc::GetQueueGlobalID(void)
{
	return apl_strtoi64(this->macQueueGlobalID, NULL, 10);
}

apl_size_t CRequestAlloc::GetSize(void)
{
	return sizeof(*this);
}

//------------------------CRequestFree------------------------//
CRequestFree::CRequestFree(void) 
{
}

CRequestFree::~CRequestFree(void)
{
}

apl_int_t CRequestFree::Decode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miSegmentID);
	
	return 0;
}

apl_int_t CRequestFree::Encode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miSegmentID);
	
	return 0;
}

void CRequestFree::SetSegmentID(apl_int_t aiSegmentID)
{
	this->miSegmentID = aiSegmentID;
}

apl_int_t CRequestFree::GetSegmentID(void)
{
	return this->miSegmentID;
}

apl_size_t CRequestFree::GetSize(void)
{
	return sizeof(*this);
}

//------------------------CRequestSegmentSize------------------------//
CRequestSegmentSize::CRequestSegmentSize(void) 
{
}

CRequestSegmentSize::~CRequestSegmentSize(void) 
{
}

apl_int_t CRequestSegmentSize::Decode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miSegmentID);
	
	return 0;
}

apl_int_t CRequestSegmentSize::Encode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miSegmentID);
	
	return 0;
}

void CRequestSegmentSize::SetSegmentID(apl_int_t aiSegmentID)
{
	this->miSegmentID = aiSegmentID;
}

apl_int_t CRequestSegmentSize::GetSegmentID(void)
{
	return this->miSegmentID;
}

apl_size_t CRequestSegmentSize::GetSize(void)
{
	return sizeof(*this);
}

//------------------------CRequestPut------------------------//
CRequestPut::CRequestPut(void) 
{
}

CRequestPut::~CRequestPut(void)
{
}

apl_int_t CRequestPut::Decode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, (CUserRequest::Decode(aoBuffer) != 0) );
        
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miSegmentID);
	STREAM_RETURN_IF(-1, loIn.Read, this->macQueueName);
	STREAM_RETURN_IF(-1, loIn.Read, this->macQueueGlobalID);
    STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miDataSize);
    if (loIn.Read(this->macData, this->miDataSize) != this->miDataSize)
    {
        return -1;
    }
	
	return 0;
}

apl_int_t CRequestPut::Encode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, (CUserRequest::Encode(aoBuffer) != 0) );
        
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miSegmentID);
	STREAM_RETURN_IF(-1,   loOut.Write, this->macQueueName);
	STREAM_RETURN_IF(-1,   loOut.Write, this->macQueueGlobalID);
    STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miDataSize);
    if (loOut.Write(this->macData, this->miDataSize) != this->miDataSize)
    {
        return -1;
    }
	
	return 0;
}

void CRequestPut::SetSegmentID(apl_int_t aiSegmentID)
{
	this->miSegmentID = aiSegmentID;
}

apl_int_t CRequestPut::GetSegmentID(void)
{
	return this->miSegmentID;
}

void CRequestPut::SetQueueName(const char* apcQueueName)
{
	apl_strncpy(this->macQueueName, apcQueueName, MAX_NAME_LEN);
}

const char* CRequestPut::GetQueueName(void)
{
	return this->macQueueName;
}

void CRequestPut::SetQueueGlobalID(apl_int64_t aiQueueGlobalID)
{
    apl_snprintf(this->macQueueGlobalID, sizeof(this->macQueueGlobalID), "%"APL_PRId64, aiQueueGlobalID );
}

apl_int64_t CRequestPut::GetQueueGlobalID(void)
{
	return apl_strtoi64(this->macQueueGlobalID, NULL, 10);
}

void CRequestPut::SetData(const acl::CMemoryBlock& aoData)
{
	this->miDataSize = aoData.GetLength();
	
	ACL_ASSERT(this->miDataSize > 0 && this->miDataSize <= (apl_int32_t)sizeof(this->macData) );
	
	apl_memcpy(this->macData, aoData.GetReadPtr(), this->miDataSize );
}

void CRequestPut::GetData(acl::CMemoryBlock& aoData)
{
	ACL_ASSERT(this->miDataSize > 0 && this->miDataSize <= (apl_int32_t)sizeof(this->macData) );
	
	aoData.Resize(this->miDataSize);
	apl_memcpy(aoData.GetWritePtr(), this->macData, this->miDataSize);
	
    aoData.SetWritePtr(this->miDataSize);
}

apl_size_t CRequestPut::GetSize(void)
{
	return sizeof(*this);
}

//------------------------CRequestGet------------------------//
CRequestGet::CRequestGet(void) 
{
}

CRequestGet::~CRequestGet(void) 
{
}

apl_int_t CRequestGet::Decode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, (CUserRequest::Decode(aoBuffer) != 0) );
        
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miSegmentID);
	STREAM_RETURN_IF(-1, loIn.Read, this->macQueueName);
	STREAM_RETURN_IF(-1, loIn.Read, this->macQueueGlobalID);
	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miWhenSec);
	
	return 0;
}

apl_int_t CRequestGet::Encode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, (CUserRequest::Encode(aoBuffer) != 0) );
        
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miSegmentID);
	STREAM_RETURN_IF(-1,   loOut.Write, this->macQueueName);
	STREAM_RETURN_IF(-1,   loOut.Write, this->macQueueGlobalID);
	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miWhenSec);
	
	return 0;
}

void CRequestGet::SetSegmentID(apl_int_t aiSegmentID)
{
	this->miSegmentID = aiSegmentID;
}

apl_int_t CRequestGet::GetSegmentID()
{
	return this->miSegmentID;
}

void CRequestGet::SetQueueName(const char* apcQueueName)
{
	apl_strncpy(this->macQueueName, apcQueueName, MAX_NAME_LEN);
}

const char* CRequestGet::GetQueueName(void)
{
	return this->macQueueName;
}

void CRequestGet::SetQueueGlobalID(apl_int64_t aiQueueGlobalID)
{
    apl_snprintf(this->macQueueGlobalID, sizeof(this->macQueueGlobalID), "%"APL_PRId64, aiQueueGlobalID );
}

apl_int64_t CRequestGet::GetQueueGlobalID(void)
{
	return apl_strtoi64(this->macQueueGlobalID, NULL, 10);
}

void CRequestGet::SetWhenSec(apl_int_t aiWhenSec)
{
	this->miWhenSec = aiWhenSec;
}

apl_int_t CRequestGet::GetWhenSec(void)
{
	return this->miWhenSec;
}

apl_size_t CRequestGet::GetSize(void)
{
	return sizeof(*this);
}

//------------------------CRequestHeartBeat------------------------//
CRequestHeartBeat::CRequestHeartBeat(void)
{
}

CRequestHeartBeat::~CRequestHeartBeat(void)
{
}

apl_int_t CRequestHeartBeat::Decode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_RETURN_IF(-1, loIn.Read, this->macModuleName);
	
	return 0;
}

apl_int_t CRequestHeartBeat::Encode(acl::CMemoryBlock& aoBuffer)
{
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_RETURN_IF(-1, loOut.Write, this->macModuleName);
	
	return 0;
}

void CRequestHeartBeat::SetModuleName(const char* apcModuleName)
{
	apl_strncpy(this->macModuleName, apcModuleName, sizeof(this->macModuleName) );
}

const char* CRequestHeartBeat::GetModuleName(void)
{
	return this->macModuleName;
}

apl_size_t CRequestHeartBeat::GetSize(void)
{
	return sizeof(*this);
}

//--------------------------------CResponseSegment---------------------------------//
CResponseSegment::CResponseSegment(void)
{
}

CResponseSegment::~CResponseSegment(void)
{
}

apl_int_t CResponseSegment::Decode(acl::CMemoryBlock& aoBuffer)
{
	APL_RETURN_IF(-1, CResponse::Decode(aoBuffer) != 0);
		
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miSegmentID);
	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miModuleID);
	STREAM_RETURN_IF(-1, loIn.Read, this->macQueueGlobalID);
	STREAM_RETURN_IF(-1, loIn.Read, this->macIpAddr);
	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miPort);
	
	return 0;
}

apl_int_t CResponseSegment::Encode(acl::CMemoryBlock& aoBuffer)
{
	CResponse::Encode(aoBuffer);
		
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miSegmentID);
	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miModuleID);
	STREAM_RETURN_IF(-1,   loOut.Write, this->macQueueGlobalID);
	STREAM_RETURN_IF(-1,   loOut.Write, this->macIpAddr);
	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miPort);
	
	return 0;
}

void CResponseSegment::SetSegmentID(apl_int_t aiSegmentID)
{
	this->miSegmentID = aiSegmentID;
}

apl_int_t CResponseSegment::GetSegmentID(void)
{
	return this->miSegmentID;
}

void CResponseSegment::SetModuleID(apl_int_t aiModuleID)
{
	this->miModuleID = aiModuleID;
}

apl_int_t CResponseSegment::GetModuleID(void)
{
	return this->miModuleID;
}

void CResponseSegment::SetQueueGlobalID(apl_int64_t aiQueueGlobalID)
{
    apl_snprintf(this->macQueueGlobalID, sizeof(this->macQueueGlobalID), "%"APL_PRId64, aiQueueGlobalID );
}

apl_int64_t CResponseSegment::GetQueueGlobalID(void)
{
	return apl_strtoi64(this->macQueueGlobalID, NULL, 10);
}

void CResponseSegment::SetIpAddr(const char* apcIpAddr)
{
	apl_strncpy(this->macIpAddr, apcIpAddr, IP_ADDR_LEN);
}

const char* CResponseSegment::GetIpAddr(void)
{
	return this->macIpAddr;
}

void CResponseSegment::SetPort(apl_int_t aiPort)
{
	this->miPort = aiPort;
}

apl_int_t CResponseSegment::GetPort(void)
{
	return this->miPort;
}

apl_size_t CResponseSegment::GetSize(void)
{
	return sizeof(*this);
}

//------------------------CResponseQueueStat------------------------//
CResponseQueueStat::CResponseQueueStat(void) 
{
}

CResponseQueueStat::~CResponseQueueStat(void)
{
}

apl_int_t CResponseQueueStat::Decode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, CResponse::Decode(aoBuffer) != 0);
		
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miSize);
	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miCapacity);
	
	return 0;
}

apl_int_t CResponseQueueStat::Encode(acl::CMemoryBlock& aoBuffer)
{
	APL_RETURN_IF(-1, CResponse::Encode(aoBuffer) != 0);
		
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miSize);
	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miCapacity);
	
	return 0;
}

void CResponseQueueStat::SetQueueSize( apl_int_t aiSize )
{
	this->miSize = aiSize;
}

apl_int_t CResponseQueueStat::GetQueueSize(void)
{
	return this->miSize;
}

void CResponseQueueStat::SetQueueCapacity( apl_int_t aiCapacity )
{
	this->miCapacity = aiCapacity;
}

apl_int_t CResponseQueueStat::GetQueueCapacity(void)
{
	return this->miCapacity;
}
	
apl_size_t CResponseQueueStat::GetSize(void)
{
	return sizeof(*this);
}

//------------------------CResponseModuleCount------------------------//
CResponseModuleCount::CResponseModuleCount(void) 
{
}

CResponseModuleCount::~CResponseModuleCount(void) 
{
}

apl_int_t CResponseModuleCount::Decode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, CResponse::Decode(aoBuffer) != 0);
		
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miCount);
	
	return 0;
}

apl_int_t CResponseModuleCount::Encode(acl::CMemoryBlock& aoBuffer)
{
	APL_RETURN_IF(-1, CResponse::Encode(aoBuffer) != 0);
		
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miCount);
	
	return 0;
}

void CResponseModuleCount::SetModuleCount( apl_int_t aiCount )
{
	this->miCount = aiCount;
}

apl_int_t CResponseModuleCount::GetModuleCount(void)
{
	return this->miCount;
}
	
apl_size_t CResponseModuleCount::GetSize(void)
{
	return sizeof(*this);
}

//------------------------CResponseSegmentSize------------------------//
CResponseSegmentSize::CResponseSegmentSize(void) 
{
}

CResponseSegmentSize::~CResponseSegmentSize(void)
{
}

apl_int_t CResponseSegmentSize::Decode(acl::CMemoryBlock& aoBuffer)
{
	APL_RETURN_IF(-1, CResponse::Decode(aoBuffer) != 0);
		
	acl::CMemoryStream loIn(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miSize);
	
	return 0;
}

apl_int_t CResponseSegmentSize::Encode(acl::CMemoryBlock& aoBuffer)
{
	APL_RETURN_IF(-1, CResponse::Encode(aoBuffer) != 0);
		
	acl::CMemoryStream loOut(&aoBuffer);

	STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miSize);
	
	return 0;
}

void CResponseSegmentSize::SetSegmentSize( apl_int_t aiSize )
{
	this->miSize = aiSize;
}

apl_int_t CResponseSegmentSize::GetSegmentSize(void)
{
	return this->miSize;
}
	
apl_size_t CResponseSegmentSize::GetSize(void)
{
	return sizeof(*this);
}

//------------------------CResponseGet------------------------//
CResponseGet::CResponseGet(void) 
{
}

CResponseGet::~CResponseGet(void) 
{
}

apl_int_t CResponseGet::Decode(acl::CMemoryBlock& aoBuffer)
{
    APL_RETURN_IF(-1, CResponse::Decode(aoBuffer) != 0);
		
	acl::CMemoryStream loIn(&aoBuffer);

    STREAM_N_RETURN_IF(-1, loIn.ReadToH, this->miDataSize);
    if (loIn.Read(this->macData, this->miDataSize) != this->miDataSize)
    {
        return -1;
    }
	
	return 0;
}

apl_int_t CResponseGet::Encode(acl::CMemoryBlock& aoBuffer)
{
	APL_RETURN_IF(-1, CResponse::Encode(aoBuffer) != 0);
		
	acl::CMemoryStream loOut(&aoBuffer);

    STREAM_N_RETURN_IF(-1, loOut.WriteToN, this->miDataSize);
    if (loOut.Write(this->macData, this->miDataSize) != this->miDataSize)
    {
        return -1;
    }

	return 0;
}

void CResponseGet::SetData(const acl::CMemoryBlock& aoData)
{
	this->miDataSize = aoData.GetLength();
	
	ACL_ASSERT(this->miDataSize > 0 && (apl_size_t)this->miDataSize <= sizeof(this->macData) );
	
	apl_memcpy(this->macData, aoData.GetReadPtr(), this->miDataSize);
}

void CResponseGet::GetData(acl::CMemoryBlock& aoData)
{ 
    ACL_ASSERT(this->miDataSize > 0 && (apl_size_t)this->miDataSize <= sizeof(this->macData) );
	
	aoData.Resize(this->miDataSize);
	apl_memcpy(aoData.GetWritePtr(), this->macData, this->miDataSize);
	aoData.SetWritePtr(this->miDataSize);
}

apl_size_t CResponseGet::GetSize(void)
{
	return sizeof(*this);
}

// end namespace
AIBC_GFQ_NAMESPACE_END

