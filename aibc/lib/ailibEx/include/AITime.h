/** 
 * @file AITime.h
 */

#ifndef __AILIBEX__AITIME_H__
#define __AILIBEX__AITIME_H__

//#include <stdint.h>
#include <sys/times.h>

#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

/////////////////////////////////////////////////////////////
//
/**
 * @brief Get month time
 *
 * @return Return the month time
 */
extern  int GetMonthTime(void);

/**
 * @brief Get the short current time. eg:20090324094313
 * 
 * @param apcOutTime    OUT - store the output time
 */
extern  void GetCurShortTime(char *apcOutTime);

/**
 * @brief Get the long current time. eg:20090324094313.249978
 * 
 * @param apcOutTime    OUT - store the output time
 */
extern  void GetCurLongTime(char *apcOutTime);

/**
 * @brief Get the difference between two time,the unit is ms.
 *
 * @param apcTime1      IN - one time,such as 20090324094313.249978
 * @param apcTime2      IN - the other time,such as 20090324094314.349978
 *
 * @return the difference,in the case you'll get 1100
 */ 
extern  int DiffLongTime(char *apcTime1, char *apcTime2);

/**
 * @brief Get the difference between the begin time and current time
 *
 * @param apcBeginTime   IN - the begin time
 *
 * @return the difference.
 */
extern  int DiffLongTime(char *apcBeginTime);

/**
 * @brief Get needed time string.
 *
 * @param apcOutData    OUT - the struct to store the specified time string
 * @param aiVal         IN - the specified time value
 * @param aiMode        IN - the style of time string
 *
 * @return Return the output time string.
 */
extern  char *GetTimeString(char *apcOutData, int aiVal, int aiMode = 0);

/////////////////////////////////////////////////////////////
//
typedef int64_t AITime_t;

#define AI_TIME_NSEC ((AIBC::AITime_t)1)
#define AI_TIME_USEC (AI_TIME_NSEC*1000)
#define AI_TIME_MSEC (AI_TIME_USEC*1000)
#define AI_TIME_SEC  (AI_TIME_MSEC*1000)
#define AI_TIME_MIN  (AI_TIME_SEC*60)
#define AI_TIME_HOUR (AI_TIME_MIN*60)
#define AI_TIME_DAY  (AI_TIME_HOUR*60)

#define AI_TIME_FOREVER ((AIBC::AITime_t)-1)

#define AI_TIME_TO_TT(tt, t)    {tt = (t)/AI_TIME_SEC;}
#define AI_TIME_FROM_TT(t, tt)  {t = (tt)*AI_TIME_SEC;}

#define AI_TIME_TO_TV(tv, t) \
{ \
    tv.tv_sec = (t)/AI_TIME_SEC; \
    tv.tv_usec = ((t)%AI_TIME_SEC)/AI_TIME_USEC; \
}

#define AI_TIME_FROM_TV(t, tv) \
{ \
    t = tv.tv_sec*AI_TIME_SEC + tv.tv_usec*AI_TIME_USEC; \
}

#define AI_TIME_TO_TS(ts, t) \
{ \
    ts.tv_sec = (t)/AI_TIME_SEC; \
    ts.tv_nsec = ((t)%AI_TIME_SEC)/AI_TIME_NSEC; \
}

#define AI_TIME_FROM_TS(t, ts) \
{ \
    t = ts.tv_sec*AI_TIME_SEC + ts.tv_nsec*AI_TIME_NSEC; \
}

#define AI_TIME_TO_D(d, t) \
{ \
    d = ((double)t)/AI_TIME_SEC; \
}

#define AI_D_TO_TIME(t, d) \
{ \
    t = (AITime_t)((d)*AI_TIME_SEC); \
}
/**
 * @brief Get current time.
 *
 * @param apTZ     IN - time zone
 */
extern AITime_t AICurTime(void* apTZ=NULL);

/**
 * @brief Remain time to sleep.
 *
 * @param atRelTime    IN - specify sleep time
 *
 * @return 0:success -1:error
 */
extern int AISleepFor(AITime_t atRelTime);

/**
 * @brief Sleep until the specified time.
 *
 * @param atAbsTime    IN - the specified absolute time
 *
 * @return 0:success -1:error
 */
extern int AISleepUntil(AITime_t atAbsTime);

/**
 * @brief Get time zone.
 *
 * @return time zone
 */
extern AITime_t AIGetTimeZone();

/**
 * @brief Format local time.
 *
 * @param apcBuf       OUT - the output buffer
 * @param aiLen        IN - the output buffer size
 * @param apcFmt       IN - the output buffer format
 * @param aiTime       IN - the need to format time
 *
 * @return Upon successfully complete,the number of bytes placed into the array pointed to by apcBuf.Otherwise,0 shall be return.

 */
extern ssize_t AIFormatLocalTime(void* apcBuf, size_t aiLen, char const* apcFmt, AITime_t aiTime);

/**
 * @brief Format GM time.
 *
 * @param apcBuf       OUT - the output buffer
 * @param aiLen        IN - the output buffer size
 * @param apcFmt       IN - the output buffer format
 * @param aiTime       IN - the need to format time
 *
 * @return Upon successfully complete,the number of bytes placed into the array pointed to by apcBuf.Otherwise,0 shall be return.

 */
extern ssize_t AIFormatGMTime(void* apcBuf, size_t aiLen, char const* apcFmt, AITime_t aiTime);

////////////////////////////////////////////////////////////////////////////

class AITimeMeter
{
public:
    AITimeMeter();

    void Reset();
    void Snapshot();

    double GetTime() const;

private:
    AITime_t    ciBegin;
    AITime_t    ciEnd;
};

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AITIME_H__

