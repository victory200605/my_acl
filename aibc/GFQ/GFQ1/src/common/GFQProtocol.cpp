
#include "gfq/GFQProtocol.h"
 
// start namespace
AI_GFQ_NAMESPACE_START

//------------------------clsHead------------------------//
clsHeader::clsHeader() : 
	ciCmd(0),cpoBody(NULL)
{
}
clsHeader::clsHeader( int32_t aiCmd, clsBody* apoBody ) : 
	ciCmd(aiCmd),cpoBody(apoBody)
{
}

clsHeader::~clsHeader()
{
}

int32_t clsHeader::Decode(AIChunkEx& aoChunk)
{
	AIChunkExIn loIn( aoChunk );
	AI_RETURN_IF( -1, ( loIn.ToHInt32( ciCmd ) != 0 ) );
	if ( cpoBody != NULL )
	{
		AI_RETURN_IF( -1, ( cpoBody->Decode( aoChunk ) != 0 ) );
	}
	return 0;
}

int32_t clsHeader::Encode(AIChunkEx& aoChunk)
{
    aoChunk.Resize( GetSize() );
    
	AIChunkExOut loOut( aoChunk );
	loOut.PutNInt32( ciCmd );
	if ( cpoBody != NULL )
	{
		AI_RETURN_IF( -1, ( cpoBody->Encode( aoChunk ) != 0 ) );
	}
	return 0;
}

int32_t clsHeader::GetSize()
{
	int32_t liBodySize = 0;
	if ( cpoBody != NULL )
	{
		liBodySize = cpoBody->GetSize();
	}
	return sizeof(int32_t) + liBodySize;
}

int32_t clsHeader::GetCmd()
{
	return ciCmd;
}

//------------------------clsUserRequest------------------------//
clsUserRequest::clsUserRequest()
{
    memset( &coBody, 0, sizeof(stBody) );
}

clsUserRequest::~clsUserRequest()
{
}

int32_t clsUserRequest::Decode(AIChunkEx& aoChunk)
{
	AIChunkExIn loIn( aoChunk );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccUserName, sizeof(coBody.ccUserName) ) != 0 ) );
	
	return 0;
}

int32_t clsUserRequest::Encode(AIChunkEx& aoChunk)
{
	AIChunkExOut loOut( aoChunk );
	loOut.PutMem( coBody.ccUserName, sizeof(coBody.ccUserName) );
	
	return 0;
}

void clsUserRequest::SetUserName( const char* apsUserName )
{
	StringCopy( coBody.ccUserName, apsUserName, sizeof(coBody.ccUserName) );
}

const char* clsUserRequest::GetUserName()
{
	return coBody.ccUserName;
}

int32_t clsUserRequest::GetSize()
{
	return sizeof(coBody);
}

//----------------------------------clsResponse--------------------------------//
clsResponse::clsResponse()
{
}

clsResponse::~clsResponse()
{
}

int32_t clsResponse::Decode(AIChunkEx& aoChunk)
{
	AIChunkExIn loIn( aoChunk );
	AI_RETURN_IF( -1, ( loIn.ToHInt32( ciStat ) != 0 ) );
	
	return 0;
}

int32_t clsResponse::Encode(AIChunkEx& aoChunk)
{
	AIChunkExOut loOut( aoChunk );
	loOut.PutNInt32( ciStat );
	
	return 0;
}

void clsResponse::SetStat(int32_t aiStat)
{
	ciStat = aiStat;
}

int32_t clsResponse::GetStat()
{
	return ciStat;
}

int32_t clsResponse::GetSize()
{
	return sizeof(int32_t);
}

//-------------------------------clsRequestAddModule--------------------------------//
clsRequestAddModule::clsRequestAddModule()
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsRequestAddModule::~clsRequestAddModule()
{
}

int32_t clsRequestAddModule::Decode(AIChunkEx& aoChunk)
{
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccModuleName, sizeof(coBody.ccModuleName) ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccIpAddr, sizeof(coBody.ccIpAddr) ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciPort ) != 0 ) );
	
	return 0;
}

