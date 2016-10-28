#include "Test.h"
#include "acl/MemoryStream.h"

using namespace acl;

class CTestAclMemoryBlock : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclMemoryBlock);
    CPPUNIT_TEST(testSetOrGet);
    CPPUNIT_TEST(testReadVS);
    CPPUNIT_TEST(testReadIS1);
    CPPUNIT_TEST(testReadIS2);
    CPPUNIT_TEST(testReadIS3);
    CPPUNIT_TEST(testReadLine1);
    CPPUNIT_TEST(testReadLine2);
    CPPUNIT_TEST(testReadString1);
    CPPUNIT_TEST(testReadString2);
    CPPUNIT_TEST(testReadT);
    CPPUNIT_TEST(testReadUntil);
    CPPUNIT_TEST(testReadTo);
    CPPUNIT_TEST(testReadToU);
    CPPUNIT_TEST(testWrite);
    CPPUNIT_TEST(testWriteV1);
    CPPUNIT_TEST(testWriteV2);
    CPPUNIT_TEST(testWriteV3);
    CPPUNIT_TEST(testWriteT);
    CPPUNIT_TEST(testWriteTo);
    CPPUNIT_TEST(testWriteToU);
    CPPUNIT_TEST_SUITE_END();
    
public:
    void setUp(void) {}
    void tearDown(void) {}
    
    void testSetOrGet(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream;
        
        loStream.SetMemoryBlock(&loBlock);
        
        CPPUNIT_ASSERT_MESSAGE("SetOrGet", loStream.GetMemoryBlock() == &loBlock );
    }

    void testReadVS(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        char lacBuffer[1024] = {0};
        
        apl_memcpy(loBlock.GetWritePtr(), "1234567890", 10 );
        loBlock.SetWritePtr(10);
        
        //case 1
        ASSERT_MESSAGE(loStream.Read(lacBuffer, 1) == 1);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "1") == 0);
        
        //case 2
        ASSERT_MESSAGE(loStream.Read(lacBuffer, 4) == 4);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "2345") == 0);
        
        //case 3
        ASSERT_MESSAGE(loStream.Read(lacBuffer, 6) == 5);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "67890") == 0);
        
        //case 4
        ASSERT_MESSAGE(loStream.Read(lacBuffer, 1) == -1);
    }

    void testReadIS1(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        char lacBuffer[1024] = {0};
        apl_iovec_t loVec[3];
        loVec[0].iov_base = lacBuffer;
        loVec[0].iov_len = 1;
        loVec[1].iov_base = lacBuffer + 2;
        loVec[1].iov_len = 4;
        loVec[2].iov_base = lacBuffer + 7;
        loVec[2].iov_len = 6;
        
        apl_memcpy(loBlock.GetWritePtr(), "1234567890", 10 );
        loBlock.SetWritePtr(10);
        
        //case 1
        ASSERT_MESSAGE(loStream.ReadV(loVec, 3) == 10);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "1") == 0);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer + 2, "2345") == 0);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer + 7, "67890") == 0);
        
        //case 4
        ASSERT_MESSAGE(loStream.Read(lacBuffer, 1) == -1);
    }
    
    void testReadIS2(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        char lacBuffer[1024] = {0};
        apl_iovec_t loVec[3];
        loVec[0].iov_base = lacBuffer;
        loVec[0].iov_len = 1;
        loVec[1].iov_base = lacBuffer + 2;
        loVec[1].iov_len = 4;
        loVec[2].iov_base = lacBuffer + 7;
        loVec[2].iov_len = 5;
        
        apl_memcpy(loBlock.GetWritePtr(), "1234567890", 10 );
        loBlock.SetWritePtr(10);
        
        //case 1
        ASSERT_MESSAGE(loStream.ReadV(loVec, 3) == 10);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "1") == 0);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer + 2, "2345") == 0);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer + 7, "67890") == 0);
        
        //case 4
        ASSERT_MESSAGE(loStream.Read(lacBuffer, 1) == -1);
    }
    
    void testReadIS3(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        char lacBuffer[1024] = {0};
        apl_iovec_t loVec[3];
        loVec[0].iov_base = lacBuffer;
        loVec[0].iov_len = 1;
        loVec[1].iov_base = lacBuffer + 2;
        loVec[1].iov_len = 4;
        loVec[2].iov_base = lacBuffer + 7;
        loVec[2].iov_len = 4;
        
        apl_memcpy(loBlock.GetWritePtr(), "1234567890", 10 );
        loBlock.SetWritePtr(10);
        
        //case 1
        ASSERT_MESSAGE(loStream.ReadV(loVec, 3) == 9);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "1") == 0);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer + 2, "2345") == 0);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer + 7, "6789") == 0);
        
        //case 4
        ASSERT_MESSAGE(loStream.Read(lacBuffer, 1) == 1);
    }
    
    void testReadLine1(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        char lacBuffer[1024] = {0};
        
        apl_memcpy(loBlock.GetWritePtr(), "123\n4567\r\n890", 13 );
        loBlock.SetWritePtr(13);

        //case 1
        apl_memset(lacBuffer, 0, sizeof(lacBuffer) );
        ASSERT_MESSAGE(loStream.ReadLine(lacBuffer, 10) == 3);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "123") == 0);
        
        //case 2
        apl_memset(lacBuffer, 0, sizeof(lacBuffer) );
        ASSERT_MESSAGE(loStream.ReadLine(lacBuffer, 10) == 4);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "4567") == 0);
        
        //case 4
        loBlock.Reset();
        loBlock.SetWritePtr(13);
        ASSERT_MESSAGE(loStream.ReadLine(lacBuffer, 3) == 2);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "12") == 0);
        
        //case 5
        ASSERT_MESSAGE(loStream.ReadLine(lacBuffer, 2) == 1);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "3") == 0);
    }
    
    void testReadLine2(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        std::string loLine;
        
        apl_memcpy(loBlock.GetWritePtr(), "123\n4567\r\n890\n", 14 );
        loBlock.SetWritePtr(14);

        //case 1
        ASSERT_MESSAGE(loStream.ReadLine(loLine) == 3);
        ASSERT_MESSAGE(loLine == "123");
        
        //case 2
        ASSERT_MESSAGE(loStream.ReadLine(loLine) == 4);
        ASSERT_MESSAGE(loLine == "4567");
        
        //case 3
        ASSERT_MESSAGE(loStream.ReadLine(loLine) == 3);
        ASSERT_MESSAGE(loLine == "890");
    }
    
    void testReadString1(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        char lacBuffer[1024] = {0};
        
        apl_memcpy(loBlock.GetWritePtr(), "12345", 6 );
        apl_memcpy(loBlock.GetWritePtr() + 6, "67890", 6 );
        loBlock.SetWritePtr(12);

        //case 1
        ASSERT_MESSAGE(loStream.ReadString(lacBuffer, 10) == 5);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "12345") == 0);
        
        //case 2
        ASSERT_MESSAGE(loStream.ReadString(lacBuffer, 4) == -1);
        ASSERT_MESSAGE(loStream.ReadString(lacBuffer, 5) == -1);
        ASSERT_MESSAGE(loStream.ReadString(lacBuffer, 6) == 5);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "67890") == 0);
    }
    
    void testReadString2(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        std::string loLine;
        
        apl_memcpy(loBlock.GetWritePtr(), "12345\00067890", 12 );
        loBlock.SetWritePtr(12);

        //case 1
        ASSERT_MESSAGE(loStream.ReadString(loLine) == 5);
        printf("[%s:%u]\n", loLine.c_str(), loLine.size() );
        ASSERT_MESSAGE(loLine == "12345");
        
        //case 2
        ASSERT_MESSAGE(loStream.ReadString(loLine) == 5);
        ASSERT_MESSAGE(loLine == "67890");
    }
    
    void testReadT(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        
        apl_int16_t li16N = 1;
        apl_int32_t li32N = 2;
        apl_int64_t li64N = 3;
        
        apl_memcpy(loBlock.GetWritePtr(), &li16N, 2 );
        apl_memcpy(loBlock.GetWritePtr() + 2, &li32N, 4 );
        apl_memcpy(loBlock.GetWritePtr() + 6, &li64N, 8 );
        loBlock.SetWritePtr(14);
        
        apl_int16_t li16N1 = 0;
        apl_int32_t li32N1 = 0;
        apl_int64_t li64N1 = 0;
        
        ASSERT_MESSAGE( loStream.Read(li16N1) == sizeof(li16N1) );
        ASSERT_MESSAGE( li16N1 == li16N );
        ASSERT_MESSAGE( loStream.Read(li32N1) == sizeof(li32N1) );
        ASSERT_MESSAGE( li32N1 == li32N );
        ASSERT_MESSAGE( loStream.Read(li64N1) == sizeof(li64N1) );
        ASSERT_MESSAGE( li64N1 == li64N );
    }
    
    void testReadUntil(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        char lacBuffer[1024] = {0};
        
        apl_memcpy(loBlock.GetWritePtr(), "123\n4567\r\n8901234567890\n", 24 );
        loBlock.SetWritePtr(24);

        //case 1
        apl_memset(lacBuffer, 0, sizeof(lacBuffer) );
        ASSERT_MESSAGE(loStream.ReadUntil(lacBuffer, 10, "\n") == 4);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "123\n") == 0);
        
        //case 2
        apl_memset(lacBuffer, 0, sizeof(lacBuffer) );
        ASSERT_MESSAGE(loStream.ReadUntil(lacBuffer, 10, "\r\n") == 6);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "4567\r\n") == 0);
        
        //case 3
        apl_memset(lacBuffer, 0, sizeof(lacBuffer) );
        ASSERT_MESSAGE(loStream.ReadLine(lacBuffer, 20) == 13);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "8901234567890") == 0);
        
        //case 4
        loBlock.Reset();
        loBlock.SetWritePtr(24);
        apl_memset(lacBuffer, 0, sizeof(lacBuffer) );
        ASSERT_MESSAGE(loStream.ReadUntil(lacBuffer, 30, "1234567890") == 23);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "123\n4567\r\n8901234567890") == 0);
        
        //case 5
        loBlock.Reset();
        loBlock.SetWritePtr(24);
        apl_memset(lacBuffer, 0, sizeof(lacBuffer) );
        ASSERT_MESSAGE(loStream.ReadUntil(lacBuffer, 14, "1234567890") == 13);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "123\n4567\r\n890") == 0);
        
        //case 6
        loBlock.Reset();
        loBlock.SetWritePtr(23);
        apl_memset(lacBuffer, 0, sizeof(lacBuffer) );
        ASSERT_MESSAGE(loStream.ReadUntil(lacBuffer, 15, "1234567890") == 13);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "123\n4567\r\n890") == 0);
        
        //case 7
        loBlock.Reset();
        loBlock.SetWritePtr(23);
        apl_memset(lacBuffer, 0, sizeof(lacBuffer) );
        ASSERT_MESSAGE(loStream.ReadUntil(lacBuffer, 30, "xx") == -1);
    }
    
    void testReadTo(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        
        apl_int16_t li16N = apl_hton16(101);
        apl_int32_t li32N = apl_hton32(202);
        apl_int64_t li64N = apl_hton64(303);
        
        apl_memcpy(loBlock.GetWritePtr(), &li16N, 2 );
        apl_memcpy(loBlock.GetWritePtr() + 2, &li32N, 4 );
        apl_memcpy(loBlock.GetWritePtr() + 6, &li64N, 8 );
        loBlock.SetWritePtr(14);
        
        apl_int16_t li16N1 = 0;
        apl_int32_t li32N1 = 0;
        apl_int64_t li64N1 = 0;
        
        ASSERT_MESSAGE( loStream.ReadToH(li16N1) == sizeof(li16N1) );
        ASSERT_MESSAGE( loStream.ReadToH(li32N1) == sizeof(li32N1) );
        ASSERT_MESSAGE( loStream.ReadToH(li64N1) == sizeof(li64N1) );
        ASSERT_MESSAGE( li16N1 == 101 );
        ASSERT_MESSAGE( li32N1 == 202 );
        ASSERT_MESSAGE( li64N1 == 303 );
    }
    
    void testReadToU(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        
        apl_uint16_t li16N = apl_hton16(101);
        apl_uint32_t li32N = apl_hton32(202);
        apl_uint64_t li64N = apl_hton64(303);
        
        apl_memcpy(loBlock.GetWritePtr(), &li16N, 2 );
        apl_memcpy(loBlock.GetWritePtr() + 2, &li32N, 4 );
        apl_memcpy(loBlock.GetWritePtr() + 6, &li64N, 8 );
        loBlock.SetWritePtr(14);
        
        apl_uint16_t li16N1 = 0;
        apl_uint32_t li32N1 = 0;
        apl_uint64_t li64N1 = 0;
        
        ASSERT_MESSAGE( loStream.ReadToH(li16N1) == sizeof(li16N1) );
        ASSERT_MESSAGE( loStream.ReadToH(li32N1) == sizeof(li32N1) );
        ASSERT_MESSAGE( loStream.ReadToH(li64N1) == sizeof(li64N1) );
        ASSERT_MESSAGE( li16N1 == 101 );
        ASSERT_MESSAGE( li32N1 == 202 );
        ASSERT_MESSAGE( li64N1 == 303 );
    }
    
    void testWrite(void)
    {
        CMemoryBlock  loBlock(11);
        CMemoryStream loStream(&loBlock);

        apl_memset(loBlock.GetBase(), 0, loBlock.GetSpace() );
        loBlock.Resize(10);

        //case 1
        ASSERT_MESSAGE(loStream.Write("0123456789", 10) == 10);
        ASSERT_MESSAGE(apl_strcmp(loBlock.GetReadPtr(), "0123456789") == 0);
        
        //case 2
        loBlock.Reset();
        ASSERT_MESSAGE(loStream.Write("0123456789x", 11) == 10);
        ASSERT_MESSAGE(apl_strcmp(loBlock.GetReadPtr(), "0123456789") == 0);
        
        //case 3
        loBlock.Reset();
        ASSERT_MESSAGE(loStream.Write("0123") == 5);
        ASSERT_MESSAGE(apl_strcmp(loBlock.GetReadPtr(), "0123") == 0);
    }
    
    void testWriteV1(void)
    {
        CMemoryBlock  loBlock(11);
        CMemoryStream loStream(&loBlock);
        char lacBuffer[128] = "0123456789";
        
        apl_iovec_t loVec[3];
        loVec[0].iov_base = lacBuffer;
        loVec[0].iov_len = 1;
        loVec[1].iov_base = lacBuffer + 1;
        loVec[1].iov_len = 3;
        loVec[2].iov_base = lacBuffer + 4;
        loVec[2].iov_len = 6;

        apl_memset(loBlock.GetBase(), 0, loBlock.GetSpace() );
        loBlock.Resize(10);

        //case 1
        ASSERT_MESSAGE(loStream.WriteV(loVec, 3) == 10);
        ASSERT_MESSAGE(apl_strcmp(loBlock.GetReadPtr(), "0123456789") == 0);
    }
    
    void testWriteV2(void)
    {
        CMemoryBlock  loBlock(11);
        CMemoryStream loStream(&loBlock);
        char lacBuffer[128] = "0123456789";
        
        apl_iovec_t loVec[3];
        loVec[0].iov_base = lacBuffer;
        loVec[0].iov_len = 1;
        loVec[1].iov_base = lacBuffer + 1;
        loVec[1].iov_len = 3;
        loVec[2].iov_base = lacBuffer + 4;
        loVec[2].iov_len = 7;

        apl_memset(loBlock.GetBase(), 0, loBlock.GetSpace() );
        loBlock.Resize(10);

        //case 1
        ASSERT_MESSAGE(loStream.WriteV(loVec, 3) == 10);
        ASSERT_MESSAGE(apl_strcmp(loBlock.GetReadPtr(), "0123456789") == 0);
    }
    
    void testWriteV3(void)
    {
        CMemoryBlock  loBlock(11);
        CMemoryStream loStream(&loBlock);
        char lacBuffer[128] = "0123456789";
        
        apl_iovec_t loVec[3];
        loVec[0].iov_base = lacBuffer;
        loVec[0].iov_len = 1;
        loVec[1].iov_base = lacBuffer + 1;
        loVec[1].iov_len = 3;
        loVec[2].iov_base = lacBuffer + 4;
        loVec[2].iov_len = 5;

        apl_memset(loBlock.GetBase(), 0, loBlock.GetSpace() );
        loBlock.Resize(10);

        //case 1
        ASSERT_MESSAGE(loStream.WriteV(loVec, 3) == 9);
        ASSERT_MESSAGE(apl_strcmp(loBlock.GetReadPtr(), "012345678") == 0);
    }
    
    void testWriteT(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        
        apl_int16_t li16N = 1;
        apl_int32_t li32N = 2;
        apl_int64_t li64N = 3;
        
        ASSERT_MESSAGE( loStream.Write(li16N) == sizeof(li16N) );
        ASSERT_MESSAGE( loStream.Write(li32N) == sizeof(li32N) );
        ASSERT_MESSAGE( loStream.Write(li64N) == sizeof(li64N) );
        
        apl_int16_t li16N1 = 0;
        apl_int32_t li32N1 = 0;
        apl_int64_t li64N1 = 0;
        
        apl_memcpy(&li16N1, loBlock.GetReadPtr(), 2 );
        apl_memcpy(&li32N1, loBlock.GetReadPtr() + 2, 4 );
        apl_memcpy(&li64N1, loBlock.GetReadPtr() + 6, 8 );

        ASSERT_MESSAGE( li16N1 == li16N );
        ASSERT_MESSAGE( li32N1 == li32N );
        ASSERT_MESSAGE( li64N1 == li64N );
    }
    
    void testWriteTo(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        
        apl_int16_t li16N = 1;
        apl_int32_t li32N = 2;
        apl_int64_t li64N = 3;
        
        ASSERT_MESSAGE( loStream.WriteToN(li16N) == sizeof(li16N) );
        ASSERT_MESSAGE( loStream.WriteToN(li32N) == sizeof(li32N) );
        ASSERT_MESSAGE( loStream.WriteToN(li64N) == sizeof(li64N) );
        
        apl_int16_t li16N1 = 0;
        apl_int32_t li32N1 = 0;
        apl_int64_t li64N1 = 0;
        
        apl_memcpy(&li16N1, loBlock.GetReadPtr(), 2 );
        apl_memcpy(&li32N1, loBlock.GetReadPtr() + 2, 4 );
        apl_memcpy(&li64N1, loBlock.GetReadPtr() + 6, 8 );
        
        li16N1 = apl_ntoh16(li16N1);
        li32N1 = apl_ntoh32(li32N1);
        li64N1 = apl_ntoh64(li64N1);
        
        ASSERT_MESSAGE( li16N1 == li16N );
        ASSERT_MESSAGE( li32N1 == li32N );
        ASSERT_MESSAGE( li64N1 == li64N );
    }
    
    void testWriteToU(void)
    {
        CMemoryBlock  loBlock(1024);
        CMemoryStream loStream(&loBlock);
        
        apl_uint16_t li16N = 1;
        apl_uint32_t li32N = 2;
        apl_uint64_t li64N = 3;
        
        ASSERT_MESSAGE( loStream.WriteToN(li16N) == sizeof(li16N) );
        ASSERT_MESSAGE( loStream.WriteToN(li32N) == sizeof(li32N) );
        ASSERT_MESSAGE( loStream.WriteToN(li64N) == sizeof(li64N) );
        
        apl_uint16_t li16N1 = 0;
        apl_uint32_t li32N1 = 0;
        apl_uint64_t li64N1 = 0;
        
        apl_memcpy(&li16N1, loBlock.GetReadPtr(), 2 );
        apl_memcpy(&li32N1, loBlock.GetReadPtr() + 2, 4 );
        apl_memcpy(&li64N1, loBlock.GetReadPtr() + 6, 8 );
        
        li16N1 = apl_ntoh16(li16N1);
        li32N1 = apl_ntoh32(li32N1);
        li64N1 = apl_ntoh64(li64N1);
        
        ASSERT_MESSAGE( li16N1 == li16N );
        ASSERT_MESSAGE( li32N1 == li32N );
        ASSERT_MESSAGE( li64N1 == li64N );
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclMemoryBlock);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
