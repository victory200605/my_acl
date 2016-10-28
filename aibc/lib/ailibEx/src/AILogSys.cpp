#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <ctype.h>
#include "AILogSys.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////
///
char gsConfigDirectory[128] = "../config";

////////////////////////////////////////////////////////////////////////////////////
/// NOTE: Unix Compile: gcc -g -o main -lpthread -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT -DTEST 

/** 
    \brief make multi level dir
    \param sFullDir: full path of dir
    \return
*/
void AILogSys::MultiLevelMkDir(char const* apcFullDir)
{
    char    lsMkDir[ 128 ] = {0};
    char    *lpcPtr = (char* )apcFullDir;

    while((lpcPtr = strchr(lpcPtr, '/')) != NULL)
    {
        if((lpcPtr - apcFullDir) > 0)
        {
            memcpy(lsMkDir, apcFullDir, lpcPtr-apcFullDir);
            lsMkDir[lpcPtr-apcFullDir] = '\0';
            mkdir(lsMkDir, 0700);
        }

        lpcPtr += 1;
    }
}

/** 
    \brief AILodSys class construct function
    \param apcName:
    \param aiType: full path of dir
    \param apcBakDir: path of back dir
    \param aiFileSize: size of log file
    \param aiTimeVal: back log file interval
    \return
*/
AILogSys::AILogSys(char const* apcName, int aiType, char const* apcBakDir, int aiFileSize, int aiTimeVal)
{
    ciFileID = -1;
    ciLogLevel = 5;
    ciType = aiType;
    ciBakInterval = (-1);
    ciBakFileSize = (-1);
    ciLastBakTime = time(NULL);
    strcpy(csBackupDIR,  "");
    strcpy(csFullPathName, apcName);
    memset(&coLogSysChild, 0, sizeof(coLogSysChild));
    AI_NEW_N_ASSERT(cpcInnBuf, char, LOG_BUF_MAX_SIZE);

    ciBakFileSize = (aiFileSize>0)?aiFileSize:5120; /* KBytes */
    ciBakInterval = (aiTimeVal>=10)?aiTimeVal:900; /* Seconds */

    strcpy(csBackupDIR, "./LogBak/");

    if(apcBakDir)
    {
        if(strlen(apcBakDir) > 0)
        {
            strcpy(csBackupDIR, apcBakDir);
            if(csBackupDIR[strlen(csBackupDIR)-1] != '/')
            {
                strcat(csBackupDIR, "/");
            }
        }
    }

    AILogSys::MultiLevelMkDir(csFullPathName);
    AILogSys::MultiLevelMkDir(csBackupDIR);
    AILogSys::CheckBackup();
}

AILogSys::~AILogSys(void)
{
    if(cpcInnBuf) 
    {
        AI_DELETE_N(cpcInnBuf);
    }

    AILogSys::Close();
}

void AILogSys::Close(void)
{
    if(ciFileID >= 0)
    {
          close(ciFileID); /* Maybe BUG */
    }

    ciFileID = (-1);
}

int AILogSys::GetLevel(void)
{
    return  (ciLogLevel);
}

void AILogSys::SetLevel(short aiLevel)
{
    if(aiLevel > 0 && aiLevel <= 12)
    {
        if(aiLevel != AILogSys::GetLevel())
        {
            AILogSys::WriteLOG("<0>Change Level From %d To %d\n", ciLogLevel, aiLevel);
            ciLogLevel = aiLevel;
        }
    }
}

void AILogSys::Lock(void)
{
    coMutexLock.Lock();
}

void AILogSys::Unlock(void)
{
    coMutexLock.Unlock();
}

