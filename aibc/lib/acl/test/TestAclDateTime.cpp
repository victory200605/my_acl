#include "Test.h"
#include "acl/DateTime.h"

using namespace acl;

class CTestAclDateTime:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclDateTime);
    CPPUNIT_TEST(testSetDateTime);
    CPPUNIT_TEST(testUpdateVoid);
    CPPUNIT_TEST(testUpdateIntTimeRegion);
    CPPUNIT_TEST(testUpdateFormatTime);
    CPPUNIT_TEST(testGetTimestamp1);
    CPPUNIT_TEST(testGetTimestamp2);
    CPPUNIT_TEST(testOPSubt);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testSetDateTime(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CDateTime loDateTime;

        //case
        TestSetDateTime(loDateTime,2009,9,9,10,0,0);
        TestSetDateTime(loDateTime,2001,1,2,3,4,5);

        //end environment
    }
    
    void TestSetDateTime(
            CDateTime &aoDateTime, 
            apl_time_t ai64Year,
            apl_time_t ai64Month,
            apl_time_t ai64Day,
            apl_time_t ai64Hour,
            apl_time_t ai64Minute,
            apl_time_t ai64Second )
    {
        apl_int_t   li_ret; 
        li_ret = aoDateTime.Set(ai64Year, ai64Month, ai64Day, ai64Hour, ai64Minute, ai64Second); 
        CPPUNIT_ASSERT_MESSAGE("Set DateTime failed.", li_ret == 0); 
        CPPUNIT_ASSERT_MESSAGE("Get year failed.", ai64Year == aoDateTime.GetYear()); 
        CPPUNIT_ASSERT_MESSAGE("Get month failed.", ai64Month == aoDateTime.GetMonth()); 
        CPPUNIT_ASSERT_MESSAGE("Get day failed.", ai64Day == aoDateTime.GetDay()); 
        CPPUNIT_ASSERT_MESSAGE("Get hour failed.", ai64Hour == aoDateTime.GetHour()); 
        CPPUNIT_ASSERT_MESSAGE("Get minute failed.", ai64Minute == aoDateTime.GetMin());
        CPPUNIT_ASSERT_MESSAGE("Get second failed.", ai64Second == aoDateTime.GetSec()); 
    }

    /* ------------------------------------------------------------- */

    void testUpdateVoid(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CDateTime loDateTime;

        //case
        loDateTime.Update(); 
        apl_time_t li64_time = apl_time(); 
        struct apl_tm_t ls_tm; 
        apl_localtime(li64_time, &ls_tm); 
        CPPUNIT_ASSERT_MESSAGE("Different year.",loDateTime.GetYear() == ls_tm.tm_year+1900); 
        CPPUNIT_ASSERT_MESSAGE("Different month.", loDateTime.GetMonth() == ls_tm.tm_mon + 1); 
        CPPUNIT_ASSERT_MESSAGE("Different day.", loDateTime.GetDay() == ls_tm.tm_mday); 
        CPPUNIT_ASSERT_MESSAGE("Different week day.", loDateTime.GetWeekday() == ls_tm.tm_wday); 
        CPPUNIT_ASSERT_MESSAGE("Different year day.", loDateTime.GetYearday() == ls_tm.tm_yday); 
        CPPUNIT_ASSERT_MESSAGE("Different hour.", loDateTime.GetHour() == ls_tm.tm_hour); 
        CPPUNIT_ASSERT_MESSAGE("Different minute.", loDateTime.GetMin() == ls_tm.tm_min); 
        CPPUNIT_ASSERT_MESSAGE("Different second.", loDateTime.GetSec() == ls_tm.tm_sec); 

        //end environment
    }

    /* ------------------------------------------------------------- */

    void testUpdateIntTimeRegion(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CDateTime loDateTime;

        //case
        UpdateIntTimeRegion(loDateTime, 8);
        UpdateIntTimeRegion(loDateTime, -8);
        UpdateIntTimeRegion(loDateTime, 12);
        UpdateIntTimeRegion(loDateTime, -12);
        UpdateIntTimeRegion(loDateTime, 13);
        UpdateIntTimeRegion(loDateTime, -13);

        //end environment
    }

    void UpdateIntTimeRegion(CDateTime &aoDateTime, apl_int_t aiTimeRegion)
    {
        apl_int_t liRet= aoDateTime.Update(aiTimeRegion);
        if(aiTimeRegion > 48 || aiTimeRegion < -48)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Not equal.", -1, (int)liRet);
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Not equal.", 0, (int)liRet);
            apl_time_t li64Time = apl_time() + aiTimeRegion*900*APL_TIME_SEC;
            struct apl_tm_t lsTm;
            apl_gmtime(li64Time, &lsTm);

            CPPUNIT_ASSERT_MESSAGE("Different year.",aoDateTime.GetYear() == lsTm.tm_year+1900); 
            CPPUNIT_ASSERT_MESSAGE("Different month.", aoDateTime.GetMonth() == lsTm.tm_mon + 1); 
            CPPUNIT_ASSERT_MESSAGE("Different day.", aoDateTime.GetDay() == lsTm.tm_mday); 
            CPPUNIT_ASSERT_MESSAGE("Different week day.", aoDateTime.GetWeekday() == lsTm.tm_wday); 
            CPPUNIT_ASSERT_MESSAGE("Different year day.", aoDateTime.GetYearday() == lsTm.tm_yday); 
            CPPUNIT_ASSERT_MESSAGE("Different hour.", aoDateTime.GetHour() == lsTm.tm_hour); 
            CPPUNIT_ASSERT_MESSAGE("Different minute.", aoDateTime.GetMin() == lsTm.tm_min); 
            CPPUNIT_ASSERT_MESSAGE("Different second.", aoDateTime.GetSec() == lsTm.tm_sec); 
        }
    }

    /* ------------------------------------------------------------- */

    void testUpdateFormatTime(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        //case
        UpdateFormatTime("20090428 14:24:32", "%Y%m%d %H:%M:%S", "20090428 14:24:32", "%Y%m%d %H:%M:%S");
        UpdateFormatTime("090428142432", CDateTime::DEFAULT_FORMAT, "090428142432", CDateTime::DEFAULT_FORMAT);

        UpdateFormatTime("20090428 14:24:32", "%Y%m%d", "20090428 00:00:00", "%Y%m%d %H:%M:%S");
        UpdateFormatTime("20090428 14:24:32", "%Y%m%d", "090428000000", CDateTime::DEFAULT_FORMAT);
        UpdateFormatTime("20090428 14:24:32", "%Y%m", "090331000000", CDateTime::DEFAULT_FORMAT);
        UpdateFormatTime("20090428 14:24:32", "%Y", "081231000000", CDateTime::DEFAULT_FORMAT);
        //end environment
    }

    void UpdateFormatTime(char const* apcStr, char const* apcFormat, 
            char const* apcExpectStr, char const* apcExpectFormat )
    {
        CDateTime loDateTime;

        apl_int_t liRet = loDateTime.Update(apcStr, apcFormat);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Update date time from string failed.", 
                (apl_int_t) 0, liRet);

        char const * lpcOut = loDateTime.Format(apcExpectFormat);
        CPPUNIT_ASSERT_MESSAGE("Format time failed.", apl_strcmp(apcExpectStr, lpcOut)==0 );
    }

    /* ------------------------------------------------------------- */

    void testGetTimestamp1(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CTimestamp loTimestamp;
        CDateTime loDateTime;

        //case
        loDateTime.Update();
        loTimestamp = loDateTime.GetTimestamp();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get timestamp failed.", apl_time(),loTimestamp.Nsec());

        //end environment
    }

    void testGetTimestamp2(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CTimestamp loTimestamp1; 
        CDateTime loDateTime;   

        //case
        loDateTime.Update();
        apl_int_t liRet = loDateTime.GetTimestamp(loTimestamp1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get timestamp failed.", (apl_int_t)0, liRet);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get timestamp failed.", apl_time(),loTimestamp1.Nsec());

        //end environment
    }

    /* ------------------------------------------------------------- */

    void testOPSubt(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CDateTime loLhs;
        CDateTime loRhs;

        //case
        loLhs.Update("090428145201");
        loRhs.Update("090428145200");
        CTimeValue loTimeValueGap = loLhs - loRhs;
        CPPUNIT_ASSERT_MESSAGE("CDateTime - failed.", loTimeValueGap.Sec() == 1);

        loLhs.Update("090428145500");
        loRhs.Update("090428145200");
        loTimeValueGap= loLhs - loRhs;
        CPPUNIT_ASSERT_MESSAGE("CDateTime - failed.", loTimeValueGap.Min() == 3);

        loLhs.Update("090430145200");
        loRhs.Update("090428145200");
        loTimeValueGap= loLhs - loRhs;
        CPPUNIT_ASSERT_MESSAGE("CDateTime - failed.", loTimeValueGap.Day() == 2);

        //end environment
    }

    /* ------------------------------------------------------------------------ */
};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclDateTime);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

