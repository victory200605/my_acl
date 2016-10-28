#include "Test.h"
#include "acl/Process.h"

using namespace acl;

class CTestAclProcessOption:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclProcessOption);
    CPPUNIT_TEST(testCmdLine);
    CPPUNIT_TEST(testPrepare);
    CPPUNIT_TEST(testProcName);
    CPPUNIT_TEST(testSetGetHandle);
    CPPUNIT_TEST(testEnv);
    CPPUNIT_TEST(testWorkingDir);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testCmdLine(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CProcessOption loProcOpt;

        //case
        loProcOpt.SetCmdLine("%s -m %d -d", "a.out",12);

        loProcOpt.Prepare();
        char* const* lppcCmdLine = loProcOpt.GetCmdLine();
        CPPUNIT_ASSERT_MESSAGE("Set/GetCmdLine failed.", apl_strcmp(lppcCmdLine[0], "a.out")==0);
        CPPUNIT_ASSERT_MESSAGE("Set/GetCmdLine failed.", apl_strcmp(lppcCmdLine[1], "-m")==0);
        CPPUNIT_ASSERT_MESSAGE("Set/GetCmdLine failed.", apl_strcmp(lppcCmdLine[2], "12")==0);
        CPPUNIT_ASSERT_MESSAGE("Set/GetCmdLine failed.", apl_strcmp(lppcCmdLine[3], "-d")==0);

        //end environment
    }

    void testPrepare(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CProcessOption loProcOpt;

        //case
        loProcOpt.SetCmdLine("%s -m %d -d", "a.out",12);

        loProcOpt.Prepare();
        char* const* lppcCmdLine = loProcOpt.GetCmdLine();
        CPPUNIT_ASSERT_MESSAGE("Set/GetCmdLine failed.", apl_strcmp(lppcCmdLine[0], "a.out")==0);
        CPPUNIT_ASSERT_MESSAGE("Set/GetCmdLine failed.", apl_strcmp(lppcCmdLine[1], "-m")==0);
        CPPUNIT_ASSERT_MESSAGE("Set/GetCmdLine failed.", apl_strcmp(lppcCmdLine[2], "12")==0);
        CPPUNIT_ASSERT_MESSAGE("Set/GetCmdLine failed.", apl_strcmp(lppcCmdLine[3], "-d")==0);

        //end environment
    }

    void testProcName(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CProcessOption loProcOpt;
        char const* lpcProcName = "procName";

        //case
        loProcOpt.SetProcName(lpcProcName);

        char const* lpcProcNameOut = loProcOpt.GetProcName();
        CPPUNIT_ASSERT_MESSAGE("Set/GetProcName failed.", 
                apl_strcmp(lpcProcNameOut,lpcProcName)==0);

        //end environment
    }

    void testSetGetHandle(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CProcessOption loProcOpt; 
        
        //case
        loProcOpt.SetHandle(0);
        apl_handle_t liHandle = loProcOpt.GetStdIn();
        CPPUNIT_ASSERT_MESSAGE("Set/GetHandle failed.", liHandle > (apl_handle_t)2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Set/GetHandle failed.", 
                liHandle + 1 , loProcOpt.GetStdOut());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Set/GetHandle failed.", 
                liHandle + 2, loProcOpt.GetStdErr());
        

        loProcOpt.SetHandle(3, 4, 5);
        liHandle = loProcOpt.GetStdIn();
        CPPUNIT_ASSERT_MESSAGE("Set/GetHandle failed.", liHandle > (apl_handle_t)5);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Set/GetHandle failed.", 
                liHandle + 1, loProcOpt.GetStdOut());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Set/GetHandle failed.", 
                liHandle + 2, loProcOpt.GetStdErr());

        //end environment
    }

    void testEnv(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CProcessOption loProcOpt;

        //case
        loProcOpt.SetEnv("%s=%s","key1","value1");
        loProcOpt.SetEnv("%s=%s","key2","value2");
        loProcOpt.SetEnv("%s=%s","key3","value3");

        char* const* lppcEnv = loProcOpt.GetEnv();

        CPPUNIT_ASSERT_MESSAGE("Set/GetEnv failed.", apl_strcmp(lppcEnv[0], "key1=value1")==0);
        CPPUNIT_ASSERT_MESSAGE("Set/GetEnv failed.", apl_strcmp(lppcEnv[1], "key2=value2")==0);
        CPPUNIT_ASSERT_MESSAGE("Set/GetEnv failed.", apl_strcmp(lppcEnv[2], "key3=value3")==0);

        //end environment
    }

    void testWorkingDir(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CProcessOption loProcOpt;

        //case
        loProcOpt.SetWorkingDir("./tmpdir");

        char const* lpcDir = loProcOpt.GetWorkingDir();
        CPPUNIT_ASSERT_MESSAGE("Set/GetWorkingDir failed.", apl_strcmp(lpcDir, "./tmpdir")==0);

        //end environment
    }

};  

