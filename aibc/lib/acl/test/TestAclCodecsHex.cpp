#include "Test.h"
#include "acl/Codecs.h"
#include "acl/Timestamp.h"

using namespace acl;

class CTestAclCodecsHex : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclCodecsHex);
    CPPUNIT_TEST(testEncode1);
    CPPUNIT_TEST(testEncode2);
    CPPUNIT_TEST(testEncode3);
    CPPUNIT_TEST(testDecode1);
    CPPUNIT_TEST(testDecode2);
    CPPUNIT_TEST(testDecode3);
    CPPUNIT_TEST(testStability);
    CPPUNIT_TEST_SUITE_END();
    
public:
    void setUp(void) {}
    void tearDown(void) {}

    void testEncode1(void)
    {
        PRINT_TITLE_2(__func__);
        
        CHexEncoder loEncoder(CHexEncoder::OPT_UPPER);
        char lacBuffer[128] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        
        //case 1
        ASSERT_MESSAGE( loEncoder.Update(lacBuffer, 4 ) == 0);
        ASSERT_MESSAGE( loEncoder.Update(lacBuffer + 4, 4 ) == 0);
        ASSERT_MESSAGE( loEncoder.Final() == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "0123456789ABCDEF") == 0);
        
        //case 2
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer, 4 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "01234567") == 0);
        
        //case 3
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer + 4, 4 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "89ABCDEF") == 0);
        
        //case 4
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer, 8 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "0123456789ABCDEF") == 0);
    }
    
    void testEncode2(void)
    {
        PRINT_TITLE_2(__func__);
        
        char lacTemp[1024];
        CHexEncoder loEncoder(lacTemp, sizeof(lacTemp), CHexEncoder::OPT_LOWER);
        char lacBuffer[128] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        
        //case 1
        ASSERT_MESSAGE( loEncoder.Update(lacBuffer, 4 ) == 0);
        ASSERT_MESSAGE( loEncoder.Update(lacBuffer + 4, 4 ) == 0);
        ASSERT_MESSAGE( loEncoder.Final() == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "0123456789abcdef") == 0);
        
        //case 2
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer, 4 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "01234567") == 0);
        
        //case 3
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer + 4, 4 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "89abcdef") == 0);
        
        //case 4
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer, 8 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "0123456789abcdef") == 0);
    }
    
    void testEncode3(void)
    {
        PRINT_TITLE_2(__func__);
        
        CMemoryBlock loBuffer(1024);
        CHexEncoder loEncoder(loBuffer, CHexEncoder::OPT_LOWER);
        char lacBuffer[128] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        
        //case 1
        ASSERT_MESSAGE( loEncoder.Update(lacBuffer, 4 ) == 0);
        ASSERT_MESSAGE( loEncoder.Update(lacBuffer + 4, 4 ) == 0);
        ASSERT_MESSAGE( loEncoder.Final() == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "0123456789abcdef") == 0);
        
        //case 2
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer, 4 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "01234567") == 0);
        
        //case 3
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer + 4, 4 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "89abcdef") == 0);
        
        //case 4
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer, 8 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "0123456789abcdef") == 0);
    }
    
    void testDecode1(void)
    {
        PRINT_TITLE_2(__func__);
        
        CHexDecoder loDecoder;
        char lacBuffer[128] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        
        //case 1
        ASSERT_MESSAGE( loDecoder.Update("01234567", 8 ) == 0);
        ASSERT_MESSAGE( loDecoder.Update("89ABCDEF", 8 ) == 0);
        ASSERT_MESSAGE( loDecoder.Final() == 0);
        ASSERT_MESSAGE( apl_memcmp(loDecoder.GetOutput(), lacBuffer, loDecoder.GetLength() ) == 0);
        
        //case 2
        ASSERT_MESSAGE( loDecoder.Final("0123456789ABCDEF", 16 ) == 0);
        ASSERT_MESSAGE( apl_memcmp(loDecoder.GetOutput(), lacBuffer, loDecoder.GetLength() ) == 0);
    }
    
    void testDecode2(void)
    {
        PRINT_TITLE_2(__func__);
        
        char lacTemp[1024];
        CHexDecoder loDecoder(lacTemp, sizeof(lacTemp) );
        char lacBuffer[128] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        
        //case 1
        ASSERT_MESSAGE( loDecoder.Update("01234567", 8 ) == 0);
        ASSERT_MESSAGE( loDecoder.Update("89abcdef", 8 ) == 0);
        ASSERT_MESSAGE( loDecoder.Final() == 0);
        ASSERT_MESSAGE( apl_memcmp(loDecoder.GetOutput(), lacBuffer, loDecoder.GetLength() ) == 0);
        
        //case 2
        ASSERT_MESSAGE( loDecoder.Final("0123456789abcdef", 16 ) == 0);
        ASSERT_MESSAGE( apl_memcmp(loDecoder.GetOutput(), lacBuffer, loDecoder.GetLength() ) == 0);
    }
    
    void testDecode3(void)
    {
        PRINT_TITLE_2(__func__);
        
        CMemoryBlock loBuffer(1024);
        CHexDecoder loDecoder(loBuffer);
        char lacBuffer[128] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        
        //case 1
        ASSERT_MESSAGE( loDecoder.Update("012\n3456\t7", 10 ) == 0);
        ASSERT_MESSAGE( loDecoder.Update("89a bc def", 10 ) == 0);
        ASSERT_MESSAGE( loDecoder.Final() == 0);
        ASSERT_MESSAGE( apl_memcmp(loDecoder.GetOutput(), lacBuffer, loDecoder.GetLength() ) == 0);
        
        //case 2
        ASSERT_MESSAGE( loDecoder.Final("0123456789abcdef", 16 ) == 0);
        ASSERT_MESSAGE( apl_memcmp(loDecoder.GetOutput(), lacBuffer, loDecoder.GetLength() ) == 0);
        
        //case 3
        ASSERT_MESSAGE( loDecoder.Final("0123456789abcdefx", 17 ) == -1);
    }
    
    void testStability(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        struct CParam {
            CParam(CHexEncoder& aoEncoder, CHexDecoder& aoDecoder, apl_int_t& aiCount)
                : moEncoder(aoEncoder)
                , moDecoder(aoDecoder)
                , miCount(aiCount)
            {}
            
            CHexEncoder& moEncoder;
            CHexDecoder& moDecoder;
            volatile apl_int_t& miCount;
        };
        
        CHexEncoder loEncoder;
        CHexDecoder loDecoder;
        apl_int_t liCount = 0;
        
        CParam loParam(loEncoder, loDecoder, liCount);
        
        printf("\n");
        START_THREAD_BODY(mybody1, CParam, loParam)
            char lacBuffer[128] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
            START_LOOP(100000)
            {
                assert(loParam.moEncoder.Final(lacBuffer, 8 ) == 0);
            }
            END_LOOP();
            //Use time=0.039000, 2564102.564103/pre
            loParam.miCount = 1;
        END_THREAD_BODY(mybody1);
        
        START_THREAD_BODY(mybody2, CParam, loParam)
            START_LOOP(100000)
            {
                assert(loParam.moDecoder.Final("0123456789abcdef", 16 ) == 0);
            }
            END_LOOP();
            //Use time=0.046000, 2173913.043478/pre
            WAIT_EXP(loParam.miCount == 1);
            loParam.miCount = 2;
        END_THREAD_BODY(mybody2);

        RUN_THREAD_BODY(mybody1);
        RUN_THREAD_BODY(mybody2);
        WAIT_EXP(liCount == 2);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclCodecsHex);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
