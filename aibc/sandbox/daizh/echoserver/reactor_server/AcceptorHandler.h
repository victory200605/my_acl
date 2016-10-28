#ifndef IOPROXY_ACCEPTOR_HANDLER_H
#define IOPROXY_ACCEPTOR_HANDLER_H

#include "acl/EventHandler.h"
#include "acl/Reactor.h"
#include "acl/SockAddr.h"
#include "acl/SockAcceptor.h"

class CAcceptorHandler : public acl::IEventHandler 
{
public:
    CAcceptorHandler(acl::CReactor& aoReactor);

    ~CAcceptorHandler(void);

    apl_int_t Start(acl::CSockAddr const& aoLocal);

    void Close(void);

public:
    apl_int_t HandleInput( apl_handle_t aiHandle );

private:
    acl::CReactor& moReactor;

    acl::CSockAcceptor moAcceptor;
};

#endif

