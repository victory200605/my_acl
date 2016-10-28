#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <time.h>

#include "AILib.h"
#include "AICMPPStru.h"

///start namespace
AIBC_NAMESPACE_START

/** 
    \brief  construct a CMPP MsgID into apcMsgID
    \param aiMsgID:seq no,4 digit
    \param aiTime:time
    \param apcISMGNo:ISMGNo or SMCNo
    \param apcMsgID:buffer to save CMPP MsgID
    \return 
*/
void EncodeCMPPMsgID(char *apcMsgID, int aiMsgID, time_t aiTime, char const*apcISMGNo)
{
    char    liChar = 0;
    int     liValue = 0;
    char    *lpcValue = (char *)&liValue;

    struct tm   tmNow;
    struct tm   *lptmNow = NULL;
    lptmNow = localtime_r( &aiTime, &tmNow);

    liChar = (lptmNow->tm_mon+1) & 0x0F;
    apcMsgID[0] = (liChar << 4);

    liChar = lptmNow->tm_mday & 0x1F;
    apcMsgID[0] = apcMsgID[0] | ((liChar >> 1) & 0x0F);
    apcMsgID[1] = ((liChar & 0x01) << 7);

    liChar = lptmNow->tm_hour & 0x1F;
    apcMsgID[1] = apcMsgID[1] | ((liChar << 2) & 0x7C);

    liChar = lptmNow->tm_min & 0x3F;
    apcMsgID[1] = apcMsgID[1] | ((liChar >> 4) & 0x03);
    apcMsgID[2] = ((liChar & 0x0F) << 4);

    liChar = lptmNow->tm_sec & 0x3F;
    apcMsgID[2] = apcMsgID[2] | ((liChar >> 2) & 0x0F);
    apcMsgID[3] = ((liChar & 0x03) << 6);
    
    liValue = htonl(strtoul( apcISMGNo, (char**)NULL, 10 ) & 0x003FFFFF);
    apcMsgID[3] = apcMsgID[3] | (lpcValue[1] & 0x3F);
    memcpy( apcMsgID+4, lpcValue+2, 2 );

    liValue = htonl(aiMsgID & 0x0000FFFF);
    memcpy( apcMsgID+6, lpcValue+2, 2 );
}

/** 
    \brief  construct a CMPP MsgID into apcMsgID
    \param aiMsgID:seq no,4 digit
    \param apcTime:time, 10bytes
    \param apcISMGNo:ISMGNo or SMCNo
    \param apcMsgID:buffer to save CMPP MsgID
    \return 
*/
void EncodeCMPPMsgID(char* apcMsgID, int aiMsgID, char const* apcTime, char const* apcISMGNo)
{
    char    liChar = 0;
    int     liValue = 0;
    char    *lpcValue = (char *)&liValue;
    unsigned long liTime = strtoul(apcTime, NULL, 10);

    liChar = (liTime/100000000%100) & 0x0F;
    apcMsgID[0] = (liChar << 4);

    liChar = ((liTime/1000000)%100) & 0x1F;
    apcMsgID[0] = apcMsgID[0] | ((liChar >> 1) & 0x0F);
    apcMsgID[1] = ((liChar & 0x01) << 7);

    liChar = ((liTime/10000)%100) & 0x1F;
    apcMsgID[1] = apcMsgID[1] | ((liChar << 2) & 0x7C);

    liChar = ((liTime/100)%100) & 0x3F;
    apcMsgID[1] = apcMsgID[1] | ((liChar >> 4) & 0x03);
    apcMsgID[2] = ((liChar & 0x0F) << 4);

    liChar = (liTime%100) & 0x3F;
    apcMsgID[2] = apcMsgID[2] | ((liChar >> 2) & 0x0F);
    apcMsgID[3] = ((liChar & 0x03) << 6);
    
    liValue = htonl(strtoul( apcISMGNo, (char**)NULL, 10 ) & 0x003FFFFF);
    apcMsgID[3] = apcMsgID[3] | (lpcValue[1] & 0x3F);
    memcpy( apcMsgID+4, lpcValue+2, 2 );

    liValue = htonl(aiMsgID & 0x0000FFFF);
    memcpy( apcMsgID+6, lpcValue+2, 2 );
}

