#ifndef IOPROXY_CHANNEL_HANDLER_H
#define IOPROXY_CHANNEL_HANDLER_H

#include "acl/EventHandler.h"
#include "acl/SockStream.h"
#include "acl/Reactor.h"

class CSessionHandler : public acl::IEventHandler
{
public:
    CSessionHandler(acl::CReactor& aoReactor, acl::CSockStream const& aoPeer);

    ~CSessionHandler(void);

    apl_int_t Start(void);

    void Close(void);

public:
    apl_int_t HandleInput( apl_handle_t aiHandle );

    apl_int_t HandleOutput( apl_handle_t aiHandle );

    apl_int_t HandleException( apl_handle_t aiHandle );

    apl_int_t HandleClose( apl_handle_t aiHandle, apl_int_t aiMask );

private:
    void Recv(acl::CSockStream& aoSour, acl::CSockStream& aoDest, acl::CMemoryBlock& aoBuffer);

    void Write(acl::CSockStream& aoSour, acl::CSockStream& aoDest, acl::CMemoryBlock& aoBuffer);

private:
    acl::CReactor& moReactor;

    acl::CSockStream moPeer;

    acl::CMemoryBlock moBuffer;
};

#endif

