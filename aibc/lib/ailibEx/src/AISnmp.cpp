#include "AISnmp.h"

///start namespace
AIBC_NAMESPACE_START

int SnmpSendPdu(int aiSocket, stSnmpPdu *apoPdu,char const* apcRemoteIp, int aiRemotePort)
{
    int liSockfd = aiSocket;
    int liSendBytes;

    unsigned char lsBuffer[AI_SNMP_MAXLINE];
    memset(lsBuffer, 0, sizeof(lsBuffer));

    int liLen;
    if((liLen=SnmpPduBuild(apoPdu, lsBuffer, sizeof(lsBuffer)))<0)
    {
        if(liLen == AI_SNMP_EWRONGCOMMAND)
        {
            return AI_SNMP_EWRONGCOMMAND;
        }
        else
        {
            return AI_SNMP_EPBUILD;
        }
    }


    if((liSendBytes = ai_socket_sendto(liSockfd, apcRemoteIp, aiRemotePort, (char *)lsBuffer, liLen)) < 0 )
    {
        return AI_SNMP_ESENDTO;
    }

    return 0;
}

//AIAgent receive PDU
int SnmpAgentRecvPdu(int aiSocket, stSnmpPdu *apoPdu, char * apcRemoteIP, int * apiRemotePort, int aiTimeout)
{
    int  liRecvBytes=0;
    
    unsigned char lsRecvLine[AI_SNMP_MAXLINE];
    memset(lsRecvLine, 0, sizeof(lsRecvLine));

    liRecvBytes = ai_socket_recvfrom(aiSocket, apcRemoteIP, 16, apiRemotePort, (char *)lsRecvLine, AI_SNMP_MAXLINE, aiTimeout);
    if(liRecvBytes <= 0)
    {
        return AI_SNMP_ERECV;
    }

    if(SnmpPduParse(apoPdu, lsRecvLine, liRecvBytes)!=0)
    {
        return AI_SNMP_EPPARSE;
    }

    return 0;
}

//AIClient receive PDU
int SnmpRecvPdu(int aiSocket, stSnmpPdu* apoPdu, int aiTimeout)
{
    int            liRecvBytes=0;
    unsigned char  lsRecvLine[AI_SNMP_MAXLINE];
    memset(lsRecvLine, 0, sizeof(lsRecvLine));

    liRecvBytes = ai_socket_recvfrom(aiSocket, NULL, 16, NULL, (char *)lsRecvLine, AI_SNMP_MAXLINE, aiTimeout);
    if(liRecvBytes <= 0)
    {
        return AI_SNMP_ERECV;
    }

    if(SnmpPduParse(apoPdu, lsRecvLine, liRecvBytes)!=0)
    {
        return AI_SNMP_EPPARSE;
    }

    //ShowReceivedData(apoPdu);
    return 0;
}

int SnmpSendV1Trap(stSnmpTrap * apoPdu, char const* apcRemoteIp, int aiRemotePort)
{
    int                liSendBytes;
    
    unsigned char      lsBuffer[AI_SNMP_MAXLINE];
    memset(lsBuffer,0,sizeof(lsBuffer));
    int liSockfd;

    if((liSockfd = ai_socket_create(AF_INET,SOCK_DGRAM,0)) == AI_SOCK_ERROR_SOCKET)
    {
        ai_socket_close(liSockfd);
        return AI_SNMP_ESOCKET;
    }

    int liLen;
    if((liLen=SnmpTrapBuild(apoPdu, lsBuffer, sizeof(lsBuffer)))<0)
    {
        ai_socket_close(liSockfd);
        return AI_SNMP_EPBUILD;
    }

    if((liSendBytes = ai_socket_sendto(liSockfd, apcRemoteIp, aiRemotePort, (char *)lsBuffer, liLen)) < 0 )
    {
        ai_socket_close(liSockfd);
        return AI_SNMP_ESENDTO;
    }

    ai_socket_close(liSockfd);
    return 0;
}

int SnmpRecvV1Trap(stSnmpTrap* apoPdu, char const* apcLocalIp, int aiLocalPort, int aiTimeout)
{
    int                liRecvBytes=0;
    unsigned char      lsRecvLine[AI_SNMP_MAXLINE];
    memset(lsRecvLine, 0, sizeof(lsRecvLine));

    int liSocket;
    if((liSocket = ai_socket_create(AF_INET,SOCK_DGRAM,0)) == AI_SOCK_ERROR_SOCKET)
    {
        ai_socket_close(liSocket);
        return AI_SNMP_ESOCKET;
    }
    ai_socket_set_nonblocking(liSocket, true);

    struct sockaddr_in loServAddr;
    memset(&loServAddr,0,sizeof(loServAddr));

    loServAddr.sin_family=AF_INET;
    loServAddr.sin_port=htons(aiLocalPort);

    if(apcLocalIp==NULL)
    {
        loServAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    }
    else
    {
        int liRet = inet_pton(AF_INET,apcLocalIp,&loServAddr.sin_addr);
        if(liRet != 1)
        {
            if(liRet == 0)
            {
                ai_socket_close(liSocket);
                return AI_SNMP_EINVALSTRING;
            }
            else
            {
                ai_socket_close(liSocket);
                return AI_SNMP_ERRPTON;
            }
        }
    }

    int liFlag = 1;
    setsockopt(liSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&liFlag, sizeof(int));

    if(bind(liSocket,(struct sockaddr *)&loServAddr,sizeof(loServAddr))==-1)
    {
        ai_socket_close(liSocket);
        return AI_SNMP_EBIND;
    }
    
    liRecvBytes = ai_socket_recvfrom(liSocket, NULL, 16, NULL, (char *)lsRecvLine, AI_SNMP_MAXLINE, aiTimeout);
    
    if(liRecvBytes <= 0)
    {
        ai_socket_close(liSocket);
        return AI_SNMP_ERECV;
    }

    if(SnmpTrapParse(apoPdu, lsRecvLine, liRecvBytes)!=0)
    {
        ai_socket_close(liSocket);
        return AI_SNMP_EPPARSE;
    }

    ai_socket_close(liSocket);
    return 0;
}