int32_t clsRequestAddModule::Encode(AIChunkEx& aoChunk)
{
	AIChunkExOut loOut(aoChunk);
	loOut.PutMem( coBody.ccModuleName, sizeof(coBody.ccModuleName) );
	loOut.PutMem( coBody.ccIpAddr, sizeof(coBody.ccIpAddr) );
	loOut.PutNInt32( coBody.ciPort );
	
	return 0;
}

void clsRequestAddModule::SetModuleName(const char* apsModuleName)
{
	StringCopy( coBody.ccModuleName, apsModuleName, AI_MAX_NAME_LEN );
}

const char* clsRequestAddModule::GetModuleName()
{
	return  coBody.ccModuleName;
}

void clsRequestAddModule::SetIpAddr(const char* apsIpAddr)
{
	StringCopy( coBody.ccIpAddr, apsIpAddr, AI_IP_ADDR_LEN );
}

const char* clsRequestAddModule::GetIpAddr()
{
	return  coBody.ccIpAddr;
}

void clsRequestAddModule::SetPort(int32_t aiPort)
{
	coBody.ciPort = aiPort;
}

int32_t clsRequestAddModule::GetPort()
{
	return coBody.ciPort;
}

int32_t clsRequestAddModule::GetSize()
{
	return sizeof(stBody);
}

//-------------------------------clsRequestDelModule--------------------------------//
clsRequestDelModule::clsRequestDelModule()
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsRequestDelModule::~clsRequestDelModule()
{
}

int32_t clsRequestDelModule::Decode(AIChunkEx& aoChunk)
{
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccModuleName, sizeof(coBody.ccModuleName) ) != 0 ) );
	
	return 0;
}

int32_t clsRequestDelModule::Encode(AIChunkEx& aoChunk)
{
	AIChunkExOut loOut(aoChunk);
	loOut.PutMem( coBody.ccModuleName, sizeof(coBody.ccModuleName) );
	
	return 0;
}

void clsRequestDelModule::SetModuleName(const char* apsModuleName)
{
	StringCopy( coBody.ccModuleName, apsModuleName, sizeof(coBody.ccModuleName) );
}

const char* clsRequestDelModule::GetModuleName()
{
	return  coBody.ccModuleName;
}

int32_t clsRequestDelModule::GetSize()
{
	return sizeof(stBody);
}

//--------------------------------clsRequestAddSeg---------------------------------//
clsRequestAddSeg::clsRequestAddSeg()
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsRequestAddSeg::~clsRequestAddSeg()
{
}

int32_t clsRequestAddSeg::Decode(AIChunkEx& aoChunk)
{
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccModuleName, sizeof(coBody.ccModuleName) ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciSegmentID ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccQueueGlobalID, sizeof(coBody.ccQueueGlobalID) ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciSegmentSize ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciSegmentCapacity ) != 0 ) );
	
	return 0;
}

int32_t clsRequestAddSeg::Encode(AIChunkEx& aoChunk)
{
	AIChunkExOut loOut(aoChunk);
	loOut.PutMem( coBody.ccModuleName, sizeof(coBody.ccModuleName) );
	loOut.PutNInt32( coBody.ciSegmentID );
	loOut.PutMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) );
	loOut.PutMem( coBody.ccQueueGlobalID, sizeof(coBody.ccQueueGlobalID) );
	loOut.PutNInt32( coBody.ciSegmentSize );
	loOut.PutNInt32( coBody.ciSegmentCapacity );
	
	return 0;
}

void clsRequestAddSeg::SetModuleName(const char* apsModuleName)
{
	StringCopy( coBody.ccModuleName, apsModuleName, AI_MAX_NAME_LEN );
}

const char* clsRequestAddSeg::GetModuleName()
{
	return  coBody.ccModuleName;
}

void clsRequestAddSeg::SetSegmentID(int32_t aiSegmentID)
{
	coBody.ciSegmentID = aiSegmentID;
}

int32_t clsRequestAddSeg::GetSegmentID()
{
	return coBody.ciSegmentID;
}

void clsRequestAddSeg::SetQueueName(const char* apsQueueName)
{
	StringCopy( coBody.ccQueueName, apsQueueName, AI_MAX_NAME_LEN );
}

