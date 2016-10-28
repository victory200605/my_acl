#include "Test.h"
#include "acl/FileLog.h"
#include "acl/ThreadManager.h"

using namespace acl;

class CCustomLogFormat
{
public:
    apl_size_t operator () (
        apl_int_t aiLevel,
        char const* apcFormat,
        va_list& aoVl,
        char* apcBuffer,
        apl_size_t aiBufferSize )
    {
        apl_size_t liLen = 0;
        char*      lpcCurrBuffer = apcBuffer;
        apl_size_t liCurrSize = aiBufferSize;
    
        apl_snprintf(lpcCurrBuffer, liCurrSize, "%s", "Custom Log# " );
        liLen = apl_strlen(lpcCurrBuffer);
        lpcCurrBuffer += liLen;
        liCurrSize -= liLen;

        apl_snprintf(
            lpcCurrBuffer,
            liCurrSize,
            "%"PRId32,
            (apl_int32_t)apl_thread_self() );
        liLen = apl_strlen(lpcCurrBuffer);
        lpcCurrBuffer += liLen;
        liCurrSize -= liLen;
    
        switch(aiLevel)
        {
            case LOG_LVL_SYSTEM:
            {
                apl_strncpy(lpcCurrBuffer, " SYSTEM: ", liCurrSize);
                break;
            }
            case LOG_LVL_ERROR:
            {
                apl_strncpy(lpcCurrBuffer, " ERROR: ", liCurrSize);
                break;
            }
            case LOG_LVL_WARN:
            {
                apl_strncpy(lpcCurrBuffer, " WARN: ", liCurrSize);
                break;
            }
            case LOG_LVL_INFO:
            {
                apl_strncpy(lpcCurrBuffer, " INFO: ", liCurrSize);
                break;
            }
            case LOG_LVL_DEBUG:
            {
                apl_strncpy(lpcCurrBuffer, " DEBUG: ", liCurrSize);
                break;
            }
            case LOG_LVL_TRACE:
            {
                apl_strncpy(lpcCurrBuffer, " TRACE: ", liCurrSize);
                break;
            }
            default:
            {
                apl_strncpy(lpcCurrBuffer, " ANY: ", liCurrSize);
                break;
            }
        };
        liLen = apl_strlen(lpcCurrBuffer);
        lpcCurrBuffer += liLen;
        liCurrSize -= liLen;
    
        //HP-UX will return -1 when buffer is no enough
        liLen = apl_vsnprintf(lpcCurrBuffer, liCurrSize, apcFormat, aoVl);
        if ( liLen < 0 || liLen >= liCurrSize )
        {
            liLen = apl_strlen(lpcCurrBuffer);
        }
        lpcCurrBuffer += liLen;
        liCurrSize -= liLen;
    
        if(*(lpcCurrBuffer - 1) != '\n')
        {
            *lpcCurrBuffer++ = '\n';
            liCurrSize -= 1;
        }
        return aiBufferSize - liCurrSize;
    }
};