int ArrToOid(char const *apcName, unsigned long *apiOid, size_t aiSize)
{
    int liCounter=0;
    const char *lpcName=apcName;
    for(lpcName=apcName; *lpcName; ++lpcName)
    {
        if (isdigit(*lpcName) == 0 && '.' != *lpcName)
        {
            return AI_SNMP_ERROIDS;
        }
    }

    lpcName = apcName;
    char * lpcNext;
    while(*lpcName == '.')
    {
        lpcName++;
    }

    for (size_t i = 0; i < aiSize; ++i)
    {
        errno = 0;
        apiOid[i] = strtoul(lpcName, &lpcNext, 10);
        if (errno != 0)
        {
              return AI_SNMP_ERRCOV;
        }
        
        ++liCounter;
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

    return liCounter;
}

int CutAddr(char *apcSrc, char *apcDes, int aiSize)
{
    int liCounter=0;
    const char *lpcName=apcSrc;
    for(lpcName=apcSrc; *lpcName; ++lpcName)
    {
        if (isdigit(*lpcName) == 0 && '.' != *lpcName)
        {
            return -1;
        }
    }

    lpcName = apcSrc;
    char * lpcNext;
    while(*lpcName == '.')
    {   
        lpcName++;
    }  

    for (int i = 0; i < aiSize; ++i)
    {   
        errno = 0;
        int c = strtoul(lpcName, &lpcNext, 10);
              
        sprintf(apcDes+i,"%c",c);
        if (errno != 0) 
        {
            return -1;
        }

        ++liCounter;
        if (*lpcNext == '\0')
        {
            break;
        }
        if (*lpcNext != '.')
        {
            return -1;
        }

        lpcName = lpcNext + 1;
    }  
    return liCounter;
}


//build variable_binding
unsigned char  * SnmpBuildVarOp(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned long * apiName,int aiNameLen, unsigned char acVarValType, char * apcVal,int aiValLen)
{
    size_t              liLength;
    unsigned char       *lpcH,*lpcHe = 0;

    lpcH = apcBuffer;
    liLength = *apiMaxSize;

    apcBuffer= BerBuildSequence(apcBuffer, apiMaxSize,(unsigned char) (AI_BER_SEQUENCE), 0); 
    if (apcBuffer== NULL)
    {
        return NULL;
    }
    lpcHe = apcBuffer; 
    
    apcBuffer = BerBuildObjid(apcBuffer, apiMaxSize,(unsigned char) (AI_BER_OBJECT_ID), apiName,aiNameLen);

    if (apcBuffer == NULL) 
    {
//        printf("Can't build OID for variable");
        return NULL;
    }

    switch (acVarValType) 
    {
    case AI_BER_GAUGE:
    case AI_BER_COUNTER:
    case AI_BER_TIMETICKS:
        apcBuffer = BerBuildUnsignedInt(apcBuffer, apiMaxSize, acVarValType,(unsigned long *) apcVal, aiValLen);
        break;

    case AI_BER_OCTET_STR:
    case AI_BER_OPAQUE:
        apcBuffer = BerBuildString(apcBuffer, apiMaxSize, acVarValType,apcVal, aiValLen);
        break;
        
    case AI_BER_IPADDRESS:
        {
        char liAddress[4];
        memset(&liAddress,0,sizeof(liAddress));
        int liAddLen = CutAddr(apcVal,liAddress,4);

        apcBuffer= BerBuildString(apcBuffer, apiMaxSize,acVarValType,(char *) liAddress, liAddLen);
        break;
        }

    case AI_BER_OBJECT_ID:
        {
        unsigned long liObjidVar[AI_SNMP_OID_NUM];
        aiValLen=ArrToOid(apcVal,liObjidVar,AI_SNMP_OID_NUM); 
        apcBuffer = BerBuildObjid(apcBuffer, apiMaxSize, acVarValType,(unsigned long *)liObjidVar,aiValLen );
        break;
        }

    case AI_BER_INTEGER:
        apcBuffer = BerBuildInt(apcBuffer, apiMaxSize, acVarValType,(long *) apcVal, aiValLen);
        break;

    case AI_BER_NULL:
        apcBuffer = BerBuildNull(apcBuffer, apiMaxSize, acVarValType);
        break;
        
    case AI_BER_BIT_STR:
        apcBuffer = BerBuildBitstring(apcBuffer, apiMaxSize, acVarValType,apcVal, aiValLen);
        break;
        
    default:
        {
            apcBuffer = NULL;
            break;
        }
    }

    if (apcBuffer == NULL) 
    {
        return NULL;
    }

    BerBuildSequence(lpcH, &liLength,(unsigned char) (AI_BER_SEQUENCE),apcBuffer- lpcHe);
    return apcBuffer;
}

int SnmpPduBuild(stSnmpPdu *apoPdu, unsigned char * apcBuffer, size_t aiMaxSize)
{
    size_t           liLength1;
    unsigned char    *lpcH0,*lpcH0e = 0;

    lpcH0=apcBuffer;
    liLength1=aiMaxSize;

    apcBuffer= BerBuildSequence(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_SEQUENCE), 0); 
    if(apcBuffer== NULL)
    {
        return AI_SNMP_ERRSEQ;
    }
    lpcH0e = apcBuffer;

    //build version
    apcBuffer = BerBuildInt(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_INTEGER), (long *) &apoPdu->ciVersion,sizeof(apoPdu->ciVersion));
    if (apcBuffer== NULL)
    {
        return AI_SNMP_ERRINT;
    }
    
    //build community
    apcBuffer= BerBuildString(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_OCTET_STR), (char *)&apoPdu->csCommunity,strlen(apoPdu->csCommunity));
    if(apcBuffer == NULL)
    {
        return AI_SNMP_ERRSTR;
    }

    size_t        liLength2;
    unsigned char *lpcH1,*lpcH1e;

    liLength2 = aiMaxSize;
    lpcH1 = apcBuffer;
    apcBuffer = BerBuildSequence(apcBuffer, &aiMaxSize, (unsigned char) apoPdu->ciCommand, 0);
    
    //test request command
    if( apoPdu->ciCommand !=0xA0 && apoPdu->ciCommand !=0xA1)
    {
        return AI_SNMP_EWRONGCOMMAND;
    }

    if (apcBuffer== NULL)
    {
        return AI_SNMP_ERRSEQ;
    }
    lpcH1e = apcBuffer;

    //build request id  
    apcBuffer= BerBuildInt(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_INTEGER), &apoPdu->ciReqid,sizeof(apoPdu->ciReqid));
    if (apcBuffer== NULL)
    {
        return AI_SNMP_ERRINT;
    }

    //build error status
    apcBuffer= BerBuildInt(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_INTEGER), &apoPdu->ciErrstat,sizeof(apoPdu->ciErrstat));
    if (apcBuffer== NULL)
    {
        return AI_SNMP_ERRINT;
    }

    //build error index
    apcBuffer = BerBuildInt(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_INTEGER), &apoPdu->ciErrindex,sizeof(apoPdu->ciErrindex));
    if (apcBuffer == NULL)
    {
        return AI_SNMP_ERRINT;
    }

    size_t          liLength3;
    unsigned char   *lpcH2,*lpcH2e;

    liLength3 = aiMaxSize;
    lpcH2 = apcBuffer;
    apcBuffer= BerBuildSequence(apcBuffer,&aiMaxSize,(unsigned char) (AI_BER_INTEGER), 0); 
    if(apcBuffer == NULL)
    {
        return AI_SNMP_ERRSEQ;
    }
    lpcH2e = apcBuffer;

    //build variable-bindings
    stVariableBinding loVp;
    
    int i;
    for(i=0;i<AI_SNMP_MAX_VARLIST_NUM;i++)
    {
        loVp=apoPdu->coVariables[i];
        
        if (loVp.ciValLen == 0)
        {
            break;
        }
        unsigned long liObjidVar[AI_SNMP_OID_NUM];
        int liNameLen=ArrToOid(loVp.csName,liObjidVar, AI_SNMP_OID_NUM);

        apcBuffer = SnmpBuildVarOp(apcBuffer, &aiMaxSize,liObjidVar,liNameLen, loVp.ccType,( char *) loVp.coVal.csString,loVp.ciValLen); 
        if (apcBuffer== NULL)
        {
            return AI_SNMP_ERRVARB;
        }
    }

        if ((apcBuffer- lpcH2e) < 0x80)
        {
            BerBuildSequence(lpcH2, &liLength3,(unsigned char) (AI_BER_SEQUENCE),apcBuffer- lpcH2e);
        } 
    
        else if ((apcBuffer - lpcH2e) <= 0xFF) 
        {
            memmove(lpcH2e + 1 , lpcH2e , apcBuffer - lpcH2e );
            liLength3 -= 1;
            BerBuildSequence(lpcH2, &liLength3,(unsigned char) (AI_BER_SEQUENCE),apcBuffer- lpcH2e);
            apcBuffer++;
        } 
        
        else if ((apcBuffer - lpcH2e) <= 0xFFFF) 
        {
            memmove(lpcH2e + 2 , lpcH2e , apcBuffer - lpcH2e );
            liLength3 -= 2;
            BerBuildSequence(lpcH2, &liLength3,(unsigned char) (AI_BER_SEQUENCE),apcBuffer- lpcH2e);
            apcBuffer+=2;
        } 
        
        if ((apcBuffer - lpcH1e) < 0x80)
        {
            BerBuildSequence(lpcH1, &liLength2, (unsigned char) apoPdu->ciCommand, apcBuffer - lpcH1e);
        } 
    
        else if ((apcBuffer - lpcH1e) <= 0xFF) 
        {
            memmove(lpcH1e + 1 , lpcH1e , apcBuffer - lpcH1e );
            liLength2 -= 1;
            BerBuildSequence(lpcH1, &liLength2, (unsigned char) apoPdu->ciCommand, apcBuffer - lpcH1e);
            apcBuffer++;
        } 
        
        else if ((apcBuffer - lpcH1e) <= 0xFFFF) 
        {
            memmove(lpcH1e + 2 , lpcH1e , apcBuffer - lpcH1e );
            liLength2 -= 2;
            BerBuildSequence(lpcH1, &liLength2, (unsigned char) apoPdu->ciCommand, apcBuffer - lpcH1e);
            apcBuffer+=2;
        } 
        
        else if ((apcBuffer - lpcH1e) <= 0xFFFFFF) 
        {
            memmove(lpcH1e + 3 , lpcH1e , apcBuffer - lpcH1e );
            liLength2 -= 3;
            BerBuildSequence(lpcH1, &liLength2, (unsigned char) apoPdu->ciCommand, apcBuffer - lpcH1e);
            apcBuffer+=3;
        } 
        
        if ((apcBuffer - lpcH0e) < 0x80)
        {
            BerBuildSequence(lpcH0, &liLength1,(unsigned char) (AI_BER_SEQUENCE),apcBuffer - lpcH0e);
            aiMaxSize = apcBuffer - lpcH0;
        } 
    
        else if ((apcBuffer - lpcH0e) <= 0xFF) 
        {
            memmove(lpcH0e + 1 , lpcH0e , apcBuffer - lpcH0e );
            liLength1 -= 1;
            BerBuildSequence(lpcH0, &liLength1,(unsigned char) (AI_BER_SEQUENCE),apcBuffer - lpcH0e );
            aiMaxSize = apcBuffer - lpcH0 + 1 ;
        } 
    
        else if ((apcBuffer - lpcH0e) <= 0xFFFF) 
        {
            memmove(lpcH0e + 2 , lpcH0e , apcBuffer - lpcH0e );
            liLength1 -= 2;
            BerBuildSequence(lpcH0, &liLength1,(unsigned char) (AI_BER_SEQUENCE),apcBuffer - lpcH0e );
            aiMaxSize = apcBuffer - lpcH0 + 2 ;
        } 
        
        else if ((apcBuffer - lpcH0e) <= 0xFFFFFF) 
        {
            memmove(lpcH0e + 3 , lpcH0e , apcBuffer - lpcH0e );
            liLength1 -= 3;
            BerBuildSequence(lpcH0, &liLength1,(unsigned char) (AI_BER_SEQUENCE),apcBuffer - lpcH0e );
            aiMaxSize = apcBuffer - lpcH0 + 3 ;
        } 
        
        else if ((unsigned)(apcBuffer - lpcH0e) <= 0xFFFFFFFFul) 
        {
            memmove(lpcH0e + 4 , lpcH0e , apcBuffer - lpcH0e );
            liLength1 -= 4;
            BerBuildSequence(lpcH0, &liLength1,(unsigned char) (AI_BER_SEQUENCE),apcBuffer - lpcH0e );
            aiMaxSize = apcBuffer - lpcH0 + 4 ;
        } 

    return aiMaxSize;
}