const char* clsRequestAddSeg::GetQueueName()
{
	return coBody.ccQueueName;
}

void clsRequestAddSeg::SetQueueGlobalID(int64_t aiQueueGlobalID)
{
	IntToString( aiQueueGlobalID, coBody.ccQueueGlobalID, AI_MAX_INT_LEN );
}

int64_t clsRequestAddSeg::GetQueueGlobalID()
{
	return atoll( coBody.ccQueueGlobalID );
}

void clsRequestAddSeg::SetSegmentSize( int32_t aiSize )
{
	coBody.ciSegmentSize = aiSize;
}

int32_t clsRequestAddSeg::GetSegmentSize()
{
	return coBody.ciSegmentSize;
}
	
void clsRequestAddSeg::SetSegmentCapacity( int32_t aiCapacity )
{
	coBody.ciSegmentCapacity = aiCapacity;
}

int32_t clsRequestAddSeg::GetSegmentCapacity()
{
	return coBody.ciSegmentCapacity;
}
	
int32_t clsRequestAddSeg::GetSize()
{
	return sizeof(coBody);
}

//------------------------------clsRequestWriteSeg------------------------------------//
clsRequestWriteSeg::clsRequestWriteSeg()
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsRequestWriteSeg::~clsRequestWriteSeg()
{
}

int32_t clsRequestWriteSeg::Decode(AIChunkEx& aoChunk)
{
    AI_RETURN_IF( -1, ( clsUserRequest::Decode( aoChunk ) != 0 ) );
    
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) ) != 0 ) );
	return 0;
}

int32_t clsRequestWriteSeg::Encode(AIChunkEx& aoChunk)
{
    AI_RETURN_IF( -1, ( clsUserRequest::Encode( aoChunk ) != 0 ) );
        
	AIChunkExOut loOut(aoChunk);
	loOut.PutMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) );
	return 0;
}

void clsRequestWriteSeg::SetQueueName(const char* apsQueueName)
{
	StringCopy( coBody.ccQueueName, apsQueueName, AI_MAX_NAME_LEN );
}

const char* clsRequestWriteSeg::GetQueueName()
{
	return coBody.ccQueueName;
}

int32_t clsRequestWriteSeg::GetSize()
{
	return sizeof(stBody) + clsUserRequest::GetSize();
}

//-----------------------------------clsRequestWriteSegSecond-----------------------------------//
clsRequestWriteSegSecond::clsRequestWriteSegSecond() 
{
	memset( &coBody, 0, sizeof(stBody) );
}
clsRequestWriteSegSecond::~clsRequestWriteSegSecond() {}

int32_t clsRequestWriteSegSecond::Decode(AIChunkEx& aoChunk)
{
    AI_RETURN_IF( -1, ( clsUserRequest::Decode( aoChunk ) != 0 ) );
        
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciSegmentID ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciModuleID ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciRetcode ) != 0 ) );
	return 0;
}

int32_t clsRequestWriteSegSecond::Encode(AIChunkEx& aoChunk)
{
    AI_RETURN_IF( -1, ( clsUserRequest::Encode( aoChunk ) != 0 ) );
        
	AIChunkExOut loOut(aoChunk);
	loOut.PutMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) );
	loOut.PutNInt32( coBody.ciSegmentID );
	loOut.PutNInt32( coBody.ciModuleID );
	loOut.PutNInt32( coBody.ciRetcode );
	return 0;
}

void clsRequestWriteSegSecond::SetQueueName(const char* apsQueueName)
{
	StringCopy( coBody.ccQueueName, apsQueueName, AI_MAX_NAME_LEN );
}

const char* clsRequestWriteSegSecond::GetQueueName()
{
	return coBody.ccQueueName;
}

void clsRequestWriteSegSecond::SetSegmentID(int32_t aiSegmentID)
{
	coBody.ciSegmentID = aiSegmentID;
}

int32_t clsRequestWriteSegSecond::GetSegmentID()
{
	return coBody.ciSegmentID;
}

void clsRequestWriteSegSecond::SetModuleID(int32_t aiModuleID)
{
	coBody.ciModuleID = aiModuleID;
}

