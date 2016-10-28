#include "Test.h"
#include <deque>
#include <vector>
#include <algorithm>
#include <string>

#define MAX_LINE  (1024)      
#define MAX_LEN   (1024*3)   
#define OPT       (128)     


/* ---------------------------------------------------------------------- */

void handler(int sig_no)
{
   apl_signal( APL_SIGPIPE, handler );
}

/* ---------------------------------------------------------------------- */

void SetNonBlocking(apl_handle_t aiSock)
{
    apl_int_t   liFlags;

    RUN_AND_CHECK_RET_INT(apl_fcntl_getfl, (aiSock, &liFlags));
    liFlags |= APL_O_NONBLOCK;
    RUN_AND_CHECK_RET_INT(apl_fcntl_setfl, (aiSock, liFlags));
}

/* ---------------------------------------------------------------------- */

apl_ssize_t RecvN(apl_handle_t aiSocket, void* apcBuf, apl_size_t auLen)
{
    apl_size_t  luRecv = 0;
    char*       lpcBuf = (char*)apcBuf;

    while (luRecv < auLen)
    {
        apl_ssize_t liRetSz;

        liRetSz = apl_recv(aiSocket, lpcBuf + luRecv, auLen - luRecv, 0);

        if ( liRetSz < 0 )
        {
            if (APL_EINTR == apl_get_errno())
            {
                continue;
            }

            if (APL_EWOULDBLOCK == apl_get_errno() || APL_EAGAIN == apl_get_errno())
            {
                break;
            }
            
            return APL_INT_C(-1);
        }

        if (liRetSz == 0)
        {
            break;
        }

        luRecv += liRetSz;
    }

    assert(luRecv >= 0 && luRecv <= auLen);
    return luRecv;
}

/* ---------------------------------------------------------------------- */

apl_ssize_t SendN(apl_handle_t aiSocket, void* apcBuf, apl_size_t auLen)
{
    apl_size_t  luSend = 0;
    char*       lpcBuf = (char*)apcBuf;

    while (luSend < auLen)
    {
        apl_ssize_t liRetSz;

        liRetSz = apl_send(aiSocket, lpcBuf + luSend, auLen - luSend, 0);

        if ( liRetSz < 0 )
        {
            apl_int_t liErrno = apl_get_errno();

            if (APL_EINTR == liErrno)
            {
                continue;
            }

            if (APL_EWOULDBLOCK == liErrno || APL_EAGAIN == liErrno)
            {
                break;
            }

            return APL_INT_C(-1);
        }
        
        if (liRetSz == 0)
        {
            break;
        }

        luSend += liRetSz;
    }

    assert(luSend >= 0 && luSend <= auLen);
    return luSend;
}

/* ---------------------------------------------------------------------- */

struct CQueue
{
    typedef std::deque<char> QueueT;

    struct apl_mutex_t	csMtx;
    struct apl_cond_t   csCondPush;
    struct apl_cond_t   csCondPop;

    QueueT  coQueue;


    CQueue() 
    {
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_init, (&csMtx));
        RUN_AND_CHECK_RET_ERRNO(apl_cond_init, (&csCondPush));
    }


    ~CQueue() 
    {
        CPPUNIT_ASSERT_MESSAGE("buffer is not empty", coQueue.empty());

        RUN_AND_CHECK_RET_ERRNO(apl_cond_destroy, (&csCondPush));
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_destroy, (&csMtx));
    }


    void Push(void const* apcData, apl_size_t aiSize) 
    {
        char* lpcData = (char*)apcData;
    
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_lock, (&csMtx)); // lock

        coQueue.insert(coQueue.end(), lpcData, lpcData + aiSize);

        RUN_AND_CHECK_RET_ERRNO(apl_cond_signal, (&csCondPush)); // signal push

        RUN_AND_CHECK_RET_ERRNO(apl_mutex_unlock, (&csMtx)); // unlock
    }


    void CheckAndPop(void const* apcData, apl_size_t aiSize)
    {
        char* lpcData = (char*)apcData;

        RUN_AND_CHECK_RET_ERRNO(apl_mutex_lock, (&csMtx)); // lock

        while (coQueue.size() < aiSize)
        {
            RUN_AND_CHECK_RET_ERRNO(apl_cond_wait, (&csCondPush, &csMtx, APL_INT64_C(-1))); // wait push
        }

        CPPUNIT_ASSERT_MESSAGE("data check failed", std::equal(lpcData, lpcData + aiSize, coQueue.begin()));

        coQueue.erase(coQueue.begin(), coQueue.begin() + aiSize);

        RUN_AND_CHECK_RET_ERRNO(apl_mutex_unlock, (&csMtx)); // unlock
    }

};

