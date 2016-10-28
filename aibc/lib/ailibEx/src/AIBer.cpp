#include "AIBer.h"

///start namespace
AIBC_NAMESPACE_START

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
//        printf("build objid: bad first subidentifier");
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
//            printf("build objid: bad second subidentifier");
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

unsigned char *BerBuildBitstring(unsigned char *apcBuffer,size_t * apiMaxSize,unsigned char acType, const char * apcValue, size_t aiValueLen)
{
    apcBuffer = BerBuildHeader(apcBuffer , apiMaxSize, acType, aiValueLen);
    if (apcBuffer==NULL)
    {
    	return (NULL);
    }

    if (aiValueLen > 0 && apcValue)
    {
    	memmove(apcBuffer, apcValue, aiValueLen);
    }
    else if (aiValueLen > 0 && !apcValue) 
    {
//        printf("no string passed into Ber_build_bitstring\n");
        return (NULL);
    }

    *apcBuffer-= aiValueLen;
    return (apcBuffer + aiValueLen);
}

unsigned char *BerParseLength(unsigned char * apcBuffer, unsigned long * apiValueLen)
{
    unsigned char lcLengthByte;

    if (!apcBuffer|| !apiValueLen) 
    {
//        printf("parse length: NULL pointer");
        return (NULL);
    }
    lcLengthByte = *apcBuffer;

    if (lcLengthByte & AI_BER_LONG_LEN) 
    {
        lcLengthByte &= ~AI_BER_LONG_LEN;    
        if (lcLengthByte == 0) 
	{
            return (NULL);
    	}
        if ((int)lcLengthByte > sizeof(long)) 
	{
	    return (NULL);
    	}
        apcBuffer++;			
        *apiValueLen= 0;          
        while (lcLengthByte--) 
	{
            *apiValueLen<<= 8;
            *apiValueLen|= *apcBuffer++;
        }
        return apcBuffer;
    } 
    else 
    { 
        *apiValueLen= (int) lcLengthByte;
        return apcBuffer + 1;
    }
}

unsigned char *BerParseHeader(unsigned char * apcBuffer, size_t * apiMaxSize, unsigned char * apcType)
{
    unsigned char       *lpcBuf;
    unsigned long       liAsnLen;

    if (!apcBuffer|| !apiMaxSize|| !apcType) 
    {
//        printf("parse header: NULL pointer");
        return (NULL);
    }
    lpcBuf = apcBuffer; 
    if (AI_IS_EXTENSION_ID(*lpcBuf)) 
    {
//        printf("can't process ID >= 30");
        return (NULL);
    }
    *apcType= *lpcBuf++; 
	
    lpcBuf = BerParseLength(lpcBuf, &liAsnLen); 

    *apiMaxSize= (size_t) liAsnLen;

    return lpcBuf;
}

unsigned char *BerParseSequence(unsigned char * apcBuffer, size_t * apiMaxSize, unsigned char * apcType, unsigned char acExpectedType)
{                                                       
    apcBuffer= BerParseHeader(apcBuffer, apiMaxSize, apcType);
    if (apcBuffer && (*apcType!= acExpectedType)) 
    {
	return (NULL);
    }
    return apcBuffer;
}

unsigned char *BerParseObjid(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char * apcType, unsigned long * apiObjid, int  *apiObjidLen)
{
    unsigned char       *lpcBuf = apcBuffer;
    unsigned long       *lpiOid = apiObjid + 1;
    unsigned long       liSubIdentifier;
    long                liLength;
    unsigned long       liAsnLen;

    *apcType = *lpcBuf++;
    lpcBuf = BerParseLength(lpcBuf, &liAsnLen);

    *apiMaxSize-= (int) liAsnLen + (lpcBuf - apcBuffer);

    if (liAsnLen == 0)
    {
    	apiObjid[0] = apiObjid[1] = 0;
    }
    liLength = liAsnLen;
    (*apiObjidLen)--;          

    while (liLength > 0 && (*apiObjidLen)-- > 0)
    {
        liSubIdentifier = 0;
        do 
	{                   
            liSubIdentifier =(liSubIdentifier << 7) + (*(unsigned char *) lpcBuf & ~(0x80));
            liLength--;
        } 
	while (*(unsigned char *) lpcBuf++ & (0x80));        

        *lpiOid++ = (unsigned long) liSubIdentifier;
    }

    if (0 != liLength) 
    {
//        printf("OID length exceeds buffer size");
        return NULL;
    }

    liSubIdentifier = (unsigned long) apiObjid[1];
    if (liSubIdentifier == 0x2B) 
    {
        apiObjid[0] = 1;
        apiObjid[1] = 3;
    } 
    else 
    {
        if (liSubIdentifier < 40) 
	{
            apiObjid[0] = 0;
            apiObjid[1] = liSubIdentifier;
        } 
	else if (liSubIdentifier < 80) 
	{
            apiObjid[0] = 1;
            apiObjid[1] = liSubIdentifier - 40;
        } 
	else 
        {
            apiObjid[0] = 2;
            apiObjid[1] = liSubIdentifier - 80;
        }
    }

    *apiObjidLen = (int) (lpiOid - apiObjid);

    return lpcBuf;
}

