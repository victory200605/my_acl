
#include "aaf/LogService.h"
#include "aaf/DaemonService.h"
#include "acl/Singleton.h"
#include "acl/TimerQueueAdapter.h"
#include "anf/IoSockAcceptor.h"
#include "anf/IoSockConnector.h"
#include "anf/filter/BencodingMessageFilter.h"
#include "anf/protocol/BencodingMessage.h"
#include "gfq2/Utility.h"
#include "MasterConnectManager.h"
#include "ServerConfig.h"
#include "ClientHandler.h"
#include "ServerHandler.h"
#include "SegmentQueue.h"
#include "vlr/VLRSegmentQueue.h"
#include "flr/FLRSegmentQueue.h"

using namespace gfq2;

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

ISegmentQueue* SegmentQueueFactory( CServerConfig* apoConfig )
{
    ISegmentQueue* lpoSegmentQueue = APL_NULL;

    if (apoConfig->moStorageType == "flr")
    {
        ACL_NEW_ASSERT(lpoSegmentQueue,
            CFLRSegmentQueue(apoConfig->moStoragePath.c_str() ) );
    }
    else
    {
        char* lpcNext = APL_NULL;
        apl_int_t liPoolSize = apl_strtoi32(apoConfig->moStorageBufferPoolSize.c_str(), &lpcNext, 10);

        //Parse buffer pool size
        if (lpcNext != APL_NULL && *lpcNext == 'k')
        {
            liPoolSize *= 1024;
        }
        else if (lpcNext != APL_NULL && *lpcNext == 'm')
        {
            liPoolSize *= (1024 * 1024);
        }
        
        ACL_NEW_ASSERT(
            lpoSegmentQueue,
            CVLRSegmentQueue(
                apoConfig->moStoragePath.c_str(),
                apoConfig->moStorageBufferPoolType.c_str(), 
                liPoolSize)
        );
    }
    
    return lpoSegmentQueue;
}

