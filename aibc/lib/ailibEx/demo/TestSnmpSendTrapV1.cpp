#include "AISnmp.h"

int main()
{
    stSnmpTrap loTrapPdu;
    memset(&loTrapPdu,0,sizeof(loTrapPdu));

    char *Enterprise = "1.3.6.1.4.1.2720";
    char *AgentAddress = "10.3.3.108";
    int GenTrapType = 6;
    int SpecificType = 1;
    long TimeStamp = time(NULL);

    loTrapPdu.ciVersion = AI_SNMP_VERSION_1;
    _StringCopy(loTrapPdu.csCommunity,"public");
    loTrapPdu.ciCommand = AI_SNMP_BER_TRAP;
    _StringCopy(loTrapPdu.csEnterprise,Enterprise);
    _StringCopy(loTrapPdu.csAgentAddr,AgentAddress);
    loTrapPdu.ciTrapType = GenTrapType;
    loTrapPdu.ciSpecificType = SpecificType;
    loTrapPdu.ciTrapType = 6;
    loTrapPdu.ciSpecificType = 1;
    loTrapPdu.ciTime = TimeStamp;

    char *OID_ALARMNO = "1.3.6.1.4.1.2720.7.1";
    SnmpAddVarStr(&loTrapPdu, OID_ALARMNO, "hello");

    char *OID_ORIGINALLEVEL = "1.3.6.1.4.1.2720.7.2";
    SnmpAddVarInt(&loTrapPdu, OID_ORIGINALLEVEL, "99");

    char *OID_ALARMCAUSE = "1.3.6.1.4.1.2720.7.3";
    SnmpAddVarOid(&loTrapPdu, OID_ALARMCAUSE, "1.3.6.1.2.1.2.3.4.5.6.7");

    char *OID_ACTIVESTATUS = "1.3.6.1.4.1.2720.7.4";
    SnmpAddVarTimeStamp(&loTrapPdu, OID_ACTIVESTATUS, "12345567");

    char *OID_TITLE = "1.3.6.1.4.1.2720.7.5";
    SnmpAddVarNULL(&loTrapPdu, OID_TITLE, "");

    char *OID_ADDITIONALINFOR = "1.3.6.1.4.1.2720.7.6";
    SnmpAddVarIPAddress(&loTrapPdu,OID_ADDITIONALINFOR,"10.3.18.91");

    char *lpcNMSIP = "10.3.18.230";
    int lpcNMSPort = AI_SNMP_TRAP_PORT;
//    char *lpcNMSIP = "127.0.0.1";
//    int lpcNMSPort = 4444;

    int ret = SnmpSendV1Trap(&loTrapPdu,lpcNMSIP,lpcNMSPort);
    printf("ret=%d\n",ret);
    return 0;
}