int32_t clsRequestWriteSegSecond::GetModuleID()
{
	return coBody.ciModuleID;
}

void clsRequestWriteSegSecond::SetRetcode(int32_t aiRetcode)
{
	coBody.ciRetcode = aiRetcode;
}

int32_t clsRequestWriteSegSecond::GetRetcode()
{
	return coBody.ciRetcode;
}

int32_t clsRequestWriteSegSecond::GetSize()
{
	return sizeof(stBody) + clsUserRequest::GetSize();
}

//------------------------------clsRequestQueueStat------------------------------------//
clsRequestQueueStat::clsRequestQueueStat()
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsRequestQueueStat::~clsRequestQueueStat()
{
}

int32_t clsRequestQueueStat::Decode(AIChunkEx& aoChunk)
{
    AI_RETURN_IF( -1, ( clsUserRequest::Decode( aoChunk ) != 0 ) );
        
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) ) != 0 ) );
	return 0;
}

int32_t clsRequestQueueStat::Encode(AIChunkEx& aoChunk)
{
    AI_RETURN_IF( -1, ( clsUserRequest::Encode( aoChunk ) != 0 ) );
        
	AIChunkExOut loOut(aoChunk);
	loOut.PutMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) );
	return 0;
}

void clsRequestQueueStat::SetQueueName(const char* apsQueueName)
{
	StringCopy( coBody.ccQueueName, apsQueueName, AI_MAX_NAME_LEN );
}

const char* clsRequestQueueStat::GetQueueName()
{
	return coBody.ccQueueName;
}

int32_t clsRequestQueueStat::GetSize()
{
	return sizeof(stBody) + clsUserRequest::GetSize();
}

//------------------------------clsRequestCommad------------------------------------//
clsRequestCommad::clsRequestCommad()
{
}

clsRequestCommad::~clsRequestCommad()
{
}

int32_t clsRequestCommad::Decode(AIChunkEx& aoChunk)
{
    AI_RETURN_IF( -1, ( clsUserRequest::Decode( aoChunk ) != 0 ) );
	return 0;
}

int32_t clsRequestCommad::Encode(AIChunkEx& aoChunk)
{
    AI_RETURN_IF( -1, ( clsUserRequest::Encode( aoChunk ) != 0 ) );
	return 0;
}

int32_t clsRequestCommad::GetSize()
{
	return clsUserRequest::GetSize();
}

//------------------------clsRequestAlloc------------------------//
clsRequestAlloc::clsRequestAlloc() 
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsRequestAlloc::~clsRequestAlloc() {}

int32_t clsRequestAlloc::Decode(AIChunkEx& aoChunk)
{
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciSegmentID ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccQueueGlobalID, sizeof(coBody.ccQueueGlobalID) ) != 0 ) );
	
	return 0;
}

int32_t clsRequestAlloc::Encode(AIChunkEx& aoChunk)
{
	AIChunkExOut loOut(aoChunk);
	loOut.PutNInt32( coBody.ciSegmentID );
	loOut.PutMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) );
	loOut.PutMem( coBody.ccQueueGlobalID, sizeof(coBody.ccQueueGlobalID) );
	
	return 0;
}

void clsRequestAlloc::SetSegmentID(int32_t aiSegmentID)
{
	coBody.ciSegmentID = aiSegmentID;
}

int32_t clsRequestAlloc::GetSegmentID()
{
	return coBody.ciSegmentID;
}

void clsRequestAlloc::SetQueueName(const char* apsQueueName)
{
	StringCopy( coBody.ccQueueName, apsQueueName, AI_MAX_NAME_LEN );
}

const char* clsRequestAlloc::GetQueueName()
{
	return coBody.ccQueueName;
}

void clsRequestAlloc::SetQueueGlobalID(int64_t aiQueueGlobalID)
{
	IntToString( aiQueueGlobalID, coBody.ccQueueGlobalID, AI_MAX_INT_LEN );
}

int64_t clsRequestAlloc::GetQueueGlobalID()
{
	return atoll( coBody.ccQueueGlobalID );
}

int32_t clsRequestAlloc::GetSize()
{
	return sizeof( stBody );
}

