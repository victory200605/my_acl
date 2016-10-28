
#include "AcceptorHandler.h"
#include "SessionHandler.h"

#include "acl/SockConnector.h"

CAcceptorHandler::CAcceptorHandler(acl::CReactor& aoReactor)
    : moReactor(aoReactor)
{
}

CAcceptorHandler::~CAcceptorHandler(void)
{
}

apl_int_t CAcceptorHandler::Start(acl::CSockAddr const& aoLocal)
{
    //Open and bind local sock address
    if (this->moAcceptor.Open(aoLocal) != 0)
    {
        return -1;
    }

    if (this->moAcceptor.Enable(acl::CSockAcceptor::OPT_NONBLOCK) != 0)
    {
        ACL_ASSERT(false);
    }

    //Register POLLIN event on acceptor handle
    if (this->moReactor.RegisterHandler(this->moAcceptor.GetHandle(), this, acl::IEventHandler::READ_MASK) != 0)
    {
        this->Close();

        return -1;
    }

    return 0;
}

void CAcceptorHandler::Close(void)
{
    this->moAcceptor.Close();
}

apl_int_t CAcceptorHandler::HandleInput( apl_handle_t aiHandle )
{
    ACL_ASSERT(aiHandle == this->moAcceptor.GetHandle() );

    acl::CSockStream loPeer;

    if (this->moAcceptor.Accept(loPeer) != 0)
    {
        apl_errprintf("Acceptor accept a new connection fail, %s", apl_strerror(apl_get_errno() ) );

        return -1;
    }
    else
    {
        apl_errprintf("Accept a new connection %"APL_PRIdINT"\n", loPeer.GetHandle() );
    }

    if (loPeer.Enable(acl::CSockAcceptor::OPT_NONBLOCK) != 0)
    {
        ACL_ASSERT(false);
    }

    CSessionHandler* lpoSession = new CSessionHandler(this->moReactor, loPeer);

    lpoSession->Start();

    return 0;
}

