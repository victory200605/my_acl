#include "Test.h"
#include "acl/codecs/Base64.h"

class CTestAclBase64Encoder : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclBase64Encoder);
    CPPUNIT_TEST(testFinal);
    CPPUNIT_TEST(testUpdate);
    CPPUNIT_TEST(testGetLength);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}
    void testDowm() {}

    void testFinal()
    {
        char lacInput[10] = { 0x04, 0x20, 0xc4, 0x14, 0x61, 0xc8 };
        char lacOutput1[] = "BCDE";
        char lacOutput2[] = "BCDEFA==";
        char lacOutput3[] = "BCDEFGE=";
        char lacOutput4[] = "BCDEFGHI";
        char lacOutput5[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        unsigned char lacBase64Val[64];
        unsigned char lacOriginalStr[49] = { 0 };

        char lacBuffer[1024];
        acl::CBase64Encoder loBase64Encoder(lacBuffer, sizeof(lacBuffer) );

        //case1
        loBase64Encoder.Final(lacInput, 3);
        CPPUNIT_ASSERT_MESSAGE("test encode final fail", apl_strcmp(loBase64Encoder.GetOutput(), lacOutput1) == 0);
        
        //case2
        loBase64Encoder.Final(lacInput, 4);
        CPPUNIT_ASSERT_MESSAGE("test encoder final fail", apl_strcmp(loBase64Encoder.GetOutput(), lacOutput2) == 0);

        //case3
        loBase64Encoder.Final(lacInput, 5);
        CPPUNIT_ASSERT_MESSAGE("test encoder final fail", apl_strcmp(loBase64Encoder.GetOutput(), lacOutput3) == 0);

        //case4
        loBase64Encoder.Final(lacInput, 6);
        CPPUNIT_ASSERT_MESSAGE("test encoder final fail", apl_strcmp(loBase64Encoder.GetOutput(), lacOutput4) == 0);

        //case5
        for(apl_int_t i = 0;i < (signed)sizeof(lacBase64Val);i++)
        {
            lacBase64Val[i] = i;
        }

        apl_int_t j = 0, liStartBit = 0;
        unsigned char lcTmp, lcTmp1;
        for(apl_int_t i = 0;i < (signed)sizeof(lacBase64Val);i++)
        {
            lcTmp = lacBase64Val[i] << 2;
            lcTmp1 = lcTmp >> liStartBit;
            lacOriginalStr[j] |= lcTmp1;
            if(liStartBit + 6 < 8)
            {
                liStartBit += 6;
            }
            else if(liStartBit + 6 == 8)
            {
                liStartBit = 0;
                j++;
            }
            else
            {
                lcTmp1 = lcTmp << ( 8 - liStartBit );
                j++;
                lacOriginalStr[j] |= lcTmp1;
                liStartBit = (liStartBit + 6) % 8;
            }
        }

        loBase64Encoder.Final(lacOriginalStr, sizeof(lacOriginalStr) - 1);
        CPPUNIT_ASSERT_MESSAGE("test encoder final fail", apl_strcmp(loBase64Encoder.GetOutput(), lacOutput5) == 0);

    }

    void testUpdate()
    {
        char lacInput[10] = { 0x04, 0x20, 0xc4, 0x14, 0x61, 0xc8 };
        char lacOutput1[] = "BCDE";
        char lacOutput2[] = "BCDEFA==";
        char lacOutput3[] = "BCDEFGE=";
        char lacOutput4[] = "BCDEFGHI";

        acl::CMemoryBlock loBuffer(1024);
        acl::CBase64Encoder loBase64Encoder(loBuffer);

        //case1
        loBase64Encoder.Update(lacInput, 3);
        loBase64Encoder.Final();
        CPPUNIT_ASSERT_MESSAGE("test encoder update fail", apl_strcmp(loBase64Encoder.GetOutput(), lacOutput1) == 0);

        //case2
        loBase64Encoder.Update(lacInput, 3);
        loBase64Encoder.Update(lacInput + 3, 1);
        loBase64Encoder.Final();
        CPPUNIT_ASSERT_MESSAGE("test encoder update fail", apl_strcmp(loBase64Encoder.GetOutput(), lacOutput2) == 0);

        //case3
        loBase64Encoder.Update(lacInput, 3);
        loBase64Encoder.Update(lacInput + 3, 1);
        loBase64Encoder.Update(lacInput + 4, 1);
        loBase64Encoder.Final();
        CPPUNIT_ASSERT_MESSAGE("test encoder update fail", apl_strcmp(loBase64Encoder.GetOutput(), lacOutput3) == 0);

        //case4
        loBase64Encoder.Update(lacInput, 3);
        loBase64Encoder.Update(lacInput + 3, 1);
        loBase64Encoder.Update(lacInput + 4, 1);
        loBase64Encoder.Update(lacInput + 5, 1);
        loBase64Encoder.Final();
        CPPUNIT_ASSERT_MESSAGE("test encoder update fail", apl_strcmp(loBase64Encoder.GetOutput(), lacOutput4) == 0);

    }

    void testGetLength()
    {
        char lacInput[10] = { 0x04, 0x20, 0xc4, 0x14, 0x61, 0xc8 };

        acl::CBase64Encoder loBase64Encoder;

        loBase64Encoder.Update(lacInput, 3);
        loBase64Encoder.Final();
        CPPUNIT_ASSERT_MESSAGE("test encoder getlength fail", loBase64Encoder.GetLength() == 4);
    }

    void testReset()
    {
        char lacInput[10] = { 0x04, 0x20, 0xc4, 0x14, 0x61, 0xc8 };
        char lacOutput1[] = "BCDE"; 
        acl::CBase64Encoder loBase64Encoder;

        loBase64Encoder.Update(lacInput, 3);
        loBase64Encoder.Reset();
        loBase64Encoder.Update(lacInput, 3);
        loBase64Encoder.Final();

        CPPUNIT_ASSERT_MESSAGE("test encoder Reset fail", apl_strcmp(loBase64Encoder.GetOutput(), lacOutput1) == 0);

    }
};