//------------------------clsRequestFree------------------------//
clsRequestFree::clsRequestFree() 
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsRequestFree::~clsRequestFree() {}

int32_t clsRequestFree::Decode(AIChunkEx& aoChunk)
{
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciSegmentID ) != 0 ) );
	
	return 0;
}

int32_t clsRequestFree::Encode(AIChunkEx& aoChunk)
{
	AIChunkExOut loOut(aoChunk);
	loOut.PutNInt32( coBody.ciSegmentID );
	
	return 0;
}

void clsRequestFree::SetSegmentID(int32_t aiSegmentID)
{
	coBody.ciSegmentID = aiSegmentID;
}

int32_t clsRequestFree::GetSegmentID()
{
	return coBody.ciSegmentID;
}

int32_t clsRequestFree::GetSize()
{
	return sizeof( stBody );
}

//------------------------clsRequestSegmentSize------------------------//
clsRequestSegmentSize::clsRequestSegmentSize() 
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsRequestSegmentSize::~clsRequestSegmentSize() {}

int32_t clsRequestSegmentSize::Decode(AIChunkEx& aoChunk)
{
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciSegmentID ) != 0 ) );
	
	return 0;
}

int32_t clsRequestSegmentSize::Encode(AIChunkEx& aoChunk)
{
	AIChunkExOut loOut(aoChunk);
	loOut.PutNInt32( coBody.ciSegmentID );
	
	return 0;
}

void clsRequestSegmentSize::SetSegmentID(int32_t aiSegmentID)
{
	coBody.ciSegmentID = aiSegmentID;
}

int32_t clsRequestSegmentSize::GetSegmentID()
{
	return coBody.ciSegmentID;
}

int32_t clsRequestSegmentSize::GetSize()
{
	return sizeof( stBody );
}

//------------------------clsRequestPut------------------------//
clsRequestPut::clsRequestPut() 
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsRequestPut::~clsRequestPut() {}

int32_t clsRequestPut::Decode(AIChunkEx& aoChunk)
{
    AI_RETURN_IF( -1, ( clsUserRequest::Decode( aoChunk ) != 0 ) );
        
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciSegmentID ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccQueueGlobalID, sizeof(coBody.ccQueueGlobalID) ) != 0 ) );
    AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciDataSize ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccData, coBody.ciDataSize ) != 0 ) );
	
	return 0;
}

int32_t clsRequestPut::Encode(AIChunkEx& aoChunk)
{
    AI_RETURN_IF( -1, ( clsUserRequest::Encode( aoChunk ) != 0 ) );
        
	AIChunkExOut loOut(aoChunk);
	loOut.PutNInt32( coBody.ciSegmentID );
	loOut.PutMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) );
	loOut.PutMem( coBody.ccQueueGlobalID, sizeof(coBody.ccQueueGlobalID) );
    loOut.PutNInt32( coBody.ciDataSize );
	loOut.PutMem( coBody.ccData, coBody.ciDataSize );
	
	return 0;
}

void clsRequestPut::SetSegmentID(int32_t aiSegmentID)
{
	coBody.ciSegmentID = aiSegmentID;
}

int32_t clsRequestPut::GetSegmentID()
{
	return coBody.ciSegmentID;
}

void clsRequestPut::SetQueueName(const char* apsQueueName)
{
	StringCopy( coBody.ccQueueName, apsQueueName, AI_MAX_NAME_LEN );
}

const char* clsRequestPut::GetQueueName()
{
	return coBody.ccQueueName;
}

void clsRequestPut::SetQueueGlobalID(int64_t aiQueueGlobalID)
{
	IntToString( aiQueueGlobalID, coBody.ccQueueGlobalID, AI_MAX_INT_LEN );
}

int64_t clsRequestPut::GetQueueGlobalID()
{
	return atoll( coBody.ccQueueGlobalID );
}

void clsRequestPut::SetData(const AIChunkEx& aoData)
{
	coBody.ciDataSize = aoData.WritePtr() - aoData.ReadPtr();
	
	assert( coBody.ciDataSize > 0 && (size_t)coBody.ciDataSize <= sizeof(coBody.ccData) );
	
	memcpy( coBody.ccData, aoData.ReadPtr(), coBody.ciDataSize );
}

