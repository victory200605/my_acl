
#include "ChannelHandler.h"

CChannelHandler::CChannelHandler(acl::CReactor& aoReactor, acl::CSockStream const& aoSour, acl::CSockStream const& aoDest)
    : moReactor(aoReactor)
    , moSour(aoSour)
    , moSourBuffer(4096)
    , moDest(aoDest)
    , moDestBuffer(4096)
{
}

CChannelHandler::~CChannelHandler(void)
{
}

apl_int_t CChannelHandler::Start(void)
{
    //Register POLLOUT event on dest handle
    if (this->moReactor.RegisterHandler(this->moDest.GetHandle(), this, acl::IEventHandler::READ_MASK) != 0)
    {
        ACL_ASSERT(false);

        return -1;
    }

    if (this->moReactor.RegisterHandler(this->moSour.GetHandle(), this, acl::IEventHandler::READ_MASK) != 0)
    {
        ACL_ASSERT(false);

        return -1;
    }

    return 0;
}

void CChannelHandler::Close(void)
{
    //Remove all event on dest and sour handle
    this->moReactor.RemoveHandler(this->moDest.GetHandle(), acl::IEventHandler::ALL_EVENTS_MASK);
    this->moReactor.RemoveHandler(this->moSour.GetHandle(), acl::IEventHandler::ALL_EVENTS_MASK);
}

apl_int_t CChannelHandler::HandleInput( apl_handle_t aiHandle )
{
    if (aiHandle == this->moDest.GetHandle() ) 
    {
        this->Recv(this->moDest, this->moSour, this->moDestBuffer);
    }
    else
    {
        this->Recv(this->moSour, this->moDest, this->moSourBuffer);
    }

    return 0;
}

void CChannelHandler::Recv(acl::CSockStream& aoSour, acl::CSockStream& aoDest, acl::CMemoryBlock& aoBuffer)
{
    bool lbAddWriteEvent = aoBuffer.GetLength() == 0;

    if (aoBuffer.GetSpace() == 0)
    {
        this->moReactor.RemoveHandler(aoSour.GetHandle(), acl::IEventHandler::READ_MASK);
    }
    else
    {
        apl_int_t liResult = aoSour.Recv(aoBuffer.GetWritePtr(), aoBuffer.GetSpace() );
        if (liResult > 0)
        {
            aoBuffer.SetWritePtr(liResult);

            if (lbAddWriteEvent)
            {
                this->moReactor.RegisterHandler(aoDest.GetHandle(), this, acl::IEventHandler::WRITE_MASK);
            }
        }
        else
        {
            //Close
            this->Close();
        }
    }
}

apl_int_t CChannelHandler::HandleOutput( apl_handle_t aiHandle )
{
    if (aiHandle == this->moDest.GetHandle() ) 
    {
        this->Write(this->moDest, this->moSour, this->moSourBuffer);
    }
    else
    {
        this->Write(this->moSour, this->moDest, this->moDestBuffer);
    }

    return 0;
}

void CChannelHandler::Write(acl::CSockStream& aoSour, acl::CSockStream& aoDest, acl::CMemoryBlock& aoBuffer)
{
    bool lbAddReadEvent = aoBuffer.GetSpace() == 0;

    if (aoBuffer.GetLength() == 0)
    {
        this->moReactor.RemoveHandler(aoDest.GetHandle(), acl::IEventHandler::WRITE_MASK);
    }
    else
    {
        apl_ssize_t liResult = aoDest.Send(aoBuffer.GetReadPtr(), aoBuffer.GetLength() );
        if (liResult > 0)
        {
            if (liResult == (apl_ssize_t)aoBuffer.GetLength() )
            {
                aoBuffer.Reset();
            }
            else
            {
                aoBuffer.SetReadPtr(liResult);
            }

            if (lbAddReadEvent)
            {
                this->moReactor.RegisterHandler(aoSour.GetHandle(), this, acl::IEventHandler::READ_MASK);
            }
        }
    }
}
apl_int_t CChannelHandler::HandleException( apl_handle_t aiHandle )
{
    apl_errprintf("Channel exception %s\n", apl_strerror(apl_get_errno() ) );

    return 0;
}

apl_int_t CChannelHandler::HandleClose( apl_handle_t aiHandle, apl_int_t aiMask )
{
    if (aiMask == acl::IEventHandler::ALL_EVENTS_MASK)
    {
        apl_close(aiHandle);
    } 

    return 0;
}

