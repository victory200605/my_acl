#include "Test.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sys/utsname.h>

#ifdef HAVE_SYS_IOCTL_H
#   include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_SOCKIO_H
#   include <sys/sockio.h>
#endif

extern "C" {
#include <net/if.h>
}

#define MCAST_PORT 12345
#define MCAST_GRP_ADDR "239.3.3.1"
#define MCAST_LOOP 3
#define MCAST_THREAD_NUM 3
#define MCAST_TIMEOUT 10

#define APL_MAXLINE 1024

apl_mutex_t gs_mtxlk;
volatile apl_int_t gu_data = MCAST_THREAD_NUM;

 
void threadRecvMcast(apl_sockaddr_storage_t ls_sas)
{
    apl_handle_t    li_skt_recv;
    apl_sockaddr_t *lps_mrecvaddr = APL_NULL;
    struct utsname ls_myname;
    apl_int_t li_n;
    char lca_line[APL_MAXLINE+1];
    char lca_buf[APL_MAXLINE+1];
    apl_int_t li_i;
    apl_sockaddr_t ls_from;
    apl_size_t li_len;

    lps_mrecvaddr = (apl_sockaddr_t *)apl_malloc(sizeof(ls_sas)); 
    apl_memcpy(lps_mrecvaddr, &ls_sas, sizeof(ls_sas));

    CPPUNIT_ASSERT_MESSAGE("uname failed", !(uname(&ls_myname) < 0));
    apl_snprintf(lca_buf, sizeof(lca_line), "%s, %d\n", ls_myname.nodename, getpid());

    li_skt_recv = apl_socket(APL_AF_INET, APL_SOCK_DGRAM, 0);
    CPPUNIT_ASSERT_MESSAGE("apl_socket failed", li_skt_recv > 0);
    CPPUNIT_ASSERT_MESSAGE("apl_setsockopt_reuseaddr failed", 0 == apl_setsockopt_reuseaddr(li_skt_recv, 1));

    CPPUNIT_ASSERT_MESSAGE("apl_bind failed", 0 == apl_bind(li_skt_recv, lps_mrecvaddr, sizeof(*lps_mrecvaddr)));;

    CPPUNIT_ASSERT_MESSAGE("apl_mcast_join failed", 0 == apl_mcast_join(li_skt_recv, 
                lps_mrecvaddr, sizeof(*lps_mrecvaddr), APL_NULL, 0));

    for (li_i=0; li_i<MCAST_LOOP; li_i++)
    {
        apl_errprintf("Thread %"APL_PRIu32": waitting message from mcast addr %s:%d\n", 
                apl_thread_self(), MCAST_GRP_ADDR, MCAST_PORT);
        memset(lca_line, 0, APL_MAXLINE+1);
        li_n = apl_recvfrom(li_skt_recv, 
                lca_line, 
                APL_MAXLINE, 
                0, 
                &ls_from, 
                &li_len);

        lca_line[li_n] = 0;

        CPPUNIT_ASSERT_MESSAGE("apl_recvfrom mcast group failed", 
                apl_strncmp(lca_line, lca_buf, apl_strlen(lca_buf)) == 0);
        apl_errprintf("Thread %"APL_PRIu32": recv message from mcast addr %s:%d [[message: %s\n", 
                apl_thread_self(), MCAST_GRP_ADDR, MCAST_PORT, lca_line);
    }

    apl_mutex_lock(&gs_mtxlk);
    gu_data --; 
    apl_mutex_unlock(&gs_mtxlk);

    CPPUNIT_ASSERT_MESSAGE("apl_mcast_leave failed", 
            0 == apl_mcast_leave(li_skt_recv, lps_mrecvaddr, sizeof(*lps_mrecvaddr)));

    apl_free(lps_mrecvaddr);
}

/** * @brief Test suite Mcast.
 * @brief Including two test cases: mcast join/leave and mcast set/get if.
 */