class CTestAclFileLog: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclFileLog);
    CPPUNIT_TEST( testWriteNormal );
    CPPUNIT_TEST( testWriteCustom );
    CPPUNIT_TEST( testBackup );
    CPPUNIT_TEST( testSize );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}
    void testWriteNormal(void)
    {
        PRINT_TITLE_2(__func__);
        NormalLog("MyLog.log", "MyLogbk.log");
    }
    void testWriteCustom(void)
    {
        PRINT_TITLE_2(__func__);
        CustomLog("MyLogcm.log", "MyLogcmbk.log", CCustomLogFormat() );
    }
    void testBackup()
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CFile::Remove("testLog");
        CFile::Remove("testLogbk");
        CFileLog   loLog;
        apl_int_t  liRet= 0;
        apl_size_t luN= 0;
        char lac[]="write log";
        loLog.Open( "testLog" );
        loLog.Write(LOG_LVL_SYSTEM, lac );
        loLog.Write(LOG_LVL_ERROR, lac );
        loLog.Write(LOG_LVL_WARN, lac );
        loLog.Write(LOG_LVL_INFO, lac );
        loLog.Write(LOG_LVL_DEBUG, lac );
        loLog.Write(LOG_LVL_TRACE, lac );
        loLog.Write(LOG_LVL_ANY, lac );

        //case
        luN= loLog.GetSize();
        liRet= loLog.Backup( "testLogbk" );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("backup customfileLog failed.", (apl_int_t)0, liRet);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("backup customfileLog failed.", (apl_ssize_t)0, loLog.GetSize());
        loLog.Close();

        CFile coFile;
        liRet= coFile.Open("testLogbk", APL_O_RDWR |APL_O_CREAT, 0755);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("backup fileLog failed.", (apl_int_t)0, liRet);
        CFileInfo loFileInfo;
        coFile.GetFileInfo(loFileInfo);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("backuplog size failed.", loFileInfo.GetSize(), luN);
        coFile.Close();

        //end environment
    }
    void testSize(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CFile::Remove("testLog");
        CFileLog   loLog;
        apl_int_t  liRet= 0;
        apl_size_t luN= 0;
        char lac[]="write log";
        loLog.Open( "testLog" );
        loLog.Write(LOG_LVL_SYSTEM, lac );
        loLog.Write(LOG_LVL_ERROR, lac );
        loLog.Write(LOG_LVL_WARN, lac );
        loLog.Write(LOG_LVL_INFO, lac );
        loLog.Write(LOG_LVL_DEBUG, lac );
        loLog.Write(LOG_LVL_TRACE, lac );
        loLog.Write(LOG_LVL_ANY, lac );

        //case
        luN= loLog.GetSize();
        loLog.Close();

        CFile coFile;
        liRet= coFile.Open("testLog", APL_O_RDWR |APL_O_CREAT, 0755);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("open file failed.", (apl_int_t)0, liRet);
        CFileInfo loFileInfo;
        coFile.GetFileInfo(loFileInfo);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get size failed.", loFileInfo.GetSize(), luN);
        coFile.Close();

        //end environment
    }
    void NormalLog(const char* apcFile, const char* apcFilebk )
    {
        //start environment
        CFile::Remove(apcFile);
        CFile::Remove(apcFilebk);
        CThreadManager loTrdManager;
        CFileLog   loLog;
        apl_int_t  liRet= 0;

        //case
        liRet= loLog.Open( apcFile );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open log fail", (apl_int_t)0, liRet);
        START_THREAD_BODY(mybody, CFileLog, loLog)
            apl_int_t liRet= 0;
            apl_int_t liTrd= 0;
            apl_int_t liN= 0;
            apl_int_t liLen= 0;
            char lac[]= "my app write log";
            char lacD[CFileLog::MAX_LOG_BUFFER_SIZE];
            char*      lpcCurrBuffer = lacD;
            apl_size_t liCurrSize= CFileLog::MAX_LOG_BUFFER_SIZE;
            apl_snprintf(
                lpcCurrBuffer,
                liCurrSize,
                "%"PRIu64,
                (apl_uint64_t)apl_thread_self() );
            liTrd = apl_strlen(lpcCurrBuffer);
            liN= (apl_int_t) apl_strlen(lac);
            liRet= loLog.Write(LOG_LVL_SYSTEM, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(system) failed.", liRet, (apl_int_t)(27+liTrd+9+liN+1) );
            liRet= loLog.Write(LOG_LVL_ERROR, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(error) failed.", liRet, (apl_int_t)(27+liTrd+8+liN+1) );
            liRet= loLog.Write(LOG_LVL_WARN, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(warn) failed.", liRet, (apl_int_t)(27+liTrd+7+liN+1) );
            liRet= loLog.Write(LOG_LVL_INFO, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(info) failed.", liRet, (apl_int_t)(27+liTrd+7+liN+1) );
            liRet= loLog.Write(LOG_LVL_DEBUG, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(debug) failed.", liRet, (apl_int_t)(27+liTrd+8+liN+1) );
            liRet= loLog.Write(LOG_LVL_TRACE, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(trace) failed.", liRet, (apl_int_t)(27+liTrd+8+liN+1) );
            liRet= loLog.Write(LOG_LVL_ANY, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(any) failed.", liRet, (apl_int_t)(27+liTrd+6+liN+1) );
 
            lpcCurrBuffer= lacD;
            apl_snprintf(lacD, CFileLog::MAX_LOG_BUFFER_SIZE, "%s", "Direct Write#");
            liCurrSize= CFileLog::MAX_LOG_BUFFER_SIZE- apl_strlen(lacD);       
            lpcCurrBuffer += apl_strlen(lacD);
            apl_snprintf(
                lpcCurrBuffer,
                liCurrSize,
                "%"PRId32,
                (apl_int32_t)apl_thread_self() );
            liLen = apl_strlen(lpcCurrBuffer);
            lpcCurrBuffer += liLen;
            liCurrSize -= liLen;
            apl_snprintf(lpcCurrBuffer, liCurrSize, "%s", " write app log\n");
            liN= (apl_int_t) apl_strlen(lacD);
            liRet= loLog.Write(lacD, liN );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog failed.", liN, liRet);
            liRet= loLog.Write(lacD, liN );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog failed.", liN, liRet); 
        END_THREAD_BODY(mybody);
        liRet= loTrdManager.SpawnN(5, ThreadFunc, mybody );
        loTrdManager.WaitAll();
      
        //end environment
        loLog.Close();
    }

    void CustomLog(const char* apcFile, const char* apcFilebk ,CAnyLogFormat aoFormat )
    {
        //start environment
        CFile::Remove(apcFile);
        CFile::Remove(apcFilebk);
        CThreadManager loTrdManager;
        CFileLog   loLog(aoFormat);
        apl_int_t  liRet= 0;

        //case
        liRet= loLog.Open( apcFile );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open customlog fail", (apl_int_t)0, liRet);
        START_THREAD_BODY(mybody, CFileLog, loLog)
            apl_int_t liRet= 0;
            apl_int_t liTrd= 0;
            apl_int_t liN= 0;
            apl_int_t liLen= 0;
            char lac[]= "my app write log";
            char lacD[CFileLog::MAX_LOG_BUFFER_SIZE];
            char*      lpcCurrBuffer = lacD;
            apl_size_t liCurrSize= CFileLog::MAX_LOG_BUFFER_SIZE;
            apl_snprintf(
                lpcCurrBuffer,
                liCurrSize,
                "%"PRId32,
                (apl_int32_t)apl_thread_self() );
            liTrd = apl_strlen(lpcCurrBuffer);
        
            apl_snprintf(lacD, CFileLog::MAX_LOG_BUFFER_SIZE, "%s", "Direct Write#");
            liCurrSize= CFileLog::MAX_LOG_BUFFER_SIZE- apl_strlen(lacD);
            liN= (apl_int_t) apl_strlen(lac);
            liRet= loLog.Write(LOG_LVL_SYSTEM, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(system) failed.", liRet, (apl_int_t)(12+liTrd+9+liN+1) );
            liRet= loLog.Write(LOG_LVL_ERROR, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(error) failed.", liRet, (apl_int_t)(12+liTrd+8+liN+1) );
            liRet= loLog.Write(LOG_LVL_WARN, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(warn) failed.", liRet, (apl_int_t)(12+liTrd+7+liN+1) );
            liRet= loLog.Write(LOG_LVL_INFO, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(info) failed.", liRet, (apl_int_t)(12+liTrd+7+liN+1) );
            liRet= loLog.Write(LOG_LVL_DEBUG, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(debug) failed.", liRet, (apl_int_t)(12+liTrd+8+liN+1) );
            liRet= loLog.Write(LOG_LVL_TRACE, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(trace) failed.", liRet, (apl_int_t)(12+liTrd+8+liN+1) );
            liRet= loLog.Write(LOG_LVL_ANY, lac );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog(any) failed.", liRet, (apl_int_t)(12+liTrd+6+liN+1) );
 
            lpcCurrBuffer += apl_strlen(lacD);
            apl_snprintf(
                lpcCurrBuffer,
                liCurrSize,
                "%"PRId32,
                (apl_int32_t)apl_thread_self() );
            liLen = apl_strlen(lpcCurrBuffer);
            lpcCurrBuffer += liLen;
            liCurrSize -= liLen;
            apl_snprintf(lpcCurrBuffer, liCurrSize, "%s", " write app log\n");
            liN= (apl_int_t) apl_strlen(lacD);
            liRet= loLog.Write(lacD, liN );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog failed.", liN, liRet);
            liRet= loLog.Write(lacD, liN );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write fileLog failed.", liN, liRet);
        END_THREAD_BODY(mybody)
        liRet= loTrdManager.SpawnN(5, ThreadFunc, mybody);
        PrintErrMsg( apl_strerror(apl_get_errno()), liRet);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SpawnN fail", (apl_int_t)0, liRet);
        loTrdManager.WaitAll();

        //end environment
        loLog.Close();
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclFileLog);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
