#include "Test.h"


#define TCP_PORT  (10002)
#define UDP_PORT  (10004)

void handler(int sig_no)
{
   printf("signal SIGPIP action\n ");
   apl_signal( APL_SIGPIPE, handler );
}


class CTestaplcliUdp : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplcliUdp);
    CPPUNIT_TEST(testUdp);
    CPPUNIT_TEST_SUITE_END();
private:
    apl_int_t mi_udpcli;
public:
    void setUp(void)
    {
        mi_udpcli =0; 
    }
    void fd_close(void)
    {
        if ( mi_udpcli > 0)
        {
           apl_close(mi_udpcli);
           mi_udpcli = 0;
        }
        sleep(2);
    }
    void tearDown(void)
    {
        fd_close();
    }
    void testUdp(void)
    {
        char lac_addr1[10]="0.0.0.0";
        testcli(APL_AF_INET, UDP_PORT, lac_addr1);

        fd_close();
        char lac_addr2[20]="/tmp/sock1";
        testcli(APL_AF_UNIX, 0, lac_addr2);
    }

    apl_size_t sendto_n(apl_int_t ai_fd, void* ap_buf, apl_int_t ai_len, 
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
                if ( errno == EINTR || errno == EWOULDBLOCK )
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

    void creat_udp(apl_int_t ai_type, apl_int_t ai_port, const char* apc_addr)
    {
        apl_signal(APL_SIGPIPE, handler);
        if ( apc_addr == NULL )
        {
            return ;
        }
        struct apl_sockaddr_storage_t ls_sas;
        struct apl_sockaddr_t* ls_addr = NULL;
        apl_sockaddr_setfamily( (apl_sockaddr_t*)(&ls_sas), ai_type);
        ls_addr = (struct apl_sockaddr_t*)(&ls_sas); 

        if ( apl_sockaddr_setaddr( ls_addr, sizeof(*ls_addr), apc_addr) < 0)
        {
            printf("udp address failed\n");
            return ;
        }
        if ( ai_type != APL_AF_UNIX && apl_sockaddr_setport( ls_addr, sizeof(*ls_addr),  ai_port) < 0 )
        {
            printf("udp port failed\n");
            return ;
        }
   
        mi_udpcli = apl_socket(ai_type, APL_SOCK_DGRAM, 0);

        if ( mi_udpcli < 0 )
        {
            printf(" udp socket failed\n");
            return ;
        }
        char lac_src[21]="abcdefghihjklmnopqrs";
        apl_int_t li_n = 0;
        for ( ; li_n < 3 ;++li_n)
        {
            if (sendto_n(mi_udpcli, lac_src, 20, ls_addr, sizeof(*ls_addr)  ) < 20 )
            {
               printf("sendto_n failed\n");
               break;
            }
            printf("udp send: %s\n", lac_src);
        }
        
    }

    void testcli(apl_int_t ai_type, apl_int_t ai_port, const char* apc_addr)
    {
        creat_udp(ai_type, ai_port, apc_addr);
    }
};


class CTestaplcli : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplcli);
    CPPUNIT_TEST(testTcp);
    CPPUNIT_TEST_SUITE_END();
private:
    apl_int_t mi_tcpcli;
