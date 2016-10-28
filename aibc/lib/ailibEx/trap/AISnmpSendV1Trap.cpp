#include "AISnmpSendV1Trap.h"

//------------------BEGIN AILIBEX---------------------
void StringCopy(char* apcDestStr, char const* apcSrcStr, size_t aiSize)
{
    const char  *lpcPtr1 = apcSrcStr;
    char        *lpcPtr2 = apcDestStr;

    *lpcPtr2=0;

    while ((*lpcPtr1) && (lpcPtr2 - apcDestStr) < (ptrdiff_t)(aiSize-1) )
    {
        *lpcPtr2++ = *lpcPtr1++;
        *lpcPtr2 = 0;
    }
}

#define _StringCopy(d, s)  StringCopy(d, s, sizeof(d))

int ai_set_sockaddr_in(struct sockaddr_in* apoSA, char const* apcIPAddr, int aiPort) 
{
    memset(apoSA, 0, sizeof(*apoSA));

    apoSA->sin_family = AF_INET;

    if (NULL == apcIPAddr || 0 == apcIPAddr[0])
    {
        apoSA->sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
#if defined(AF_INET6)
        if(inet_pton(AF_INET, apcIPAddr, &apoSA->sin_addr) < 0)
        {
            return AI_SOCK_ERROR_INVAL;
        }
#else
        apoSA->sin_addr.s_addr = inet_addr(apcIPAddr);
#endif
    }

    if(aiPort > (int)UINT16_MAX || aiPort < 0)
    {
        return AI_SOCK_ERROR_INVAL;
    }
    
    apoSA->sin_port = htons(aiPort);

    return 0;
}

ssize_t ai_socket_sendto(int aiSocket, char const* apcToAddr, int aiToPort, char const* apcData, size_t aiSize)
{
    ssize_t liRetSize;
    struct sockaddr_in  loInetAddr;
    
    if(ai_set_sockaddr_in(&loInetAddr, apcToAddr, aiToPort) < 0)
    {
        return AI_SOCK_ERROR_INVAL;
    }
  
    liRetSize = sendto(aiSocket, apcData, aiSize, 0, (struct sockaddr*)&loInetAddr, sizeof(loInetAddr));  

    if(liRetSize < 0)
    {
        return AI_SOCK_ERROR_SENDTO;
    } 

    return liRetSize;
}

//-------------------END AILIBEX----------------------

//------------------BEGIN BER---------------------

unsigned char *BerBuildLength(unsigned char * apcBuffer, size_t * apiMaxSize,size_t aiValueLen)
{
    unsigned char * lpcBuffer = apcBuffer;
    if (aiValueLen< 0x80)
    {
        if (*apiMaxSize< 1)
    	{
            return (NULL);
    	}
        *apcBuffer ++ = (unsigned char) aiValueLen;
    } 

    else if (aiValueLen<= 0xFF) 
    {
        if (*apiMaxSize < 2) 
        {
            return (NULL);
    	}
        *apcBuffer ++ = (unsigned char) (0x01 | 0x80);
        *apcBuffer ++ = (unsigned char) aiValueLen ;	
    } 
	
    else if (aiValueLen<= 0xFFFF)
    {                   
        if (*apiMaxSize < 3) 
        {
           return (NULL);
    	}
        *apcBuffer ++ = (unsigned char) (0x02 | 0x80);
        *apcBuffer ++ = (unsigned char) ((aiValueLen >> 8) & 0xFF);
        *apcBuffer ++ = (unsigned char) (aiValueLen & 0xFF);
    }
    else if (aiValueLen<= 0xFFFFFF)
    {					
        if (*apiMaxSize < 4) 
        {
            return (NULL);
        }
        *apcBuffer ++ = (unsigned char) (0x03 | 0x80);
        *apcBuffer ++ = (unsigned char) ((aiValueLen >> 16) & 0xFF);
        *apcBuffer ++ = (unsigned char) ((aiValueLen >> 8) & 0xFF);
        *apcBuffer ++ = (unsigned char) (aiValueLen & 0xFF);
    }
    else
    {
        if(*apiMaxSize<5)
        {
            return (NULL);
        }
        *apcBuffer++ = (unsigned char)(0x04 | 0x80);
        *apcBuffer++ = (unsigned char)((aiValueLen >> 24) & 0xFF);
        *apcBuffer++ = (unsigned char)((aiValueLen >> 16) & 0xFF);
        *apcBuffer++ = (unsigned char)((aiValueLen >> 8) & 0xFF);
        *apcBuffer++ = (unsigned char)(aiValueLen & 0xFF);
    }

    *apiMaxSize -= (apcBuffer - lpcBuffer); 
    return (apcBuffer);
}

