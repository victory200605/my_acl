
#include "../include/CSProtocol.h"

AI_CACHE_NAMESPACE_START

/////////////////////////////////////////////// clsHeader /////////////////////////////////////////
clsHeader::clsHeader( int aiCmd /* = 0 */, clsBody* apoBody /* = NULL */)
    : ciTotalLen(0)
    , ciCmd(aiCmd)
    , cpoBody(apoBody)
{
}

clsHeader::~clsHeader()
{
}

int clsHeader::Decode(AIBC::AIChunkEx& aoBuffer)
{
    AIBC::AIChunkExIn loInput( aoBuffer );
    
    INVALID_RET( loInput.ToHInt32( this->ciTotalLen ), -1 );
    INVALID_RET( loInput.ToHInt32( this->ciCmd ), -1 );
    
    if ( this->cpoBody != NULL )
    {
        return this->cpoBody->Decode(aoBuffer);
    }
    
	return 0;
}

int clsHeader::Encode(AIBC::AIChunkEx& aoBuffer)
{
	AIBC::AIChunkExOut loOutput( aoBuffer );
	
	this->ciTotalLen = this->GetSize();
	
	loOutput.PutNInt32( this->ciTotalLen );
    loOutput.PutNInt32( this->ciCmd );
    
    if ( this->cpoBody != NULL )
    {
        return this->cpoBody->Encode(aoBuffer);
    }
	return 0;
}

int clsHeader::GetCmd()
{
    return this->ciCmd;
}

int clsHeader::GetSize()
{
    if ( this->cpoBody != NULL )
    {
        return this->cpoBody->GetSize() + sizeof(int);
    }
    
	return sizeof(int);
}

/////////////////////////////////////////////////// Request ///////////////////////////////////////
clsRequestPut::clsRequestPut()
    : ciMsgType(0)
    , coKey(AI_DEFAULT_KEY_LEN)
    , coValue(AI_DEFAULT_VALUE_LEN)
{
    this->coKey.Resize(0);
    this->coValue.Resize(0);
}

clsRequestPut::~clsRequestPut()
{
}

int clsRequestPut::Decode(AIBC::AIChunkEx& aoBuffer)
{
	AIBC::AIChunkExIn loInput( aoBuffer );
	int         liKeySize   = 0;
	int         liValueSize = 0;
	
	INVALID_RET( loInput.ToHInt32( this->ciMsgType ), -1 );
	INVALID_RET( loInput.ToHInt32( liKeySize ), -1 );
	
	AI_RETURN_IF( -1, (size_t)liKeySize > AI_MAX_KEY_LEN );
	this->coKey.Resize(liKeySize);
	
	INVALID_RET( loInput.ToMem( this->coKey.BasePtr(), this->coKey.GetSize() ), -1 );
	
	INVALID_RET( loInput.ToHInt32( liValueSize ), -1 );
	
	AI_RETURN_IF( -1, (size_t)liValueSize > AI_MAX_VALUE_LEN );
	this->coValue.Resize(liValueSize);
	
	INVALID_RET( loInput.ToMem( this->coValue.BasePtr(), this->coValue.GetSize() ), -1 );
	
	return 0;
}

int clsRequestPut::Encode(AIBC::AIChunkEx& aoBuffer)
{
	AIBC::AIChunkExOut loOutput( aoBuffer );
	int          liKeySize   = this->coKey.GetSize();
	int          liValueSize = this->coValue.GetSize();
	
	loOutput.PutNInt32( this->ciMsgType );
	loOutput.PutNInt32( liKeySize );
	loOutput.PutMem( this->coKey.BasePtr(), this->coKey.GetSize() );
	
	loOutput.PutNInt32( liValueSize );
	loOutput.PutMem( this->coValue.BasePtr(), this->coValue.GetSize() );
    	
	return 0;
}

int clsRequestPut::GetSize()
{
	return sizeof(int) + this->coKey.GetSize() + this->coValue.GetSize();
}

AIBC::AIChunkEx& clsRequestPut::GetKey()
{
    return this->coKey;
}

AIBC::AIChunkEx& clsRequestPut::GetValue()
{
    return this->coValue;
}
/////////////////////end clsRequestPut///////////////////////////////

////////////////////start clsRequestGet//////////////////////////////
clsRequestGet::clsRequestGet()
    : coKey(AI_DEFAULT_KEY_LEN)
{
    this->coKey.Resize(0);
}

clsRequestGet::~clsRequestGet()
{
}

int clsRequestGet::Decode(AIBC::AIChunkEx& aoBuffer)
{
	AIBC::AIChunkExIn loInput( aoBuffer );
	int         liKeySize   = 0;
	
	INVALID_RET( loInput.ToHInt32( liKeySize ), -1 );
	
	AI_RETURN_IF( -1, (size_t)liKeySize > AI_MAX_KEY_LEN );
	this->coKey.Resize(liKeySize);
	
	INVALID_RET( loInput.ToMem( this->coKey.BasePtr(), this->coKey.GetSize() ), -1 );
	
	return 0;
}