void clsRequestPut::GetData(AIChunkEx& aoData)
{
	assert( coBody.ciDataSize > 0 && (size_t)coBody.ciDataSize <= sizeof(coBody.ccData) );
	
	aoData.Resize( coBody.ciDataSize );
	memcpy( aoData.WritePtr(), coBody.ccData, coBody.ciDataSize );
	aoData.WritePtr(coBody.ciDataSize);
}

int32_t clsRequestPut::GetSize()
{
	//return sizeof( stBody ) + clsUserRequest::GetSize();
    return sizeof(int32_t) + sizeof(coBody.ccQueueName) + sizeof(coBody.ccQueueGlobalID) + coBody.ciDataSize + sizeof(int32_t) + clsUserRequest::GetSize();
}

//------------------------clsRequestGet------------------------//
clsRequestGet::clsRequestGet() 
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsRequestGet::~clsRequestGet() {}

int32_t clsRequestGet::Decode(AIChunkEx& aoChunk)
{
    AI_RETURN_IF( -1, ( clsUserRequest::Decode( aoChunk ) != 0 ) );
        
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciSegmentID ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccQueueGlobalID, sizeof(coBody.ccQueueGlobalID) ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciWhenSec ) != 0 ) );
	
	return 0;
}

int32_t clsRequestGet::Encode(AIChunkEx& aoChunk)
{
    AI_RETURN_IF( -1, ( clsUserRequest::Encode( aoChunk ) != 0 ) );
        
	AIChunkExOut loOut(aoChunk);
	loOut.PutNInt32( coBody.ciSegmentID );
	loOut.PutMem( coBody.ccQueueName, sizeof(coBody.ccQueueName) );
	loOut.PutMem( coBody.ccQueueGlobalID, sizeof(coBody.ccQueueGlobalID) );
	loOut.PutNInt32( coBody.ciWhenSec );
	
	return 0;
}

void clsRequestGet::SetSegmentID(int32_t aiSegmentID)
{
	coBody.ciSegmentID = aiSegmentID;
}

int32_t clsRequestGet::GetSegmentID()
{
	return coBody.ciSegmentID;
}

void clsRequestGet::SetQueueName(const char* apsQueueName)
{
	StringCopy( coBody.ccQueueName, apsQueueName, AI_MAX_NAME_LEN );
}

const char* clsRequestGet::GetQueueName()
{
	return coBody.ccQueueName;
}

void clsRequestGet::SetQueueGlobalID(int64_t aiQueueGlobalID)
{
	IntToString( aiQueueGlobalID, coBody.ccQueueGlobalID, AI_MAX_INT_LEN );
}

int64_t clsRequestGet::GetQueueGlobalID()
{
	return atoll( coBody.ccQueueGlobalID );
}

void clsRequestGet::SetWhenSec(int32_t aiWhenSec)
{
	coBody.ciWhenSec = aiWhenSec;
}

int32_t clsRequestGet::GetWhenSec()
{
	return coBody.ciWhenSec;
}

int32_t clsRequestGet::GetSize()
{
	return sizeof( stBody ) + clsUserRequest::GetSize();
}

//------------------------clsRequestHeartBeat------------------------//
clsRequestHeartBeat::clsRequestHeartBeat()
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsRequestHeartBeat::~clsRequestHeartBeat()
{
}

int32_t clsRequestHeartBeat::Decode(AIChunkEx& aoChunk)
{
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccModuleName, sizeof(coBody.ccModuleName) ) != 0 ) );
	
	return 0;
}

int32_t clsRequestHeartBeat::Encode(AIChunkEx& aoChunk)
{
	AIChunkExOut loOut(aoChunk);
	loOut.PutMem( coBody.ccModuleName, sizeof(coBody.ccModuleName) );
	
	return 0;
}

void clsRequestHeartBeat::SetModuleName(const char* apsModuleName)
{
	StringCopy( coBody.ccModuleName, apsModuleName, sizeof(coBody.ccModuleName) );
}

