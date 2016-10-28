#include "Test.h"

#define NLOOP 10000

#define NTHREAD 3

apl_uintmax_t       gu_data=0;

apl_time_t gua_timeout[6] = {APL_INT64_C(-1), 0, APL_TIME_SEC, 10*APL_TIME_MSEC, 500*APL_TIME_USEC, 50000*APL_TIME_NSEC};

struct CMtxTime
{
    apl_mutex_t *mpsMtx; 
    apl_time_t mtTime;
};

#define TEST_SYNCH_INIT(type, synch) \
    struct apl_##type##_t    synch; \
    do { \
        apl_int_t   li_ret; \
        li_ret = apl_##type##_init(&synch); \
        if ( 0 != li_ret && #type) \
        { \
            PrintErrMsg(#type " init failed\n", -1); \
        } \
    } while(0)


#define TEST_SYNCH_OP1(type, synch, acquire, release, timeout) \
    do { \
        apl_int_t   li_ret; \
        li_ret = apl_##type##_##acquire(&synch, timeout); \
        if (0 != li_ret && #acquire) \
        { \
            PrintErrMsg(#type #acquire" failed\n", -1); \
        } \
        li_ret = apl_##type##_##release(&synch); \
        if (0 != li_ret && #release) \
        { \
            PrintErrMsg(#type #release " failed \n", -1); \
        } \
    } while(0)




#define TEST_SYNCH_OP3(type,  cond,    mtx,    acquire,  release,  ret) \
    do { \
        apl_int_t   li_ret; \
        li_ret = apl_##type##_##acquire(&cond); \
        if (0 != li_ret && #acquire) \
        { \
            PrintErrMsg(#type #acquire " failed \n", -1); \
        } \
        li_ret = apl_mutex_lock(&mtx); \
        li_ret = apl_##type##_##release(&cond, &mtx, timeout); \
        if (ret != li_ret && #release) \
        { \
            PrintErrMsg(#type #release " failed \n", -1); \
        } \
        apl_mutex_unlock(&mtx); \
    } while(0)


#define TEST_SYNCH_DESTROY(type,  synch) \
    do { \
        apl_int_t   li_ret; \
        li_ret = apl_##type##_destroy(&synch); \
        if (0 != li_ret && #type) \
        { \
            PrintErrMsg(#type "destroy failed\n", -1); \
        } \
    } while(0)

/* ---------------------------------------------------------------------- */

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
        RUN_AND_CHECK_RET_ERRNO(apl_thread_create, (&miThrId, &CThread::_ThreadEntry, this, 0, 0));
        
        mbStarted = true;
    }


    void Join()
    {
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_lock, (&msMtx));

        while (mbStarted)
        {
            RUN_AND_CHECK_RET_ERRNO(apl_cond_wait, (&msCond, &msMtx, APL_TIME_SEC * 30));
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

struct CMutexTest
    :public CThread
{
    struct CMtxTime *mpsMtxtime;

    CMutexTest(char const*     apcName)
        :CThread(apcName)
        ,mpsMtxtime(NULL)
    {}

    void SetMtxTime(struct CMtxTime *apsMtxtime)
    {
        mpsMtxtime = apsMtxtime;
    }

    virtual void Run()
    {
        apl_uintmax_t li_seq = NLOOP;    
        apl_uintmax_t li_val; 
        apl_uint_t li_ret; 

        sleep(1);

        while (li_seq--)
        {
            li_ret = apl_mutex_lock(mpsMtxtime->mpsMtx);
            if (li_ret == 0)
            {
                li_val = gu_data;
                //printf("Thread %d: gu_data = %llu\n", pthread_self(), li_val + 1);
                gu_data = li_val + 1;
                RUN_AND_CHECK_RET_ERRNO(apl_mutex_unlock, (mpsMtxtime->mpsMtx));
            }
            else 
            {
                li_seq++;
                //printf("Thread %d: mutex_lock failed, reason: %d\n", pthread_self(), li_ret);
            }
        }

    }
};


/**
 * @brief Test suite Syn.
 * @brief Including five test cases: Mutex, Sema, Condition, RWlock 
 * and Spinlock.
 */
class CTestaplsyn: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplsyn);
    CPPUNIT_TEST(testMutex);
    CPPUNIT_TEST(testSema);
    CPPUNIT_TEST(testCond);
    CPPUNIT_TEST(testRWlock);
    CPPUNIT_TEST(testSpinlock);
    CPPUNIT_TEST_SUITE_END();

    apl_mutex_t msMtx; 
    struct CMtxTime msMtxTime;
public:
    void setUp(void) {
        printf("setUp call\n");
        fsync(1);
    }

    void tearDown(void) {
        printf("tearDown call\n");
        fsync(1);
    }


    /** 
     * Test case Mutex.
     */
    void testMutex(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        struct CMutexTest loM1("Thread1");
        struct CMutexTest loM2("Thread2");
        struct CMutexTest loM3("Thread3");
        apl_int_t li_itr;

        for (li_itr=0; li_itr<6; li_itr++)
        {
            gu_data = 0;
            RUN_AND_CHECK_RET_ERRNO(apl_mutex_init, (&msMtx));
            msMtxTime.mpsMtx = &msMtx; 
            msMtxTime.mtTime = gua_timeout[li_itr];

            loM1.SetMtxTime(&msMtxTime);
            loM2.SetMtxTime(&msMtxTime);
            loM3.SetMtxTime(&msMtxTime);

            loM1.Start(); 
            loM2.Start(); 
            loM3.Start(); 

            loM1.Join(); 
            loM2.Join(); 
            loM3.Join(); 

            RUN_AND_CHECK_RET_ERRNO(apl_mutex_destroy, (&msMtx));
            CPPUNIT_ASSERT_MESSAGE("mutex test failed",  gu_data == NLOOP * 3);
        }

        //end environment
    }

    /** 
     * Test case Sema.
     */
    void testSema(void)
    {
        PRINT_TITLE_2(__func__);
        struct apl_sema_t ls_sema;

        RUN_AND_CHECK_RET_ERRNO(apl_sema_init, (&ls_sema, 0));

        RUN_AND_CHECK_RET_ERRNO(apl_sema_post, (&ls_sema));
        RUN_AND_CHECK_RET_ERRNO(apl_sema_wait, (&ls_sema, APL_INT64_C(-1)));
        RUN_AND_CHECK_RET_ERRNO(apl_sema_post, (&ls_sema));
        RUN_AND_CHECK_RET_ERRNO(apl_sema_wait, (&ls_sema, 0));
        RUN_AND_CHECK_RET_ERRNO(apl_sema_post, (&ls_sema));
        RUN_AND_CHECK_RET_ERRNO(apl_sema_wait, (&ls_sema, APL_TIME_NSEC));
        RUN_AND_CHECK_RET_ERRNO(apl_sema_post, (&ls_sema));
        RUN_AND_CHECK_RET_ERRNO(apl_sema_wait, (&ls_sema, APL_TIME_USEC));
        RUN_AND_CHECK_RET_ERRNO(apl_sema_post, (&ls_sema));
        RUN_AND_CHECK_RET_ERRNO(apl_sema_wait, (&ls_sema, APL_TIME_MSEC));
        RUN_AND_CHECK_RET_ERRNO(apl_sema_post, (&ls_sema));
        RUN_AND_CHECK_RET_ERRNO(apl_sema_wait, (&ls_sema, APL_TIME_SEC));

        RUN_AND_CHECK_RET_ERRNO(apl_sema_destroy, (&ls_sema));

    }

    /** 
     * Test case Cond.
     */
    void testCond(void)
    {
        PRINT_TITLE_2(__func__);

        //TEST_SYNCH_INIT(cond,  ls_cond);
        struct apl_cond_t ls_cond;
        struct apl_mutex_t ls_mtx;
        apl_int_t   li_ret;


        RUN_AND_CHECK_RET_ERRNO(apl_cond_init, (&ls_cond));
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_init, (&ls_mtx));

        RUN_AND_CHECK_RET_ERRNO(apl_cond_signal, (&ls_cond));
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_lock, (&ls_mtx));
        li_ret = apl_cond_wait(&ls_cond, &ls_mtx, 0);
        CPPUNIT_ASSERT_MESSAGE("apl_cond_wait failed",  li_ret == APL_INT64_C(-1)
                && (APL_ETIMEDOUT == apl_get_errno()));
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_unlock, (&ls_mtx));

        RUN_AND_CHECK_RET_ERRNO(apl_cond_signal, (&ls_cond));
        apl_mutex_lock(&ls_mtx);
        li_ret = apl_cond_wait(&ls_cond, &ls_mtx, APL_TIME_NSEC);
        CPPUNIT_ASSERT_MESSAGE("apl_cond_wait failed", li_ret == APL_INT64_C(-1) 
                && APL_ETIMEDOUT == apl_get_errno());
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_unlock, (&ls_mtx));

        RUN_AND_CHECK_RET_ERRNO(apl_cond_signal, (&ls_cond));
        apl_mutex_lock(&ls_mtx);
        li_ret = apl_cond_wait(&ls_cond, &ls_mtx, APL_TIME_USEC);
        CPPUNIT_ASSERT_MESSAGE("apl_cond_wait failed", li_ret == APL_INT64_C(-1) 
                && APL_ETIMEDOUT == apl_get_errno());
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_unlock, (&ls_mtx));

        TEST_SYNCH_DESTROY(cond,  ls_cond);
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_destroy, (&ls_mtx));
    }

    void testRWlock(void)
    {
        PRINT_TITLE_2(__func__);
        struct apl_rwlock_t    ls_rwl;
        apl_int_t li_itr;

        RUN_AND_CHECK_RET_ERRNO(apl_rwlock_init, (&ls_rwl));

        for (li_itr=0; li_itr<5; li_itr++)
        {
            RUN_AND_CHECK_RET_ERRNO(apl_rwlock_wrlock, (&ls_rwl, -1));
            RUN_AND_CHECK_RET_ERRNO(apl_rwlock_unlock, (&ls_rwl));
            RUN_AND_CHECK_RET_ERRNO(apl_rwlock_wrlock, (&ls_rwl, 0));
            RUN_AND_CHECK_RET_ERRNO(apl_rwlock_unlock, (&ls_rwl));
            RUN_AND_CHECK_RET_ERRNO(apl_rwlock_wrlock, (&ls_rwl, APL_TIME_NSEC));
            RUN_AND_CHECK_RET_ERRNO(apl_rwlock_unlock, (&ls_rwl));
            RUN_AND_CHECK_RET_ERRNO(apl_rwlock_wrlock, (&ls_rwl, APL_TIME_USEC));
            RUN_AND_CHECK_RET_ERRNO(apl_rwlock_unlock, (&ls_rwl));
            RUN_AND_CHECK_RET_ERRNO(apl_rwlock_wrlock, (&ls_rwl, APL_TIME_MSEC));
            RUN_AND_CHECK_RET_ERRNO(apl_rwlock_unlock, (&ls_rwl));
            RUN_AND_CHECK_RET_ERRNO(apl_rwlock_wrlock, (&ls_rwl, APL_TIME_NSEC));
            RUN_AND_CHECK_RET_ERRNO(apl_rwlock_unlock, (&ls_rwl));
        }

        RUN_AND_CHECK_RET_ERRNO(apl_rwlock_destroy, (&ls_rwl));
    }

    void testSpinlock(void)
    {
        PRINT_TITLE_2(__func__);
        struct apl_spin_t ls_spin;
        apl_int_t li_itr;

        RUN_AND_CHECK_RET_ERRNO(apl_spin_init, (&ls_spin));

        for (li_itr=0; li_itr<5; li_itr++)
        {
            RUN_AND_CHECK_RET_ERRNO(apl_spin_lock, (&ls_spin));
            RUN_AND_CHECK_RET_ERRNO(apl_spin_unlock, (&ls_spin));
        }

        RUN_AND_CHECK_RET_ERRNO(apl_spin_destroy, (&ls_spin));
    }
#if 0
    void testMutexCond(void)
    {
        apl_uintmax_t li_data = 0;
        apl_mutex_t ls_mtx;
        apl_cond_t ls_cond;

struct CThreadMtxCond
{
    volatile bool       mbStarted;
    apl_uintmax_t       muSeq;
    struct apl_mutex_t  msMtx;
    struct apl_cond_t   msCond;
};

struct CThreadMtxCond ls_th1;
struct CThreadMtxCond ls_th2;
struct CThreadMtxCond ls_th3;

        RUN_AND_CHECK_RET_ERRNO(apl_mutex_init, (&ls_mtx));
        RUN_AND_CHECK_RET_ERRNO(apl_cond_init, (&ls_cond));

        START_THREAD_BODY(MutexCondProducer, struct CThreadMtxCond, ls_th1)
        {
            t1status = true;
            while (li_time1 < 10000)
            {
                RUN_AND_CHECK_RET_ERRNO(apl_mutex_lock, (&ls_mtx, APL_INT64_C(-1)));
                li_data ++;
                RUN_AND_CHECK_RET_ERRNO(apl_cond_signal, (&ls_cond));
                RUN_AND_CHECK_RET_ERRNO(apl_mutex_unlock, (&ls_mtx));
                li_time1 ++;
            }

            RUN_AND_CHECK_RET_ERRNO(apl_mutex_lock, (&ls_mtxThread1, APL_INT64_C(-1)));
            t1status = false;
            RUN_AND_CHECK_RET_ERRNO(apl_cond_signal, (&ls_condThread1));
            RUN_AND_CHECK_RET_ERRNO(apl_mutex_unlock, (&ls_mtxThread1));
        }
        END_THREAD_BODY(MutexCondProducer)
        RUN_THREAD_BODY(MutexCondProducer);

        //wait for thread 1 finished
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_lock, (&ls_mtxThread1, APL_INT64_C(-1)));
        while (t1status)
        {
            RUN_AND_CHECK_RET_ERRNO(apl_cond_wait, (&ls_condThread1, ls_mtxThread1, APL_INT64_C(-1)));
        }
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_unlock, (&ls_mtxThread1));
    }
#endif
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplsyn);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
