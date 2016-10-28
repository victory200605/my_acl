#include "apl/poll.h"
#include "Test.h"
#include <algorithm>


#define SRC_BUF_SIZE 100

struct CFdbuf
{
    apl_int_t lai_fd[2];
    char *lpc_srcbuf;
    char *lpc_dstbuf;
};

void handler(int sig_no)
{
   apl_signal( APL_SIGPIPE, handler );
}

apl_size_t Recv_n(apl_int_t ai_fd, char* apc_chr, apl_size_t ai_len)
{
    if ( apc_chr == 0 )
    {
        return 0;
    }
    apl_size_t li_nread = 0;
    apl_size_t li_n = 0;
    while( li_nread < ai_len )
    {
        li_n = apl_recv(ai_fd, apc_chr+li_nread, ai_len-li_nread, 0);
        if ( li_n <= 0 )
        {
            if ( errno == APL_EINTR || errno == APL_EWOULDBLOCK || errno == APL_EAGAIN)
            {
                continue;
            }
            break;
        }
        li_nread += li_n;
    }
    return li_nread;
}

apl_size_t Send_n(apl_int_t ai_fd, void* ap_buf, apl_int_t ai_len)
{
    apl_int_t li_cou=0, li_ir=0;
    while( ai_len > 0)
    {
        li_ir = apl_send(ai_fd, (char*)ap_buf+li_cou, ai_len, 0);
        if ( li_ir <= 0)
        {
            if ( errno == APL_EINTR || errno == APL_EWOULDBLOCK || errno == APL_EAGAIN)
            {
                continue;
            }
            li_cou = 0;
            break;
        }
        li_cou += li_ir;
        ai_len -= li_ir;
    }
    return li_cou;
}


apl_size_t Recvfrom_n(apl_int_t ai_fd, char* apc_chr, apl_size_t ai_len,
                  struct apl_sockaddr_t* aps_sa, apl_size_t* apu_salen)
{
    if ( apc_chr == NULL || aps_sa == NULL || apu_salen == NULL )
    {
        return 0;
    }
    apl_size_t li_nread = 0;
    apl_size_t li_n = 0;
    while( li_nread < ai_len )
    {
        li_n = apl_recvfrom(ai_fd, apc_chr+li_nread, ai_len-li_nread, 0, aps_sa, apu_salen);
        if ( li_n <= 0 )
        {
            if ( errno == APL_EINTR || errno == APL_EWOULDBLOCK || errno == APL_EAGAIN)
            {
                continue;
            }
            break;
        }
        li_nread += li_n;
    }
    return li_nread;
}

apl_size_t Sendto_n(apl_int_t ai_fd, void* ap_buf, apl_int_t ai_len,
                    struct apl_sockaddr_t* aps_sa, apl_size_t au_salen )
{
    if ( ap_buf == NULL || aps_sa == NULL )
    {
        return 0;
    }
    apl_int_t li_cou=0, li_ir=0;
    while( ai_len > 0)
    {
        li_ir = apl_sendto(ai_fd, (char*)ap_buf+li_cou, ai_len, 0, aps_sa, au_salen);
        if ( li_ir <= 0)
        {
            if ( errno == EINTR || errno == EWOULDBLOCK || errno == APL_EAGAIN)
            {
                continue;
            }
            li_cou = 0;
            break;
        }
        li_cou += li_ir;
        ai_len -= li_ir;
    }
    return li_cou;
}