/* ---------------------------------------------------------------------- */

struct CThread
{
    std::string     moName;
    volatile bool   mbStarted;
    apl_thread_t    miThrId;
    apl_mutex_t     msMtx;
    apl_cond_t      msCond;

    CThread(char const* apcName)
        :moName(apcName)
        ,mbStarted(false)
    {
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_init, (&msMtx));
        RUN_AND_CHECK_RET_ERRNO(apl_cond_init, (&msCond));
    }

    virtual ~CThread()
    {
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_destroy, (&msMtx));
        RUN_AND_CHECK_RET_ERRNO(apl_cond_destroy, (&msCond));
    }

    char const* GetName() const
    {
        return moName.c_str();
    }

    void Start()
    {
        mbStarted = true;

        RUN_AND_CHECK_RET_ERRNO(apl_thread_create, (&miThrId, &CThread::_ThreadEntry, this, 0, 0));
    }


    void Join()
    {
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_lock, (&msMtx));

        while (mbStarted)
        {
            RUN_AND_CHECK_RET_ERRNO(apl_cond_wait, (&msCond, &msMtx, APL_INT64_C(-1)));
        }

        RUN_AND_CHECK_RET_ERRNO(apl_mutex_unlock, (&msMtx));
    }

    virtual void Run() = 0;

    void OnExit()
    {
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_lock, (&msMtx));

        mbStarted = false;

        RUN_AND_CHECK_RET_ERRNO(apl_cond_signal, (&msCond));

        RUN_AND_CHECK_RET_ERRNO(apl_mutex_unlock, (&msMtx));
    }

    static void* _ThreadEntry(void* apv_arg)
    {
        CThread* lpoThr = (CThread*)apv_arg;

        lpoThr->Run();
        
        lpoThr->OnExit();

        return APL_NULL;
    }
};

/* ---------------------------------------------------------------------- */

/** 
 * @brief Producer of mpoQueue, generate and push random data to the queue.
 */
struct CProducer
    :public CThread
{
    apl_handle_t    miSocket;
    apl_size_t      muBufSize;
    apl_size_t      muSendSize;
    CQueue*         mpoQueue;
    
    CProducer(
            char const*     apcName,
            apl_handle_t    aiSocket, 
            apl_size_t      auBufSize, 
            apl_size_t      auSendSize, 
            CQueue*         apoQueue)
        :CThread(apcName)
        ,miSocket(aiSocket)
        ,muBufSize(auBufSize)
        ,muSendSize(auSendSize)
        ,mpoQueue(apoQueue)
    {}


    virtual void Run()
    {
        apl_size_t  luTotal = 0;
        char*       lpcBuf = (char*)apl_malloc(muBufSize);

        while( luTotal < muSendSize )
        {
            apl_ssize_t liRetSz;

            std::generate_n(
                lpcBuf, 
                muBufSize,
                rand);
            
            liRetSz = SendN(miSocket, lpcBuf, muBufSize);
     
            CPPUNIT_ASSERT_MESSAGE("SendN", liRetSz > 0);
            
            mpoQueue->Push(lpcBuf, liRetSz);

            luTotal += liRetSz;
        }

        apl_free(lpcBuf);

        apl_errprintf("%s: finishing %"APL_PRIuINT" bytes\n", GetName(), luTotal);
    }
};