/** 
    \brief backup log file
    \return
*/
void AILogSys::CheckBackup(void)
{
    if(ciFileID >= 0)
    {
        int     liCurFileOffset = (int)lseek(ciFileID, 0, SEEK_CUR);
        int     liCurTime = time(NULL);
        
        if((liCurTime - ciLastBakTime) >= ciBakInterval || 
            ( liCurFileOffset >= (ciBakFileSize*1024) && liCurTime - ciLastBakTime >= 1 /*must more than 1 sec*/ ) )
        {
            char    lsTimeString[32] = {0};
            char    lsBakTmpName[152] = {0};
            char    *lpcPtr = csFullPathName;

            AILogSys::GetFileTime(lsTimeString, sizeof(lsTimeString));
            AILogSys::Close();

            if((lpcPtr = strrchr(csFullPathName, '/')))
            {
                lpcPtr += 1;
            }
            snprintf(lsBakTmpName, sizeof(lsBakTmpName), "%s%s[%s]", csBackupDIR, lpcPtr?lpcPtr:csFullPathName, lsTimeString);

            AILogSys::MultiLevelMkDir(csBackupDIR);
            rename(csFullPathName, lsBakTmpName);
            ciLastBakTime = time(NULL);
        }
    } /* if(ciFileID > 0) */

    if(ciFileID < 0)
    {
        ciLastBakTime = time(NULL);
        AILogSys::MultiLevelMkDir(csFullPathName);

        if((ciFileID = open(csFullPathName, O_CREAT|O_WRONLY, 0600)) >= 0)
        {
            lseek(ciFileID, 0, SEEK_END);
        }
        else
        {
            /* open fail, do nothing */
            fprintf(stderr, "FATAL:  Create File %s ... Fail, %s\n", csFullPathName, strerror(errno));
        }
    } /* if(ciFileID <= 0) */
}

void AILogSys::ExecBackup(void)
{
    AILogSys::Lock();
    AILogSys::CheckBackup();
    AILogSys::Unlock();
}

int AILogSys::GetStrLevel(char const* apcFmtStr)
{
    int     liLevel = (-1);
    char    *lpcPtr = (char* )apcFmtStr+1;

    if(apcFmtStr[0] == '<')
    {
        for(; *lpcPtr && isdigit(*lpcPtr); lpcPtr++);
        if(*lpcPtr == '>' && (lpcPtr-apcFmtStr) >= 2)
        {
            liLevel = atoi(apcFmtStr+1);
            return  (liLevel);
        }
    }

    return  (liLevel);
}

void AILogSys::GetFileTime(char* apcTimeString, int aiSize)
{
    time_t  liCurTime;
    struct  tm   loTmpBuf;
    struct  tm  *loTm = NULL;
    char    lsCurTimeStr[16] = {0};

    liCurTime = time(NULL);
    loTm = (struct tm *)localtime_r(&liCurTime, &loTmpBuf);
    sprintf(lsCurTimeStr, "%04d%02d%02d%02d%02d%02d", loTm->tm_year+1900, loTm->tm_mon+1, loTm->tm_mday,
        loTm->tm_hour, loTm->tm_min, loTm->tm_sec);

    memset(apcTimeString, 0, aiSize);
    strncpy(apcTimeString, lsCurTimeStr, aiSize-1);
}