int SnmpTrapBuild(stSnmpTrap *apoPdu, unsigned char * apcBuffer, size_t  aiMaxSize)
{
    size_t           liLength;
    unsigned char    *lpcH,*lpcHe = 0;

    lpcH=apcBuffer;
    liLength=aiMaxSize;

    apcBuffer= BerBuildSequence(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_SEQUENCE), 0); 
    if(apcBuffer== NULL)
    {
        return AI_SNMP_ERRSEQ;
    }
    lpcHe = apcBuffer;

    //build version
    apcBuffer = BerBuildInt(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_INTEGER), (long *) &apoPdu->ciVersion,sizeof(apoPdu->ciVersion));
    if (apcBuffer== NULL)
    {
        return AI_SNMP_ERRINT;
    }
    
    //build community
    apcBuffer= BerBuildString(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_OCTET_STR), (char *)&apoPdu->csCommunity,strlen(apoPdu->csCommunity));
    if(apcBuffer == NULL)
    {
        return AI_SNMP_ERRSTR;
    }
    
    size_t          liLength1;
    unsigned char   *lpcH0,*lpcH0e = 0;
    liLength1 = aiMaxSize;
    
    lpcH0 = apcBuffer;
    apcBuffer = BerBuildSequence(apcBuffer, &aiMaxSize, (unsigned char) (AI_SNMP_BER_TRAP), 0);
    if (apcBuffer == NULL)
    {
        return AI_SNMP_ERRSEQ;
    }
    lpcH0e = apcBuffer;

    //enterprise 
    unsigned long liObjidVar[AI_SNMP_OID_NUM];
    
    int liEnterpriseLen = ArrToOid(apoPdu->csEnterprise,liObjidVar, AI_SNMP_OID_NUM);
    apcBuffer= BerBuildObjid(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_OBJECT_ID),(unsigned long *) liObjidVar,liEnterpriseLen);
    if (apcBuffer== NULL)
    {
        return AI_SNMP_ERROID;
    }
    
    //agent address
    char liAddress[4];
    memset(&liAddress,0,sizeof(liAddress));
    int liAddLen = CutAddr(apoPdu->csAgentAddr,liAddress,4);
    apcBuffer= BerBuildString(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_IPADDRESS),(char *) liAddress, liAddLen); 
    if (apcBuffer== NULL)
    {
        return AI_SNMP_ERRSTR;
    }
    
    //trap type
    apcBuffer = BerBuildInt(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_INTEGER),(long *) &apoPdu->ciTrapType,sizeof(apoPdu->ciTrapType));
    if (apcBuffer== NULL)
    {
        return AI_SNMP_ERRINT;
    }
    
    //special
    apcBuffer = BerBuildInt(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_INTEGER),(long *) &apoPdu->ciSpecificType,sizeof(apoPdu->ciSpecificType));
    if (apcBuffer == NULL)
    {
        return AI_SNMP_ERRINT;
    }
    
    //timestamp
    apcBuffer= BerBuildUnsignedInt(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_TIMETICKS), &apoPdu->ciTime,sizeof(apoPdu->ciTime));
    if (apcBuffer == NULL)
    {
        return AI_SNMP_ERRUINT;
    }

    size_t          liLength2;
    unsigned char   *lpcH1,*lpcH1e;
    
    liLength2 = aiMaxSize;
    lpcH1 = apcBuffer;
    apcBuffer= BerBuildSequence(apcBuffer, &aiMaxSize,(unsigned char) (AI_BER_SEQUENCE), 0); 
    if (apcBuffer == NULL)
    {
        return AI_SNMP_ERRSEQ;
    }
    lpcH1e = apcBuffer;
    
    //store variable-bindings
    stVariableBinding loVp;
    
    for(int i=0;i<AI_SNMP_MAX_VARLIST_NUM;i++)
    {
        loVp = apoPdu->coVariables[i];
        if (loVp.ciValLen == 0)
        {
            break;
        }

        unsigned long liObjidVar[AI_SNMP_OID_NUM];
        int liNameLength=ArrToOid(loVp.csName,liObjidVar,AI_SNMP_OID_NUM);
        
        apcBuffer = SnmpBuildVarOp(apcBuffer, &aiMaxSize,liObjidVar,liNameLength, loVp.ccType,( char *) loVp.coVal.csString,loVp.ciValLen); 
        if (apcBuffer == NULL)
        {
            return AI_SNMP_ERRVARB;
        }
    }

    if ((apcBuffer - lpcH1e) < 0x80)
    {
        BerBuildSequence(lpcH1, &liLength2,(unsigned char) (AI_BER_SEQUENCE),apcBuffer- lpcH1e);
    } 
    
    else if ((apcBuffer - lpcH1e) <= 0xFF) 
    {
        memmove(lpcH1e + 1 , lpcH1e , apcBuffer - lpcH1e );
        liLength2 -= 1;
        BerBuildSequence(lpcH1, &liLength2,(unsigned char) (AI_BER_SEQUENCE),apcBuffer- lpcH1e);
        apcBuffer++;
    } 
    else if ((apcBuffer - lpcH1e) <= 0xFFFF) 
    {
        memmove(lpcH1e + 2 , lpcH1e , apcBuffer - lpcH1e );
        liLength2 -= 2;
        BerBuildSequence(lpcH1, &liLength2,(unsigned char) (AI_BER_SEQUENCE),apcBuffer- lpcH1e);
        apcBuffer += 2;
    } 

    //-----------------------

    if ((apcBuffer - lpcH0e) < 0x80)
    {
        BerBuildSequence(lpcH0, &liLength1, (unsigned char) (AI_SNMP_BER_TRAP), apcBuffer - lpcH0e);
        //aiMaxSize = apcBuffer - lpcH0;
    } 
    
    else if ((apcBuffer - lpcH0e) <= 0xFF) 
    {
        memmove(lpcH0e + 1 , lpcH0e , apcBuffer - lpcH0e );
        liLength1 -= 1;
        BerBuildSequence(lpcH0, &liLength1, (unsigned char) (AI_SNMP_BER_TRAP), apcBuffer - lpcH0e);
        //aiMaxSize = apcBuffer - lpcH0 + 1 ;
        apcBuffer++;
    } 
    
    else if ((apcBuffer - lpcH0e) <= 0xFFFF) 
    {
        memmove(lpcH0e + 2 , lpcH0e , apcBuffer - lpcH0e );
        liLength1 -= 2;
        BerBuildSequence(lpcH0, &liLength1, (unsigned char) (AI_SNMP_BER_TRAP), apcBuffer - lpcH0e);
        //aiMaxSize = apcBuffer - lpcH0 + 2 ;
        apcBuffer+=2;
    } 
    else if ((apcBuffer - lpcH0e) <= 0xFFFFFF) 
    {
        memmove(lpcH0e + 3 , lpcH0e , apcBuffer - lpcH0e );
        liLength1 -= 3;
        BerBuildSequence(lpcH0, &liLength1, (unsigned char) (AI_SNMP_BER_TRAP), apcBuffer - lpcH0e);
        //aiMaxSize = apcBuffer - lpcH0 + 3 ;
        apcBuffer+=3;
    } 

    //--------------------------------

    if ((apcBuffer - lpcHe) < 0x80)
    {
        BerBuildSequence(lpcH, &liLength,(unsigned char) (AI_BER_SEQUENCE),apcBuffer - lpcHe);
        aiMaxSize = apcBuffer - lpcH;
    } 

    else if ((apcBuffer - lpcHe) <= 0xFF) 
    {
        memmove(lpcHe + 1 , lpcHe , apcBuffer - lpcHe );
        liLength -= 1;
        BerBuildSequence(lpcH, &liLength,(unsigned char) (AI_BER_SEQUENCE),apcBuffer - lpcHe );
        aiMaxSize = apcBuffer - lpcH + 1 ;
    } 

    else if ((apcBuffer - lpcHe) <= 0xFFFF) 
    {
        memmove(lpcHe + 2 , lpcHe , apcBuffer - lpcHe );
        liLength -= 2;
        BerBuildSequence(lpcH, &liLength,(unsigned char) (AI_BER_SEQUENCE),apcBuffer - lpcHe );
        aiMaxSize = apcBuffer - lpcH + 2 ;
    } 
    
    else if ((apcBuffer - lpcHe) <= 0xFFFFFF) 
    {
        memmove(lpcHe + 3 , lpcHe , apcBuffer - lpcHe );
        liLength -= 3;
        BerBuildSequence(lpcH, &liLength,(unsigned char) (AI_BER_SEQUENCE),apcBuffer - lpcHe );
        aiMaxSize = apcBuffer - lpcH + 3 ;
    } 
    
    else if ((unsigned)(apcBuffer - lpcHe) <= 0xFFFFFFFFul) 
    {
        memmove(lpcHe + 4 , lpcHe , apcBuffer - lpcHe );
        liLength -= 4;
        BerBuildSequence(lpcH, &liLength,(unsigned char) (AI_BER_SEQUENCE),apcBuffer - lpcHe );
        aiMaxSize = apcBuffer - lpcH + 4 ;
    } 

    return aiMaxSize;
}

