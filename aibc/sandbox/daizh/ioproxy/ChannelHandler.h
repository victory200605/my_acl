#ifndef IOPROXY_CHANNEL_HANDLER_H
#define IOPROXY_CHANNEL_HANDLER_H

#include "acl/EventHandler.h"
#include "acl/SockStream.h"
#include "acl/Reactor.h"

class CChannelHandler : public acl::IEventHandler
{
public:
    CChannelHandler(acl::CReactor& aoReactor, acl::CSockStream const& aoSour, acl::CSockStream const& aoDest);

    ~CChannelHandler(void);

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

    acl::CSockStream moSour;

    acl::CMemoryBlock moSourBuffer;

    acl::CSockStream moDest;

    acl::CMemoryBlock moDestBuffer;
};

#endif

