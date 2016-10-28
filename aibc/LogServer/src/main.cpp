
//#include "acl/acl.h"
#include "acl/Singleton.h"
#include "acl/GetOpt.h"
#include "aaf/ConfigServer.h"
//#include "aaf/LogServer.h"
#include "aaf/DaemonServer.h"

//#include "Utility.h"

#define CONFIG_FILE "../config/LogServer.ini"


int main( int argc, char* argv[] )
{
#if 0
    acl::Instance<aaf::CDaemonServer>()->SetArg(argc, argv);

    //FIXME, what's the usage of CONFIG_FILE here
    if ( acl::Instance<aaf::CDaemonServer>()->Startup(CONFIG_FILE) != 0 ) 
    {
        return -1;
    }
    
    if ( acl::Instance<aaf::CConfigServer>()->Startup(CONFIG_FILE) != 0 )
    {
        apl_errprintf("Startup config server fail, %s\n", apl_strerror(apl_get_errno() ) );
        return -1;
    }

    aaf::CConfigServer* lpoConfig = acl::Instance<aaf::CConfigServer>();
    //LogServer
    lpoConfig->RegisterValue("IpAddress", "Main", "LogServer",  "IpAddress");
    lpoConfig->RegisterValue("ServerPort", "Main", "LogServer", "ServerPort");
    lpoConfig->RegisterValue("MaxConnection", "Main", "LogServer", "MaxConnection", "1024");
    lpoConfig->RegisterValue("BufferSize", "Main", "LogServer", "ConnBufferSize", "4096");
    lpoConfig->RegisterValue("SHTimeInterval", "Main", "LogServer", "ClientShakehandInterval", "15");
    lpoConfig->RegisterValue("DynamicLib", "Main", "LogServer", "DynamicLib");

    //FIXME
    //FIXME
    //FIXME
    //FIXME
    //FIXME
    //FIXME
    //FIXME
    //FIXME
    //how to get unkonw key=value from [$DynamicLib.so]
        


    if (lpoConfig->Update("Main") != 0)
    {
        apl_errprintf("ConfigServer Load main config fail\n");
        return -1;
    }
    
    
    //Startup Logserver reator
    acl::CReactor loReactor;
    if (loReactor.Initialize( lpoConfig->GetValue("Main.MaxConnection").ToInt() ) != 0)
    {
        apl_errprintf("Reactor initialize fail, %s\n", apl_strerror(apl_get_errno() ) );
        return -1;
    }
    
    //dlopen DynamicLib.so and Get interface ready. 
    
    //Startup server listen for API
    acl::Instance<CScpAcceptor>()->SetReactor(&loReactor);
    if ( acl::Instance<CScpAcceptor>()->Startup(
        lpoConfig->GetValue("Main.IpAddress").ToString(),
        lpoConfig->GetValue("Main.ServerPort").ToInt() ) != 0 )
    {
        apl_errprintf("Startup scp proxy server fail, %s\n", apl_strerror(apl_get_errno() ) );
        return -1;
    }
    
    //Startup ACL logger
    char lacLogFile[128] = {0};
    apl_snprintf(lacLogFile, sizeof(lacLogFile), "%s.log", acl::Instance<aaf::CDaemonServer>()->GetProcessName() );
    acl::Instance<aaf::CLogServer>()->SetFileName(lacLogFile);
    apl_snprintf(lacLogFile, sizeof(lacLogFile), "%s.level", acl::Instance<aaf::CDaemonServer>()->GetProcessName() );
    acl::Instance<aaf::CLogServer>()->SetLevelFileName(lacLogFile);
    acl::Instance<aaf::CLogServer>()->SetLevel(acl::Instance<aaf::CDaemonServer>()->GetLogLevel() );
    acl::Instance<aaf::CLogServer>()->SetReactor(&loReactor);
    if ( acl::Instance<aaf::CLogServer>()->Startup(CONFIG_FILE) != 0 )
    {
        apl_errprintf("Startup log server fail, %s\n", apl_strerror(apl_get_errno() ) );
        return -1;
    }
    
    AAF_LOG_INFO("Startup server success");
    printf("Startup server success\n");
    
    {
        CScpRoutingTableOutput loRoutingTableOutput("scp_route_table.dat");
        AAF_LOG_INFO("scp_route_table");
        AAF_LOG_INFO("-------------------------------------------------------------------------------------");
        AAF_LOG_INFO("|%27s|%27s|%27s|", "start_no", "end_no", "scp_no");
        AAF_LOG_INFO("-------------------------------------------------------------------------------------");
        for (apl_size_t luN = 0; luN < lpoConfig->GetArrayValue("Ldap.RTScpNo").size(); luN++)
        {
            AAF_LOG_INFO(
                "|%27s|%27s|%27s|",
                lpoConfig->GetArrayValue("Ldap.RTStartNo")[luN].ToString(),
                lpoConfig->GetArrayValue("Ldap.RTEndNo")[luN].ToString(),
                lpoConfig->GetArrayValue("Ldap.RTScpNo")[luN].ToString() );
                
            loRoutingTableOutput.Write(
                lpoConfig->GetArrayValue("Ldap.RTStartNo")[luN].ToString(),
                lpoConfig->GetArrayValue("Ldap.RTEndNo")[luN].ToString(),
                lpoConfig->GetArrayValue("Ldap.RTScpNo")[luN].ToString() );
        }
        AAF_LOG_INFO("-------------------------------------------------------------------------------------");
    }
    
    {
        CScpServerTableOutput loServerTableOutput("scp_server_table.dat");
        AAF_LOG_INFO("scp_server_table");
        AAF_LOG_INFO("-------------------------------------------------------------------------------------");
        AAF_LOG_INFO("|%20s|%20s|%20s|%20s|", "scp_no", "server_ip", "server_port", "conn_num");
        AAF_LOG_INFO("-------------------------------------------------------------------------------------");
        for (apl_size_t luN = 0; luN < lpoConfig->GetArrayValue("Ldap.STScpNo").size(); luN++)
        {
            if (lpoConfig->GetArrayValue("Ldap.STAvailable")[luN].ToInt() == 0)
            {
                continue;
            }
            
            AAF_LOG_INFO(
                "|%20s|%20s|%20"APL_PRIdINT"|%20"APL_PRIdINT"|",
                lpoConfig->GetArrayValue("Ldap.STScpNo")[luN].ToString(),
                lpoConfig->GetArrayValue("Ldap.STServerIp")[luN].ToString(),
                lpoConfig->GetArrayValue("Ldap.STServerPort")[luN].ToInt(),
                lpoConfig->GetArrayValue("Ldap.STConnNum")[luN].ToInt() );
            
            loServerTableOutput.Write(
                lpoConfig->GetArrayValue("Ldap.STScpNo")[luN].ToString(),
                lpoConfig->GetArrayValue("Ldap.STServerIp")[luN].ToString(),
                lpoConfig->GetArrayValue("Ldap.STServerPort")[luN].ToInt(),
                lpoConfig->GetArrayValue("Ldap.STConnNum")[luN].ToInt() );
        }
        AAF_LOG_INFO("-------------------------------------------------------------------------------------");
    }
    
    acl::CTimeValue loTimeout(1);
    while( !acl::Instance<aaf::CDaemonServer>()->IsShutdown() )
    {
        if ( loReactor.HandleEvent(loTimeout) < 0 )
        {
            apl_errprintf("Reactor handle event fail,%s\n", apl_strerror(apl_get_errno() ) );
            apl_sleep(APL_TIME_SEC);
        }
    }
    
    AAF_LOG_INFO("Shutdown server now");
    printf("Shutdown server now\n");
    
#endif
    return 0;
}
