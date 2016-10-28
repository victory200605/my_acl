#include "Test.h"
#include "apl/poll.h"


#define TCP_PORT  (10002)
#define UDP_PORT  (10004)



void handler(int sig_no)
{
   printf("signal SIGPIP action\n ");
   apl_signal( APL_SIGPIPE, handler );
}


apl_size_t recv_n(apl_int_t ai_fd, char* apc_chr, apl_size_t ai_len)
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
            if ( errno == APL_EINTR || errno == APL_EWOULDBLOCK )
            {
                continue;
            }
            break;
        }
        li_nread += li_n;
    }
    return li_nread;
}

apl_size_t send_n(apl_int_t ai_fd, void* ap_buf, apl_int_t ai_len)
{
    apl_int_t li_cou=0, li_ir=0;
    while( ai_len > 0)
    {
        li_ir = apl_send(ai_fd, (char*)ap_buf+li_cou, ai_len, 0);
        if ( li_ir <= 0)
        {
            if ( errno == APL_EINTR || errno == APL_EWOULDBLOCK )
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



class CTestaplservUdp : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplservUdp);
    CPPUNIT_TEST(testUdp);
    CPPUNIT_TEST_SUITE_END();
private:
    apl_int_t mi_udp;
public:
    void setUp(void) 
    {
        mi_udp =0;
    }
    void tearDown(void) 
    {
        fd_close(); 
    }
    void fd_close(void)
    {
        if ( mi_udp > 0 )
        {
            close(mi_udp);
            mi_udp =0;
        }
        //sleep(2);
    }

    void testUdp(void)
    {
        char lpc_addr1[10]="0.0.0.0";
        testserv(APL_AF_INET, UDP_PORT, lpc_addr1, 1);/* AF_INET */

        fd_close();
        char lpc_addr2[20]="/tmp/sock1";
        system("rm -f /tmp/sock1");
        testserv(APL_AF_UNIX, 0, lpc_addr2, 1);/* AF_UNIX */
    }

    void testserv(apl_int_t ai_type, apl_int_t ai_port, const char* lpc_addr, apl_int_t ai_readdr)
    {
        create_udp(ai_type, ai_port, lpc_addr, ai_readdr);
        if ( mi_udp < 0 )
        {
            return ;    
        }
        apl_int_t li_in = 0;
        char lac_buf[21]="";
        char lac_src[21]="abcdefghihjklmnopqrs"; 
        struct apl_sockaddr_storage_t ls_sas;
        struct apl_sockaddr_in_t* ls_sai= ( struct apl_sockaddr_in_t*)&ls_sas;
        apl_size_t lau_len =0;
        while( li_in < 3 )
        {
            if ( recvfrom_n(mi_udp, lac_buf, 20, (struct apl_sockaddr_t*)ls_sai, 
                             &lau_len ) < 20 )
            {
                 break; 
            }
            li_in++;
            lac_buf[20]='\0';
            //CPPUNIT_ASSERT_EQUAL_MESSAGE("udp recv failed", lac_src, lac_buf);
            if ( strcmp(lac_buf, lac_src) != 0 ) 
            {
                    CPPUNIT_FAIL("udp recv failed:");
                    CPPUNIT_FAIL(lac_buf);
                    CPPUNIT_FAIL("\n");
            }
            printf("udp recv:%s\n", lac_buf);
            apl_memset(lac_buf, 0x0, sizeof(lac_buf));
        }        
    }

    apl_size_t recvfrom_n(apl_int_t ai_fd, char* apc_chr, apl_size_t ai_len, 
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
                if ( errno == APL_EINTR || errno == APL_EWOULDBLOCK )
                {
                    continue;
                }
                break;
            }
            li_nread += li_n;
        }
        return li_nread;
    }

    void create_udp(apl_int_t ai_type, apl_int_t ai_port, const char* apc_addr, apl_int_t ai_readdr)
    {
        if ( apc_addr == NULL )
        {
            return ;
        }
        mi_udp = 0;
        struct apl_sockaddr_storage_t ls_sas;
        struct apl_sockaddr_t* lps_addr = NULL;

        apl_sockaddr_setfamily( (apl_sockaddr_t*)(&ls_sas), ai_type);
        lps_addr = (struct apl_sockaddr_t*)(&ls_sas); 


        if ( apl_sockaddr_setaddr( lps_addr, sizeof(*lps_addr), apc_addr) < 0)
        {
            printf("udp address failed\n");
            return ;
        }
        if ( APL_AF_UNIX != ai_type && apl_sockaddr_setport( lps_addr, sizeof(*lps_addr),  ai_port) < 0 )
        {
            printf("udp port failed\n");
            return ;
        }

        mi_udp = apl_socket(ai_type, APL_SOCK_DGRAM, 0);
        if ( mi_udp < 1 )
        {
            printf("udp sock failed\n");
            return ;
        }
        apl_int_t li_ir = apl_setsockopt_reuseaddr(mi_udp, ai_readdr);
        if ( li_ir < 0 )
        {
            printf("udp readdr failed\n");
            return ;
        }
        li_ir = apl_bind(mi_udp, lps_addr, sizeof(*lps_addr));
        if ( li_ir < 0 )
        {
            printf(" udp bind failed%s\n", strerror(errno));
            return ;
        }
        apl_int_t li_flag = 0;
        apl_fcntl_getfd(mi_udp, &li_flag);

        if ( apl_fcntl_setfd(mi_udp,  li_flag | O_NONBLOCK) < 0 )
        {
            printf("udp noblock failed\n");
            return ;
        } 
    }

};



