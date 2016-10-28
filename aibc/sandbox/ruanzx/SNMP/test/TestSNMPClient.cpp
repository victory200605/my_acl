#include "AISnmp.h"
int ReadTxtFile(char * apFileName, stSnmpPdu * apoPdu)
{
    FILE *fp = NULL;
    char lsStr[AI_SNMP_MAXLINE];
    memset(&lsStr,0,sizeof(lsStr));

    if((fp=fopen(apFileName,"r"))==NULL)
    {      
        return -1;
    }  
    fgets(lsStr,sizeof(lsStr),fp);
   
    char *lpcPos=lsStr;
    int liCounter=1;
    while(*lpcPos!=0)
    {      
        if(*lpcPos==':')
        {
            liCounter++;
        }
        lpcPos++;
    }

    char *lpcToken[liCounter];
    char *lpcRn;
    lpcToken[0]=strtok(lsStr,":");

    for(int i=0;i<liCounter&&lpcToken!=NULL;i++)
    {  
        lpcToken[i+1]=strtok(NULL,":");
    }  

    apoPdu->ciVersion = strtol(lpcToken[0],NULL,0);
    _StringCopy(apoPdu->csCommunity,lpcToken[1]);
    apoPdu->ciCommand = strtol(lpcToken[2],NULL,0);

    apoPdu->ciReqid = strtol(lpcToken[3],NULL,0);
    apoPdu->ciErrstat = strtol(lpcToken[4],NULL,0);
    apoPdu->ciErrindex = strtol(lpcToken[5],NULL,0);
    for(int i=0;i<(liCounter-6)/3-1;i++)
    {
        _StringCopy(apoPdu->coVariables[i].csName,lpcToken[6+3*i]);
        apoPdu->coVariables[i].ccType=(unsigned char)(strtol(lpcToken[7+3*i],NULL,16));

        switch(strtol(lpcToken[7+3*i],NULL,16))
        {
        case AI_BER_INTEGER:
            apoPdu->coVariables[i].coVal.ciInteger=strtol(lpcToken[8+3*i],NULL,0);
            apoPdu->coVariables[i].ciValLen=sizeof(apoPdu->coVariables[i].coVal.ciInteger);
            break;
        case AI_BER_OCTET_STR:
            _StringCopy(apoPdu->coVariables[i].coVal.csString,lpcToken[8+3*i]);
            apoPdu->coVariables[i].ciValLen=strlen(apoPdu->coVariables[i].coVal.csString);
            break;
        case AI_BER_OBJECT_ID:
            _StringCopy(apoPdu->coVariables[i].coVal.csObjid,lpcToken[8+3*i]);
            apoPdu->coVariables[i].ciValLen=strlen(apoPdu->coVariables[i].coVal.csObjid);
            break;
        case AI_BER_TIMETICKS:
            apoPdu->coVariables[i].coVal.ciInteger=strtoul(lpcToken[8+3*i],NULL,0);
            apoPdu->coVariables[i].ciValLen=sizeof(apoPdu->coVariables[i].coVal.ciInteger);
            break;
        case AI_BER_NULL:
            _StringCopy(apoPdu->coVariables[i].coVal.csString,lpcToken[8+3*i]);
            apoPdu->coVariables[i].ciValLen=1;
            break;
        }
    }
    _StringCopy(apoPdu->coVariables[((liCounter-6)/3-1)].csName,lpcToken[liCounter-3]);
    apoPdu->coVariables[((liCounter-6)/3-1)].ccType=(unsigned char)(strtol(lpcToken[liCounter-2],NULL,16));
    
    switch(strtol(lpcToken[liCounter-2],NULL,16))
    {
    case AI_BER_INTEGER:
        apoPdu->coVariables[((liCounter-6)/3-1)].coVal.ciInteger=strtol(lpcToken[liCounter-1],NULL,0);
        apoPdu->coVariables[((liCounter-6)/3-1)].ciValLen=sizeof(apoPdu->coVariables[((liCounter-6)/3-1)].coVal.ciInteger);
        break;
    case AI_BER_OCTET_STR:
        lpcRn=strstr(lpcToken[liCounter-1],"\r\n");
        *lpcRn=0;
        _StringCopy(apoPdu->coVariables[((liCounter-6)/3-1)].coVal.csString,lpcToken[liCounter-1]);
        apoPdu->coVariables[((liCounter-6)/3-1)].ciValLen=strlen(apoPdu->coVariables[((liCounter-6)/3-1)].coVal.csString);
        break;
    case AI_BER_OBJECT_ID:
        lpcRn=strstr(lpcToken[liCounter-1],"\r\n");
        *lpcRn=0;
        _StringCopy(apoPdu->coVariables[((liCounter-6)/3-1)].coVal.csObjid,lpcToken[liCounter-1]);
        apoPdu->coVariables[((liCounter-6)/3-1)].ciValLen=strlen(apoPdu->coVariables[((liCounter-6)/3-1)].coVal.csObjid);
        break;
    case AI_BER_TIMETICKS:
        apoPdu->coVariables[((liCounter-6)/3-1)].coVal.ciInteger=strtoul(lpcToken[liCounter-1],NULL,0);
        apoPdu->coVariables[((liCounter-6)/3-1)].ciValLen=sizeof(apoPdu->coVariables[((liCounter-6)/3-1)].coVal.ciInteger);
        break;
    
    case AI_BER_NULL:
        _StringCopy(apoPdu->coVariables[((liCounter-6)/3-1)].coVal.csString,lpcToken[liCounter-1]);
        apoPdu->coVariables[((liCounter-6)/3-1)].ciValLen=1;
        break;
    }
    

    fclose(fp);
    return 0;
}

