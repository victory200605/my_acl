#include "Test.h"


/**
 * @brief Test suite IO
 * @brief Including three test cases: readwrite, readwritev and dup
 */
class CTestaplio: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplio);
    CPPUNIT_TEST(testrdwr);
    CPPUNIT_TEST(testrdwrv);
    CPPUNIT_TEST(testdup);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}

    /** 
     * Test case readwrite.
     */
    void testrdwr(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        testRDWR("testio");

        //end environment
    }

    void testrdwrv(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case 
        testRDWRv("testio_v");

        //end environment
    }
    void testdup(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case 
        testDup(APL_STDOUT_FILENO, APL_STDERR_FILENO);

        //end environment
    }

    ///called by testrdwr
    void testRDWR(const char* apc_filename)
    {
        PRINT_TITLE_2(__func__);
        apl_int_t li_ir = APL_NULL ;
        apl_int_t li_ifd = apl_open(apc_filename, APL_O_RDWR |APL_O_CREAT|APL_O_TRUNC, 0755);
        if ( li_ifd <= 0)
        {
             return ;
        }
        char lac_chr[16] ="1234567890abcde";
        char lac_des[16] = {0};
        apl_int_t li_in = APL_NULL;
        bool lb_flag = true;
        /*
         * if p==NULL then core???
        */
        char *lpc_c =NULL;
        apl_write(li_ifd, lpc_c, 0);
        apl_write(li_ifd, lpc_c, 2);

        apl_lseek(li_ifd, 0, APL_SEEK_SET);

        while( li_in < 10 && lb_flag )
        {
            li_ir = apl_write(li_ifd, lac_chr, sizeof(lac_chr) );
            if ( li_ir < 0 && apl_get_errno() != APL_EINTR )
            {
               lb_flag = false;
            }
            if ( li_ir < 0)
            {
               continue;
            }
            li_in++; 
        }

        apl_lseek(li_ifd, 0, APL_SEEK_SET);

        while( lb_flag )
        {
            li_ir = apl_read(li_ifd, lac_des, sizeof(lac_des) );
            if ( li_ir < 0 && apl_get_errno() != APL_EINTR )
            {
                lb_flag = false;
            }
            if ( li_ir == 0 )
            {
                break; 
            }
            li_in--; 
        }

        CPPUNIT_ASSERT(!li_in);
        apl_close(li_ifd);
        apl_unlink(apc_filename); 
    }

    ///called by testrdwrv
    void testRDWRv(const char* apc_filename)
    {
        PRINT_TITLE_2(__func__);
        apl_int_t li_ir = APL_NULL;
        apl_int_t li_ifd = apl_open(apc_filename, APL_O_RDWR |APL_O_CREAT|APL_O_TRUNC, 0755);
        if ( li_ifd <= 0 )
        {
           return ;
        }
        char lac_chr[16] ="1234567890abcde";
        char lac_des[16] ;
        apl_int_t li_in = APL_NULL;

        struct apl_iovec_t lav_iov1[1], lav_iov2[1];
        lav_iov1[0].iov_base = lac_chr;
        lav_iov1[0].iov_len = sizeof(lac_chr);
        
        lav_iov2[0].iov_base = lac_des;
        lav_iov2[0].iov_len  = sizeof(lac_des); 
        bool lb_flag = true;
        while( li_in < 10 && lb_flag )
        {
            li_ir = apl_writev(li_ifd, lav_iov1, sizeof(lav_iov1)/sizeof(lav_iov1[0]) );
            if ( li_ir < 0 && apl_get_errno() != APL_EINTR )
            {
               lb_flag = false;
            }
            if ( li_ir < 0)
            {
               continue;
            }
            li_in++;
        }

        apl_lseek(li_ifd, 0, APL_SEEK_SET);

        while( lb_flag )
        {
            li_ir = apl_readv(li_ifd, lav_iov2, sizeof(lav_iov2)/sizeof(lav_iov2[0]) );
            if ( li_ir <= 0 && apl_get_errno() != APL_EINTR )
            {
                lb_flag = false;
            }
            if ( li_ir <= 0 )
            {
                continue;
            }
            li_in--;
        }

        CPPUNIT_ASSERT(!li_in);
        apl_close(li_ifd);
        apl_unlink(apc_filename);
    }

    ///call by testdup
    void testDup(apl_int_t ai_fd, apl_int_t ai_fd2)
    {
         char lac_src[16] ="1234567890abcd\n";
         apl_int_t li_ifd = apl_dup(ai_fd);

         PrintErrMsg("apl_dup", li_ifd);
         apl_write(li_ifd, lac_src, sizeof(lac_src));

         ai_fd2 = apl_dup2(ai_fd, ai_fd2);
         PrintErrMsg("apl_dup", ai_fd2);
         apl_write(ai_fd2, lac_src, sizeof(lac_src)); 
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplio);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
