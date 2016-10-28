#include "Test.h"
#include "acl/TimeValue.h"
#include "acl/Timestamp.h"

using namespace acl;

#define TEST_GETSET(obj, func, num) \
    do { \
        obj.func(num); \
        apl_time_t lti = obj.func(); \
        CPPUNIT_ASSERT_MESSAGE(#func" failed\n", lti == num); \
    } while(0)


#define TEST_PLUS_EQUAL(objts, objtv) \
    do { \
        apl_time_t ltNsec = objts.Nsec(); \
        apl_time_t ltUsec = objts.Usec(); \
        apl_time_t ltMsec = objts.Msec(); \
        apl_time_t ltSec  = objts.Sec(); \
        apl_time_t ltNsec1= objtv.Nsec(); \
        apl_time_t ltUsec1= objtv.Usec(); \
        apl_time_t ltMsec1= objtv.Msec(); \
        apl_time_t ltSec1 = objtv.Sec(); \
        objts += objtv; \
        CPPUNIT_ASSERT_MESSAGE("failed", objts.Nsec() == ltNsec + ltNsec1 ); \
        CPPUNIT_ASSERT_MESSAGE("failed", objts.Usec() == ltUsec + ltUsec1 ); \
        CPPUNIT_ASSERT_MESSAGE("failed", objts.Msec() == ltMsec + ltMsec1 ); \
        CPPUNIT_ASSERT_MESSAGE("failed", objts.Sec()  == ltSec + ltSec1 ); \
    } while(0) 

#define TEST_SUBT_EQUAL(objts, objtv) \
    do { \
        apl_time_t ltNsec = objts.Nsec(); \
        apl_time_t ltUsec = objts.Usec(); \
        apl_time_t ltMsec = objts.Msec(); \
        apl_time_t ltSec  = objts.Sec(); \
        apl_time_t ltNsec1= objtv.Nsec(); \
        apl_time_t ltUsec1= objtv.Usec(); \
        apl_time_t ltMsec1= objtv.Msec(); \
        apl_time_t ltSec1 = objtv.Sec(); \
        objts -= objtv; \
        CPPUNIT_ASSERT_MESSAGE("failed", objts.Nsec() == ltNsec - ltNsec1 ); \
        CPPUNIT_ASSERT_MESSAGE("failed", objts.Usec() == ltUsec - ltUsec1 ); \
        CPPUNIT_ASSERT_MESSAGE("failed", objts.Msec() == ltMsec - ltMsec1 ); \
        CPPUNIT_ASSERT_MESSAGE("failed", objts.Sec()  == ltSec - ltSec1 ); \
    } while(0)


#define TEST_G_SUBT(objts, objtv, obj) \
    do { \
        apl_time_t ltNsec = objts.Nsec(); \
        apl_time_t ltUsec = objts.Usec(); \
        apl_time_t ltMsec = objts.Msec(); \
        apl_time_t ltSec  = objts.Sec(); \
        apl_time_t ltNsec1= objtv.Nsec(); \
        apl_time_t ltUsec1= objtv.Usec(); \
        apl_time_t ltMsec1= objtv.Msec(); \
        apl_time_t ltSec1 = objtv.Sec(); \
        obj = objts - objtv; \
        CPPUNIT_ASSERT_MESSAGE("failed", obj.Nsec() == ltNsec - ltNsec1 ); \
        CPPUNIT_ASSERT_MESSAGE("failed", obj.Usec() == ltUsec - ltUsec1 ); \
        CPPUNIT_ASSERT_MESSAGE("failed", obj.Msec() == ltMsec - ltMsec1 ); \
        CPPUNIT_ASSERT_MESSAGE("failed", obj.Sec()  == ltSec - ltSec1 ); \
    } while(0)


/* ---------------------------------------------------------------------- */

#define ASSERT_TM_FIELD(field, tm1, tm2) \
    do { \
        CPPUNIT_ASSERT_EQUAL_MESSAGE( \
            "struct apl_tm_t::" #field " FAILED", \
            tm1.field, \
            tm2.field); \
    } while(0)

/* ---------------------------------------------------------------------- */

#define ASSERT_TM_EQUAL(tm1, tm2) \
    do { \
        ASSERT_TM_FIELD(tm_wday, tm1, tm2); \
        ASSERT_TM_FIELD(tm_year, tm1, tm2); \
        ASSERT_TM_FIELD(tm_mon, tm1, tm2); \
        ASSERT_TM_FIELD(tm_mday, tm1, tm2); \
        ASSERT_TM_FIELD(tm_hour, tm1, tm2); \
        ASSERT_TM_FIELD(tm_min, tm1, tm2); \
        ASSERT_TM_FIELD(tm_sec, tm1, tm2); \
    } while(0)


#define TEST_UPDATE(kind, attr, precision, aplvalue ) \
    do { \
        CTimestamp loSt1; \
        CTimestamp loSt2; \
        apl_time_t li64Time2; \
        apl_time_t li64Time1; \
        apl_time_t li64Time; \
        loSt1.Nsec(0); \
        loSt2.Nsec(0); \
        li64Time2= aplvalue; \
        loSt1.Update(kind); \
        loSt2.Update(kind); \
        if ( kind == CTimestamp::PRC_NSEC ) \
        { \
	    li64Time1 = loSt1.Nsec(); \
	    li64Time = li64Time1 - li64Time2 / precision; \
	    CPPUNIT_ASSERT_MESSAGE("update fail", \
		li64Time >= 0); \
        } \
        if ( kind == CTimestamp::PRC_USEC || \
             kind == CTimestamp::PRC_MSEC ) \
        { \
            li64Time1= loSt1.Usec(); \
            li64Time= li64Time1- li64Time2/precision; \
            CPPUNIT_ASSERT_MESSAGE("update fail", \
                li64Time >= 0 ); \
        } \
        if ( kind == CTimestamp::PRC_SEC ) \
        { \
            li64Time1= loSt1.Sec(); \
            li64Time= li64Time1- li64Time2/precision; \
            CPPUNIT_ASSERT_MESSAGE("update fail", \
                li64Time >= 0 ); \
        } \
        li64Time = loSt1.Sec(); \
        CPPUNIT_ASSERT_MESSAGE("update fail", \
            li64Time != 0 ); \
        li64Time = loSt2.Sec()- loSt1.Sec(); \
        CPPUNIT_ASSERT_MESSAGE("update fail Sec", \
            li64Time >= 0 ); \
        li64Time = loSt2.Msec()- loSt1.Msec(); \
        CPPUNIT_ASSERT_MESSAGE("update fail Msec", \
            li64Time >= 0 ); \
        li64Time = loSt2.Usec()- loSt1.Usec(); \
        CPPUNIT_ASSERT_MESSAGE("update fail Usec", \
            li64Time >= 0 ); \
        li64Time = loSt2.Nsec()- loSt1.Nsec(); \
        CPPUNIT_ASSERT_MESSAGE("update fail Nsec", \
            li64Time >= 0 ); \
    } while(0)



class CTestAclTimestamp: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclTimestamp);
    CPPUNIT_TEST(testGetSet);
    CPPUNIT_TEST(testOPPlusEqual);
    CPPUNIT_TEST(testOPSubtEqual);
    CPPUNIT_TEST(testOPGSubt);
    CPPUNIT_TEST(testUpdate);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}
    void testGetSet(void)
    {
        PRINT_TITLE_2(__func__);
        //set environment
        CTimestamp loSt1;
        //case
        TEST_GETSET(loSt1, Nsec, 5);
        TEST_GETSET(loSt1, Usec, 8);
        TEST_GETSET(loSt1, Msec, 10);
        TEST_GETSET(loSt1,  Sec, 11);
        
        //end environment
    }

    void testOPPlusEqual(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimestamp loSst1(10);
        CTimeValue loStv1(5, APL_TIME_NSEC);
        //case
        TEST_PLUS_EQUAL(loSst1, loStv1);    

        CTimestamp loSst2(5);
        CTimeValue loStv2(6);
        TEST_PLUS_EQUAL(loSst2, loStv2);    
 
        CTimestamp loSst3(6);
        CTimeValue loStv3(6.5);
        loSst3 += loStv3;
        CPPUNIT_ASSERT_MESSAGE("failed", loSst3.Sec() == 6 );
        
        //end environment
    }

    void testOPSubtEqual(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimestamp loSst1(10);
        CTimeValue loStv1(5, APL_TIME_NSEC);
        TEST_SUBT_EQUAL(loSst1, loStv1);

        CTimestamp loSst2(10*APL_TIME_SEC);
        CTimeValue loStv2(6);
        TEST_SUBT_EQUAL(loSst2, loStv2);

        CTimestamp loSst3(10*APL_TIME_SEC);
        CTimeValue loStv3(6.5);
        loSst3 -= loStv3;
        CPPUNIT_ASSERT_MESSAGE("failed", loSst3.Sec() == 3 );
    }

    void testOPGSubt(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CTimestamp loSst1(20);
        CTimestamp loSst2(9 );
        CTimeValue loStv1;
        //case
        TEST_G_SUBT(loSst1, loSst2, loStv1);

        //end environment
    }
  
    void testUpdate(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment

        //case
        TEST_UPDATE( CTimestamp::PRC_NSEC, Nsec, APL_TIME_NSEC, apl_clock_gettime() );
        TEST_UPDATE( CTimestamp::PRC_USEC, Usec, APL_TIME_USEC, apl_gettimeofday() );
        TEST_UPDATE( CTimestamp::PRC_MSEC, Msec, APL_TIME_USEC, apl_gettimeofday() );
        TEST_UPDATE( CTimestamp::PRC_SEC ,  Sec, APL_TIME_SEC,  apl_time() );

        //end environment
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclTimestamp);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
