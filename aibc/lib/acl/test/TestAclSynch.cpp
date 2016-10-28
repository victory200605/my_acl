#include "Test.h"
#include "acl/Synch.h"

using namespace acl;

class CTestAclNullLock:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclNullLock);
    CPPUNIT_TEST(testNullLock);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testNullLock(void)
    {
        PrintTitle("CTestAclNullLock", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        struct stNullLock                                             
        {                                                             
            stNullLock(apl_int_t liCnt) : ciCount(liCnt) {}

            apl_int_t ciCount;                                        
            CNullLock coLock;                                         
        };                                                            
        stNullLock loNullLock(0);

        //case
        START_THREAD_BODY(mybody, stNullLock, loNullLock)
        {
            loNullLock.coLock.Lock();      
            loNullLock.ciCount++;          
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testNullLock failed.", (apl_int_t)1, loNullLock.ciCount);

            apl_sleep(500*APL_TIME_MSEC);

            loNullLock.ciCount++;                                                              
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testNullLock failed.", (apl_int_t)1, loNullLock.ciCount);
            loNullLock.coLock.Unlock();    
        }
        END_THREAD_BODY(mybody)            
        RUN_THREAD_BODY(mybody)        

        WAIT_EXP(loNullLock.ciCount == 1);

        START_THREAD_BODY(mybody2, stNullLock, loNullLock)
        {
            loNullLock.coLock.Lock();      
            loNullLock.ciCount--;          
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testNullLock failed.", (apl_int_t)0, loNullLock.ciCount);
            loNullLock.coLock.Unlock();    
        }
        END_THREAD_BODY(mybody2)            
        RUN_THREAD_BODY(mybody2)        

        apl_sleep(500*APL_TIME_MSEC);

        //end environment
    }
};