void testSockPeer(apl_int_t ai_fd)
{
    if ( ai_fd < 0 )
    {
       return ;
    }

    char lac_adr[124];
    struct apl_sockaddr_storage_t ls_tmp;
    struct apl_sockaddr_in_t* lps_tmp= ( struct apl_sockaddr_in_t*)&ls_tmp;
    apl_size_t lau_len ;
    if ( apl_getsockname(ai_fd, (struct apl_sockaddr_t*)lps_tmp, &lau_len) == 0 )
    {
        if ( apl_sockaddr_getaddr((struct apl_sockaddr_t*)lps_tmp, lac_adr, sizeof(*lps_tmp)) < 0 )
        {
            CPPUNIT_FAIL("getsockname failed\n");
        }
    }
    if ( apl_getpeername(ai_fd, (struct apl_sockaddr_t*)lps_tmp, &lau_len) == 0 )
    {
        if ( apl_sockaddr_getaddr((struct apl_sockaddr_t*)lps_tmp, lac_adr, sizeof(*lps_tmp)) <0 )
        {
            CPPUNIT_FAIL("getpeername failed\n");
        }
    }
}

void CreateServTcp(apl_int_t ai_type, apl_int_t& ai_tcplisten,
                apl_int_t ai_port, const char* apc_addr, apl_int_t ai_readdr )
{
    if ( apc_addr == NULL )
    {
        return ;
    }
    struct apl_sockaddr_storage_t ls_sas;
    struct apl_sockaddr_t* lps_adr = NULL; 

    apl_sockaddr_setfamily( (apl_sockaddr_t*)(&ls_sas), ai_type);
    lps_adr = (apl_sockaddr_t*)(&ls_sas);

    if ( apl_sockaddr_setaddr( lps_adr, sizeof(*lps_adr), apc_addr) < 0)
    {
        return ;
    }
    if ( APL_AF_UNIX != ai_type  && apl_sockaddr_setport( lps_adr, sizeof(*lps_adr),  ai_port) < 0 )
    {
        return ;
    }

    ai_tcplisten = apl_socket(ai_type, APL_SOCK_STREAM, 0);


    if ( ai_tcplisten < 1 )
    {
        return ;
    }
    if ( ai_readdr > 0)
    {
        ai_readdr = 1;
    }else
    {
        ai_readdr = 0;
    }
    apl_int_t li_ir = apl_setsockopt_reuseaddr(ai_tcplisten, ai_readdr);
    if ( li_ir < 0 )
    {
        return ;
    }
    li_ir = apl_bind(ai_tcplisten, lps_adr, sizeof(*lps_adr)) ;
    if ( li_ir < 0 )
    {
        return ;
    }
    li_ir = apl_listen(ai_tcplisten, 10);
    if ( li_ir < 0 ) 
    {
        return ;
    }

}

