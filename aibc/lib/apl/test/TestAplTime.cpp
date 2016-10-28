#include "Test.h"

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
        /*ASSERT_TM_FIELD(tm_yday, tm1, tm2);*/ \
        ASSERT_TM_FIELD(tm_wday, tm1, tm2); \
        ASSERT_TM_FIELD(tm_year, tm1, tm2); \
        ASSERT_TM_FIELD(tm_mon, tm1, tm2); \
        ASSERT_TM_FIELD(tm_mday, tm1, tm2); \
        ASSERT_TM_FIELD(tm_hour, tm1, tm2); \
        ASSERT_TM_FIELD(tm_min, tm1, tm2); \
        ASSERT_TM_FIELD(tm_sec, tm1, tm2); \
    } while(0)

/* ---------------------------------------------------------------------- */

#define PRINT_TM_FIELD(ptm, field) apl_errprintf("struct apl_tm_t::"#field": %d\n", aps_tm->field)

/* ---------------------------------------------------------------------- */

void PrintTm(struct apl_tm_t const* aps_tm) 
{
    apl_errprintf("struct apl_tm_t::tm_yday: %d\n", aps_tm->tm_yday);
    apl_errprintf("struct apl_tm_t::tm_wday: %d\n", aps_tm->tm_wday);
    apl_errprintf("struct apl_tm_t::tm_year: %d\n", aps_tm->tm_year);
    apl_errprintf("struct apl_tm_t::tm_mon: %d\n", aps_tm->tm_mon);
    apl_errprintf("struct apl_tm_t::tm_mday: %d\n", aps_tm->tm_mday);
    apl_errprintf("struct apl_tm_t::tm_hour: %d\n", aps_tm->tm_hour);
    apl_errprintf("struct apl_tm_t::tm_min: %d\n", aps_tm->tm_min);
    apl_errprintf("struct apl_tm_t::tm_sec: %d\n", aps_tm->tm_sec);
}

/* ---------------------------------------------------------------------- */

void LocalTimeFormat(char const* apc_title, apl_time_t li64_time) 
{
    char            lac_msg[64];
    char            lac_strf[32];
    char            lac_asc[32];
    char const      lac_fmt[] = "%a %b %d %H:%M:%S %Y\n";
    struct apl_tm_t ls_tm;
    struct apl_tm_t ls_tm_strf;
    struct apl_tm_t ls_tm_asc;
    void*           lpc_ret;
    apl_int_t       li_ret;

    apl_snprintf(lac_msg, sizeof(lac_msg), "%s (%s)", apc_title, "apl_localtime");
    PRINT_TITLE_3(lac_msg);

    // convert to apl_tm_t
    lpc_ret = apl_localtime(li64_time, &ls_tm);
    CPPUNIT_ASSERT_MESSAGE("apl_localtime", &ls_tm == lpc_ret);

    PrintTm(&ls_tm);

    // format to string (strf)
    li_ret = apl_strftime(
        lac_strf, 
        sizeof(lac_strf),
        lac_fmt, 
        &ls_tm);

    CPPUNIT_ASSERT_MESSAGE(
        "apl_strftime() FAILED",
        li_ret > 0 && strlen(lac_strf) == (apl_size_t)li_ret);

    apl_errprintf("apl_strftime: %s", lac_strf);

    // parse from string (strf)
    lpc_ret = apl_strptime(lac_strf, lac_fmt, &ls_tm_strf);
    ASSERT_TM_EQUAL(ls_tm, ls_tm_strf);

    ////////////////////////////////////////////////////////////////////
    // format to string again (asc)
    lpc_ret = apl_asctime(&ls_tm_strf, lac_asc, sizeof(lac_asc));

    CPPUNIT_ASSERT_MESSAGE(
        "apl_asctime() FAILED",
        lac_asc == lpc_ret);

    apl_errprintf("apl_asctime: %s", lac_asc);

    ////////////////////////////////////////////////////////////////////
    // parse from string(asc)
    lpc_ret = apl_strptime(lac_asc, lac_fmt, &ls_tm_asc);

    ASSERT_TM_EQUAL(ls_tm, ls_tm_asc);

    ////////////////////////////////////////////////////////////////////
    // apl_strftime and apl_asctime should get the same result
    ASSERT_TM_EQUAL(ls_tm_strf, ls_tm_asc);

    /*
     * string date time would not be the same 
     * for example:
     * (strf)   Thu Jan 01 08:00:00 1970 
     * (asc)    Thu Jan  1 08:00:00 1970
     */

    /* 
    CPPUNIT_ASSERT_MESSAGE(
        "apl_asctime() and apl_strptime() FAILED",
        apl_strcmp(lac_strf, lac_asc) == 0);
    */
}

