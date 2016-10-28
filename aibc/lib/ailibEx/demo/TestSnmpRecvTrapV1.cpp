#include "AISnmp.h"

int main()
{
    char *lpsIP = NULL;
    int liPort = 4444;
    
    stSnmpTrap loSnmpTrap;
    memset(&loSnmpTrap,0,sizeof(loSnmpTrap));

    int ret = SnmpRecvV1Trap(&loSnmpTrap,lpsIP,liPort,5);

    if(ret == 0)
    {
        ShowTrapRecvData(&loSnmpTrap);
    }
    else
    {
        printf("no recv data\n");
    }
    return 0;
}