class CTestAclLock:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclLock);
    CPPUNIT_TEST(testLock);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testLock(void)
    {
        PrintTitle("CTestAclLock", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        struct stLock
        {
            stLock(apl_int_t liCnt) : ciCount(liCnt) {}
            apl_int_t ciCount;
            CLock coLock;
        };

        stLock loLock(0);

        //case
        START_THREAD_BODY(mybody, stLock, loLock)
        {
            loLock.coLock.Lock();
            loLock.ciCount++;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testLock failed.", (apl_int_t)1, loLock.ciCount);

            apl_sleep(100*APL_TIME_MSEC);

            loLock.ciCount++;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testLock failed.", (apl_int_t)2, loLock.ciCount);
            loLock.coLock.Unlock();
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        WAIT_EXP(loLock.ciCount == 1);

        START_THREAD_BODY(mybody2, stLock, loLock)
        {
            loLock.coLock.Lock();
            loLock.ciCount--;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testLock failed.", (apl_int_t)1, loLock.ciCount);
            loLock.coLock.Unlock();
        }
        END_THREAD_BODY(mybody2)
        RUN_THREAD_BODY(mybody2)

        apl_sleep(500*APL_TIME_MSEC);

        //end environment
    }
};

class CTestAclSpinLock:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclSpinLock);
    CPPUNIT_TEST(testSpinLock);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testSpinLock(void)
    {
        PrintTitle("CTestAclSpinLock", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        struct stSpinLock
        {
            stSpinLock(apl_int_t liCnt) : ciCount(liCnt) {}
            apl_int_t ciCount;
            CSpinLock coLock;
        };

        stSpinLock loSpinLock(0);

        //case
        START_THREAD_BODY(mybody, stSpinLock, loSpinLock)
        {
            loSpinLock.coLock.Lock();
            loSpinLock.ciCount++;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testSpinLock failed.", (apl_int_t)1, loSpinLock.ciCount);

            apl_sleep(500*APL_TIME_MSEC);

            loSpinLock.ciCount++;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testSpinLock failed.", (apl_int_t)2, loSpinLock.ciCount);
            loSpinLock.coLock.Unlock();
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        WAIT_EXP(loSpinLock.ciCount == 1);

        START_THREAD_BODY(mybody2, stSpinLock, loSpinLock)
        {
            loSpinLock.coLock.Lock();
            loSpinLock.ciCount--;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testSpinLock failed.", (apl_int_t)1,loSpinLock.ciCount);
            loSpinLock.coLock.Unlock();
        }
        END_THREAD_BODY(mybody2)
        RUN_THREAD_BODY(mybody2)

        apl_sleep(500*APL_TIME_MSEC);

        //end environment
    }
};

class CTestAclFileLock:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclFileLock);
    CPPUNIT_TEST(testFileLock);
    CPPUNIT_TEST(testFileTryLock);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testFileLock(void)
    {
        PrintTitle("CTestAclFileLock", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        char const* lpcFile = "tmp.txt";
        apl_unlink(lpcFile);
        apl_int_t liRet = -1;

        CFileLock loFileLock(lpcFile);
        apl_pid_t liChildID = apl_fork();
        if(liChildID < 0)
        {
            CPPUNIT_FAIL("Fork failed");
        }
        else if(liChildID == 0)
        {
            //case
            liRet = loFileLock.Lock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testFileLock - Lock failed.", (apl_int_t)0, liRet);
            apl_handle_t liFd = apl_open(lpcFile, APL_O_RDWR|APL_O_APPEND, 0666);
            apl_write(liFd, "abcd", 4);

            apl_sleep(100*APL_TIME_MSEC);

            apl_write(liFd, "abcd", 4);
            apl_close(liFd);
            liRet = loFileLock.Unlock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testFileLock - Unlock failed.", (apl_int_t)0, liRet);
            exit(0);
        }
        else
        {
            apl_sleep(10*APL_TIME_MSEC);

            loFileLock.Lock();
            apl_handle_t liFd = apl_open(lpcFile, APL_O_RDWR|APL_O_APPEND, 0666);
            apl_write(liFd, "1234", 4);
            apl_close(liFd);
            loFileLock.Unlock();
        }

        if(apl_waitpid(liChildID, NULL, 0) != liChildID)
        {
            CPPUNIT_FAIL("Wait pid failed.");
        }

        char const *lpcContent = "abcdabcd1234";
        char lsBuf[20];
        memset(lsBuf, 0, sizeof(lsBuf));
        apl_int_t liFd = apl_open(lpcFile, APL_O_RDONLY, 0666);
        apl_read(liFd, lsBuf, sizeof(lsBuf));
        apl_close(liFd);
        CPPUNIT_ASSERT_MESSAGE("testFileLock failed.", apl_strcmp(lsBuf, lpcContent)==0);

        //end environment
        apl_unlink(lpcFile);
    }

    void testFileTryLock(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char const* lpcFile = "tmp.txt";
        apl_unlink(lpcFile);
        apl_int_t liRet = -1;

        CFileLock loFileLock(lpcFile);
        apl_pid_t liChildID = apl_fork();
        if(liChildID < 0)
        {
            CPPUNIT_FAIL("Fork failed");
        }
        else if(liChildID == 0)
        {
            liRet = loFileLock.Lock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testFileTryLock - Lock failed.", (apl_int_t)0, liRet);
            apl_handle_t liFd = apl_open(lpcFile, APL_O_RDWR|APL_O_APPEND, 0666);
            apl_write(liFd, "abcd", 4);

            apl_sleep(100*APL_TIME_MSEC);

            apl_write(liFd, "abcd", 4);
            apl_close(liFd);
            liRet = loFileLock.Unlock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testFileTryLock - Unlock failed.", (apl_int_t)0, liRet);
            exit(0);
        }
        else
        {
            apl_sleep(10*APL_TIME_MSEC);

            //case
            liRet = loFileLock.TryLock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("TryLock failed.", (apl_int_t)-1, liRet);

            apl_sleep(100*APL_TIME_MSEC);
            liRet = loFileLock.TryLock();

            apl_handle_t liFd = apl_open(lpcFile, APL_O_RDWR|APL_O_APPEND, 0666);
            apl_write(liFd, "1234", 4);
            apl_close(liFd);
            loFileLock.Unlock();
        }

        if(apl_waitpid(liChildID, NULL, 0) != liChildID)
        {
            CPPUNIT_FAIL("Wait pid failed.");
        }

        char const *lpcContent = "abcdabcd1234";
        char lsBuf[20];
        memset(lsBuf, 0, sizeof(lsBuf));
        apl_int_t liFd = apl_open(lpcFile, APL_O_RDONLY, 0666);
        apl_read(liFd, lsBuf, sizeof(lsBuf));
        apl_close(liFd);
        CPPUNIT_ASSERT_MESSAGE("testFileTryLock failed.", apl_strcmp(lsBuf, lpcContent)==0);

        //end environment
        apl_unlink(lpcFile);
    }
};

