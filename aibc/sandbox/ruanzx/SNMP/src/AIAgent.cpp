#include "AIAgent.h"
#include <strings.h>

int VariableBindingCompareFunc(const void * apOid1,const void * apOid2)
{
    stVariableBindingAgent *lpoOid1 = (stVariableBindingAgent *)apOid1;
    stVariableBindingAgent *lpoOid2 = (stVariableBindingAgent *)apOid2;
    
    long const* lpiOid1 = lpoOid1->caOid;
    long const* lpiOid2 = lpoOid2->caOid;

    int i=AI_SNMP_OID_NUM;
    while(i--)
    {
        if(*lpiOid1 != *lpiOid2)
        {
            if(*lpiOid1 < *lpiOid2)
            {
                return -1;
            }
            else 
            {
                return 1;
            }
        }
        lpiOid1++;
        lpiOid2++;
    }
    return 0;
}

int VariableBindingCompareFuncCallback(const void* apOidPrefix, const void* apOid)
{
    stCallbackEntry *lpoOidPrefix = (stCallbackEntry *)apOidPrefix;
    stCallbackEntry *lpoOid = (stCallbackEntry *)apOid;
    
    int i=0,j=0,liCount=0;
    long const* lpiOid1 = lpoOidPrefix->caOid;
    long const* lpiOid2 = lpoOid->caOid;

    while(*lpiOid1++ != -1)
    {
        i++;
    }
    while(*lpiOid2++ != -1)
    {
        j++;
    }
    liCount=((i<j)?i:j);

    lpiOid1 = lpoOidPrefix->caOid;
    lpiOid2 = lpoOid->caOid;

    while(liCount--)
    {
        if(*lpiOid1 != *lpiOid2)
        {
            if(*lpiOid1 > *lpiOid2)
            {
                return 1;
            }
            else
            {
                return -1;
            }
        }
        lpiOid1++;
        lpiOid2++;
    }
    return 0;
}

int ArrToLongOid(char const* apcName,long* apiOid, size_t aiSize)
{
    char const  *lpcName;

    for(lpcName = apcName; *lpcName; ++lpcName)
    {   
        if (isdigit(*lpcName) == 0 && '.' != *lpcName)
        {   
            return AI_SNMP_ERROIDS;
        }
    }
                    
    lpcName = apcName; 
    while(*lpcName == '.')
    {   
        lpcName++;
    }
                      
    memset(apiOid, -1, sizeof(long)*aiSize);

    for (size_t i = 0; i < aiSize; ++i)
    {   
        char* lpcNext;
        errno = 0;
        apiOid[i] = strtol(lpcName, &lpcNext, 10);
        if (errno != 0)
        {     
            return AI_SNMP_ERRCOV;
        }

        if (*lpcNext == '\0')
        {   
            break;
        }

        if (*lpcNext != '.')
        {  
            return AI_SNMP_ERRCOV;
        }
        lpcName = lpcNext + 1;
    }   
    return 0;
}

int AIAgent::SetData(stVariableBindingAgent& aoData)
{
    AISmartLock loLock(coLock);

    if(coSnmpDataTable.size() > 0)
    {
        stVariableBindingAgent* lpoFind = (stVariableBindingAgent* )bsearch( (void *)&aoData, &(coSnmpDataTable)[0], coSnmpDataTable.size(),sizeof(stVariableBindingAgent),VariableBindingCompareFunc);

        if(lpoFind != 0)
        {
            *lpoFind = aoData; 
        }
        else
        {
            if( ArrToLongOid(aoData.csName,aoData.caOid,AI_SNMP_OID_NUM) != 0)
            {
                return AI_SNMP_AGENT_EARRTOLONG;
            }
            coSnmpDataTable.push_back(aoData);
            qsort(&coSnmpDataTable[0], coSnmpDataTable.size(),sizeof(stVariableBindingAgent),VariableBindingCompareFunc);
        }        
    }
    else
    {
        if( ArrToLongOid(aoData.csName, aoData.caOid,AI_SNMP_OID_NUM) != 0)
        {
            return AI_SNMP_AGENT_EARRTOLONG;
        }
        coSnmpDataTable.push_back(aoData);
    }    
    return 0;
}