void* TestServTcp(apl_int_t li_type ) 
{
    apl_int_t li_port = 0;
    apl_int_t li_readdr = 1;
    apl_int_t li_timeout = -1; 
    apl_int_t li_tcp = 0;
    apl_int_t li_tcplisten = 0;

    if ( APL_AF_INET == li_type )
    {
        li_port = 10002;
        CreateServTcp(li_type, li_tcplisten, li_port, "0.0.0.0", li_readdr);
    }
    if ( APL_AF_UNIX == li_type )
    {
        system("rm -f /tmp/sock_xe2");
        li_port = 0;
        CreateServTcp(li_type, li_tcplisten, li_port, "/tmp/sock_xe2", li_readdr);
    }

    CPPUNIT_ASSERT(li_tcplisten > 0); 
   
    apl_int_t li_ir = 0;
    char lac_src[21]="1234567890ABCDEFGHIJ";

    struct apl_pollset_t ls_cli;
    struct apl_pollfd_t ls_pfd[10];
    if ( apl_pollset_init( &ls_cli, 10) <0 )
    {
        CPPUNIT_FAIL("apl_pollset_init failed");
        return NULL;
    }
    if ( apl_pollset_add_handle( &ls_cli, li_tcplisten, APL_POLLIN ) < 0 )
    {
        apl_errprintf("poll add hander error: %s\n", strerror(errno) );
        goto END;        
    }
    while(true)
    {
        apl_memset(ls_pfd, 0x0, sizeof(ls_pfd) );
        li_ir = apl_poll(&ls_cli, ls_pfd, sizeof(ls_pfd), li_timeout);
        if ( li_ir < 0 )
        {
            break;
        }
        if ( li_ir == 0)
        {
            break;
        }
        for( int li_in=0;li_in<li_ir;++li_in)
        {
            if ( APL_POLLFD_GET_HANDLE( &(ls_pfd[li_in]) ) == li_tcplisten
                   && ( APL_POLLFD_GET_EVENTS(&(ls_pfd[li_in]) ) & APL_POLLIN) )
            {
                struct apl_sockaddr_storage_t ls_sas;
                struct apl_sockaddr_in_t* ls_sai= ( struct apl_sockaddr_in_t*)&ls_sas;
                apl_size_t lu_len = sizeof(*ls_sai);
                li_tcp = apl_accept(li_tcplisten, (struct apl_sockaddr_t*)ls_sai, &lu_len);
                if ( li_tcp < 0 )
                {
                    if ( errno == APL_EINTR)
                    {
                        --li_in;
                        continue;
                    }
                }
                apl_int_t li_flag ;
                apl_fcntl_getfd(li_tcp, &li_flag);

                if ( apl_fcntl_setfd(li_tcp,  li_flag | O_NONBLOCK) )
                {
                }
                if ( apl_pollset_add_handle( &ls_cli, li_tcp, APL_POLLIN) == -1)
                {
                    apl_errprintf("poll add hander error:%s\n", strerror(errno) );
                    CPPUNIT_FAIL("apl_pollset_add_handle");
                    goto END;
                }

                //??
                apl_pollset_set_events(&ls_cli, li_tcplisten, 0);
                continue;
            }
            if ( APL_POLLFD_GET_HANDLE(&(ls_pfd[li_in]) ) == li_tcp
                    && ( APL_POLLFD_GET_EVENTS(&ls_pfd[li_in])& APL_POLLIN ) )
            {
                char lac_chr[21];
                apl_memset(lac_chr, 0x0, sizeof(lac_chr));
                apl_size_t li_len = Recv_n(li_tcp, lac_chr, 20);

                if ( li_len < 20 )
                {
                   goto END;
                }

                if ( strcmp(lac_src, lac_chr) != 0 )
                {
                    CPPUNIT_FAIL("tcp recv failed:");
                    CPPUNIT_FAIL(lac_chr);
                    CPPUNIT_FAIL("\n");
                }
                apl_errprintf("tcp recv:%s\n", lac_chr);
            }
        }
    }
END:    apl_pollset_destroy(&ls_cli);
    if ( li_tcp > 0 )
    {
        apl_close(li_tcp);
    }
    if ( li_tcplisten > 0 )
    {
        apl_close(li_tcplisten);
    }
    return NULL;
}

void CreateTcpCli(apl_int_t ai_type, apl_int_t& ap_tcpcli, apl_int_t ai_port,
              const char* apc_addr, apl_int_t ai_readdr, apl_int_t ai_timeout )
{
    if ( apc_addr == NULL )
    {
        return ;
    }
    struct apl_sockaddr_storage_t ls_sas;
    struct apl_sockaddr_t* lp_addr = NULL;
    apl_sockaddr_setfamily( (apl_sockaddr_t*)(&ls_sas), ai_type);
    lp_addr = (struct apl_sockaddr_t*)(&ls_sas);


    if ( apl_sockaddr_setaddr( lp_addr, sizeof(*lp_addr), apc_addr) < 0)
    {
        return ;
    }
    if ( ai_port != 0 && apl_sockaddr_setport( lp_addr, sizeof(*lp_addr),  ai_port) < 0 )
    {
        return ;
    }

    ap_tcpcli = apl_socket(ai_type, APL_SOCK_STREAM, 0);

    if ( ap_tcpcli < 1 )
    {
        return ;
    }

    
    apl_int_t li_flag ;
    apl_fcntl_getfd(ap_tcpcli, &li_flag);
    if ( apl_fcntl_setfd(ap_tcpcli,  li_flag | O_NONBLOCK) < 0 )
    {
        CPPUNIT_FAIL("apl_fcntl_setfd failed");
    }

    if ( apl_connect(ap_tcpcli, lp_addr, sizeof(*lp_addr) ) == 0 )
    {
       return ;
    }else
    {
        if ( errno != EINPROGRESS && errno != EINTR )
        {
            return ;
        }
    }

    fd_set ls_rset;
    FD_ZERO(&ls_rset);
    FD_SET(ap_tcpcli, &ls_rset);
    struct timeval ls_time;
    ls_time.tv_sec = 75;
    ls_time.tv_usec =0;
    if ( ai_timeout > 0 )
    {
        ls_time.tv_sec = ai_timeout;
    }

    apl_int_t li_ir;
    while (true)
    {
        li_ir = select(ap_tcpcli+1, &ls_rset, NULL, NULL, &ls_time);
        if ( li_ir == 0 )
        {
            return ;
        }else if ( li_ir < 0 )
        {
            if ( errno == APL_EINTR )
            {
               continue;
            }
        }

        if ( FD_ISSET(ap_tcpcli, &ls_rset) )
        {
            FD_ZERO(&ls_rset);
            break;
        }
    }
}

