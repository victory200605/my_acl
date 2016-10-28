
#ifndef ANF_CONNECT_FUTURE_H
#define ANF_CONNECT_FUTURE_H

#include "acl/SmartPtr.h"
#include "acl/SockAddr.h"
#include "anf/IoFuture.h"

ANF_NAMESPACE_START

class CConnectFuture : public CIoFuture
{
public:
    typedef acl::TSharePtr<CConnectFuture, acl::CLock> PointerType;

public:
    CConnectFuture( acl::CSockAddr const& aoRemoteAddress );
    
    void SetSession( SessionPtrType& aoSession );
    
    SessionPtrType& GetSession(void);
    
    acl::CSockAddr& GetRemoteAddress(void);

private:
    acl::CSockAddr moRemoteAddress;
    
    SessionPtrType moSession;
};

ANF_NAMESPACE_END

#endif//ANF_CONNECT_FUTURE_H