int main( int argc, char* argv[] )
{
    //helper
    if (Helper(argc, argv) )
    {
        return 0;
    }

    apl_int_t liRetCode = 0;
    ISegmentQueue* lpoSegmentQueue = APL_NULL;
    acl::Instance<aaf::CDaemonService>()->SetOption('f', true, "config", "config cmd, value format is <so_path:args> and default val=default:file_path");
    acl::Instance<aaf::CDaemonService>()->SetOption('h', false, "help", "usage");
    acl::Instance<aaf::CDaemonService>()->SetArg(argc, argv);

    if ( acl::Instance<aaf::CDaemonService>()->IsHasOption('h') )
    {
        acl::Instance<aaf::CDaemonService>()->PrintUsage();
        return 0;
    }

    if ( acl::Instance<aaf::CDaemonService>()->Startup(NULL) != 0 )
    {
        return -1;
    }

    //Load config
    CServerConfig* lpoConfig = acl::Instance<CServerConfig>();
    if (acl::Instance<aaf::CDaemonService>()->IsHasOption('f') )
    {
        liRetCode = lpoConfig->Initialize(acl::Instance<aaf::CDaemonService>()->GetOptArg('f') );
    }
    else
    {
        liRetCode = lpoConfig->Initialize("default:../config/SegmentServer.ini");
    }
    
    if (liRetCode != 0)
    {
        apl_errprintf("Load config from local file fail %s\n", (apl_get_errno() != 0 ? apl_strerror(apl_get_errno() ) : "" ) );
        return -1;
    }
    
    lpoSegmentQueue = SegmentQueueFactory(lpoConfig);

    if (lpoSegmentQueue->Open() != 0)
    {
        printf("Initialize segment storage fail\n");

        return -1;
    }

    if (lpoSegmentQueue->Check() != 0)
    {
        printf("Segment storage check fail\n");

        return -1;
    }

    //Startup log server
    acl::Instance<aaf::CLogService>()->SetLevel(acl::Instance<aaf::CDaemonService>()->GetLogLevel() );
    acl::Instance<aaf::CLogService>()->SetDefaultName(lpoConfig->moDefaultName.c_str() );
    acl::Instance<aaf::CLogService>()->SetLevelFileName(lpoConfig->moLevelFileName.c_str() );
    acl::Instance<aaf::CLogService>()->SetDefaultBufferSize(lpoConfig->miDefaultBufferSize);
    acl::Instance<aaf::CLogService>()->SetDefaultBackupSize(lpoConfig->miDefaultBackupSize);
    acl::Instance<aaf::CLogService>()->SetDefaultBackupTimeInterval(lpoConfig->miDefaultBackupTimeInterval);
    for (CServerConfig::LogHandlerTableType::iterator loIter = lpoConfig->moLogHandlers.begin();
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
    
    AAF_LOG_INFO("============================== start up segment server  ===================================");

    CClientHandler* lpoClientHandler = NULL;
    CServerHandler* lpoServerHandler = NULL;
    
    //Startup segment server
    {
        ACL_NEW_ASSERT(lpoServerHandler, CServerHandler(lpoSegmentQueue) );

        acl::Instance<anf::CIoSockAcceptor>()->GetFilterChain()->AddFirst("becoding-filter", new anf::CBencodingMessageFilter);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetProcessorPoolSize(lpoConfig->miAcceptorPPSize);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetReadBufferSize(lpoConfig->miReadBufferSize);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetWriteBufferSize(lpoConfig->miWriteBufferSize);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetReadIdleTime(lpoConfig->miConnectionIdleTimedout);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetMaxListenerCount(128);
        acl::Instance<anf::CIoSockAcceptor>()->GetConfiguration()->SetMaxConnectionCount(lpoConfig->miMaxConnectionCount);
        acl::Instance<anf::CIoSockAcceptor>()->SetHandler(lpoServerHandler);

        acl::CSockAddr loLocalAddress(lpoConfig->miLocalPort, lpoConfig->moLocalAddress.c_str() );

        if ( acl::Instance<anf::CIoSockAcceptor>()->Bind(loLocalAddress) != 0)
        {
            printf("Startup segment server fail, %s\n", apl_strerror(apl_get_errno() ) );
            return -1;
        }
    }

    if (acl::Instance<acl::CTimerQueueAdapter>()->Activate() != 0)
    {
        printf("Timer queue adapter activate fail, %s\n", apl_strerror(apl_get_errno() ) );
        return -1;
    }
    
    //Connect to master
    {
        ACL_NEW_ASSERT(lpoClientHandler, 
            CClientHandler(
                lpoConfig->moNodeName.c_str(), 
                lpoConfig->moNodeGroup.c_str(),
                lpoConfig->moStorageType.c_str(),
                lpoConfig->moLocalAddress.c_str(), 
                lpoConfig->miLocalPort, 
                lpoConfig->miNodePriority,
                lpoConfig->miSegmentInfoSyncInterval,
                lpoConfig->miStatInfoSyncInterval,
                lpoSegmentQueue) );

        acl::Instance<anf::CIoSockConnector>()->SetConnectTimeout(5);
        acl::Instance<anf::CIoSockConnector>()->GetFilterChain()->AddFirst("benc-filter", new anf::CBencodingMessageFilter);
        acl::Instance<anf::CIoSockConnector>()->GetConfiguration()->SetProcessorPoolSize(lpoConfig->miConnectorPPSize);
        acl::Instance<anf::CIoSockConnector>()->GetConfiguration()->SetReadBufferSize(lpoConfig->miReadBufferSize);
        acl::Instance<anf::CIoSockConnector>()->GetConfiguration()->SetWriteBufferSize(lpoConfig->miWriteBufferSize);
        acl::Instance<anf::CIoSockConnector>()->GetConfiguration()->SetWriteIdleTime(lpoConfig->miHeartbeatInterval);
        acl::Instance<anf::CIoSockConnector>()->GetConfiguration()->SetMaxProgressConnectionCount(128);
        acl::Instance<anf::CIoSockConnector>()->GetConfiguration()->SetMaxConnectionCount(128);
        acl::Instance<anf::CIoSockConnector>()->SetHandler(lpoClientHandler);

        if (acl::Instance<CMasterConnectManager>()->Initialize(acl::Instance<anf::CIoSockConnector>(), 1) != 0)
        {
            printf("Initialize connect manager fail, %s\n", apl_strerror(apl_get_errno() ) );
            return -1;
        }

        acl::CSockAddr loRemoteAddress(lpoConfig->miMasterPort, lpoConfig->moMasterAddress.c_str() );

        acl::Instance<CMasterConnectManager>()->Connect(loRemoteAddress);
    }
    
    AAF_LOG_INFO("Startup server success");
    printf("Startup server success\n");

    //Print all server info
    {
        lpoSegmentQueue->Dump();
        lpoConfig->PrintAll();
    }
    
    while( !acl::Instance<aaf::CDaemonService>()->IsShutdown() )
    {
        apl_sleep(APL_TIME_SEC);
    }
    
    AAF_LOG_INFO("Shutdown server now");
    printf("Shutdown server now\n");

    acl::Instance<anf::CConnectManager>()->Close();
    acl::Instance<anf::CIoSockConnector>()->Dispose();
    acl::Instance<anf::CIoSockAcceptor>()->Dispose();
    acl::Instance<aaf::CLogService>()->Shutdown();
    
    lpoSegmentQueue->Close();
    ACL_DELETE(lpoSegmentQueue);
    
    return 0;
}

