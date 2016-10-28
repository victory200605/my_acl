
#include "aaf/DaemonService.h"
#include "acl/Singleton.h"
#include "anf/IoSockAcceptor.h"
#include "anf/IoSockConnector.h"
#include "CmdLineMessageFilter.h"
#include "CmdLineMessage.h"
#include "Utility.h"
#include "ServerHandler.h"
#include "ServerConfig.h"
#include "TaskProcessor.h"

bool Helper(int argc, char* argv[] )
{
    if (argc == 3 
        && apl_strcmp(argv[1], "?") == 0)
    {
        apl_int_t liErrno = apl_strtoi32(argv[2], APL_NULL, 10);

        printf("MessageCode(%"APL_PRIdINT"):%s\n", liErrno, StrError(liErrno) );

        return true;
    }
    else
    {
        return false;
    }
}

int main( int argc, char* argv[] )
{
    //helper
    if (Helper(argc, argv) )
    {
        return 0;
    }

    acl::Instance<aaf::CDaemonService>()->SetOption('f', true, "config", "config file");
    acl::Instance<aaf::CDaemonService>()->SetArg(argc, argv);

    if ( acl::Instance<aaf::CDaemonService>()->Startup(NULL) != 0 )
    {
        return -1;
    }
    
    if (acl::Instance<aaf::CDaemonService>()->IsHasOption('f') )
    {
        SetConfigFilePath(acl::Instance<aaf::CDaemonService>()->GetOptArg('f') );
    }

    //Load config
    CServerConfig* lpoConfig = acl::Instance<CServerConfig>();

    if (lpoConfig->HandleUpdate() != 0)
    {
        apl_errprintf("Load config info fail, %s\n", apl_strerror(apl_get_errno() ) );
        return -1;
    }
    
    EMP_LOG_INFO("%s", "============================== start up emproxy server  ===================================");

    lpoConfig->PrintAll();

    {//Init processor
        for (CServerConfig::ProcessorListType::iterator loIter = lpoConfig->moPOPProcessores.begin();
            loIter != lpoConfig->moPOPProcessores.end(); ++loIter)
        {
            if (loIter->moHostName == "default")
            {
                acl::Instance<CTaskProcessor>()->SetDefaultProcessor(loIter->miQueueSize, loIter->miThreadCount);
            }
            else
            {
                acl::Instance<CTaskProcessor>()->AddProcessor(
                    loIter->moHostName.c_str(),
                    loIter->miQueueSize,
                    loIter->miThreadCount );
            }
        }
    }
    
    {//Startup segment server
        CServerHandler* lpoServerHandler = NULL;
        ACL_NEW_ASSERT(lpoServerHandler, CServerHandler);

        acl::Instance<anf::CIoSockAcceptor>()->GetFilterChain()->AddFirst("cmd-line-filter", new CCmdLineMessageFilter);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetProcessorPoolSize(lpoConfig->miAcceptorProcessorPoolSize);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetReadBufferSize(lpoConfig->miReadBufferSize);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetWriteBufferSize(lpoConfig->miWriteBufferSize);
        acl::Instance<anf::CIoSockAcceptor>()->SetHandler(lpoServerHandler);

        acl::CSockAddr loLocalAddress(lpoConfig->miLocalPort, lpoConfig->moLocalAddress.c_str() );

        if ( acl::Instance<anf::CIoSockAcceptor>()->Bind(loLocalAddress) != 0)
        {
            printf("Startup segment server fail, %s\n", apl_strerror(apl_get_errno() ) );
            return -1;
        }
    }

    EMP_LOG_INFO("%s", "Startup server success");
    printf("Startup server success\n");

    while( !acl::Instance<aaf::CDaemonService>()->IsShutdown() )
    {
        apl_sleep(APL_TIME_SEC);
    }
    
    EMP_LOG_INFO("%s", "Shutdown server now");
    printf("Shutdown server now\n");
    
    acl::Instance<anf::CIoSockAcceptor>()->Dispose();

    return 0;
}

