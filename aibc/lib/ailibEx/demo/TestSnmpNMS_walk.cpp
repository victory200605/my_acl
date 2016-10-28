#include "AISnmp.h"
int IniPdu(stSnmpPdu *apoPdu, char *apcName)
{
    apoPdu->ciVersion = 0;
    _StringCopy(apoPdu->csCommunity,"public");
    apoPdu->ciCommand = 0xA0;
    apoPdu->ciReqid = 0;
    apoPdu->ciErrstat = 0;
    apoPdu->ciErrindex = 0;
    
    _StringCopy(apoPdu->coVariables[0].csName,apcName);
    apoPdu->coVariables[0].ccType = 0x05;
    _StringCopy(apoPdu->coVariables[0].coVal.csString,"NULL") ;
    apoPdu->coVariables[0].ciValLen = 1;

    return 0;
}

int Test001()
{
    stSnmpPdu loPdu;
    
    char *lpcOid = "1.3.6";

    int liSockfd;
    char *lpcAgentIp="127.0.0.1";
    int liAgentPort=4444;

//    char *lpcAgentIp="10.3.18.230";
//    int liAgentPort = 161;

//    char *lpcAgentIp="10.3.3.108";
//    int liAgentPort = 161;

    int ret;
    int i = 0;
//    while(i++ < 10000)
    {
        
        if((liSockfd = ai_socket_create(AF_INET,SOCK_DGRAM,0)) == AI_SOCK_ERROR_SOCKET)
        {
            ai_socket_close(liSockfd);
            return AI_SNMP_ESOCKET;
        }
        

        memset(&loPdu, 0, sizeof(loPdu));
        IniPdu(&loPdu,lpcOid);

        printf("csName=%s\n",loPdu.coVariables[0].csName);

        if((ret = SnmpSendPdu(liSockfd, &loPdu,lpcAgentIp,liAgentPort))!=0)
        {   
            if(ret == AI_SNMP_EWRONGCOMMAND)
            {
                AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"Wrong request command!\n");
                ai_socket_close(liSockfd);
                return AI_SNMP_EWRONGCOMMAND;
            }
            else
            {
                AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"your request pdu is too big!\n");
                ai_socket_close(liSockfd);
                return AI_SNMP_EPSEND;
            }
        }

        stSnmpPdu loPdu2;
        memset(&loPdu2, 0, sizeof(loPdu2));

        int ret = 0;
        if((ret = SnmpRecvPdu(liSockfd,&loPdu2,5))!=0)
        {
            ai_socket_close(liSockfd);
            return AI_SNMP_EPRECV;
        }


        lpcOid = loPdu2.coVariables[0].csName;
        if(loPdu2.coVariables[0].ccType == 0x05)
        {
            lpcOid = "1.3.6";
        }
        ai_socket_close(liSockfd);
    }

    return 0; 
}

int main(int argc,char **argv)
{
    AIInitIniHandler();
    AIInitLOGHandler();
   
    AIChangeLOGLevel(12);

    struct timeval ltStartTime,ltEndTime;
    memset(&ltStartTime,0,sizeof(ltStartTime));
    memset(&ltEndTime,0,sizeof(ltEndTime));

    gettimeofday(&ltStartTime,NULL);
    
    int ret = Test001();
    if(ret == 0)
    {
        gettimeofday(&ltEndTime,NULL);
        long time_ms = (ltEndTime.tv_sec - ltStartTime.tv_sec)*1000 + (ltEndTime.tv_usec - ltStartTime.tv_usec)/1000;
        printf("\nret=%d\n\trun time = %ld ms\n",ret,time_ms);
    }
    AICloseLOGHandler();
    AICloseIniHandler();
    return 0;
}

DECLARE_CMDOPT_BEGIN()
DECLARE_CMDOPT_END()