class CTestAclBase64Decoder : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclBase64Decoder);
    CPPUNIT_TEST(testFinal);
    CPPUNIT_TEST(testUpdate);
    CPPUNIT_TEST(testGetLength);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST_SUITE_END();

public:
    void startUp() {}
    void tearDown() {}

    void testFinal()
    {
        char lacOutput[10] = { 0x04, 0x20, 0xc4, 0x14, 0x61, 0xc8 };
        char lacInput1[] = "BCDE";
        char lacInput2[] = "BCDEFA==";
        char lacInput3[] = "BCDEFGE=";
        char lacInput4[] = "BCDEFGHI";

        acl::CBase64Decoder loBase64Decoder;

        //case1
        loBase64Decoder.Final(lacInput1);
        CPPUNIT_ASSERT_MESSAGE("test decoder final fail", apl_memcmp(lacOutput, loBase64Decoder.GetOutput(), 3) == 0);

        //case2
        loBase64Decoder.Final(lacInput2);
        CPPUNIT_ASSERT_MESSAGE("test decoder final fail", apl_memcmp(lacOutput, loBase64Decoder.GetOutput(), 4) == 0);

        //case3
        loBase64Decoder.Final(lacInput3);
        CPPUNIT_ASSERT_MESSAGE("test decoder final fail", apl_memcmp(lacOutput, loBase64Decoder.GetOutput(), 5) == 0);
       
        //case4
        loBase64Decoder.Final(lacInput4);
        CPPUNIT_ASSERT_MESSAGE("test decoder final fail", apl_memcmp(lacOutput, loBase64Decoder.GetOutput(), 6) == 0);

        //case5
        char lacInput5[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        unsigned char lacBase64Val[64];
        unsigned char lacOriginalStr[49] = { 0 };

        for(apl_int_t i = 0;i < (signed)sizeof(lacBase64Val);i++)
        {
            lacBase64Val[i] = i;
        }

        apl_int_t j = 0, liStartBit = 0;
        unsigned char lcTmp, lcTmp1;
        for(apl_int_t i = 0;i < (signed)sizeof(lacBase64Val);i++)
        {
            lcTmp = lacBase64Val[i] << 2;
            lcTmp1 = lcTmp >> liStartBit;
            lacOriginalStr[j] |= lcTmp1;
            if(liStartBit + 6 < 8)
            {
                liStartBit += 6;
            }
            else if(liStartBit + 6 == 8)
            {
                liStartBit = 0;
                j++;
            }
            else
            {
                lcTmp1 = lcTmp << ( 8 - liStartBit );
                j++;
                lacOriginalStr[j] |= lcTmp1;
                liStartBit = (liStartBit + 6) % 8;
            }
        }

        loBase64Decoder.Final(lacInput5);
        CPPUNIT_ASSERT_MESSAGE("test decoder final fail", apl_memcmp(loBase64Decoder.GetOutput(), lacOriginalStr, sizeof(lacOriginalStr) - 1) == 0);



    }

    void testUpdate()
    {
        char lacOutput[10] = { 0x04, 0x20, 0xc4, 0x14, 0x61, 0xc8 };
        char lacInput1[] = "BCDE";

        acl::CBase64Decoder loBase64Decoder;

        //case1
        loBase64Decoder.Update(lacInput1, 3);
        loBase64Decoder.Update(lacInput1 + 3, 1);
        loBase64Decoder.Final();
        CPPUNIT_ASSERT_MESSAGE("test decoder update fail", apl_memcmp(lacOutput, loBase64Decoder.GetOutput(), 3) == 0);
    }

    void testGetLength()
    {
        //char lacOutput[10] = { 0x04, 0x20, 0xc4, 0x14, 0x61, 0xc8 };
        char lacInput1[] = "BCDE";

        acl::CBase64Decoder loBase64Decoder;

        loBase64Decoder.Final(lacInput1);
        CPPUNIT_ASSERT_MESSAGE("test decoder getlength fail", loBase64Decoder.GetLength() == 3);
    }

    void testReset()
    {
        char lacOutput[10] = { 0x04, 0x20, 0xc4, 0x14, 0x61, 0xc8 };
        char lacInput1[] = "BCDE";

        acl::CBase64Decoder loBase64Decoder;

        loBase64Decoder.Update(lacInput1);
        loBase64Decoder.Reset();
        loBase64Decoder.Update(lacInput1);
        CPPUNIT_ASSERT_MESSAGE("test decoder reset fail", apl_memcmp(lacOutput, loBase64Decoder.GetOutput(), 3) == 0);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclBase64Encoder);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclBase64Decoder);

int main()
{
        RUN_ALL_TEST(__FILE__);
}