class CTestAclNullRWLock:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclNullRWLock);
    CPPUNIT_TEST(testNullRWLock);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testNullRWLock(void)
    {
        PrintTitle("CTestAclNullRWLock", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liRet = -1;
        CNullRWLock loNullRWLock;

        //case
        liRet = loNullRWLock.RLock();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("RLock failed.", (apl_int_t)0, liRet);

        liRet = loNullRWLock.WLock();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("WLock failed.", (apl_int_t)0, liRet);

        liRet = loNullRWLock.Unlock();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlock failed.", (apl_int_t)0, liRet);

        //end environment
    }
};

class CTestAclRWLock:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclRWLock);
    CPPUNIT_TEST(testRLock);
    CPPUNIT_TEST(testWLock);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testRLock(void)
    {
        PrintTitle("CTestAclRWLock", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        struct stRWLock
        {
            stRWLock(apl_int_t aiNum) : ciNum(aiNum) {}
            apl_int_t ciNum;
            CRWLock coLock;
        };

        stRWLock loRWLock(0);

        //case
        START_THREAD_BODY(mybody, stRWLock, loRWLock)
        {
            apl_int_t liRet = loRWLock.coLock.RLock();    
            CPPUNIT_ASSERT_EQUAL_MESSAGE("RLock failed.", (apl_int_t)0, liRet);
            loRWLock.ciNum = 1;
            
            WAIT_EXP(loRWLock.ciNum == 3);

            loRWLock.coLock.Unlock();
            loRWLock.ciNum = 4;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        //can RLock again
        START_THREAD_BODY(mybody2, stRWLock, loRWLock)
        {
            WAIT_EXP(loRWLock.ciNum == 1);
            apl_int_t liRet = loRWLock.coLock.RLock();    
            CPPUNIT_ASSERT_EQUAL_MESSAGE("RLock failed.", (apl_int_t)0, liRet);
            loRWLock.ciNum = 2;
            loRWLock.coLock.Unlock();
        }
        END_THREAD_BODY(mybody2)
        RUN_THREAD_BODY(mybody2)

        //can't WLock while RLock
        START_THREAD_BODY(mybody3, stRWLock, loRWLock)
        {
            WAIT_EXP(loRWLock.ciNum == 2);
            CTimeValue loTimeValue(500, APL_TIME_MSEC);
            apl_int_t liRet = loRWLock.coLock.WLock(loTimeValue);    
            CPPUNIT_ASSERT_EQUAL_MESSAGE("WLock failed.", (apl_int_t)-1, liRet);
            loRWLock.ciNum = 3;
        }
        END_THREAD_BODY(mybody3)
        RUN_THREAD_BODY(mybody3)

        //can WLock after RLock
        START_THREAD_BODY(mybody4, stRWLock, loRWLock)
        {
            WAIT_EXP(loRWLock.ciNum == 4);   
            apl_int_t liRet = loRWLock.coLock.WLock();    
            CPPUNIT_ASSERT_EQUAL_MESSAGE("WLock failed.", (apl_int_t)0, liRet);
            loRWLock.coLock.Unlock();
            loRWLock.ciNum = 5;
        }
        END_THREAD_BODY(mybody4)
        RUN_THREAD_BODY(mybody4)

        WAIT_EXP(loRWLock.ciNum == 5);   
        //end environment
    }

    void testWLock(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        struct stRWLock                                
        {
            stRWLock(apl_int_t aiNum) : ciNum(aiNum) {}
            apl_int_t ciNum;
            CRWLock coLock;                            
        };  

        //WLock
        stRWLock loRWLock(0);

        //case
        START_THREAD_BODY(mybody, stRWLock, loRWLock)  
        {
            loRWLock.coLock.WLock();
            loRWLock.ciNum = 1;
            WAIT_EXP(loRWLock.ciNum == 3);
            loRWLock.coLock.Unlock();
            loRWLock.ciNum = 4;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        //can't WLock again while WLock
        START_THREAD_BODY(mybody2, stRWLock, loRWLock)  
        {
            WAIT_EXP(loRWLock.ciNum == 1);
            CTimeValue loTimeValue(500, APL_TIME_MSEC);
            apl_int_t liRet = loRWLock.coLock.WLock(loTimeValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("WLock failed.", (apl_int_t)-1, liRet);
            loRWLock.ciNum = 2;
        }
        END_THREAD_BODY(mybody2)
        RUN_THREAD_BODY(mybody2)
    
        //can't RLock while WLock
        START_THREAD_BODY(mybody3, stRWLock, loRWLock)  
        {
            WAIT_EXP(loRWLock.ciNum == 2);   
            CTimeValue loTimeValue(500, APL_TIME_MSEC);
            apl_int_t liRet = loRWLock.coLock.RLock(loTimeValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("WLock failed.", (apl_int_t)-1, liRet);
            loRWLock.ciNum = 3;
        }
        END_THREAD_BODY(mybody3)
        RUN_THREAD_BODY(mybody3)
        
        WAIT_EXP(loRWLock.ciNum == 4);
        //end environment
    }
};

class CTestAclFileRWLock:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclFileRWLock);
    CPPUNIT_TEST(testFileRLock);
    CPPUNIT_TEST(testFileWLock);
    //CPPUNIT_TEST(testFileRTryLock);
    CPPUNIT_TEST(testFileWTryLock);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testFileRLock(void)
    {
        PrintTitle("CTestAclFileRWLock", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        char const* lpcFile = "tmp.txt";
        apl_unlink(lpcFile);
        apl_int_t liRet = -1;

        CFileRWLock loFileRWLock(lpcFile);
        apl_pid_t liChildID = apl_fork();
        if(liChildID < 0)
        {
            CPPUNIT_FAIL("Fork failed");
        }
        else if(liChildID == 0)
        {
            //case
            liRet = loFileRWLock.RLock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("RLock failed.", (apl_int_t)0, liRet);
            apl_sleep(APL_TIME_SEC);
            loFileRWLock.Unlock();
            exit(0);
        }
        else
        {
            apl_sleep(10*APL_TIME_MSEC);

            //can RLock while RLock
            liRet = loFileRWLock.RLock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("RLock failed.", (apl_int_t)0, liRet);
            loFileRWLock.Unlock();

            //can't WLock while RLock
            liRet = loFileRWLock.WTryLock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("WTryLock failed.", (apl_int_t)-1, liRet);

            if(apl_waitpid(liChildID, NULL, 0) != liChildID)
            {
                CPPUNIT_FAIL("Wait pid failed.");
            }

            //end environment
            apl_unlink(lpcFile);
        }

    }

    void testFileWLock(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char const* lpcFile = "tmp.txt";
        apl_unlink(lpcFile);
        apl_int_t liRet = -1;

        CFileRWLock loFileRWLock(lpcFile);
        apl_pid_t liChildID = apl_fork();
        if(liChildID < 0)
        {
            CPPUNIT_FAIL("Fork failed");
        }
        else if(liChildID == 0)
        {
            //case
            liRet = loFileRWLock.WLock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("WLock failed.", (apl_int_t)0, liRet);
            apl_sleep(APL_TIME_SEC);
            loFileRWLock.Unlock();
            exit(0);
        }
        else
        {
            apl_sleep(10*APL_TIME_MSEC);

            //can't RLock while WLock
            liRet = loFileRWLock.RTryLock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("RTryLock failed.", (apl_int_t)-1, liRet);
            loFileRWLock.Unlock();

            //can't WLock while WLock
            liRet = loFileRWLock.WTryLock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("WTryLock failed.", (apl_int_t)-1, liRet);

            if(apl_waitpid(liChildID, NULL, 0) != liChildID)
            {
                CPPUNIT_FAIL("Wait pid failed.");
            }

            //end environment
            apl_unlink(lpcFile);
        }
    }

    void testFileRTryLock(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char const* lpcFile = "tmp.txt";
        apl_unlink(lpcFile);
        apl_int_t liRet = -1;

        CFileRWLock loFileRWLock(lpcFile);
        apl_pid_t liChildID = apl_fork();
        if(liChildID < 0)
        {
            CPPUNIT_FAIL("Fork failed");
        }
        else if(liChildID == 0)
        {
            liRet = loFileRWLock.WLock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("WLock failed.", (apl_int_t)0, liRet);
            apl_sleep(500*APL_TIME_MSEC);
            loFileRWLock.Unlock();
            exit(0);
        }
        else
        {
            apl_sleep(10*APL_TIME_MSEC);

            //case
            liRet = loFileRWLock.RTryLock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("RTryLock failed.", (apl_int_t)-1, liRet);
            loFileRWLock.Unlock();

            apl_sleep(500*APL_TIME_MSEC);
            liRet = loFileRWLock.RTryLock();                                       
            CPPUNIT_ASSERT_EQUAL_MESSAGE("RTryLock failed.", (apl_int_t)0, liRet);
            loFileRWLock.Unlock();                                       

            if(apl_waitpid(liChildID, NULL, 0) != liChildID)
            {
                CPPUNIT_FAIL("Wait pid failed.");
            }

            //end environment
            apl_unlink(lpcFile);
        }
    }

    void testFileWTryLock(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char const* lpcFile = "tmp.txt";
        apl_unlink(lpcFile);
        apl_int_t liRet = -1;

        CFileRWLock loFileRWLock(lpcFile);
        apl_pid_t liChildID = apl_fork();
        if(liChildID < 0)
        {
            CPPUNIT_FAIL("Fork failed");
        }
        else if(liChildID == 0)
        {
            liRet = loFileRWLock.WLock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("WLock failed.", (apl_int_t)0, liRet);
            apl_sleep(500*APL_TIME_MSEC);
            loFileRWLock.Unlock();
            exit(0);
        }
        else
        {
            apl_sleep(10*APL_TIME_MSEC);

            //case
            liRet = loFileRWLock.WTryLock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("WTryLock failed.", (apl_int_t)-1, liRet);
            loFileRWLock.Unlock();

            apl_sleep(500*APL_TIME_MSEC);
            liRet = loFileRWLock.WTryLock();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("WTryLock failed.", (apl_int_t)0, liRet);
            loFileRWLock.Unlock();

            if(apl_waitpid(liChildID, NULL, 0) != liChildID)
            {
                CPPUNIT_FAIL("Wait pid failed.");
            }

            //end environment
            apl_unlink(lpcFile);
        }
    }
};

class CTestAclSmartLock:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclSmartLock);
    CPPUNIT_TEST(testSmartLock);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testSmartLock(void)
    {
        PrintTitle("CTestAclSmartLock", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        struct stLock
        {
            stLock(apl_int_t liCnt) : ciCount(liCnt) {}
            apl_int_t ciCount;
            CLock coLock;
        };

        stLock loLock(0);

        //case
        START_THREAD_BODY(mybody, stLock, loLock)
        {
            {
                TSmartLock<CLock> loSmartLock(loLock.coLock);
                loLock.ciCount++;
                apl_sleep(10*APL_TIME_MSEC);
                loLock.ciCount++;
                apl_sleep(10*APL_TIME_MSEC);
                loLock.ciCount++;
                CPPUNIT_ASSERT_EQUAL_MESSAGE("testSmartLock failed.", (apl_int_t)3, loLock.ciCount);
            }        
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        WAIT_EXP(loLock.ciCount == 1);

        START_THREAD_BODY(mybody2, stLock, loLock)
        {
            {
                TSmartLock<CLock> loSmartLock(loLock.coLock);
                loLock.ciCount--;
                CPPUNIT_ASSERT_EQUAL_MESSAGE("testSmartLock failed.", (apl_int_t)2, loLock.ciCount);
            }        
        }
        END_THREAD_BODY(mybody2)
        RUN_THREAD_BODY(mybody2)

        apl_sleep(500*APL_TIME_MSEC);

        //end environment
    }
};

class CTestAclSmartRLock:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclSmartRLock);
    CPPUNIT_TEST(testSmartRLock);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testSmartRLock(void)
    {
        PrintTitle("CTestAclSmartRLock", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        char const* lpcFile = "tmp.txt";
        apl_unlink(lpcFile);
        char lsBuf[20];

        CFileRWLock loFileRWLock(lpcFile);

        //case
        apl_pid_t liChildID = apl_fork();
        if(liChildID < 0)
        {
            CPPUNIT_FAIL("Fork failed");
        }
        else if(liChildID == 0)
        {
            {
                TSmartWLock<CFileRWLock> loLock(loFileRWLock);
                apl_handle_t liFd = apl_open(lpcFile, APL_O_RDWR|APL_O_APPEND, 0666);
                apl_write(liFd, "abcd", 4);
                apl_close(liFd);
            }

            {   
                TSmartRLock<CFileRWLock> loLock(loFileRWLock);
                apl_handle_t liFd = apl_open(lpcFile, APL_O_RDONLY, 0666);

                memset(lsBuf, 0, sizeof(lsBuf));
                apl_read(liFd, lsBuf, sizeof(lsBuf));
                CPPUNIT_ASSERT_MESSAGE("TestFileSmartRWLockParent failed.", 
                        apl_strcmp(lsBuf, "abcd")==0);
                apl_close(liFd);
                apl_sleep(10*APL_TIME_MSEC);
            }
            exit(0);
        }
        else
        {
            apl_sleep(10*APL_TIME_MSEC);

            {   
                TSmartRLock<CFileRWLock> loLock(loFileRWLock);
                apl_handle_t liFd = apl_open(lpcFile, APL_O_RDONLY, 0666);

                memset(lsBuf, 0, sizeof(lsBuf));
                apl_read(liFd, lsBuf, sizeof(lsBuf));
                CPPUNIT_ASSERT_MESSAGE("TestFileSmartRWLockParent failed.", 
                        apl_strcmp(lsBuf, "abcd")==0);
                apl_close(liFd);
            }

            if(apl_waitpid(liChildID, NULL, 0) != liChildID)
            {
                CPPUNIT_FAIL("Wait pid failed.");
            }

            apl_unlink(lpcFile);
        }

        //end environment
    }
};

class CTestAclSmartWLock:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclSmartWLock);
    CPPUNIT_TEST(testSmartWLock);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testSmartWLock(void)
    {
        PrintTitle("CTestAclSmartWLock", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        char const* lpcFile = "tmp.txt";
        apl_unlink(lpcFile);

        CFileRWLock loFileRWLock(lpcFile);
        
        //case
        apl_pid_t liChildID = apl_fork();
        if(liChildID < 0)
        {
            CPPUNIT_FAIL("Fork failed");
        }
        else if(liChildID == 0)
        {
            TSmartWLock<CFileRWLock> loLock(loFileRWLock);
            apl_handle_t liFd = apl_open(lpcFile, APL_O_RDWR|APL_O_APPEND, 0666);
            apl_write(liFd, "abcd", 4);
            apl_sleep(APL_TIME_SEC);
            apl_write(liFd, "1234", 4);
            apl_close(liFd);
            exit(0);
        }
        else
        {
            apl_sleep(10*APL_TIME_MSEC);

            {
                TSmartWLock<CFileRWLock> loLock(loFileRWLock);
                apl_handle_t liFd = apl_open(lpcFile, APL_O_RDWR|APL_O_APPEND, 0666);
                apl_write(liFd, "xyz", 4);
            }

            {   
                TSmartRLock<CFileRWLock> loLock(loFileRWLock);
                apl_handle_t liFd = apl_open(lpcFile, APL_O_RDONLY, 0666);

                char const *lpcContent = "abcd1234xyz";
                char lsBuf[20];
                memset(lsBuf, 0, sizeof(lsBuf));
                apl_read(liFd, lsBuf, sizeof(lsBuf));
                CPPUNIT_ASSERT_MESSAGE("testSmartWLock failed.", 
                        apl_strcmp(lsBuf, lpcContent)==0);
                apl_close(liFd);
            }

            if(apl_waitpid(liChildID, NULL, 0) != liChildID)
            {
                CPPUNIT_FAIL("Wait pid failed.");
            }

            apl_unlink(lpcFile);
        }

        //end environment
    }
};

class CTestAclSemaphore:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclSemaphore);
    CPPUNIT_TEST(testGetValue);
    CPPUNIT_TEST(testWaitPost);
    CPPUNIT_TEST(testWaitPost1);
    CPPUNIT_TEST(testWait);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testGetValue(void)
    {
        PrintTitle("CTestAclSemaphore", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        CSemaphore loSema;

        //case
        apl_int_t liRet = loSema.GetValue();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue failed.", (apl_int_t)0, liRet);

        loSema.Post();
        liRet = loSema.GetValue();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue failed.", (apl_int_t)1, liRet);
        loSema.Post();
        liRet = loSema.GetValue();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue failed.", (apl_int_t)2, liRet);

        loSema.Wait();
        liRet = loSema.GetValue();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue failed.", (apl_int_t)1, liRet);

        //end environment
    }

    void testWaitPost(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        struct stSema
        {
            stSema(apl_int_t aiCount) : ciCount(aiCount) {}
            apl_int_t ciCount;
            CSemaphore coSema;
        };

        stSema loSema(2);
        apl_int_t liVal = loSema.coSema.GetValue();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue failed.", (apl_int_t)0, liVal);

        //case
        START_THREAD_BODY(mybody, stSema, loSema)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testWaitPost failed.", (apl_int_t)2, loSema.ciCount);
            loSema.coSema.Wait();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wait failed.", (apl_int_t)3, loSema.ciCount);
            loSema.ciCount = 7;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(10*APL_TIME_MSEC);

        START_THREAD_BODY(mybody2, stSema, loSema)
        {
            loSema.ciCount = 3;
            loSema.coSema.Post();

            apl_sleep(100*APL_TIME_MSEC);

            CPPUNIT_ASSERT_EQUAL_MESSAGE("Post failed.", (apl_int_t)7, loSema.ciCount);
        }
        END_THREAD_BODY(mybody2)
        RUN_THREAD_BODY(mybody2)

        apl_sleep(500*APL_TIME_MSEC);

        //end environment
    }
    
    void testWaitPost1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        struct stSema
        {
            stSema(apl_int_t aiCount) : ciCount(aiCount) {}
            apl_int_t ciCount;
            CSemaphore coSema;
        };

        stSema loSema(2);
        apl_int_t liVal = loSema.coSema.GetValue();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue failed.", (apl_int_t)0, liVal);

        //case
        START_THREAD_BODY(mybody, stSema, loSema)
        {
            acl::CTimestamp loExpired(acl::CTimestamp::PRC_NSEC);

            loExpired += 2;

            CPPUNIT_ASSERT_EQUAL_MESSAGE("testWaitPost failed.", (apl_int_t)2, loSema.ciCount);
            loSema.coSema.Wait(loExpired);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wait failed.", (apl_int_t)3, loSema.ciCount);
            loSema.ciCount = 7;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(10*APL_TIME_MSEC);

        START_THREAD_BODY(mybody2, stSema, loSema)
        {
            loSema.ciCount = 3;
            loSema.coSema.Post();

            apl_sleep(100*APL_TIME_MSEC);

            CPPUNIT_ASSERT_EQUAL_MESSAGE("Post failed.", (apl_int_t)7, loSema.ciCount);
        }
        END_THREAD_BODY(mybody2)
        RUN_THREAD_BODY(mybody2)

        apl_sleep(500*APL_TIME_MSEC);

        //end environment
    }
    
    void testWait(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        acl::CTimestamp loEndTime;
        acl::CTimeValue loTimedout(2);
        CSemaphore loSema;
        
        {
            acl::CTimestamp loStartTime(acl::CTimestamp::PRC_NSEC);
            
            loSema.Wait(loTimedout);
        
            acl::CTimestamp loEndTime(acl::CTimestamp::PRC_NSEC);
        
            ASSERT_MESSAGE(loEndTime.Sec() - loStartTime.Sec() >= 2);
        }

        {
            acl::CTimestamp loStartTime(acl::CTimestamp::PRC_NSEC);
            
            loSema.Wait(loStartTime + loTimedout);
        
            acl::CTimestamp loEndTime(acl::CTimestamp::PRC_NSEC);
        
            ASSERT_MESSAGE(loEndTime.Sec() - loStartTime.Sec() >= 2);
        }
        
        //end environment
    }
};