int SnmpSetVarObjid(stVariableBinding * apoVp, unsigned long * apiObjid, int aiNameLen)
{
    if (apiObjid[0] != 0)
    {    
        char* lpcName = apoVp->csName;
        for(int i=0;i<(aiNameLen-1);i++)
        {
            int liSetVal = sprintf(lpcName,"%lu.",apiObjid[i]);
            if (liSetVal < 0)
            {
                return AI_SNMP_ERRSETV;
            }
            lpcName += liSetVal;
        }
        sprintf(lpcName,"%lu",apiObjid[aiNameLen-1]);

    }
    return 0;
}

unsigned char *SnmpParseVarOp(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned long * apiName,int  *apiNameLen,unsigned char * apcVarValType,unsigned char ** appcVal,int * apiValLen )
{
    unsigned char          lcVarOpType;
    size_t                 liVarOpLen = *apiMaxSize;
    unsigned char          *lpcVarOpStart = apcBuffer;

    apcBuffer = BerParseSequence(apcBuffer, &liVarOpLen, &lcVarOpType,(unsigned char)(AI_BER_SEQUENCE));
    if (apcBuffer == NULL) 
    {
        return NULL;
    }

    apcBuffer =BerParseObjid(apcBuffer, &liVarOpLen, &lcVarOpType, apiName,apiNameLen);
    if (apcBuffer == NULL) 
    {
//        printf("No OID for variable");
        return NULL;
    }

    if (lcVarOpType !=(unsigned char) (AI_BER_OBJECT_ID))
    {
        return NULL;
    }

    *appcVal = apcBuffer;            
    apcBuffer = BerParseHeader(apcBuffer, &liVarOpLen, apcVarValType);
    if (apcBuffer == NULL) 
    {
        return NULL;
    }

    *apiValLen = liVarOpLen;
    apcBuffer += liVarOpLen;
    *apiMaxSize -= (size_t) (apcBuffer - lpcVarOpStart);
    return apcBuffer;
}