unsigned char * BerBuildSequence(unsigned char * apcBuffer,size_t * apiMaxSize, unsigned char acType, size_t aiValueLen)
{
    if (*apiMaxSize < 4)
    {
    	return (NULL);
    }

    *apiMaxSize -= 1;
    *apcBuffer++ = acType;

    return BerBuildLength(apcBuffer,apiMaxSize,aiValueLen);
}

unsigned char *BerBuildHeader(unsigned char * apcBuffer,size_t * apiMaxSize, unsigned char acType, size_t aiValueLen)
{
    if (*apiMaxSize< 1) 
    {
    	return (NULL);
    }

    *apcBuffer ++ = acType; 
    (*apiMaxSize)--;
	
    return BerBuildLength(apcBuffer, apiMaxSize, aiValueLen);
}

unsigned char *BerBuildInt(unsigned char  * apcBuffer, size_t * apiMaxSize, const unsigned char acType , const long *apiValue, size_t aiValueLen)
{
    long              liVal;
    unsigned long     liMask;

    if (aiValueLen!= sizeof(long)) 
    {
	return (NULL);
    }
    liVal= *apiValue;

    liMask = 0x1FFul << ((8 * (sizeof(long) - 1)) - 1);
 
    while ((((liVal & liMask) == 0) || ((liVal & liMask) == liMask))&& aiValueLen > 1) 
    {
        aiValueLen--;
        liVal<<= 8;
    }
	
    apcBuffer= BerBuildHeader(apcBuffer, apiMaxSize, acType, aiValueLen);
    if(apcBuffer==NULL)
    {
    	return (NULL);
    }

    *apiMaxSize -= aiValueLen;
    liMask=  0xFFul << (8 * (sizeof(long) - 1));
    while (aiValueLen--) 
    {
        *apcBuffer++ = (char ) ((liVal & liMask) >> (8 * (sizeof(long) - 1)));
        liVal<<= 8;
    }
    return (apcBuffer);
}

unsigned char * BerBuildString(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char acType, const char * apcValue, size_t aiValueLen)
{
    apcBuffer = BerBuildHeader( apcBuffer, apiMaxSize, acType, aiValueLen );

    if( apcBuffer == NULL )
    {
        return  (NULL);
    }
    if( *apiMaxSize < aiValueLen )
    {
        return  (NULL);
    } 
    memcpy( apcBuffer, apcValue, aiValueLen );
    *apiMaxSize -= aiValueLen;
    return  (apcBuffer + aiValueLen);
}