int clsRequestGet::Encode(AIBC::AIChunkEx& aoBuffer)
{
	AIBC::AIChunkExOut loOutput( aoBuffer );
	int          liKeySize   = this->coKey.GetSize();
	
	loOutput.PutNInt32( liKeySize );
	loOutput.PutMem( this->coKey.BasePtr(), this->coKey.GetSize() );
    
	return 0;
}

int clsRequestGet::GetSize()
{
	return sizeof(int) + this->coKey.GetSize();
}

AIBC::AIChunkEx& clsRequestGet::GetKey()
{
    return this->coKey;
}
///////////////////end clsRequestGet////////////////////////////////

////////////////////////////////////////// start clsRequestDelTimeout ///////////////////////////////////
clsRequestDelTimeOut::clsRequestDelTimeOut()
    : ciMsgType(0)
{
}

clsRequestDelTimeOut::~clsRequestDelTimeOut()
{
}

int clsRequestDelTimeOut::Decode(AIBC::AIChunkEx& aoBuffer)
{
    AIBC::AIChunkExIn loInput( aoBuffer );
	
	INVALID_RET( loInput.ToHInt32( this->ciMsgType ), -1 );
	
	return 0;
}

int clsRequestDelTimeOut::Encode(AIBC::AIChunkEx& aoBuffer)
{
    AIBC::AIChunkExOut loOutput( aoBuffer );

	loOutput.PutNInt32( this->ciMsgType );
	
	return 0;
}
	
int clsRequestDelTimeOut::GetSize()
{
    return sizeof(int);
}
////////////////////////////////////////// end clsRequestDelTimeout ////////////////////////////////////

////////////////////start clsRequestDelTimeOut///////////////////////
clsRequestCommon::clsRequestCommon()
{
}

clsRequestCommon::~clsRequestCommon()
{
}

int clsRequestCommon::Decode(AIBC::AIChunkEx& aoBuffer)
{
	return 0;
}

int clsRequestCommon::Encode(AIBC::AIChunkEx& aoBuffer)
{
	return 0;
}

int clsRequestCommon::GetSize()
{
	return 0;
}
////////////////////end clsRequestDelTimeOut////////////////////////

/////////////////////////////////////////////// Response ///////////////////////////////////
clsResponse::clsResponse()
    : ciStat(0)
{
}

clsResponse::~clsResponse()
{
}

int clsResponse::Decode(AIBC::AIChunkEx& aoBuffer)
{
    AIBC::AIChunkExIn loInput( aoBuffer );
    
    INVALID_RET( loInput.ToHInt32( this->ciStat ), -1 );
    
	return 0;
}

int clsResponse::Encode(AIBC::AIChunkEx& aoBuffer)
{
    AIBC::AIChunkExOut loOutput( aoBuffer );
    
    loOutput.PutNInt32( this->ciStat );
    
	return 0;
}

int clsResponse::GetStat()
{
    return this->ciStat;
}

void clsResponse::SetStat(int aiStat)
{
    this->ciStat = aiStat;
}

int clsResponse::GetSize()
{
	return sizeof(int);
}
/////////////////////////////end clsResponse/////////////////////////

/////////////////////////////start clsResponseGet////////////////////
clsResponseGet::clsResponseGet()
    : coValue(AI_DEFAULT_VALUE_LEN)
{
    this->coValue.Resize(0);
}

clsResponseGet::~clsResponseGet()
{
}

int clsResponseGet::Decode(AIBC::AIChunkEx& aoBuffer)
{
    AI_RETURN_IF( -1, clsResponse::Decode( aoBuffer ) != 0 );
        
	AIBC::AIChunkExIn loInput( aoBuffer );
	int         liValueSize = 0;

	INVALID_RET( loInput.ToHInt32( liValueSize ), -1 );
	
	AI_RETURN_IF( -1, (size_t)liValueSize > AI_MAX_VALUE_LEN );
	this->coValue.Resize(liValueSize);
	
	INVALID_RET( loInput.ToMem( this->coValue.BasePtr(), this->coValue.GetSize() ), -1 );
	
	return 0;
}

int clsResponseGet::Encode(AIBC::AIChunkEx& aoBuffer)
{
    AI_RETURN_IF( -1, clsResponse::Encode( aoBuffer ) != 0 );
        
	AIBC::AIChunkExOut loOutput( aoBuffer );
	int          liValueSize = this->coValue.GetSize();
	
	loOutput.PutNInt32( liValueSize );
	loOutput.PutMem( this->coValue.BasePtr(), this->coValue.GetSize() );
	
	return 0;
}

AIBC::AIChunkEx& clsResponseGet::GetValue()
{
    return this->coValue;
}

