
#include "anf/future/ConnectFuture.h"
#include "anf/IoSession.h"

ANF_NAMESPACE_START

CConnectFuture::CConnectFuture( acl::CSockAddr const& aoRemoteAddress )
    : moRemoteAddress(aoRemoteAddress)
{
}

void CConnectFuture::SetSession( SessionPtrType& aoSession )
{
    this->moSession = aoSession;
}

SessionPtrType& CConnectFuture::GetSession(void)
{
    return this->moSession; 
}
    
acl::CSockAddr& CConnectFuture::GetRemoteAddress(void)
{
    return this->moRemoteAddress;
}

ANF_NAMESPACE_END