/** 
    \brief  convert SGIP to CMPP MsgID
    \param apcSGIPMsgID:SGIP
    \param apcMsgID:buffer to save CMPP MsgID
    \return 
*/
void SGIPToCMPPMsgID(char *apcMsgID, const char *apcSGIPMsgID)
{
    char    liChar = 0;
    int     liValue = 0;
    int     liMsgID = 0;
    char    lsTime[10+1] = {0};
    char    lsISMGNo[10+1] = {0};
    char    *lpcValue = (char *)&liValue;

    memcpy(  &liValue, apcSGIPMsgID+0, 4 );
    sprintf( lsISMGNo, "%010u", (unsigned int)ntohl(liValue) );

    memcpy(  &liValue, apcSGIPMsgID+4, 4 );
    sprintf( lsTime, "%010u", (unsigned int)ntohl(liValue) );

    memcpy(  &liValue, apcSGIPMsgID+8, 4 );
    liMsgID = ntohl(liValue) & 0xFFFF;

    liChar = ((lsTime[0]-'0')*10 + lsTime[1]-'0') & 0x0F;
    apcMsgID[0] = (liChar << 4);

    liChar = ((lsTime[2]-'0')*10 + lsTime[3]-'0') & 0x1F;
    apcMsgID[0] = apcMsgID[0] | ((liChar >> 1) & 0x0F);
    apcMsgID[1] = ((liChar & 0x01) << 7);

    liChar = ((lsTime[4]-'0')*10 + lsTime[5]-'0') & 0x1F;
    apcMsgID[1] = apcMsgID[1] | ((liChar << 2) & 0x7C);

    liChar = ((lsTime[6]-'0')*10 + lsTime[7]-'0') & 0x3F;
    apcMsgID[1] = apcMsgID[1] | ((liChar >> 4) & 0x03);
    apcMsgID[2] = ((liChar & 0x0F) << 4);

    liChar = ((lsTime[8]-'0')*10 + lsTime[9]-'0') & 0x3F;
    apcMsgID[2] = apcMsgID[2] | ((liChar >> 2) & 0x0F);
    apcMsgID[3] = ((liChar & 0x03) << 6);
    
    liValue = htonl(strtoul( lsISMGNo+5, (char**)NULL, 10 ) & 0x003FFFFF);
    apcMsgID[3] = apcMsgID[3] | (lpcValue[1] & 0x3F);
    memcpy( apcMsgID+4, lpcValue+2, 2 );

    liValue = htonl(liMsgID & 0x0000FFFF);
    memcpy( apcMsgID+6, lpcValue+2, 2 );
}

/** 
    \brief  decode a CMPP MsgID
    \param apcMsgID:CMPP MsgID
    \param aiMsgID:seq no,4 digit
    \param aiTime:time
    \param apcISMGNo:ISMGNo or SMCNo
    \return 
*/
void DecodeMsgID(int *apiMsgID, char *apcISMGNo, char *apcTime, char const*apcMsgID)
{
    char    liChar = 0;
    int     liValue = 0;
    char    *lpcValue = (char *)&liValue;

    liChar = (apcMsgID[0] & 0xF0) >> 4;
    sprintf( apcTime, "%02d", liChar );

    liChar = ((apcMsgID[0] & 0x0F) << 1) | ((apcMsgID[1] & 0x80) >> 7);
    sprintf( apcTime+2, "%02d", liChar );

    liChar = (apcMsgID[1] >> 2) & 0x1F;
    sprintf( apcTime+4, "%02d", liChar );

    liChar = ((apcMsgID[1] & 0x03) << 4) | ((apcMsgID[2] & 0xF0) >> 4);
    sprintf( apcTime+6, "%02d", liChar );

    liChar = ((apcMsgID[2] & 0x0F) <<2 ) | ((apcMsgID[3] & 0xC0) >> 6);
    sprintf( apcTime+8, "%02d", liChar );

    liValue = 0;
    lpcValue[1] = apcMsgID[3] & 0x3F;
    memcpy( lpcValue+2, apcMsgID+4, 2 );
    sprintf( apcISMGNo, "%06u", ntohl( liValue ) );

    liValue = 0;
    memcpy( lpcValue+2, apcMsgID+6, 2 );
    *apiMsgID = ntohl( liValue );
}


/** 
    \brief  convert CMPP MsgID to RateMsgID
    \param apcSGIPMsgID:buffer to save RateMsgID
    \param apcMsgID:CMPP MsgID
    \return 
*/
void GetRateMsgID(char *apcRateMsgID, const char *apcMsgID)
{
    int     liMsgID = 0;
    char    lsTime[10+1] = {0};
    char    lsISMGNo[6+1] = {0};

    DecodeMsgID(&liMsgID, lsISMGNo, lsTime, apcMsgID);
    sprintf( apcRateMsgID, "%10s%5s%05d", lsTime, lsISMGNo+1, liMsgID );
}


static inline ssize_t _CheckIsDigit(char const* apcPhoneNo)
{
    size_t liCount = 0;
    while(*apcPhoneNo)
    {
        AI_RETURN_IF(-1, !isdigit(*apcPhoneNo++));
        ++liCount;
    }

    return liCount;
}


int Remove86(char *apcPhoneNo)
{
    static char const*  lppcStrs[]  = {"86", "+86", "0086", "+"};
    ssize_t liLen = strlen(apcPhoneNo);
    ssize_t liRetSize;
        
    for (size_t i = 0; i < AI_ARRAY_SIZE(lppcStrs); ++i)
    {
        ssize_t liPreLen = strlen(lppcStrs[i]);

        if (strncmp(apcPhoneNo, lppcStrs[i], liPreLen) == 0)
        {
            liRetSize = _CheckIsDigit(apcPhoneNo + liPreLen);
            if (liRetSize > 0 && liRetSize <= 20)
            {
                memmove(apcPhoneNo, apcPhoneNo + liPreLen, liLen + 1 - liPreLen);
                return 0;
            }
        }
    }

    liRetSize = _CheckIsDigit(apcPhoneNo);
    if (liRetSize > 0 && liRetSize <= 20)
    {
        return 0;
    }

    return -1;
}

///end namespace
AIBC_NAMESPACE_END
