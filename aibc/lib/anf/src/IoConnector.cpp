
#include "anf/IoConnector.h"

ANF_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////////////////////
CIoConnector::CIoConnector(void)
    : moConnectTimeout(10)
{
}
    
CIoConnector::~CIoConnector(void)
{
}

void CIoConnector::SetConnectTimeout( acl::CTimeValue const& aoTimeout )
{
    this->moConnectTimeout = aoTimeout;
}
        
acl::CTimeValue& CIoConnector::GetConnectTimeout(void)
{
    return this->moConnectTimeout;
}

void CIoConnector::SetDefaultRemoteAddress( acl::CSockAddr const& aoRemoteAddress )
{
    this->moRemoteAddress.Set(aoRemoteAddress);
}
    
void CIoConnector::GetDefaultRemoteAddress( acl::CSockAddr& aoRemoteAddress )
{
    aoRemoteAddress.Set(this->moRemoteAddress);
}

CConnectFuture::PointerType CIoConnector::Connect(void)
{
    return this->Connect(this->moRemoteAddress);
}

CConnectFuture::PointerType CIoConnector::Connect( acl::CSockAddr const& aoAddress )
{
    return this->Connect0(aoAddress);
}

ANF_NAMESPACE_END
