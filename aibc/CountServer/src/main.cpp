
#include "Module.h"
#include "CountServer.h"
#include "ConfigureServer.h"
#include "LogServer.h"

class CountServerModule : public CModule
{
public:
    virtual apl_int_t Main( apl_int_t argc, char* argv[] )
    {
        apl_int_t     liRetCode = 0;
        acl::CReactor loReactor;
        
        if ( (liRetCode = loReactor.Initialize(1024) ) != 0 )
        {
            apl_errprintf("Initialize Reactor fail %s\n", apl_strerror( apl_get_errno() ) );
            return -1;
        }
        
        char lasLogName[APL_NAME_MAX] = {0};
        apl_snprintf(lasLogName, sizeof(lasLogName), "%s.out", this->GetProcessName() );
        
        CLogServer::Instance()->SetProcessName(this->GetProcessName() );
        CLogServer::Instance()->SetLevel(this->GetLogLevel() );
            
        if ( (liRetCode = CLogServer::Instance()->Startup(loReactor, lasLogName) ) != 0 )
        {
            apl_errprintf("Startup LogServer fail %s\n", apl_strerror(apl_get_errno() ) );
            return -1;
        }
        
        CNTSRV_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>> CountServer <<<<<<<<<<<<<<<<<<<<<<<<<<<");
        
        if ( CConfigureServer::Instance()->Startup(loReactor, "../config/CountServer.ini") != 0 )
        {
            apl_errprintf("Startup ConfigureServer fail\n");
            return -1;
        }
        
        //Set LogServer varibale
        CLogServer::Instance()->SetMaxTime( CConfigureServer::Instance()->GetLogMaxTime() );
        CLogServer::Instance()->SetMaxLogSize( CConfigureServer::Instance()->GetLogMaxSize() );
        CLogServer::Instance()->SetCacheSize( CConfigureServer::Instance()->GetLogCacheSize() );

        //Startup CountServer
        CCountServer::Instance()->SetRequestNumLimit( CConfigureServer::Instance()->GetRequestNumLimit() );
        if ( (liRetCode = CCountServer::Instance()->Startup(
            loReactor,
            CConfigureServer::Instance()->GetIpAddress(),
            CConfigureServer::Instance()->GetServerPort() ) ) != 0 )
        {
            apl_errprintf("Startup CountServer fail\n");
            return -1;
        }

        CNTSRV_LOG_INFO("Start server success");
        printf("Start server success\n");
        
        while( !IsShutdown() )
        {
            if ( loReactor.HandleEvent(1) < 0 )
            {
                CNTSRV_LOG_ERROR_NB( "HandleEvent fail, [errno=%"APL_PRIdINT"]", apl_get_errno() );
                apl_sleep(APL_TIME_SEC);
            }
        }
        
        //Release all
        CCountServer::Release();
        CConfigureServer::Release();
        CLogServer::Release();
        CTable::Release();
        loReactor.Close();
        
        CNTSRV_LOG_INFO("Exit server success");
        printf("Exit server success\n");
        
        return 0;
    }
};

RUN_APP(CountServerModule);
