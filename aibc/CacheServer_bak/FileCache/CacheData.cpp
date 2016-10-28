
#include "CacheData.h"

AI_CACHE_NAMESPACE_START

///////////////////////////////////////////stDataNode////////////////////////////////////
stDataNodeHeader::stDataNodeHeader()
{
    memset( this, 0, sizeof(stDataNodeHeader) );
}

int stDataNodeHeader::Encode( 
    AIBC::AIChunkEx& aoBuffer, const AIBC::AIChunkEx& aoKey, const AIBC::AIChunkEx& aoValue, int aiTimestamp )
{
    AIBC::AIChunkExOut loOutput(aoBuffer);
    
    //reset buffer
    aoBuffer.Reset();
    aoBuffer.Resize(0);
    
    this->ciKeySize   = aoKey.GetSize();
    this->ciValueSize = aoValue.GetSize();
    this->ciTimestamp = aiTimestamp;
    
    loOutput.PutNInt32( this->ciKeySize );
    loOutput.PutNInt32( this->ciValueSize );
    loOutput.PutNInt32( this->ciTimestamp );
    
    loOutput.PutMem( aoKey.BasePtr(), aoKey.GetSize() );
    loOutput.PutMem( aoValue.BasePtr(), aoValue.GetSize() );
    
    return 0;
}

int stDataNodeHeader::Decode( 
    AIBC::AIChunkEx& aoBuffer, AIBC::AIChunkEx& aoKey,  AIBC::AIChunkEx& aoValue )
{
    AIBC::AIChunkExIn loInput(aoBuffer);
    
    //reset buffer
    aoBuffer.Reset();
    aoBuffer.WritePtr( aoBuffer.GetSize() );
    
    AI_RETURN_IF( -1, loInput.ToHInt32( this->ciKeySize ) !=0 );
    AI_RETURN_IF( -1, loInput.ToHInt32( this->ciValueSize ) !=0 );
    AI_RETURN_IF( -1, loInput.ToHInt32( this->ciTimestamp ) !=0 );
    
    AI_RETURN_IF( -1, this->ciKeySize <= 0 || (size_t)this->ciKeySize > AI_MAX_KEY_LEN );
    AI_RETURN_IF( -1, this->ciValueSize <= 0 || (size_t)this->ciValueSize > AI_MAX_VALUE_LEN );
    
    aoKey.Resize(this->ciKeySize);
    aoValue.Resize(this->ciValueSize);
    
    AI_RETURN_IF( -1, loInput.ToMem( aoKey.BasePtr(), aoKey.GetSize() ) != 0 );
    AI_RETURN_IF( -1, loInput.ToMem( aoValue.BasePtr(), aoValue.GetSize() ) != 0 );
    
    return 0;
}

size_t stDataNodeHeader::GetNodeSize( size_t aiKeySize, size_t aiValueSize )
{
    return sizeof(int)*3 + aiKeySize + aiValueSize;
}

////////////////////////////////////////////// clsCacheData ///////////////////////////////////
//Default Constructor
clsCacheData::clsCacheData()
    : coNodeBuffer(0)
{
}
    
//Default Destructor
clsCacheData::~clsCacheData()
{
}

int clsCacheData::Read( int aiIdx, AIBC::AIChunkEx& aoKey, AIBC::AIChunkEx& aoValue, int* apiTimestamp )
{
    this->coNodeBuffer.Resize( TBase::GetNodeSize() );
    
    int liRet = TBase::Read( aiIdx, this->coNodeBuffer );
    if ( liRet != AI_NO_ERROR )
    {
        //Oh, no, read data node fail!
        return liRet;
    }

    if ( this->coNodeHeader.Decode( this->coNodeBuffer, aoKey, aoValue ) != 0 )
    {
        //Oh, invalid data, may be filesystem destroy
        return AI_ERROR_INVALID_DATA;
    }
    
    //set return variable
    if ( apiTimestamp != NULL ) *apiTimestamp = this->coNodeHeader.ciTimestamp;
    
    return AI_NO_ERROR;
}

int clsCacheData::Write( int aiIdx, const AIBC::AIChunkEx& aoKey, const AIBC::AIChunkEx& aoValue, int aiTimestamp )
{
    if ( this->coNodeHeader.Encode( this->coNodeBuffer, aoKey, aoValue, aiTimestamp ) != 0 )
    {
        return AI_ERROR_INVALID_DATA;
    }
    
    return TBase::Write( aiIdx, this->coNodeBuffer );
}

size_t clsCacheData::GetNodeSize( size_t aiKeySize, size_t aiValueSize )
{
    return stDataNodeHeader::GetNodeSize( aiKeySize, aiValueSize );
}
	
AI_CACHE_NAMESPACE_END