unsigned  char *BerBuildObjid(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char acType, unsigned long *apiValue, size_t aiValueLen) 
{
    size_t	         liAsnLength;
    unsigned long        *lpiOp ;
    unsigned char        lsObjidSize[128];
    unsigned long        liObjidVal;
    unsigned long        liFirstObjidVal;
    int                  i;

    lpiOp= apiValue;

    if (aiValueLen == 0) 
    {
        liObjidVal = 0;
        aiValueLen = 2;
    } 
    else if (apiValue[0] > 2) 
    {
        printf("build objid: bad first subidentifier");
        return (NULL);
    } 
    else if (aiValueLen == 1) 
    {
        liObjidVal = (lpiOp[0] * 40);
        aiValueLen = 2;
        lpiOp++;
    } 
    else 
    {
    	if ((lpiOp[1] > 40) && (lpiOp[0] < 2))
	{
            printf("build objid: bad second subidentifier");
            return (NULL);
        }
        liObjidVal = (lpiOp[0] * 40) + lpiOp[1]; 
        lpiOp += 2;
    }   
    liFirstObjidVal = liObjidVal;

    if (aiValueLen > 128)
    {
    	return (NULL);
    }
    for (i = 1, liAsnLength = 0;;) 
    {
        if (liObjidVal < (unsigned) 0x80) 
        {
            lsObjidSize[i] = 1;
            liAsnLength += 1;
        } 
	else if (liObjidVal < (unsigned) 0x4000) 
	{
            lsObjidSize[i] = 2;
            liAsnLength += 2;
        } 
	else if (liObjidVal < (unsigned) 0x200000) 
	{
            lsObjidSize[i] = 3;
            liAsnLength += 3;
        } 
	else if (liObjidVal < (unsigned) 0x10000000) 
	{
            lsObjidSize[i] = 4;
            liAsnLength += 4;
        } 
	else 
	{
            lsObjidSize[i] = 5;
            liAsnLength += 5;
        }
        i++;
		
        if (i >= (int) aiValueLen)
        {
        	break;
    	}
        liObjidVal = *lpiOp++;	
    }

    apcBuffer = BerBuildHeader(apcBuffer , apiMaxSize , acType, liAsnLength);
    if( apcBuffer == NULL )
    {
	return  (NULL);
    }

    for (i = 1, liObjidVal = liFirstObjidVal, lpiOp = apiValue+ 2;i < (int) aiValueLen; i++) 
    {
        if (i != 1) 
	{
            liObjidVal = *lpiOp++;
        }

	switch (lsObjidSize[i]) 
	{
        case 1:
            *apcBuffer++ = (unsigned char) liObjidVal;
            break;

        case 2:
            *apcBuffer++ = (unsigned char) ((liObjidVal >> 7) | 0x80);
            *apcBuffer++ = (unsigned char) (liObjidVal & 0x07f);
            break;

        case 3:
            *apcBuffer++ = (unsigned char) ((liObjidVal >> 14) | 0x80);
            *apcBuffer++ = (unsigned char) ((liObjidVal >> 7 & 0x7f) | 0x80);
            *apcBuffer++ = (unsigned char) (liObjidVal & 0x07f);
            break;

        case 4:
            *apcBuffer++ = (unsigned char) ((liObjidVal >> 21) | 0x80);
            *apcBuffer++ = (unsigned char) ((liObjidVal >> 14 & 0x7f) | 0x80);
            *apcBuffer++ = (unsigned char) ((liObjidVal >> 7 & 0x7f) | 0x80);
            *apcBuffer++ = (unsigned char) (liObjidVal & 0x07f);
            break;

        case 5:
            *apcBuffer++ = (unsigned char) ((liObjidVal >> 28) | 0x80);
            *apcBuffer++ = (unsigned char) ((liObjidVal >> 21 & 0x7f) | 0x80);
            *apcBuffer++ = (unsigned char) ((liObjidVal >> 14 & 0x7f) | 0x80);
            *apcBuffer++ = (unsigned char) ((liObjidVal >> 7 & 0x7f) | 0x80);
            *apcBuffer++ = (unsigned char) (liObjidVal & 0x07f);
            break;
	}
    }

    *apiMaxSize -= liAsnLength;
    return apcBuffer;
}

unsigned char *BerBuildUnsignedInt(unsigned char * apcBuffer , size_t * apiMaxSize,unsigned char acType, const unsigned long  * apiValue , int aiValueLen)
{
    unsigned long       liVal;
    unsigned long       liMask;

    int                 liAddNullByte = 0;

    if (aiValueLen!= sizeof(long))
    {
       	return (NULL);
    }
    liVal = *apiValue;

    liMask= 0xFFul << (8 * (sizeof(long) - 1));
    if ((unsigned char ) ((liVal & liMask) >> (8 * (sizeof(long) - 1))) & 0x80) 
    {
	liAddNullByte = 1;
        aiValueLen++;
    } 
    else 
    {
        liMask = ((unsigned long ) 0x1FF) << ((8 * (sizeof(long) - 1)) - 1);
        while ((((liVal & liMask) == 0) || ((liVal & liMask) == liMask))&& aiValueLen> 1) 
	{
            aiValueLen--;
            liVal<<= 8;
       	}
    }
    apcBuffer= BerBuildHeader(apcBuffer, apiMaxSize, acType, aiValueLen);
    if (apcBuffer==NULL)
    {
       	return (NULL);
    }
    *apiMaxSize -= aiValueLen;
	
    if (liAddNullByte == 1) 
    {
        *apcBuffer++ = '\0';
        aiValueLen--;
    }
    liMask= ((unsigned long) 0xFF) << (8 * (sizeof(long) - 1));
    while (aiValueLen--) 
    {
        *apcBuffer++ = (unsigned char ) ((liVal & liMask) >> (8 * (sizeof(long) - 1)));
        liVal<<= 8;
    }
    return (apcBuffer);
}

unsigned char *BerBuildNull(unsigned char * apcBuffer ,size_t * apiMaxSize, unsigned char acType)
{
    apcBuffer= BerBuildHeader(apcBuffer, apiMaxSize, acType, 0);
    return (apcBuffer);
}

//-------------------END BER----------------------