/** 
    \brief Inner write log with string "SYS:" or "ERR:" or ...
    \param aiLevel:Log level
    \param apcFmtStr,ap: the string want to wrte
    \return
*/
void AILogSys::InnerLOG(int aiLevel, char const* apcFmtStr, va_list ap)
{
    int     liSize = 0;
    int     liRetSize = 0;
    char    *lpcPtr = cpcInnBuf;
    char    lsCurTimeStr[24] = {0};

    if(ciFileID < 0 || aiLevel > ciLogLevel)
    {
        return;
    }

    if(aiLevel >= 0)
    {
        time_t  liCurTime;
        struct  tm   loTmpBuf;
        struct  tm  *loTm = NULL;

        liCurTime = time(NULL);
        loTm = (struct tm *)localtime_r(&liCurTime, &loTmpBuf);
        sprintf(lsCurTimeStr, "%04d/%02d/%02d.%02d:%02d:%02d", loTm->tm_year+1900, loTm->tm_mon+1, loTm->tm_mday,
            loTm->tm_hour, loTm->tm_min, loTm->tm_sec);

        liSize = snprintf(lpcPtr, LOG_BUF_MAX_SIZE-1, "%s#%05u ", lsCurTimeStr, (unsigned int)pthread_self());
        lpcPtr += liSize;

        if(aiLevel <= AILOG_LEVEL_SYSTEM)
        {
            strcat(lpcPtr, AILOG_LEVEL_SYSTEM_STR);
            lpcPtr += strlen(AILOG_LEVEL_SYSTEM_STR); 
            liSize += strlen(AILOG_LEVEL_SYSTEM_STR);
        }
        else if(aiLevel <= AILOG_LEVEL_ERROR)
        {
            strcat(lpcPtr, AILOG_LEVEL_ERROR_STR);
            lpcPtr += strlen(AILOG_LEVEL_ERROR_STR); 
            liSize += strlen(AILOG_LEVEL_ERROR_STR);
        }
        else if(aiLevel <= AILOG_LEVEL_WARN)
        {
            strcat(lpcPtr, AILOG_LEVEL_WARN_STR);
            lpcPtr += strlen(AILOG_LEVEL_WARN_STR); 
            liSize += strlen(AILOG_LEVEL_WARN_STR);
        }
        else if(aiLevel <= AILOG_LEVEL_INFO)
        {
            strcat(lpcPtr, AILOG_LEVEL_INFO_STR);
            lpcPtr += strlen(AILOG_LEVEL_INFO_STR); 
            liSize += strlen(AILOG_LEVEL_INFO_STR);
        }
        else if(aiLevel <= AILOG_LEVEL_DEBUG)
        {
            strcat(lpcPtr, AILOG_LEVEL_DEBUG_STR);
            lpcPtr += strlen(AILOG_LEVEL_DEBUG_STR); 
            liSize += strlen(AILOG_LEVEL_DEBUG_STR);
        }
        else if(aiLevel <= AILOG_LEVEL_TRACE)
        {
            strcat(lpcPtr, AILOG_LEVEL_TRACE_STR);
            lpcPtr += strlen(AILOG_LEVEL_TRACE_STR); 
            liSize += strlen(AILOG_LEVEL_TRACE_STR);
        }
        else
        {
            strcat(lpcPtr, AILOG_LEVEL_ANY_STR);
            lpcPtr += strlen(AILOG_LEVEL_ANY_STR); 
            liSize += strlen(AILOG_LEVEL_ANY_STR);
        }
    }
    
    //HP-UX will return -1 when buffer is no enough
    liRetSize = vsnprintf(lpcPtr, LOG_BUF_MAX_SIZE-liSize-1, apcFmtStr, ap);
    if ( liRetSize < 0 //HP-UNIX
        || liRetSize > LOG_BUF_MAX_SIZE-liSize-1 )
    {
        lpcPtr += strlen(lpcPtr);
    }
    else
    {
        lpcPtr += liRetSize;
    }
    
    if((liSize = lpcPtr-cpcInnBuf) > 0 && *(lpcPtr-1) != '\n')
    {
        *lpcPtr++ = '\n'; liSize += 1;
    }

    write(ciFileID, cpcInnBuf, liSize);
}

/** 
    \brief write log
    \param apcFmtStr,...: the string want to wrte
    \return
*/
void AILogSys::WriteLOG(char const* apcFmtStr, ...)
{
    va_list     ap;
    int         liLevel = (-1);
    char const* lpcPtr = apcFmtStr;

    assert(apcFmtStr);
    if(strlen(apcFmtStr) <= 0)    return;

    if(liLevel = AILogSys::GetStrLevel(apcFmtStr), liLevel >= 0)
    {
        lpcPtr = strchr(apcFmtStr, '>') + 1;
    }

    apcFmtStr = lpcPtr;
    va_start(ap, apcFmtStr);
    // va_start(ap, lpcPtr); //

    AILogSys::Lock();
    AILogSys::InnerLOG(liLevel, lpcPtr, ap);
    
    if(liLevel <= ciLogLevel)
    {
        AILogSys::CheckBackup();
    }
    AILogSys::Unlock();

    va_end(ap);
}