void* TestTcpCli(apl_int_t li_type) 
{
    apl_signal(SIGPIPE, handler);

    apl_int_t li_port = 0;
    apl_int_t li_readdr = 1;
    apl_int_t li_timeout = -1;
    apl_int_t li_tcpcli = -1;

    if ( APL_AF_INET == li_type )
    {
        li_port = 10002;
        CreateTcpCli(li_type, li_tcpcli, li_port, "0.0.0.0", li_readdr, li_timeout);
    }

    if ( APL_AF_UNIX == li_type )
    {
        li_port = 0;
        CreateTcpCli(li_type, li_tcpcli, li_port, "/tmp/sock_xe2", li_readdr, li_timeout);
    }

    CPPUNIT_ASSERT(li_tcpcli > 0);

    char lac_src[21]="1234567890ABCDEFGHIJ";
    apl_int_t li_n = 0;

    for ( ; li_n < 3 ;++li_n)
    {
        if (Send_n(li_tcpcli, lac_src, 20) < 20 )
        {
            break;
        }

        apl_errprintf("tcp send: %s\n", lac_src);
    }

    if ( li_tcpcli > 0 )
    {
        apl_close( li_tcpcli );
    }

    return NULL;
}

void CreateUdp(apl_int_t ai_type, apl_int_t& ai_udp, apl_int_t ai_port, const char* apc_addr, apl_int_t ai_readdr)
{
    if ( apc_addr == NULL )
    {
        return ;
    }
    struct apl_sockaddr_storage_t ls_sas;
    struct apl_sockaddr_t* lps_addr = NULL;

    lps_addr = (struct apl_sockaddr_t*)(&ls_sas);
    apl_sockaddr_setfamily( (apl_sockaddr_t*)(&ls_sas), ai_type);


    if ( apl_sockaddr_setaddr( lps_addr, sizeof(*lps_addr), apc_addr) < 0)
    {
        return ;
    }
    if ( APL_AF_UNIX != ai_type && apl_sockaddr_setport( lps_addr, sizeof(*lps_addr),  ai_port) < 0 )
    {
        return ;
    }

    ai_udp = apl_socket(ai_type, APL_SOCK_DGRAM, 0);
    if ( ai_udp < 1 )
    {
        return ;
    }
    apl_int_t li_ir = apl_setsockopt_reuseaddr(ai_udp, ai_readdr);
    if ( li_ir < 0 )
    {
        return ;
    }
    li_ir = apl_bind(ai_udp, lps_addr, sizeof(*lps_addr));
    if ( li_ir < 0 )
    {
        return ;
    }
    apl_int_t li_flag = 0;
    apl_fcntl_getfd(ai_udp, &li_flag);

    if ( apl_fcntl_setfd(ai_udp,  li_flag | O_NONBLOCK) < 0 )
    {
        return ;
    }
}

