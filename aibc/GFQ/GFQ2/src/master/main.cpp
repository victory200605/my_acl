
#include "aaf/LogService.h"
#include "aaf/DaemonService.h"
#include "acl/Singleton.h"
#include "anf/IoSockAcceptor.h"
#include "anf/filter/BencodingMessageFilter.h"
#include "anf/protocol/BencodingMessage.h"
#include "MasterConfig.h"
#include "ServerHandler.h"
#include "EventObserver.h"
#include "QueuePoll.h"
#include "KeepAlived.h"

using namespace gfq2;

std::string goDefaultConfigCommand("default:../config/Master.ini");

bool Helper(int argc, char* argv[] )
{
    if (argc == 3 
        && apl_strcmp(argv[1], "?") == 0)
    {
        apl_int_t liErrno = apl_strtoi32(argv[2], APL_NULL, 10);

        if (liErrno > 0)
        {
            liErrno = -1 * liErrno;
        }

        printf("GFQ-MessageCode(%"APL_PRIdINT"):%s\n", liErrno, StrError(liErrno) );

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

    acl::Instance<CKeepAlived>()->SetOption('f', true, "config", "config cmd, value format is <so_path:args> and default val=default:file_path");
    acl::Instance<CKeepAlived>()->SetOption('a', false, "keepalived", "enable keepalived");
    acl::Instance<CKeepAlived>()->SetOption('h', false, "help", "usage");
    acl::Instance<CKeepAlived>()->SetArg(argc, argv);

    if (acl::Instance<CKeepAlived>()->IsHasOption('h') )
    {
        acl::Instance<CKeepAlived>()->PrintUsage();
        return 0;
    }

    if (acl::Instance<CKeepAlived>()->IsHasOption('f') )
    {
        goDefaultConfigCommand =  acl::Instance<CKeepAlived>()->GetOptArg('f');
    }

    if ( acl::Instance<CKeepAlived>()->Startup(
        acl::Instance<CKeepAlived>()->IsHasOption('a'),
        goDefaultConfigCommand.c_str() ) != 0 )
    {
        return -1;
    }

    //Load config
    CMasterConfig* lpoConfig = acl::Instance<CMasterConfig>();
    if (lpoConfig->Initialize(goDefaultConfigCommand.c_str() ) != 0)
    {
        apl_errprintf("Load config from local file fail %s\n", (apl_get_errno() != 0 ? apl_strerror(apl_get_errno() ) : "" ) );
        return -1;
    }
    
    //Startup log server
    acl::Instance<aaf::CLogService>()->SetLevel(acl::Instance<CKeepAlived>()->GetLogLevel() );
    acl::Instance<aaf::CLogService>()->SetDefaultName(lpoConfig->moDefaultName.c_str() );
    acl::Instance<aaf::CLogService>()->SetLevelFileName(lpoConfig->moLevelFileName.c_str() );
    acl::Instance<aaf::CLogService>()->SetDefaultBufferSize(lpoConfig->miDefaultBufferSize);
    acl::Instance<aaf::CLogService>()->SetDefaultBackupSize(lpoConfig->miDefaultBackupSize);
    acl::Instance<aaf::CLogService>()->SetDefaultBackupTimeInterval(lpoConfig->miDefaultBackupTimeInterval);
    for (CMasterConfig::LogHandlerTableType::iterator loIter = lpoConfig->moLogHandlers.begin();
         loIter != lpoConfig->moLogHandlers.end(); ++loIter)
    {
        apl_int_t liTriggerType = aaf::CLogService::TRIGGER_SELF;
        for (std::string::iterator loCh = loIter->moTriggerType.begin(); loCh != loIter->moTriggerType.end(); ++loCh)
        {
            switch(*loCh)
            {
                case 'b': ACL_SET_BITS(liTriggerType, aaf::CLogService::TRIGGER_ALL);break; 
                case 'a': ACL_SET_BITS(liTriggerType, aaf::CLogService::TRIGGER_ANY);break; 
            };
        }

        if ( acl::Instance<aaf::CLogService>()->AddHandler(
            loIter->moName.c_str(),
            loIter->moType.c_str(),
            loIter->miTriggerLevel,
            liTriggerType,
            loIter->miBufferSize,
            loIter->miBackupSize,
            loIter->miBackupTimeInterval,
            loIter->moBackupDir.c_str() ) != 0)
        {
            apl_errprintf("LogService add handler fail, (Name=%s,Type=%s)\n", loIter->moName.c_str(), loIter->moType.c_str() );
            return -1;
        }
    }
    if ( acl::Instance<aaf::CLogService>()->Startup(NULL) != 0 )
    {
        printf("Startup log server fail, %s\n", apl_strerror(apl_get_errno() ) );
        return -1;
    }
    
    //Startup event observer
    if ( acl::Instance<CEventObserver>()->Initialize() != 0 )
    {
        printf("Startup event observer fail, %s\n", apl_strerror(apl_get_errno() ) );
        return -1;
    }
    
    //Startup queue poll
    if ( acl::Instance<CQueuePoll>()->Initialize() != 0 )
    {
        printf("Startup queue poll fail, %s\n", apl_strerror(apl_get_errno() ) );
        return -1;
    }

    //Startup timer queue adapter
    if (acl::Instance<acl::CTimerQueueAdapter>()->Activate() != 0)
    {
        printf("Timer queue adapter activate fail, %s\n", apl_strerror(apl_get_errno() ) );
        return -1;
    }
    
    GetServerNodeManager()->SetAllocateWeighting(lpoConfig->miAllocateWeighting);
    
    AAF_LOG_INFO("============================== start up master server  ===================================");
    //Startup master server
    {
        CServerHandler* lpoHandler = NULL;
        ACL_NEW_ASSERT(lpoHandler, CServerHandler);

        acl::Instance<anf::CIoSockAcceptor>()->GetFilterChain()->AddFirst("becoding-filter", new anf::CBencodingMessageFilter);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetProcessorPoolSize(lpoConfig->miAcceptorPPSize);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetReadBufferSize(lpoConfig->miReadBufferSize);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetWriteBufferSize(lpoConfig->miWriteBufferSize);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetReadIdleTime(lpoConfig->miConnectionIdleTimedout);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetMaxListenerCount(128);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetMaxConnectionCount(lpoConfig->miMaxConnectionCount);
        acl::Instance<anf::CIoSockAcceptor>()->SetHandler(lpoHandler);

        acl::CSockAddr loLocalAddress(lpoConfig->miLocalPort, lpoConfig->moLocalAddress.c_str() );

        if ( acl::Instance<anf::CIoSockAcceptor>()->Bind(loLocalAddress) != 0)
        {
            printf("Startup server fail, %s\n", apl_strerror(apl_get_errno() ) );
            return -1;
        }
    }
    
    AAF_LOG_INFO("Startup server success");
    printf("Startup server success\n");

    lpoConfig->PrintAll();
    
    while( !acl::Instance<aaf::CDaemonService>()->IsShutdown() )
    {
        apl_sleep(APL_TIME_SEC);
    }
    
    AAF_LOG_INFO("Shutdown server now");
    printf("Shutdown server now\n");
    
    acl::Instance<acl::CTimerQueueAdapter>()->Deactivate();
    acl::Instance<anf::CIoSockAcceptor>()->Dispose();
    acl::Instance<CQueuePoll>()->Close();
    acl::Instance<CEventObserver>()->Close();
    acl::Instance<aaf::CLogService>()->Shutdown();
    acl::Instance<CMasterConfig>()->Close();
    acl::Instance<CKeepAlived>()->Shutdown();

    return 0;
}

