#include "Test.h"
#include "acl/SockDgram.h"

using namespace acl;

#define SERVPORT  10001
#define CliPORT   10002
#define BUFSIZE   50

/* ------------------------------------------------------------------ */
class CTestAclSockDgram:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclSockDgram);
    CPPUNIT_TEST(testOpen);
    CPPUNIT_TEST(testRecv1);
    CPPUNIT_TEST(testRecv2);
    CPPUNIT_TEST(testRecvTimeout1);
    CPPUNIT_TEST(testRecvTimeout2);
    CPPUNIT_TEST(testSend1);
    CPPUNIT_TEST(testSend2);
    CPPUNIT_TEST(testSendTimeout1);
    CPPUNIT_TEST(testSendTimeout2);
    CPPUNIT_TEST(testRecvUntil1);
    CPPUNIT_TEST(testRecvUntil2);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testOpen(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockAddr loServAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAddr loSourAddr;
            apl_int_t liRet = -1;
            CSockDgram loSockDgram;

            //case
            liRet = loSockDgram.Open(loServAddr, APL_AF_INET, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testOpen - Create socket server failed.", (apl_int_t)0, liRet);

            char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
            char lsRecvBuf[BUFSIZE];

            memset(lsRecvBuf, 0, BUFSIZE);
            apl_ssize_t liLen = loSockDgram.Recv(lsRecvBuf, BUFSIZE, &loSourAddr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testOpen - Recv failed.", (apl_ssize_t)apl_strlen(lsRecvBuf), liLen);
            CPPUNIT_ASSERT_MESSAGE("testOpen - Recv failed.", apl_strcmp(lsRecvBuf, lsSendBuf) == 0);
            loSockDgram.Close();
            liTmp = 1;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        CSockAddr loDestAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockAddr loLocalAddr(CliPORT, "127.0.0.1", APL_AF_INET);
        CSockDgram loSockDgram;

        apl_int_t liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOpen - Create socket client failed.", (apl_int_t)0, liRet);
        char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
        apl_ssize_t liSendLen = loSockDgram.Send(lsSendBuf, apl_strlen(lsSendBuf), loDestAddr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOpen - Send failed.", (apl_ssize_t)apl_strlen(lsSendBuf), liSendLen);

        //end environment
        loSockDgram.Close();
        WAIT_EXP(liTmp == 1);
    }

    void testRecv1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockAddr loServAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAddr loSourAddr;
            apl_int_t liRet = -1;
            CSockDgram loSockDgram;

            liRet = loSockDgram.Open(loServAddr, APL_AF_INET, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecv1 - Create socket server failed.", (apl_int_t)0, liRet);

            char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
            char lsRecvBuf[BUFSIZE];

            //case
            memset(lsRecvBuf, 0, BUFSIZE);
            apl_ssize_t liPeekLen = loSockDgram.Recv(lsRecvBuf, BUFSIZE, &loSourAddr, CSockDgram::OPT_PEEK);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecv1 - Recv failed.", (apl_ssize_t)apl_strlen(lsRecvBuf), liPeekLen);
            CPPUNIT_ASSERT_MESSAGE("testRecv1 - Recv failed.", apl_strcmp(lsRecvBuf, lsSendBuf) == 0);

            memset(lsRecvBuf, 0, BUFSIZE);
            apl_ssize_t liRecvLen = loSockDgram.Recv(lsRecvBuf, BUFSIZE, &loSourAddr, CSockDgram::OPT_UNSPEC);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecv1 - Recv failed.", (apl_ssize_t)apl_strlen(lsRecvBuf), liRecvLen);
            CPPUNIT_ASSERT_MESSAGE("testRecv1 - Recv failed.", apl_strcmp(lsRecvBuf, lsSendBuf) == 0);

            //end environment
            loSockDgram.Close();
            liTmp = 1;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_int_t liRet = -1;
        CSockAddr loDestAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockAddr loLocalAddr(CliPORT, "127.0.0.1", APL_AF_INET);
        CSockDgram loSockDgram;

        liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecv1 - Create socket client failed.", (apl_int_t)0, liRet);

        char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
        apl_ssize_t liSendLen = apl_sendto(loSockDgram.GetHandle(), lsSendBuf, apl_strlen(lsSendBuf), CSockDgram::OPT_UNSPEC, loDestAddr.GetAddr(), loDestAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecv1 - Send failed.", (apl_ssize_t)apl_strlen(lsSendBuf), liSendLen);

        loSockDgram.Close();
        WAIT_EXP(liTmp == 1);
    }

    void testRecv2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            apl_int_t liRet = -1;
            CSockAddr loServAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAddr loSourAddr;

            CSockDgram loSockDgram;

            liRet = loSockDgram.Open(loServAddr, APL_AF_INET, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecv2 - Create socket server failed.", (apl_int_t)0, liRet);

            char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
            char lsRecvBuf[BUFSIZE];

            //case
            memset(lsRecvBuf, 0, BUFSIZE);
            apl_ssize_t liRecvLen = loSockDgram.Recv(lsRecvBuf, BUFSIZE, &loSourAddr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecv2 - Recv failed.", (apl_ssize_t)apl_strlen(lsRecvBuf), liRecvLen);
            CPPUNIT_ASSERT_MESSAGE("testRecv2 - Recv failed.", apl_strcmp(lsRecvBuf, lsSendBuf) == 0);

            //end environment
            loSockDgram.Close();
            liTmp = 1;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_int_t liRet = -1;
        CSockAddr loDestAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockAddr loLocalAddr(CliPORT, "127.0.0.1", APL_AF_INET);
        CSockDgram loSockDgram;

        liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecv2 - Create socket client failed.", (apl_int_t)0, liRet);

        char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
        apl_ssize_t liSendLen = apl_sendto(loSockDgram.GetHandle(), lsSendBuf, apl_strlen(lsSendBuf), CSockDgram::OPT_UNSPEC, loDestAddr.GetAddr(), loDestAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecv2 - Send failed.", (apl_ssize_t)apl_strlen(lsSendBuf), liSendLen);

        loSockDgram.Close();
        WAIT_EXP(liTmp == 1);
    }

    void testRecvTimeout1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp =0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            apl_int_t liRet = -1;
            CSockAddr loLocalAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAddr loSourAddr;

            CSockDgram loSockDgram;
            CTimeValue loTimeValue(1, APL_TIME_SEC);

            char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
            char lsRecvBuf[BUFSIZE];

            //case
            liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout1 - Create socket server failed.", (apl_int_t)0, liRet);
            memset(lsRecvBuf, 0, BUFSIZE);
            liTmp = 1;
            WAIT_EXP(liTmp == 2);
            apl_ssize_t liPeekLen = loSockDgram.Recv(lsRecvBuf, BUFSIZE, &loSourAddr, CSockDgram::OPT_PEEK, loTimeValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout1 - Recv peek failed.", (apl_ssize_t)apl_strlen(lsRecvBuf), liPeekLen);
            CPPUNIT_ASSERT_MESSAGE("testRecvTimeout1 - Recv peek failed.", apl_strcmp(lsRecvBuf, lsSendBuf) == 0);

            memset(lsRecvBuf, 0, BUFSIZE);
            apl_ssize_t liRecvLen = loSockDgram.Recv(lsRecvBuf, BUFSIZE, &loSourAddr,CSockDgram::OPT_UNSPEC, loTimeValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout1 - Recv unspec failed.", (apl_ssize_t)apl_strlen(lsRecvBuf), liRecvLen);
            CPPUNIT_ASSERT_MESSAGE("testRecvTimeout1 - Recv unspec failed.", apl_strcmp(lsRecvBuf, lsSendBuf) == 0);
            //end environment
            loSockDgram.Close();
            
            //case
            liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout1 - Create socket server failed.", (apl_int_t)0, liRet);
            memset(lsRecvBuf, 0, BUFSIZE);
            liRecvLen = loSockDgram.Recv(lsRecvBuf, BUFSIZE, &loSourAddr, CSockDgram::OPT_UNSPEC, loTimeValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout1 - Recv failed.",(apl_ssize_t)0, liRecvLen);
            CPPUNIT_ASSERT_MESSAGE("testRecvTimeout1 - Recv failed.", apl_strcmp(lsRecvBuf, lsSendBuf) != 0);
            //end environment
            loSockDgram.Close();
            liTmp = 3;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(100*APL_TIME_MSEC);
        apl_int_t liRet = -1;
        CSockAddr loDestAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockAddr loLocalAddr(CliPORT, "127.0.0.1", APL_AF_INET);
        CSockDgram loSockDgram;
        WAIT_EXP(liTmp == 1);
        liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout1 - Create socket client failed.", (apl_int_t)0, liRet);

        char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
        apl_ssize_t liSendLen = apl_sendto(loSockDgram.GetHandle(), lsSendBuf, apl_strlen(lsSendBuf), CSockDgram::OPT_UNSPEC, loDestAddr.GetAddr(), loDestAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout1 - Send failed.", (apl_ssize_t)apl_strlen(lsSendBuf), liSendLen);
        liTmp = 2;
        loSockDgram.Close();

         WAIT_EXP(liTmp == 3);
    }

    void testRecvTimeout2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            apl_int_t liRet = -1;
            CSockAddr loLocalAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAddr loSourAddr;
            CSockDgram loSockDgram;
            CTimeValue loTimeValue(1, APL_TIME_SEC);
            char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
            char lsRecvBuf[BUFSIZE];

            //case
            liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout2 - Create socket server failed.", (apl_int_t)0, liRet);
            memset(lsRecvBuf, 0, BUFSIZE);
            apl_ssize_t liRecvLen = loSockDgram.Recv(lsRecvBuf, BUFSIZE, &loSourAddr, loTimeValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout2 - Recv unspec failed.", (apl_ssize_t)apl_strlen(lsRecvBuf), liRecvLen);
            CPPUNIT_ASSERT_MESSAGE("testRecvTimeout2 - Recv unspec failed.", apl_strcmp(lsRecvBuf, lsSendBuf) == 0);
            //end environment
            loSockDgram.Close();

            //case
            liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout2 - Create socket server failed.", (apl_int_t)0, liRet);
            memset(lsRecvBuf, 0, BUFSIZE);
            liRecvLen = loSockDgram.Recv(lsRecvBuf, BUFSIZE, &loSourAddr, loTimeValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout2 - Recv failed.",(apl_ssize_t)0, liRecvLen);
            CPPUNIT_ASSERT_MESSAGE("testRecvTimeout2 - Recv failed.", apl_strcmp(lsRecvBuf, lsSendBuf) != 0);
            //end environment
            loSockDgram.Close();
            liTmp = 1;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_int_t liRet = -1;
        CSockAddr loDestAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockAddr loLocalAddr(CliPORT, "127.0.0.1", APL_AF_INET);
        CSockDgram loSockDgram;

        liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout2 - Create socket client failed.", (apl_int_t)0, liRet);

        char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
        apl_ssize_t liSendLen = apl_sendto(loSockDgram.GetHandle(), lsSendBuf, apl_strlen(lsSendBuf), CSockDgram::OPT_UNSPEC, loDestAddr.GetAddr(), loDestAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvTimeout2 - Send failed.", (apl_ssize_t)apl_strlen(lsSendBuf), liSendLen);

        loSockDgram.Close();
        WAIT_EXP(liTmp == 1);
    }

    void testSend1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            apl_int_t liRet = -1;
            CSockAddr loServAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAddr loSourAddr;

            CSockDgram loSockDgram;

            liRet = loSockDgram.Open(loServAddr, APL_AF_INET, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testSend1 - Create socket server failed.", (apl_int_t)0, liRet);

            char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
            char lsRecvBuf[BUFSIZE];
            memset(lsRecvBuf, 0, BUFSIZE);
            apl_ssize_t liRecvLen = loSockDgram.Recv(lsRecvBuf, BUFSIZE, &loSourAddr, CSockDgram::OPT_UNSPEC);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testSend1 - Recv failed.", (apl_ssize_t)apl_strlen(lsRecvBuf), liRecvLen);
            CPPUNIT_ASSERT_MESSAGE("testSend1 - Recv failed.", apl_strcmp(lsRecvBuf, lsSendBuf) == 0);
            loSockDgram.Close();
            liTmp = 1;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_int_t liRet = -1;
        CSockAddr loDestAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockAddr loLocalAddr(CliPORT, "127.0.0.1", APL_AF_INET);
        CSockDgram loSockDgram;

        liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSend1 - Create socket client failed.", (apl_int_t)0, liRet);

        char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
        //case
        apl_ssize_t liSendLen = loSockDgram.Send(lsSendBuf, apl_strlen(lsSendBuf), loDestAddr, CSockDgram::OPT_UNSPEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSend1 - Send failed.", (apl_ssize_t)apl_strlen(lsSendBuf), liSendLen);

        //end environment
        loSockDgram.Close();

        WAIT_EXP(liTmp == 1);
    }

    void testSend2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            apl_int_t liRet = -1;
            CSockAddr loServAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAddr loSourAddr;

            CSockDgram loSockDgram;

            liRet = loSockDgram.Open(loServAddr, APL_AF_INET, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testSend2 - Create socket server failed.", (apl_int_t)0, liRet);

            char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
            char lsRecvBuf[BUFSIZE];
            memset(lsRecvBuf, 0, BUFSIZE);
            apl_ssize_t liRecvLen = loSockDgram.Recv(lsRecvBuf, BUFSIZE, &loSourAddr, CSockDgram::OPT_UNSPEC);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testSend2 - Recv failed.", (apl_ssize_t)apl_strlen(lsRecvBuf), liRecvLen);
            CPPUNIT_ASSERT_MESSAGE("testSend2 - Recv failed.", apl_strcmp(lsRecvBuf, lsSendBuf) == 0);
            loSockDgram.Close();
            liTmp = 1;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_int_t liRet = -1;
        CSockAddr loDestAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockAddr loLocalAddr(CliPORT, "127.0.0.1", APL_AF_INET);
        CSockDgram loSockDgram;

        liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSend2 - Create socket client failed.", (apl_int_t)0, liRet);
        char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";

        //case
        apl_ssize_t liSendLen = loSockDgram.Send(lsSendBuf, apl_strlen(lsSendBuf), loDestAddr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSend2 - Send failed.", (apl_ssize_t)apl_strlen(lsSendBuf), liSendLen);

        //end environment
        loSockDgram.Close();
        WAIT_EXP(liTmp == 1);
    }

    void testSendTimeout1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            apl_int_t liRet = -1;
            CSockAddr loServAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAddr loSourAddr;

            CSockDgram loSockDgram;

            liRet = loSockDgram.Open(loServAddr, APL_AF_INET, 0);
            if ( liRet != 0 )
            {
                printf("Open sockdgram fail - msg:%s\n", strerror(errno));
                CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout1 - Create socket server failed.", (apl_int_t)0, liRet);
            }
            char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
            char lsRecvBuf[BUFSIZE];
            memset(lsRecvBuf, 0, BUFSIZE);
            apl_ssize_t liRecvLen = loSockDgram.Recv(lsRecvBuf, BUFSIZE, &loSourAddr, CSockDgram::OPT_UNSPEC);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout1 - Recv failed.", (apl_ssize_t)apl_strlen(lsRecvBuf), liRecvLen);
            CPPUNIT_ASSERT_MESSAGE("testSendTimeout1 - Recv failed.", apl_strcmp(lsRecvBuf, lsSendBuf) == 0);
            loSockDgram.Close();

            liTmp = 1;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_int_t liRet = -1;
        CSockAddr loDestAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockAddr loLocalAddr(CliPORT, "127.0.0.1", APL_AF_INET);
        CSockDgram loSockDgram;
        CTimeValue loTimeValue(1,APL_TIME_SEC);

        liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout1 - Create socket client failed.", (apl_int_t)0, liRet);
        char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";

        //case
        apl_ssize_t liSendLen = loSockDgram.Send(lsSendBuf, apl_strlen(lsSendBuf), loDestAddr, CSockDgram::OPT_UNSPEC, loTimeValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout1 - Send failed.", (apl_ssize_t)apl_strlen(lsSendBuf), liSendLen);

        //end environment
        loSockDgram.Close();
        WAIT_EXP(liTmp == 1);
    }

    void testSendTimeout2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            apl_int_t liRet = -1;
            CSockAddr loServAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAddr loSourAddr;

            CSockDgram loSockDgram;

            liRet = loSockDgram.Open(loServAddr, APL_AF_INET, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout2 - Create socket server failed.", (apl_int_t)0, liRet);

            char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
            char lsRecvBuf[BUFSIZE];
            memset(lsRecvBuf, 0, BUFSIZE);
            apl_ssize_t liRecvLen = loSockDgram.Recv(lsRecvBuf, BUFSIZE, &loSourAddr, CSockDgram::OPT_UNSPEC);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout2 - Recv failed.", (apl_ssize_t)apl_strlen(lsRecvBuf), liRecvLen);
            CPPUNIT_ASSERT_MESSAGE("testSendTimeout2 - Recv failed.", apl_strcmp(lsRecvBuf, lsSendBuf) == 0);
            loSockDgram.Close();
            liTmp = 1;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_int_t liRet = -1;
        CSockAddr loDestAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockAddr loLocalAddr(CliPORT, "127.0.0.1", APL_AF_INET);
        CSockDgram loSockDgram;
        CTimeValue loTimeValue(1,APL_TIME_SEC);

        liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout2 - Create socket client failed.", (apl_int_t)0, liRet);

        char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";

        //case
        apl_ssize_t liSendLen = loSockDgram.Send(lsSendBuf, apl_strlen(lsSendBuf), loDestAddr, loTimeValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSendTimeout2 - Send failed.", (apl_ssize_t)apl_strlen(lsSendBuf), liSendLen);

        //end environment
        loSockDgram.Close();
        WAIT_EXP(liTmp == 1);
    }

    void testRecvUntil1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            apl_int_t liRet = -1;
            CSockAddr loLocalAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAddr loSourAddr;

            CTimeValue loTimeValue(2, APL_TIME_SEC);
            CSockDgram loSockDgram;

            liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvUntil1 - Create socket server failed.", (apl_int_t)0, liRet);

            char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
            char const* lpcTag = "mn"; 
            char lsRecvBuf[BUFSIZE];
            memset(lsRecvBuf, 0, BUFSIZE);

            //case
            apl_ssize_t liPeekLen = loSockDgram.RecvUntil(lsRecvBuf, BUFSIZE, &loSourAddr, lpcTag, CSockDgram::OPT_PEEK, loTimeValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvUntil1 - TestRecvUntilServ - Recv failed.", (apl_ssize_t)14, liPeekLen);
            CPPUNIT_ASSERT_MESSAGE("testRecvUntil1 - Recv failed.", apl_strncmp(lsRecvBuf, lsSendBuf, 14) == 0);

            memset(lsRecvBuf, 0, BUFSIZE);
            apl_ssize_t liRecvLen = loSockDgram.RecvUntil(lsRecvBuf, BUFSIZE, &loSourAddr, lpcTag, CSockDgram::OPT_UNSPEC, loTimeValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvUntil1 - Recv failed.", (apl_ssize_t)14, liRecvLen);
            CPPUNIT_ASSERT_MESSAGE("testRecvUntil1 - Recv failed.", apl_strncmp(lsRecvBuf, lsSendBuf, 14) == 0);

            //end environment
            loSockDgram.Close();
            liTmp = 1;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_int_t liRet = -1;
        CSockAddr loDestAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockAddr loLocalAddr(CliPORT, "127.0.0.1", APL_AF_INET);
        CSockDgram loSockDgram;

        liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvUntil1 - Create socket client failed.", (apl_int_t)0, liRet);

        char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
        apl_ssize_t liSendLen = loSockDgram.Send(lsSendBuf, apl_strlen(lsSendBuf), loDestAddr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvUntil1 - Send failed.", (apl_ssize_t)apl_strlen(lsSendBuf), liSendLen);
        loSockDgram.Close();
        WAIT_EXP(liTmp == 1);
    }

    void testRecvUntil2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            apl_int_t liRet = -1;
            CSockAddr loLocalAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAddr loSourAddr;

            CTimeValue loTimeValue(2, APL_TIME_SEC);
            CSockDgram loSockDgram;

            liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
            if ( liRet != 0 )
            {
                printf("Open sockdgram fail - msg:%s\n", strerror(errno));
                CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvUntil2 - Create socket server failed.", (apl_int_t)0, liRet);
            }

            char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
            char const* lpcTag = "mn"; 
            char lsRecvBuf[BUFSIZE];
            memset(lsRecvBuf, 0, BUFSIZE);

            //case
            apl_ssize_t liPeekLen = loSockDgram.RecvUntil(lsRecvBuf, BUFSIZE, &loSourAddr, lpcTag, CSockDgram::OPT_PEEK, loTimeValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvUntil2 - Recv failed.", (apl_ssize_t)14, liPeekLen);
            CPPUNIT_ASSERT_MESSAGE("testRecvUntil2 - Recv failed.", apl_strncmp(lsRecvBuf, lsSendBuf, 14) == 0);

            memset(lsRecvBuf, 0, BUFSIZE);
            apl_ssize_t liRecvLen = loSockDgram.RecvUntil(lsRecvBuf, BUFSIZE, &loSourAddr, lpcTag, CSockDgram::OPT_UNSPEC, loTimeValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvUntil2 - Recv failed.", (apl_ssize_t)14, liRecvLen);
            CPPUNIT_ASSERT_MESSAGE("testRecvUntil2 - Recv failed.", apl_strncmp(lsRecvBuf, lsSendBuf, 14) == 0);

            //end environment
            loSockDgram.Close();
            liTmp = 1;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_int_t liRet = -1;
        CSockAddr loDestAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockAddr loLocalAddr(CliPORT, "127.0.0.1", APL_AF_INET);
        CSockDgram loSockDgram;

        liRet = loSockDgram.Open(loLocalAddr, APL_AF_INET, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvUntil2 - Create socket client failed.", (apl_int_t)0, liRet);

        char lsSendBuf[]="abcdefghijklmnopqrstuvwxyz";
        apl_ssize_t liSendLen = loSockDgram.Send(lsSendBuf, apl_strlen(lsSendBuf), loDestAddr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRecvUntil2 - Send failed.", (apl_ssize_t)apl_strlen(lsSendBuf), liSendLen);
        loSockDgram.Close();
        WAIT_EXP(liTmp == 1);
    }
};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclSockDgram);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