/* ----------------------------------------------------------------- */

static apl_int_t giFlag = 0;

void handler(int sig_no)
{
    giFlag = 1;
} 

class CTestAclProcess:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclProcess);
    CPPUNIT_TEST(testWait);
    CPPUNIT_TEST(testGetPid);
    CPPUNIT_TEST(testGetPpid);
    CPPUNIT_TEST(testKill);
    CPPUNIT_TEST(testProcessExec);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testWait(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start case environment
        CProcessOption loOption;
        CMyProcessWait loProcess;

        //case
        if ( loProcess.Spawn( loOption ) != 0 )
        {   
            CPPUNIT_FAIL("Spawn fail.");
        }

        //end environment
    }

    class CMyProcessWait:public CProcess
    {
    public:
        void Parent()
        {
            apl_int_t liExitCode = -1;
            this->Wait(&liExitCode);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wait failed.", (apl_int_t)0, liExitCode);
        }

        void Child()
        {
            apl_sleep(APL_TIME_SEC);

            exit(0);
        }
    };

    /* ------------------------------------------------------------- */

    void testGetPid(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CProcessOption loOption;
        CMyProcessGetPid loProcess;

        //case
        if ( loProcess.Spawn( loOption ) != 0 )
        {
            CPPUNIT_FAIL("Spawn fail.");
        }

        //end environment
    }

    class CMyProcessGetPid:public CProcess
    {
    public:
        CMyProcessGetPid()
        {
            apl_int_t liRet = apl_socketpair(APL_AF_UNIX, APL_SOCK_STREAM, 0, &ciFd[0], &ciFd[1]);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Pipe failed.", (apl_int_t)0, liRet);
        }

        void Parent()
        {
            close(ciFd[1]);

            char lsIDBuf[20];
            memset(lsIDBuf, 0, sizeof(lsIDBuf));
            read(ciFd[0],lsIDBuf, sizeof(lsIDBuf));

            apl_int_t liExitCode = -1;
            apl_int_t liChildID = this->Wait(&liExitCode);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wait failed.", (apl_int_t)0, liExitCode);

            char lsChkBuf[20];
            memset(lsChkBuf, 0, sizeof(lsChkBuf));
            snprintf(lsChkBuf, sizeof(lsChkBuf), "%d", (int)liChildID);
            CPPUNIT_ASSERT_MESSAGE("GetPid failed.", apl_strcmp(lsIDBuf, lsChkBuf) == 0);
        }

        void Child()
        {
            close(ciFd[0]);

            apl_int_t liChildID = this->GetPid();
            char lsIDBuf[20];
            memset(lsIDBuf, 0, sizeof(lsIDBuf));
            snprintf(lsIDBuf, sizeof(lsIDBuf), "%d", (int)liChildID);
            write(ciFd[1], lsIDBuf, apl_strlen(lsIDBuf));
            apl_sleep(APL_TIME_SEC);

            exit(0);
        }

    private:
        apl_handle_t ciFd[2];
    };

    /* ------------------------------------------------------------- */

    void testGetPpid(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CProcessOption loOption;
        CMyProcessGetPpid loProcess;

        //case
        if ( loProcess.Spawn( loOption ) != 0 )
        {
            CPPUNIT_FAIL("Spawn fail.");
        }

        //end environment
    }

    class CMyProcessGetPpid:public CProcess
    {
    public:
        CMyProcessGetPpid()
        {
            apl_int_t liRet = apl_socketpair(APL_AF_UNIX, APL_SOCK_STREAM, 0, &ciFd[0], &ciFd[1]);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Pipe failed.", (apl_int_t)0, liRet);
        }

        void Parent()
        {
            close(ciFd[1]);

            char lsIDBuf[20];
            memset(lsIDBuf, 0, sizeof(lsIDBuf));
            read(ciFd[0],lsIDBuf, sizeof(lsIDBuf));

            apl_int_t liExitCode = -1;
            this->Wait(&liExitCode);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wait failed.", (apl_int_t)0, liExitCode);

            apl_int_t liParentID = this->GetPid();
            char lsChkBuf[20];
            memset(lsChkBuf, 0, sizeof(lsChkBuf));
            snprintf(lsChkBuf, sizeof(lsChkBuf), "%d", (int)liParentID);
            CPPUNIT_ASSERT_MESSAGE("GetPid and GetPpid failed.", 
                    apl_strcmp(lsIDBuf, lsChkBuf) == 0);
        }

        void Child()
        {
            close(ciFd[0]);

            apl_int_t liParentID = this->GetPpid();
            char lsIDBuf[20];
            memset(lsIDBuf, 0, sizeof(lsIDBuf));
            snprintf(lsIDBuf, sizeof(lsIDBuf), "%d", (int)liParentID);
            write(ciFd[1], lsIDBuf, apl_strlen(lsIDBuf));
            apl_sleep(APL_TIME_SEC);

            exit(0);
        }

    private:
        apl_handle_t ciFd[2];
    };

    /* ------------------------------------------------------------- */

    void testKill(void)
    {
        PRINT_TITLE_2(__func__); 

        //start case environment
        CProcessOption loOption;
        CMyProcessKill loProcess;   

        //case
        if ( loProcess.Spawn( loOption ) != 0 )
        {                                      
            CPPUNIT_FAIL("Spawn fail.");       
        }    

        //end environment
    }

    class CMyProcessKill:public CProcess
    {
    public:
        void Parent()
        {
            apl_sleep(100*APL_TIME_MSEC);
            this->Kill(APL_SIGUSR1);
        }

        void Child()
        {
            apl_signal(APL_SIGUSR1, handler);
            apl_sleep(500*APL_TIME_MSEC);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Kill failed.", (apl_int_t)1, giFlag);
            exit(0);
        }
    };

    /* ------------------------------------------------------------- */
    void testProcessExec(void)
    {
        PRINT_TITLE_2(__func__);
        apl_int_t liExitCode = 0;

        //start  case environment
        apl_handle_t liHandle = -1;
        CProcessOption loOption;
        loOption.SetProcName("testProExe");
        loOption.SetEnv("%s=%s","key1","value1");
        loOption.SetEnv("%s=%s","key2","value2");
        loOption.SetEnv("%s=%s", "LD_LIBRARY_PATH", "/usr/local/lib");

        char const* lpcFilename = "execfile.txt";
        liHandle = apl_open(lpcFilename, APL_O_RDWR |APL_O_CREAT, 0666);
        loOption.PassHandle(liHandle);
        loOption.SetCmdLine("%s %d","testProExe", liHandle);

        //case
        CProcess loProcess;
        if ( loProcess.Spawn( loOption ) != 0 )
        {
            CPPUNIT_FAIL("Spawn fail.");
        }
        loProcess.Wait(&liExitCode);
        apl_close(liHandle);
        liHandle = apl_open(lpcFilename, APL_O_RDONLY, 0666);
        char lsBuf[40];
        apl_memset(lsBuf, 0, sizeof(lsBuf));
        apl_ssize_t liSize = apl_read(liHandle, lsBuf, sizeof(lsBuf));
        char const*lpcExpectOutput = "key1=value1,key2=value2";
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test exec child process failed.", 
               (apl_ssize_t)apl_strlen(lpcExpectOutput), liSize);
        CPPUNIT_ASSERT_MESSAGE("Test exec child process failed.", 
                apl_strcmp(lsBuf, lpcExpectOutput) == 0);

        //end environment
        apl_close(liHandle);

    }
};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclProcessOption);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclProcess);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