unsigned char *BerParseInt(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char * apcType, long *apiValue, int aiValueLen)
{
    unsigned char          *lpcBuf = apcBuffer;
    unsigned long          liAsnLen;
    long                   liValue = 0;

    if (aiValueLen!= sizeof(long)) 
    {
	return (NULL);
    }
    *apcType = *lpcBuf++;
	
    lpcBuf = BerParseLength(lpcBuf, &liAsnLen);

    if ((int) liAsnLen > aiValueLen)
    {
	return (NULL);
    }

    *apiMaxSize -= (int) liAsnLen + (lpcBuf - apcBuffer);
	
    if (*lpcBuf & 0x80)		
    {
        liValue = -1;  
    }    
	
    while (liAsnLen--)
    {
       	liValue = (liValue << 8) | *lpcBuf++;
    }
    *apiValue= liValue;
    return lpcBuf;
}

unsigned char *BerParseString (unsigned char *apcBuffer, size_t *apiMaxSize, unsigned char *apcType, char *apcValue, int aiValueLen)
{
    unsigned char       *lpcBuf = apcBuffer;
    unsigned long       liAsnLen;

    *apcType= *lpcBuf++;
    lpcBuf = BerParseLength(lpcBuf, &liAsnLen);
	
    if ((int) liAsnLen > aiValueLen) 
    {
	return (NULL);
    }

    memmove(apcValue, lpcBuf, liAsnLen);
	
    if ((int) liAsnLen > aiValueLen)
    {
        apcValue[liAsnLen] = 0;   
    }    
	
    aiValueLen= (int) liAsnLen;
    *apiMaxSize -= (size_t) liAsnLen + (lpcBuf - apcBuffer);

    return (lpcBuf + liAsnLen);
}

unsigned char *BerParseAgentAddr (unsigned char *apcBuffer, size_t *apiMaxSize, unsigned char *apcType, char *apcValue, int aiValueLen)
{
    unsigned char       *lpcBuf = apcBuffer;
    unsigned long       liAsnLen;

    *apcType= *lpcBuf++;
    lpcBuf = BerParseLength(lpcBuf, &liAsnLen);
	
    if ((int) liAsnLen > aiValueLen) 
    {
	return (NULL);
    }

    int count = 0;

    count = sprintf(apcValue,"%d.",lpcBuf[0]);
    count += sprintf(apcValue+count,"%d.",lpcBuf[1]);
    count += sprintf(apcValue+count,"%d.",lpcBuf[2]);
    count += sprintf(apcValue+count,"%d",lpcBuf[3]);

    if ((int) liAsnLen > aiValueLen)
    {
        apcValue[liAsnLen] = 0;   
    }    
	
    aiValueLen= (int) liAsnLen;
    *apiMaxSize -= (size_t) liAsnLen + (lpcBuf - apcBuffer);

    return (lpcBuf + liAsnLen);
}

unsigned char *BerParseNull(unsigned char * apcBuffer, size_t * apiMaxSize, unsigned char * apcType)
{
    unsigned char        *lpcBuf = apcBuffer;
    unsigned long        liAsnLen;

    *apcType = *lpcBuf++;
    lpcBuf = BerParseLength(lpcBuf, &liAsnLen);
    if (lpcBuf == NULL)
    {
//        printf("parse null: bad length");
        return (NULL);
    }
    if (liAsnLen != 0) 
    {
//        printf("parse null: malformed BER.1 null");
        return (NULL);
    }

    *apiMaxSize-= (lpcBuf - apcBuffer);

    return lpcBuf + liAsnLen;
}

unsigned char *BerParseBitstring(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char * apcType, char * apcValue, int * apiValueLen)
{
    unsigned char          *lpcBuf = apcBuffer;
    unsigned long          liAsnLen;

    *apcType = *lpcBuf++;
    lpcBuf = BerParseLength(lpcBuf, &liAsnLen);

    if ((int) liAsnLen > *apiValueLen) 
    {
	return (NULL);
    }
    if (liAsnLen<1)
    {
    	return (NULL);
    }
    memmove(apcValue, lpcBuf, liAsnLen);
    *apiValueLen= (int) liAsnLen;
    *apiMaxSize-= (size_t) liAsnLen + (lpcBuf - apcBuffer);
    return lpcBuf + liAsnLen;
}

unsigned char *BerParseUnsignedInt(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char * apcType, unsigned long * apiValue, int aiValueLen)
{
    unsigned char          *lpcBuf = apcBuffer;
    unsigned long          liAsnLen;
    unsigned long          liValue = 0;

    if (aiValueLen!= sizeof(int)) 
    {
	return (NULL);
    }
	
    *apcType= *lpcBuf++;
	
    lpcBuf = BerParseLength(lpcBuf, &liAsnLen);
    if (((int) liAsnLen > (aiValueLen+ 1)) ||(((int) liAsnLen == aiValueLen+ 1) && *lpcBuf != 0x00)) 
    {
        return NULL;
    }
	
    *apiMaxSize-= (size_t) liAsnLen + (lpcBuf - apcBuffer);
    if (*lpcBuf & 0x80)
    {
        liValue = ~liValue;        
    }

    while (liAsnLen--)
    {
        liValue = (liValue << 8) | *lpcBuf++;
    }    

    *apiValue= liValue;
    return lpcBuf;
}

///end namespace
AIBC_NAMESPACE_END