void AILogSys::WriteLOG(int aiLevel, char const* apcFmtStr, ...)
{
    va_list     ap;

    assert(apcFmtStr);
    if(strlen(apcFmtStr) <= 0)
    {
        return;
    }

    va_start(ap, apcFmtStr);

    AILogSys::Lock();
    AILogSys::InnerLOG(aiLevel, apcFmtStr, ap);

    if(aiLevel <= ciLogLevel)
    {
        AILogSys::CheckBackup();
    }
    AILogSys::Unlock();

    va_end(ap);
}

void AILogSys::WriteLOGEx(int aiLevel, char const* apcFmtStr, va_list ap)
{
    assert(apcFmtStr);
    if (strlen(apcFmtStr) <= 0)
    {
        return;
    }

    AILogSys::Lock();
    AILogSys::InnerLOG(aiLevel, apcFmtStr, ap);

    if(aiLevel <= ciLogLevel)
    {
        AILogSys::CheckBackup();
    }
    AILogSys::Unlock();
}

/** 
    \brief write hex log
    \param apcTitle:title
    \param apcInBuf:
    \param aiBufSize:
    \return
*/
void AILogSys::WriteHexLOG(char const* apcTitle, char const* apcInBuf, int aiBufSize)
{
    int     liRet;
    size_t  liHexSize = 0;
    char    *lpcPtr = NULL;
    char    *lpcBuffer = NULL;
    #define INN_HEX_BUF_MAX_SIZE    (9216)
    #define HEXLOG_BYTES_PER_LINE   (16)
    char const* const CHARSET = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    
    liHexSize = (aiBufSize >= 1840)?1840:aiBufSize;
    if(aiBufSize < 0 || ciLogLevel < 8)
        return;

    AI_NEW_N_ASSERT(lpcBuffer, char, INN_HEX_BUF_MAX_SIZE);
    lpcPtr = lpcBuffer;

    liRet = sprintf(lpcBuffer, "<8>%s: (nSize=%d/0x%X)\n", (apcTitle != NULL ? apcTitle : ""), aiBufSize, aiBufSize);
    assert(liRet > 0);
    lpcPtr += liRet;

    for(size_t liSection = 0; liSection < liHexSize; liSection += HEXLOG_BYTES_PER_LINE)
    {
        liRet = sprintf(lpcPtr, "%08lX: ", liSection);
        assert(liRet > 0);
        lpcPtr += liRet;

        for(size_t liOffset = liSection; liOffset < (liSection + HEXLOG_BYTES_PER_LINE); ++liOffset)
        {
            if (liOffset < liHexSize)
            {
                liRet = sprintf(lpcPtr, "%02X%c", (unsigned char)apcInBuf[liOffset], 
                    (liOffset % HEXLOG_BYTES_PER_LINE) == (HEXLOG_BYTES_PER_LINE / 2 - 1) ? '-' : ' ');
            }
            else
            {
                liRet = sprintf(lpcPtr, "%s", "   ");
            }

            assert(liRet > 0);
            lpcPtr += liRet;
        }

        for(size_t liOffset = liSection; liOffset < liSection + HEXLOG_BYTES_PER_LINE; liOffset++)
        {
            if (liOffset < liHexSize)
            {
                if (apcInBuf[liOffset] != '\0' && strchr(CHARSET, apcInBuf[liOffset]) != NULL)
                {
                    *lpcPtr = apcInBuf[liOffset];
                }
                else
                {
                    *lpcPtr = '.';
                }
            }
            else
            {
                *lpcPtr = ' ';
            }

            ++lpcPtr;
        }

        *lpcPtr = '\n';
        ++lpcPtr;
    }

    *lpcPtr = '\0';

    AILogSys::WriteLOG(lpcBuffer);
    AI_DELETE_N(lpcBuffer);
}

void AILogSys::WriteCDR(char const* apcFmtStr, ...)
{
    va_list     ap;

    assert(apcFmtStr);
    if(strlen(apcFmtStr) <= 0)
    {
        return;
    }

    va_start(ap, apcFmtStr);

    AILogSys::Lock();
    AILogSys::InnerLOG(-1, apcFmtStr, ap);
    AILogSys::CheckBackup();
    AILogSys::Unlock();

    va_end(ap);
}