void* TestServUdp(apl_int_t li_type)
{
    apl_int_t li_port =0;
    apl_int_t li_readdr = 1;
    apl_int_t li_udp = 0;
  
    if ( APL_AF_INET == li_type )
    {
        char lac_addr[10]="0.0.0.0";
        li_port = 10006;
        CreateUdp(li_type, li_udp, li_port, lac_addr, li_readdr);
    }
    if ( APL_AF_UNIX == li_type )
    {
        system("rm -f /tmp/sock_xe1");
        li_port = 0;
        CreateUdp(li_type, li_udp, li_port, "/tmp/sock_xe1", li_readdr);
    }
    if ( li_udp < 0 )
    {
        return NULL;
    }


    apl_int_t  li_ir = 0;
    apl_int_t  li_ev = 0;
    apl_int_t  li_in = 0;
    apl_time_t li_time = -1;
   
    char lac_buf[21]="";
    char lac_src[21]="abcdefghihjklmnopqrs";
    struct apl_sockaddr_storage_t ls_sas;
    struct apl_sockaddr_in_t* ls_sai= ( struct apl_sockaddr_in_t*)&ls_sas;
    apl_size_t lau_len =0;
    while( li_in < 3 )
    {
        li_ir = apl_poll_one(li_udp, APL_POLLIN, &li_ev, li_time);
        if ( li_ir <= 0 )
        {
            break;
        } 
        if ( Recvfrom_n(li_udp, lac_buf, 20, (struct apl_sockaddr_t*)ls_sai,
                         &lau_len ) < 20 )
        {
             break;
        }
        li_in++;
        lac_buf[20]='\0';
        if ( strcmp(lac_buf, lac_src) != 0 )
        {
                CPPUNIT_FAIL("udp recv failed:");
                CPPUNIT_FAIL(lac_buf);
                CPPUNIT_FAIL("\n");
        }
        apl_errprintf("udp recv:%s\n", lac_buf);
        apl_memset(lac_buf, 0x0, sizeof(lac_buf));
    }
    apl_close(li_udp);
    return NULL;
}

void* TestUdpCli(apl_int_t li_type)
{
    apl_int_t li_udpcli = -1;   
    apl_signal(APL_SIGPIPE, handler);
    apl_int_t li_port = 10006;
    struct apl_sockaddr_storage_t ls_sas;
    struct apl_sockaddr_t* ls_addr = NULL;
    apl_sockaddr_setfamily( (apl_sockaddr_t*)(&ls_sas), li_type);
    ls_addr = (struct apl_sockaddr_t*)(&ls_sas);

    apl_int_t li_ir= -1;
    if ( APL_AF_INET == li_type )
    {
        char lac_addr[20]="0.0.0.0";   
        li_ir = apl_sockaddr_setaddr( ls_addr, sizeof(*ls_addr), lac_addr);
    } 
    if ( APL_AF_UNIX == li_type )
    {
        char lac_addr[20]="/tmp/sock_xe1";
        li_ir = apl_sockaddr_setaddr( ls_addr, sizeof(*ls_addr), lac_addr);
    }
    if ( li_ir < 0)
    {
        return NULL;
    }
    if ( li_type != APL_AF_UNIX && apl_sockaddr_setport( ls_addr, sizeof(*ls_addr),  li_port) < 0 )
    {
        return NULL;
    }
  
    li_udpcli = apl_socket(li_type, APL_SOCK_DGRAM, 0);

    if ( li_udpcli < 0 )
    {
        return NULL;
    }
    char lac_src[21]="abcdefghihjklmnopqrs";
    apl_int_t li_n = 0;
    for ( ; li_n < 3 ;++li_n)
    {
        if (Sendto_n(li_udpcli, lac_src, 20, ls_addr, sizeof(*ls_addr)  ) < 20 )
        {
           break;
        }
        apl_errprintf("udp send:%s\n",lac_src);
    }
    apl_close(li_udpcli);
    return NULL;
}