/* ---------------------------------------------------------------------- */

/** 
 * @brief Consumer of mpoQueue, check and pop data from the queue 
 * whenever it received.
 */
struct CConsumer
    :public CThread
{
    apl_handle_t    miSocket;
    apl_size_t      muBufSize;
    apl_size_t      muRecvSize;
    CQueue*         mpoQueue;

    
    CConsumer(
            char const*     apcName,
            apl_handle_t    aiSocket, 
            apl_size_t      auBufSize, 
            apl_size_t      auRecvSize, 
            CQueue*         ap_cq) 
        :CThread(apcName)
        ,miSocket(aiSocket)
        ,muBufSize(auBufSize)
        ,muRecvSize(auRecvSize)
        ,mpoQueue(ap_cq)
    {}

    virtual void Run()
    {
        apl_size_t  luTotal = 0;
        char*       lpcBuf = (char*)apl_malloc(muBufSize);

        CPPUNIT_ASSERT_MESSAGE("apl_malloc", lpcBuf);

        while (luTotal < muRecvSize)
        {
            apl_size_t  luMaxRcv = muRecvSize - luTotal;
            apl_ssize_t liRetSz;

            if (luMaxRcv > muBufSize)
            {
                luMaxRcv = muBufSize;
            }

            liRetSz = apl_recv(miSocket, lpcBuf, luMaxRcv, 0);

            CPPUNIT_ASSERT_MESSAGE("apl_recv", liRetSz > 0);

            mpoQueue->CheckAndPop(lpcBuf, liRetSz);

            luTotal += liRetSz;
        }

        apl_free(lpcBuf);

        apl_errprintf("%s: finishing %"APL_PRIuINT" bytes\n", GetName(), luTotal);
    }
};

/* ---------------------------------------------------------------------- */

/** 
 * @brief Receive data from miSockIn and transfer to miSockOut.
 */