void AILogSys::WriteCDREx(char const* apcFmtStr, va_list ap)
{
    assert(apcFmtStr);
    if(strlen(apcFmtStr) <= 0)
    {
        return;
    }

    AILogSys::Lock();
    AILogSys::InnerLOG(-1, apcFmtStr, ap);
    AILogSys::CheckBackup();
    AILogSys::Unlock();
}

////////////////////////////////////////////////////////////////////////////////////
///
static LISTHEAD     soGlobalLogSysHead;
static AIMutexLock  soGlobalLogSysLock;

void AIInitLOGHandler(void) /* MUST after fork() */
{
    ai_init_list_head(&(soGlobalLogSysHead));
}

void AICloseLOGHandler(void)
{
    AILogSys        *lpoLogHandle = NULL;
    LISTHEAD        *lpoHead = &(soGlobalLogSysHead);
    AISmartLock     loSmartLock(soGlobalLogSysLock);

    while(!ai_list_is_empty(lpoHead))
    {
        LISTHEAD    *lpoTemp = NULL;
        
        ai_list_del_head(lpoTemp, lpoHead);
        lpoLogHandle = AI_GET_STRUCT_PTR(lpoTemp, AILogSys, coLogSysChild);
        
        delete  lpoLogHandle;
        lpoLogHandle = NULL;
    }
}

int GetSystemLevel(char *apcProgram)
{
    int     liRetCode;
    FILE    *lpoFp = NULL;
    char    lsBuf[128] = {0};

    sprintf(lsBuf, "%s.level", apcProgram);
    if((lpoFp = fopen(lsBuf, "r")) == NULL)
        return  (-1);

    memset(lsBuf, 0, sizeof(lsBuf));
    if(fgets(lsBuf, sizeof(lsBuf), lpoFp) == NULL)
    {
        fclose(lpoFp);
        return  (-2);
    }

    liRetCode = atoi(lsBuf);
    fclose(lpoFp);

    if(liRetCode >= 0 && liRetCode <= 12)
        return  (liRetCode);

    return  (-3);
}

/** 
    \brief change all log level in soGlobalLogSysHead link
    \return
*/
void AIDynChangeLOGLevel( const char *apcFileName )
{
    int             liLevel = 0;
    char            lsTmpString[64];
    AISmartLock     loSmartLock(soGlobalLogSysLock);

    if( apcFileName == NULL )
        GetFileName(gsGlobalProgName, lsTmpString, sizeof(lsTmpString));
    else
        strcpy( lsTmpString, apcFileName );

    liLevel = GetSystemLevel(lsTmpString);

    AIChangeLOGLevel( liLevel );
}


void AIChangeLOGLevel(short aiLevel)
{
    AILogSys*   lpoLogHandle = NULL;
    LISTHEAD*   lpoTemp = NULL;
    LISTHEAD*   lpoHead = &(soGlobalLogSysHead);

    if(aiLevel >= 0)
    {
        for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
        {
            lpoLogHandle = AI_GET_STRUCT_PTR(lpoTemp, AILogSys, coLogSysChild);
            if(lpoLogHandle->ciType == LOG_SYS_REC_TYPE_NOR /* Normal LOG */)
            {
                lpoLogHandle->SetLevel(aiLevel);
            }

            /* Force Backup Check */
            lpoLogHandle->ExecBackup();
        }

        giGlobalLogLevel = aiLevel;
    }
}

/** 
    \brief Search LOG Handler from soGlobalLogSysHead link by log file name:apcLogName
    \param apcLogName:the file name want to search
    \return
*/
AILogSys *AISearchLOGHandler(char const* apcLogName)
{
    LISTHEAD        *lpoTemp = NULL;
    AILogSys        *lpoLogHandle = NULL;
    LISTHEAD        *lpoHead = &(soGlobalLogSysHead);

    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoLogHandle = AI_GET_STRUCT_PTR(lpoTemp, AILogSys, coLogSysChild);
        if(strcmp(lpoLogHandle->csFullPathName, apcLogName) == 0)
        {
            return  (lpoLogHandle);
        }
    }

    return  (NULL);
}