int AIAgent::GetData(stSnmpPdu *apoPdu)
{
    AISmartLock loLock(coLock);

    bool lbFlag = 1;
    for(int i=0;i<apoPdu->ciVBNum;i++)
    {
        stVariableBindingAgent loVB;
        _StringCopy(loVB.csName, apoPdu->coVariables[i].csName);

        if( ArrToLongOid(loVB.csName,loVB.caOid,AI_SNMP_OID_NUM) != 0)
        {
            return AI_SNMP_AGENT_EARRTOLONG;
        }
        stVariableBindingAgent *lpoData;
        lpoData =(stVariableBindingAgent *)bsearch((void *)&loVB, &coSnmpDataTable[0], coSnmpDataTable.size(), sizeof(stVariableBindingAgent),VariableBindingCompareFunc);

        if(lpoData!=NULL)
        {
            apoPdu->coVariables[i].ccType = lpoData->ccType;

            if(apoPdu->coVariables[i].ccType != AI_BER_NULL)
            {
                if(lpoData->ccType!=AI_BER_INTEGER && lpoData->ccType!=AI_BER_COUNTER && lpoData->ccType!=AI_BER_TIMETICKS)
                {
                    _StringCopy(apoPdu->coVariables[i].coVal.csString,lpoData->coVal.csString);
                    apoPdu->coVariables[i].ciValLen=strlen(apoPdu->coVariables[i].coVal.csString);
                }
                else
                {
                    apoPdu->coVariables[i].coVal.ciInteger=lpoData->coVal.ciInteger;
                    apoPdu->coVariables[i].ciValLen=sizeof(lpoData->coVal.ciInteger);
                }
            }
        }
        //Else if no such oid,can't get!
        else  
        {
                apoPdu->coVariables[i].ccType = AI_BER_NULL;
                apoPdu->ciErrstat = AI_SNMP_ERRSTAT_NOSUCHNAME;
                apoPdu->coVariables[i].ciValLen = 1;
                if(lbFlag)
                {
                    apoPdu->ciErrindex = i;          
                    lbFlag = 0;
                }
        }
    }
    return 0;
}

int AIAgent::FindNextData(stSnmpPdu *apoPdu)
{
    AISmartLock loLock(coLock);
    
    for(int i=0;i<apoPdu->ciVBNum;i++)
    {
        long liName[AI_SNMP_OID_NUM];
        memset(liName,0,sizeof(liName));

        if( ArrToLongOid(apoPdu->coVariables[i].csName,liName,AI_SNMP_OID_NUM) != 0 )
        {
            return AI_SNMP_AGENT_EARRTOLONG;
        }

        if(coSnmpDataTable.size() > 0)
        {
            if(VariableBindingCompareFunc(liName,coSnmpDataTable.back().caOid)==-1)
            {
                for( SnmpDataTable::iterator it = coSnmpDataTable.begin();it!=coSnmpDataTable.end();it++)
                {
                    if(VariableBindingCompareFunc(liName, it->caOid)==-1)
                    {
                        apoPdu->coVariables[i].ccType = it->ccType;
                        if(apoPdu->coVariables[i].ccType == AI_BER_NULL)
                        {
                            apoPdu->ciErrstat = AI_SNMP_ERRSTAT_NOSUCHNAME;
                            apoPdu->coVariables[i].ciValLen = 1;
                            break;
                        }
                        else
                        {
                            if(it->ccType!=AI_BER_INTEGER && it->ccType!=AI_BER_COUNTER && it->ccType!=AI_BER_TIMETICKS)
                            {
                                _StringCopy(apoPdu->coVariables[i].csName, it->csName);
                                _StringCopy(apoPdu->coVariables[i].coVal.csString,it->coVal.csString);
                                apoPdu->coVariables[i].ciValLen=strlen(apoPdu->coVariables[i].coVal.csString);
                                break;
                            }
                            else
                            {
                                _StringCopy(apoPdu->coVariables[i].csName, it->csName);
                                apoPdu->coVariables[i].coVal.ciInteger=it->coVal.ciInteger;
                                apoPdu->coVariables[i].ciValLen=sizeof(it->coVal.ciInteger);
                                break;
                            }
                        }
                    }
                }
            } 
            //Else if the oid is out of range,can't get-next
            else
            {
                apoPdu->coVariables[i].ccType = AI_BER_NULL;
                apoPdu->ciErrstat = AI_SNMP_ERRSTAT_NOSUCHNAME;
                apoPdu->coVariables[i].ciValLen = 1;
            }
        }
        else
        {
            apoPdu->coVariables[i].ccType = AI_BER_NULL;
            apoPdu->ciErrstat = AI_SNMP_ERRSTAT_NOSUCHNAME;
            apoPdu->coVariables[i].ciValLen = 1;
        }
    }
    return 0;
}

