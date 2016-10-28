#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "AITime.h"

///start namespace
AIBC_NAMESPACE_START

int GetMonthTime(void)
{
    int     liRetCode = 0;
    char    lsTimeStr[10] = {0};
    struct  tm   loTmpBuf;
    struct  tm   *lpoTm = NULL;
    struct  timeval     tvNow;

    gettimeofday(&tvNow, NULL);
    lpoTm = (struct tm *)localtime_r(&(tvNow.tv_sec), &loTmpBuf);
    sprintf(lsTimeStr, "%04d%02d", lpoTm->tm_year+1900, lpoTm->tm_mon+1);

    liRetCode = strtoul(lsTimeStr, (char**)NULL, 10);
    return  (liRetCode);
}

void GetCurShortTime(char *apcOutTime)
{
    struct  tm   loTmpBuf;
    struct  tm   *lpoTm = NULL;
    struct  timeval     loTvNow;

    gettimeofday(&loTvNow, NULL);
    lpoTm = (struct tm *)localtime_r(&(loTvNow.tv_sec), &loTmpBuf);
    sprintf(apcOutTime, "%04d%02d%02d%02d%02d%02d", lpoTm->tm_year+1900, lpoTm->tm_mon+1, lpoTm->tm_mday,
        lpoTm->tm_hour, lpoTm->tm_min, lpoTm->tm_sec);
}

void GetCurLongTime(char *apcOutTime)
{
    struct  tm   loTmpBuf;
    struct  tm   *lpoTm = NULL;
    struct  timeval     loTvNow;

    gettimeofday(&loTvNow, NULL);
    lpoTm = (struct tm *)localtime_r(&(loTvNow.tv_sec), &loTmpBuf);
    sprintf(apcOutTime, "%04d%02d%02d%02d%02d%02d.%06d", lpoTm->tm_year+1900, lpoTm->tm_mon+1, lpoTm->tm_mday,
        lpoTm->tm_hour, lpoTm->tm_min, lpoTm->tm_sec, static_cast<int>(loTvNow.tv_usec));
}

int DiffLongTime(char *apcTime1, char *apcTime2)
{
#define GET_VAL(b,o,l,a,z) \
    { \
        memcpy(lsValue, b+o,l);\
        lsValue[l]=0; a=atoi(lsValue)-z;\
    }

    struct  tm  loTm;
    int     liRetCode1 = 0;
    int     liRetCode2 = 0;
    time_t  liTimeDiff = 0;
    char    lsValue[10] = {0};

    liTimeDiff = time(NULL);
    localtime_r(&liTimeDiff, &loTm);

    GET_VAL(apcTime2,  0,4,loTm.tm_year,1900);
    GET_VAL(apcTime2,  4,2,loTm.tm_mon,1);
    GET_VAL(apcTime2,  6,2,loTm.tm_mday,0);
    GET_VAL(apcTime2,  8,2,loTm.tm_hour,0);
    GET_VAL(apcTime2, 10,2,loTm.tm_min,0);
    GET_VAL(apcTime2, 12,2,loTm.tm_sec,0);
    GET_VAL(apcTime2, 15,6,liRetCode1,0);
    liRetCode1 = liRetCode1/1000;
    liTimeDiff = mktime(&loTm);

    GET_VAL(apcTime1,  0,4,loTm.tm_year,1900);
    GET_VAL(apcTime1,  4,2,loTm.tm_mon,1);
    GET_VAL(apcTime1,  6,2,loTm.tm_mday,0);
    GET_VAL(apcTime1,  8,2,loTm.tm_hour,0);
    GET_VAL(apcTime1, 10,2,loTm.tm_min,0);
    GET_VAL(apcTime1, 12,2,loTm.tm_sec,0);
    GET_VAL(apcTime1,15,6,liRetCode2,0);
    liRetCode2 = liRetCode2/1000;
    liTimeDiff -= mktime(&loTm);

    if(liTimeDiff >= 0)   //bug? liTimeDiff > 0 
    {
        if(liRetCode1 >= liRetCode2)
        {
            liRetCode1 -= liRetCode2;
        }
        else
        {
            liTimeDiff -= 1;
            liRetCode1 += (1000-liRetCode2);
        }

        return  (liTimeDiff*1000)+liRetCode1;
    }

    if(liRetCode2 >= liRetCode1)
    {
        liRetCode2 -= liRetCode1;
    }
    else
    {
        liTimeDiff += 1;
        liRetCode2 += (1000-liRetCode1);
    }

    /* return millisecond */
    return  (liTimeDiff*1000)-liRetCode2;
}

int DiffLongTime(char *apcBeginTime)
{
    char    lsEndTime[32] = {0};

    GetCurLongTime(lsEndTime);
    return  DiffLongTime(apcBeginTime, lsEndTime);
}


