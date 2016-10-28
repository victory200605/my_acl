#include "Test.h"
#include "apl/apl.h"
#include "acl/Reactor.h"

using namespace acl;

#define RUN_AND_CHECK_RET_INT(func, args) \
    do { \
           apl_int_t liRet; \
           liRet = func args; \
           CPPUNIT_ASSERT_MESSAGE(#func #args "fail", liRet >= 0); \
       } while(0)
  
class CAddOnWriteReady : public IEventHandler
{
private:
    apl_int_t& testN;
    apl_int_t addN;
public:
    CAddOnWriteReady(apl_int_t& liN, apl_int_t liAddN)
        : testN(liN)
        , addN(liAddN)
    {
    }

    ~CAddOnWriteReady()
    {
    }

    virtual apl_int_t HandleInput(apl_handle_t aiHandle)
    {
        CPPUNIT_FAIL("Reactor callback fail");
        return -1;
    }

    virtual apl_int_t HandleOutput(apl_handle_t aiHandle)
    {
        testN += addN;
        return 0;
    }
};

class CAutoTermEvent : public IEventHandler
{
private:
    apl_int_t& testN;
    apl_int_t addN;
    apl_int_t maxSize;
    CReactor* cBase;
public:
    CAutoTermEvent(apl_int_t& liN
        , apl_int_t liAddN
        , apl_int_t liMaxSize
        , CReactor* liBase)
        : testN(liN)
        , addN(liAddN)
        , maxSize(liMaxSize)
        , cBase(liBase)
    {
    } 

    ~CAutoTermEvent()
    {
    }

    virtual apl_int_t HandleInput(apl_handle_t aiHandle)
    {
        CPPUNIT_FAIL("Reactor callback fail");
        return -1;
    }

    virtual apl_int_t HandleOutput(apl_handle_t aiHandle)
    {
        testN += addN;
        if(testN >= maxSize)
        {
            cBase->Deactivate(); 
        }
        return 0;
    }
};

class CConIncrease : public IEventHandler
{
private:
    apl_int_t& testN;
    apl_int_t maxSize;
    bool con;
    CReactor* cBase;
    apl_handle_t otherHandle;
public:
    CConIncrease(apl_int_t& liN
        , apl_int_t liMaxSize
        , bool liCon
        , CReactor* liBase
        , apl_handle_t liOther)
        : testN(liN)
        , maxSize(liMaxSize)
        , con(liCon)
        , cBase(liBase)
        , otherHandle(liOther)
    {
    }
   
    ~CConIncrease()
    {
    }

    virtual apl_int_t HandleInput(apl_handle_t aiHandle)
    {
        CPPUNIT_FAIL("Reactor callback fail");
        return -1;
    }

    virtual apl_int_t HandleOutput(apl_handle_t aiHandle)
    {
        apl_int_t liRet;
        if(con == true)
        {
            if(testN % 2 == 0)
            {
                testN++;
                liRet = cBase->SuspendHandler(aiHandle);
                CPPUNIT_ASSERT_MESSAGE("SuspendHandler() fail", liRet == 0);
                liRet = cBase->ResumeHandler(otherHandle);
                //CPPUNIT_ASSERT_MESSAGE("ResumeHandler() fail", liRet == 0); 
            }
        }
        else
        {
            if(testN % 2 != 0)
            {
                testN++;
                liRet = cBase->SuspendHandler(aiHandle);
                CPPUNIT_ASSERT_MESSAGE("SuspendHandler() fail", liRet == 0);
                liRet = cBase->ResumeHandler(otherHandle);
                //CPPUNIT_ASSERT_MESSAGE("ResumeHandler() fail", liRet == 0); 
            }
        }
        
        if(testN >= maxSize)
        {
            cBase->Deactivate();
        }

        return 0;
    }
};

class CTimerIncrease : public IEventHandler
{
private:
    apl_int_t& testN;
    apl_int_t maxSize;
    apl_int_t addN;
    CTimestamp& futTime;
public:
    CTimerIncrease(apl_int_t& litestN
        , apl_int_t limaxSize
        , apl_int_t liAddN
        , CTimestamp& liFutTime)
        : testN(litestN)
        , maxSize(limaxSize)
        , addN(liAddN)
        , futTime(liFutTime)
    {
    }

    ~CTimerIncrease()
    {
    }

    virtual apl_int_t HandleInput(apl_handle_t aiHandle)
    {
        CPPUNIT_FAIL("Reactor callback fail");
        return -1;
    }

    virtual apl_int_t HandleOutput(apl_handle_t aiHandle)
    {
        return 0;
    }

    virtual apl_int_t HandleTimeout(CTimestamp const& aoCurrentTime, void const* apvAct)
    {
        if(aoCurrentTime <= futTime)
        {
            CPPUNIT_FAIL("Timer calculate fail");
        }

        testN += addN;
        if(testN >= maxSize)
        {
            CReactor* cBase = (CReactor*)apvAct;
            cBase->Deactivate();    
        }

        return 0;
    }
};

class CTestAclReactor : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclReactor);
    CPPUNIT_TEST(testInitialize);
    CPPUNIT_TEST(testRegisterHandler);
    CPPUNIT_TEST(testHandleEvent);
    CPPUNIT_TEST(testRunEventLoop);
    CPPUNIT_TEST(testSuspendAndResume);
    CPPUNIT_TEST(testRemove);
    CPPUNIT_TEST(testTimer);
    CPPUNIT_TEST(testCancelTimer);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}

    void testInitialize(void)
    {
        CReactor loRA;
        apl_handle_t sockA, sockB;
        apl_socketpair(APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockA, &sockB);

        apl_int_t testN = 0;
        CAddOnWriteReady loA1(testN, 1);
        CAddOnWriteReady loA2(testN, 2);
        RUN_AND_CHECK_RET_INT(loRA.Initialize, (1));
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockA, &loA1, IEventHandler::WRITE_MASK));
        apl_int_t liRet;
        liRet = loRA.RegisterHandler(sockB, &loA2, IEventHandler::WRITE_MASK);
        CPPUNIT_ASSERT_MESSAGE("Initialize fail", liRet == -1);  

        for(apl_int_t i = 0;i < 2;i++)
        {
            liRet = loRA.HandleEvent();
            CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", liRet >= 0);
        }
        CPPUNIT_ASSERT_MESSAGE("Initialize fail", testN == 2);
    }

    void testRegisterHandler(void)
    {
        CReactor loRA;
        apl_int_t testN = 0;
        apl_handle_t sockA, sockB, sockC, sockD;
        apl_socketpair(APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockA, &sockB);
        apl_socketpair(APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockC, &sockD); 

        CAddOnWriteReady loA1(testN, 1);
        CAddOnWriteReady loA2(testN, 2);
        CAddOnWriteReady loA3(testN, 3);
        CAddOnWriteReady loA4(testN, 4);
 
        RUN_AND_CHECK_RET_INT(loRA.Initialize, (10));
 
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockA, &loA1, IEventHandler::WRITE_MASK | IEventHandler::READ_MASK));
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockB, &loA2, IEventHandler::WRITE_MASK | IEventHandler::READ_MASK));
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockC, &loA3, IEventHandler::READ_MASK));
        
        apl_int_t liRet;

        //normal test
        for(apl_int_t i = 0;i < 2;i++)
        {
            liRet = loRA.HandleEvent();
            CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", liRet >= 0);
        }

        CPPUNIT_ASSERT_MESSAGE("RegisterHandler fail", testN == 3);     
        
        //test modify event mask 
        testN = 0; 
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockC, &loA3, IEventHandler::WRITE_MASK));
        for(apl_int_t i = 0;i < 3;i++)
        {
            liRet = loRA.HandleEvent();
            CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", liRet >= 0);
        } 

        CPPUNIT_ASSERT_MESSAGE("RegisterHandler fail", testN == 6); 

        //test mutiple register
        testN = 0;
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockC, &loA3, IEventHandler::WRITE_MASK));
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockA, &loA1, IEventHandler::WRITE_MASK | IEventHandler::READ_MASK));
        for(apl_int_t i = 0;i < 6;i++)
        {
            liRet = loRA.HandleEvent();
            CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", liRet >= 0);
        }

        CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", testN == 12);

        //test increate handler
        testN = 0;
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockD, &loA4, IEventHandler::WRITE_MASK));
        for(apl_int_t i = 0;i < 4;i++)
        {
            liRet = loRA.HandleEvent();
            CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", liRet >= 0);
        } 

        CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", testN = 10);

    }
    
    void testHandleEvent(void)
    {
        CReactor loRA;
        apl_int_t testN = 0;
        apl_handle_t sockA, sockB;
        apl_socketpair(APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockA, &sockB);
    
        CAddOnWriteReady loA1(testN, 1);
        CAddOnWriteReady loA2(testN, 2);
        
        RUN_AND_CHECK_RET_INT(loRA.Initialize, (10));
 
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockA, &loA1, IEventHandler::WRITE_MASK | IEventHandler::READ_MASK));
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockB, &loA2, IEventHandler::WRITE_MASK | IEventHandler::READ_MASK));

        apl_int_t liRet;
        for(apl_int_t i = 0;i < 10;i++)
        {
            liRet = loRA.HandleEvent();
            CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", liRet >= 0);
        }

        CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", testN == 15); 

        //test Timeout
        testN = 0;
        loRA.RemoveHandler(sockA, IEventHandler::WRITE_MASK | IEventHandler::READ_MASK);
        loRA.RemoveHandler(sockB, IEventHandler::WRITE_MASK | IEventHandler::READ_MASK);
        liRet = loRA.HandleEvent(1);
        CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", liRet == 0);
        CPPUNIT_ASSERT_MESSAGE("HandlerEvent() fail", testN == 0);
    }

    void testRunEventLoop(void)
    {
        CReactor loRA;
        apl_int_t testN = 0;
        apl_handle_t sockA, sockB;
        apl_socketpair(APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockA, &sockB);

        CAutoTermEvent loA1(testN, 2, 45, &loRA);
        CAutoTermEvent loA2(testN, 3, 45, &loRA);

        RUN_AND_CHECK_RET_INT(loRA.Initialize, (10));

        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockA, &loA1, IEventHandler::WRITE_MASK | IEventHandler::READ_MASK));
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockB, &loA2, IEventHandler::WRITE_MASK | IEventHandler::READ_MASK));
        
        apl_int_t liRet;
        liRet = loRA.RunEventLoop();
        CPPUNIT_ASSERT_MESSAGE("RunEventLoop() fail", liRet == 0);
        CPPUNIT_ASSERT_MESSAGE("RunEventLoop() fail", testN == 45);
    }

    void testSuspendAndResume(void)
    {
        CReactor loRA; 
        apl_int_t testN = 0;
        apl_handle_t sockA, sockB, sockC, sockD;
        apl_socketpair(APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockA, &sockB);
        apl_socketpair(APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockC, &sockD);
        
        //normal test
        CConIncrease loC1(testN, 20, true, &loRA, sockB); 
        CConIncrease loC2(testN, 20, false, &loRA, sockA);

        RUN_AND_CHECK_RET_INT(loRA.Initialize, (10));

        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockA, &loC1, IEventHandler::WRITE_MASK));
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockB, &loC2, IEventHandler::WRITE_MASK));

        apl_int_t liRet;
        liRet = loRA.RunEventLoop();
        CPPUNIT_ASSERT_MESSAGE("RunEventLoop() fail", liRet == 0);
        CPPUNIT_ASSERT_MESSAGE("RunEventLoop() fail", testN == 20);
    }

    void testRemove(void)
    {
        CReactor loRA;
        apl_handle_t sockA, sockB;
        apl_int_t testN = 0;
        apl_int_t liRet;
        apl_socketpair(APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockA, &sockB);
  
        CAddOnWriteReady loA1(testN, 1);
        CAddOnWriteReady loA2(testN, 2);

        RUN_AND_CHECK_RET_INT(loRA.Initialize, (10));
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockA, &loA1, IEventHandler::WRITE_MASK | IEventHandler::READ_MASK));
        RUN_AND_CHECK_RET_INT(loRA.RegisterHandler, (sockB, &loA2, IEventHandler::WRITE_MASK | IEventHandler::READ_MASK));

        //normal test
        RUN_AND_CHECK_RET_INT(loRA.RemoveHandler, (sockB, IEventHandler::READ_MASK));
        for(apl_int_t i = 0;i < 2;i++)
        {
            liRet = loRA.HandleEvent();
            CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", liRet >= 0);
        } 
        
        CPPUNIT_ASSERT_MESSAGE("RemoveHandler() fail", testN == 3);

        testN = 0;
        RUN_AND_CHECK_RET_INT(loRA.RemoveHandler, (sockB, IEventHandler::WRITE_MASK)); 
        for(apl_int_t i = 0;i < 2;i++) 
        {
            liRet = loRA.HandleEvent();
            CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", liRet >= 0);
        } 
        
        CPPUNIT_ASSERT_MESSAGE("RemoveHandler() fail", testN == 2);

        //test remove a suspended handle
        RUN_AND_CHECK_RET_INT(loRA.SuspendHandler, (sockA));
        RUN_AND_CHECK_RET_INT(loRA.RemoveHandler, (sockA, IEventHandler::WRITE_MASK | IEventHandler::READ_MASK));
    }

    void testTimer(void)
    {
        CReactor loRA;
        apl_int_t testN = 0;
        CTimeValue aoTimeout(1);
        apl_int_t timeID1, timeID2;
        apl_handle_t sockA, sockB;
        apl_socketpair(APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockA, &sockB);

        CTimestamp futTime1;
        CTimestamp futTime2;
        futTime1.Nsec(apl_gettimeofday());
        futTime1 += aoTimeout;
        futTime2.Nsec(apl_gettimeofday());
        CTimerIncrease loI1(testN, 10, 2, futTime1);
        CTimerIncrease loI2(testN, 10, 3, futTime2);

        loRA.Initialize(2, new CTimerQueue(2) );
        timeID1 = loRA.ScheduleTimer(&loI1, &loRA, futTime1);
        timeID2 = loRA.ScheduleTimer(&loI2, &loRA, futTime2);
        apl_int_t liRet;
        for(apl_int_t i = 0;i < 2;i++)
        {
            liRet = loRA.HandleEvent(aoTimeout);
            CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", liRet >= 0);
        }     

        CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", testN == 5);
        
        //test RunEventLoop()
        testN = 0;
        loRA.CancelTimer(timeID1, NULL);
        loRA.CancelTimer(timeID2, NULL);
        CTimestamp futTime3;
        CTimestamp futTime4;
        futTime3.Nsec(apl_gettimeofday());
        futTime3 += aoTimeout;
        futTime4.Nsec(apl_gettimeofday());
        CTimerIncrease loI3(testN, 25, 2, futTime3);
        CTimerIncrease loI4(testN, 25, 3, futTime4);
        timeID1 = loRA.ScheduleTimer(&loI3, &loRA, futTime3, aoTimeout);
        timeID2 = loRA.ScheduleTimer(&loI4, &loRA, futTime4, aoTimeout);
        liRet = loRA.RunEventLoop();
 
        CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", liRet >= 0);
        CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", testN == 25); 

        //test overloaded function
        testN = 0;
        CTimeValue aoTimeout2(2);
        CReactor loRA2;
        loRA2.Initialize(2, new CTimerQueue(2));
        CTimestamp futTime5;
        CTimestamp futTime6;
        futTime5.Nsec(apl_gettimeofday());
        futTime5 += aoTimeout;
        futTime6.Nsec(apl_gettimeofday());
        futTime6 += aoTimeout;
        CTimerIncrease loI5(testN, 10, 2, futTime5);
        CTimerIncrease loI6(testN, 10, 3, futTime6);
        timeID1 = loRA2.ScheduleTimer(&loI5, &loRA2, aoTimeout);
        timeID2 = loRA2.ScheduleTimer(&loI6, &loRA2, aoTimeout);
        for(apl_int_t i = 0;i < 2;i++)
        {
            liRet = loRA2.HandleEvent(aoTimeout2);
            CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", liRet >= 0);
        }     

        CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", testN == 5);

        // test overloaded function in RunEventLoop()
        testN = 0;
        loRA2.CancelTimer(timeID1, NULL);
        loRA2.CancelTimer(timeID2, NULL);
        CTimestamp futTime7;
        CTimestamp futTime8;
        futTime7.Nsec(apl_gettimeofday());
        futTime7 += aoTimeout;
        futTime8.Nsec(apl_gettimeofday());
        futTime8 += aoTimeout;
        CTimerIncrease loI7(testN, 2, 25, futTime7);
        CTimerIncrease loI8(testN, 3, 25, futTime8);
        timeID1 = loRA2.ScheduleTimer(&loI7, &loRA2, aoTimeout, aoTimeout);
        timeID2 = loRA2.ScheduleTimer(&loI8, &loRA2, aoTimeout, aoTimeout);
        liRet = loRA2.RunEventLoop();
 
        CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", liRet >= 0);
        CPPUNIT_ASSERT_MESSAGE("HandleEvent() fail", testN >= 25); 
    }

    void testCancelTimer(void)
    {
        CReactor loRA;
        CReactor* loRA2;
        CTimeValue aoTimeout(1);
        apl_int_t timeID;
        apl_int_t testN = 0;
        CTimestamp futTime;
        futTime.Nsec(apl_gettimeofday());
        CTimerIncrease loI(testN, 10, 2, futTime);
       
        loRA.Initialize(1, new CTimerQueue(2));
        timeID = loRA.ScheduleTimer(&loI, &loRA, futTime);
        loRA.CancelTimer(timeID,(const void**)&loRA2);

        apl_int_t liRet = loRA.HandleEvent(aoTimeout);
        CPPUNIT_ASSERT_MESSAGE("cancel fail", liRet == 0);
         
        CReactor* loRA3 = &loRA;
        CPPUNIT_ASSERT_MESSAGE("cancel fail", loRA3 == loRA2);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclReactor);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