const char* clsRequestHeartBeat::GetModuleName()
{
	return  coBody.ccModuleName;
}

int32_t clsRequestHeartBeat::GetSize()
{
	return sizeof(stBody);
}

//--------------------------------clsResponseSegment---------------------------------//
clsResponseSegment::clsResponseSegment()
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsResponseSegment::~clsResponseSegment()
{
}

int32_t clsResponseSegment::Decode(AIChunkEx& aoChunk)
{
	clsResponse::Decode(aoChunk);
		
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciSegmentID ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciModuleID ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccQueueGlobalID, sizeof(coBody.ccQueueGlobalID) ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccIpAddr, sizeof(coBody.ccIpAddr) ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciPort ) != 0 ) );
	
	return 0;
}

int32_t clsResponseSegment::Encode(AIChunkEx& aoChunk)
{
	clsResponse::Encode(aoChunk);
		
	AIChunkExOut loOut(aoChunk);
	loOut.PutNInt32( coBody.ciSegmentID );
	loOut.PutNInt32( coBody.ciModuleID );
	loOut.PutMem( coBody.ccQueueGlobalID, sizeof(coBody.ccQueueGlobalID) );
	loOut.PutMem( coBody.ccIpAddr, sizeof(coBody.ccIpAddr) );
	loOut.PutNInt32( coBody.ciPort );
	
	return 0;
}

void clsResponseSegment::SetSegmentID(int32_t aiSegmentID)
{
	coBody.ciSegmentID = aiSegmentID;
}

int32_t clsResponseSegment::GetSegmentID()
{
	return coBody.ciSegmentID;
}

void clsResponseSegment::SetModuleID(int32_t aiModuleID)
{
	coBody.ciModuleID = aiModuleID;
}

int32_t clsResponseSegment::GetModuleID()
{
	return coBody.ciModuleID;
}

void clsResponseSegment::SetQueueGlobalID(int64_t aiQueueGlobalID)
{
	IntToString( aiQueueGlobalID, coBody.ccQueueGlobalID, AI_MAX_INT_LEN );
}

int64_t clsResponseSegment::GetQueueGlobalID()
{
	return atoll( coBody.ccQueueGlobalID );
}

void clsResponseSegment::SetIpAddr(const char* apsIpAddr)
{
	StringCopy( coBody.ccIpAddr, apsIpAddr, AI_IP_ADDR_LEN );
}

const char* clsResponseSegment::GetIpAddr()
{
	return  coBody.ccIpAddr;
}

void clsResponseSegment::SetPort(int32_t aiPort)
{
	coBody.ciPort = aiPort;
}

int32_t clsResponseSegment::GetPort()
{
	return coBody.ciPort;
}

int32_t clsResponseSegment::GetSize()
{
	return clsResponse::GetSize() + sizeof(stBody);
}

//------------------------clsResponseQueueStat------------------------//
clsResponseQueueStat::clsResponseQueueStat() 
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsResponseQueueStat::~clsResponseQueueStat() {}

int32_t clsResponseQueueStat::Decode(AIChunkEx& aoChunk)
{
	clsResponse::Decode(aoChunk);
		
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciSize ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciCapacity ) != 0 ) );
	
	return 0;
}

int32_t clsResponseQueueStat::Encode(AIChunkEx& aoChunk)
{
	clsResponse::Encode(aoChunk);
		
	AIChunkExOut loOut(aoChunk);
	loOut.PutNInt32( coBody.ciSize );
	loOut.PutNInt32( coBody.ciCapacity );
	
	return 0;
}

void clsResponseQueueStat::SetQueueSize( int32_t aiSize )
{
	coBody.ciSize = aiSize;
}

int32_t clsResponseQueueStat::GetQueueSize()
{
	return coBody.ciSize;
}

void clsResponseQueueStat::SetQueueCapacity( int32_t aiCapacity )
{
	coBody.ciCapacity = aiCapacity;
}

int32_t clsResponseQueueStat::GetQueueCapacity()
{
	return coBody.ciCapacity;
}
	
int32_t clsResponseQueueStat::GetSize()
{
	return clsResponse::GetSize() + sizeof( stBody );
}

