#include "Test.h"
#include "acl/SockAcceptor.h"

using namespace acl;

#define SERVPORT 10001

class CTestAclSockAcceptor:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclSockAcceptor);
    CPPUNIT_TEST(testOpen);
    CPPUNIT_TEST(testAccept);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testOpen(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CSockAddr loLocalAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockAcceptor loSockAcceptor;

        //case
        apl_int_t liRet = loSockAcceptor.Open(loLocalAddr, APL_AF_INET, 10, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open failed.", (apl_int_t)0, liRet);

        //end environment
        loSockAcceptor.Close();
    }

    void testAccept(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;

        //case
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            apl_int_t liRet = -1;
            CSockAddr loLocalAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAcceptor loSockAcceptor;
            CSockStream loSockStream;
            CTimeValue loTimeValue;

            loTimeValue.Sec(2);
            liRet = loSockAcceptor.Open(loLocalAddr, APL_AF_INET, 10, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Open failed.", (apl_int_t)0, liRet);
            
            liTmp = 1;
            apl_int_t liFlag = 1;
            while(true)
            {
                liRet = loSockAcceptor.Accept(loSockStream, NULL, loTimeValue);
                if(liFlag--)
                {
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Accept failed.", (apl_int_t)0, liRet);

                    char lsRecvBuf[50];                                         
                    memset(lsRecvBuf, 0x0, sizeof(lsRecvBuf));                  
                    char const*lpcSendBuf = "1234567890abcdefghijklmn";         

                    apl_ssize_t liRecvSize = -1;                                
                    liRecvSize = loSockStream.Recv(lsRecvBuf, sizeof(lsRecvBuf),
                            CSockStream::OPT_UNSPEC);                           
                    CPPUNIT_ASSERT_MESSAGE("testRecv1- Recv failed.",           
                            liRecvSize == (apl_ssize_t)apl_strlen(lsRecvBuf));  
                    CPPUNIT_ASSERT_MESSAGE("testRecv1- Recv failed.",           
                            apl_strcmp(lpcSendBuf, lsRecvBuf) == 0);            
                                                                                
                    loSockStream.Close();
                }
                else
                {
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Test accept timeout failed.", 
                            (apl_int_t)-1, liRet);
                    loSockStream.Close();
                    break;
                }
            }
            loSockAcceptor.Close();
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        WAIT_EXP( liTmp == 1 );

        apl_int_t liRet = -1;
        char const *lpcBuf = "1234567890abcdefghijklmn";                                              
        CTimeValue loTimeValue(2);                                                              

        CSockAddr loRemoteAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockStream loSockStream;

        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Create socket failed.", (apl_int_t)0, liRet);
        liRet = apl_connect(loSockStream.GetHandle(), loRemoteAddr.GetAddr(), loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Connect socket failed.", (apl_int_t)0, liRet);

        apl_ssize_t liSendSize = loSockStream.Send((void*)lpcBuf, apl_strlen(lpcBuf),           
                CSockStream::OPT_UNSPEC, loTimeValue);                                          
        CPPUNIT_ASSERT_MESSAGE("Send failed.", (apl_ssize_t)apl_strlen(lpcBuf) == liSendSize);  
        apl_sleep(APL_TIME_SEC);                                                                

        apl_sleep(2*APL_TIME_SEC);

        //end environment
        loSockStream.Close();
    }
};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclSockAcceptor);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

