
#include "AcceptorHandler.h"
#include "ChannelHandler.h"

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

    acl::CSockStream loSourPeer;

    if (this->moAcceptor.Accept(loSourPeer) != 0)
    {
        apl_errprintf("Acceptor accepte a new connection fail, %s", apl_strerror(apl_get_errno() ) );

        return -1;
    }
    
    if (loSourPeer.Enable(acl::CSockAcceptor::OPT_NONBLOCK) != 0)
    {
        ACL_ASSERT(false);
    }

    //Connector to destination peer
    acl::CSockConnector loConnector;
    acl::CSockAddr loRemote(5555, "127.0.0.1");
    acl::CSockStream loDestPeer;

    if (loConnector.Connect(loDestPeer, loRemote) != 0 
        && apl_get_errno() != APL_EINPROGRESS)
    {
        apl_errprintf("Proxy connect remote address fail, %s", apl_strerror(apl_get_errno() ) );

        loSourPeer.Close();

        return -1;
    }

    if (loDestPeer.Enable(acl::CSockAcceptor::OPT_NONBLOCK) != 0)
    {
        ACL_ASSERT(false);
    }

    CChannelHandler* lpoChannel = new CChannelHandler(this->moReactor, loSourPeer, loDestPeer);

    lpoChannel->Start();

    return 0;
}