/**
 * @brief Test suite SocketTcp.
 * @brief Test case testTcp including two sub-cases, one protocol 
 * (APL_AF_INET and APL_AF_UNIX) per sub-case.
 */
class CTestapltcp : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestapltcp);
    CPPUNIT_TEST(testTcp);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}

    void tearDown(void) {}

    /** 
     * Test case testTcp including two sub-cases.
     */
    void testTcp(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_int_t li_type1=APL_AF_INET;
        apl_int_t li_type2=APL_AF_UNIX;

        //case
        //sub-case1 APL_AF_INET
        START_THREAD_BODY(ServTcpInet, apl_int_t, li_type1);
        TestServTcp(li_type1);
        END_THREAD_BODY(ServTcpInet);
        RUN_THREAD_BODY(ServTcpInet);

        sleep(1);
        START_THREAD_BODY(ServCliInet, apl_int_t, li_type1);
        TestTcpCli(li_type1);
        END_THREAD_BODY(ServCliInet);
        RUN_THREAD_BODY(ServCliInet);

        sleep(2);

        //sub-case1 APL_AF_INET
        START_THREAD_BODY(ServTcpUnix, apl_int_t, li_type2);
        TestServTcp(li_type2);
        END_THREAD_BODY(ServTcpUnix);
        RUN_THREAD_BODY(ServTcpUnix);

        sleep(1);
        START_THREAD_BODY(ServCliUnix, apl_int_t, li_type2);
        TestTcpCli(li_type2);
        END_THREAD_BODY(ServCliUnix);
        RUN_THREAD_BODY(ServCliUnix);

        sleep(3);

        //end environment
    }
};

/**
 * @brief Test suite SocketUdp.
 * @brief Test case testUdp including two sub-cases, one protocol 
 * (APL_AF_INET and APL_AF_UNIX) per sub-case;
 */
class CTestapludp : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestapludp);
    CPPUNIT_TEST(testUdp);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}

    void tearDown(void) {}

    /** 
     * Test case testUdp including two sub-cases.
     */
    void testUdp(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_int_t li_type1= APL_AF_INET;
        apl_int_t li_type2= APL_AF_UNIX;

        //case 
        //sub-case1 APL_AF_INET
        START_THREAD_BODY(ServUdpInet, apl_int_t, li_type1);
        TestServUdp(li_type1);
        END_THREAD_BODY(ServUdpInet);
        RUN_THREAD_BODY(ServUdpInet);

        sleep(1);
        START_THREAD_BODY(CliUdpInet, apl_int_t, li_type1);
        TestUdpCli(li_type1);
        END_THREAD_BODY(CliUdpInet);
        RUN_THREAD_BODY(CliUdpInet);

        sleep(2);

        //sub-case2 APL_AF_UNIX
        START_THREAD_BODY(ServUdpUnix, apl_int_t, li_type2);
        TestServUdp(li_type2);
        END_THREAD_BODY(ServUdpUnix);
        RUN_THREAD_BODY(ServUdpUnix);

        sleep(1);

        START_THREAD_BODY(CliUdpUnix, apl_int_t, li_type2);
        TestUdpCli(li_type2);
        END_THREAD_BODY(CliUdpUnix);
        RUN_THREAD_BODY(CliUdpUnix);

        sleep(3);

        //end environment
    }
};

/**
 * @brief Test suite Socketpair.
 * @brief Test case testPair including two sub-cases: APL_SOCK_DGRAM 
 * and APL_SOCK_STREAM.
 */
class CTestaplsockpair: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplsockpair);
    CPPUNIT_TEST(testPair);
    CPPUNIT_TEST_SUITE_END();

    struct CFdbuf masFdDgram;
    struct CFdbuf masFdStream;
    char macSrc[SRC_BUF_SIZE];
    char macDst[SRC_BUF_SIZE];

