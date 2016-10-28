
#include "cacheserver/MemCacheClient.h"
#include "../common/AICmdLineTcp.h"

/////////////////////////////////////////////////////////////////////////
class clsMemCacheClientImpl
{
public:
    clsMemCacheClientImpl();
    
	int Connect( const char* apcServerIp, int aiPort, int aiConnCnt = 1 );
	void Close();
	
	int Add( const char* apcKey, const char* apcValue );
	
    int Put( const char* apcKey, const char* apcValue );

	int Get( const char* apcKey, AIBC::AIChunkEx& aoValue );
	
	int Del( const char* apcKey, AIBC::AIChunkEx& aoValue );
	
// attribute
protected:
    cacheserver::AIBaseConnector  coConnector;
    cacheserver::AICmdLineChannel coChannel;
};

//////////////////////////////////////////////////////////////////////////
clsMemCacheClientImpl::clsMemCacheClientImpl()
    : coChannel(cacheserver::AIBaseChannel::NORMAL)
{
}

int clsMemCacheClientImpl::Connect( const char* apcServerIp, int aiPort, int aiConnCnt )
{
    return this->coConnector.Connect( apcServerIp, aiPort, coChannel );
}

void clsMemCacheClientImpl::Close(void)
{
    this->coConnector.Close();
}

int clsMemCacheClientImpl::Add( const char* apcKey, const char* apcValue )
{
    int liRetCode = 0;
    AIBC::AIChunkEx loRequest(0);
    AIBC::AIChunkEx loResponse(0);
    cacheserver::AICmdLineParamter loParamter;
        
    cacheserver::AICmdLineParamter::Encode( loRequest, "add k=%s v=%s", apcKey, apcValue );
    
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
        return cacheserver::AI_ERROR_INVALID_RESPONSE;
    }
    
    if ( loParamter.GetPtr('r') == NULL )
    {
        return cacheserver::AI_ERROR_INVALID_RESPONSE;
    }

    liRetCode = atoi(loParamter.GetPtr('r'));
    
    return liRetCode;
}

int clsMemCacheClientImpl::Put( const char* apcKey, const char* apcValue )
{
    int liRetCode = 0;
    AIBC::AIChunkEx loRequest(0);
    AIBC::AIChunkEx loResponse(0);
    cacheserver::AICmdLineParamter loParamter;
        
    cacheserver::AICmdLineParamter::Encode( loRequest, "put k=%s v=%s", apcKey, apcValue );
    
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
        return cacheserver::AI_ERROR_INVALID_RESPONSE;
    }
    
    if ( loParamter.GetPtr('r') == NULL )
    {
        return cacheserver::AI_ERROR_INVALID_RESPONSE;
    }

    liRetCode = atoi(loParamter.GetPtr('r'));
    
    return liRetCode;
}

int clsMemCacheClientImpl::Get( const char* apcKey, AIBC::AIChunkEx& aoValue )
{
    int liRetCode = 0;
    AIBC::AIChunkEx loRequest(0);
    AIBC::AIChunkEx loResponse(0);
    cacheserver::AICmdLineParamter loParamter;
    
    cacheserver::AICmdLineParamter::Encode( loRequest, "get k=%s", apcKey );
    
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
        return cacheserver::AI_ERROR_INVALID_RESPONSE;
    }
    
    if ( loParamter.GetPtr('r') == NULL || loParamter.GetPtr('v') == NULL )
    {
        return cacheserver::AI_ERROR_INVALID_RESPONSE;
    }

    liRetCode = atoi(loParamter.GetPtr('r'));
    
    aoValue.Resize( loParamter.GetLen('v') + 1 );
    ::memcpy( aoValue.BasePtr(), loParamter.GetPtr('v'), loParamter.GetLen('v') );
    
    return liRetCode;
}

int clsMemCacheClientImpl::Del( const char* apcKey, AIBC::AIChunkEx& aoValue )
{
    int liRetCode = 0;
    AIBC::AIChunkEx loRequest(0);
    AIBC::AIChunkEx loResponse(0);
    cacheserver::AICmdLineParamter loParamter;
    
    cacheserver::AICmdLineParamter::Encode( loRequest, "del k=%s", apcKey );
    
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
        return cacheserver::AI_ERROR_INVALID_RESPONSE;
    }
    
    if ( loParamter.GetPtr('r') == NULL || loParamter.GetPtr('v') == NULL )
    {
        return cacheserver::AI_ERROR_INVALID_RESPONSE;
    }

    liRetCode = atoi(loParamter.GetPtr('r'));
    
    aoValue.Resize( loParamter.GetLen('v') + 1 );
    ::memcpy( aoValue.BasePtr(), loParamter.GetPtr('v'), loParamter.GetLen('v') );
    
    return liRetCode;
}

////////////////////////////////////////////////////////////////////////////
clsMemCacheClient::clsMemCacheClient(void)
{
    AI_NEW_ASSERT(this->mpoImpl, clsMemCacheClientImpl);
}

clsMemCacheClient::~clsMemCacheClient(void)
{
    AI_DELETE(this->mpoImpl);
}

int clsMemCacheClient::Connect( const char* apcServerIp, int aiPort, int aiConnCnt )
{
    return this->mpoImpl->Connect(apcServerIp, aiPort, aiConnCnt);
}

void clsMemCacheClient::Close()
{
    return this->mpoImpl->Close();
}

int clsMemCacheClient::Add( const char* apcKey, const char* apcValue )
{
    return this->mpoImpl->Add(apcKey, apcValue);
}

int clsMemCacheClient::Put( const char* apcKey, const char* apcValue )
{
    return this->mpoImpl->Put(apcKey, apcValue);
}

int clsMemCacheClient::Get( const char* apcKey, AIBC::AIChunkEx& aoValue )
{
    return this->mpoImpl->Get(apcKey, aoValue);
}

int clsMemCacheClient::Del( const char* apcKey, AIBC::AIChunkEx& aoValue )
{
    return this->mpoImpl->Del(apcKey, aoValue);
}
	
