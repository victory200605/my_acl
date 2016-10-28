
#include "AcceptorHandler.h"
//#include "ChannelHandler.h"

int main(int argc, char* argv[] )
{
    acl::CReactor loReactor;

    if (loReactor.Initialize(256) != 0)
    {
        apl_errprintf("Initialize reactor fail %s\n", apl_strerror(apl_get_errno() ) );

        return -1;
    }

    CAcceptorHandler loAcceptor(loReactor);

    if (loAcceptor.Start(acl::CSockAddr(5555, "127.0.0.1") ) != 0)
    {
        apl_errprintf("Start acceptor fail %s\n", apl_strerror(apl_get_errno() ) );

        return -1;
    }

    //loReactor.RegisterHandler(0, &loAcceptor, APL_POLLOUT);
    //loReactor.RegisterHandler(1, &loAcceptor, APL_POLLOUT);
    //loReactor.RegisterHandler(3, &loAcceptor, APL_POLLOUT);

    while(true)
    {
        loReactor.HandleEvent(1);
    }

    return 0;
}
