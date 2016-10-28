
#include "SessionHandler.h"

CSessionHandler::CSessionHandler(acl::CReactor& aoReactor, acl::CSockStream const& aoPeer)
    : moReactor(aoReactor)
    , moPeer(aoPeer)
    , moBuffer(4096)
{
}

CSessionHandler::~CSessionHandler(void)
{
}

apl_int_t CSessionHandler::Start(void)
{
    //Register POLLOUT event on dest handle
    if (this->moReactor.RegisterHandler(this->moPeer.GetHandle(), this, acl::IEventHandler::READ_MASK) != 0)
    {
        ACL_ASSERT(false);

        return -1;
    }

    return 0;
}

void CSessionHandler::Close(void)
{
    //Remove all event on dest and sour handle
    this->moReactor.RemoveHandler(this->moPeer.GetHandle(), acl::IEventHandler::ALL_EVENTS_MASK);
}

apl_int_t CSessionHandler::HandleInput( apl_handle_t aiHandle )
{
    bool lbAddWriteEvent = this->moBuffer.GetLength() == 0;

    if (this->moBuffer.GetSpace() == 0)
    {
        this->moReactor.RemoveHandler(this->moPeer.GetHandle(), acl::IEventHandler::READ_MASK);
    }
    else
    {
        apl_int_t liResult = this->moPeer.Recv(this->moBuffer.GetWritePtr(), this->moBuffer.GetSpace() );
        if (liResult > 0)
        {
            this->moBuffer.SetWritePtr(liResult);

            if (lbAddWriteEvent)
            {
                this->moReactor.RegisterHandler(this->moPeer.GetHandle(), this, acl::IEventHandler::WRITE_MASK);
            }
        }
        else
        {
            //Close
            this->Close();
        }
    }

    return 0;
}

apl_int_t CSessionHandler::HandleOutput( apl_handle_t aiHandle )
{
    bool lbAddReadEvent = this->moBuffer.GetSpace() == 0;

    if (this->moBuffer.GetLength() == 0)
    {
        this->moReactor.RemoveHandler(this->moPeer.GetHandle(), acl::IEventHandler::WRITE_MASK);
    }
    else
    {
        apl_ssize_t liResult = this->moPeer.Send(this->moBuffer.GetReadPtr(), this->moBuffer.GetLength() );
        if (liResult > 0)
        {
            if (liResult == (apl_ssize_t)this->moBuffer.GetLength() )
            {
                this->moBuffer.Reset();
            }
            else
            {
                apl_int_t liNewSize = this->moBuffer.GetLength() - liResult;
                apl_memmove(this->moBuffer.GetBase(), this->moBuffer.GetReadPtr() + liResult, liNewSize);
                this->moBuffer.Reset();
                this->moBuffer.SetWritePtr(liNewSize);
            }

            if (lbAddReadEvent)
            {
                this->moReactor.RegisterHandler(this->moPeer.GetHandle(), this, acl::IEventHandler::READ_MASK);
            }
        }
    }

    return 0;
}

apl_int_t CSessionHandler::HandleException( apl_handle_t aiHandle )
{
    apl_errprintf("Channel exception %s\n", apl_strerror(apl_get_errno() ) );

    return 0;
}

apl_int_t CSessionHandler::HandleClose( apl_handle_t aiHandle, apl_int_t aiMask )
{
    if (aiMask == acl::IEventHandler::ALL_EVENTS_MASK)
    {
        apl_errprintf("Connection %"APL_PRIdINT" closed\n", aiHandle);
        apl_close(aiHandle);
        delete this;
    } 

    return 0;
}

