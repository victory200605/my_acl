#include "Test.h"
#include "acl/TimeValue.h"

using namespace acl;

#define TEST_GETSET(obj, func, num) \
    do { \
        obj.func(num); \
        apl_time_t li_a = obj.func(); \
        CPPUNIT_ASSERT_MESSAGE(#func" failed\n", li_a == num); \
    } while(0)

#define TEST_OP_CMP_LESS(objl, objr, flag) \
    do { \
        CPPUNIT_ASSERT_MESSAGE("friend <  failed", (objl <  objr) == flag ); \
    } while(0) 

#define TEST_OP_CMP_GREAT(objl, objr, flag) \
    do { \
        CPPUNIT_ASSERT_MESSAGE("friend >  failed", (objl >  objr) == flag ); \
    } while(0)

#define TEST_OP_CMP_GREATEQUAL(objl, objr, flag) \
    do { \
        CPPUNIT_ASSERT_MESSAGE("friend >  failed", (objl >=  objr) == flag ); \
    } while(0)

#define TEST_OP_CMP_LESSEQUAL(objl, objr, flag) \
    do { \
        CPPUNIT_ASSERT_MESSAGE("friend >  failed", (objl <=  objr) == flag ); \
    } while(0)

#define TEST_OP_CMP_EQUALEQUAL(objl, objr, flag) \
    do { \
        CPPUNIT_ASSERT_MESSAGE("friend ==  failed", (objl ==  objr) == flag ); \
    } while(0)

#define TEST_OP_CMP_NOTEQUAL(objl, objr, flag) \
    do { \
        CPPUNIT_ASSERT_MESSAGE("friend !=  failed", (objl !=  objr) == flag ); \
    } while(0)


class CTestAclTimeValue: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclTimeValue);
    CPPUNIT_TEST(testGetSet);
    CPPUNIT_TEST(testOPPlusEqual);
    CPPUNIT_TEST(testOPSubtEqual);
    CPPUNIT_TEST(testOPPlusPlus);
    CPPUNIT_TEST(testOPSubtSubt);
    CPPUNIT_TEST(testOPGPlus);
    CPPUNIT_TEST(testOPGSubt);
    CPPUNIT_TEST(testOPGLess);
    CPPUNIT_TEST(testOPGGreat);
    CPPUNIT_TEST(testOPGGreatEqual);
    CPPUNIT_TEST(testOPGLessEqual);
    CPPUNIT_TEST(testOPGEqualEqual);
    CPPUNIT_TEST(testOPGNotEqual);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}
    void testGetSet(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment

        //case
        CTimeValue loTime;
        TEST_GETSET(loTime, Nsec,  5);
        TEST_GETSET(loTime, Nsec, -5);
        TEST_GETSET(loTime, Usec,  6);
        TEST_GETSET(loTime, Msec, 10);
        TEST_GETSET(loTime,  Sec, 11);
        TEST_GETSET(loTime,  Min, 60);
        TEST_GETSET(loTime, Hour, 15);
        TEST_GETSET(loTime,  Day,  7);
        TEST_GETSET(loTime, Week,  7);

        CTimeValue loTime1(3.2);
        CPPUNIT_ASSERT_MESSAGE(" failed", loTime1.Sec() == 3 );
        CPPUNIT_ASSERT_MESSAGE(" failed", loTime1.Msec() == 3200 );
         
        CTimeValue loTime2(5, APL_TIME_MIN);
        CPPUNIT_ASSERT_MESSAGE("failed", loTime2.Min() == 5 );
        CPPUNIT_ASSERT_MESSAGE("failed", loTime2.Sec() == 300 );        
    
        CPPUNIT_ASSERT_MESSAGE("max failed", CTimeValue::MAXTIME.Nsec() == APL_INT64_MAX );
        CPPUNIT_ASSERT_MESSAGE("max failed", CTimeValue::ZERO.Nsec() == 0 );

        //end environment
    }

    void testOPPlusEqual(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimeValue loTime1(6), loTime2(10.5);
        //case 
        loTime1 += loTime2;
        CPPUNIT_ASSERT_MESSAGE("CTimeValue += failed", loTime1.Sec() == 16 );
        CPPUNIT_ASSERT_MESSAGE("CTimeValue += failed", loTime1.Msec() == 16500 );

        loTime1 += 5;
        CPPUNIT_ASSERT_MESSAGE("CTimeValue += failed", loTime1.Sec() == 21 );
        //end environment
    }

    void testOPEqual(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimeValue loTime1(6, APL_TIME_SEC), loTime2(10, APL_TIME_SEC);
        //case
        loTime1 = 20;
        CPPUNIT_ASSERT_MESSAGE("CTimeValue = failed", loTime1.Sec() == 20 );

        loTime1 = loTime2;     
        CPPUNIT_ASSERT_MESSAGE("CTimeValue = failed", loTime1.Sec() == 10 ); 

        //end environment
    }
   
    void testOPSubtEqual(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimeValue loTime1(6.2), loTime2(10);
        loTime2 -= loTime1;
        //case 
        CPPUNIT_ASSERT_MESSAGE("CTimeValue -= failed", loTime2.Sec() == 3);
        CPPUNIT_ASSERT_MESSAGE("CTimeValue -= failed", loTime2.Msec() == 3800 );
        loTime1 -= loTime2;
        CPPUNIT_ASSERT_MESSAGE("CTimeValue -= failed", loTime1.Sec() == 2);
        
        //end environment
    }

    void testOPPlusPlus(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimeValue loTime1(6, APL_TIME_SEC);
        //case
        CPPUNIT_ASSERT_MESSAGE("CTimeValue ++(int) failed", (loTime1++).Sec() == 6 );
        CPPUNIT_ASSERT_MESSAGE("CTimeValue ++(int) failed", loTime1.Sec() == 7 );
        CPPUNIT_ASSERT_MESSAGE("CTimeValue ++() failed", (++loTime1).Sec() == 8 ); 
        CPPUNIT_ASSERT_MESSAGE("CTimeValue ++() failed", loTime1.Sec() == 8 );
       
        //end environment
    }

    void testOPSubtSubt(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimeValue loTime1(10, APL_TIME_SEC);
        //case
        CPPUNIT_ASSERT_MESSAGE("CTimeValue --(int) failed", (loTime1--).Sec() == 10 );
        CPPUNIT_ASSERT_MESSAGE("CTimeValue --(int) failed", loTime1.Sec() == 9 );
        CPPUNIT_ASSERT_MESSAGE("CTimeValue --() failed", (--loTime1).Sec() == 8 ); 
        CPPUNIT_ASSERT_MESSAGE("CTimeValue --() failed", loTime1.Sec() == 8 ); 
        
        //environment
    }

    void testOPGPlus(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimeValue loTime1(10), loTime2(20);
        //case
        loTime1 = loTime1 + loTime2;
        CPPUNIT_ASSERT_MESSAGE("friend + failed", loTime1.Sec() == 30 );
        loTime2 = loTime1 + loTime2;
        CPPUNIT_ASSERT_MESSAGE("friend + failed", loTime2.Sec() == 50 );
        
        //case environment
    }

    void testOPGSubt(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimeValue loTime1(5), loTime2(8);
        //case
        loTime2 = loTime2 - loTime1;
        CPPUNIT_ASSERT_MESSAGE("friend - failed", loTime2.Sec() == 3 );
        loTime1 = loTime1 - loTime2;
        CPPUNIT_ASSERT_MESSAGE("friend - failed", loTime1.Sec() == 2 );

        //end environment
    }
    
    void testOPGLess(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimeValue loTime1(10);
        CTimeValue loTime2(8 );
        //case
        TEST_OP_CMP_LESS(loTime1, loTime2, false);
        TEST_OP_CMP_LESS(loTime2, loTime1, true);

        //end environment
    }

    void testOPGGreat(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimeValue loTime1(10);
        CTimeValue loTime2(8 );
        //case
        TEST_OP_CMP_GREAT(loTime1, loTime2, true);
        TEST_OP_CMP_GREAT(loTime2, loTime1, false);
     
        //end environment
    }

    void testOPGGreatEqual(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimeValue loTime1(10);
        CTimeValue loTime2(8 );
        //case
        TEST_OP_CMP_GREATEQUAL(loTime1, loTime2, true);
        TEST_OP_CMP_GREATEQUAL(loTime2, loTime1, false);
    
        //end environment
    }

    void testOPGLessEqual(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimeValue loTime1(10);
        CTimeValue loTime2(8 );
        //case
        TEST_OP_CMP_LESSEQUAL(loTime1, loTime2, false);
        TEST_OP_CMP_LESSEQUAL(loTime2, loTime1, true);
    
        //end environment
    }

    void testOPGEqualEqual(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimeValue loTime1(10);
        CTimeValue loTime2(8 );
        //case
        TEST_OP_CMP_EQUALEQUAL(loTime1, loTime2, false);
        loTime1.Msec(5);
        loTime2.Msec(5); 
        TEST_OP_CMP_EQUALEQUAL(loTime1, loTime2, true);
        //end environment
    }

    void testOPGNotEqual(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimeValue loTime1(10);
        CTimeValue loTime2(8 );
        //case
        TEST_OP_CMP_NOTEQUAL(loTime1, loTime2, true);
        loTime1.Msec(5);
        loTime2.Msec(5); 
        TEST_OP_CMP_NOTEQUAL(loTime2, loTime1, false);
       
        //end environment
    }
 
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclTimeValue);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