struct CTransfer
    :public CThread
{
    apl_handle_t    miSockIn;
    apl_handle_t    miSockOut;
    apl_size_t      muBufSize;
    apl_size_t      muTranSize;
    apl_time_t      mi64PollTimeout;


    CTransfer(
            char const*     apcName,
            apl_handle_t    aiSockIn, 
            apl_handle_t    aiSockOut, 
            apl_size_t      auBufSize, 
            apl_size_t      auTranSize, 
            apl_time_t      ai64PollTimeout)
        :CThread(apcName)
        ,miSockIn(aiSockIn)
        ,miSockOut(aiSockOut)
        ,muBufSize(auBufSize)
        ,muTranSize(auTranSize)
        ,mi64PollTimeout(ai64PollTimeout)
    {}

    
    virtual void Run()
    {
        apl_pollset_t   lsPs;
        apl_size_t      luRecvTotal = 0;
        apl_size_t      luSendTotal = 0;
        char*           lpcBuf = (char*)apl_malloc(muBufSize);
        apl_size_t      luBufCur = 0;

        RUN_AND_CHECK_RET_INT(apl_pollset_init, (&lsPs, APL_OPEN_MAX));
        RUN_AND_CHECK_RET_INT(apl_pollset_add_handle, (&lsPs, miSockIn, APL_POLLIN));
        RUN_AND_CHECK_RET_INT(apl_pollset_add_handle, (&lsPs, miSockOut, APL_POLLOUT));

        while (luRecvTotal < muTranSize || luSendTotal < muTranSize)
        {
            apl_ssize_t     liRetSz;
            apl_pollfd_t    lasPfds[2];
            apl_ssize_t     liRet;

            CPPUNIT_ASSERT(luBufCur <= muBufSize);

            if (luBufCur == muBufSize)
            { // full
                RUN_AND_CHECK_RET_INT(apl_pollset_set_events, (&lsPs, miSockIn, 0));
            }
            else
            { // not full
                RUN_AND_CHECK_RET_INT(apl_pollset_set_events, (&lsPs, miSockIn, APL_POLLIN));
            }

            if (0 == luBufCur)
            { // empty
                RUN_AND_CHECK_RET_INT(apl_pollset_set_events, (&lsPs, miSockOut, 0));
            }
            else
            { // not empty
                RUN_AND_CHECK_RET_INT(apl_pollset_set_events, (&lsPs, miSockOut, APL_POLLOUT));
            }

            while (true)
            {
                liRet = apl_poll(&lsPs, lasPfds, APL_SIZEOFARRAY(lasPfds), mi64PollTimeout);

                if (liRet < 0)
                {
                    if (apl_get_errno() == APL_EINTR)
                    {
                        continue;
                    }

                    CPPUNIT_FAIL("apl_poll()");
                }
                
                break;
            }

            for (apl_pollfd_t* lpsPfd = lasPfds; lpsPfd != lasPfds + liRet; ++lpsPfd)
            {
                if (APL_POLLFD_GET_HANDLE(lpsPfd) == miSockIn) 
                { // read
                    liRetSz = RecvN(miSockIn, lpcBuf + luBufCur, muBufSize - luBufCur);

                    if (liRetSz >= 0)
                    {
                        luBufCur += liRetSz;
                        luRecvTotal += liRetSz;
                    }
                    else
                    {
                        if (apl_get_errno() == APL_EWOULDBLOCK || apl_get_errno() == APL_EAGAIN)
                        {
                            continue;
                        }

                        CPPUNIT_ASSERT_MESSAGE("RecvN", liRetSz >= 0);
                    }
                }
                else if (APL_POLLFD_GET_HANDLE(lpsPfd) == miSockOut)
                { // write
                    liRetSz = SendN(miSockOut, lpcBuf, luBufCur); 

                    if (liRetSz >= 0)
                    {
                        luBufCur -= liRetSz;
                        apl_memmove(lpcBuf, lpcBuf + liRetSz, luBufCur);
                        luSendTotal += liRetSz;
                    }
                    else
                    {
                        CPPUNIT_ASSERT_MESSAGE("SendN", liRetSz >= 0);
                    }
                }
                // else {}
            }
        }

        RUN_AND_CHECK_RET_INT(apl_pollset_del_handle, (&lsPs, miSockIn));
        RUN_AND_CHECK_RET_INT(apl_pollset_del_handle, (&lsPs, miSockOut));
        RUN_AND_CHECK_RET_INT(apl_pollset_destroy, (&lsPs));

        apl_free(lpcBuf);

        apl_errprintf("%s: finishing %"APL_PRIuINT" bytes\n", GetName(), muTranSize);
    }
};

/* ---------------------------------------------------------------------- */

/**
 * @brief Test suite epoll/devpoll.
 * @brief Including three test cases: PC, PTC and PTTC while P represents 
 * Producer, T represents Transfer and C represents Consumer.
 */
