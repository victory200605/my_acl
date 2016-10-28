#include "Test.h"

// 2*set == get only happen in linux
#define TEST_OPT_INT(gname, sname, fd, value, kind) \
    { \
        apl_int_t li_n =0; \
        RUN_AND_CHECK_RET_INT(sname, (fd,value)); \
        RUN_AND_CHECK_RET_INT(gname, (fd, &li_n)); \
        CPPUNIT_ASSERT(value == li_n || 2*(value) == li_n); \
    }

#define TEST_OPT_LINGER(gname, sname, fd, onff, ling, kind) \
    { \
        apl_int_t li_onoff =0; \
        apl_int_t li_li    =0; \
        RUN_AND_CHECK_RET_INT(sname, (fd, onff, ling)); \
        RUN_AND_CHECK_RET_INT(gname, (fd, &li_onoff, &li_li)); \
        CPPUNIT_ASSERT(onff==li_onoff && ling==li_li); \
    }

/**
 * @brief Test suite Socketopt.
 * @brief Including nine test cases: SO_BROADCAST, SO_DEBUG, SO_DONTROUTE,
 * SO_KEEPALIVE,SO_RCVBUF,SO_SNDBUF,SO_REUSEADDR,TCP_NODELAY and SO_LINGER.
 */
class CTestaplSocketopt: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplSocketopt);
    CPPUNIT_TEST(testBroad);
    CPPUNIT_TEST(testDebug);
    CPPUNIT_TEST(testDontrt);
    CPPUNIT_TEST(testKeeplive);
    CPPUNIT_TEST(testRcvbuf);
    CPPUNIT_TEST(testSndbuf);
    CPPUNIT_TEST(testReaddr);
    CPPUNIT_TEST(testTcplay);
    CPPUNIT_TEST(testLinger);

    CPPUNIT_TEST_SUITE_END();
private:
    int mi_tcp;
    int mi_udp;
public:
    void setUp(void) 
    {
        mi_tcp = socket(APL_AF_INET, APL_SOCK_STREAM, 0);
        mi_udp = socket(APL_AF_INET, APL_SOCK_DGRAM , 0);
    }

    void tearDown(void) 
    {
        apl_close(mi_tcp);
        apl_close(mi_udp);
    }

    /** 
     * Test case SO_BROADCAST.
     */
    void testBroad(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_OPT_INT(apl_getsockopt_broadcast, apl_setsockopt_broadcast, mi_tcp, 0, tcp)
        TEST_OPT_INT(apl_getsockopt_broadcast, apl_setsockopt_broadcast, mi_udp, 0, udp)
    }

    /** 
     * Test case SO_DEBUG.
     */
    void testDebug(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_OPT_INT(apl_getsockopt_debug, apl_setsockopt_debug, mi_tcp, 0, tcp)
        TEST_OPT_INT(apl_getsockopt_debug, apl_setsockopt_debug, mi_udp, 0, udp)
    }

    /** 
     * Test case SO_DONTROUTE.
     */
    void testDontrt(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_OPT_INT(apl_getsockopt_dontroute, apl_setsockopt_dontroute, mi_tcp, 0, tcp)
        TEST_OPT_INT(apl_getsockopt_dontroute, apl_setsockopt_dontroute, mi_udp, 0, udp)
    }

    /** 
     * Test case SO_KEEPALIVE.
     */
    void testKeeplive(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_OPT_INT(apl_getsockopt_keepalive, apl_setsockopt_keepalive, mi_tcp, 0, tcp)
    }

    /** 
     * Test case SO_RCVBUF.
     */
    void testRcvbuf(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_OPT_INT(apl_getsockopt_rcvbuf, apl_setsockopt_rcvbuf, mi_tcp, 65536, tcp)
        TEST_OPT_INT(apl_getsockopt_rcvbuf, apl_setsockopt_rcvbuf, mi_udp, 65536, udp)
    }

    /** 
     * Test case SO_SNDBUF.
     */
    void testSndbuf(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_OPT_INT(apl_getsockopt_sndbuf, apl_setsockopt_sndbuf, mi_tcp, 65536, tcp) 
        TEST_OPT_INT(apl_getsockopt_sndbuf, apl_setsockopt_sndbuf, mi_udp, 65536, udp) 
    }

    /** 
     * Test case SO_REUSEADDR.
     */
    void testReaddr(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_OPT_INT(apl_getsockopt_reuseaddr, apl_setsockopt_reuseaddr, mi_tcp, 0, tcp)
        TEST_OPT_INT(apl_getsockopt_reuseaddr, apl_setsockopt_reuseaddr, mi_udp, 0, udp)
    } 

    /** 
     * Test case TCP_NODELAY.
     */
    void testTcplay(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_OPT_INT(apl_getsockopt_tcpnodelay, apl_setsockopt_tcpnodelay, mi_tcp, 0, tcp)
    } 

    /** 
     * Test case SO_LINGER.
     */
    void testLinger(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_OPT_LINGER(apl_getsockopt_linger, apl_setsockopt_linger, mi_tcp, 0, 0, tcp)   
        TEST_OPT_LINGER(apl_getsockopt_linger, apl_setsockopt_linger, mi_udp, 0, 0, udp)   
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplSocketopt);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