int SnmpPduParse(stSnmpPdu  *apoPdu, unsigned char * apcBuffer, size_t aiMaxSize)
{
    unsigned char      lcType;
    long               liVersion = -1;
    size_t             liLen = 0;
    unsigned char      *lpcVarVal;
    stVariableBinding   *lpoVp = NULL;
    unsigned long      lsObjid[128];
    memset(lsObjid,0,sizeof(lsObjid));

    //version
    apcBuffer = BerParseSequence(apcBuffer, &aiMaxSize, &lcType,(char)(AI_BER_SEQUENCE));
    apcBuffer = BerParseInt(apcBuffer, &aiMaxSize, &lcType, &liVersion, sizeof(liVersion));

    if (!apcBuffer || lcType != (char)(AI_BER_INTEGER)) 
    {
        return AI_SNMP_ERRINTP;
    }
    
    apoPdu->ciVersion = liVersion;

    //community
    apcBuffer=BerParseString (apcBuffer, &aiMaxSize, &lcType, (char *)&apoPdu->csCommunity, sizeof(apoPdu->csCommunity));
    if (!apcBuffer || lcType != (char)(AI_BER_OCTET_STR))
    {
        return AI_SNMP_ERRSTRP;
    }

    //Get the PDU type 
    unsigned char          lcMsgType;
    apcBuffer = BerParseHeader(apcBuffer, &aiMaxSize, &lcMsgType);
    if (apcBuffer == NULL)
    {
        return AI_SNMP_ERRHEADP;
    }
    apoPdu->ciCommand = lcMsgType;

    // request id 
    apcBuffer = BerParseInt(apcBuffer, &aiMaxSize, &lcType, &apoPdu->ciReqid,sizeof(apoPdu->ciReqid));
    if (!apcBuffer || lcType != (char)(AI_BER_INTEGER)) 
    {
         return AI_SNMP_ERRINTP;
    }

    //error status (getbulk non-repeaters) 
    apcBuffer = BerParseInt(apcBuffer, &aiMaxSize, &lcType, &apoPdu->ciErrstat,sizeof(apoPdu->ciErrstat));
    if (!apcBuffer || lcType != (char)(AI_BER_INTEGER)) 
    {
        return AI_SNMP_ERRINTP;
    }
    
    //error index (getbulk max-repetitions) 
    apcBuffer = BerParseInt(apcBuffer, &aiMaxSize, &lcType, &apoPdu->ciErrindex,sizeof(apoPdu->ciErrindex));
    if (!apcBuffer || lcType != (char)(AI_BER_INTEGER)) 
    {
        return AI_SNMP_ERRINTP;
    }

    //get header for variable-bindings sequence 
    apcBuffer = BerParseSequence(apcBuffer, &aiMaxSize, &lcType,(char)(AI_BER_SEQUENCE));
    if (!apcBuffer || lcType != (char)(AI_BER_SEQUENCE))
    {
        return AI_SNMP_ERRSEQP;
    }
      
    for (int i=0;i < AI_SNMP_MAX_VARLIST_NUM;++i)
    {
        if (aiMaxSize <= 0)
        {
            apoPdu->ciVBNum = i; 
            break;
        }
              
        lpoVp = &(apoPdu->coVariables[i]);
              
        memset(lpoVp->csName,0,sizeof(lpoVp->csName));
        memset(lpoVp->coVal.csString,0,sizeof(lpoVp->coVal.csString));
        int liNameLen = sizeof(lsObjid) / sizeof(unsigned long);
              
        apcBuffer = SnmpParseVarOp(apcBuffer, &aiMaxSize, lsObjid, &liNameLen, &lpoVp->ccType, &lpcVarVal,&lpoVp->ciValLen);
        if (apcBuffer == NULL)
        {
            return AI_SNMP_ERRVARBP;
        }
              
        SnmpSetVarObjid(lpoVp, lsObjid, liNameLen);
          
        switch ((short) lpoVp->ccType) 
        {
        case AI_BER_INTEGER:
            lpoVp->ciValLen = sizeof(int);
            BerParseInt(lpcVarVal, &liLen, &lpoVp->ccType,(long *) &lpoVp->coVal.ciInteger,sizeof(lpoVp->coVal.ciInteger));
            break;

        case AI_BER_COUNTER:
        case AI_BER_GAUGE:
        case AI_BER_TIMETICKS:
            lpoVp->ciValLen = sizeof(unsigned long);
            BerParseUnsignedInt(lpcVarVal, &liLen, &lpoVp->ccType,(unsigned long *) &lpoVp->coVal.ciInteger,lpoVp->ciValLen);
            break;

        case AI_BER_OCTET_STR:
        case AI_BER_OPAQUE:
            BerParseString(lpcVarVal, &liLen, &lpoVp->ccType, (char *)&lpoVp->coVal.csString,lpoVp->ciValLen);
            break;

        case AI_BER_IPADDRESS:
            //int liAgeLen=4;
            //BerParseAgentAddr(lpcVarVal, &liLen, &lpoVp->ccType,(char *) lpoVp->coVal.csString, liAgeLen);
            BerParseString(lpcVarVal, &liLen, &lpoVp->ccType, (char *)&lpoVp->coVal.csString,lpoVp->ciValLen);
            break;

        case AI_BER_OBJECT_ID:
            unsigned long   liObjid2[AI_SNMP_OID_NUM];
            lpoVp->ciValLen = AI_SNMP_OID_NUM;
            BerParseObjid(lpcVarVal, &liLen, &lpoVp->ccType, liObjid2, &lpoVp->ciValLen);
            if (liObjid2)
            {    
                char* lpcOid = lpoVp->coVal.csObjid;
                for(int i=0;i<(lpoVp->ciValLen-1);i++)
                {
                    int liOid = sprintf(lpcOid,"%lu.",liObjid2[i]);
                    if (liOid < 0)
                    {
                         return AI_SNMP_ERRCOV;
                    }
                    lpcOid += liOid;
                }
                sprintf(lpcOid,"%lu",liObjid2[lpoVp->ciValLen-1]);
            }
            break;

        case AI_BER_NULL:
            break;

        case AI_BER_BIT_STR:
            BerParseBitstring(lpcVarVal, &liLen, &lpoVp->ccType,lpoVp->coVal.csBitstring, &lpoVp->ciValLen);
            break;

        default:
            return AI_SNMP_ERRTYPE;
            break;
         }
    }
          return 0;
}
int SnmpTrapParse(stSnmpTrap *apoPdu, unsigned char * apcBuffer, size_t aiMaxSize)
{
    long                    liVersion = -1;
    unsigned char           lcType;
    unsigned char           lcMsgType;
    unsigned char           *lpcVarVal;
    stVariableBinding       *lpoVp = NULL;
    unsigned long           lsObjid[AI_SNMP_OID_NUM];
    memset(lsObjid, 0, sizeof(lsObjid));
    size_t                  liLen = 0;

    //version
    apcBuffer = BerParseSequence(apcBuffer, &aiMaxSize, &lcType,(char)(AI_BER_SEQUENCE));
    apcBuffer = BerParseInt(apcBuffer, &aiMaxSize, &lcType, &liVersion, sizeof(liVersion));

    if (!apcBuffer || lcType != (char)(AI_BER_INTEGER)) 
    {
        return AI_SNMP_ERRINTP;
    }
                    
    apoPdu->ciVersion = liVersion;

    //community
    apcBuffer=BerParseString (apcBuffer, &aiMaxSize, &lcType, (char *)&apoPdu->csCommunity, sizeof(apoPdu->csCommunity));
    if (!apcBuffer || lcType != (char)(AI_BER_OCTET_STR))
    {
        return AI_SNMP_ERRSTRP;
    }

    apcBuffer = BerParseHeader(apcBuffer, &aiMaxSize, &lcMsgType);
    if (apcBuffer == NULL)
    {
        return AI_SNMP_ERRHEADP;
    }
    apoPdu->ciCommand = lcMsgType;

    int liEnterpriseLen =AI_SNMP_OID_NUM;
    apcBuffer = BerParseObjid(apcBuffer, &aiMaxSize, &lcType, lsObjid,&liEnterpriseLen);
    if (apcBuffer == NULL)
    {
        return AI_SNMP_ERROIDP;
    }

    if (lsObjid[0] != 0)
    {    
        char * lpcOid = apoPdu->csEnterprise;

        for(int i=0;i<(liEnterpriseLen-1);i++)
        {
            int liOid = sprintf(lpcOid,"%lu.",lsObjid[i]);
            if (liOid < 0)
            {
                return AI_SNMP_ERRCOV;
            }
            lpcOid += liOid;
        }
        sprintf(lpcOid,"%lu",lsObjid[liEnterpriseLen-1]);
    }

    int liAgeLen=4;
    apcBuffer = BerParseAgentAddr(apcBuffer, &aiMaxSize, &lcType,(char *) apoPdu->csAgentAddr, liAgeLen);
    if (apcBuffer == NULL)
    {
        return AI_SNMP_ERRSTRP;
    }
    
    apcBuffer = BerParseInt(apcBuffer, &aiMaxSize, &lcType, (long *) &apoPdu->ciTrapType, sizeof(apoPdu->ciTrapType));
    if (apcBuffer == NULL)
    {
        return AI_SNMP_ERRINTP;
    }
    apcBuffer =BerParseInt(apcBuffer, &aiMaxSize, &lcType,(long *) &apoPdu->ciSpecificType,sizeof(apoPdu->ciSpecificType));
    if (apcBuffer == NULL)
    {
        return AI_SNMP_ERRINTP;
    }

    apcBuffer = BerParseUnsignedInt(apcBuffer, &aiMaxSize, &lcType, &apoPdu->ciTime,sizeof(apoPdu->ciTime));
    if (apcBuffer == NULL)
    {
        return AI_SNMP_ERRUINTP;
    }

    //get header for variable-bindings sequence 
    apcBuffer = BerParseSequence(apcBuffer, &aiMaxSize, &lcType,(unsigned char)(AI_BER_SEQUENCE));
    if (!apcBuffer||lcType != (unsigned char)(AI_BER_SEQUENCE))
    {
         return AI_SNMP_ERRSEQP;
    }
  
    for (int i=0;i < AI_SNMP_MAX_VARLIST_NUM;++i)
    {
        if (aiMaxSize <= 0)
        {
            apoPdu->ciVBNum = i;
            break;
        }
         
        lpoVp=&(apoPdu->coVariables[i]);
          
        memset(lpoVp->csName,0,sizeof(lpoVp->csName));
        memset(lpoVp->coVal.csString,0,sizeof(lpoVp->coVal.csString));
        int liNameLength=sizeof(lsObjid)/sizeof(unsigned long);
        apcBuffer = SnmpParseVarOp(apcBuffer, &aiMaxSize, lsObjid, &liNameLength, &lpoVp->ccType, &lpcVarVal,&lpoVp->ciValLen);
        if (apcBuffer == NULL)
        {
            return AI_SNMP_ERRVARBP;
        }
          
        SnmpSetVarObjid(lpoVp, lsObjid, liNameLength);
    
        switch ((short) lpoVp->ccType) 
        {
        case AI_BER_INTEGER:
            lpoVp->ciValLen = sizeof(int);
            BerParseInt(lpcVarVal, &liLen, &lpoVp->ccType,(long *) &lpoVp->coVal.ciInteger,sizeof(lpoVp->coVal.ciInteger));
            break;

        case AI_BER_COUNTER:
        case AI_BER_GAUGE:
        case AI_BER_TIMETICKS:
            lpoVp->ciValLen = sizeof(unsigned long);
            BerParseUnsignedInt(lpcVarVal, &liLen, &lpoVp->ccType,(unsigned long *) &lpoVp->coVal.ciInteger,lpoVp->ciValLen);
            break;
        
        case AI_BER_OCTET_STR:
        case AI_BER_OPAQUE:
            BerParseString(lpcVarVal, &liLen, &lpoVp->ccType, (char *)&lpoVp->coVal.csString,lpoVp->ciValLen);
            break;
        
        case AI_BER_IPADDRESS:
            BerParseAgentAddr(lpcVarVal, &liLen, &lpoVp->ccType,(char *) lpoVp->coVal.csString, liAgeLen);
            break;

        case AI_BER_OBJECT_ID:
            {
            unsigned long   liObjid2[AI_SNMP_OID_NUM];
            memset(liObjid2, 0, sizeof(liObjid2));
            lpoVp->ciValLen = AI_SNMP_OID_NUM;
            BerParseObjid(lpcVarVal, &liLen, &lpoVp->ccType, liObjid2, &lpoVp->ciValLen);
            if (liObjid2[0] != 0)
            {    
                char* lpcName = lpoVp->coVal.csObjid;
                
                for(int i=0;i<(lpoVp->ciValLen-1);i++)
                {
                    int liOid = sprintf(lpcName,"%lu.",liObjid2[i]);
                    if (liOid < 0)
                    {
                        return AI_SNMP_ERRCOV;
                    }
                    lpcName += liOid;
                }
                sprintf(lpcName,"%lu",liObjid2[lpoVp->ciValLen-1]);
            }
            break;
            }

        case AI_BER_NULL:
            break;

        case AI_BER_BIT_STR:
            BerParseBitstring(lpcVarVal, &liLen, &lpoVp->ccType,lpoVp->coVal.csBitstring, &lpoVp->ciValLen);
            break;

        default:
            return AI_SNMP_ERRTYPE;
            break;
        }
    }
    return 0;
}

