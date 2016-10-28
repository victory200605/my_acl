
#include "anf/future/AcceptorOperationFuture.h"

ANF_NAMESPACE_START

CAcceptorOperationFuture::CAcceptorOperationFuture( acl::CSockAddr const& aoLocalAddress )
    : moLocalAddress(aoLocalAddress)
{
}
    
acl::CSockAddr& CAcceptorOperationFuture::GetLocalAddress(void)
{
    return this->moLocalAddress;
}

ANF_NAMESPACE_END