int AIAgent::DelData(char *apcOid)
{
    AISmartLock loLock(coLock);

    SnmpDataTable::iterator i;
    for(i = coSnmpDataTable.begin();i!=coSnmpDataTable.end();i++)
    {   
        if(StringMatch(i->csName,apcOid,0))
        {
            coSnmpDataTable.erase(i--);
        }
    }
    return 0;
}

int AIAgent::StartUp(char *apcLocalIp, int aiLocalPort)
{
    struct sockaddr_in loLocalAddr;
    memset(&loLocalAddr,0,sizeof(loLocalAddr));
    
    if((ciSockfd=socket(AF_INET,SOCK_DGRAM,0))==-1)
    {       
        return AI_SNMP_ESOCKET;
    }

    loLocalAddr.sin_family=AF_INET;
    loLocalAddr.sin_port=htons(aiLocalPort);   
    if(apcLocalIp==NULL)
    {
        loLocalAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    }
    else
    {
        int liRet = inet_pton(AF_INET, apcLocalIp, &loLocalAddr.sin_addr);
        if(liRet != 1)
        {
            if(liRet == 0)
            {
                return AI_SNMP_EINVALSTRING;
            }
            else
            {
                return AI_SNMP_ERRPTON;
            }
        }
    }    

    if(bind(ciSockfd,(struct sockaddr *)&loLocalAddr,sizeof(loLocalAddr))==-1)
    {
        return AI_SNMP_EBIND;
    }
    cbIsRunning=true;
    pthread_t liId;

    int ret=pthread_create(&liId, NULL, ThreadEntry, (void *)this);
    if(ret!=0)
    {
        perror("Thread create failed!");
        return -1;
    }
    return 0;
}

void* AIAgent::ThreadEntry(void* apArg)
{   
    AIAgent*    lpoAgent = (AIAgent*)apArg;
            
    int ret=pthread_detach(pthread_self());
    if(ret!=0)
    {
        perror("Thread detach failed!");
        return NULL;
    }

    while (lpoAgent->cbIsRunning)
    {   
        lpoAgent->ProcessFunc();
    }
    return NULL;
}

