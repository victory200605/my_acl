
#ifndef ANF_ACCEPTOR_OPERATION_FUTURE_H
#define ANF_ACCEPTOR_OPERATION_FUTURE_H

#include "acl/SockAddr.h"
#include "acl/SmartPtr.h"
#include "anf/IoFuture.h"

ANF_NAMESPACE_START

class CAcceptorOperationFuture : public CIoFuture
{
public:
    typedef acl::TSharePtr<CAcceptorOperationFuture, acl::CLock> PointerType;
    
public:
    CAcceptorOperationFuture( acl::CSockAddr const& aoLocalAddress );
    
    acl::CSockAddr& GetLocalAddress(void);

private:
    acl::CSockAddr moLocalAddress;
};

ANF_NAMESPACE_END

#endif//ANF_ACCEPTOR_OPERATION_FUTURE_H