class CTestaplserv: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplserv);
    CPPUNIT_TEST(testTcp);
    CPPUNIT_TEST_SUITE_END();
private:
    apl_int_t mi_tcp;
    apl_int_t mi_tcplisten;
    fd_set mfd_allset;
public:
    void setUp(void) 
    {
        mi_tcp =0;
    }

    void testTcp(void)
    {
        char lac_chr[10]="0.0.0.0";
        testserv(APL_AF_INET, TCP_PORT, lac_chr, 1, 10);


        fd_close();
        char lac_chr1[20]="/tmp/sock2";
        system("rm -f /tmp/sock2");
        testserv(APL_AF_UNIX, 0, lac_chr1, 1, 0);
    }

    void fd_close(void)
    {
        if ( mi_tcp > 0 )
        {
            apl_shutdown(mi_tcp, APL_SHUT_RDWR);
            mi_tcp = 0;
        }
        if ( mi_tcplisten > 0 )
        {
            apl_shutdown(mi_tcplisten, APL_SHUT_RDWR);
            mi_tcplisten = 0;
        }
        //sleep(2);        
    }

    void testserv(apl_int_t ai_type, apl_int_t ai_port, const char* apc_addr, apl_int_t ai_readdr, apl_int_t ai_timeout)
    {
        if ( apc_addr == NULL )
        {
            return ;
        }
        create_tcp(ai_type, ai_port, apc_addr, ai_readdr);
        if  ( mi_tcplisten < 1 )
        {
            return ;
        }
        mi_tcp = 0;
        apl_int_t li_ir = 0;
        char lac_src[21]="1234567890ABCDEFGHIJ";

        mi_tcp =-1;
        struct apl_pollset_t ls_cli;
        struct apl_pollfd_t ls_pfd[10];
        if ( apl_pollset_init( &ls_cli, 10) <0 )
        {
            printf("poll init error\n");
        }
        apl_pollset_add_handle( &ls_cli, mi_tcplisten );
        apl_pollset_set_events( &ls_cli, mi_tcplisten, APL_POLLIN);
        while(true)
        { 
            apl_memset(ls_pfd, 0x0, sizeof(ls_pfd) );
            li_ir = apl_poll(&ls_cli, ls_pfd, sizeof(ls_pfd), -1);
            if ( li_ir < 0 )
            {
                printf("poll error%s", strerror(errno) );
                break;
            }
            if ( li_ir == 0)
            {
                printf("poll nothing\n");
                break;
            }
            for( int li_in=0;li_in<li_ir;++li_in)
            {
                if ( APL_POLLFD_GET_HANDLE( &(ls_pfd[li_in]) ) == mi_tcplisten 
                       && ( APL_POLLFD_GET_EVENTS(&(ls_pfd[li_in]) ) & APL_POLLIN) )
                {
                    struct apl_sockaddr_storage_t ls_sas;
                    struct apl_sockaddr_in_t* ls_sai= ( struct apl_sockaddr_in_t*)&ls_sas;
                    apl_size_t lu_len = sizeof(*ls_sai);
                    mi_tcp = apl_accept(mi_tcplisten, (struct apl_sockaddr_t*)ls_sai, &lu_len);
                    if ( mi_tcp < 0 )
                    {
                        if ( errno == APL_EINTR)
                        {
                            --li_in; 
                            continue;
                        }
                    }
                    apl_int_t li_flag ;
                    apl_fcntl_getfd(mi_tcp, &li_flag);
    
                    if ( apl_fcntl_setfd(mi_tcp,  li_flag | O_NONBLOCK) )
                    {
                        printf("set noblock failed\n");
                    }
                    if ( apl_pollset_add_handle( &ls_cli, mi_tcp) == -1)
                    {
                        goto END;
                    } 
                    if ( apl_pollset_set_events( &ls_cli, mi_tcp, APL_POLLIN) == -1)
                    {
                        goto END;
                    } 

                
                    apl_pollset_set_events(&ls_cli, mi_tcplisten, 0);
                    continue; 
                }
                if ( APL_POLLFD_GET_HANDLE(&(ls_pfd[li_in]) ) == mi_tcp
                        && ( APL_POLLFD_GET_EVENTS(&ls_pfd[li_in])& APL_POLLIN ) )
                {
                    char lac_chr[21];
                    apl_memset(lac_chr, 0x0, sizeof(lac_chr));
                    apl_size_t li_len = recv_n(mi_tcp, lac_chr, 20);
                    if ( li_len < 20 )
                    {
                       goto END;
                    }
                    //CPPUNIT_ASSERT_EQUAL_MESSAGE("tcp recv failed", lac_src, lac_chr);
                    if ( strcmp(lac_src, lac_chr) != 0 )
                    {
                        CPPUNIT_FAIL("tcp recv failed:");
                        CPPUNIT_FAIL(lac_chr);
                        CPPUNIT_FAIL("\n");
                    }
                    printf("tcp recv:%s\n", lac_chr);
                } 
            }
        }
END:    apl_pollset_destroy(&ls_cli); 

    }

    void create_tcp(apl_int_t ai_type, apl_int_t ai_port, const char* apc_addr, apl_int_t ai_readdr)
    {
        if ( apc_addr == NULL )
        {
            return ;
        }
        mi_tcplisten = 0;
        mi_tcp = 0;
        struct apl_sockaddr_storage_t ls_sas;
        struct apl_sockaddr_t* lps_adr = NULL;
        
        apl_sockaddr_setfamily( (apl_sockaddr_t*)(&ls_sas), ai_type);
        lps_adr = (apl_sockaddr_t*)(&ls_sas);

        if ( ai_type == 20)
        {
            struct apl_sockaddr_un_t* ls_sai= ( struct apl_sockaddr_un_t*)&ls_sas;
            apl_sockaddr_setfamily( (apl_sockaddr_t*)ls_sai, APL_AF_UNIX);
            lps_adr = (apl_sockaddr_t*)ls_sai;
        }

        if ( apl_sockaddr_setaddr( lps_adr, sizeof(*lps_adr), apc_addr) < 0)
        {
            printf(" tcp address failed\n");
            return ;
        }
        if ( APL_AF_UNIX != ai_type  && apl_sockaddr_setport( lps_adr, sizeof(*lps_adr),  ai_port) < 0 )
        {
            printf(" tcp port failed:%s\n", strerror(errno) );
            return ;
        }

        mi_tcplisten = apl_socket(ai_type, APL_SOCK_STREAM, 0);

        if ( ai_type == 20)
        {
            mi_tcplisten = apl_socket(APL_AF_UNIX, APL_SOCK_STREAM, 0);
        }

        if ( mi_tcplisten < 1 )
        {
            printf(" tcp socket failed\n");
            return ;
        }
        if ( ai_readdr > 0)
        {
            ai_readdr = 1;
        }else
        {
            ai_readdr = 0;
        }
        apl_int_t li_ir = apl_setsockopt_reuseaddr(mi_tcplisten, ai_readdr);
        if ( li_ir < 0 )
        {
            printf("tcp readdr failed\n");
            return ;
        }
        li_ir = apl_bind(mi_tcplisten, lps_adr, sizeof(*lps_adr));
        if ( li_ir < 0 )
        {
            apl_close(mi_tcplisten);
            printf("tcp bind failed: %s\n", strerror(errno));
            return ;
        }
        li_ir = apl_listen(mi_tcplisten, 10);
        if ( li_ir < 0 )
        {
            printf("tcp listen failed\n");
            return ; 
        }
        
    }

    void tearDown(void) 
    {
        fd_close(); 
    }

    apl_int_t max(apl_int_t ai_elem1, apl_int_t ai_elem2)
    {
        apl_int_t li_elem;
        if ( ai_elem1 < ai_elem2 )
        {
           li_elem = ai_elem2;
        }else
        {
           li_elem = ai_elem1;
        }
        return li_elem ;
    }

    void testpeername(apl_int_t ai_fd)
    {
        if ( ai_fd < 0 )
           return ;

        char lac_adr[124];
        struct apl_sockaddr_storage_t ls_tmp;
        struct apl_sockaddr_in_t* lps_tmp= ( struct apl_sockaddr_in_t*)&ls_tmp;
        apl_size_t lau_len ;
        if ( apl_getsockname(ai_fd, (struct apl_sockaddr_t*)lps_tmp, &lau_len) == 0 )
        {
            apl_sockaddr_getaddr((struct apl_sockaddr_t*)lps_tmp, lac_adr, sizeof(*lps_tmp));
            //printf("local:%s\n", lac_adr);
        }
        if ( apl_getpeername(ai_fd, (struct apl_sockaddr_t*)lps_tmp, &lau_len) == 0 )
        {
            apl_sockaddr_getaddr((struct apl_sockaddr_t*)lps_tmp, lac_adr, sizeof(*lps_tmp));
            //printf("remove:%s\n", lac_adr);
        }
    }

};