int Test001(char *apcFileName)
{
    stSnmpPdu loPdu;
    memset(&loPdu, 0, sizeof(loPdu));
    
    if((ReadTxtFile(apcFileName, &loPdu))!=0)
    {
        return -1;
    }

    int liSockfd;
    char *lpcAgentIp="127.0.0.1";
    int liAgentPort=6666;

    //char *lpcAgentIp="10.3.18.161";
    //int liAgentPort=161;

    if((liSockfd=socket(AF_INET,SOCK_DGRAM,0))==-1)
    {   
        return AI_SNMP_ESOCKET;
    }
    
    int ret;
    if((ret = SnmpSendPdu(liSockfd,&loPdu,lpcAgentIp,liAgentPort))!=0)
    {
        if(ret == AI_SNMP_EWRONGCOMMAND)
        {
            AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"Wrong request command!\n");
            return AI_SNMP_EWRONGCOMMAND;
        }
        else
        {
            AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"your request pdu is too big!\n");
            return AI_SNMP_EPSEND;
        }
    }

//////////////////////////////////////////////////////////
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"\n----Receive get data----\n");
    stSnmpPdu loPdu2;
    memset(&loPdu2, 0, sizeof(loPdu2));

    char *lpcLocalIp=NULL;
    int liLocalPort=8888;

    if((liSockfd=socket(AF_INET,SOCK_DGRAM,0))==-1)
    {
        return AI_SNMP_ESOCKET;
    }

    if(SnmpRecvPdu(liSockfd,&loPdu2,lpcLocalIp,liLocalPort)!=0)
    {
        return AI_SNMP_EPRECV;
    }
    
    close(liSockfd);
    ShowReceivedData(&loPdu2);
    return 0; 
}

int main(int argc,char **argv)
{
    if(argc<2)
    {
	printf("please input text file");
	return 0;
    }
    char *lpcFileName = argv[1];

    AIInitIniHandler();
    AIInitLOGHandler();
   
    AIChangeLOGLevel(12);

    Test001(lpcFileName);
    
    AICloseLOGHandler();
    AICloseIniHandler();
    return 0;
}

DECLARE_CMDOPT_BEGIN()
DECLARE_CMDOPT_END()
