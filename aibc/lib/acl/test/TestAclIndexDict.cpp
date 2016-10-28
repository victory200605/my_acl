#include "Test.h"
#include "acl/IndexDict.h"
#include "acl/MemoryBlock.h"

using namespace acl;

class CTestAclIndexDict:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclIndexDict);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testOps);
    CPPUNIT_TEST(testCompare);
    CPPUNIT_TEST(testSwap);
    CPPUNIT_TEST(testDecodeEncode);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}

    void tearDown(void) {}

    void testConstructor(void)
    {
        CIndexDict  loDict1;
        CIndexDict  loDict2(loDict1);

        CPPUNIT_ASSERT_EQUAL(loDict1.TotalTags(), (apl_size_t)0);
        CPPUNIT_ASSERT_EQUAL(loDict2.TotalTags(), (apl_size_t)0);

        loDict2.Add(0, "abc");
        
        CPPUNIT_ASSERT_EQUAL(loDict2.TotalTags(), (apl_size_t)1);

        loDict1 = loDict2;

        CPPUNIT_ASSERT_EQUAL(loDict2.TotalTags(), (apl_size_t)1);
        CPPUNIT_ASSERT(loDict2.GetStr2(0, 0) == std::string("abc"));
    }

    void testOps(void)
    {
        CIndexDict loDict;
        apl_size_t const N = 4;

        CPPUNIT_ASSERT(loDict.TotalTags() == 0);

        for (apl_size_t i = 0; i < 2 * N; ++i)
        {
            CPPUNIT_ASSERT(!loDict.Has(i));
            CPPUNIT_ASSERT_EQUAL(loDict.Get(i), (char const*)APL_NULL);
            CPPUNIT_ASSERT_EQUAL(loDict.GetInt(i), (apl_intmax_t)0);
            CPPUNIT_ASSERT(loDict.GetStr(i) == "");
            
            for (apl_size_t j = 0; j < i; ++j)
            {
                char lacBuf[24];
                apl_intmax_t n = i * N + j;
                apl_snprintf(lacBuf, sizeof(lacBuf), "%"APL_PRIdMAX, n);

                loDict.Add(i, lacBuf);
            }
        }

        CPPUNIT_ASSERT_EQUAL(loDict.TotalTags(), 2*N - 1);
        CPPUNIT_ASSERT_EQUAL(loDict.GetInt2(0,0), (apl_intmax_t)0);

        for (apl_size_t i = 0; i < N; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(loDict.Count(i), i);

            for (apl_size_t j = 0; j < i; ++j)
            {
                char lacBuf[24];
                apl_intmax_t n = i * N + j;
                apl_snprintf(lacBuf, sizeof(lacBuf), "%"APL_PRIdMAX, n);

                CPPUNIT_ASSERT(apl_strcmp(loDict.Get2(i, j), lacBuf) == 0);
                CPPUNIT_ASSERT(loDict.GetStr2(i, j) == std::string(lacBuf));
                CPPUNIT_ASSERT_EQUAL(loDict.GetInt2(i, j), n);
            }
        }

        CPPUNIT_ASSERT_EQUAL(loDict.TotalTags(), 2 * N - 1);
        CPPUNIT_ASSERT_EQUAL(loDict.GetInt2(0,0), (apl_intmax_t)0);

        for (apl_size_t i = 0; i < N; ++i)
        {
            for (apl_size_t j = 0; j < i; ++j)
            {
                CPPUNIT_ASSERT(j < loDict.Count(i));
                CPPUNIT_ASSERT_EQUAL(loDict.Set2(i, j, "abc"), (apl_int_t)0);
                CPPUNIT_ASSERT(loDict.GetStr2(i, j) == std::string("abc"));
            }
        }

        CPPUNIT_ASSERT_EQUAL(loDict.TotalTags(), 2*N - 1);
        CPPUNIT_ASSERT_EQUAL(loDict.GetInt2(0,0), (apl_intmax_t)0);

        for (apl_size_t i = 0; i < N; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(loDict.Count(i), i);

            for (apl_size_t j = 0; j < i; ++j)
            {
                CPPUNIT_ASSERT(apl_strcmp(loDict.Get2(i, j), "abc") == 0);
                CPPUNIT_ASSERT(loDict.GetStr2(i, j) == std::string("abc"));
                CPPUNIT_ASSERT_EQUAL(loDict.GetInt2(i, j), (apl_intmax_t)0);
            }
        }

        CPPUNIT_ASSERT_EQUAL(loDict.TotalTags(), 2*N - 1);
        CPPUNIT_ASSERT_EQUAL(loDict.GetInt2(0,0), (apl_intmax_t)0);

        for (apl_size_t i = 0; i < N; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(loDict.Count(i), i);

            for (apl_size_t j = 0; j < i; ++j)
            {
                loDict.Set2(i, j, "xyz", 3);
            }
        }

        CPPUNIT_ASSERT_EQUAL(loDict.TotalTags(), 2*N - 1);
        CPPUNIT_ASSERT_EQUAL(loDict.GetInt2(0,0), (apl_intmax_t)0);

        for (apl_size_t i = 0; i < N; ++i)
        {
            CPPUNIT_ASSERT(loDict.Count(i) == i);

            for (apl_size_t j = 0; j < i; ++j)
            {
                CPPUNIT_ASSERT(apl_strcmp(loDict.Get2(i, j), "xyz") == 0);
                CPPUNIT_ASSERT(loDict.GetStr2(i, j) == std::string("xyz"));
                CPPUNIT_ASSERT_EQUAL(loDict.GetInt2(i, j), (apl_intmax_t)0);
            }
        }

        CPPUNIT_ASSERT_EQUAL(loDict.TotalTags(), 2*N - 1);
        CPPUNIT_ASSERT_EQUAL(loDict.GetInt2(0,0), (apl_intmax_t)0);

        for (apl_size_t i = 0; i < N; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(loDict.Count(i), i);

            loDict.Set(i, std::string("123"));
        }

        CPPUNIT_ASSERT_EQUAL(loDict.TotalTags(), 2*N);

        for (apl_size_t i = 0; i < N; ++i)
        {
            CPPUNIT_ASSERT(apl_strcmp(loDict.Get2(i, 0), "123") == 0);
            CPPUNIT_ASSERT(loDict.GetStr2(i, 0) == std::string("123"));
            CPPUNIT_ASSERT_EQUAL(loDict.GetInt2(i, 0), (apl_intmax_t)123);
        }

        CPPUNIT_ASSERT(loDict.TotalTags() == 2*N);

        for (apl_size_t i = 0; i < N; ++i)
        {
            loDict.Del(i);
            CPPUNIT_ASSERT(!loDict.Has(i));
            CPPUNIT_ASSERT_EQUAL(loDict.Count(i), (apl_size_t)0);
            CPPUNIT_ASSERT_EQUAL(loDict.Get2(i, 0), (char const*)APL_NULL);
            CPPUNIT_ASSERT(loDict.GetStr2(i, 0) == std::string(""));
            CPPUNIT_ASSERT_EQUAL(loDict.GetInt2(i, 0), (apl_intmax_t)0);
        }

        CPPUNIT_ASSERT(loDict.TotalTags() == N);

        for (apl_size_t i = N; i < 2 * N; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(loDict.Count(i), i);

            for (apl_size_t j = 0; j < i; ++j)
            {
                loDict.Del(i, 0);
                CPPUNIT_ASSERT_EQUAL(loDict.Count(i), (apl_size_t)(i - j - 1));
            }

            CPPUNIT_ASSERT(!loDict.Has(i));
            CPPUNIT_ASSERT(loDict.Count(i) == 0);
        }
    }

    void testCompare(void)
    {
        CIndexDict loDict1;
        CIndexDict loDict2;

        CPPUNIT_ASSERT(loDict1 == loDict2);
        CPPUNIT_ASSERT(!(loDict1 != loDict2));

        loDict1.Set(0, "abc");

        CPPUNIT_ASSERT(!(loDict1 == loDict2));
        CPPUNIT_ASSERT(loDict1 != loDict2);

        loDict2.Set(0, "abc");

        CPPUNIT_ASSERT(loDict1 == loDict2);
        CPPUNIT_ASSERT(!(loDict1 != loDict2));

        loDict1.Add(0, "abc");

        CPPUNIT_ASSERT(!(loDict1 == loDict2));
        CPPUNIT_ASSERT(loDict1 != loDict2);

        loDict2.Add(0, "abc");

        CPPUNIT_ASSERT(loDict1 == loDict2);
        CPPUNIT_ASSERT(!(loDict1 != loDict2));

        loDict1.Clear();

        CPPUNIT_ASSERT(!(loDict1 == loDict2));
        CPPUNIT_ASSERT(loDict1 != loDict2);

        loDict2.Clear();

        CPPUNIT_ASSERT(loDict1 == loDict2);
        CPPUNIT_ASSERT(!(loDict1 != loDict2));
    }


    void testSwap(void)
    {
        CIndexDict loDict1;
        loDict1.Set(1, "abc");

        CIndexDict loDict2;

        CIndexDict loDict3(loDict1);

        CIndexDict loDict4;

        loDict4 = loDict1;

        CPPUNIT_ASSERT(loDict1 == loDict3);
        CPPUNIT_ASSERT(loDict1 == loDict4);
        CPPUNIT_ASSERT(loDict1 != loDict2);

        loDict1.Swap(loDict2);

        CPPUNIT_ASSERT(loDict2 == loDict3);
        CPPUNIT_ASSERT(loDict2 == loDict4);
        CPPUNIT_ASSERT(loDict2 != loDict1);

        loDict2.Clear();

        CPPUNIT_ASSERT(loDict2 == loDict1);
        CPPUNIT_ASSERT(loDict3 == loDict4);
        CPPUNIT_ASSERT(loDict3 != loDict1);
    }


    void testDecodeEncode(void)
    {
        apl_int_t const N = 16;
        apl_size_t const S = 1024;

        CIndexDict  loDict1;

        for (apl_int_t i = 0; i < N; ++i)
        {
            for (apl_int_t j = 0; j < i; ++j)
            {
                char lacBuf[24];
                apl_intmax_t n = i * N + j;
                apl_snprintf(lacBuf, sizeof(lacBuf), "%"APL_PRIdMAX, n);

                loDict1.Add(i, lacBuf);
            }
        }

        CMemoryBlock loMB1(S);
        
        apl_int_t liRet = loDict1.Encode(&loMB1);
        apl_errprintf("%.*s\n", loMB1.GetLength(), loMB1.GetReadPtr());

        CPPUNIT_ASSERT(liRet == 0);
        CPPUNIT_ASSERT(loMB1.GetWritePtr()[-1] == 'e');

        CIndexDict loDict2;

        liRet = loDict2.Decode(&loMB1);

        CPPUNIT_ASSERT_EQUAL(liRet, (apl_int_t)0);

        CPPUNIT_ASSERT(loDict1 == loDict2);
        CPPUNIT_ASSERT_EQUAL(loMB1.GetLength(), (apl_size_t)0);

        CMemoryBlock loMB2(S);

        liRet = loDict2.Encode(&loMB2);

        apl_errprintf("%.*s\n", loMB2.GetLength(), loMB2.GetReadPtr());

        CPPUNIT_ASSERT_EQUAL(liRet, (apl_int_t)0);
        CPPUNIT_ASSERT_EQUAL(loMB2.GetWritePtr()[-1], 'e');
    }

};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclIndexDict);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