/** 
    \brief Search LOG Handler from soGlobalLogSysHead link by log file name:apcLogName,then write it
    \ if search NULL then init a log and add it into soGlobalLogSysHead link
    \param apcLogName:the file name want to search
    \param aiLogLevel:log level
    \param apcFmtStr, ...:the string want to write
    \return
*/
void AIWriteLOG(char const* apcLogName, int aiLogLevel, char const* apcFmtStr, ...)
{
    char        lsTmpString[64] = {0};
    char        lsFieldName[64] = {0};
    char        lsConfigName[128] = {0};
    char        lsLogPathName[128] = {0};
    AIConfig    *lpoConfigHandle = NULL;
    AILogSys    *lpoLogSys = (AILogSys *)NULL;


    assert(apcFmtStr);
    GetFileName(gsGlobalProgName, lsTmpString, sizeof(lsTmpString));
    sprintf(lsConfigName, "%s/%slog.ini", gsConfigDirectory, lsTmpString);

    if(apcLogName == NULL || strlen(apcLogName) <= 0)
    {
        sprintf(lsLogPathName, "%s.out", lsTmpString);
    }
    else
    {
        strcpy(lsLogPathName, apcLogName);
        if(strchr(lsLogPathName, '.') == NULL)
        {
            strcat(lsLogPathName, ".log");
        }
    }

    soGlobalLogSysLock.Lock();
    GetFileName(lsLogPathName, lsFieldName, sizeof(lsFieldName));
    if((lpoLogSys = AISearchLOGHandler(lsLogPathName)) == NULL)
    {       
        soGlobalLogSysLock.Unlock();

        if((lpoConfigHandle = AIGetIniHandler(lsConfigName)))
        {
            int     liTimeVal = 0;
            int     liFileSize = 0;
            char    lsBakDir[64] = {0};

            liFileSize = lpoConfigHandle->GetIniInt(lsFieldName, "MaxFileSize", 0);
            liTimeVal = lpoConfigHandle->GetIniInt(lsFieldName, "MaxBakInterval", 0);
            lpoConfigHandle->GetIniString(lsFieldName, "BakDir", lsBakDir, sizeof(lsBakDir));

            if(liFileSize <= 0 && liTimeVal <= 0 && strlen(lsBakDir) <= 0)
            {
                liFileSize = lpoConfigHandle->GetIniInt("DEFAULTLOG", "MaxFileSize", 5120);
                liTimeVal = lpoConfigHandle->GetIniInt("DEFAULTLOG", "MaxBakInterval", 900);
                lpoConfigHandle->GetIniString("DEFAULTLOG", "BakDir", lsBakDir, sizeof(lsBakDir));
            }

            soGlobalLogSysLock.Lock();
            if((lpoLogSys = AISearchLOGHandler(lsLogPathName)) == NULL)
            {
                AI_NEW_ASSERT(lpoLogSys, AILogSys(lsLogPathName, LOG_SYS_REC_TYPE_NOR, lsBakDir, liFileSize, liTimeVal));
                ai_list_add_tail(&(lpoLogSys->coLogSysChild), &(soGlobalLogSysHead));
            }
            soGlobalLogSysLock.Unlock();

            lpoLogSys->SetLevel(giGlobalLogLevel);
        }
        else
        {
            soGlobalLogSysLock.Lock();
            if((lpoLogSys = AISearchLOGHandler(lsLogPathName)) == NULL)
            {
                AI_NEW_ASSERT(lpoLogSys, AILogSys(lsLogPathName, LOG_SYS_REC_TYPE_NOR));
                ai_list_add_tail(&(lpoLogSys->coLogSysChild), &(soGlobalLogSysHead));
            }
            soGlobalLogSysLock.Unlock();

            lpoLogSys->SetLevel(giGlobalLogLevel);
        }

        va_list     ap;
        va_start(ap, apcFmtStr);
        lpoLogSys->WriteLOGEx(aiLogLevel, apcFmtStr, ap);
        va_end(ap);
        return;
    }
    else
    {
        soGlobalLogSysLock.Unlock();
    }

    va_list     ap;
    va_start(ap, apcFmtStr);
    lpoLogSys->WriteLOGEx(aiLogLevel, apcFmtStr, ap);
    va_end(ap);
}


