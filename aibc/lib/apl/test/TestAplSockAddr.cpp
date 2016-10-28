#include "Test.h"

/* ---------------------------------------------------------------------- */

#define PRINT_DEFINE(def) \
    do { \
        apl_errprintf("APL_%s = [%"APL_PRIdINT"]\n", #def, APL_##def); \
        CPPUNIT_ASSERT_MESSAGE("DEFINE", (def) == (APL_##def)); \
    } while(0)

/* ---------------------------------------------------------------------- */

void TestFamily(struct apl_sockaddr_t* aps_sa, apl_size_t au_size, apl_int_t ai_family)
{
    apl_int_t   li_check;

    RUN_AND_CHECK_RET_INT(apl_sockaddr_setfamily, (aps_sa, ai_family));
    RUN_AND_CHECK_RET_INT(apl_sockaddr_getfamily, (aps_sa, &li_check));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sockaddr_family", ai_family, li_check);
}


#define TEST_FAMILY(sas, family) \
    do { \
        apl_errprintf("testing %s\n", #family); \
        TestFamily((struct apl_sockaddr_t*)&sas, sizeof(sas), family); \
    } while(0)

/* ---------------------------------------------------------------------- */

void TestAddr(struct apl_sockaddr_t* aps_sa, apl_size_t au_size, apl_int_t ai_family, char const* lpc_addr)
{
    char        lac_check[APL_NAME_MAX];

    TestFamily(aps_sa, au_size, ai_family);

    RUN_AND_CHECK_RET_INT(apl_sockaddr_setaddr, (aps_sa, au_size, lpc_addr));
    RUN_AND_CHECK_RET_INT(apl_sockaddr_getaddr, (aps_sa, lac_check, sizeof(lac_check)));
   
    CPPUNIT_ASSERT_MESSAGE("check addr", apl_strcmp(lpc_addr, lac_check) == 0);

    CPPUNIT_ASSERT_MESSAGE(
        "apl_sockaddr_getlen", 
        (apl_size_t)apl_sockaddr_getlen(aps_sa) <= au_size);

    switch (ai_family)
    {
        case APL_AF_INET:
            CPPUNIT_ASSERT_MESSAGE(
                "apl_sockaddr_get (APL_AF_INET)", 
                apl_sockaddr_getlen(aps_sa) == sizeof(struct apl_sockaddr_in_t));
            break;

#if defined(APL_AF_INET6)
        case APL_AF_INET6:
            CPPUNIT_ASSERT_MESSAGE(
                "apl_sockaddr_get (APL_AF_INET6)", 
                apl_sockaddr_getlen(aps_sa) == sizeof(struct apl_sockaddr_in6_t));
            break;
#endif
        case APL_AF_UNIX:
            CPPUNIT_ASSERT_MESSAGE(
                "apl_sockaddr_get (APL_AF_UNIX)", 
                (apl_size_t)apl_sockaddr_getlen(aps_sa) >= APL_OFFSETOF(struct apl_sockaddr_un_t, sun_path));
            break; 

        default:
            CPPUNIT_FAIL("not supported af");
            break;
    }
}


#define TEST_ADDR(sas, family, addr) \
    do { \
        apl_errprintf("testing %s with \'%s\'\n", #family, addr); \
        TestAddr((struct apl_sockaddr_t*)&sas, sizeof(sas), family, addr); \
    } while(0)

/* ---------------------------------------------------------------------- */

void TestPort(struct apl_sockaddr_t* aps_sa, apl_size_t au_size, apl_int_t ai_family, apl_uint16_t au16_port)
{
    apl_uint16_t    lu16_check;

    TestFamily(aps_sa, au_size, ai_family);

    RUN_AND_CHECK_RET_INT(apl_sockaddr_setport, (aps_sa, au_size, au16_port));
    RUN_AND_CHECK_RET_INT(apl_sockaddr_getport, (aps_sa, &lu16_check));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("sockaddr_port", au16_port, lu16_check);
}


#define TEST_PORT(sas, family, port) \
    do { \
        apl_errprintf("testing %s with [%"APL_PRIu16"]\n", #family, port); \
        TestPort((struct apl_sockaddr_t*)&sas, sizeof(sas), family, port); \
    } while(0)

/* ---------------------------------------------------------------------- */

typedef apl_int_t (*SetAddrFunc)(struct apl_sockaddr_t*, apl_size_t);
typedef apl_int_t (*CheckAddrFunc)(struct apl_sockaddr_t const*, apl_size_t);


void TestXxx(
    struct apl_sockaddr_t*  aps_sa, 
    apl_size_t              au_size, 
    apl_int_t               ai_family, 
    SetAddrFunc             apf_setfunc, 
    char const*             apc_setfuncname,
    CheckAddrFunc           apf_checkfunc, 
    char const*             apc_checkfuncname)
{
    char        lac_check[APL_NAME_MAX];
    apl_int_t   li_ret;

    TestFamily(aps_sa, au_size, ai_family);

    RUN_AND_CHECK_RET_INT(apf_setfunc, (aps_sa, au_size));

    RUN_AND_CHECK_RET_INT(apl_sockaddr_getaddr, (aps_sa, lac_check, sizeof(lac_check)));
    apl_errprintf("addr: %s\n", lac_check);

    li_ret = apf_checkfunc(aps_sa, au_size);
    PrintErrMsg(apc_checkfuncname, li_ret);

    CPPUNIT_ASSERT(li_ret > 0);
}


#define TEST_XXX(sas, family, setfunc, checkfunc) \
    do { \
        apl_errprintf("testing %s\n", #family); \
        TestXxx((struct apl_sockaddr_t*)&sas, sizeof(sas), family, setfunc, #setfunc, checkfunc, #checkfunc); \
    } while(0)

/* ---------------------------------------------------------------------- */

/**
 * @brief Test suite Sockaddr
 * @brief Including six test cases: Defines, Family, Addr, Port, 
 * Wild and Loopback. 
 */
class CTestaplSocketaddr: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplSocketaddr);
    CPPUNIT_TEST(testDefines);
    CPPUNIT_TEST(testFamily);
    CPPUNIT_TEST(testAddr);
    CPPUNIT_TEST(testPort);
    CPPUNIT_TEST(testWild);
    CPPUNIT_TEST(testLoopback);
    CPPUNIT_TEST(testGetHostAddrList);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    /** 
     * Test case Defines.
     */
    void testDefines(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        
        //case
        PRINT_DEFINE(AF_UNSPEC);
        PRINT_DEFINE(AF_INET);

#if defined(APL_AF_INET6)
        PRINT_DEFINE(AF_INET6);
#endif
        PRINT_DEFINE(AF_UNIX);

        PRINT_DEFINE(SOCK_STREAM);
        PRINT_DEFINE(SOCK_DGRAM);
        PRINT_DEFINE(SOCK_SEQPACKET);

        PRINT_DEFINE(IPPROTO_IP);
        PRINT_DEFINE(IPPROTO_ICMP);
        PRINT_DEFINE(IPPROTO_TCP); 
        PRINT_DEFINE(IPPROTO_UDP); 
        PRINT_DEFINE(IPPROTO_RAW); 

#if defined(APL_AF_INET6)
        PRINT_DEFINE(IPPROTO_IPV6);
#endif

        //end environment
    }

    /** 
     * Test case Family. setfamily/getfamily
     */
    void testFamily(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        struct apl_sockaddr_storage_t ls_sas;

        //case
        TEST_FAMILY(ls_sas, APL_AF_UNSPEC);
        TEST_FAMILY(ls_sas, APL_AF_INET);
#if defined(APL_AF_INET6)
        TEST_FAMILY(ls_sas, APL_AF_INET6);
#endif
        TEST_FAMILY(ls_sas, APL_AF_UNIX);

        //end cae environment
    }

    /** 
     * Test case Addr. setaddr/getaddr.
     */
    void testAddr(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        
        struct apl_sockaddr_storage_t ls_sas;

        //case
        TEST_ADDR(ls_sas, APL_AF_INET, "127.0.0.1");
        TEST_ADDR(ls_sas, APL_AF_INET, "10.1.2.3");
#if defined(APL_AF_INET6)
#endif
        TEST_ADDR(ls_sas, APL_AF_UNIX, "/tmp/sock1");
        TEST_ADDR(ls_sas, APL_AF_UNIX, "./sock1");
        TEST_ADDR(ls_sas, APL_AF_UNIX, "/1/2/3/4/5/6/7/8/9/10/11/12/13/14/15/16/17/18/19/20");

        //end environment
    }

    /** 
     * Test case Port. setport/getport.
     */
    void testPort(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        struct apl_sockaddr_storage_t ls_sas;

        //case
        TEST_PORT(ls_sas, APL_AF_INET, 0);
        TEST_PORT(ls_sas, APL_AF_INET, 1);
        TEST_PORT(ls_sas, APL_AF_INET, 1023);
        TEST_PORT(ls_sas, APL_AF_INET, 1024);
        TEST_PORT(ls_sas, APL_AF_INET, 1025);
        TEST_PORT(ls_sas, APL_AF_INET, 65534);
        TEST_PORT(ls_sas, APL_AF_INET, 65535);

#if defined(APL_AF_INET6)
        TEST_PORT(ls_sas, APL_AF_INET6, 0);
        TEST_PORT(ls_sas, APL_AF_INET6, 1);
        TEST_PORT(ls_sas, APL_AF_INET6, 1023);
        TEST_PORT(ls_sas, APL_AF_INET6, 1024);
        TEST_PORT(ls_sas, APL_AF_INET6, 1025);
        TEST_PORT(ls_sas, APL_AF_INET6, 65534);
        TEST_PORT(ls_sas, APL_AF_INET6, 65535);
#endif

        //end environment
    }

    /** 
     * Test case Wild.
     */
    void testWild(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        struct apl_sockaddr_storage_t ls_sas;

        //case
        TEST_XXX(ls_sas, APL_AF_INET, apl_sockaddr_setwild, apl_sockaddr_checkwild);
        TEST_XXX(ls_sas, APL_AF_INET6, apl_sockaddr_setwild, apl_sockaddr_checkwild);

        //end environment
    }

    /** 
     * Test case Loopback.
     */
    void testLoopback(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        struct apl_sockaddr_storage_t ls_sas;

        //case
        TEST_XXX(ls_sas, APL_AF_INET, apl_sockaddr_setloopback, apl_sockaddr_checkloopback);
        TEST_XXX(ls_sas, APL_AF_INET6, apl_sockaddr_setloopback, apl_sockaddr_checkloopback);

        //end environment
    }

    void testGetHostAddrList(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        char** lpaAddrList= new char*[10];

        for(int i = 0;i < 10;i++)
        {
            lpaAddrList[i] = new char[INET_ADDRSTRLEN];
            memset(lpaAddrList[i],0,INET_ADDRSTRLEN);
        }

        apl_int_t liRet = 0;
        apl_int_t liIdx;

        liRet = apl_get_hostaddr_list(lpaAddrList, INET_ADDRSTRLEN, 20);

        if (liRet <= 0)
        {
            CPPUNIT_ASSERT_MESSAGE("Get host address list failed!", 0);
        }

        apl_errprintf("There are %"APL_PRId32" interfaces in this machine. They are:\n", liRet);

        for (liIdx = 0; liIdx < liRet; liIdx++)
        {
            apl_errprintf("%"APL_PRId32") %s\n", liIdx, lpaAddrList[liIdx]);
        }

        for(int i = 0;i < 10;i++)
        {
            delete [] lpaAddrList[i];
        }

        delete [] lpaAddrList ;

    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplSocketaddr);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