class CTestaplmcast: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplmcast);
    CPPUNIT_TEST(testMcastjoin);
    CPPUNIT_TEST(testMcastIf);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}

    void tearDown(void) {}

    /** 
     * Test case mcast join/leave. Only the AF_INET are tested here. 
     */
    void testMcastjoin(void)
    {
        //start case environment
        apl_handle_t    li_skt_send;
        apl_sockaddr_storage_t ls_sas;
        apl_sockaddr_t *lps_mcastaddr = APL_NULL;
        char lca_line[APL_MAXLINE];
        struct utsname ls_myname;
        apl_int_t li_i;
        apl_mutex_init(&gs_mtxlk);

        lps_mcastaddr = (apl_sockaddr_t*)(&ls_sas);

        CPPUNIT_ASSERT_MESSAGE("sockaddr_setfamily failed", 
                !(apl_sockaddr_setfamily(lps_mcastaddr, APL_AF_INET) < 0));
        CPPUNIT_ASSERT_MESSAGE("sockaddr_setaddr failed", 
                !(apl_sockaddr_setaddr(lps_mcastaddr, sizeof(*lps_mcastaddr), MCAST_GRP_ADDR) < 0));

        CPPUNIT_ASSERT_MESSAGE("sockaddr_setport failed", 
                !(apl_sockaddr_setport(lps_mcastaddr, sizeof(*lps_mcastaddr), MCAST_PORT) < 0));

        li_skt_send = apl_socket(APL_AF_INET, APL_SOCK_DGRAM, 0);
        CPPUNIT_ASSERT_MESSAGE("apl_socket failed", li_skt_send > 0);

        CPPUNIT_ASSERT_MESSAGE("apl_mcast_set_loop", apl_mcast_set_loop(li_skt_send, 1) == 0);

        CPPUNIT_ASSERT_MESSAGE("uname failed", !(uname(&ls_myname) < 0));
        apl_snprintf(lca_line, sizeof(lca_line), "%s, %d\n", ls_myname.nodename, getpid());

        //thread-1, recv the mcast packet and verified it.
        START_THREAD_BODY(recvThread1, apl_sockaddr_storage_t, ls_sas)
        {
            threadRecvMcast(ls_sas);
        }
        END_THREAD_BODY(recvThread1)

        //thread-2, recv the mcast packet and verified it.
        START_THREAD_BODY(recvThread2, apl_sockaddr_storage_t, ls_sas)
        {
            threadRecvMcast(ls_sas);
        }
        END_THREAD_BODY(recvThread2)

        //thread-3, recv the mcast packet and verified it.
        START_THREAD_BODY(recvThread3, apl_sockaddr_storage_t, ls_sas)
        {
            threadRecvMcast(ls_sas);
        }
        END_THREAD_BODY(recvThread3)

        //case
        RUN_THREAD_BODY(recvThread1)
        RUN_THREAD_BODY(recvThread2)
        RUN_THREAD_BODY(recvThread3)

        /* main thread send out the message */
        for (li_i=0; li_i<MCAST_LOOP; li_i++)
        {
            sleep(1);
            apl_errprintf("Thread %"APL_PRIu32": mcast to addr %s:%d [[message: %s\n", 
                    apl_thread_self(), MCAST_GRP_ADDR, MCAST_PORT, lca_line);
            apl_sendto(li_skt_send, 
                    lca_line, 
                    strlen(lca_line), 
                    0, 
                    lps_mcastaddr, 
                    sizeof(apl_sockaddr_t)); 
        }

        /* wating for all the thread end */
        apl_int_t li_timeout = 0;
        while (1) 
        {
            if (!gu_data || li_timeout > MCAST_TIMEOUT ) break; 
            sleep(1);
            li_timeout ++;
        }

        CPPUNIT_ASSERT_MESSAGE("mcast join/leave failed", gu_data == 0);

        //end environment
        apl_mutex_destroy(&gs_mtxlk);
    }


    /** 
     * Test case mcast set/get interface including two sub-cases.
     * Only the AF_INET are tested here. 
     */
    void testMcastIf(void)
    {
        //start case environment
        apl_handle_t    li_socket;
        apl_uint_t      lu_ifidxget;
        struct ifreq    ls_ifreq;    
        struct if_nameindex *lps_nameidxbase;
        struct if_nameindex *lps_nameidxcur;

        li_socket= apl_socket(APL_AF_INET, APL_SOCK_DGRAM, 0);

        if (li_socket < 0)   
        {
            CPPUNIT_FAIL("udp socket failed\n");
            return; 
        }

        lps_nameidxbase = if_nameindex();

        //case 
        //sub-case1: mcast set/get specified interfaces
        for (lps_nameidxcur=lps_nameidxbase; ;)
        {
            if (lps_nameidxcur->if_index == 0 || lps_nameidxcur->if_name == NULL)
            {
                apl_errprintf("This is the end of nameindex list, leaving ... \n");
                break;
            }

            /* check if the interface setable or not */ 
            apl_strncpy(ls_ifreq.ifr_name, lps_nameidxcur->if_name, IFNAMSIZ);
            if (ioctl(li_socket, SIOCGIFADDR, &ls_ifreq) < 0)
            {
                lps_nameidxcur ++;
                continue;
            }

            /* set mcast if */
            apl_errprintf(" mcast setting interface %s ifindex(%u)\n", 
                    lps_nameidxcur->if_name, lps_nameidxcur->if_index);

            if (apl_mcast_set_if(li_socket, 
                        lps_nameidxcur->if_name, 
                        lps_nameidxcur->if_index) < 0) 
            {
                CPPUNIT_FAIL("apl_mcast_set_if failed");
                break;
            }

            /* get mcast if */
            apl_errprintf(" mcast getting interface ...\n"); 

            if (apl_mcast_get_if(li_socket, &lu_ifidxget) < 0) 
            {
                CPPUNIT_FAIL("apl_mcast_get_if failed");
                break;
            }

            apl_errprintf("The set idx=%u, The get idx=%u\n", 
                    lps_nameidxcur->if_index, lu_ifidxget);

            CPPUNIT_ASSERT_MESSAGE("mcast_get/set interface failed", 
                    lps_nameidxcur->if_index == lu_ifidxget);

            lps_nameidxcur ++;
        }

        //sub-case2: mcast set/get INADDR_ANY (only available in AF_INET)
        if (apl_mcast_set_if(li_socket, APL_NULL, 0) < 0) 
        {
            CPPUNIT_FAIL("apl_mcast_set_if failed");
        }

        CPPUNIT_ASSERT_MESSAGE("apl_mcast_set/get_if INADDR_ANY failed", 
                apl_mcast_get_if(li_socket, &lu_ifidxget) == 0 
                && lu_ifidxget == 0); 

        //end case environment
        apl_close(li_socket);
        if_freenameindex(lps_nameidxbase);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplmcast);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
