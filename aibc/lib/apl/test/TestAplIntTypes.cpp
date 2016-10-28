#include "Test.h"


#define TEST_INTN_PRINTF(bits) \
    do { \
        apl_errprintf("apl_int" #bits "_t MAX: %" APL_PRId##bits "\n", APL_INT##bits##_MAX); \
        apl_errprintf("apl_int" #bits "_t MAX: %" APL_PRIi##bits "\n", APL_INT##bits##_MAX); \
        apl_errprintf("apl_int" #bits "_t MIN: %" APL_PRId##bits "\n", APL_INT##bits##_MIN); \
        apl_errprintf("apl_int" #bits "_t MIN: %" APL_PRIi##bits "\n", APL_INT##bits##_MIN); \
        apl_errprintf("apl_uint" #bits "_t MAX: %" APL_PRIu##bits "\n", APL_UINT##bits##_MAX); \
        apl_errprintf("apl_uint" #bits "_t MAX: %" APL_PRIo##bits "\n", APL_UINT##bits##_MAX); \
        apl_errprintf("apl_uint" #bits "_t MAX: %" APL_PRIx##bits "\n", APL_UINT##bits##_MAX); \
        apl_errprintf("apl_uint" #bits "_t MAX: %" APL_PRIX##bits "\n", APL_UINT##bits##_MAX); \
    } while(0)



#define TEST_STRTOI(pritype, apltyp, num, name, base) \
    do { \
        char lac_chr[64]; \
        char lac_msg[64]; \
        apltyp li_al = (apltyp)(num); \
        sprintf(lac_chr, "%"pritype, li_al); \
        apltyp li_ap = (apltyp)name(lac_chr, (char**)NULL, base); \
        sprintf(lac_msg, "Fail:%s:%s", #name, lac_chr); \
        CPPUNIT_ASSERT_EQUAL_MESSAGE(lac_msg, li_ap, li_al);  \
    } while(0) 
 


#define TEST_INT_SIZE(apltyp, typ) \
    do { \
        CPPUNIT_ASSERT_EQUAL_MESSAGE( \
            "sizeof(" #apltyp ") != sizeof(" #typ ")", \
            sizeof(apltyp), \
            sizeof(typ) ); \
    } while(0)


#define  TEST_INT_C(apltyp, num) \
    do { \
        int li_al = sizeof(apltyp); \
        CPPUNIT_ASSERT_EQUAL_MESSAGE(#apltyp"failed", li_al, num ); \
    } while(0)


#define TEST_MAX_LIMIT(apltyp, aplmax, aplmin) \
    do { \
        apltyp li_ai = aplmax; \
        apltyp li_ai_min = aplmin; \
        ++li_ai; \
        CPPUNIT_ASSERT_EQUAL_MESSAGE("overflow Fail:"#apltyp, li_ai, li_ai_min); \
    } while(0)


#define TEST_MIN_LIMIT(apltyp, aplmin, aplmax)  \
    do { \
        apltyp li_ai = aplmin; \
        apltyp li_ai_max = aplmax; \
        --li_ai; \
        CPPUNIT_ASSERT_EQUAL_MESSAGE("downflow Fail:"#apltyp, li_ai, li_ai_max); \
    } while(0)


#define TEST_BYTE_ORDER(bits, num) \
    do { \
        apl_uint##bits##_t  li_num = num; \
        apl_uint##bits##_t  li_net  = apl_hton##bits(num); \
        apl_uint##bits##_t  li_host = apl_ntoh##bits(li_net); \
        char        lac_msg[256]; \
        \
        apl_snprintf( \
            lac_msg, \
            sizeof(lac_msg), \
            "apl_ntoh"#bits"(apl_hton"#bits"(0x%"APL_PRIX##bits")) != 0x%"APL_PRIX##bits"", \
            li_num, \
            li_host); \
        \
        CPPUNIT_ASSERT_EQUAL_MESSAGE(lac_msg, li_host, li_num); \
        \
        for (apl_size_t li_itr = 0; \
            li_itr < sizeof(apl_uint##bits##_t); \
            ++li_itr) \
        { \
            apl_uint8_t lu8_n = ((apl_uint8_t*)&li_net)[li_itr]; \
            apl_uint8_t lu8_h = li_host >> ((sizeof(apl_uint##bits##_t) - li_itr - 1)*8); \
            \
            apl_snprintf( \
                lac_msg, \
                sizeof(lac_msg), \
                "bytes[%"APL_PRIdINT"] of apl_uint"#bits"_t check error", \
                li_itr); \
            \
	    CPPUNIT_ASSERT_EQUAL_MESSAGE(lac_msg, lu8_n, lu8_h); \
        } \
    } while(0)

        
/**
 * @brief Test suite INT range.
 * @brief Including three test cases: testRange, testOrder and testPrint.
 */
class CTestaplint_range: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplint_range);
    CPPUNIT_TEST(testRange);
    CPPUNIT_TEST(testOrder);
    CPPUNIT_TEST(testPrint);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}

    /** 
     * Test case testRange.
     */
    void testRange(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case 
        testIntSize();
        testIntMin();
        testIntMax();
        testIntStr();

        //end environment
    }

    /** 
     * Test case testPrint
     */
    void testPrint(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case 
        TEST_INTN_PRINTF(8);
        TEST_INTN_PRINTF(16);
        TEST_INTN_PRINTF(32);
        TEST_INTN_PRINTF(64);

        //end environment
    }

    /** 
     * Test case testOrder
     */
    void testOrder(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        testIntHn();
        //testIntIntc();
        
        //end environment
    }

    void testIntSize(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_INT_SIZE(apl_int8_t, int8_t);
        TEST_INT_SIZE(apl_int16_t, int16_t);
        TEST_INT_SIZE(apl_int32_t, int32_t);
        TEST_INT_SIZE(apl_int64_t, int64_t);
        TEST_INT_SIZE(apl_intmax_t, intmax_t);
        TEST_INT_SIZE(apl_int_t, intptr_t);
    
        TEST_INT_SIZE(apl_uint8_t, uint8_t);
        TEST_INT_SIZE(apl_uint16_t, uint16_t);
        TEST_INT_SIZE(apl_uint32_t, uint32_t);
        TEST_INT_SIZE(apl_uint64_t, uint64_t);
        TEST_INT_SIZE(apl_uintmax_t, uintmax_t);
        TEST_INT_SIZE(apl_uint_t, uintptr_t);
    
    
        if ( sizeof(apl_intmax_t) < sizeof(apl_int64_t) )
        {
            CPPUNIT_FAIL("apl_int64_t > apl_intmax_t ");
        }
        if ( sizeof(apl_uintmax_t) < sizeof(apl_uint64_t) )
        {
            CPPUNIT_FAIL("apl_uint64_t >apl_uintmax_t ");
        }
        if ( sizeof(apl_int_t) != sizeof(void *) )
        {
            CPPUNIT_FAIL("the size of apl_int_t failed");
        }
    
        if ( sizeof(apl_uint_t) != sizeof(void *) )
        {
            CPPUNIT_FAIL("the size of apl_uint_t failed");
        }

    }

    void testIntMin(void)
    {
        PRINT_TITLE_2(__func__);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_INT8_MIN failed", (INT8_MIN), APL_INT8_MIN  );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_INT16_MIN failed", (INT16_MIN), APL_INT16_MIN );  
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_INT32_MIN failed", (INT32_MIN), APL_INT32_MIN );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_INT64_MIN failed", (INT64_MIN), APL_INT64_MIN );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_INTMAX_MIN failed", (INTMAX_MIN), APL_INTMAX_MIN );
        //CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_INT_MIN failed", (INTPTR_MIN), APL_INT_MIN );
    
     
        TEST_MIN_LIMIT(apl_int8_t, APL_INT8_MIN, APL_INT8_MAX);
        TEST_MIN_LIMIT(apl_int16_t, APL_INT16_MIN, APL_INT16_MAX);
        TEST_MIN_LIMIT(apl_int32_t, APL_INT32_MIN, APL_INT32_MAX);
        TEST_MIN_LIMIT(apl_int64_t, APL_INT64_MIN, APL_INT64_MAX);
    }
    void testIntMax(void)
    {
        PRINT_TITLE_2(__func__);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_INT8_MAX failed", (INT8_MAX), APL_INT8_MAX );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_INT16_MAX failed", (INT16_MAX), APL_INT16_MAX);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_INT32_MAX failed", (INT32_MAX), APL_INT32_MAX);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_INT64_MAX failed", (INT64_MAX), APL_INT64_MAX);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_INTMAX_MAX failed", (INTMAX_MAX), APL_INTMAX_MAX);
        //CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_UINT_MAX failed", (UINTPTR_MAX), APL_UINT_MAX );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_UINT8_MAX failed", (UINT8_MAX), APL_UINT8_MAX);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_UINT16_MAX failed", (UINT16_MAX), APL_UINT16_MAX);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_UINT32_MAX failed", (UINT32_MAX), APL_UINT32_MAX);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_UINT64_MAX failed", (UINT64_MAX), APL_UINT64_MAX);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_UINTMAX_MAX failed", (UINTMAX_MAX), APL_UINTMAX_MAX);
        //CPPUNIT_ASSERT_EQUAL_MESSAGE("APL_UINT_MAX failed", (UINTPTR_MAX), APL_UINT_MAX);
    
    
        TEST_MAX_LIMIT(apl_int8_t, APL_INT8_MAX, APL_INT8_MIN);
        TEST_MAX_LIMIT(apl_int16_t, APL_INT16_MAX, APL_INT16_MIN);
        TEST_MAX_LIMIT(apl_int32_t, APL_INT32_MAX, APL_INT32_MIN);
        TEST_MAX_LIMIT(apl_int64_t, APL_INT64_MAX, APL_INT64_MIN);
    }
    void testIntStr(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_STRTOI(APL_PRId8, apl_int8_t, APL_INT8_MAX, apl_strtoi32, 10);
        TEST_STRTOI(APL_PRId8, apl_int8_t, APL_INT8_MIN, apl_strtoi32, 10);
    
        TEST_STRTOI(APL_PRId16, apl_int16_t, APL_INT16_MAX, apl_strtoi32, 10);
        TEST_STRTOI(APL_PRId16, apl_int16_t, APL_INT16_MIN, apl_strtoi32, 10);
    
        TEST_STRTOI(APL_PRId32, apl_int32_t, APL_INT32_MAX, apl_strtoi32, 10);
        TEST_STRTOI(APL_PRId32, apl_int32_t, APL_INT32_MIN, apl_strtoi32, 10);
    
        TEST_STRTOI(APL_PRId64, apl_int64_t, APL_INT64_MAX, apl_strtoi64, 10);
        TEST_STRTOI(APL_PRId64, apl_int64_t, APL_INT64_MIN, apl_strtoi64, 10);
    
        TEST_STRTOI(APL_PRIdMAX, apl_intmax_t, APL_INTMAX_MAX, apl_strtoimax, 10);
        TEST_STRTOI(APL_PRIdMAX, apl_intmax_t, APL_INTMAX_MIN, apl_strtoimax, 10);

#if (SIZEOF_INTPTR_T == SIZEOF_INT32_T)
        TEST_STRTOI(APL_PRIdINT, apl_int_t, APL_INT_MAX, apl_strtoi32, 10);
        TEST_STRTOI(APL_PRIdINT, apl_int_t, APL_INT_MIN, apl_strtoi32, 10);
#elif (SIZEOF_INTPTR_T == SIZEOF_INT64_T)
        TEST_STRTOI(APL_PRIdINT, apl_int_t, APL_INT_MAX, apl_strtoi64, 10);
        TEST_STRTOI(APL_PRIdINT, apl_int_t, APL_INT_MIN, apl_strtoi64, 10);
#endif 
    
        TEST_STRTOI(APL_PRIi8 , apl_int8_t, APL_INT8_MAX, apl_strtoi32, 10);
        TEST_STRTOI(APL_PRIi8 , apl_int8_t, APL_INT8_MIN, apl_strtoi32, 10);
    
        TEST_STRTOI(APL_PRIi16, apl_int16_t, APL_INT16_MAX, apl_strtoi32, 10);
        TEST_STRTOI(APL_PRIi16, apl_int16_t, APL_INT16_MIN, apl_strtoi32, 10);
    
        TEST_STRTOI(APL_PRIi32, apl_int32_t, APL_INT32_MAX, apl_strtoi32, 10);
        TEST_STRTOI(APL_PRIi32, apl_int32_t, APL_INT32_MIN, apl_strtoi32, 10);
    
        TEST_STRTOI(APL_PRIi64, apl_int64_t, APL_INT64_MAX, apl_strtoi64, 10);
        TEST_STRTOI(APL_PRIi64, apl_int64_t, APL_INT64_MIN, apl_strtoi64, 10);
    
        TEST_STRTOI(APL_PRIiMAX, apl_intmax_t, APL_INTMAX_MAX, apl_strtoimax, 10);
        TEST_STRTOI(APL_PRIiMAX, apl_intmax_t, APL_INTMAX_MIN, apl_strtoimax, 10);
    
#if (SIZEOF_INTPTR_T == SIZEOF_INT32_T)
        TEST_STRTOI(APL_PRIiINT, apl_int_t, APL_INT_MAX, apl_strtoi32, 10);
        TEST_STRTOI(APL_PRIiINT, apl_int_t, APL_INT_MIN, apl_strtoi32, 10);
#elif (SIZEOF_INTPTR_T == SIZEOF_INT64_T)
        TEST_STRTOI(APL_PRIiINT, apl_int_t, APL_INT_MAX, apl_strtoi64, 10);
        TEST_STRTOI(APL_PRIiINT, apl_int_t, APL_INT_MIN, apl_strtoi64, 10);
#endif
    
        
        TEST_STRTOI(APL_PRIu8, apl_uint8_t, APL_UINT8_MAX, apl_strtou32, 10);
        TEST_STRTOI(APL_PRIu16, apl_uint16_t, APL_UINT16_MAX, apl_strtou32, 10);
        TEST_STRTOI(APL_PRIu32, apl_uint32_t, APL_UINT32_MAX, apl_strtou32, 10);
        TEST_STRTOI(APL_PRIu64, apl_uint64_t, APL_UINT64_MAX, apl_strtou64, 10);
        TEST_STRTOI(APL_PRIuMAX, apl_uintmax_t, APL_UINTMAX_MAX, apl_strtoumax, 10);

#if (SIZEOF_INTPTR_T == SIZEOF_INT32_T)
        TEST_STRTOI(APL_PRIuINT, apl_uint_t, APL_UINT_MAX, apl_strtou32, 10);
#elif (SIZEOF_INTPTR_T == SIZEOF_INT64_T)
        TEST_STRTOI(APL_PRIuINT, apl_uint_t, APL_UINT_MAX, apl_strtou64, 10);
#endif
    
    
        TEST_STRTOI(APL_PRIo8, apl_uint8_t, APL_UINT8_MAX, apl_strtou32, 8);
        TEST_STRTOI(APL_PRIo16, apl_uint16_t, APL_UINT16_MAX, apl_strtou32, 8);
        TEST_STRTOI(APL_PRIo32, apl_uint32_t, APL_UINT32_MAX, apl_strtou32, 8);
        TEST_STRTOI(APL_PRIo64, apl_uint64_t, APL_UINT64_MAX, apl_strtou64, 8);
        TEST_STRTOI(APL_PRIoMAX, apl_uintmax_t, APL_UINTMAX_MAX, apl_strtoumax, 8);
#if (SIZEOF_INTPTR_T == SIZEOF_INT32_T)
        TEST_STRTOI(APL_PRIoINT, apl_uint_t, APL_UINT_MAX, apl_strtou32, 8);
#elif (SIZEOF_INTPTR_T == SIZEOF_INT64_T)
        TEST_STRTOI(APL_PRIoINT, apl_uint_t, APL_UINT_MAX, apl_strtou64, 8);
#endif
    
    
        TEST_STRTOI(APL_PRIx8, apl_uint8_t, APL_UINT8_MAX, apl_strtou32, 16);
        TEST_STRTOI(APL_PRIx16, apl_uint16_t, APL_UINT16_MAX, apl_strtou32, 16);
        TEST_STRTOI(APL_PRIx32, apl_uint32_t, APL_UINT32_MAX, apl_strtou32, 16);
        TEST_STRTOI(APL_PRIx64, apl_uint64_t, APL_UINT64_MAX, apl_strtou64, 16);
        TEST_STRTOI(APL_PRIxMAX, apl_uintmax_t, APL_UINTMAX_MAX, apl_strtoumax, 16);
#if (SIZEOF_INTPTR_T == SIZEOF_INT32_T)
        TEST_STRTOI(APL_PRIxINT, apl_uint_t, APL_UINT_MAX, apl_strtou32, 16);
#elif (SIZEOF_INTPTR_T == SIZEOF_INT64_T)
        TEST_STRTOI(APL_PRIxINT, apl_uint_t, APL_UINT_MAX, apl_strtou64, 16);
#endif


        TEST_STRTOI(APL_PRIX8, apl_uint8_t, APL_UINT8_MAX, apl_strtou32, 16);
        TEST_STRTOI(APL_PRIX16, apl_uint16_t, APL_UINT16_MAX, apl_strtou32, 16);
        TEST_STRTOI(APL_PRIX32, apl_uint32_t, APL_UINT32_MAX, apl_strtou32, 16);
        TEST_STRTOI(APL_PRIX64, apl_uint64_t, APL_UINT64_MAX, apl_strtou64, 16);
        TEST_STRTOI(APL_PRIXMAX, apl_uintmax_t, APL_UINTMAX_MAX, apl_strtoumax, 16);
#if (SIZEOF_INTPTR_T == SIZEOF_INT32_T)
        TEST_STRTOI(APL_PRIXINT, apl_uint_t, APL_UINT_MAX, apl_strtou32, 16);
#elif (SIZEOF_INTPTR_T == SIZEOF_INT64_T)
        TEST_STRTOI(APL_PRIXINT, apl_uint_t, APL_UINT_MAX, apl_strtou64, 16);
#endif
    }

    void testIntHn(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_BYTE_ORDER(16, APL_UINT16_C(0x0102));
        TEST_BYTE_ORDER(32, APL_UINT32_C(0x01020304));
        TEST_BYTE_ORDER(64, APL_UINT64_C(0x0102030405060708));
    }

    void testIntIntc(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_INT_C(APL_INT8_C(1) , 1);
        TEST_INT_C(APL_INT16_C(1), 2);
        TEST_INT_C(APL_INT32_C(1), 4);
        TEST_INT_C(APL_INT64_C(1), 8);
        int isize = sizeof(apl_intmax_t);
        TEST_INT_C(APL_INTMAX_C(1), isize);
    }
};



CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplint_range);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
