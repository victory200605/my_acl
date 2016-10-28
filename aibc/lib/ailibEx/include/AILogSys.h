#ifndef __AILIBEX__AILOGSYS_H__
#define __AILIBEX__AILOGSYS_H__

#include "AILib.h"
#include "AIString.h"
#include "AIConfig.h"
#include "AIDualLink.h"
#include "AISynch.h"
#include "AIModule.h"

///start namespace
AIBC_NAMESPACE_START

char const* const   AILIB_LOGFILE   = "ailib.log";

////////////////////////////////////////////////////////////////////////////////////
#define     AILOG_LEVEL_SYSTEM          0
#define     AILOG_LEVEL_ERROR           1
#define     AILOG_LEVEL_WARN            2
#define     AILOG_LEVEL_INFO            5
#define     AILOG_LEVEL_DEBUG           10
#define     AILOG_LEVEL_TRACE           11
#define     AILOG_LEVEL_ANY             12
 
////////////////////////////////////////////////////////////////////////////////////
#define     AILOG_LEVEL_SYSTEM_STR      "SYS: "
#define     AILOG_LEVEL_ERROR_STR       "ERR: "
#define     AILOG_LEVEL_WARN_STR        "WARN: "
#define     AILOG_LEVEL_INFO_STR        "INFO: "
#define     AILOG_LEVEL_DEBUG_STR       "DEBUG: "
#define     AILOG_LEVEL_TRACE_STR       "TRACE: "
#define     AILOG_LEVEL_ANY_STR         "ANY: "

////////////////////////////////////////////////////////////////////////////////////
#define     LOG_SYS_REC_TYPE_NOR        (0)
#define     LOG_SYS_REC_TYPE_CDR        (1)
#define     LOG_BUF_MAX_SIZE            (1024*1024) /* 1M Bytes */

////////////////////////////////////////////////////////////////////////////////////
//////
class AILogSys
{
private:
    int     ciType;
    int     ciFileID;
    char*   cpcInnBuf;
    short   ciLogLevel;
    int     ciLastBakTime;
    int     ciBakInterval;
    int     ciBakFileSize;
    char    csBackupDIR[128];
    char    csFullPathName[128];

private:
    LISTHEAD    coLogSysChild;
    AIMutexLock     coMutexLock;

public:
    AILogSys(char const* apcName, int aiType, char const* apcBakDir=NULL, int aiFileSize=5120, int aiTimeVal=900);
    ~AILogSys(void);

private:    
    void    Lock(void);
    void    Unlock(void);
    void    Close(void);
    void    CheckBackup(void);
    int     GetStrLevel(char const* apcFmtStr);
    void    MultiLevelMkDir(char const* apcFullDir);
    void    GetFileTime(char *apcTimeString, int aiSize);
    void    InnerLOG(int aiLevel, char const* apcFmtStr, va_list ap);

public:
    int     GetLevel(void);
    void    ExecBackup(void);
    void    SetLevel(short aiLevel);
    void    WriteLOG(char const* apcFmtStr, ...);
    void    WriteCDR(char const* apcFmtStr, ...);
    void    WriteCDREx(char const* apcFmtStr, va_list ap);
    void    WriteLOG(int aiLevel, char const* apcFmtStr, ...);
    void    WriteLOGEx(int aiLevel, char const* apcFmtStr, va_list ap);
    void    WriteHexLOG(char const* apcTitle, char const* apcInBuf, int aiBufSize);

public:
    friend  void AICloseLOGHandler(void);
    friend  AILogSys* AISearchLOGHandler(char const* apcLogName);
    friend  void AIDynChangeLOGLevel(char const*apcFileName);
    friend  void AIChangeLOGLevel(short aiLevel);
    friend  void AIWriteCDR(char const* apcLogName, char const* apcFmtStr, ...);
    friend  void AIWriteLOG(char const* apcLogName, int aiLogLevel, char const* apcFmtStr, ...);
    friend  void AIWriteHexLOG(char const* apcLogName, char const* apcTitle, char const* apcInBuf, int aiBufSize);
};

///////////////////////////////////////////////////////////////////////////////
/////
extern  void AIInitLOGHandler(void);
extern  void AICloseLOGHandler(void);
extern  void AIDynChangeLOGLevel(char const* apcFileName = NULL);
extern  void AIChangeLOGLevel(short aiLevel);
extern  void AIWriteCDR(char const* apcLogName, char const* apcFmtStr, ...);
extern  void AIWriteLOG(char const* apcLogName, int aiLogLevel, char const* apcFmtStr, ...);
extern  void AIWriteHexLOG(char const* apcLogName, char const* apcTitle, char const* apcInBuf, int aiBufSize);

///////////////////////////////////////////////////////////////////////////////
/////
extern char gsConfigDirectory[128];

///end namespace
AIBC_NAMESPACE_END

#endif // __AILOGSYS_H__