int AIAgent::ProcessFunc()
{
    struct sockaddr_in loCliAddr;
    memset(&loCliAddr, 0, sizeof(loCliAddr));

    AIChunkEx loChunk(sizeof(stSnmpPdu));
    stSnmpPdu *lpoPdu = (stSnmpPdu *)loChunk.GetPointer();

    if(SnmpRecvPdu(ciSockfd,lpoPdu,&loCliAddr)!=0)
    {   
        return AI_SNMP_EPRECV;
    }
    
    RefreshSnmpDataTable();
    ShowDataINSnmpDataTable();
    ShowReceivedData(lpoPdu);

    //Get Request   
    if(lpoPdu->ciCommand == AI_SNMP_BER_GET)
    { 
        if( GetData(lpoPdu) != 0)
        {
            return AI_SNMP_AGENT_EGETDATA;
        }
    }

    //GetNext Request   
    else if(lpoPdu->ciCommand == AI_SNMP_BER_GETNEXT)
    {  
        if( FindNextData(lpoPdu) != 0)
        {
            return AI_SNMP_AGENT_EFINDNEXTDATA;
        }
    }

    ShowReceivedData(lpoPdu);

    //send back to AIClient
    char lsRemoteIp[16];
    memset(lsRemoteIp, 0, sizeof(lsRemoteIp));
    if((inet_ntop(AF_INET, &loCliAddr.sin_addr, lsRemoteIp, sizeof(lsRemoteIp)))==NULL)
    {
        return AI_SNMP_ERRNTOP;
    }

    int liRemotePort=8888;
    if(SnmpSendPdu(ciSockfd,lpoPdu,lsRemoteIp,liRemotePort) != EMSGSIZE)
    {   
    //if the pdu can't contain all data
        AIChunkEx loChunkL(sizeof(stSnmpPdu));
        stSnmpPdu *lpoPduL = (stSnmpPdu *)loChunkL.GetPointer();

        lpoPduL->ciVersion = lpoPdu->ciVersion;
        _StringCopy(lpoPduL->csCommunity,lpoPdu->csCommunity);
        lpoPduL->ciCommand = lpoPdu->ciCommand;
        lpoPduL->ciReqid = lpoPdu->ciReqid;
        lpoPduL->ciErrstat = AI_SNMP_ERRSTAT_TOOBIG;
        lpoPduL->ciErrindex = 0;            
        _StringCopy(lpoPduL->coVariables[0].csName,"0.0.0.0");
        lpoPdu->coVariables[0].ccType = AI_BER_OCTET_STR;
        _StringCopy(lpoPduL->coVariables[0].coVal.csString,"too big!!!");
        lpoPduL->coVariables[0].ciValLen = strlen(lpoPduL->coVariables[0].coVal.csString);
        SnmpSendPdu(ciSockfd,lpoPduL,lsRemoteIp,liRemotePort);
        
        return AI_SNMP_EPSEND;
    }

    return 0;
}

int AIAgent::Shutdown()
{
    cbIsRunning=false;

    ai_socket_close(ciSockfd);

    return 0;
}

int AIAgent::RegisterCallback(char const* apcOidPrefix, AIAgentCallback apfCallback, time_t aiRefreshInterval)
{   
    AISmartLock loLock(coLock);

    stCallbackEntry loCallbackEntry;
    _StringCopy(loCallbackEntry.csOid,apcOidPrefix);       
    loCallbackEntry.cpCallback = apfCallback;
    loCallbackEntry.ciInterval = aiRefreshInterval;
    loCallbackEntry.ciLastUpdate = 0;
                
    if(coAIAgentCallbackTable.size() > 0)
    {
        if(ArrToLongOid(loCallbackEntry.csOid,loCallbackEntry.caOid,AI_SNMP_OID_NUM) != 0)
        {
            return AI_SNMP_AGENT_EARRTOLONG;
        }
        
        stCallbackEntry *loData;
        loData =(stCallbackEntry *)bsearch((void *)&loCallbackEntry, &coAIAgentCallbackTable[0], coAIAgentCallbackTable.size(), sizeof(stCallbackEntry),VariableBindingCompareFuncCallback);

        if(loData != NULL)
        {
            return AI_SNMP_AGENT_ESAMEOIDPREFIX;
        }
        else
        {
            coAIAgentCallbackTable.push_back(loCallbackEntry);
            qsort(&coAIAgentCallbackTable[0],coAIAgentCallbackTable.size(),sizeof(stCallbackEntry),VariableBindingCompareFunc);
        }
    }
    else
    {
        if( ArrToLongOid(loCallbackEntry.csOid,loCallbackEntry.caOid,AI_SNMP_OID_NUM) != 0)
        {
            return AI_SNMP_AGENT_EARRTOLONG;
        }
        coAIAgentCallbackTable.push_back(loCallbackEntry);
    }
    return 0;
}

