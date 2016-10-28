#include "Test.h"


void TestFileCommon(char const* apc_filename)
{
    apl_int_t li_ir;
    apl_int_t li_in; 
    apl_int_t li_ifd;
    char lac_chr[16] ="1234567890abcde";
    char lac_buf[16];
    apl_int_t li_ic = APL_NULL;

    li_ifd = apl_open(apc_filename, APL_O_RDWR|APL_O_CREAT|APL_O_TRUNC, 0755);
    PrintErrMsg("apl_open", li_ifd);

    li_in = 0;
    while (li_in < 10)
    {
        RUN_AND_CHECK_RET_INT(apl_pwrite, (li_ifd, (void*)lac_chr, sizeof(lac_chr), li_in * sizeof(lac_chr)));

        RUN_AND_CHECK_RET_INT(apl_fsync, (li_ifd));
        
        apl_sync();

        li_in++;
    }

    while (li_in)
    {
        RUN_AND_CHECK_RET_INT(apl_pread, (li_ifd,(void*)lac_buf,sizeof(lac_buf),li_in*sizeof(lac_buf)));

        li_in--;
    }

    RUN_AND_CHECK_RET_INT(apl_ftruncate, (li_ifd,20));

    RUN_AND_CHECK_RET_INT(apl_lseek, (li_ifd,0,APL_SEEK_END));

    RUN_AND_CHECK_RET_INT(apl_lseek, (li_ifd,0,APL_SEEK_CUR));

    RUN_AND_CHECK_RET_INT(apl_lseek, (li_ifd,0,APL_SEEK_SET));

    RUN_AND_CHECK_RET_INT(apl_close, (li_ifd));

    RUN_AND_CHECK_RET_INT(apl_remove, (apc_filename));
}

/**
 * @brief Test suite File
 * @brief Including four test cases: Common, Access, Allocate and Advise
 */
class CTestaplfile: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplfile);
    CPPUNIT_TEST(testCommon);
    CPPUNIT_TEST(testAccess);
    CPPUNIT_TEST(testAllocate);
    CPPUNIT_TEST(testAdvise);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    /**
     * Test case Common involved open, pwrite, pread, fsync, sync, 
     * ftruncate, lseek and remove.
     */
    void testCommon(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        TestFileCommon("testfile1");
        TestFileCommon("testfile2");    

        //end environment
    }

    /**
     * Test case Access involved access.
     */
    void testAccess(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        system("touch foo.tmp.dat");

        //case
        RUN_AND_CHECK_RET_INT(apl_access, ("foo.tmp.dat", F_OK));
        RUN_AND_CHECK_RET_INT(apl_access, ("foo.tmp.dat", R_OK|W_OK));

        //end environment
    }

    /**
     * Test case Access involved fallocate.
     */
    void testAllocate(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_int_t li_ifd3;
        apl_int_t li_ifd4;

        li_ifd3 = apl_open("testfile3", APL_O_RDWR|APL_O_CREAT|APL_O_TRUNC,0755);
        PrintErrMsg("apl_open", li_ifd3);
        li_ifd4 = apl_open("testfile4", APL_O_RDWR|APL_O_CREAT|APL_O_TRUNC,0755);
        PrintErrMsg("apl_open", li_ifd4);

        //case
        RUN_AND_CHECK_RET_INT(apl_fallocate, (li_ifd3, 0, 10));
        RUN_AND_CHECK_RET_INT(apl_fallocate, (li_ifd4, 0, 1));

        //end environment
        RUN_AND_CHECK_RET_INT(apl_close, (li_ifd3));
        RUN_AND_CHECK_RET_INT(apl_remove, ("testfile3"));
        RUN_AND_CHECK_RET_INT(apl_close, (li_ifd4));
        RUN_AND_CHECK_RET_INT(apl_remove, ("testfile4"));
    }

    /**
     * Test case Access involved fadvise.
     */
    void testAdvise(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_int_t li_ifd5;
        apl_int_t li_ifd6;
        li_ifd5 = apl_open("testfile5", APL_O_RDWR|APL_O_CREAT|APL_O_TRUNC,0755);
        PrintErrMsg("apl_open", li_ifd5);
        li_ifd6 = apl_open("testfile6", APL_O_RDWR|APL_O_CREAT|APL_O_TRUNC,0755);
        PrintErrMsg("apl_open", li_ifd6);

        //case
        RUN_AND_CHECK_RET_INT(apl_fadvise, (li_ifd5, 0, 0, APL_FADV_NORMAL));
        RUN_AND_CHECK_RET_INT(apl_fadvise, (li_ifd6, 0, 0, APL_FADV_SEQUENTIAL));

        //end environment
        RUN_AND_CHECK_RET_INT(apl_close, (li_ifd5));
        RUN_AND_CHECK_RET_INT(apl_remove, ("testfile5"));
        RUN_AND_CHECK_RET_INT(apl_close, (li_ifd6));
        RUN_AND_CHECK_RET_INT(apl_remove, ("testfile6"));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplfile);


int main()
{
    RUN_ALL_TEST(__FILE__);
}
