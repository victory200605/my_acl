#include "apl/apl.h"
#include "anf/IoSockAcceptor.h"
#include "anf/IoHandler.h"
#include "anf/IoSession.h"
#include "anf/SimpleObjectPool.h"
#include "acl/MemoryBlock.h"
#include "acl/SockAddr.h"
#include "acl/GetOpt.h"

class CEchoHandler : public anf::IoHandler
{
public:
    virtual void Exception( anf::CThrowable const& aoCause )
    {
        apl_errprintf("Echo server exception - %s\n", aoCause.GetMessage() );
    }
    
    virtual void SessionException( anf::SessionPtrType& aoSession, anf::CThrowable const& aoCause )
    {
        apl_errprintf("Session exception - %s\n", aoCause.GetMessage() );
    }

    virtual void SessionOpened( anf::SessionPtrType& aoSession )
    {
        apl_errprintf(
            "Create a new session [%s:%"APL_PRIdINT"]\n",
            aoSession->GetRemoteAddress().GetIpAddr(),
            (apl_int_t)aoSession->GetRemoteAddress().GetPort() );
    }

    virtual void SessionClosed( anf::SessionPtrType& aoSession )
    {
        apl_errprintf(
            "Close session [%s:%"APL_PRIdINT"]\n",
            aoSession->GetRemoteAddress().GetIpAddr(),
            (apl_int_t)aoSession->GetRemoteAddress().GetPort() );
    }

    virtual void SessionIdle( anf::SessionPtrType& aoSession, apl_int_t aiStatus )
    {
        switch(aiStatus)
        {
            case anf::SESSION_READ_IDLE:
                apl_errprintf("Session read idle\n");
                break;
            case anf::SESSION_WRITE_IDLE:
                apl_errprintf("Session write idle\n");
                break;
        };
    }

    virtual void MessageReceived( anf::SessionPtrType& aoSession, acl::CAny const& aoMessage )
    {
        acl::CMemoryBlock* lpoRecved = NULL;

        ACL_ASSERT(aoMessage.CastTo<acl::CMemoryBlock*>(lpoRecved) );

        //send back
        acl::CMemoryBlock* lpoSending = anf::SimpleObjectPoolInstance<acl::CMemoryBlock>()->Allocate();
        lpoSending->Resize(lpoRecved->GetLength() );
        lpoSending->Reset();

        apl_memcpy(lpoSending->GetWritePtr(), lpoRecved->GetReadPtr(), lpoRecved->GetLength() );
        lpoSending->SetWritePtr(lpoRecved->GetLength() );
        lpoRecved->Reset();

        aoSession->Write(aoSession, lpoSending);
    }

    virtual void MessageSent( anf::SessionPtrType& aoSession, anf::CWriteFuture::PointerType& aoWriteFuture ) {};
};

int main( int argc, char* argv[] )
{
    apl_int_t liOptChar;
    apl_int16_t li16Port = 5555;
    apl_size_t  liReadBufferSize = 1024;
    apl_size_t  liWriteBufferSize = 1024;
    apl_size_t  liProcessorPoolSize = 2;
    char lacIpAddress[12] = "127.0.0.1";
    acl::CGetOpt loOpt(argc, argv, "i:p:r:w:t:");
    while( (liOptChar = loOpt() ) != EOF )
    {
        switch (liOptChar)
        {
            case 'i':
                apl_strncpy(lacIpAddress, loOpt.OptArg(), sizeof(lacIpAddress) );
                break;
            case 'p':
                li16Port = apl_strtoi32(loOpt.OptArg(), NULL, 10);
                break;
            case 'r':
                liReadBufferSize = apl_strtoi32(loOpt.OptArg(), NULL, 10);
                break;
            case 'w':
                liWriteBufferSize = apl_strtoi32(loOpt.OptArg(), NULL, 10);
                break;
            case 't':
                liProcessorPoolSize = apl_strtoi32(loOpt.OptArg(), NULL, 10);
                break;
        }
    }

    apl_signal(APL_SIGPIPE, APL_SIG_IGN); 

    acl::CSockAddr loAddr(li16Port, lacIpAddress);
    anf::CIoSockAcceptor loAcceptor;

    loAcceptor.GetConfiguration()->SetReadBufferSize(liReadBufferSize);
    loAcceptor.GetConfiguration()->SetWriteBufferSize(liWriteBufferSize);
    loAcceptor.GetConfiguration()->SetProcessorPoolSize(liProcessorPoolSize);
    loAcceptor.SetHandler(new CEchoHandler);

    //service startup
    if ( loAcceptor.Bind(loAddr) != 0)
    {
        apl_errprintf("Echo server bind address [%s:%"APL_PRId16"] fail\n", lacIpAddress, li16Port);
        return -1;
    }

    apl_errprintf("Echo server startup [OK]\npress any key to exit\n");

    getchar();

    loAcceptor.Dispose();

    return 0;
}
