#include "Test.h"


int gi = 2;

void handlerzero(int sig_no)
{
   gi = 0;
   //apl_signal( sig_no, handler );
}

void handler(int sig_no)
{
    gi = 1;

} 

///call by testSignal.
void testsig(apl_int_t ai_no)
{
    apl_signal(ai_no, APL_SIG_DFL);
    apl_signal(ai_no, APL_SIG_IGN);
    apl_signal(ai_no, APL_SIG_ERR);
    apl_signal(ai_no, handler);
    kill(apl_getpid(), ai_no);
    if ( gi != 1 )
    {
        CPPUNIT_FAIL("signal fail");
    }
    apl_signal(ai_no, handlerzero);
    kill(apl_getpid(), ai_no);
    if ( gi != 0 )
    {
        CPPUNIT_FAIL("signal fail");
    }
}

/**
 * @brief Test suite Signal.
 */
class CTestaplsignal: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplsignal);
    CPPUNIT_TEST(testSignal);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}

    /** 
     * Test case Signal.
     */
    void testSignal(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        testsig(APL_SIGPIPE); 
        testsig(APL_SIGALRM); 
        testsig(APL_SIGCHLD); 
        testsig(APL_SIGCONT); 
        testsig(APL_SIGHUP);
        testsig(APL_SIGILL);
        testsig(APL_SIGINT);
        testsig(APL_SIGQUIT);
        testsig(APL_SIGSEGV);
        testsig(APL_SIGTSTP);
        testsig(APL_SIGTTIN);
        testsig(APL_SIGTTOU);
        testsig(APL_SIGUSR1);
        testsig(APL_SIGUSR2);
        testsig(APL_SIGPOLL);
        testsig(APL_SIGPROF);
        testsig(APL_SIGSYS);
        testsig(APL_SIGTRAP);
        testsig(APL_SIGURG);
        testsig(APL_SIGVTALRM);
        testsig(APL_SIGXCPU);
        testsig(APL_SIGXFSZ); 

        //end environment
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplsignal);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
