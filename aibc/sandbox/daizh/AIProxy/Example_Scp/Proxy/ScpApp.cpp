
#include "../../AIProxyRouter.h"
#include "AIConfig.h"
#include "ScpServer.h"
#include "ScpProtocol.h"
#include "ScpRouting.h"

using namespace AIPROXY;

bool gbIsShutdown = false;

void Exit( int aiSignal )
{
    clsScpInnerServer* lpoInnerServer = GetInnerServer<clsScpInnerServer*>();
    lpoInnerServer->Close();
    gbIsShutdown = true;
}

int main( int agrc, char* agrv[] )
{
    signal( SIGINT, Exit );
    signal( SIGTERM, Exit );
    signal( SIGQUIT, Exit );
    
    AIInitIniHandler();
    
    ///create outer server
    clsScpOuterServer* lpoOuterServer = new clsScpOuterServer( new clsScpOuterProtocol );
    AIRouter::Instance()->OuterServer(lpoOuterServer);
    if ( lpoOuterServer->Initialize( "../config/scpproxy.ini" ) != 0 )
    {
        return -1;
    }
    
	///create inner server
    clsScpInnerServer* lpoInnerServer = new clsScpInnerServer( new clsScpInnerProtocol );
    AIRouter::Instance()->InnerServer(lpoInnerServer);
    if ( lpoInnerServer->Initialize( "../config/scpproxy.ini" ) != 0 )
    {
        return -1;
    }
    
    ///create inner/outer routing
    AIRouting* lpoInnerRouting = new AIDefaultInnerRouting;
    AIRouting* lpoOuterRouting = new clsOuterRouting;
    
    /// set all 
    AIRouter::Instance()->InnerRouting(lpoInnerRouting);
    AIRouter::Instance()->OuterRouting(lpoOuterRouting);
    
    while( !gbIsShutdown )
    {
        lpoOuterServer->Initialize( "../config/scpproxy.ini" );
        sleep(10);
    }
}