//------------------BEGIN Build PDU---------------------
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
        printf("Can't build OID for variable");
        return NULL;
    }

    switch (acVarValType) 
    {
    case AI_BER_TIMETICKS:
        apcBuffer = BerBuildUnsignedInt(apcBuffer, apiMaxSize, acVarValType,(unsigned long *) apcVal, aiValLen);
        break;

    case AI_BER_OCTET_STR:
        apcBuffer = BerBuildString(apcBuffer, apiMaxSize, acVarValType,apcVal, aiValLen);
        break;
        
    case AI_BER_IPADDRESS:
        char liAddress[4];
        memset(&liAddress,0,sizeof(liAddress));
        int liAddLen = CutAddr(apcVal,liAddress,4);
        apcBuffer= BerBuildString(apcBuffer, apiMaxSize,acVarValType,(char *) liAddress, liAddLen);
        break;

    case AI_BER_OBJECT_ID:
        unsigned long liObjidVar[AI_SNMP_OID_NUM];
        aiValLen=ArrToOid(apcVal,liObjidVar,AI_SNMP_OID_NUM); 
        apcBuffer = BerBuildObjid(apcBuffer, apiMaxSize, acVarValType,(unsigned long *)liObjidVar,aiValLen );
        break;

    case AI_BER_INTEGER:
        apcBuffer = BerBuildInt(apcBuffer, apiMaxSize, acVarValType,(long *) apcVal, aiValLen);
        break;

    case AI_BER_NULL:
        apcBuffer = BerBuildNull(apcBuffer, apiMaxSize, acVarValType);
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
    } 
    
    else if ((apcBuffer - lpcH0e) <= 0xFF) 
    {
        memmove(lpcH0e + 1 , lpcH0e , apcBuffer - lpcH0e );
        liLength1 -= 1;
        BerBuildSequence(lpcH0, &liLength1, (unsigned char) (AI_SNMP_BER_TRAP), apcBuffer - lpcH0e);
        apcBuffer++;
    } 
    
    else if ((apcBuffer - lpcH0e) <= 0xFFFF) 
    {
        memmove(lpcH0e + 2 , lpcH0e , apcBuffer - lpcH0e );
        liLength1 -= 2;
        BerBuildSequence(lpcH0, &liLength1, (unsigned char) (AI_SNMP_BER_TRAP), apcBuffer - lpcH0e);
        apcBuffer+=2;
    } 
    else if ((apcBuffer - lpcH0e) <= 0xFFFFFF) 
    {
        memmove(lpcH0e + 3 , lpcH0e , apcBuffer - lpcH0e );
        liLength1 -= 3;
        BerBuildSequence(lpcH0, &liLength1, (unsigned char) (AI_SNMP_BER_TRAP), apcBuffer - lpcH0e);
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

//-------------------END Build PDU----------------------

//------------------BEGIN Send V1 Trap---------------------

int SnmpSendV1Trap(stSnmpTrap * apoPdu, char const* apcRemoteIp, int aiRemotePort)
{
    int                liSendBytes;
    
    unsigned char      lsBuffer[AI_SNMP_MAXLINE];
    memset(lsBuffer,0,sizeof(lsBuffer));
    int liSockfd;

    if((liSockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0 )
    {
        if(liSockfd > 0)
        {
            close(liSockfd);
        }

        liSockfd = -1;
        return AI_SNMP_ESOCKET;
    }

    int liLen;
    if((liLen=SnmpTrapBuild(apoPdu, lsBuffer, sizeof(lsBuffer)))<0)
    {
        if(liSockfd > 0)
        {
            close(liSockfd);
        }

        liSockfd = -1;
        return AI_SNMP_ESOCKET;

        return AI_SNMP_EPBUILD;
    }

    if((liSendBytes = ai_socket_sendto(liSockfd, apcRemoteIp, aiRemotePort, (char *)lsBuffer, liLen)) < 0 )
    {
        if(liSockfd > 0)
        {
            close(liSockfd);
        }

        liSockfd = -1;
        return AI_SNMP_ESENDTO;
    }

    if(liSockfd > 0)
    {
        close(liSockfd);
    }

    return 0;
}

//-------------------END Send V1 Trap----------------------

//------------------BEGIN Test----------------------------

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

    struct timeval ltStartTime,ltEndTime;
    memset(&ltStartTime,0,sizeof(ltStartTime));
    memset(&ltEndTime,0,sizeof(ltEndTime)); 
    gettimeofday(&ltStartTime,NULL);

    for(int i=0;i<100000;i++)
    {
        int ret = SnmpSendV1Trap(&loTrapPdu,lpcNMSIP,lpcNMSPort);
//        printf("[%d]-ret=%d\n",i,ret);
    }

    gettimeofday(&ltEndTime,NULL);
    long time_ms = (ltEndTime.tv_sec - ltStartTime.tv_sec)*1000 + (ltEndTime.tv_usec - ltStartTime.tv_usec)/1000;
    printf("\n\n\trun time = %ld ms\n",time_ms);

    return 0;
}

//------------------END Test----------------------------


