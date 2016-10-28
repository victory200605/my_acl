#include "Test.h"
#include "acl/Codecs.h"
#include "acl/Timestamp.h"

using namespace acl;

class CTestAclCodecsBinary : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclCodecsBinary);
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
        
        char lacTemp[1024];
        CBinaryEncoder loEncoder(lacTemp, sizeof(lacTemp) );
        char lacBuffer[128] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        
        int liN = 0x04030201;
        liN = apl_hton32(liN);
        loEncoder.Final(&liN, 4);
        printf("%s\n", loEncoder.GetOutput() );
        return;
        //case 1
        ASSERT_MESSAGE( loEncoder.Update(lacBuffer, 4 ) == 0);
        ASSERT_MESSAGE( loEncoder.Update(lacBuffer + 4, 4 ) == 0);
        ASSERT_MESSAGE( loEncoder.Final() == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "0000000100100011010001010110011110001001101010111100110111101111") == 0);
        
        //case 2
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer, 4 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "00000001001000110100010101100111") == 0);
        
        //case 3
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer + 4, 4 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "10001001101010111100110111101111") == 0);
        
        //case 4
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer, 8 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "0000000100100011010001010110011110001001101010111100110111101111") == 0);
    }

    void testEncode2(void)
    {
        PRINT_TITLE_2(__func__);
        
        CMemoryBlock loBuffer(1024);
        CBinaryEncoder loEncoder(loBuffer);
        char lacBuffer[128] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        
        int liN = 0x04030201;
        liN = apl_hton32(liN);
        loEncoder.Final(&liN, 4);
        printf("%s\n", loEncoder.GetOutput() );
        return;
        //case 1
        ASSERT_MESSAGE( loEncoder.Update(lacBuffer, 4 ) == 0);
        ASSERT_MESSAGE( loEncoder.Update(lacBuffer + 4, 4 ) == 0);
        ASSERT_MESSAGE( loEncoder.Final() == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "0000000100100011010001010110011110001001101010111100110111101111") == 0);
        
        //case 2
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer, 4 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "00000001001000110100010101100111") == 0);
        
        //case 3
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer + 4, 4 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "10001001101010111100110111101111") == 0);
        
        //case 4
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer, 8 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "0000000100100011010001010110011110001001101010111100110111101111") == 0);
    }

    void testEncode3(void)
    {
        PRINT_TITLE_2(__func__);
        
        CBinaryEncoder loEncoder;
        char lacBuffer[128] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        
        int liN = 0x04030201;
        liN = apl_hton32(liN);
        loEncoder.Final(&liN, 4);
        printf("%s\n", loEncoder.GetOutput() );
        return;
        //case 1
        ASSERT_MESSAGE( loEncoder.Update(lacBuffer, 4 ) == 0);
        ASSERT_MESSAGE( loEncoder.Update(lacBuffer + 4, 4 ) == 0);
        ASSERT_MESSAGE( loEncoder.Final() == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "0000000100100011010001010110011110001001101010111100110111101111") == 0);
        
        //case 2
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer, 4 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "00000001001000110100010101100111") == 0);
        
        //case 3
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer + 4, 4 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "10001001101010111100110111101111") == 0);
        
        //case 4
        ASSERT_MESSAGE( loEncoder.Final(lacBuffer, 8 ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), "0000000100100011010001010110011110001001101010111100110111101111") == 0);
    }

    void testDecode1(void)
    {
        PRINT_TITLE_2(__func__);
        
        char lacTemp[1024];
        CBinaryDecoder loDecoder(lacTemp, sizeof(lacTemp) );
        char lacBuffer[128] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        
        //case 1
        ASSERT_MESSAGE( loDecoder.Update("00000001001000110100010101100111", 32 ) == 0);
        ASSERT_MESSAGE( loDecoder.Update("10001001101010111100110111101111", 32 ) == 0);
        ASSERT_MESSAGE( loDecoder.Final() == 0);
        ASSERT_MESSAGE( apl_memcmp(loDecoder.GetOutput(), lacBuffer, loDecoder.GetLength() ) == 0);
        
        //case 2
        ASSERT_MESSAGE( loDecoder.Final("0000000100100011010001010110011110001001101010111100110111101111", 64 ) == 0);
        ASSERT_MESSAGE( apl_memcmp(loDecoder.GetOutput(), lacBuffer, loDecoder.GetLength() ) == 0);
    }

    void testDecode2(void)
    {
        PRINT_TITLE_2(__func__);
        
        CBinaryDecoder loDecoder;
        char lacBuffer[128] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        
        //case 1
        ASSERT_MESSAGE( loDecoder.Update("10001001101 01\n0\t111100110111101111", 35 ) == 0);
        ASSERT_MESSAGE( loDecoder.Update("1000100110101 0111\n100110111101111", 34 ) == 0);
        ASSERT_MESSAGE( loDecoder.Final() == 0);
        ASSERT_MESSAGE( apl_memcmp(loDecoder.GetOutput(), lacBuffer, loDecoder.GetLength() ) == 0);
        
        //case 2
        ASSERT_MESSAGE( loDecoder.Final("0000000100100011010001010110011110001001101010111100110111101111", 64 ) == 0);
        ASSERT_MESSAGE( apl_memcmp(loDecoder.GetOutput(), lacBuffer, loDecoder.GetLength() ) == 0);
        
        //case 3
        ASSERT_MESSAGE( loDecoder.Final("0000000100100011010001010110011110001001101010111100110111101112", 64 ) == -1);
    }

    void testDecode3(void)
    {
        PRINT_TITLE_2(__func__);
        
        CMemoryBlock loBuffer(1024);
        CBinaryDecoder loDecoder(loBuffer);
        char lacBuffer[128] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        
        //case 1
        ASSERT_MESSAGE( loDecoder.Update("00000001001000110100010101100111", 32 ) == 0);
        ASSERT_MESSAGE( loDecoder.Update("10001001101010111100110111101111", 32 ) == 0);
        ASSERT_MESSAGE( loDecoder.Final() == 0);
        ASSERT_MESSAGE( apl_memcmp(loDecoder.GetOutput(), lacBuffer, loDecoder.GetLength() ) == 0);
        
        //case 2
        ASSERT_MESSAGE( loDecoder.Final("0000000100100011010001010110011110001001101010111100110111101111", 64 ) == 0);
        ASSERT_MESSAGE( apl_memcmp(loDecoder.GetOutput(), lacBuffer, loDecoder.GetLength() ) == 0);
    }

    
    void testStability(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        struct CParam {
            CParam(CBinaryEncoder& aoEncoder, CBinaryDecoder& aoDecoder, apl_int_t& aiCount)
                : moEncoder(aoEncoder)
                , moDecoder(aoDecoder)
                , miCount(aiCount)
            {}
            
            CBinaryEncoder& moEncoder;
            CBinaryDecoder& moDecoder;
            volatile apl_int_t& miCount;
        };
        
        CBinaryEncoder loEncoder;
        CBinaryDecoder loDecoder;
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
            //Use time=0.076000, 1315789.473684/pre
            loParam.miCount = 1;
        END_THREAD_BODY(mybody1);
        
        START_THREAD_BODY(mybody2, CParam, loParam)
            START_LOOP(100000)
            {
                assert(loParam.moDecoder.Final("0000000100100011010001010110011110001001101010111100110111101111", 64 ) == 0);
            }
            END_LOOP();
            //Use time=0.154000, 649350.649351/pre
            WAIT_EXP(loParam.miCount == 1);
            loParam.miCount = 2;
        END_THREAD_BODY(mybody2);

        RUN_THREAD_BODY(mybody1);
        RUN_THREAD_BODY(mybody2);
        WAIT_EXP(liCount == 2);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclCodecsBinary);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