int AIAgent::UnregisterCallback(char const* apcOidPrefix)
{
    AISmartLock loLock(coLock);

    if(coAIAgentCallbackTable.size() > 0)
    {
        AIAgentCallbackTable::iterator i;
        for(i=coAIAgentCallbackTable.begin(); i!=coAIAgentCallbackTable.end(); i++)
        {
            if(strcmp(i->csOid, apcOidPrefix)==0)
            {
                coAIAgentCallbackTable.erase(i--);
                return 0;
            }
        }
        return AI_SNMP_AGENT_ECANTUREG;
    }
    return AI_SNMP_AGENT_ENODATA;
}

int AIAgent::UnregisterCallback(AIAgentCallback apfCallback)
{
    AISmartLock loLock(coLock);

    if(coAIAgentCallbackTable.size() > 0)
    {
        AIAgentCallbackTable::iterator i;
        for(i=coAIAgentCallbackTable.begin(); i!=coAIAgentCallbackTable.end(); i++)
        {
            if(i->cpCallback == apfCallback)
            {
                coAIAgentCallbackTable.erase(i--);
                return 0;
            }
        }
        return AI_SNMP_AGENT_ECANTUREG;
    }
    return AI_SNMP_AGENT_ENODATA;
}

int AIAgent::RefreshSnmpDataTable()
{
    if(coAIAgentCallbackTable.size() > 0)
    {
        for( AIAgentCallbackTable::iterator i = coAIAgentCallbackTable.begin(); i!=coAIAgentCallbackTable.end(); i++)
        {
            if(time(NULL) >= i->ciLastUpdate + i->ciInterval)
            {
                char * lpcOid = i->csOid;
                DelData(strcat(lpcOid,".*"));
                i->cpCallback(this);        
                i->ciLastUpdate = time(NULL);
            }
        }
        return 0;
    }
    return AI_SNMP_AGENT_ENODATA;
}

int AIAgent::ShowDataINSnmpDataTable()
{
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE, "******************The data in SnmpDataTable is as follows************************");
    if(coSnmpDataTable.size()>0)
    {
        for(SnmpDataTable::iterator i=coSnmpDataTable.begin();i!=coSnmpDataTable.end();i++)
        {
            if(i->ccType!=AI_BER_INTEGER && i->ccType!=AI_BER_COUNTER && i->ccType!=AI_BER_TIMETICKS)
            {
                AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"The oid of variable is %s, the value of variable is %s", i->csName, i->coVal.csString);
            }
            else
            {
                AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"The oid of variable is %s, the value of variable is %ld", i->csName, i->coVal.ciInteger);
            }   
        }
    }
    else
    {
        AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"There is no data in SnmpDataTable.");
        AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"***********************************************************************************");
        return -1;
    }
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"***********************************************************************************");
    return 0;
}

int AIAgent::ShowDataINAIAgentCallbackTable()
{
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"******************The data in AIAgentCallbackTable is as follows******************");
    if(coAIAgentCallbackTable.size()>0)
    {
        for(AIAgentCallbackTable::iterator i=coAIAgentCallbackTable.begin();i!=coAIAgentCallbackTable.end();i++)
        {
            AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"The oid prefix is %s,the ciInterval is %ld, the ciLastUpdate is %ld\n",i->csOid,i->ciInterval,i->ciLastUpdate);    
        }
    }
    else
    {
        AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"There is no data in AgentCallbackTable.");
        AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"***********************************************************************************");
        return -1;
    }   
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"***********************************************************************************");
    return 0;
}