int ShowReceivedData(stSnmpPdu * apoPdu)
{
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"version=%ld",apoPdu->ciVersion);
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"community= %s", apoPdu->csCommunity);
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"command=%2X",apoPdu->ciCommand);
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"reqid=%ld",apoPdu->ciReqid);
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"errstat=%ld",apoPdu->ciErrstat);
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"errindex=%ld",apoPdu->ciErrindex);

    for(int i=0;i<apoPdu->ciVBNum;i++)
    {
        AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"apoPdu->coVariables[%d].csName=%s",i,apoPdu->coVariables[i].csName);
        AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"coVariables[%d].ccType=%d",i,apoPdu->coVariables[i].ccType);
        if(apoPdu->coVariables[i].ccType==0x05)
        {
            _StringCopy(apoPdu->coVariables[i].coVal.csString,"NULL");
            AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"coVariables[%d].coVal=%s",i,apoPdu->coVariables[i].coVal.csString);
        }
        else if(apoPdu->coVariables[i].ccType!=0x02 && apoPdu->coVariables[i].ccType!=0x41 && apoPdu->coVariables[i].ccType!=0x43)
        {
            AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"coVariables[%d].coVal=%s",i,apoPdu->coVariables[i].coVal.csString);
        }
        else
        {
            AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"coVariables[%d].coVal.ciInteger=%ld",i,apoPdu->coVariables[i].coVal.ciInteger);
        }
    }
    AIWriteLOG(AILIB_LOGFILE, AILOG_LEVEL_TRACE,"------------------------------------------");
    return 0;
}

