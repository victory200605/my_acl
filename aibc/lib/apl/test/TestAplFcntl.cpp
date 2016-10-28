#include "Test.h"

/**
 * @brief Test suite fcntl
 * @brief Including four test cases: rdlock/wrlock, setfd/getfd, 
 * setfl/getfl and setown/getown.
 */
class CTestaplfcntl: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplfcntl);
    CPPUNIT_TEST(testLock);
    CPPUNIT_TEST(testFdfd);
    CPPUNIT_TEST(testFdfl);
    CPPUNIT_TEST(testFdown);
    CPPUNIT_TEST_SUITE_END();
private:
    apl_handle_t mi_ifd;

public:
   
    void setUp(void) 
    {
        mi_ifd =  apl_open("testfcnt", O_RDWR | O_CREAT | O_EXCL, 0755);
    }
    void tearDown(void)
    {
        close(mi_ifd);
        apl_remove("testfcnt");
    }

    /**
     * Test case rdlock/wrlock and unlock.
     */
    void testLock(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_int_t li_time = 2;

        //case
        while (li_time--)
        {
            RUN_AND_CHECK_RET_INT(apl_fcntl_rdlock, (mi_ifd, SEEK_SET, 0, 0, APL_NULL));
            RUN_AND_CHECK_RET_INT(apl_fcntl_unlock, (mi_ifd, SEEK_SET, 0, 0));

            RUN_AND_CHECK_RET_INT(apl_fcntl_wrlock, (mi_ifd, SEEK_SET, 0, 0, APL_NULL));
            RUN_AND_CHECK_RET_INT(apl_fcntl_unlock, (mi_ifd, SEEK_SET, 0, 0));

            RUN_AND_CHECK_RET_INT(apl_fcntl_rdlock, (mi_ifd, SEEK_SET, 0, 0, 1));
            RUN_AND_CHECK_RET_INT(apl_fcntl_unlock, (mi_ifd, SEEK_SET, 0, 0));

            RUN_AND_CHECK_RET_INT(apl_fcntl_wrlock, (mi_ifd, SEEK_SET, 0, 0, 1));
            RUN_AND_CHECK_RET_INT(apl_fcntl_unlock, (mi_ifd, SEEK_SET, 0, 0));
        }

        //end environment
    }
    
    /**
     * Test case setfd and getfd.
     */
    void testFdfd(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_int_t li_ifd = APL_NULL;

        //case
        RUN_AND_CHECK_RET_INT(apl_fcntl_setfd, (mi_ifd, O_WRONLY)); 
        RUN_AND_CHECK_RET_INT(apl_fcntl_getfd, (mi_ifd, &li_ifd));

        RUN_AND_CHECK_RET_INT(apl_fcntl_setfd, (mi_ifd, O_RDWR)); 
        RUN_AND_CHECK_RET_INT(apl_fcntl_getfd, (mi_ifd, &li_ifd));

        //end environment
    }
    
    
    /**
     * Test case setfl and getfl.
     */
    void testFdfl(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_int_t li_ifd = APL_NULL;

        //case
        RUN_AND_CHECK_RET_INT(apl_fcntl_setfl, (mi_ifd, APL_O_NONBLOCK|APL_O_APPEND));
        RUN_AND_CHECK_RET_INT(apl_fcntl_getfl, (mi_ifd, &li_ifd));

        RUN_AND_CHECK_RET_INT(apl_fcntl_setfl, (mi_ifd, O_SYNC));
        RUN_AND_CHECK_RET_INT(apl_fcntl_getfl, (mi_ifd, &li_ifd));

        //end environment
    }
    
    /**
     * Test case setown and getown
     */
    void testFdown(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_int_t li_ifd = APL_NULL;
        apl_int_t li_fd = apl_socket(APL_AF_INET, APL_SOCK_STREAM, 0);

        //case
        RUN_AND_CHECK_RET_INT(apl_fcntl_setown, (li_fd, apl_getpid()));
        RUN_AND_CHECK_RET_INT(apl_fcntl_getown, (li_fd, &li_ifd));
        apl_close(li_fd);

        li_fd = apl_socket(APL_AF_UNIX, APL_SOCK_STREAM, 0);
        RUN_AND_CHECK_RET_INT(apl_fcntl_setown, (li_fd, apl_getpid()));
        RUN_AND_CHECK_RET_INT(apl_fcntl_getown, (li_fd, &li_ifd));

        //end environment
        apl_close(li_fd);
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplfcntl);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
