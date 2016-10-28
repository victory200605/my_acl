#include "Test.h"
#include "acl/Thread.h"
#include "acl/ThreadManager.h"
#include "acl/ThreadPool.h"
#include "acl/Synch.h"
#include "acl/Timestamp.h"
#include "acl/ThreadTask.h"

using namespace acl;

void* TrdFunc(void* apvParam)
{
    IThreadBody* lpoBody = (IThreadBody*)(apvParam);

    lpoBody->DoIt();

    //delete lpoBody;

    return NULL;
}

CLock goMtxLk;

#define START_THREAD_BODY_EX( name, paramtype, param ) \
    IThreadBody* name = NULL; \
    { \
        paramtype& _param = param; \
        struct _threadbody : public IThreadBody \
        { \
            paramtype& param; \
            _threadbody(paramtype& apvParam) \
                : param(apvParam) \
            { \
            } \
            virtual void DoIt() \
            { \
                goMtxLk.Lock();

#define END_THREAD_BODY_EX( name ) \
                goMtxLk.Unlock(); \
            } \
         }; \
         name = new _threadbody(_param); \
     }


class CTestAclThreadManager : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclThreadManager);
    CPPUNIT_TEST( testSpawn );
    CPPUNIT_TEST( testScope );
//    CPPUNIT_TEST( testRunState );
//    CPPUNIT_TEST( testWait );
//    CPPUNIT_TEST( testWaitGrp );
//    CPPUNIT_TEST( testWaitAll );
//    CPPUNIT_TEST( testCount );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) 
    {
    }
    void tearDown(void) {}
    void testSpawn(void)
    {
        PrintTitle("ThreadManager", '#', 20);
        PRINT_TITLE_2(__func__);
        apl_int_t    liRetcode = 0;
        apl_thread_t ltTrdid;
        //start Environment
        CThreadManager loTrdManager;
        apl_int_t    liN= 0;
        //case
        START_THREAD_BODY_EX(mybody, apl_int_t, liN)
            liN++;
        END_THREAD_BODY_EX(mybody);
        
        //case
        liN = 0;
        liRetcode= loTrdManager.Spawn(&ltTrdid, TrdFunc, mybody, APL_THREAD_KERNEL);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Spawn fail", (apl_int_t)0, liRetcode);
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Spawn fail", (apl_int_t)1, liN);

        liN = 0;
        liRetcode= loTrdManager.Spawn(&ltTrdid, TrdFunc, mybody, APL_THREAD_KERNEL, 2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Spawn fail", (apl_int_t)0, liRetcode);
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Spawn fail", (apl_int_t)1, liN);
        
        liN = 0;
        liRetcode= loTrdManager.Spawn(&ltTrdid, TrdFunc, mybody, APL_THREAD_KERNEL, 3, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Spawn fail", (apl_int_t)0, liRetcode);
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Spawn fail", (apl_int_t)1, liN);
        
        liN = 0;
        liRetcode= loTrdManager.Spawn(TrdFunc, mybody, APL_THREAD_KERNEL);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Spawn fail", (apl_int_t)0, liRetcode);
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Spawn fail", (apl_int_t)1, liN);
        
        
        liN = 0;
        liRetcode= loTrdManager.Spawn(TrdFunc, mybody, APL_THREAD_KERNEL, 2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Spawn fail", (apl_int_t)0, liRetcode);
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Spawn fail", (apl_int_t)1, liN);
        
        liN = 0;
        liRetcode= loTrdManager.Spawn(TrdFunc, mybody, APL_THREAD_KERNEL, 3, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Spawn fail", (apl_int_t)0, liRetcode);
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Spawn fail", (apl_int_t)1, liN);
        
        //end Environment
        loTrdManager.WaitAll();
    }
    void testSpawnN(void)
    {
        PRINT_TITLE_2(__func__);
        apl_int_t    lir = 0;
        apl_thread_t latTrdid[5];
        //start Environment
        CThreadManager loTrdManager;
        apl_int_t    liN= 0;
        //case
        START_THREAD_BODY_EX(mybody, apl_int_t, liN)
            liN +=2;
        END_THREAD_BODY_EX(mybody);

        lir= loTrdManager.SpawnN(latTrdid, 5, TrdFunc, mybody, APL_THREAD_KERNEL);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SpawnN fail", (apl_int_t)5, lir);
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SpawnN fail", (apl_int_t)10, liN);

        liN= 0;
        lir= 0;
        lir= loTrdManager.SpawnN(latTrdid, 5, TrdFunc, mybody, APL_THREAD_KERNEL, 2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SpawnN fail", (apl_int_t)5, lir);
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SpawnN fail", (apl_int_t)10, liN);

        liN= 0;
        lir= 0;
        lir= loTrdManager.SpawnN(latTrdid, 5, TrdFunc, mybody, APL_THREAD_KERNEL, 3, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SpawnN fail", (apl_int_t)5, lir);
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SpawnN fail", (apl_int_t)10, liN);
        
        liN= 0;
        lir= 0;
        lir= loTrdManager.SpawnN(5, TrdFunc, mybody, APL_THREAD_KERNEL);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SpawnN fail", (apl_int_t)5, lir);
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SpawnN fail", (apl_int_t)10, liN);

        liN= 0;
        lir= 0;
        lir= loTrdManager.SpawnN(5, TrdFunc, mybody, APL_THREAD_KERNEL, 2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SpawnN fail", (apl_int_t)5, lir);
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SpawnN fail", (apl_int_t)10, liN);

        liN= 0;
        lir= 0;
        lir= loTrdManager.SpawnN(5, TrdFunc, mybody, APL_THREAD_KERNEL, 3, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SpawnN fail", (apl_int_t)5, lir);
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SpawnN fail", (apl_int_t)10, liN);
       
        //end environment
        loTrdManager.WaitAll();
    }
    void testScope(void)
    {
        PRINT_TITLE_2(__func__);
        apl_thread_t ltThrd1;
        apl_thread_t ltThrd2;
        
        //start Environment
        CThreadManager loTrdManager;
        apl_int_t liN= 0; 
        START_THREAD_BODY_EX(mybody, apl_int_t, liN)
        END_THREAD_BODY_EX(mybody);
       
        //case 
        loTrdManager.Spawn(&ltThrd1, TrdFunc, mybody, APL_THREAD_KERNEL);
        CPPUNIT_ASSERT_EQUAL_MESSAGE( 
            "Scope fail", (apl_int_t)APL_THREAD_KERNEL, loTrdManager.GetScope(ltThrd1) );
        loTrdManager.WaitAll();

        loTrdManager.Spawn(&ltThrd2, TrdFunc, mybody, APL_THREAD_USER);
        CPPUNIT_ASSERT_MESSAGE(
            "Scope fail", (apl_int_t)APL_THREAD_USER == loTrdManager.GetScope(ltThrd2) );
        loTrdManager.WaitAll();
        
        //end environment
    }

    void testRunState(void)
    {
        PRINT_TITLE_2(__func__);
        apl_thread_t ltTrdid;
           
        //start environment
        CThreadManager loTrdManager;
        apl_int_t      liN = 0;
        
        START_THREAD_BODY_EX(mybody, apl_int_t, liN)
            liN = 1;
            WAIT_EXP(liN == 2);
        END_THREAD_BODY_EX(mybody);
        
        loTrdManager.Spawn(&ltTrdid, TrdFunc, mybody, APL_THREAD_KERNEL);
            
        //case
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "runstate fail", (apl_int_t)CThreadManager::THR_RUNNING, loTrdManager.GetRunState(ltTrdid) );
        
        liN = 2;
        apl_sleep(100*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "runstate fail", (apl_int_t)CThreadManager::THR_TERMINATED, loTrdManager.GetRunState(ltTrdid) );

        //end environment
        loTrdManager.WaitAll();
    }

    void testWait(void)
    {
        PRINT_TITLE_2(__func__);
        apl_thread_t ltThrd1;
        apl_thread_t ltThrd2;
        apl_int_t    liRetcode = 0;
        
        //start environment
        CThreadManager loTrdManager;
        apl_int_t      liN = 0;
        
        START_THREAD_BODY_EX(mybody1, apl_int_t, liN)
            WAIT_EXP(liN == 2);
        END_THREAD_BODY_EX(mybody1);
        
        START_THREAD_BODY_EX(mybody2, apl_int_t, liN)
            //NULL;
        END_THREAD_BODY_EX(mybody2);
        
        loTrdManager.Spawn(&ltThrd1, TrdFunc, mybody1, APL_THREAD_KERNEL);
        loTrdManager.Spawn(&ltThrd2, TrdFunc, mybody2, APL_THREAD_KERNEL);
        
        //case
        CTimeValue loTimeValue(1);
        liRetcode = loTrdManager.Wait(ltThrd1, APL_NULL, loTimeValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wait timeout fail", (apl_int_t)-1, liRetcode);
        
        liN = 2;
        apl_sleep(100*APL_TIME_MSEC);
        liRetcode = loTrdManager.Wait(ltThrd1, APL_NULL, loTimeValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wait timeout fail", (apl_int_t)0, liRetcode);

        liRetcode = loTrdManager.Wait(ltThrd2, APL_NULL);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wait timeout fail", (apl_int_t)0, liRetcode);

        //end environment
        loTrdManager.WaitAll();
    }
    void testWaitGrp(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadManager loTrdManager;
        apl_thread_t latTrdid1[5];
        apl_thread_t latTrdid2[5];
        apl_int_t liR = 0;
        apl_int_t liN = 0;

        START_THREAD_BODY_EX(mybody1, apl_int_t, liN)
            WAIT_EXP(liN == 2);
        END_THREAD_BODY_EX(mybody1);

        START_THREAD_BODY_EX(mybody2, apl_int_t, liN)
            //NULL;
        END_THREAD_BODY_EX(mybody2);

        loTrdManager.SpawnN(latTrdid1, 5, TrdFunc, mybody1, APL_THREAD_KERNEL, 3);
        loTrdManager.SpawnN(latTrdid2, 5, TrdFunc, mybody2, APL_THREAD_KERNEL, 2);

        //case
        CTimeValue loTimeValue(1);
        liR = loTrdManager.WaitGrp(3, loTimeValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wait timeout fail", (apl_int_t)-1, liR);

        liN = 2;
        apl_sleep(100*APL_TIME_MSEC);
        liR = loTrdManager.WaitGrp(3, loTimeValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wait timeout fail", (apl_int_t)0, liR);

        liR = loTrdManager.WaitGrp(2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wait timeout fail", (apl_int_t)0, liR);

        //end environment
        loTrdManager.WaitAll();
    }
    void testCount(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadManager loTrdManager;
        apl_int_t lir= 0;
        apl_int_t liN= 0;
        CTimeValue lotm;
        apl_thread_t ltTrdid;
        lotm.Msec(10);

        START_THREAD_BODY_EX(mybody1, apl_int_t, liN)
            WAIT_EXP(liN == 2);
        END_THREAD_BODY_EX(mybody1);

        //case
        lir= loTrdManager.SpawnN(5, TrdFunc, mybody1, APL_THREAD_KERNEL, 2);
        apl_sleep(10*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getcount fail", (apl_size_t)5, loTrdManager.GetCount() );
        liN= 2;
        loTrdManager.WaitGrp(2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getcount fail", (apl_size_t)0, loTrdManager.GetCount() );
       
        liN= 0;
        lir= loTrdManager.Spawn(&ltTrdid, TrdFunc, mybody1, APL_THREAD_KERNEL);
        apl_sleep(APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getcount fail", (apl_size_t)1, loTrdManager.GetCount() );
        liN= 2;
        loTrdManager.Wait(ltTrdid, APL_NULL);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getcount fail", (apl_size_t)0, loTrdManager.GetCount() );

        //end environment
        loTrdManager.WaitAll();
    }
    void testWaitAll(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadManager loTrdManager;
        apl_int_t lir= 0;
        apl_int_t liN= 0;
        CTimeValue lotm;
        apl_thread_t ltTrdid;
        lotm.Msec(10);

        START_THREAD_BODY_EX(mybody1, apl_int_t, liN)
            WAIT_EXP(liN == 2);
        END_THREAD_BODY_EX(mybody1);

        //case
        lir= loTrdManager.SpawnN(5, TrdFunc, mybody1, APL_THREAD_KERNEL, 2);
        apl_sleep(10*APL_TIME_MSEC);
        lir= loTrdManager.WaitAll(lotm);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("waitall fail", (apl_int_t)-1, lir);
        liN= 2;
        
        lir= loTrdManager.Spawn(&ltTrdid, TrdFunc, mybody1, APL_THREAD_KERNEL);
        apl_sleep(APL_TIME_MSEC);
        lir= loTrdManager.WaitAll();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("waitall fail", (apl_int_t)0, lir);
         
        //end environment
    }
};

class CTestRunnable
{
public:
    virtual apl_int_t operator()(void)
    {
        goMtxLk.Lock();
        goMtxLk.Unlock();
        apl_sleep(APL_TIME_SEC);
        return 0;
    }
};

class CTestAclThreadPool : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclThreadPool);
    CPPUNIT_TEST( testInitialize );
    CPPUNIT_TEST( testScope );
    CPPUNIT_TEST( testStackSize );
    CPPUNIT_TEST( testPoolSize );
    CPPUNIT_TEST( testIdleCount );
    CPPUNIT_TEST( testBusy );
    CPPUNIT_TEST( testRun );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void)
    {
    }
    void tearDown(void) {}
    void testInitialize(void)
    {
        PrintTitle("ThreadPool", '#', 20);
        PRINT_TITLE_2(__func__);
        //start environment 
        CThreadPool loTrdPool1;
        CThreadPool loTrdPool2;
        apl_int_t  lir= 0;
        //case
        lir= loTrdPool1.Initialize(5, APL_THREAD_KERNEL, 0); 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("trdPool fail", (apl_int_t)0, lir);
        lir= loTrdPool2.Initialize(5, APL_THREAD_USER, 0); 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("trdPool fail", (apl_int_t)0, lir);
   
        //end environment
    }
    void testScope(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadPool loTrdPool1;
        CThreadPool loTrdPool2;
        loTrdPool1.Initialize(5, APL_THREAD_KERNEL, 0);
        loTrdPool2.Initialize(5, APL_THREAD_USER, 0);
        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE( 
            "Scope fail", (apl_int_t)APL_THREAD_KERNEL, loTrdPool1.GetScope() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( 
            "Scope fail", (apl_int_t)APL_THREAD_USER, loTrdPool2.GetScope() );
        
         //end environment
    }
    void testStackSize(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadPool loTrdPool1;
        CThreadPool loTrdPool2;
        loTrdPool1.Initialize(5, APL_THREAD_KERNEL, APL_THREAD_STACK_MIN+1024);
        loTrdPool2.Initialize(5, APL_THREAD_USER, APL_THREAD_STACK_MIN+1024);
        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE( 
            "StackSize fail", (apl_size_t)(APL_THREAD_STACK_MIN+1024), loTrdPool1.GetStackSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "StackSize fail", (apl_size_t)(APL_THREAD_STACK_MIN+1024), loTrdPool2.GetStackSize() );
        
         //end environment
    }
    void testPoolSize(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadPool loTrdPool1;
        CThreadPool loTrdPool2;
        loTrdPool1.Initialize(5, APL_THREAD_KERNEL, 0);
        loTrdPool2.Initialize(8, APL_THREAD_USER, 0);
        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE( 
            "PoolSize fail", (apl_size_t)5, loTrdPool1.GetPoolSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "PoolSize fail", (apl_size_t)8, loTrdPool2.GetPoolSize() );
        
         //end environment
    }
    void testIdleCount(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadPool loTrdPool1;
        apl_size_t luN= 0;
        CTimeValue lotm;
        lotm.Msec(5);
        loTrdPool1.Initialize(5, APL_THREAD_KERNEL, 0);
        //case 
        apl_sleep(10*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getidlecount fail", (apl_size_t)5, loTrdPool1.GetIdleCount() );
        for(luN= 0; luN<3; ++luN)
        {
            loTrdPool1.Run( CTestRunnable() );
            apl_sleep(APL_TIME_MSEC);
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getidlecount fail", (apl_size_t)2, loTrdPool1.GetIdleCount() );

        //end environment
        loTrdPool1.Close();
    }
    void testBusy(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadPool loTrdPool1;
        apl_size_t luN= 0;
        CTimestamp lotmVal;
        CTimestamp lotmVal1;
        CTimestamp lotmVal2;
        lotmVal1.Update(CTimestamp::PRC_SEC);
        loTrdPool1.Initialize(5, APL_THREAD_KERNEL, 0);
        //case
        apl_sleep(10*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("busyCount fail", (apl_size_t)0, loTrdPool1.GetBusyCount() );
        for(luN= 0; luN<5; ++luN)
        {
            loTrdPool1.Run( CTestRunnable() );
            apl_sleep(10*APL_TIME_MSEC);
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("busycount fail", (apl_size_t)5, loTrdPool1.GetBusyCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("topbusycount fail", (apl_size_t)5, loTrdPool1.GetTopBusyCount() );
        apl_sleep(2*APL_TIME_SEC);

        for(luN= 0; luN<3; ++luN)
        {
            loTrdPool1.Run( CTestRunnable() );
            apl_sleep(100*APL_TIME_MSEC);
        } 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("busycount fail", (apl_size_t)3, loTrdPool1.GetBusyCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("topbusycount fail", (apl_size_t)5, loTrdPool1.GetTopBusyCount() );

        lotmVal = loTrdPool1.GetTopBusyTime();
        lotmVal2.Update(CTimestamp::PRC_SEC);
        if ( (lotmVal2- lotmVal).Sec() < 0
             || (lotmVal- lotmVal1).Sec() < 0 )
        {
            CPPUNIT_FAIL("GetTopBusyTime fail");
        }

        //end environment
        loTrdPool1.Close();
    }
    void testRun(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadPool loTrdPool1;
        apl_int_t  lir= 0;
        apl_size_t luN= 0;
        CTimeValue lotm;

        loTrdPool1.Initialize(5, APL_THREAD_KERNEL, 0);
        //case
        apl_sleep(APL_TIME_SEC);
        for(luN= 0; luN<5; ++luN)
        {
            lir= loTrdPool1.Run( CTestRunnable() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Run func fail", (apl_int_t)0, lir);
            apl_sleep(10*APL_TIME_MSEC);
        }
        apl_sleep(10*APL_TIME_MSEC);

        lotm.Msec(5);
        lir= loTrdPool1.Run(CTestRunnable(), lotm);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Run func fail", (apl_int_t)-1, lir);

        apl_sleep(2*APL_TIME_SEC);
        lir= loTrdPool1.Run(CTestRunnable(), lotm);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Run func fail", (apl_int_t)0, lir);
        
        loTrdPool1.Close();
        //end environment
    }
};

class CTestAclThreadTask : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclThreadTask);
    CPPUNIT_TEST( testInitialize );
    CPPUNIT_TEST( testScope );
    CPPUNIT_TEST( testStackSize );
    CPPUNIT_TEST( testThreadCount );
    CPPUNIT_TEST( testCapacity );
    CPPUNIT_TEST( testIdleCount );
    CPPUNIT_TEST( testBusy );
    CPPUNIT_TEST( testRun );
    CPPUNIT_TEST( testWait );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void)
    {
    }
    void tearDown(void) {}
    void testInitialize(void)
    {
        PrintTitle("ThreadTask", '#', 20);
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadTask loTrdTsk1;
        CThreadTask loTrdTsk2;
        apl_int_t  lir= 0;
        //case
        lir= loTrdTsk1.Initialize(2, 5, APL_THREAD_KERNEL, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("trdTsk fail", (apl_int_t)0, lir);
        lir= loTrdTsk2.Initialize(6, 5, APL_THREAD_USER, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("trdTsk fail", (apl_int_t)0, lir);

        //end environment
    }
    void testScope(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadTask loTrdTsk1;
        CThreadTask loTrdTsk2;
        loTrdTsk1.Initialize(2, 5, APL_THREAD_KERNEL, 0);
        loTrdTsk2.Initialize(6, 5, APL_THREAD_USER, 0);
        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Scope fail", (apl_int_t)APL_THREAD_KERNEL, loTrdTsk1.GetScope() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Scope fail", (apl_int_t)APL_THREAD_USER, loTrdTsk2.GetScope() );

         //end environment
    }
    void testStackSize(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadTask loTrdTsk1;
        CThreadTask loTrdTsk2;
        loTrdTsk1.Initialize(2, 5, APL_THREAD_KERNEL, APL_THREAD_STACK_MIN+1024);
        loTrdTsk2.Initialize(3, 5, APL_THREAD_USER, APL_THREAD_STACK_MIN+1024);
        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "StackSize fail", (apl_size_t)(APL_THREAD_STACK_MIN+1024), loTrdTsk1.GetStackSize() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "StackSize fail", (apl_size_t)(APL_THREAD_STACK_MIN+1024), loTrdTsk2.GetStackSize() );

         //end environment
    }
    void testThreadCount(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadTask loTrdTsk1;
        CThreadTask loTrdTsk2;
        loTrdTsk1.Initialize(2, 5, APL_THREAD_KERNEL, 0);
        loTrdTsk2.Initialize(8, 8, APL_THREAD_USER, 0);
        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "TrdCount fail", (apl_size_t)5, loTrdTsk1.GetThreadCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "TrdCount fail", (apl_size_t)8, loTrdTsk2.GetThreadCount() );

         //end environment
    }
    void testCapacity(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadTask loTrdTsk1;
        CThreadTask loTrdTsk2;
        loTrdTsk1.Initialize(2, 5, APL_THREAD_KERNEL, 0);
        loTrdTsk2.Initialize(8, 8, APL_THREAD_USER, 0);
        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Capacity fail", (apl_size_t)2, loTrdTsk1.GetCapacity() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Capacity fail", (apl_size_t)8, loTrdTsk2.GetCapacity() );

         //end environment
    }
    void testIdleCount(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadTask loTrdTsk1;
        apl_size_t luN= 0;
        
        loTrdTsk1.Initialize(3, 5, APL_THREAD_KERNEL, 0);
        //case
        apl_sleep(10*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getidlecount fail", (apl_size_t)5, loTrdTsk1.GetIdleCount() );
        for(luN= 0; luN<3; ++luN)
        {
            loTrdTsk1.Run(CTestRunnable() );
            apl_sleep(10*APL_TIME_MSEC);
        }
        luN= loTrdTsk1.GetIdleCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getidlecount fail", (apl_size_t)2, luN );
        apl_sleep(APL_TIME_SEC);
        luN= loTrdTsk1.GetIdleCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getidlecount fail", (apl_size_t)5, luN );
        //end environment
        loTrdTsk1.Close();
    }

    void testBusy(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadTask loTrdTsk1;
        apl_size_t luN= 0;
        CTimestamp lotmVal;
        CTimestamp lotmVal1;
        CTimestamp lotmVal2;
        lotmVal1.Update(CTimestamp::PRC_SEC);
        loTrdTsk1.Initialize(3, 5, APL_THREAD_KERNEL, 0);
        //case
        apl_sleep(10*APL_TIME_MSEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("busyCount fail", (apl_size_t)0, loTrdTsk1.GetBusyCount() );
        for(luN= 0; luN<5; ++luN)
        {
            loTrdTsk1.Run(CTestRunnable());
            apl_sleep(10*APL_TIME_MSEC);
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("busycount fail", (apl_size_t)5, loTrdTsk1.GetBusyCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("topbusycount fail", (apl_size_t)5, loTrdTsk1.GetTopBusyCount() );
        apl_sleep(APL_TIME_SEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("busycount fail", (apl_size_t)0, loTrdTsk1.GetBusyCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("topbusycount fail", (apl_size_t)5, loTrdTsk1.GetTopBusyCount() );

        for(luN= 0; luN<3; ++luN)
        {
            loTrdTsk1.Run(CTestRunnable());
            apl_sleep(10*APL_TIME_MSEC);
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("busycount fail", (apl_size_t)3, loTrdTsk1.GetBusyCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("topbusycount fail", (apl_size_t)5, loTrdTsk1.GetTopBusyCount() );
        apl_sleep(APL_TIME_SEC);
        lotmVal = loTrdTsk1.GetTopBusyTime();
        lotmVal2.Update(CTimestamp::PRC_SEC);
        if ( (lotmVal2- lotmVal).Sec() < 0
             || (lotmVal- lotmVal1).Sec() < 0 )
        {
            CPPUNIT_FAIL("GetTopBusyTime fail");
        }

        //end environment
        loTrdTsk1.Close();
    }

    void testWait(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadTask loTrdTsk1;
        apl_int_t  liN= 0;
        apl_size_t luN= 0;
        CTimestamp lotmVal;
        CTimestamp lotmVal1;
        CTimestamp lotmVal2;
        lotmVal1.Update(CTimestamp::PRC_SEC);
        loTrdTsk1.Initialize(3, 5, APL_THREAD_KERNEL, 0);
        //case
        apl_sleep(10*APL_TIME_MSEC);
        for(luN= 0; luN<8; ++luN)
        {
            loTrdTsk1.Run(CTestRunnable());
            apl_sleep(10*APL_TIME_MSEC);
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("waitcount fail", (apl_size_t)3, loTrdTsk1.GetWaitCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("topwaitcount fail", (apl_size_t)3, loTrdTsk1.GetTopWaitCount() );
        apl_sleep(2*APL_TIME_SEC);

        for(luN= 0; luN<7; ++luN)
        {
            loTrdTsk1.Run(CTestRunnable() );
            apl_sleep(10*APL_TIME_MSEC);
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("waitcount fail", (apl_size_t)2, loTrdTsk1.GetWaitCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("topwaitcount fail", (apl_size_t)3, loTrdTsk1.GetTopWaitCount() );
        liN= 2;

        lotmVal = loTrdTsk1.GetTopWaitTime();
        lotmVal2.Update(CTimestamp::PRC_SEC);
        if ( (lotmVal2- lotmVal).Sec() < 0
             || (lotmVal- lotmVal1).Sec() < 0 )
        {
            CPPUNIT_FAIL("GetTopWaitTime fail");
        }

        //end environment
        loTrdTsk1.Close();
    }

    void testRun(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CThreadTask loTrdTsk1;
        apl_int_t  lir= 0;
        apl_size_t luN= 0;
        CTimeValue lotm;

        loTrdTsk1.Initialize(3, 5, APL_THREAD_KERNEL, 0);
        //case
        for(luN= 0; luN<8; ++luN)
        {
            lir= loTrdTsk1.Run( CTestRunnable() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Run func fail", (apl_int_t)0, lir);
            apl_sleep(10*APL_TIME_MSEC);
        }
        lotm.Msec(5);
        lir= loTrdTsk1.Run( CTestRunnable(), lotm);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Run func fail", (apl_int_t)-1, lir);
        apl_sleep(APL_TIME_SEC);
        lir= loTrdTsk1.Run( CTestRunnable(), lotm);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Run func fail", (apl_int_t)0, lir);

        //end environment
        loTrdTsk1.Close();
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclThreadManager);
//CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclThreadPool);
//CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclThreadTask);

int main()
{
    RUN_ALL_TEST(__FILE__);
} 