void AIWriteHexLOG(char const* apcLogName, char const* apcTitle, char const* apcInBuf, int aiBufSize)
{
    char        lsTmpString[64] = {0};
    char        lsFieldName[64] = {0};
    char        lsConfigName[128] = {0};
    char        lsLogPathName[128] = {0};
    AIConfig    *lpoConfigHandle = NULL;
    AILogSys    *lpoLogSys = (AILogSys *)NULL;


    GetFileName(gsGlobalProgName, lsTmpString, sizeof(lsTmpString));
    sprintf(lsConfigName, "%s/%slog.ini", gsConfigDirectory, lsTmpString);

    if(apcLogName == NULL || strlen(apcLogName) <= 0)
    {
        sprintf(lsLogPathName, "%s.out", lsTmpString);
    }
    else
    {
        strcpy(lsLogPathName, apcLogName);
        if(strchr(lsLogPathName, '.') == NULL)
        {
            strcat(lsLogPathName, ".log");
        }
    }

    soGlobalLogSysLock.Lock();
    GetFileName(lsLogPathName, lsFieldName, sizeof(lsFieldName));
    if((lpoLogSys = AISearchLOGHandler(lsLogPathName)) == NULL)
    {       
        soGlobalLogSysLock.Unlock();

        if((lpoConfigHandle = AIGetIniHandler(lsConfigName)))
        {
            int     liTimeVal = 0;
            int     liFileSize = 0;
            char    lsBakDir[64] = {0};

            liFileSize = lpoConfigHandle->GetIniInt(lsFieldName, "MaxFileSize", 0);
            liTimeVal = lpoConfigHandle->GetIniInt(lsFieldName, "MaxBakInterval", 0);
            lpoConfigHandle->GetIniString(lsFieldName, "BakDir", lsBakDir, sizeof(lsBakDir));

            if(liFileSize <= 0 && liTimeVal <= 0 && strlen(lsBakDir) <= 0)
            {
                liFileSize = lpoConfigHandle->GetIniInt("DEFAULTLOG", "MaxFileSize", 5120);
                liTimeVal = lpoConfigHandle->GetIniInt("DEFAULTLOG", "MaxBakInterval", 900);
                lpoConfigHandle->GetIniString("DEFAULTLOG", "BakDir", lsBakDir, sizeof(lsBakDir));
            }

            soGlobalLogSysLock.Lock();
            if((lpoLogSys = AISearchLOGHandler(lsLogPathName)) == NULL)
            {
                AI_NEW_ASSERT(lpoLogSys, AILogSys(lsLogPathName, LOG_SYS_REC_TYPE_NOR, lsBakDir, liFileSize, liTimeVal));
                ai_list_add_tail(&(lpoLogSys->coLogSysChild), &(soGlobalLogSysHead));
            }
            soGlobalLogSysLock.Unlock();

            lpoLogSys->SetLevel(giGlobalLogLevel);
        }
        else
        {
            soGlobalLogSysLock.Lock();
            if((lpoLogSys = AISearchLOGHandler(lsLogPathName)) == NULL)
            {
                AI_NEW_ASSERT(lpoLogSys, AILogSys(lsLogPathName, LOG_SYS_REC_TYPE_NOR));
                ai_list_add_tail(&(lpoLogSys->coLogSysChild), &(soGlobalLogSysHead));
            }
            soGlobalLogSysLock.Unlock();

            lpoLogSys->SetLevel(giGlobalLogLevel);
        }

        lpoLogSys->WriteHexLOG(apcTitle, apcInBuf, aiBufSize);
        return;
    }
    else
    {
        soGlobalLogSysLock.Unlock();
    }

    lpoLogSys->WriteHexLOG(apcTitle, apcInBuf, aiBufSize);
}

