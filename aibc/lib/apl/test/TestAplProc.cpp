#include "Test.h"

///call by testDir
void test_dir(const char* lpc_dir)
{
     apl_int_t li_ir;
     char lac_dir[1024];

     RUN_AND_CHECK_RET_INT(apl_chdir, (lpc_dir));
     RUN_AND_CHECK_RET_INT(apl_getcwd, (lac_dir, sizeof(lac_dir)));

     apl_errprintf("pwd: %s\n", lac_dir);
}

///call by testSpawn
void test_spawn(char const* apc_cmd, char** appc_args, char** appc_envs)
{
    apl_pid_t   li_pid;

    RUN_AND_CHECK_RET_INT(apl_spawn, (&li_pid, apc_cmd, appc_args, appc_envs));
}


#define TEST_SPAWN_0(cmd)\
    do{ \
        test_spawn(cmd, NULL, NULL); \
    }while(0)


#define TEST_SPAWN_N(cmd, ...)\
    do{ \
        char* laac_args[] = { __VA_ARGS__, NULL }; \
        test_spawn(cmd, laac_args, NULL); \
    }while(0)


/**
 * @brief Test suite proc.
 * @brief Including three test cases: testDir, testPid and testSpawn.
 */
class CTestaplproc: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplproc);
    CPPUNIT_TEST(testDir);
    CPPUNIT_TEST(testPid);
    CPPUNIT_TEST(testSpawn);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}

    void tearDown(void) {}

    /** 
     * Test case testDir.
     */
    void testDir(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        test_dir("/bin");
        test_dir("/lib");
        test_dir("/usr");
        test_dir("/tmp");

        //end environment
    }
    
    /** 
     * Test case testPid.
     */
    void testPid()
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_pid_t   li_pid;

        //case
        li_pid = apl_getpid();
        apl_errprintf("pid: %"APL_PRIuINT"\n", li_pid);

        //end environment
    }

    void testSpawn(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        TEST_SPAWN_0("/bin/true");
        TEST_SPAWN_0("/bin/false");

        //end environment
    }

};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplproc);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