/*
int ShowTrapRecvData(stSnmpTrap *apoTrap)
{
    printf("version=%ld\n",apoTrap->ciVersion);
    printf("community=%s\n", apoTrap->csCommunity);
    printf("command=%2X\n",apoTrap->ciCommand);
    printf("csEnterprise=%s\n",apoTrap->csEnterprise);
    printf("csAgentAddr=%s\n",apoTrap->csAgentAddr);
    printf("ciTrapType=%ld\n",apoTrap->ciTrapType);
    printf("ciSpecificType=%ld\n",apoTrap->ciSpecificType);
    printf("ciTime=%lu\n",apoTrap->ciTime);
    for(int i=0;i<apoTrap->ciVBNum;i++)
    {
        printf("apoTrap->coVariables[%d].csName=%s\n",i,apoTrap->coVariables[i].csName);
        printf("coVariables[%d].ccType=%d\n",i,apoTrap->coVariables[i].ccType);

        if(apoTrap->coVariables[i].ccType==0x05)
        {
            _StringCopy(apoTrap->coVariables[i].coVal.csString,"NULL");
            printf("coVariables[%d].coVal=%s\n",i,apoTrap->coVariables[i].coVal.csString);
        }
        else if(apoTrap->coVariables[i].ccType!=0x02 && apoTrap->coVariables[i].ccType!=0x41 && apoTrap->coVariables[i].ccType!=0x43)
        {
            printf("coVariables[%d].coVal=%s\n",i,apoTrap->coVariables[i].coVal.csString);
        }
        else
        {
            printf("coVariables[%d].coVal.ciInteger=%ld\n",i,apoTrap->coVariables[i].coVal.ciInteger);
        }
    }
    return 0;
}
*/
int SnmpAddVar(stSnmpTrap *apoTrapPdu, char *apcOidName, unsigned char acType, char *apcValue)
{
    stVariableBinding loVp;
        
    int i;
    for(i=0;i<AI_SNMP_MAX_VARLIST_NUM;i++)
    {
        loVp=apoTrapPdu->coVariables[i];
                                       
        if (loVp.ciValLen == 0)
        {
            _StringCopy(apoTrapPdu->coVariables[i].csName,apcOidName);

            switch(acType)
            {
            case 'i':
                apoTrapPdu->coVariables[i].ccType = AI_BER_INTEGER;
                apoTrapPdu->coVariables[i].coVal.ciInteger=strtol(apcValue,NULL,10);
                apoTrapPdu->coVariables[i].ciValLen=sizeof(apoTrapPdu->coVariables[i].coVal.ciInteger);
                break;

            case 's':
                apoTrapPdu->coVariables[i].ccType = AI_BER_OCTET_STR;
                _StringCopy(apoTrapPdu->coVariables[i].coVal.csString,apcValue);    
                apoTrapPdu->coVariables[i].ciValLen=strlen(apoTrapPdu->coVariables[i].coVal.csString);
                break;

            case 'o':
                apoTrapPdu->coVariables[i].ccType = AI_BER_OBJECT_ID;
                _StringCopy(apoTrapPdu->coVariables[i].coVal.csObjid,apcValue);
                apoTrapPdu->coVariables[i].ciValLen=strlen(apoTrapPdu->coVariables[i].coVal.csObjid);    
                break;

            case 't':
                apoTrapPdu->coVariables[i].ccType = AI_BER_TIMETICKS;
                apoTrapPdu->coVariables[i].coVal.ciInteger=strtoul(apcValue,NULL,10);
                apoTrapPdu->coVariables[i].ciValLen=sizeof(apoTrapPdu->coVariables[i].coVal.ciInteger);
                break;

            case 'n':
                apoTrapPdu->coVariables[i].ccType = AI_BER_NULL;
                _StringCopy(apoTrapPdu->coVariables[i].coVal.csString,"NULL");
                apoTrapPdu->coVariables[i].ciValLen=1;
                break;        

            case 'a':
                apoTrapPdu->coVariables[i].ccType = AI_BER_IPADDRESS;
                _StringCopy(apoTrapPdu->coVariables[i].coVal.csString,apcValue);
                apoTrapPdu->coVariables[i].ciValLen=strlen(apoTrapPdu->coVariables[i].coVal.csString);    
                break;
            }
            return 0; 
        }
    }
    return AI_SNMP_ERRADDVAR;
}