public:
    void setUp(void) 
    {
        macSrc[SRC_BUF_SIZE-1]=0;
        apl_memset(macDst, 0x0, sizeof(macDst) );
        masFdDgram.lpc_srcbuf = macSrc;
        masFdDgram.lpc_dstbuf = macDst;
        masFdStream.lpc_srcbuf = macSrc;
        masFdStream.lpc_dstbuf = macDst;

        std::generate_n(
            macSrc,
            sizeof(macSrc)-1,
            rand);

        if (apl_socketpair(APL_AF_UNIX, 
                    APL_SOCK_DGRAM, 
                    0, 
                    masFdDgram.lai_fd, 
                    masFdDgram.lai_fd+1) < 0)
        {
            CPPUNIT_FAIL("apl_socketpair failed");
        }

        if (apl_socketpair(APL_AF_UNIX, 
                    APL_SOCK_STREAM, 
                    0, 
                    masFdStream.lai_fd, 
                    masFdStream.lai_fd+1) < 0)
        {
            CPPUNIT_FAIL("apl_socketpair failed");
        }
    }

    void tearDown(void) 
    {
        apl_int_t li_in = sizeof(masFdDgram.lai_fd)/sizeof(masFdDgram.lai_fd[0]);

        while (--li_in) 
        {
            if (masFdDgram.lai_fd[li_in]) close(masFdDgram.lai_fd[li_in]);   
            if (masFdStream.lai_fd[li_in]) close(masFdStream.lai_fd[li_in]);   
        }
    }

    /** 
     * Test case testPair including two sub-cases.
     */
    void testPair(void)
    {
        //case start environment
        PRINT_TITLE_2(__func__); 

        //case
        //sub-case1 socketpair APL_AF_UNIX, APL_SOCK_DGRAM
        START_THREAD_BODY(SktPairUnixDgramSend, struct CFdbuf, masFdDgram)
        {
            Send_n(masFdDgram.lai_fd[0], masFdDgram.lpc_srcbuf, SRC_BUF_SIZE);
        }  
        END_THREAD_BODY(SktPairUnixDgramSend);
        RUN_THREAD_BODY(SktPairUnixDgramSend);

        sleep(1);
        START_THREAD_BODY(SktPairUnixDgramRecv, struct CFdbuf, masFdDgram)
        {
            Recv_n(masFdDgram.lai_fd[1], masFdDgram.lpc_dstbuf, SRC_BUF_SIZE);
        }  
        END_THREAD_BODY(SktPairUnixDgramRecv);
        RUN_THREAD_BODY(SktPairUnixDgramRecv);

        sleep(2);
        CPPUNIT_ASSERT(strncmp(macSrc, macDst, SRC_BUF_SIZE) == 0);
        apl_memset(macDst, 0x0, sizeof(macDst) );

        //sub-case2 socketpair APL_AF_UNIX, APL_SOCK_STREAM
        START_THREAD_BODY(SktPairUnixStreamSend, struct CFdbuf, masFdStream)
        {
            Send_n(masFdStream.lai_fd[0], masFdStream.lpc_srcbuf, SRC_BUF_SIZE);
        }  
        END_THREAD_BODY(SktPairUnixStreamSend);
        RUN_THREAD_BODY(SktPairUnixStreamSend);

        sleep(1);
        START_THREAD_BODY(SktPairUnixStreamRecv, struct CFdbuf, masFdStream)
        {
            Recv_n(masFdStream.lai_fd[1], masFdStream.lpc_dstbuf, SRC_BUF_SIZE);
        }  
        END_THREAD_BODY(SktPairUnixStreamRecv);
        RUN_THREAD_BODY(SktPairUnixStreamRecv);

        sleep(3);
        CPPUNIT_ASSERT(strncmp(macSrc, macDst, SRC_BUF_SIZE) == 0);

        //end environment
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestapltcp);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestapludp);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplsockpair);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
