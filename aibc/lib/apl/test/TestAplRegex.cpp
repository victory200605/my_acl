#include "Test.h"
#include "apl/regex.h"

///call by testRegex
void regexRun(const char* apcStr, const char* apcFmt, apl_int_t aiFlag)
{
    apl_regex_t lt_reg;
    apl_regmatch_t lat_match[10];
    char lacBuf[128] ="";
    apl_int_t li_i = 0;
    apl_int_t li_r = apl_regcomp(&lt_reg, apcFmt, aiFlag);
    if ( li_r != 0 )
    {
        apl_regerror(li_r, &lt_reg, lacBuf, sizeof(lacBuf) );
        apl_errprintf(" %s pattern", lacBuf);
        return ;
    }
     
    li_r = apl_regexec(&lt_reg, apcStr, 10, lat_match, 0);
    if ( li_r == REG_NOMATCH || li_r != 0 )
    {
        return ;
    }

    regex_t  le_reg;
    regmatch_t lae_match[10];        
    li_r = regcomp(&le_reg, apcFmt, aiFlag);
    if ( li_r != 0 )
    {
        CPPUNIT_FAIL(" regex fail");
    }
    li_r = regexec(&le_reg, apcStr, 10, lae_match, 0);
    if ( li_r == REG_NOMATCH || li_r != 0 )
    {
        return ;
    }

    //compare the result between apl and posix. 
    for(li_i=0; li_i<10 && lat_match[li_i].rm_so != -1; ++li_i)
    {
        if ( lat_match[li_i].rm_so != lae_match[li_i].rm_so ||
             lat_match[li_i].rm_eo != lae_match[li_i].rm_eo )
        {
            CPPUNIT_FAIL(" regex fail");
        }
    }
    if ( li_i<10 && lae_match[li_i].rm_so != -1 )
    {
        CPPUNIT_FAIL("regex failed");
    }
    apl_regfree(&le_reg);
    apl_regfree(&lt_reg); 
}

/**
 * @brief Test suite Regex.
 */
class CTestaplregex: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplregex);
    CPPUNIT_TEST(testRegex);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}
    /**
     * Test case regex.
     */
    void testRegex(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        char lacStr[] = "regexec(abc,,,regexecvesef";
        char lacFmt[] = "regex[a-z]";

        //case
        regexRun(lacStr, lacFmt, REG_EXTENDED);

        //end environment
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplregex);
int main()
{
    RUN_ALL_TEST(__FILE__);
}
