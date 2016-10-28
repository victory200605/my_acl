#include "Test.h"

/**
 * @brief Test suite DIR operation.
 * @brief Including two test cases: 
 * 1)opendir/closedir, readdir/seekdir
 * 2)mkdir/rmdir.
 */
class CTestapldir: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestapldir);
    CPPUNIT_TEST(testOprd);
    CPPUNIT_TEST(testMkrm);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}

    void tearDown(void) {}

    /**
     * Test case opendir/closedir and readdir/seekdir.
     */
    void testOprd(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_dir_t ldir;

        //case
        RUN_AND_CHECK_RET_ERRNO(apl_opendir, (&ldir,".") );

        while( true )
        {
            if ( APL_NULL == apl_readdir(&ldir) )
            {
                break;
            }
        }

        RUN_AND_CHECK_RET_INT(apl_seekdir, (&ldir, apl_telldir(&ldir)) );

        if ( ldir.mps_dir != NULL )
        {
            RUN_AND_CHECK_RET_INT(apl_closedir, (&ldir) );
        }

        //end environment
    }

    /**
     * Test case mkdir and rmdir.
     */
    void testMkrm()
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        RUN_AND_CHECK_RET_INT(apl_mkdir, ("ttt", 0777) );

        RUN_AND_CHECK_RET_INT(apl_rmdir, ("ttt") );

        //end environment
    }
};



CPPUNIT_TEST_SUITE_REGISTRATION(CTestapldir);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