/** 
    \brief Search LOG Handler from soGlobalLogSysHead link by log file name:apcLogName,then write it
    \ if search NULL then init a log and add it into soGlobalLogSysHead link
    \param apcLogName:the file name want to search
    \param apcFmtStr, ...: the string want to write
    \return
*/
void AIWriteCDR(char const* apcLogName, char const* apcFmtStr, ...)
{
    char        lsTmpString[64] = {0};
    char        lsFieldName[64] = {0};
    char        lsConfigName[128] = {0};
    char        lsLogPathName[128] = {0};
    AIConfig    *lpoConfigHandle = NULL;
    AILogSys    *lpoLogSys = (AILogSys *)NULL;


    assert(apcFmtStr);
    assert(apcLogName);

    GetFileName(gsGlobalProgName, lsTmpString, sizeof(lsTmpString));
    sprintf(lsConfigName, "%s/%slog.ini", gsConfigDirectory, lsTmpString);
    strcpy(lsLogPathName, apcLogName);

    if(strchr(lsLogPathName, '.') == NULL)
    {
        strcat(lsLogPathName, ".log");
    }

    soGlobalLogSysLock.Lock();
    GetFileName(lsLogPathName, lsFieldName, sizeof(lsFieldName));
    if((lpoLogSys = AISearchLOGHandler(lsLogPathName)) == NULL)
    {       
        soGlobalLogSysLock.Unlock();

        if((lpoConfigHandle = AIGetIniHandler(lsConfigName)))
        {
            int     liTimeVal = 0;
            int     liFileSize = 0;
            char    lsBakDir[64] = {0};

            liFileSize = lpoConfigHandle->GetIniInt(lsFieldName, "MaxFileSize", 0);
            liTimeVal = lpoConfigHandle->GetIniInt(lsFieldName, "MaxBakInterval", 0);
            lpoConfigHandle->GetIniString(lsFieldName, "BakDir", lsBakDir, sizeof(lsBakDir));

            if(liFileSize <= 0 && liTimeVal <= 0 && strlen(lsBakDir) <= 0)
            {
                liFileSize = lpoConfigHandle->GetIniInt("DEFAULTCDR", "MaxFileSize", 5120);
                liTimeVal = lpoConfigHandle->GetIniInt("DEFAULTCDR", "MaxBakInterval", 900);
                lpoConfigHandle->GetIniString("DEFAULTCDR", "BakDir", lsBakDir, sizeof(lsBakDir));
            }

            soGlobalLogSysLock.Lock();
            if((lpoLogSys = AISearchLOGHandler(lsLogPathName)) == NULL)
            {
                AI_NEW_ASSERT(lpoLogSys, AILogSys(lsLogPathName, LOG_SYS_REC_TYPE_CDR, lsBakDir, liFileSize, liTimeVal));
                ai_list_add_tail(&(lpoLogSys->coLogSysChild), &(soGlobalLogSysHead));
            }
            soGlobalLogSysLock.Unlock();
        }
        else
        {
            soGlobalLogSysLock.Lock();
            if((lpoLogSys = AISearchLOGHandler(lsLogPathName)) == NULL)
            {
                AI_NEW_ASSERT(lpoLogSys, AILogSys(lsLogPathName, LOG_SYS_REC_TYPE_CDR));
                ai_list_add_tail(&(lpoLogSys->coLogSysChild), &(soGlobalLogSysHead));
            }
            soGlobalLogSysLock.Unlock();
        }

        va_list     ap;
        va_start(ap, apcFmtStr);
        lpoLogSys->WriteCDREx(apcFmtStr, ap);
        va_end(ap);
        return;
    }
    else
    {
        soGlobalLogSysLock.Unlock();
    }

    va_list     ap;
    va_start(ap, apcFmtStr);
    lpoLogSys->WriteCDREx(apcFmtStr, ap);
    va_end(ap);
}

///end namespace
AIBC_NAMESPACE_END
