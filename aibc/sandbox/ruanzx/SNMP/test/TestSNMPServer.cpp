#include "AISnmp.h"
#include "AIAgent.h"

int Callback1(void* apCaller)
{
    MIB_BEGIN(apCaller);
    MIB_SET_INT("1.3.11.11.11.11.11",11);
    MIB_SET_STR("1.3.22.22.22.22.22","22orland");
    MIB_SET_INT("1.3.88.88.88.88.88.8888",88);
    MIB_SET_INT("1.3.77.77.77",77777);
    MIB_SET_INT("1.3.66.66",66);
    
    MIB_SET_OID("1.3.55.55.1.1.4.0","1.3.55.5.5.55");
    MIB_SET_OID("1.3.44.44.1.4.0","1.3.44.44.44");

    MIB_TABLE_BEGIN("1.3.7.1.2.1.4.20");
    MIB_ROW_BEGIN(1);
    MIB_ROW_SET_INT(1, 1);
    MIB_ROW_SET_STR(2,"2xxxxx");
    MIB_ROW_SET_STR(3,"3aaaaa");
    MIB_ROW_END();

    MIB_ROW_BEGIN(2);
    MIB_ROW_SET_INT(1, 1321);
    MIB_ROW_SET_STR(2,"2bbbbb");
    MIB_ROW_SET_STR(3,"3ccccc");
    MIB_ROW_END();
    MIB_TABLE_END();

    MIB_TABLE_BEGIN("1.3.17.81.32.1.4.20");
    MIB_ROW_BEGIN(1);
    MIB_ROW_SET_INT(1, 1999);
    MIB_ROW_SET_STR(2,"2ddddd");
    MIB_ROW_SET_STR(3,"3eeeee");
    MIB_ROW_END();

    MIB_ROW_BEGIN(2);
    MIB_ROW_SET_INT(1, 1888);
    MIB_ROW_SET_STR(2,"2fffff");
    MIB_ROW_SET_STR(3,"3ggggg");
    MIB_ROW_SET_STR(4,"4hhhhh");
    MIB_ROW_SET_STR(5,"5iiiii");
    MIB_ROW_END();
    MIB_TABLE_END();

    MIB_END();
}


int Callback2(void* apCaller)
{
    MIB_BEGIN(apCaller);
    MIB_SET_INT("1.3.6.1.8.0",8);
    MIB_SET_STR("1.3.6.1.8.8.8.0","neworland");
    MIB_END();
}

int Callback3(void* apCaller)
{
    MIB_BEGIN(apCaller);
    MIB_TABLE_BEGIN("1.3.16");
    MIB_ROW_BEGIN(1);
    MIB_ROW_SET_INT(1, 1);
    MIB_ROW_SET_STR(2,"x");
    MIB_ROW_SET_STR(3,"a");
    MIB_ROW_END();
                           
    MIB_ROW_BEGIN(2);
    MIB_ROW_SET_INT(1, 2);
    MIB_ROW_SET_STR(2,"b");
    MIB_ROW_SET_STR(3,"c");
    MIB_ROW_END();
    MIB_TABLE_END();

    MIB_END();
}

int Callback4(void* apCaller)
{
    MIB_BEGIN(apCaller);
    MIB_SET_INT("1.1.2.1",4);
    MIB_END();
}

int Callback5(void* apCaller)
{
    
}

int Test001(AIAgent * apoAgent)
{
    Callback1(apoAgent);
    
    apoAgent->ShowDataINSnmpDataTable();
    apoAgent->RegisterCallback("1.3.11",Callback2,5);
    apoAgent->RegisterCallback("1.3.7",Callback5,5);
    apoAgent->RegisterCallback("1.1.2",Callback4,5);
    apoAgent->RegisterCallback("1.3.16",Callback3,5);
    apoAgent->RegisterCallback("1.3.17",Callback5,5);

    apoAgent->ShowDataINSnmpDataTable();
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"#########################################");    
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"\ttesting the same oid prefix!");
    apoAgent->RegisterCallback("1.1.2.1",Callback5,5);
    apoAgent->RegisterCallback("1.1.2",Callback5,5);
    apoAgent->RegisterCallback("1.1",Callback5,5);
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"#########################################");

    apoAgent->ShowDataINSnmpDataTable();
    apoAgent->ShowDataINAIAgentCallbackTable();
    apoAgent->UnregisterCallback(Callback3);
    apoAgent->UnregisterCallback("1.1.2");
    apoAgent->ShowDataINAIAgentCallbackTable();

    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"#########################################");
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"\trefresh callback table,in main");
    apoAgent->RefreshSnmpDataTable();
    apoAgent->ShowDataINSnmpDataTable();
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"#########################################");
    apoAgent->RegisterCallback("1.3.17",Callback5,5);
    apoAgent->ShowDataINSnmpDataTable();

    char *lpcLocalIp="127.0.0.1";
    //char *lpcLocalIp="10.3.3.108";
    int liLocalPort=6666;

    apoAgent->StartUp(lpcLocalIp, liLocalPort);
    while(true)
    {
        sleep(1);
    }
    apoAgent->Shutdown();

    return 0;
}

int main (void)
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