public:
    void setUp(void) 
    {
        mi_tcpcli =0;
    }
    void tearDown(void) 
    {
        fd_close();
    }

    void fd_close(void)
    {

        if ( mi_tcpcli > 0)
        {
           apl_close(mi_tcpcli);
           mi_tcpcli =0;
        }
        sleep(2);
    }

    void testTcp(void)
    {
        char lac_addr[10]="0.0.0.0";
        testcli(APL_AF_INET, TCP_PORT, lac_addr, 1, 0);


        fd_close();
        char lac_addr1[20]="/tmp/sock2";
        testcli(APL_AF_UNIX, 0, lac_addr1, 1, 0);
    }

    void testcli(apl_int_t ai_type, apl_int_t ai_port, const char* apc_addr, apl_int_t ai_readdr, apl_int_t ai_timeout)
    {
        apl_signal(SIGPIPE, handler);
        if ( apc_addr == NULL )
        {
            return ;
        }
        create_tcp(ai_type, ai_port, apc_addr, ai_readdr, ai_timeout);
        if ( mi_tcpcli == 0 )
        {
            return ;
        }

        char lac_src[21]="1234567890ABCDEFGHIJ";
        apl_int_t li_n = 0;
        for ( ; li_n < 3 ;++li_n)
        {
            if (send_n(mi_tcpcli, lac_src, 20) < 20 )
            {
               break;
            }
            printf("tcp send: %s\n", lac_src);
        }
    }

    apl_size_t send_n(apl_int_t ai_fd, void* ap_buf, apl_int_t ai_len)
    {
        if ( ap_buf == NULL )
        {
            return 0;
        }
        apl_int_t li_cou=0, li_ir=0;
        while( ai_len > 0)
        {
            li_ir = apl_send(ai_fd, (char*)ap_buf+li_cou, ai_len, 0);
            if ( li_ir <= 0)
            {
                if ( errno == EINTR )
                    continue;
                li_cou = 0;
                break;
            }
            li_cou += li_ir;
            ai_len -= li_ir;
        }
        return li_cou;
    }

    void create_tcp(apl_int_t ai_type, apl_int_t ai_port, const char* apc_addr, apl_int_t ai_readdr, apl_int_t ai_timeout)
    {
        mi_tcpcli = 0;
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
            printf(" tcp address failed\n");
            return ;
        }
        if ( ai_port != 0 && apl_sockaddr_setport( lp_addr, sizeof(*lp_addr),  ai_port) < 0 )
        {
            printf("tcp port failed:%s\n", strerror(errno) );
            return ;
        }

        mi_tcpcli = apl_socket(ai_type, APL_SOCK_STREAM, 0);

        if ( mi_tcpcli < 1 )
        {
            printf("tcp socket failed\n");
            return ;
        }

        apl_int_t li_flag ;
        apl_fcntl_getfd(mi_tcpcli, &li_flag);
        if ( apl_fcntl_setfd(mi_tcpcli,  li_flag | O_NONBLOCK) < 0 )
        {
             printf("set noblock failed\n");
        }

        if ( apl_connect(mi_tcpcli, lp_addr, sizeof(*lp_addr) ) == 0 )
        {
           printf(" tcp connected\n");
           return ;
        }else
        {
            if ( errno != EINPROGRESS && errno != EINTR )
            {
                printf(" tcp connect failed: %s\n", strerror(errno));
                fd_close();
                return ;
            } 
        }

        fd_set ls_rset;
        FD_ZERO(&ls_rset);
        FD_SET(mi_tcpcli, &ls_rset);
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
            li_ir = select(mi_tcpcli+1, &ls_rset, NULL, NULL, &ls_time); 
            if ( li_ir == 0 )
            {
                fd_close();
                printf(" tcp connect timeout: \n");
                return ;
            }else if ( li_ir < 0 )
            {
                if ( errno == APL_EINTR )
                { 
                   continue;
                }
            }

            if ( FD_ISSET(mi_tcpcli, &ls_rset) )
            {
                FD_ZERO(&ls_rset);
                break;
            }
        }

    }


};




CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplcliUdp);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplcli);



void init_daemon(void)
{
     apl_pid_t pid;
     if (pid=apl_fork() )
     {
        sleep(1);
        exit(0);
     } else if ( pid < 0 )
     {
        exit(1);
     }
     setsid();
     return ;
}


int main(int argc, char *argv[])
{
    init_daemon();
    apl_signal(APL_SIGHUP, APL_SIG_IGN);
    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    if (runner.run())
       return 0;
    else
       return 1;
}
