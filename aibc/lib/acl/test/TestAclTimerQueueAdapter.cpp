#include "Test.h"
#include "acl/TimerQueueAdapter.h"
#include "acl/Timestamp.h"
#include "acl/EventHandler.h"

using namespace acl;

static apl_int_t giNum1;
static apl_int_t giNum2;

class MyTimerEvent1 : public IEventHandler
{
    public:
        MyTimerEvent1()
        {
            printf("Timer 1: Construct\n");
        }
        virtual ~MyTimerEvent1()
        {
            printf("Timer 1: Disconstruct\n");
        }
        apl_int_t HandleTimeout( 
                CTimestamp const& aoCurrentTime, void const* apvAct = 0 )
        {
            printf("Timer 1: Time out\n");
            giNum1++;

            return 0;
        }
        apl_int_t HandleClose( apl_handle_t aiHandle, apl_int_t aiMask )
        {
            printf("Timer 1: Close\n");
            delete this;
            return 0;
        }
};

class MyTimerEvent2 : public IEventHandler
{
    public:
        MyTimerEvent2()
        {
            printf("Timer 2: Construct\n");
        }
        virtual ~MyTimerEvent2()
        {
            printf("Timer 2: Disconstruct\n");
        }
        apl_int_t HandleTimeout(
                CTimestamp const& aoCurrentTime, void const* apvAct = 0 )
        {
            printf("Timer 2: Time out\n");
            giNum2++;

            return 0;
        }
        apl_int_t HandleClose( apl_handle_t aiHandle, apl_int_t aiMask )
        {
            printf("Timer 2: Close\n");
            delete this;
            return 0;
        }
};