char *GetTimeString(char *apcOutData, int aiVal, int aiMode)
{
    time_t  liCurTime = time(NULL);
    struct  tm   loTmpBuf;
    struct  tm  *lpoTm = NULL;


    switch(aiMode)
    {
    case  0:
        lpoTm = (struct tm *)localtime_r(&liCurTime, &loTmpBuf);
        sprintf(apcOutData, "%02d:%02d:%02d", lpoTm->tm_hour, lpoTm->tm_min, lpoTm->tm_sec);
        break;

    case  1:
        lpoTm = (struct tm *)localtime_r(&liCurTime, &loTmpBuf);
        sprintf(apcOutData, "%04d/%02d/%02d %02d:%02d:%02d", lpoTm->tm_year+1900, lpoTm->tm_mon+1, lpoTm->tm_mday,
            lpoTm->tm_hour, lpoTm->tm_min, lpoTm->tm_sec);
        break;

    case  2:
        lpoTm = (struct tm *)localtime_r(&liCurTime, &loTmpBuf);
        sprintf(apcOutData, "%04d%02d%02d", lpoTm->tm_year+1900, lpoTm->tm_mon+1, lpoTm->tm_mday);
        break;

    case  3:
        liCurTime += aiVal;
        lpoTm = (struct tm *)localtime_r(&liCurTime, &loTmpBuf);
        sprintf(apcOutData, "%02d%02d%02d%02d%02d%02d032+", lpoTm->tm_year-100, lpoTm->tm_mon+1,
            lpoTm->tm_mday, lpoTm->tm_hour, lpoTm->tm_min, lpoTm->tm_sec);
        break;

    case  4:
        lpoTm = (struct tm *)localtime_r(&liCurTime, &loTmpBuf);
        sprintf(apcOutData, "%02d%02d%02d%02d%02d", lpoTm->tm_mon+1, lpoTm->tm_mday, 
            lpoTm->tm_hour, lpoTm->tm_min, lpoTm->tm_sec);
        break;

    case  5:
        lpoTm = (struct tm *)localtime_r(&liCurTime, &loTmpBuf);
        sprintf(apcOutData, "%04d%02d%02d%02d%02d%02d", lpoTm->tm_year-100, lpoTm->tm_mon+1,
            lpoTm->tm_mday, lpoTm->tm_hour, lpoTm->tm_min, lpoTm->tm_sec);
        break;

    case  6:
        lpoTm = (struct tm *)localtime_r(&liCurTime, &loTmpBuf);
        sprintf(apcOutData, "%04d%02d%02d%02d%02d%02d", lpoTm->tm_year+1900, lpoTm->tm_mon+1,
            lpoTm->tm_mday, lpoTm->tm_hour, lpoTm->tm_min, lpoTm->tm_sec);
        break;
    }

    return  (apcOutData);
}

////////////////////////////////////////////////////////////////////////////
AITime_t AICurTime(void* apTZ)
{
    struct timeval ltTV;
    AITime_t liTime;

    gettimeofday(&ltTV, (struct timezone*)apTZ);
    AI_TIME_FROM_TV(liTime, ltTV);

    return liTime;
}

////////////////////////////////////////////////////////////////////////////
int AISleepFor(AITime_t aiRelTime)
{
    return AISleepUntil(AICurTime() + aiRelTime);
}

////////////////////////////////////////////////////////////////////////////
int AISleepUntil(AITime_t aiAbsTime)
{
    AITime_t liInterval = aiAbsTime - AICurTime();
    while (liInterval > 0)
    {
        struct timeval ltTimeout;
        AI_TIME_TO_TV(ltTimeout, liInterval);

        if (select(0, NULL, NULL, NULL, &ltTimeout) < 0)
        {
            if (EINTR != errno)
            {
                return -1;
            }
        }

        liInterval = aiAbsTime - AICurTime();
    };

    return 0;
}

////////////////////////////////////////////////////////////////////////////
AITime_t AIGetTimeZone()
{
    struct tm loLocal;
    struct tm loUtc;
    time_t liNow = time(NULL);

    localtime_r(&liNow, &loLocal);
    gmtime_r(&liNow, &loUtc);

    return (mktime(&loLocal) - mktime(&loUtc))*AI_TIME_SEC;
}

////////////////////////////////////////////////////////////////////////////
ssize_t AIFormatLocalTime(void* apcBuf, size_t aiLen, char const* apcFmt, AITime_t aiTime)
{
    time_t      liTime;
    struct tm   loTm;
    struct tm*  lpoTm;
    
    AI_TIME_TO_TT(liTime, aiTime);

    lpoTm = localtime_r(&liTime, &loTm); 

    AI_RETURN_IF(-1, (NULL == lpoTm));

    return strftime((char*)apcBuf, aiLen, apcFmt, lpoTm);
}

////////////////////////////////////////////////////////////////////////////
ssize_t AIFormatGMTime(void* apcBuf, size_t aiLen, char const* apcFmt, AITime_t aiTime)
{
    time_t      liTime;
    struct tm   loTm;
    struct tm*  lpoTm;
    
    AI_TIME_TO_TT(liTime, aiTime);

    lpoTm = gmtime_r(&liTime, &loTm); 

    AI_RETURN_IF(-1, (NULL == lpoTm));

    return strftime((char*)apcBuf, aiLen, apcFmt, lpoTm);
}

////////////////////////////////////////////////////////////////////////////
AITimeMeter::AITimeMeter()
{
    Reset();
}
          
void AITimeMeter::Reset()
{
    ciBegin = AICurTime();
}

void AITimeMeter::Snapshot()
{
    ciEnd = AICurTime();
}
                      
double AITimeMeter::GetTime() const
{
    return (double(ciEnd - ciBegin))/AI_TIME_SEC;
}

///end namespace
AIBC_NAMESPACE_END
