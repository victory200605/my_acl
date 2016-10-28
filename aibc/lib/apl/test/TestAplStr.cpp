#include "Test.h"
#include <algorithm>


char const* gpc_str = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

#define GPCSTR_LEN strlen(gpc_str)

/**
 * @brief Test suite Str.
 * @brief Including seven test cases: strdup, strncpy, strcmp, strcasecmp, 
 * strchr, strcat and snprintf.
 */
class CTestaplstr: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplstr);
    CPPUNIT_TEST(testDup);
    CPPUNIT_TEST(testCpy);
    CPPUNIT_TEST(testCmp);
    CPPUNIT_TEST(testCasecmp);
    CPPUNIT_TEST(testChr);
    CPPUNIT_TEST(testCat);
    CPPUNIT_TEST(testPrt);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}

    void tearDown(void) {}

    /** 
     * Test case strdup.
     */
    void testDup(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        char *lpc_str = NULL;

        //case 
        lpc_str = apl_strdup(gpc_str);

        CPPUNIT_ASSERT_MESSAGE("apl_strdup failed", lpc_str != NULL);
        CPPUNIT_ASSERT_MESSAGE("apl_strdup failed", strcmp(lpc_str, gpc_str) == 0);

        //end environment
        delete lpc_str;
    }

    /** 
     * Test case strncpy
     */
    void testCpy(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        char lac_buf[GPCSTR_LEN+1];
        apl_size_t li_itr;

        lac_buf[0] = '1';
        lac_buf[1] = '\0';
        apl_strncpy(lac_buf,  "", sizeof(lac_buf));
        CPPUNIT_ASSERT_MESSAGE("apl_strncpy failed", !memcmp(lac_buf, "\0\0", 2));
 
        lac_buf[0] = '1';
        lac_buf[1] = '\0';
        apl_strncpy(lac_buf,  "", 0);
        CPPUNIT_ASSERT_MESSAGE("apl_strncpy failed", !memcmp(lac_buf, "1\0", 2));
 
        lac_buf[0] = '1';
        lac_buf[1] = '\0';
        apl_strncpy(lac_buf,  "2", 0);
        CPPUNIT_ASSERT_MESSAGE("apl_strncpy failed", !memcmp(lac_buf, "1\0", 2));
 
        //case
        for (li_itr = 0; li_itr < sizeof(lac_buf); ++li_itr)
        {
            apl_strncpy(lac_buf,  gpc_str, li_itr+1);
            CPPUNIT_ASSERT_MESSAGE("apl_strncpy failed", 
                    !strncmp(lac_buf, gpc_str, li_itr));
        } 

        //end environment
    }

    /** 
     * Test case strcmp
     */
    void testCmp(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        char lac_buf[GPCSTR_LEN+1];
        apl_strncpy(lac_buf,  gpc_str, GPCSTR_LEN+1);

        //case
        CPPUNIT_ASSERT_MESSAGE("apl_strcmp failed", 
                apl_strcmp(lac_buf, gpc_str) == strcmp(lac_buf, gpc_str));

        lac_buf[GPCSTR_LEN] = 0;
        std::generate_n(
            lac_buf,
            sizeof(lac_buf)-1,
            rand);

        CPPUNIT_ASSERT_MESSAGE("apl_strcmp failed", 
                apl_strcmp(lac_buf, gpc_str) == strcmp(lac_buf, gpc_str));
    }

    /** 
     * Test case strcasecmp.
     */
    void testCasecmp(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        char lpc_src[4]="abc";
        char lpc_src1[4]="Abc";
        char lpc_src2[4]="ABC";
        char lpc_src3[4]="aDc";

        //case
        CPPUNIT_ASSERT_MESSAGE("apl_strcasecmp failed", 
                apl_strcasecmp(lpc_src, lpc_src1) == strcasecmp(lpc_src, lpc_src1));

        CPPUNIT_ASSERT_MESSAGE("apl_strcasecmp failed", 
                apl_strcasecmp(lpc_src, lpc_src2) == strcasecmp(lpc_src, lpc_src2));

        CPPUNIT_ASSERT_MESSAGE("apl_strcasecmp failed", 
                apl_strcasecmp(lpc_src, lpc_src3) == strcasecmp(lpc_src, lpc_src3));

        CPPUNIT_ASSERT_MESSAGE("apl_strncasecmp failed", 
                apl_strncasecmp(lpc_src, lpc_src1, 3)
                == strncasecmp(lpc_src, lpc_src1, 3));

        CPPUNIT_ASSERT_MESSAGE("apl_strncasecmp failed", 
                apl_strncasecmp(lpc_src, lpc_src2, 1)
                == strncasecmp(lpc_src, lpc_src2, 1));

        CPPUNIT_ASSERT_MESSAGE("apl_strncasecmp failed", 
                apl_strncasecmp(lpc_src, lpc_src2, 2)
                == strncasecmp(lpc_src, lpc_src2, 2));

        CPPUNIT_ASSERT_MESSAGE("apl_strncasecmp failed", 
                apl_strncasecmp(lpc_src, lpc_src2, 3)
                == strncasecmp(lpc_src, lpc_src2, 3));

        CPPUNIT_ASSERT_MESSAGE("apl_strncasecmp failed", 
                apl_strncasecmp(lpc_src, lpc_src3, 1)
                == strncasecmp(lpc_src, lpc_src3, 1));

        //end environment
    }

    /** 
     * Test case strchr.
     */
    void testChr(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        CPPUNIT_ASSERT_MESSAGE("apl_strchr failed", 
                strchr(gpc_str, '2') == apl_strchr(gpc_str, '2'));
        CPPUNIT_ASSERT_MESSAGE("apl_strchr failed", 
                strchr(gpc_str, 'A') == apl_strchr(gpc_str, 'A'));

        CPPUNIT_ASSERT_MESSAGE("apl_strchr failed", 
                strrchr(gpc_str, 'a') == apl_strrchr(gpc_str, 'a'));
        CPPUNIT_ASSERT_MESSAGE("apl_strchr failed", 
                strrchr(gpc_str, 'A') == apl_strrchr(gpc_str, 'A'));

        //end environment
    }

    /** 
     * Test case strncat including three sub-cases.
     */
    void testCat(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        char lac_buf[GPCSTR_LEN+1];
        apl_size_t li_itr; 

        //case
        ///sub-case1 when the origial str is 0
        apl_memset(lac_buf, 0, sizeof(lac_buf));
        apl_strncat(lac_buf,  gpc_str, sizeof(lac_buf));
        CPPUNIT_ASSERT_MESSAGE("apl_strncat failed", 
                !strcmp(lac_buf, gpc_str));

        ///sub-case2 when the origial str is full with buf
        apl_memset(lac_buf, '1', sizeof(lac_buf)-1);
        apl_strncat(lac_buf,  gpc_str, sizeof(lac_buf));
        CPPUNIT_ASSERT_MESSAGE("apl_strncat failed", 
                strlen(lac_buf) == sizeof(lac_buf)-1 && lac_buf[GPCSTR_LEN] == 0);

        for (li_itr=0; li_itr<sizeof(lac_buf)-1; li_itr++)
        {
            CPPUNIT_ASSERT_MESSAGE("apl_strncat failed", 
                    lac_buf[li_itr] == '1'); 
        }

        ///sub-case3 normal case
        apl_memset(lac_buf, 0, sizeof(lac_buf));
        apl_memset(lac_buf, '2', sizeof(lac_buf)/2);
        apl_strncat(lac_buf,  gpc_str, sizeof(lac_buf));
        char *lpc_half = lac_buf+sizeof(lac_buf)/2;
        //the later half should same as the first half of gpc_str now
        CPPUNIT_ASSERT_MESSAGE("apl_strncat failed", 
                !strncmp(lpc_half, gpc_str, lac_buf+sizeof(lac_buf)-lpc_half-1));
        for (li_itr=0; li_itr<sizeof(lac_buf)/2; li_itr++)
        {
            CPPUNIT_ASSERT_MESSAGE("apl_strncat failed", 
                    lac_buf[li_itr] == '2'); 
        }

        //end environment 
    }

    /** 
     * Test case snprintf.
     */
    void testPrt(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        char        lac_buf1[GPCSTR_LEN+10]; 
        char        lac_buf2[GPCSTR_LEN+10];
        apl_size_t   li_itr;
        apl_int_t   li_ret[3];

        //case
        //sub-case1 dest_str not NULL, len from 0 to max 
        li_ret[2] = snprintf(lac_buf1, 2, gpc_str);
        
        for (li_itr=2; li_itr<=sizeof(lac_buf1); li_itr++)
        {
            apl_memset(lac_buf1, 0, sizeof(lac_buf1));
            apl_memset(lac_buf2, 0, sizeof(lac_buf2));

            li_ret[0] = apl_snprintf(lac_buf1, li_itr, gpc_str); 
            li_ret[1] = snprintf(lac_buf2, li_itr, gpc_str); 

            apl_errprintf("apl_snprintf(notNull, %d, gpc_str) = %d\n", 
                    li_itr, li_ret[0]);

            CPPUNIT_ASSERT_MESSAGE("apl_snprintf failed", 
                    li_ret[0]==li_ret[1] && li_ret[1]==li_ret[2]);

            CPPUNIT_ASSERT_MESSAGE("apl_snprintf failed", 
                    !strcmp(lac_buf1, lac_buf2));
        }

        //sub-case2 dest_str not NULL, when len = 0 len = 1
        li_ret[1] = snprintf(lac_buf1, 2, gpc_str);

        for (li_itr=0; li_itr<=1; li_itr++)
        {
            li_ret[0] = apl_snprintf(lac_buf1, li_itr, gpc_str); 
            apl_errprintf("apl_snprintf(notNull, %d, gpc_str) = %d\n", 
                    li_itr, li_ret[0]);
            CPPUNIT_ASSERT_MESSAGE("apl_snprintf failed", 
                    li_ret[0]==li_ret[1]);
        }
        
        //sub-case3 dest_str NULL, when len == 0, it would always return the sizeof Format.

        apl_errprintf("strlen(gpc_str) = %d\ngpc_str = %s\n", 
                strlen(gpc_str), gpc_str);

        li_ret[0] = apl_snprintf(lac_buf1, 2, gpc_str); 
        li_ret[1] = apl_snprintf(NULL, 0, gpc_str); 
        li_ret[2] = snprintf(lac_buf1, 2, gpc_str); 

        apl_errprintf("apl_snprintf(Null, 0, gpc_str) = %d\n", 
                li_ret[1]);
        CPPUNIT_ASSERT_MESSAGE("apl_snprintf failed", 
                    li_ret[0]==li_ret[1] && li_ret[1]==li_ret[2]);

        //sub-case4 dest_str NULL, when len != 0, it would always return -1
        for (li_itr=1; li_itr<=sizeof(lac_buf1); li_itr++)
        {
            li_ret[0] = apl_snprintf(NULL, li_itr, gpc_str);
            apl_errprintf("apl_snprintf(0, %d, gpc_str) = %d\n", 
                    li_itr, li_ret[0]);
            CPPUNIT_ASSERT_MESSAGE("apl_snprintf failed", 
                    li_ret[0]== -1);
        }

        //end environment
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplstr);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