class CTestAclCondition:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclCondition);
    CPPUNIT_TEST(testWait);
    CPPUNIT_TEST(testWait1);
    CPPUNIT_TEST(testWait2);
    CPPUNIT_TEST(testSignal);
    CPPUNIT_TEST(testBroadcast);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testWait(void)
    {
        PrintTitle("CTestAclCondition", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        struct stCond
        {
            stCond(apl_int_t aiFlag) : ciFlag(aiFlag) {}
            apl_int_t  ciFlag;
            CLock coLock;
            CCondition coCond;
        };

        stCond loCond(0);

        //case
        START_THREAD_BODY(mybody, stCond, loCond)
        {
            loCond.coLock.Lock();
            loCond.ciFlag = 1;
            apl_int_t liRet = loCond.coCond.Wait(loCond.coLock);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wait failed.", (apl_int_t)0, liRet);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Signal failed.", (apl_int_t)2, loCond.ciFlag);
            loCond.coLock.Unlock();

            WAIT_EXP(loCond.ciFlag == 2);

            loCond.coLock.Lock();
            CTimeValue loTimeValue(1);
            liRet = loCond.coCond.Wait(loCond.coLock, loTimeValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wait timeout failed.", (apl_int_t)-1, liRet);
            loCond.ciFlag = 3;
            loCond.coLock.Unlock();
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        START_THREAD_BODY(mybody2, stCond, loCond)
        {
            WAIT_EXP(loCond.ciFlag == 1);
            loCond.coLock.Lock();
            loCond.ciFlag = 2;
            loCond.coCond.Signal();
            loCond.coLock.Unlock();
        }
        END_THREAD_BODY(mybody2)
        RUN_THREAD_BODY(mybody2)

        WAIT_EXP(loCond.ciFlag == 3);

        //end environment
    }

    void testWait1(void)
    {
        PrintTitle("CTestAclCondition", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        CLock loLock;
        CCondition loCond;

        //case
        loLock.Lock();
        acl::CTimestamp loStartTime(acl::CTimestamp::PRC_NSEC);
        acl::CTimeValue loTimedout(2);

        apl_int_t liRet = loCond.Wait(loLock, (loStartTime + loTimedout) );
        ASSERT_MESSAGE(liRet == -1);

        loLock.Unlock();

        acl::CTimestamp loEndTime(acl::CTimestamp::PRC_NSEC);

        ASSERT_MESSAGE(loEndTime.Sec() - loStartTime.Sec() >= 2);

        //end environment
    }
    
    void testWait2(void)
    {
        PrintTitle("CTestAclCondition", '#', 20);
        PRINT_TITLE_2(__func__);

        //start case environment
        struct stCond
        {
            stCond(apl_int_t aiFlag) : ciFlag(aiFlag) {}
            apl_int_t  ciFlag;
            CLock coLock;
            CCondition coCond;
        };

        stCond loCond(0);

        //case
        START_THREAD_BODY(mybody, stCond, loCond)
        {
            loCond.coLock.Lock();
            loCond.ciFlag = 1;
            
            acl::CTimestamp loExpired(acl::CTimestamp::PRC_NSEC);
            acl::CTimeValue loTimedout(100);
            
            apl_int_t liRet = loCond.coCond.Wait(loCond.coLock, (loExpired + loTimedout) );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wait failed.", (apl_int_t)0, liRet);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Signal failed.", (apl_int_t)2, loCond.ciFlag);
            loCond.coLock.Unlock();

            WAIT_EXP(loCond.ciFlag == 2);

            loCond.coLock.Lock();
            CTimeValue loTimeValue(1);
            liRet = loCond.coCond.Wait(loCond.coLock, loTimeValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wait timeout failed.", (apl_int_t)-1, liRet);
            loCond.ciFlag = 3;
            loCond.coLock.Unlock();
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        START_THREAD_BODY(mybody2, stCond, loCond)
        {
            WAIT_EXP(loCond.ciFlag == 1);
            loCond.coLock.Lock();
            loCond.ciFlag = 2;
            loCond.coCond.Signal();
            loCond.coLock.Unlock();
        }
        END_THREAD_BODY(mybody2)
        RUN_THREAD_BODY(mybody2)

        WAIT_EXP(loCond.ciFlag == 3);

        //end environment
    }

    void testSignal(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        struct stCond
        {
            stCond(apl_int_t aiFlag) : ciFlag(aiFlag) {}
            apl_int_t  ciFlag;
            CLock coLock;
            CCondition coCond;
        };

        stCond loCond(0);

        //case
        START_THREAD_BODY(mybody, stCond, loCond)
        {
            loCond.coLock.Lock();
            loCond.ciFlag = 1;
            apl_int_t liRet = loCond.coCond.Wait(loCond.coLock);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wait failed.", (apl_int_t)0, liRet);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Signal failed.", (apl_int_t)2, loCond.ciFlag);
            loCond.coLock.Unlock();
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        WAIT_EXP(loCond.ciFlag == 1);

        START_THREAD_BODY(mybody2, stCond, loCond)
        {
            loCond.coLock.Lock();
            loCond.ciFlag = 2;
            loCond.coCond.Signal();
            loCond.coLock.Unlock();
        }
        END_THREAD_BODY(mybody2)
        RUN_THREAD_BODY(mybody2)

        apl_sleep(100*APL_TIME_MSEC);

        //end environment
    }

    void testBroadcast(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        struct stCond
        {
            stCond(apl_int_t aiFlag) : ciFlag(aiFlag) {}
            apl_int_t  ciFlag;
            CLock coLock;
            CCondition coCond;
        };

        stCond loCond(0);

        //case
        START_THREAD_BODY(mybody, stCond, loCond)
        {
            loCond.coLock.Lock();
            loCond.ciFlag = 1;
            apl_int_t liRet = loCond.coCond.Wait(loCond.coLock);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wait failed.", (apl_int_t)0, liRet);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Broadcast failed.", (apl_int_t)2, loCond.ciFlag);
            loCond.coLock.Unlock();
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        START_THREAD_BODY(mybody2, stCond, loCond)
        {
            WAIT_EXP(loCond.ciFlag == 1);
            loCond.coLock.Lock();
            loCond.ciFlag = 2;
            loCond.coCond.Broadcast();
            loCond.coLock.Unlock();
        }
        END_THREAD_BODY(mybody2)
        RUN_THREAD_BODY(mybody2)

        apl_sleep(100*APL_TIME_MSEC);
        //end environment
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclNullLock);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclLock);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclSpinLock);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclFileLock);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclNullRWLock);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclRWLock);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclFileRWLock);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclSmartLock);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclSmartRLock);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclSmartWLock);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclSemaphore);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclCondition);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

