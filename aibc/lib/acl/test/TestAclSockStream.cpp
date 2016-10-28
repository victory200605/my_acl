#include "Test.h"
#include "acl/SockStream.h"
#include "acl/SockAcceptor.h"

using namespace acl;

#define BEGIN_SERV(paramtype, param, port) \
        paramtype& _param = param; \
        apl_int_t liRet = -1; \
        CSockAddr loLocalAddr(port, "127.0.0.1", APL_AF_INET); \
        liRet = _param.Open(APL_AF_INET, APL_SOCK_STREAM, 0); \
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Create socket failed.",  (apl_int_t)0, liRet); \
        liRet = _param.SetOption(CSockHandle::OPT_REUSEADDR, 1); \
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Set reuse addr failed.",  (apl_int_t)0, liRet); \
        if(apl_bind(_param.GetHandle(), loLocalAddr.GetAddr(), loLocalAddr.GetLength()) != 0) \
        { \
            _param.Close(); \
            CPPUNIT_FAIL("Bind failed"); \
            return ; \
        } \
        if ( apl_listen(_param.GetHandle(), 10) != 0 ) \
        { \
            _param.Close(); \
            CPPUNIT_FAIL("Listen failed"); \
            return ; \
        } \
        apl_size_t liLen = 0; \
        apl_handle_t liListenfd = _param.GetHandle(); \
        while(true) \
        { \
            apl_handle_t liConnfd = apl_accept(liListenfd, NULL, &liLen) ; \
            if (liConnfd < 0) \
            { \
                if(errno == APL_EINTR) \
                { \
                    continue; \
                } \
                CPPUNIT_FAIL("Accept failed"); \
                return ; \
            } \
            _param.SetHandle(liConnfd); 

#define END_SERV(param) \
            param.Close(); \
       } 
            

#define BEGIN_CLI(handle, port) \
{\
        CSockAddr loRemoteAddr(port, "127.0.0.1", APL_AF_INET); \
        handle = apl_socket(APL_AF_INET, APL_SOCK_STREAM, 0); \
        PrintErrMsg( apl_strerror(apl_get_errno()), handle); \
        apl_int_t liRet = apl_connect(handle, loRemoteAddr.GetAddr(), loRemoteAddr.GetLength()); \
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Connect socket failed.", (apl_int_t)0, liRet); 

#define END_CLI(handle) \
        apl_close(handle); \
}\

