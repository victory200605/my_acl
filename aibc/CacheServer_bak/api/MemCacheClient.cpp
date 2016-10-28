
#include "../include/MemCacheClient.h"

clsMemCacheClient::clsMemCacheClient()
    : coChannel(AICache::AIBaseChannel::NORMAL)
{
}

int clsMemCacheClient::Connect( const char* apcServerIp, int aiPort, int aiConnCnt )
{
    return this->coConnector.Connect( apcServerIp, aiPort, coChannel );
}

int clsMemCacheClient::Add( const char* apcKey, const char* apcValue )
{
    int liRetCode = 0;
    AIBC::AIChunkEx loRequest(0);
    AIBC::AIChunkEx loResponse(0);
    AICache::AICmdLineParamter loParamter;
        
    AICache::AICmdLineParamter::Encode( loRequest, "add k=%s v=%s", apcKey, apcValue );
    
    if ( (liRetCode = this->coChannel.Send(loRequest.BasePtr(), loRequest.GetSize()) ) != 0 )
    {
        return liRetCode;
    }
    
    if ( (liRetCode = this->coChannel.Recv( loResponse, 10 ) ) != 0 )
    {
        return liRetCode;
    }
    
    if ( loParamter.Decode( loResponse.BasePtr(), loResponse.GetSize() ) != 0 )
    {
        return AICache::AI_ERROR_INVALID_RESPONSE;
    }
    
    if ( loParamter.GetPtr('r') == NULL )
    {
        return AICache::AI_ERROR_INVALID_RESPONSE;
    }

    liRetCode = atoi(loParamter.GetPtr('r'));
    
    return liRetCode;
}

int clsMemCacheClient::Put( const char* apcKey, const char* apcValue )
{
    int liRetCode = 0;
    AIBC::AIChunkEx loRequest(0);
    AIBC::AIChunkEx loResponse(0);
    AICache::AICmdLineParamter loParamter;
        
    AICache::AICmdLineParamter::Encode( loRequest, "put k=%s v=%s", apcKey, apcValue );
    
    if ( (liRetCode = this->coChannel.Send(loRequest.BasePtr(), loRequest.GetSize()) ) != 0 )
    {
        return liRetCode;
    }
    
    if ( (liRetCode = this->coChannel.Recv( loResponse, 10 ) ) != 0 )
    {
        return liRetCode;
    }
    
    if ( loParamter.Decode( loResponse.BasePtr(), loResponse.GetSize() ) != 0 )
    {
        return AICache::AI_ERROR_INVALID_RESPONSE;
    }
    
    if ( loParamter.GetPtr('r') == NULL )
    {
        return AICache::AI_ERROR_INVALID_RESPONSE;
    }

    liRetCode = atoi(loParamter.GetPtr('r'));
    
    return liRetCode;
}

int clsMemCacheClient::Get( const char* apcKey, AIBC::AIChunkEx& aoValue )
{
    int liRetCode = 0;
    AIBC::AIChunkEx loRequest(0);
    AIBC::AIChunkEx loResponse(0);
    AICache::AICmdLineParamter loParamter;
    
    AICache::AICmdLineParamter::Encode( loRequest, "get k=%s", apcKey );
    
    if ( (liRetCode = this->coChannel.Send(loRequest.BasePtr(), loRequest.GetSize()) ) != 0 )
    {
        return liRetCode;
    }
    
    if ( (liRetCode = this->coChannel.Recv( loResponse ) ) != 0 )
    {
        return liRetCode;
    }
    
    if ( loParamter.Decode( loResponse.BasePtr(), loResponse.GetSize() ) != 0 )
    {
        return AICache::AI_ERROR_INVALID_RESPONSE;
    }
    
    if ( loParamter.GetPtr('r') == NULL || loParamter.GetPtr('v') == NULL )
    {
        return AICache::AI_ERROR_INVALID_RESPONSE;
    }

    liRetCode = atoi(loParamter.GetPtr('r'));
    
    aoValue.Resize( loParamter.GetLen('v') + 1 );
    ::memcpy( aoValue.BasePtr(), loParamter.GetPtr('v'), loParamter.GetLen('v') );
    
    return liRetCode;
}

int clsMemCacheClient::Del( const char* apcKey, AIBC::AIChunkEx& aoValue )
{
    int liRetCode = 0;
    AIBC::AIChunkEx loRequest(0);
    AIBC::AIChunkEx loResponse(0);
    AICache::AICmdLineParamter loParamter;
    
    AICache::AICmdLineParamter::Encode( loRequest, "del k=%s", apcKey );
    
    if ( (liRetCode = this->coChannel.Send(loRequest.BasePtr(), loRequest.GetSize()) ) != 0 )
    {
        return liRetCode;
    }
    
    if ( (liRetCode = this->coChannel.Recv( loResponse ) ) != 0 )
    {
        return liRetCode;
    }
    
    if ( loParamter.Decode( loResponse.BasePtr(), loResponse.GetSize() ) != 0 )
    {
        return AICache::AI_ERROR_INVALID_RESPONSE;
    }
    
    if ( loParamter.GetPtr('r') == NULL || loParamter.GetPtr('v') == NULL )
    {
        return AICache::AI_ERROR_INVALID_RESPONSE;
    }

    liRetCode = atoi(loParamter.GetPtr('r'));
    
    aoValue.Resize( loParamter.GetLen('v') + 1 );
    ::memcpy( aoValue.BasePtr(), loParamter.GetPtr('v'), loParamter.GetLen('v') );
    
    return liRetCode;
}