//------------------------clsResponseModuleCount------------------------//
clsResponseModuleCount::clsResponseModuleCount() 
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsResponseModuleCount::~clsResponseModuleCount() {}

int32_t clsResponseModuleCount::Decode(AIChunkEx& aoChunk)
{
	clsResponse::Decode(aoChunk);
		
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciCount ) != 0 ) );
	
	return 0;
}

int32_t clsResponseModuleCount::Encode(AIChunkEx& aoChunk)
{
	clsResponse::Encode(aoChunk);
		
	AIChunkExOut loOut(aoChunk);
	loOut.PutNInt32( coBody.ciCount );
	
	return 0;
}

void clsResponseModuleCount::SetModuleCount( int32_t aiCount )
{
	coBody.ciCount = aiCount;
}

int32_t clsResponseModuleCount::GetModuleCount()
{
	return coBody.ciCount;
}
	
int32_t clsResponseModuleCount::GetSize()
{
	return clsResponse::GetSize() + sizeof( stBody );
}

//------------------------clsResponseSegmentSize------------------------//
clsResponseSegmentSize::clsResponseSegmentSize() 
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsResponseSegmentSize::~clsResponseSegmentSize() {}

int32_t clsResponseSegmentSize::Decode(AIChunkEx& aoChunk)
{
	clsResponse::Decode(aoChunk);
		
	AIChunkExIn loIn(aoChunk);
	AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciSize ) != 0 ) );
	
	return 0;
}

int32_t clsResponseSegmentSize::Encode(AIChunkEx& aoChunk)
{
	clsResponse::Encode(aoChunk);
		
	AIChunkExOut loOut(aoChunk);
	loOut.PutNInt32( coBody.ciSize );
	
	return 0;
}

void clsResponseSegmentSize::SetSegmentSize( int32_t aiSize )
{
	coBody.ciSize = aiSize;
}

int32_t clsResponseSegmentSize::GetSegmentSize()
{
	return coBody.ciSize;
}
	
int32_t clsResponseSegmentSize::GetSize()
{
	return clsResponse::GetSize() + sizeof( stBody );
}

//------------------------clsResponseGet------------------------//
clsResponseGet::clsResponseGet() 
{
	memset( &coBody, 0, sizeof(stBody) );
}

clsResponseGet::~clsResponseGet() {}

int32_t clsResponseGet::Decode(AIChunkEx& aoChunk)
{
	clsResponse::Decode(aoChunk);
		
	AIChunkExIn loIn(aoChunk);
    AI_RETURN_IF( -1, ( loIn.ToHInt32( coBody.ciDataSize ) != 0 ) );
	AI_RETURN_IF( -1, ( loIn.ToMem( coBody.ccData, coBody.ciDataSize ) != 0 ) );
	
	return 0;
}

int32_t clsResponseGet::Encode(AIChunkEx& aoChunk)
{
	clsResponse::Encode(aoChunk);
		
	AIChunkExOut loOut(aoChunk);
    loOut.PutNInt32( coBody.ciDataSize );
	loOut.PutMem( coBody.ccData, coBody.ciDataSize );
	
	return 0;
}

void clsResponseGet::SetData(const AIChunkEx& aoData)
{
	coBody.ciDataSize = aoData.WritePtr() - aoData.ReadPtr();
	
	assert( coBody.ciDataSize > 0 && (size_t)coBody.ciDataSize <= sizeof(coBody.ccData) );
	
	memcpy( coBody.ccData, aoData.ReadPtr(), coBody.ciDataSize );
}

void clsResponseGet::GetData(AIChunkEx& aoData)
{ 
	assert( coBody.ciDataSize > 0 && (size_t)coBody.ciDataSize <= sizeof(coBody.ccData) );
	
	aoData.Resize( coBody.ciDataSize );
	memcpy( aoData.WritePtr(), coBody.ccData, coBody.ciDataSize );
	aoData.WritePtr(coBody.ciDataSize);
}

int32_t clsResponseGet::GetSize()
{
	return clsResponse::GetSize() + sizeof(int32_t) + coBody.ciDataSize;
}


// end namespace
AI_GFQ_NAMESPACE_END

