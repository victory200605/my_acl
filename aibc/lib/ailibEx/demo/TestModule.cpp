#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "AILogSys.h"
#include "AIModule.h"
#include "AIAsyncTcp.h"

int ChildMain( int argc, char *argv[], pid_t atParentPID )
{
	int		liNewSocket;
	int		liClientPort;
	char	lsClientIp[32];
	clsAsyncServer	loServer(NULL,0);
	
	AIWriteLOG( NULL, AILOG_LEVEL_INFO, "%s Startup\n", __FUNCTION__ );
	loServer.StartDaemon( "AsyncListenPort", GetNewConnection, GetListenSocket );
	
	AIWriteLOG( NULL, AILOG_LEVEL_INFO, "%s loServer.StartDaemon Ok\n", __FUNCTION__ );
	while( giGlobalShutDown == 0 )
	{
		if( (liNewSocket = GetNewConnection( "DataListenPort", lsClientIp, &liClientPort )) <= 0 )
		{
			AIWriteLOG( NULL, AILOG_LEVEL_ERROR, "%s GetNewConnection ... fail\n",
				__FUNCTION__ );
			continue;
		}
		
		AIWriteLOG( NULL, AILOG_LEVEL_INFO, "%d Accept %s:%d connected\n", 
			time(NULL), lsClientIp, liClientPort );
		sleep( 10 );
	}
	
	AIWriteLOG( NULL, AILOG_LEVEL_INFO, "%s ShutDown\n", __FUNCTION__ );
	loServer.ShutDown();
	
	return  (0);
}

HANDLE_MAIN_MACRO( ChildMain, "|DataListenPort|AsyncListenPort|" )

/* TestModule.ini
[SERVER]
ModuleID = XCZ01
MaxChildCount = 3
AdminListenPort = 7878
AdminListenIpAddr = 10.3.3.26
AdminListenPassword = ABCDEFG
DataListenPort = 7979
AsyncListenPort = 9090

*/