class CTestAclSockStream:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclSockStream);
    CPPUNIT_TEST(testRecv1);
    CPPUNIT_TEST(testRecv2);
    CPPUNIT_TEST(testRecvTimeout1);
    CPPUNIT_TEST(testRecvTimeout2);
    CPPUNIT_TEST(testSend1);
    CPPUNIT_TEST(testSend2);
    CPPUNIT_TEST(testSendTimeout1);
    CPPUNIT_TEST(testSendTimeout2);
    CPPUNIT_TEST(testRecvUntil);
    CPPUNIT_TEST(testRecvV);
    CPPUNIT_TEST(testRecvVTimeout);
    CPPUNIT_TEST(testSendV);
    CPPUNIT_TEST(testSendVTimeout);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testRecv1(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        apl_int_t liTmp = 0;

        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;
            BEGIN_SERV(CSockStream, loSockStream, 19371)
            {
                char lsPeekBuf[50];
                char lsRecvBuf[50];
                memset(lsPeekBuf, 0x0, sizeof(lsPeekBuf));
                memset(lsRecvBuf, 0x0, sizeof(lsRecvBuf));
                char const*lpcSendBuf = "1234567890abcdefghijklmn";

                apl_ssize_t liPeekSize = -1;
                apl_ssize_t liRecvSize = -1;

                //case
                liPeekSize = loSockStream.Recv(lsPeekBuf, sizeof(lsPeekBuf), 
                        CSockStream::OPT_PEEK);
                CPPUNIT_ASSERT_MESSAGE("testRecv1- Recv failed.", 
                        liPeekSize == (apl_ssize_t)apl_strlen(lsPeekBuf));
                CPPUNIT_ASSERT_MESSAGE("testRecv1- Recv failed.", 
                        apl_strcmp(lpcSendBuf, lsPeekBuf) == 0);

                liRecvSize = loSockStream.Recv(lsRecvBuf, sizeof(lsRecvBuf), 
                        CSockStream::OPT_UNSPEC);
                CPPUNIT_ASSERT_MESSAGE("testRecv1- Recv failed.", 
                        liRecvSize == (apl_ssize_t)apl_strlen(lpcSendBuf));
                CPPUNIT_ASSERT_MESSAGE("testRecv1- Recv failed.", 
                        apl_strcmp(lpcSendBuf, lsRecvBuf) == 0);
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        const char *lpcBuf = "1234567890abcdefghijklmn";
        apl_handle_t liHandle = -1;
        BEGIN_CLI(liHandle, 19371)
        {
            apl_ssize_t liSendSize = apl_send(liHandle, (void*)lpcBuf, apl_strlen(lpcBuf), 0);
            CPPUNIT_ASSERT_MESSAGE("testRecv1 - Send failed.", 
                    (apl_ssize_t)apl_strlen(lpcBuf) == liSendSize);
        }
        END_CLI(liHandle)

        //end environment
    }

    void testRecv2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;

        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;
            BEGIN_SERV(CSockStream, loSockStream, 19372)
            {
                char lsPeekBuf[50];
                char lsRecvBuf[50];
                memset(lsPeekBuf, 0x0, sizeof(lsPeekBuf));
                memset(lsRecvBuf, 0x0, sizeof(lsRecvBuf));
                const char *lpcSendBuf = "1234567890abcdefghijklmn";

                //case
                apl_ssize_t liRecvSize = -1;
                liRecvSize = loSockStream.Recv(lsRecvBuf, sizeof(lsRecvBuf));
                CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecv2- Recv failed.", 
                        (apl_ssize_t)24, liRecvSize);
                CPPUNIT_ASSERT_MESSAGE("testRecv2- Recv failed.", 
                        apl_strcmp(lpcSendBuf, lsRecvBuf) == 0);
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        const char *lpcBuf = "1234567890abcdefghijklmn";
        apl_handle_t liHandle = -1;
        BEGIN_CLI(liHandle ,19372)
        {
            apl_ssize_t liSendSize = apl_send(liHandle, (void*)lpcBuf, apl_strlen(lpcBuf), 0);
            CPPUNIT_ASSERT_MESSAGE("testRecv2 - Send failed.", 
                    (apl_ssize_t)apl_strlen(lpcBuf) == liSendSize);
        }
        END_CLI(liHandle)

        //end environment
    }

    void testRecvTimeout1(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        apl_int_t liTmp = 0;

        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;

            apl_int_t liFlag = 1;
            BEGIN_SERV(CSockStream, loSockStream, 19373)
            {
                char lsRecvBuf[50];
                memset(lsRecvBuf, 0x0, sizeof(lsRecvBuf));

                apl_ssize_t liRecvSize = -1;

                CTimeValue loTimeValue(1);
                
                //case
                if(liFlag--)
                {
                    liRecvSize = loSockStream.Recv(lsRecvBuf, 10, 
                            CSockStream::OPT_UNSPEC,loTimeValue);
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout1 - Recv failed.", 
                            (apl_ssize_t)10, liRecvSize);
                    CPPUNIT_ASSERT_MESSAGE("testRecvTimeout1 - Recv failed.", 
                            apl_strcmp(lsRecvBuf, "1234567890") == 0);
                }
                else
                {
                    liRecvSize = loSockStream.Recv(lsRecvBuf, 10,
                            CSockStream::OPT_UNSPEC,loTimeValue);
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout1 - Recv timeout failed.",
                            (apl_ssize_t)-1, liRecvSize);
                    liTmp = 1;
                }
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_handle_t liHandle = -1;
        char const* lpcBuf = "1234567890abcdefghijklmn";
        BEGIN_CLI(liHandle, 19373)
        {
            apl_ssize_t liSendSize = apl_send(liHandle, (void*)lpcBuf, apl_strlen(lpcBuf), 0);
            CPPUNIT_ASSERT_MESSAGE("testRecvTimeout1 - Send failed.", 
                    (apl_ssize_t)apl_strlen(lpcBuf) == liSendSize);
        }
        END_CLI(liHandle)

        BEGIN_CLI(liHandle, 19373)
        {
            WAIT_EXP(liTmp == 1);
        }
        END_CLI(liHandle)
        //end environment
    }

    void testRecvTimeout2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;

            apl_int_t liFlag = 1;
            BEGIN_SERV(CSockStream, loSockStream, 19374)
            {
                char lsRecvBuf[50];
                memset(lsRecvBuf, 0x0, sizeof(lsRecvBuf));

                apl_ssize_t liRecvSize = -1;

                CTimeValue loTimeValue(1);
                
                if(liFlag--)
                {
                    //case
                    liRecvSize = loSockStream.Recv(lsRecvBuf, 10, loTimeValue);
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout2 - Recv failed.", 
                            (apl_ssize_t)10, liRecvSize);
                    CPPUNIT_ASSERT_MESSAGE("testRecvTimeout2 - Recv failed.", 
                            apl_strcmp(lsRecvBuf, "1234567890") == 0);
                }
                else
                {
                    liRecvSize = loSockStream.Recv(lsRecvBuf, 10, loTimeValue);
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout2 - Recv timeout failed.",
                            (apl_ssize_t)-1, liRecvSize);
                    liTmp = 1;
                }
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_handle_t liHandle = -1;
        char const* lpcBuf = "1234567890abcdefghijklmn";
        BEGIN_CLI(liHandle, 19374)
        {
            apl_ssize_t liSendSize = apl_send(liHandle, (void*)lpcBuf, apl_strlen(lpcBuf), 0);
            CPPUNIT_ASSERT_MESSAGE("testRecvTimeout2 - Send failed.", 
                    (apl_ssize_t)apl_strlen(lpcBuf) == liSendSize);
        }
        END_CLI(liHandle)

        BEGIN_CLI(liHandle, 19374)
        {
            WAIT_EXP(liTmp == 1);
        }
        END_CLI(liHandle)

        //end environment
    }

    void testSend1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        apl_int_t liRet = -1;

        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;

            BEGIN_SERV(CSockStream, loSockStream, 19375)
            {
                char lsRecvBuf[50];
                memset(lsRecvBuf, 0x0, sizeof(lsRecvBuf));
                char const*lpcSendBuf = "1234567890abcdefghijklmn";

                apl_ssize_t liRecvSize = -1;
                liRecvSize = loSockStream.Recv(lsRecvBuf, sizeof(lsRecvBuf),
                        CSockStream::OPT_UNSPEC);                           
                CPPUNIT_ASSERT_MESSAGE("testSend1 - Recv failed.",           
                        liRecvSize == (apl_ssize_t)apl_strlen(lpcSendBuf));  
                CPPUNIT_ASSERT_MESSAGE("testSend1 - Recv failed.",           
                        apl_strcmp(lpcSendBuf, lsRecvBuf) == 0);            
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        CSockAddr loRemoteAddr(19375, "127.0.0.1", APL_AF_INET);
        CSockStream loSockStream;
        const char *lpcBuf = "1234567890abcdefghijklmn";

        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSend1 - Create socket failed.", (apl_int_t)0, liRet);
        liRet = apl_connect(loSockStream.GetHandle(), loRemoteAddr.GetAddr(), 
                loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSend1 - Connect socket failed.", (apl_int_t)0, liRet);

        //case
        apl_ssize_t liSendSize = loSockStream.Send((void*)lpcBuf, apl_strlen(lpcBuf), 
                CSockStream::OPT_UNSPEC);
        CPPUNIT_ASSERT_MESSAGE("testSend1 - Send failed.", (apl_ssize_t)apl_strlen(lpcBuf) == liSendSize);

        //end environment
        loSockStream.Close();
    }

    void testSend2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        apl_int_t liRet = -1;

        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;

            BEGIN_SERV(CSockStream, loSockStream, 19376)
            {
                char lsRecvBuf[50];
                memset(lsRecvBuf, 0x0, sizeof(lsRecvBuf));
                char const*lpcSendBuf = "1234567890abcdefghijklmn";

                apl_ssize_t liRecvSize = -1;
                liRecvSize = loSockStream.Recv(lsRecvBuf, sizeof(lsRecvBuf),
                        CSockStream::OPT_UNSPEC);                           
                CPPUNIT_ASSERT_MESSAGE("testSend2 - Recv failed.",           
                        liRecvSize == (apl_ssize_t)apl_strlen(lpcSendBuf));  
                CPPUNIT_ASSERT_MESSAGE("testSend2 - Recv failed.",           
                        apl_strcmp(lpcSendBuf, lsRecvBuf) == 0);            
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        CSockAddr loRemoteAddr(19376, "127.0.0.1", APL_AF_INET);
        CSockStream loSockStream;
        const char *lpcBuf = "1234567890abcdefghijklmn";

        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSend2 - Create socket failed.", (apl_int_t)0, liRet);
        liRet = apl_connect(loSockStream.GetHandle(), loRemoteAddr.GetAddr(), 
                loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSend2 - Connect socket failed.", (apl_int_t)0, liRet);

        //case
        apl_ssize_t liSendSize = loSockStream.Send((void*)lpcBuf, apl_strlen(lpcBuf));
        CPPUNIT_ASSERT_MESSAGE("testSend2 - Send failed.", (apl_ssize_t)apl_strlen(lpcBuf) == liSendSize);

        //end environment
        loSockStream.Close();
    }

    void testSendTimeout1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;
            apl_int_t liFlag = 1;
                                                        
            BEGIN_SERV(CSockStream, loSockStream, 19377)
            {
                if(liFlag--)
                {
                    char lsRecvBuf[50];
                    memset(lsRecvBuf, 0x0, sizeof(lsRecvBuf));                  
                    char const*lpcSendBuf = "1234567890abcdefghijklmn";

                    apl_ssize_t liRecvSize = -1;
                    liRecvSize = loSockStream.Recv(lsRecvBuf, sizeof(lsRecvBuf),
                            CSockStream::OPT_UNSPEC);
                    CPPUNIT_ASSERT_MESSAGE("testSendTimeout1 - Recv failed.",           
                            liRecvSize == (apl_ssize_t)apl_strlen(lpcSendBuf));
                    CPPUNIT_ASSERT_MESSAGE("testSendTimeout1 - Recv failed.",
                            apl_strcmp(lpcSendBuf, lsRecvBuf) == 0);       

                    loSockStream.Close();
                }
                else
                {
                    WAIT_EXP(liTmp == 1);
                    loSockStream.Close();
                }
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        CSockAddr loRemoteAddr(19377, "127.0.0.1", APL_AF_INET);
        CSockStream loSockStream;
        const char *lpcBuf = "1234567890abcdefghijklmn";
        CTimeValue loTimeValue(1);

        apl_int_t liRet = -1;
        apl_ssize_t liSendSize = -1;

        //case
        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout1 - Create socket failed.", (apl_int_t)0, liRet);
        liRet = apl_connect(loSockStream.GetHandle(), loRemoteAddr.GetAddr(), loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout1 - Connect socket failed.", (apl_int_t)0, liRet);

        liSendSize = loSockStream.Send((void*)lpcBuf, apl_strlen(lpcBuf), 
                CSockStream::OPT_UNSPEC, loTimeValue);
        CPPUNIT_ASSERT_MESSAGE("testSendTimeout1 - Send failed.", (apl_ssize_t)apl_strlen(lpcBuf) == liSendSize);
        apl_sleep(APL_TIME_SEC);
        //end environment
        loSockStream.Close();

        //case
        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout1 - Create socket failed.", (apl_int_t)0, liRet);
        liRet = apl_connect(loSockStream.GetHandle(), loRemoteAddr.GetAddr(), loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout1 - Connect socket failed.", (apl_int_t)0, liRet);
        //loSockStream.SetOption(CSockHandle::OPT_SNDBUF, 50);

        //apl_int_t liFlag = 200;
        while(true)
        {
            liSendSize = loSockStream.Send((void*)lpcBuf, apl_strlen(lpcBuf), 
                    CSockStream::OPT_UNSPEC, loTimeValue);
            if(liSendSize == -1) break;
        }
        liTmp = 1;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout1 - Test Send timeout failed.", (apl_ssize_t)-1, liSendSize);
        apl_sleep(APL_TIME_SEC);

        //end environment
        loSockStream.Close();
    }

    void testSendTimeout2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;
            apl_int_t liFlag = 1;
                                                        
            BEGIN_SERV(CSockStream, loSockStream, 19378)
            {
                if(liFlag--)
                {
                    char lsRecvBuf[50];
                    memset(lsRecvBuf, 0x0, sizeof(lsRecvBuf));                  
                    char const*lpcSendBuf = "1234567890abcdefghijklmn";

                    apl_ssize_t liRecvSize = -1;
                    liRecvSize = loSockStream.Recv(lsRecvBuf, sizeof(lsRecvBuf),
                            CSockStream::OPT_UNSPEC);
                    CPPUNIT_ASSERT_MESSAGE("testSendTimeout2 - Recv failed.",           
                            liRecvSize == (apl_ssize_t)apl_strlen(lpcSendBuf));
                    CPPUNIT_ASSERT_MESSAGE("testSendTimeout2 - Recv failed.",
                            apl_strcmp(lpcSendBuf, lsRecvBuf) == 0);       

                    loSockStream.Close();
                }
                else
                {
                    WAIT_EXP(liTmp == 1);
                    loSockStream.Close();
                }
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        CSockAddr loRemoteAddr(19378, "127.0.0.1", APL_AF_INET);
        CSockStream loSockStream;
        const char *lpcBuf = "1234567890abcdefghijklmn";
        CTimeValue loTimeValue(1);

        apl_int_t liRet = -1;
        apl_ssize_t liSendSize = -1;

        //case
        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout2 - Create socket failed.", (apl_int_t)0, liRet);
        liRet = apl_connect(loSockStream.GetHandle(), loRemoteAddr.GetAddr(), loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout2 - Connect socket failed.", (apl_int_t)0, liRet);

        liSendSize = loSockStream.Send((void*)lpcBuf, apl_strlen(lpcBuf), loTimeValue);
        CPPUNIT_ASSERT_MESSAGE("testSendTimeout2 - Send failed.", (apl_ssize_t)apl_strlen(lpcBuf) == liSendSize);
        apl_sleep(APL_TIME_SEC);

        //end environment
        loSockStream.Close();

        //case
        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout2 - Create socket failed.", (apl_int_t)0, liRet);
        liRet = apl_connect(loSockStream.GetHandle(), loRemoteAddr.GetAddr(), loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout2 - Connect socket failed.", (apl_int_t)0, liRet);
        //loSockStream.SetOption(CSockHandle::OPT_SNDBUF, 10);

        while(true)
        {
            liSendSize = loSockStream.Send((void*)lpcBuf, apl_strlen(lpcBuf), loTimeValue);
            if(liSendSize == -1) break;
        }
        liTmp = 1;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout2 - Test Send timeout failed.", (apl_ssize_t)-1, liSendSize);
        apl_sleep(APL_TIME_SEC);

        //end environment
        loSockStream.Close();
    }

    void testRecvUntil(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)     
        {                                               
            CSockStream loSockStream;                   
            BEGIN_SERV(CSockStream, loSockStream, 19379)
            {
                char lsRecvBuf[50];
                const char *lpcSendBuf = "1234567890abcdefghijklmn";

                CTimeValue loTimeValue(1,APL_TIME_SEC);

                //case
                apl_ssize_t liPeekSize = loSockStream.RecvUntil(lsRecvBuf, sizeof(lsRecvBuf), 
                        "ab", CSockStream::OPT_PEEK,loTimeValue);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvUntil - RecvUntil failed.",
                        (apl_ssize_t)12, liPeekSize);
                CPPUNIT_ASSERT_MESSAGE("TestRecvUntilServ - Recv failed.", 
                        apl_strcmp(lpcSendBuf, lsRecvBuf) == 0);


                apl_ssize_t liRecvSize = loSockStream.RecvUntil(lsRecvBuf, sizeof(lsRecvBuf), 
                        "ab", CSockStream::OPT_UNSPEC,loTimeValue);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("TestRecvUntilServ - RecvUntil failed.", 
                        (apl_ssize_t)12, liRecvSize);
                CPPUNIT_ASSERT_MESSAGE("TestRecvUntilServ - RecvUntil failed.", 
                        apl_strcmp(lpcSendBuf, lsRecvBuf) == 0);
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)
        apl_sleep(100*APL_TIME_MSEC);

        CSockAddr loRemoteAddr(19379, "127.0.0.1", APL_AF_INET);
        CSockStream loSockStream;
        const char *lpcBuf = "1234567890abcdefghijklmn";

        apl_int_t liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvUntil - Create socket failed.", (apl_int_t)0, liRet);
        liRet = apl_connect(loSockStream.GetHandle(), loRemoteAddr.GetAddr(), 
                loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvUntil - Connect socket failed.", (apl_int_t)0, liRet);
        apl_ssize_t liSendSize = loSockStream.Send((void*)lpcBuf, apl_strlen(lpcBuf), 
                CSockStream::OPT_UNSPEC);
        CPPUNIT_ASSERT_MESSAGE("testRecvUntil - Send failed.", (apl_ssize_t)apl_strlen(lpcBuf) == liSendSize);

        //end environment
        loSockStream.Close();
    }

    void testRecvV(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;
            BEGIN_SERV(CSockStream, loSockStream, 10010)
            {
                char lsIovSend1[] = "1234567890";
                char lsIovSend2[] = "abcdefghij";

                char lsIovRecv1[11];
                char lsIovRecv2[11];
                apl_memset(lsIovRecv1, 0x0, sizeof(lsIovRecv1));
                apl_memset(lsIovRecv2, 0x0, sizeof(lsIovRecv2));
                struct apl_iovec_t loIov[2];
                loIov[0].iov_base = lsIovRecv1;
                loIov[0].iov_len = sizeof(lsIovRecv1)-1;
                loIov[1].iov_base = lsIovRecv2;
                loIov[1].iov_len = sizeof(lsIovRecv2)-1;

                apl_ssize_t liRecvSize = -1;

                //case
                liRecvSize  = loSockStream.RecvV(loIov, sizeof(loIov)/sizeof(loIov[0]));
                CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvV - RecvV failed.", (apl_ssize_t)20, liRecvSize);
                CPPUNIT_ASSERT_MESSAGE("testRecvV - RecvV failed.", apl_strcmp(lsIovSend1, lsIovRecv1)==0);
                CPPUNIT_ASSERT_MESSAGE("testRecvV - RecvV failed.", apl_strcmp(lsIovSend2, lsIovRecv2)==0);
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_handle_t liHandle = -1;
        char lsIov1[] = "1234567890";
        char lsIov2[] = "abcdefghij";
        struct apl_iovec_t loIov[2];
        loIov[0].iov_base = lsIov1;
        loIov[0].iov_len = apl_strlen(lsIov1);
        loIov[1].iov_base = lsIov2;
        loIov[1].iov_len = apl_strlen(lsIov2);

        BEGIN_CLI(liHandle, 10010)
        {
            apl_ssize_t liSendSize = apl_writev(liHandle, loIov, sizeof(loIov)/sizeof(loIov[0]));
            CPPUNIT_ASSERT_MESSAGE("testRecvV - Send failed.",  
                    (apl_ssize_t)(loIov[0].iov_len + loIov[1].iov_len) == liSendSize);
        }
        END_CLI(liHandle)

        //end environment
    }

    void testRecvVTimeout(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;
            apl_int_t liFlag = 1;
            BEGIN_SERV(CSockStream, loSockStream, 10011)
            {
                char lsIovSend1[] = "1234567890";
                char lsIovSend2[] = "abcdefghij";

                char lsIovRecv1[11];
                char lsIovRecv2[11];
                apl_memset(lsIovRecv1, 0x0, sizeof(lsIovRecv1));
                apl_memset(lsIovRecv2, 0x0, sizeof(lsIovRecv2));
                struct apl_iovec_t loIov[2];
                loIov[0].iov_base = lsIovRecv1;
                loIov[0].iov_len = sizeof(lsIovRecv1)-1;
                loIov[1].iov_base = lsIovRecv2;
                loIov[1].iov_len = sizeof(lsIovRecv2)-1;

                apl_ssize_t liRecvSize = -1;

                //case
                if(liFlag--)
                {
                    CTimeValue loTimeValue(3, APL_TIME_SEC);
                    liRecvSize = loSockStream.RecvV(loIov, 
                            sizeof(loIov)/sizeof(loIov[0]),loTimeValue);
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvVTimeout - RecvV failed.", 
                            (apl_ssize_t)20, liRecvSize);
                    CPPUNIT_ASSERT_MESSAGE("testRecvVTimeout - RecvV failed.", 
                            apl_strcmp(lsIovSend1, lsIovRecv1)==0);
                    CPPUNIT_ASSERT_MESSAGE("testRecvVTimeout - RecvV failed.", 
                            apl_strcmp(lsIovSend2, lsIovRecv2)==0);
                }
                else
                {
                    CTimeValue loTimeValue(1, APL_TIME_SEC);
                    liRecvSize = loSockStream.RecvV(loIov, 
                            sizeof(loIov)/sizeof(loIov[0]),loTimeValue);
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvVTimeout - RecvV timeout failed.",
                            (apl_ssize_t)-1, liRecvSize);
                    liTmp = 1;
                }

            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_handle_t liHandle = -1;
        char lsIov1[] = "1234567890";
        char lsIov2[] = "abcdefghij";
        struct apl_iovec_t loIov[2];
        loIov[0].iov_base = lsIov1;
        loIov[0].iov_len = apl_strlen(lsIov1);
        loIov[1].iov_base = lsIov2;
        loIov[1].iov_len = apl_strlen(lsIov2);

        BEGIN_CLI(liHandle, 10011)
        {
            apl_ssize_t liSendSize = apl_writev(liHandle, loIov, sizeof(loIov)/sizeof(loIov[0]));
            CPPUNIT_ASSERT_MESSAGE("testRecvVTimeout - apl_writev failed.",  
                    (apl_ssize_t)(loIov[0].iov_len + loIov[1].iov_len) == liSendSize);
        }
        END_CLI(liHandle)

        apl_sleep(100*APL_TIME_MSEC);

        BEGIN_CLI(liHandle, 10011)
        {
            WAIT_EXP(liTmp == 1);
        }
        END_CLI(liHandle)

        //end environment
    }

    void testSendV(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        char lsIovSend1[] = "1234567890";
        char lsIovSend2[] = "abcdefghij";
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;
            BEGIN_SERV(CSockStream, loSockStream, 10012)
            {
                char lsIovRecv1[11];
                char lsIovRecv2[11];
                apl_memset(lsIovRecv1, 0x0, sizeof(lsIovRecv1));
                apl_memset(lsIovRecv2, 0x0, sizeof(lsIovRecv2));
                struct apl_iovec_t loIov[2];
                loIov[0].iov_base = lsIovRecv1;
                loIov[0].iov_len = sizeof(lsIovRecv1)-1;
                loIov[1].iov_base = lsIovRecv2;
                loIov[1].iov_len = sizeof(lsIovRecv2)-1;

                apl_ssize_t liRecvSize = -1;
                liRecvSize  = loSockStream.RecvV(loIov, sizeof(loIov)/sizeof(loIov[0]));
                CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendV - RecvV failed.", (apl_ssize_t)20, liRecvSize);
                CPPUNIT_ASSERT_MESSAGE("testSendV - RecvV failed.", apl_strcmp("1234567890", lsIovRecv1)==0);
                CPPUNIT_ASSERT_MESSAGE("testSendV - RecvV failed.", apl_strcmp("abcdefghij", lsIovRecv2)==0);
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_int_t liRet = -1;
        CSockAddr loRemoteAddr(10012, "127.0.0.1", APL_AF_INET);
        CSockStream loSockStream;

        struct apl_iovec_t loIov[2];
        loIov[0].iov_base = lsIovSend1;
        loIov[0].iov_len = apl_strlen(lsIovSend1);
        loIov[1].iov_base = lsIovSend2;
        loIov[1].iov_len = apl_strlen(lsIovSend2);

        //case
        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendV - Create socket failed.", (apl_int_t)0, liRet);
        liRet = apl_connect(loSockStream.GetHandle(), 
                loRemoteAddr.GetAddr(), loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendV - Connect socket failed.", (apl_int_t)0, liRet);
        apl_ssize_t liSendSize = loSockStream.SendV(loIov, sizeof(loIov)/sizeof(loIov[0]));
        CPPUNIT_ASSERT_MESSAGE("testSendV - Send failed.",  
                (apl_ssize_t)(loIov[0].iov_len + loIov[1].iov_len) == liSendSize);

        //end environment
        loSockStream.Close();
    }

    void testSendVTimeout(void)
    {
        PRINT_TITLE_2(__func__); 

        //start case environment
        apl_int_t liTmp = 0;
        char lsIovSend1[] = "1234567890";
        char lsIovSend2[] = "abcdefghij";
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;
            apl_int_t liFlag = 1;
            BEGIN_SERV(CSockStream, loSockStream, 10013)
            {
                char lsIovRecv1[11];
                char lsIovRecv2[11];
                apl_memset(lsIovRecv1, 0x0, sizeof(lsIovRecv1));
                apl_memset(lsIovRecv2, 0x0, sizeof(lsIovRecv2));
                struct apl_iovec_t loIov[2];
                loIov[0].iov_base = lsIovRecv1;
                loIov[0].iov_len = sizeof(lsIovRecv1)-1;
                loIov[1].iov_base = lsIovRecv2;
                loIov[1].iov_len = sizeof(lsIovRecv2)-1;

                if(liFlag--)
                {
                    apl_ssize_t liRecvSize = -1;
                    liRecvSize  = loSockStream.RecvV(loIov, sizeof(loIov)/sizeof(loIov[0]));
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendVTimeout - RecvV failed.", (apl_ssize_t)20, liRecvSize);
                    CPPUNIT_ASSERT_MESSAGE("testSendVTimeout - RecvV failed.", 
                            apl_strcmp("1234567890", lsIovRecv1)==0);
                    CPPUNIT_ASSERT_MESSAGE("testSendVTimeout - RecvV failed.", 
                            apl_strcmp("abcdefghij", lsIovRecv2)==0);
                }
                else
                {
                    WAIT_EXP(liTmp == 1);
                }
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_int_t liRet = -1;
        CSockAddr loRemoteAddr(10013, "127.0.0.1", APL_AF_INET);
        CSockStream loSockStream;

        struct apl_iovec_t loIov[2];
        loIov[0].iov_base = lsIovSend1;
        loIov[0].iov_len = apl_strlen(lsIovSend1);
        loIov[1].iov_base = lsIovSend2;
        loIov[1].iov_len = apl_strlen(lsIovSend2);

        //case
        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendVTimeout - Create socket failed.", (apl_int_t)0, liRet);
        liRet = apl_connect(loSockStream.GetHandle(), 
                loRemoteAddr.GetAddr(), loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendVTimeout - Connect socket failed.", (apl_int_t)0, liRet);
        CTimeValue loTimeValue(1, APL_TIME_SEC);
        apl_ssize_t liSendSize = -1;
        liSendSize = loSockStream.SendV(loIov, sizeof(loIov)/sizeof(loIov[0]),loTimeValue);
        CPPUNIT_ASSERT_MESSAGE("testSendVTimeout - Send failed.",  
                (apl_ssize_t)(loIov[0].iov_len + loIov[1].iov_len) == liSendSize);
        //end environment
        loSockStream.Close();

        //case
        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendVTimeout - Create socket failed.", (apl_int_t)0, liRet);
        liRet = apl_connect(loSockStream.GetHandle(), 
                loRemoteAddr.GetAddr(), loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendVTimeout - Connect socket failed.", (apl_int_t)0, liRet);
        while(true)
        {
            liSendSize = loSockStream.SendV(loIov, sizeof(loIov)/sizeof(loIov[0]),loTimeValue);
            if(liSendSize == -1) break;
        }
        liTmp = 1;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendVTimeout - SendV timeout failed.", 
                (apl_ssize_t)-1, liSendSize);
        //end environment
        loSockStream.Close();
    }

};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclSockStream);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