int clsResponseGet::GetSize()
{
	return clsResponse::GetSize() + sizeof(int) + this->coValue.GetSize();
}
/////////////////////////////end clsResponseGet////////////////////

/////////////////////////////start clsResponseDelTimeOut////////////////////
clsResponseDelTimeOut::clsResponseDelTimeOut()
    : coKey(AI_DEFAULT_KEY_LEN)
    , coValue(AI_DEFAULT_VALUE_LEN)
{
    this->coKey.Resize(0);
    this->coValue.Resize(0);
}

clsResponseDelTimeOut::~clsResponseDelTimeOut()
{
}

int clsResponseDelTimeOut::Decode(AIBC::AIChunkEx& aoBuffer)
{
    AI_RETURN_IF( -1, clsResponse::Decode( aoBuffer ) != 0 );
        
	AIBC::AIChunkExIn loInput( aoBuffer );
	int         liKeySize   = 0;
	int         liValueSize = 0;
	
	INVALID_RET( loInput.ToHInt32( liKeySize ), -1 );
	
	AI_RETURN_IF( -1, (size_t)liKeySize > AI_MAX_KEY_LEN );
	this->coKey.Resize(liKeySize);
	
	INVALID_RET( loInput.ToMem( this->coKey.BasePtr(), this->coKey.GetSize() ), -1 );

	INVALID_RET( loInput.ToHInt32( liValueSize ), -1 );
	
	AI_RETURN_IF( -1, (size_t)liValueSize > AI_MAX_VALUE_LEN );
	this->coValue.Resize(liValueSize);
	
	INVALID_RET( loInput.ToMem( this->coValue.BasePtr(), this->coValue.GetSize() ), -1 );
	
	return 0;
}

int clsResponseDelTimeOut::Encode(AIBC::AIChunkEx& aoBuffer)
{
    AI_RETURN_IF( -1, clsResponse::Encode( aoBuffer ) != 0 );
        
	AIBC::AIChunkExOut loOutput( aoBuffer );
	int          liKeySize   = this->coKey.GetSize();
	int          liValueSize = this->coValue.GetSize();
	
	loOutput.PutNInt32( liKeySize );
	loOutput.PutMem( this->coKey.BasePtr(), this->coKey.GetSize() );
	
	loOutput.PutNInt32( liValueSize );
	loOutput.PutMem( this->coValue.BasePtr(), this->coValue.GetSize() );
	
	return 0;
}

int clsResponseDelTimeOut::GetSize()
{
    return clsResponse::GetSize() + this->coKey.GetSize() + this->coValue.GetSize();
}

AIBC::AIChunkEx& clsResponseDelTimeOut::GetKey()
{
    return this->coKey;
}

AIBC::AIChunkEx& clsResponseDelTimeOut::GetValue()
{
    return this->coValue;
}
/////////////////////////////end clsResponseDelTimeOut////////////////////

/////////////////////////////start clsResponseStat////////////////////
clsResponseStat::clsResponseStat()
    : ciFileCount(0)
    , ciCapacity(0)
    , ciSize(0)
{
}

clsResponseStat::~clsResponseStat()
{
}

int clsResponseStat::Decode(AIBC::AIChunkEx& aoBuffer)
{
    AI_RETURN_IF( -1, clsResponse::Decode( aoBuffer ) != 0 );
        
	AIBC::AIChunkExIn loInput( aoBuffer );
    INVALID_RET( loInput.ToHInt32( this->ciFileCount), -1 );
    INVALID_RET( loInput.ToHInt32( this->ciCapacity ), -1 );
    INVALID_RET( loInput.ToHInt32( this->ciSize ), -1 );
    
    return 0;
}

int clsResponseStat::Encode(AIBC::AIChunkEx& aoBuffer)
{
    AI_RETURN_IF( -1, clsResponse::Encode( aoBuffer ) != 0 );
        
	AIBC::AIChunkExOut loOutput( aoBuffer );
    loOutput.PutNInt32( this->ciFileCount );
    loOutput.PutNInt32( this->ciCapacity );
    loOutput.PutNInt32( this->ciSize );
    
    return 0;
}

int clsResponseStat::GetSize()
{
	return sizeof(int) * 3 + clsResponse::GetSize();
}

void clsResponseStat::SetFileCount(int aiFileCnt )
{
    this->ciFileCount = aiFileCnt;
}

int clsResponseStat::GetFileCount()
{
	return this->ciFileCount;
}

void clsResponseStat::SetCacheCapacity(int aiCapacity)
{
    this->ciCapacity = aiCapacity;
}

int clsResponseStat::GetCacheCapacity()
{
    return this->ciCapacity;
}

void clsResponseStat::SetCacheSize(int aiSize)
{
	this->ciSize = aiSize;
}

int clsResponseStat::GetCacheSize()
{
	return this->ciSize;
}
/////////////////////////////end clsResponseStat////////////////////

AI_CACHE_NAMESPACE_END