class CTestAclTimerQueueAdapter:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclTimerQueueAdapter);
    CPPUNIT_TEST(testActivate);
    CPPUNIT_TEST(testScheduleFut);
    CPPUNIT_TEST(testScheduleAft);
    CPPUNIT_TEST(testCancel);
    CPPUNIT_TEST(testDeactivate);
    CPPUNIT_TEST(testWait);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) 
    {
        giNum1 = 0;
        giNum2 = 0;
    }
    void tearDown(void) {}

    void testActivate(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment 
        apl_int_t liRet = -1;
        CTimerQueueAdapter loTimerQueue(2);
        apl_int_t liTimeID;
        //case
        liTimeID = loTimerQueue.Schedule(
                new MyTimerEvent1, NULL, CTimeValue(0.1), CTimeValue(1) );
        loTimerQueue.Wait(CTimeValue(2));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Haven't Activate.", (apl_int_t)0, giNum1);
        
        liRet = loTimerQueue.Activate();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Activate failed.", (apl_int_t)0, liRet);
        loTimerQueue.Wait(CTimeValue(2));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Activate failed.", (apl_int_t)3, giNum1);
        //end environment
        loTimerQueue.Deactivate();
    }

    void testScheduleFut(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        apl_int_t liRet = -1;
        CTimerQueueAdapter loTimerQueue(2);
        CTimestamp loFutureTime;
        CTimeValue loTimeValue;
        time_t liStart;

        liRet = loTimerQueue.Activate();                                      
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Activate failed.", (apl_int_t)0, liRet);

        //case
        loFutureTime.Update(CTimestamp::PRC_USEC);
        loTimeValue.Sec(1);
        loFutureTime += loTimeValue;
        time(&liStart);
        apl_int_t liTimeID = loTimerQueue.Schedule(
                new MyTimerEvent1, NULL, loFutureTime );
        CPPUNIT_ASSERT_MESSAGE("Schedule failed.",(apl_int_t)0 <= liTimeID);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test schedule future time failed.", (apl_int_t)0, giNum1);
        WAIT_EXP(giNum1 == 1);
        CPPUNIT_ASSERT_MESSAGE("Test schedule future time failed.", time(NULL) - liStart >= 1 );

        loFutureTime.Update(CTimestamp::PRC_USEC);
        loTimeValue.Sec(2);                       
        loFutureTime += loTimeValue;              
        time(&liStart);
        liTimeID = loTimerQueue.Schedule(new MyTimerEvent2, NULL, loFutureTime );
        CPPUNIT_ASSERT_MESSAGE("Schedule failed.",(apl_int_t)0 <= liTimeID);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test schedule future time failed.", (apl_int_t)0, giNum2);
        WAIT_EXP(giNum2 == 1);
        CPPUNIT_ASSERT_MESSAGE("Test schedule future time failed.", time(NULL) - liStart >= 2 );
        
        //end environment
        loTimerQueue.Deactivate();
    }

    void testScheduleAft(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        apl_int_t liRet = -1;
        CTimerQueueAdapter loTimerQueue(2);
        CTimeValue loTimeValue;
        time_t liStart;

        liRet = loTimerQueue.Activate();                                      
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Activate failed.", (apl_int_t)0, liRet);

        //case
        time(&liStart);
        apl_int_t liTimeID = loTimerQueue.Schedule(
                new MyTimerEvent1, NULL, CTimeValue(1) );
        CPPUNIT_ASSERT_MESSAGE("Schedule failed.",(apl_int_t)0 <= liTimeID);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test schedule aft time failed.", (apl_int_t)0, giNum1);
        WAIT_EXP(giNum2 == 1);
        CPPUNIT_ASSERT_MESSAGE("Test schedule aft time failed.", time(NULL) - liStart >= 1 );

        time(&liStart);
        liTimeID = loTimerQueue.Schedule(new MyTimerEvent2, NULL, CTimeValue(2) );
        CPPUNIT_ASSERT_MESSAGE("Schedule failed.",(apl_int_t)0 <= liTimeID);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test schedule aft time failed.", (apl_int_t)0, giNum2);
        
        WAIT_EXP(giNum2 == 1);
        CPPUNIT_ASSERT_MESSAGE("Test schedule aft time failed.", time(NULL) - liStart >= 2 );
        
        //end environment
        loTimerQueue.Deactivate();
    }

    void testCancel(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CTimerQueueAdapter loTimerQueue(2);

        apl_int_t liTimeID = loTimerQueue.Schedule(
                new MyTimerEvent1, NULL, CTimeValue(0.1), CTimeValue(1));
        loTimerQueue.Schedule(new MyTimerEvent2, NULL, CTimeValue(0.1), CTimeValue(1));
        loTimerQueue.Activate();
        loTimerQueue.Wait(CTimeValue(1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Schedule failed.",(apl_int_t)2, giNum1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Schedule failed.",(apl_int_t)2, giNum2);

        //case
        loTimerQueue.Cancel(liTimeID, NULL);
        loTimerQueue.Wait(CTimeValue(1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cancle failed.",(apl_int_t)2, giNum1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cancle failed.",(apl_int_t)4, giNum2);

        //end environment
        loTimerQueue.Deactivate();
    }

    void testDeactivate(void)
    {
        PRINT_TITLE_2(__func__);           
        //start case environment
        CTimerQueueAdapter loTimerQueue(2);
        apl_int_t liNum1 = 0;
        apl_int_t liNum2 = 0;
        
        loTimerQueue.Schedule(new MyTimerEvent1, NULL, CTimeValue(0.5), CTimeValue(1));
        loTimerQueue.Activate();
        loTimerQueue.Wait(CTimeValue(1));
        liNum1 = giNum1;

        //case
        loTimerQueue.Deactivate();
        loTimerQueue.Wait(CTimeValue(3)); 
        liNum2 = giNum1;
        
        CPPUNIT_ASSERT_MESSAGE("Schedule failed.", liNum2 - liNum1 >= 0 && liNum2 - liNum1 <= 1 );

        //end environment
    }

    void testWait(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CTimerQueueAdapter loTimerQueue(2);
        loTimerQueue.Schedule(new MyTimerEvent1, NULL, CTimeValue(0.1), CTimeValue(1));
        loTimerQueue.Activate();                                                       

        //case
        loTimerQueue.Wait(CTimeValue(1));                                              
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Schedule failed.",(apl_int_t)2, giNum1);         

        //end environment
        loTimerQueue.Deactivate();
    }
};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclTimerQueueAdapter);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

