#include "AISnmp.h"
#include "AIAgent.h"
#include "ISMG_snmp_mib.h"

int Test001(AIAgent * apoAgent)
{
    MIB_Initialize(apoAgent);

    apoAgent->RefreshSnmpDataTable();
    apoAgent->ShowDataINSnmpDataTable();

    char *lpcLocalIp="127.0.0.1";
    int liLocalPort=4444;

    apoAgent->StartUp(lpcLocalIp, liLocalPort, 5);

    while(true)
    {
        sleep(1);
    }

    apoAgent->Shutdown();

    return 0;
}

int main()
{
    AIInitIniHandler();
    AIInitLOGHandler();

    AIChangeLOGLevel(12);

    AIAgent loAgent;
    Test001(&loAgent);

    AICloseLOGHandler();
    AICloseIniHandler();

    return 0;
}

DECLARE_CMDOPT_BEGIN()
DECLARE_CMDOPT_END()