void GmTimeFormat(char const* apc_title, apl_time_t li64_time) 
{
    char            lac_msg[64];
    char            lac_strf[32];
    char            lac_asc[32];
    char const      lac_fmt[] = "%a %b %d %H:%M:%S %Y\n";
    struct apl_tm_t ls_tm;
    struct apl_tm_t ls_tm_strf;
    struct apl_tm_t ls_tm_asc;
    void*           lpc_ret;
    apl_int_t       li_ret;

    apl_snprintf(lac_msg, sizeof(lac_msg), "%s (%s)", apc_title, "apl_gmtime");
    PRINT_TITLE_3(lac_msg);

    // convert to apl_tm_t
    lpc_ret = apl_gmtime(li64_time, &ls_tm);
    CPPUNIT_ASSERT_MESSAGE("apl_gmtime", &ls_tm == lpc_ret);

    PrintTm(&ls_tm);

    // format to string (strf)
    li_ret = apl_strftime(
        lac_strf, 
        sizeof(lac_strf),
        lac_fmt, 
        &ls_tm);

    CPPUNIT_ASSERT_MESSAGE(
        "apl_strftime() FAILED",
        li_ret > 0 && strlen(lac_strf) == (apl_size_t)li_ret);

    apl_errprintf("apl_strftime: %s", lac_strf);

    // parse from string (strf)
    lpc_ret = apl_strptime(lac_strf, lac_fmt, &ls_tm_strf);
    ASSERT_TM_EQUAL(ls_tm, ls_tm_strf);

    ////////////////////////////////////////////////////////////////////
    // format to string again (asc)
    lpc_ret = apl_asctime(&ls_tm_strf, lac_asc, sizeof(lac_asc));

    CPPUNIT_ASSERT_MESSAGE(
        "apl_asctime() FAILED",
        lac_asc == lpc_ret);

    apl_errprintf("apl_asctime: %s", lac_asc);

    ////////////////////////////////////////////////////////////////////
    // parse from string(asc)
    lpc_ret = apl_strptime(lac_asc, lac_fmt, &ls_tm_asc);

    ASSERT_TM_EQUAL(ls_tm, ls_tm_asc);

    ////////////////////////////////////////////////////////////////////
    // apl_strftime and apl_asctime should get the same result
    ASSERT_TM_EQUAL(ls_tm_strf, ls_tm_asc);
}

/**
 * @brief Test suite Time. 
 * @brief Including two test cases: Precision and Format.
 */
class CTestAplTime: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAplTime);
    CPPUNIT_TEST(testTimePrecision);
    CPPUNIT_TEST(testTimeFormat);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}

    void tearDown(void) {}

    /** 
     * Test case Precision.
     */
    void testTimePrecision()
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_time_t li64_time ;

        //case
        li64_time = apl_time();
        CPPUNIT_ASSERT_MESSAGE("apl_time() % APL_TIME_SEC", 
                li64_time % APL_TIME_SEC == 0);

        li64_time  = apl_gettimeofday(); 
        CPPUNIT_ASSERT_MESSAGE("apl_gettimeofday() % APL_TIME_USEC", 
                li64_time % APL_TIME_USEC == 0);

        li64_time  = apl_clock_gettime(); 
        CPPUNIT_ASSERT_MESSAGE("apl_clock_gettime() % APL_TIME_NSEC", 
                li64_time % APL_TIME_NSEC == 0);

        //end environment
    }

    /** 
     * Test case Format. The convertion between tm, time_t and string.  
     */
    void testTimeFormat()
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_int_t li_i;

        //case
        LocalTimeFormat("apl_time()", apl_time()); 
        GmTimeFormat("apl_time()", apl_time()); 

        LocalTimeFormat("apl_gettimeofday()", apl_gettimeofday()); 
        GmTimeFormat("apl_gettimeofday()", apl_gettimeofday()); 

        LocalTimeFormat("apl_clock_gettime()", apl_clock_gettime());
        GmTimeFormat("apl_clock_gettime()", apl_clock_gettime());

        LocalTimeFormat("NULL", 0);
        GmTimeFormat("NULL", 0);

        LocalTimeFormat("APL_TIME_SEC", APL_TIME_SEC);
        GmTimeFormat("APL_TIME_SEC", APL_TIME_SEC);

        for (li_i=0; li_i<5000; li_i++)
        {
            LocalTimeFormat("APL_TIME_MIN", li_i * APL_TIME_MIN);
            GmTimeFormat("APL_TIME_MIN", li_i * APL_TIME_MIN);
        }

        for (li_i=0; li_i<500; li_i++)
        {
            LocalTimeFormat("APL_TIME_HOUR", li_i * APL_TIME_HOUR);
            GmTimeFormat("APL_TIME_HOUR", li_i * APL_TIME_HOUR);
        }

        LocalTimeFormat("APL_TIME_DAY", APL_TIME_DAY);
        GmTimeFormat("APL_TIME_DAY", APL_TIME_DAY);

        LocalTimeFormat("APL_TIME_WEEK", APL_TIME_WEEK);
        GmTimeFormat("APL_TIME_WEEK", APL_TIME_WEEK);

        //end environment
    }
};

/* ---------------------------------------------------------------------- */

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAplTime);

/* ---------------------------------------------------------------------- */

int main()
{
    RUN_ALL_TEST(__FILE__);
}