apl_int_t ga_fd[2];

class CTestaplsockpair: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplsockpair);
    CPPUNIT_TEST(testPair);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void)
    {
    }
    void tearDown(void)
    {
    }
    void testPair(void)
    {
         test_pair(APL_SOCK_DGRAM);
         //sleep(2);
         test_pair(APL_SOCK_STREAM);
    }
    void test_pair(apl_int_t ai_type)
    {
        apl_signal(APL_SIGPIPE, handler);
        if ( apl_socketpair(APL_AF_UNIX, ai_type, 0, ga_fd, ga_fd+1)<0 )
        {
            printf("sockpair error%s\n", strerror(errno) );
            return ;
        }

        char lac_src[10]="xyuiop";
        char lac_dst[10]="";

        apl_pid_t lp_pid =0;
        if ( (lp_pid=apl_fork())==0 )
        {
            close(ga_fd[1]);
            send_n(ga_fd[0], lac_src, 5);
            close(ga_fd[0]);
            exit(0);
        }else if (lp_pid < 0 )
        {
            close(ga_fd[0]);
            close(ga_fd[1]);
            exit(1);
        }
        close(ga_fd[0]);
        //sleep(1);
        recv_n(ga_fd[1], lac_dst, 5);
        close(ga_fd[1]);
        if ( apl_strncmp(lac_src, lac_dst, 5) != 0 )
        {
            printf("\nsocketpair send: xyuio\n");
            printf("recv : %s\n", lac_dst);
            CPPUNIT_FAIL("socketpair failed\n");
        }
    }


};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplservUdp);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplserv);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplsockpair);




void init_daemon(void)
{
     apl_pid_t pid;
     if (pid=apl_fork() )
     {
        //sleep(1);
        exit(0);
     } else if ( pid <0 )
     {
        exit(1);
     } 
     setsid(); 
     return ;
}




int main(int argc, char *argv[])
{

//    init_daemon();
    apl_signal(APL_SIGHUP, APL_SIG_IGN); 
    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    if (runner.run())
       return 0;
    else
       return 1;
}