class CTestAplPoll: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAplPoll);
    CPPUNIT_TEST(testPollPC);
    CPPUNIT_TEST(testPollPTC);
    CPPUNIT_TEST(testPollPTTC);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void)
    {
        apl_signal(APL_SIGPIPE, handler);
    }


    void tearDown(void)
    {
    }

    /** 
     * Test case PC. Producer generates random data to Consumer. 
     * apl_poll is not involved in the case.
     */
    void testPollPC(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_handle_t    liSockA;
        apl_handle_t    liSockB;

        apl_size_t const    BUF_SIZE = 512;
        apl_size_t const    TEST_SIZE = 10*1024*1024;

        RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &liSockA, &liSockB));

        CQueue      loQ;
        CProducer   loP("Producer", liSockA, BUF_SIZE, TEST_SIZE, &loQ);
        CConsumer   loC("Consumer", liSockB, BUF_SIZE, TEST_SIZE, &loQ);

        //case
        loP.Start();
        loC.Start();

        //end environment
        loP.Join();
        loC.Join();

        RUN_AND_CHECK_RET_INT(apl_close, (liSockA));
        RUN_AND_CHECK_RET_INT(apl_close, (liSockB));
    }

    /** 
     * Test case PTC. Producer generates random data to Consumer 
     * via Transfer. apl_poll is involved in the Transfer.
     */
    void testPollPTC(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_handle_t    liSockA;
        apl_handle_t    liSockB;
        apl_handle_t    liSockC;
        apl_handle_t    liSockD;

        apl_size_t const    BUF_SIZE = 1024;
        apl_size_t const    TEST_SIZE = 10*1024*1024;

        RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &liSockA, &liSockB));
        RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &liSockC, &liSockD));

        SetNonBlocking(liSockB);
        SetNonBlocking(liSockC);

        CQueue      loQ;
        CProducer   loP("Producer", liSockA, BUF_SIZE, TEST_SIZE, &loQ);
        CTransfer   loT1("Transfer-1", liSockB, liSockC, BUF_SIZE, TEST_SIZE, APL_INT64_C(-1));
        CConsumer   loC("Consumer", liSockD, BUF_SIZE, TEST_SIZE, &loQ);

        //case
        loP.Start();
        loT1.Start();
        loC.Start();

        //end environment
        loP.Join();
        loT1.Join();
        loC.Join();

        RUN_AND_CHECK_RET_INT(apl_close, (liSockA));
        RUN_AND_CHECK_RET_INT(apl_close, (liSockB));
        RUN_AND_CHECK_RET_INT(apl_close, (liSockC));
        RUN_AND_CHECK_RET_INT(apl_close, (liSockD));
    }


    /** 
     * Test case PTTC. Producer generates random data to Consumer 
     * via two Transfers. apl_poll is involved twice in this case. 
     * Each Transfers once.
     */
    void testPollPTTC(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_handle_t    liSockA;
        apl_handle_t    liSockB;
        apl_handle_t    liSockC;
        apl_handle_t    liSockD;
        apl_handle_t    liSockE;
        apl_handle_t    liSockF;

        apl_size_t const    BUF_SIZE = 65536;
        apl_size_t const    TEST_SIZE = 20*1024*1024;

        RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &liSockA, &liSockB));
        RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &liSockC, &liSockD));
        RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &liSockE, &liSockF));

        SetNonBlocking(liSockB);
        SetNonBlocking(liSockC);
        SetNonBlocking(liSockD);
        SetNonBlocking(liSockE);

        CQueue      loQ;
        CProducer   loP("Producer", liSockA, BUF_SIZE, TEST_SIZE, &loQ);
        CTransfer   loT1("Transfer-1", liSockB, liSockC, BUF_SIZE, TEST_SIZE, APL_INT64_C(-1));
        CTransfer   loT2("Transfer-2", liSockD, liSockE, BUF_SIZE, TEST_SIZE, APL_INT64_C(-1));
        CConsumer   loC("Consumer", liSockF, BUF_SIZE, TEST_SIZE, &loQ);

        //case
        loP.Start();
        loT1.Start();
        loT2.Start();
        loC.Start();

        //end environment
        loP.Join();
        loT1.Join();
        loT2.Join();
        loC.Join();

        RUN_AND_CHECK_RET_INT(apl_close, (liSockA));
        RUN_AND_CHECK_RET_INT(apl_close, (liSockB));
        RUN_AND_CHECK_RET_INT(apl_close, (liSockC));
        RUN_AND_CHECK_RET_INT(apl_close, (liSockD));
        RUN_AND_CHECK_RET_INT(apl_close, (liSockE));
        RUN_AND_CHECK_RET_INT(apl_close, (liSockF));
    }

};


CPPUNIT_TEST_SUITE_REGISTRATION( CTestAplPoll );


int main()
{
    RUN_ALL_TEST(__FILE__);
}
