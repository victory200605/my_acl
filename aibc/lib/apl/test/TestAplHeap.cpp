#include "Test.h"

/*
 * mal : malloc numbers
 * real: realloc numbers
*/
#define TEST_HEAP(mal, real) \
    do { \
        void *lpv = APL_NULL; \
        if ( mal > 0) {\
            lpv = apl_malloc(mal); \
            CPPUNIT_ASSERT(lpv != APL_NULL); \
        } \
        if ( real > 0) \
        {\
            lpv = apl_realloc(lpv, real); \
            CPPUNIT_ASSERT(lpv != APL_NULL); \
        } \
        if ( APL_NULL != lpv ) \
        {\
            apl_free(lpv); \
        } \
    } while(0)

/**
 * @brief Test suite Heap.
 * @brief Involved malloc, realloc and free.
 */
class CTestaplheap: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplheap);
    CPPUNIT_TEST(testHeap);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}

    /**
     * Test case heap involved malloc, realloc and free.
     */
    void testHeap(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        TEST_HEAP(1024, 0); ///malloc/free 
        TEST_HEAP(0, 2048); ///realloc/free 
        TEST_HEAP(1024, 2048); ///malloc/realloc/free 
        TEST_HEAP(2048, 1024); ///malloc/realloc/free 

        //end environment
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplheap);
int main()
{
    RUN_ALL_TEST(__FILE__);
}
