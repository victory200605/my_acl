#include "Test.h"

/**
 * @brief Test suite FS.
 * @brief Including two test cases: Link and FK.
 */
class CTestaplfs: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplfs);
    CPPUNIT_TEST(testLink);
    CPPUNIT_TEST(testFk);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}

    /**
     * Test case Link involved link, lstat, unlink, symlink and readlink.
     */
    void testLink(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        char lac_link[1024];
        struct apl_stat_t ls_st;
        apl_int_t li_ir;

        //case
        RUN_AND_CHECK_RET_INT(apl_link, ("Makefile", "testfs_link"));
        RUN_AND_CHECK_RET_INT(apl_lstat, ("testfs_link", &ls_st));
        RUN_AND_CHECK_RET_INT(apl_unlink, ("testfs_link"));
        RUN_AND_CHECK_RET_INT(apl_symlink, ("Makefile", "testfs_link"));
        RUN_AND_CHECK_RET_INT(apl_lstat, ("testfs_link", &ls_st));

        li_ir = apl_readlink("testfs_link", lac_link, sizeof(lac_link)-1 );

        if ( li_ir >= 0 ) 
        {
            lac_link[li_ir] ='\0';
        }

        PrintErrMsg("apl_readlink", li_ir);

        RUN_AND_CHECK_RET_INT(apl_unlink, ("testfs_link"));

        //end environment
    }

    /**
     * Test case Link involved fstat, access, stat and realpath.
     */
    void testFk(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        struct apl_stat_t ls_st;
        char lac_link[1024];
        apl_int_t li_ifd;

        li_ifd = apl_open("testf1", APL_O_RDWR |APL_O_CREAT, 0755);
        PrintErrMsg("apl_open", li_ifd);

        //case
        RUN_AND_CHECK_RET_INT(apl_fstat, (li_ifd, &ls_st));
        RUN_AND_CHECK_RET_INT(apl_access, ("testf1", APL_F_OK));
        RUN_AND_CHECK_RET_INT(apl_stat, ("testf1", &ls_st));

        if (apl_realpath("testf1", lac_link, sizeof(lac_link) ) == NULL)
        {
            CPPUNIT_FAIL("apl_realpath failed");
        }

        //end environment
        RUN_AND_CHECK_RET_INT(apl_unlink, ("testf1"));
        apl_close(li_ifd);
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplfs);


int main()
{
    RUN_ALL_TEST(__FILE__);
}