int SnmpAddVarInt(stSnmpTrap *apoTrapPdu, char *apcOidName, char *apcValue)
{
    for(int i=0; i<AI_SNMP_MAX_VARLIST_NUM;i++)
    {
        if(apoTrapPdu->coVariables[i].ciValLen == 0)
        {
            _StringCopy(apoTrapPdu->coVariables[i].csName,apcOidName);
            apoTrapPdu->coVariables[i].ccType = AI_BER_INTEGER;
            apoTrapPdu->coVariables[i].coVal.ciInteger=strtol(apcValue,NULL,10);
            apoTrapPdu->coVariables[i].ciValLen=sizeof(apoTrapPdu->coVariables[i].coVal.ciInteger);
            return 0;
        }
    }
    return AI_SNMP_ERRADDVAR;
}

int SnmpAddVarStr(stSnmpTrap *apoTrapPdu, char *apcOidName, char *apcValue)
{
    for(int i=0; i<AI_SNMP_MAX_VARLIST_NUM;i++)
    {
        if(apoTrapPdu->coVariables[i].ciValLen == 0)
        {
            _StringCopy(apoTrapPdu->coVariables[i].csName,apcOidName);
            apoTrapPdu->coVariables[i].ccType = AI_BER_OCTET_STR;
            _StringCopy(apoTrapPdu->coVariables[i].coVal.csString,apcValue);    
            apoTrapPdu->coVariables[i].ciValLen=strlen(apoTrapPdu->coVariables[i].coVal.csString);
            return 0;
        }
    }
    return AI_SNMP_ERRADDVAR;
}


int SnmpAddVarOid(stSnmpTrap *apoTrapPdu, char *apcOidName, char *apcValue)
{
    for(int i=0; i<AI_SNMP_MAX_VARLIST_NUM;i++)
    {
        if(apoTrapPdu->coVariables[i].ciValLen == 0)
        {
            _StringCopy(apoTrapPdu->coVariables[i].csName,apcOidName);
            apoTrapPdu->coVariables[i].ccType = AI_BER_OBJECT_ID;
            _StringCopy(apoTrapPdu->coVariables[i].coVal.csObjid,apcValue);
            apoTrapPdu->coVariables[i].ciValLen=strlen(apoTrapPdu->coVariables[i].coVal.csObjid);
            return 0;
        }
    }
    return AI_SNMP_ERRADDVAR;
}

int SnmpAddVarTimeStamp(stSnmpTrap *apoTrapPdu, char *apcOidName, char *apcValue)
{
    for(int i=0; i<AI_SNMP_MAX_VARLIST_NUM;i++)
    {
        if(apoTrapPdu->coVariables[i].ciValLen == 0)
        {
            _StringCopy(apoTrapPdu->coVariables[i].csName,apcOidName);
            apoTrapPdu->coVariables[i].ccType = AI_BER_TIMETICKS;
            apoTrapPdu->coVariables[i].coVal.ciInteger=strtoul(apcValue,NULL,10);
            apoTrapPdu->coVariables[i].ciValLen=sizeof(apoTrapPdu->coVariables[i].coVal.ciInteger);
            return 0;
        }
    }
    return AI_SNMP_ERRADDVAR;
}

int SnmpAddVarNULL(stSnmpTrap *apoTrapPdu, char *apcOidName, char *apcValue)
{
    for(int i=0; i<AI_SNMP_MAX_VARLIST_NUM;i++)
    {
        if(apoTrapPdu->coVariables[i].ciValLen == 0)
        {
            _StringCopy(apoTrapPdu->coVariables[i].csName,apcOidName);
            apoTrapPdu->coVariables[i].ccType = AI_BER_NULL;
            _StringCopy(apoTrapPdu->coVariables[i].coVal.csString,"NULL");
            apoTrapPdu->coVariables[i].ciValLen=1;
            return 0;
        }
    }
    return AI_SNMP_ERRADDVAR;
}

int SnmpAddVarIPAddress(stSnmpTrap *apoTrapPdu, char *apcOidName, char *apcValue)
{
    for(int i=0; i<AI_SNMP_MAX_VARLIST_NUM;i++)
    {
        if(apoTrapPdu->coVariables[i].ciValLen == 0)
        {
            _StringCopy(apoTrapPdu->coVariables[i].csName,apcOidName);
            apoTrapPdu->coVariables[i].ccType = AI_BER_IPADDRESS;
            _StringCopy(apoTrapPdu->coVariables[i].coVal.csString,apcValue);
            apoTrapPdu->coVariables[i].ciValLen=strlen(apoTrapPdu->coVariables[i].coVal.csString);
            return 0;
        }
    }
    return AI_SNMP_ERRADDVAR;
}    

///end namespace
AIBC_NAMESPACE_END
