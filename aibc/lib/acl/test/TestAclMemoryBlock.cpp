#include "Test.h"
#include "acl/MemoryBlock.h"

using namespace acl;

class CTestAclMemoryBlock:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclMemoryBlock);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testGetSize);
    CPPUNIT_TEST(testResize);
    CPPUNIT_TEST(testLength);
    CPPUNIT_TEST(testGetCapacity);
    CPPUNIT_TEST(testGetSpace);
    CPPUNIT_TEST(testGetTotalLength);
    CPPUNIT_TEST(testGetTotalSize);
    CPPUNIT_TEST(testGetTotalCapacity);
    CPPUNIT_TEST(testRead);
    CPPUNIT_TEST(testWriteStr);
    CPPUNIT_TEST(testWriteBuf);
    CPPUNIT_TEST(testSetReadPtrPos);
    CPPUNIT_TEST(testSetReadPtrInt);
    CPPUNIT_TEST(testSetWritePtrPos);
    CPPUNIT_TEST(testSetWritePtrInt);
    CPPUNIT_TEST(testGetWritePtr);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST(testGetBase);
    CPPUNIT_TEST(testGetEnd);
    CPPUNIT_TEST(testSetBase);
    CPPUNIT_TEST(testClone);
    CPPUNIT_TEST(testDuplicate);
    CPPUNIT_TEST(testGetReferCount);
    CPPUNIT_TEST(testCont);
    CPPUNIT_TEST(testDataBlock);
    CPPUNIT_TEST(testSwapDataBlock);
    CPPUNIT_TEST(testRelease);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testConstructor(void)
    {
        PRINT_TITLE_2(__func__);
        
        char lsBuf[50];
        memset(lsBuf, 0, sizeof(lsBuf));

        //CMemoryBlock(size, Cont==NULL)
        CMemoryBlock loMemoryBlock1st(100);
        loMemoryBlock1st.Write("1234567890");
        loMemoryBlock1st.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("Constructor loMemoryBlock1st failed.", 
                apl_strcmp(lsBuf, "1234567890") == 0);

        //CMemoryBlock(size, Cont!=NULL)
        CMemoryBlock loMemoryBlock1stCont(10, &loMemoryBlock1st);
        loMemoryBlock1stCont.Write("12345");
        memset(lsBuf, 0, sizeof(lsBuf));
        loMemoryBlock1stCont.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("Constructor loMemoryBlock1stCont failed.",
                apl_strcmp(lsBuf, "12345") == 0);

        CMemoryBlock* lpoMemoryBlock1st = loMemoryBlock1stCont.GetCont();
        lpoMemoryBlock1st->SetReadPtr(lpoMemoryBlock1st->GetBase());
        memset(lsBuf, 0, sizeof(lsBuf));
        lpoMemoryBlock1st->Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("Constructor loMemoryBlock1stCont failed.",
                apl_strcmp(lsBuf, "1234567890") == 0);

        //copy constructor
        loMemoryBlock1st.SetReadPtr(loMemoryBlock1st.GetBase());
        CMemoryBlock loMemoryBlock2nd(loMemoryBlock1st);
        memset(lsBuf, 0, sizeof(lsBuf)); 
        loMemoryBlock2nd.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("copy constructor failed.", apl_strcmp(lsBuf, "1234567890") == 0); 
        loMemoryBlock1stCont.SetCont(NULL);

        //CMemoryBlock(char*,size,DONT_DELETE,Cont==NULL)
        char const* lpcIn = "abcdefg";
        CMemoryBlock loMemoryBlock3rd(lpcIn, strlen(lpcIn));
        memset(lsBuf, 0, sizeof(lsBuf)); 
        loMemoryBlock3rd.SetWritePtr(loMemoryBlock3rd.GetEnd());
        loMemoryBlock3rd.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("Constructor loMemoryBlock3rd failed.",
                apl_strcmp(lsBuf, lpcIn) == 0);

        //CMemoryBlock(char*,size,DO_DELETE,Cont!=NULL)
        char* lpcTmp = NULL;
        ACL_NEW_N_ASSERT( lpcTmp, char, 20 );
        apl_strncpy(lpcTmp, lpcIn, 20);
        CMemoryBlock loMemoryBlock3rdCont(lpcTmp, strlen(lpcTmp), CMemoryBlock::DO_DELETE, &loMemoryBlock3rd);
        memset(lsBuf, 0, sizeof(lsBuf));
        loMemoryBlock3rdCont.SetWritePtr(loMemoryBlock3rdCont.GetEnd());
        loMemoryBlock3rdCont.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("Constructor loMemoryBlock3rdCont failed.",
                apl_strcmp(lsBuf, lpcTmp) == 0);
        loMemoryBlock3rdCont.SetCont(NULL);

        //CMemoryBlock(CDataBlock,Cont==NULL)
        CDataBlock* loDataBlock = new CDataBlock(20); 
        CMemoryBlock loMemoryBlock4th(loDataBlock);
        loMemoryBlock4th.Write(lpcIn);
        memset(lsBuf, 0, sizeof(lsBuf));
        loMemoryBlock4th.Read(lsBuf, sizeof(lsBuf));
    }

    void testGetSize(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CMemoryBlock loMemoryBlock(1024);

        //case
        apl_size_t liSize = loMemoryBlock.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetSize failed.", (apl_size_t)1024, liSize);

        //end environment
    }

    void testResize(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CMemoryBlock loMemoryBlock(1024);

        apl_size_t liSize = loMemoryBlock.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetSize failed.", (apl_size_t)1024, liSize);

        //case
        loMemoryBlock.Resize(512);
        liSize = loMemoryBlock.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetSize failed.", (apl_size_t)512, liSize);

        //end environment
    }

    void testLength(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CMemoryBlock loMemoryBlock(1024);
        loMemoryBlock.Write("1234567890abcdefghijklmno");

        apl_size_t liLen = 0;
        //case
        liLen = loMemoryBlock.GetLength();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetLength failed.", (apl_size_t)26, liLen);
        loMemoryBlock.SetLength(20);
        liLen = loMemoryBlock.GetLength();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetLength failed.", (apl_size_t)20, liLen);

        //end environment
    }

    void testGetCapacity(void)
    {
        PRINT_TITLE_2(__func__);         
        //start case environment
        CMemoryBlock loMemoryBlock(1024);

        //case
        apl_size_t liSize = loMemoryBlock.GetCapacity();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetCapacity failed.", (apl_size_t)1024, liSize);

        //end environment
    }

    void testGetSpace(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CMemoryBlock loMemoryBlock(1024);

        //case
        loMemoryBlock.Write("1234567890abcdefghijklmno");
        apl_size_t liSize = loMemoryBlock.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetSpace failed.", (apl_size_t)998, liSize);

        loMemoryBlock.Resize(256);
        liSize = loMemoryBlock.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetSpace failed.", (apl_size_t)230, liSize);

        //end environment
    }

    void testGetTotalLength(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CMemoryBlock loMemoryBlock(1024);

        loMemoryBlock.Write("1234567890abcdefghijklmno");

        //case
        apl_size_t liSize = loMemoryBlock.GetTotalLength();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetTotalLength failed.", (apl_size_t)26, liSize);

        CMemoryBlock loMemoryBlockCont(128, &loMemoryBlock);
        loMemoryBlockCont.Write("12345",3);
        liSize = loMemoryBlockCont.GetTotalLength();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetTotalLength failed.", (apl_size_t)29, liSize); 
        loMemoryBlockCont.SetCont(NULL);
        //end environment
    }

    void testGetTotalSize(void)
    {
        PRINT_TITLE_2(__func__);         
        //start case environment
        CMemoryBlock loMemoryBlock(1024);
        loMemoryBlock.Resize(200);

        //case
        apl_size_t liSize = loMemoryBlock.GetTotalSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetTotalSize failed.", (apl_size_t)200, liSize);

        CMemoryBlock loMemoryBlockCont(128, &loMemoryBlock);
        loMemoryBlockCont.Resize(20);
        liSize = loMemoryBlockCont.GetTotalSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetTotalSize failed.", (apl_size_t)220, liSize);
        loMemoryBlockCont.SetCont(NULL);
        //end environment
    }

    void testGetTotalCapacity(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CMemoryBlock loMemoryBlock(1024);

        //case
        apl_size_t liSize = loMemoryBlock.GetTotalCapacity();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetTotalCapacity failed.", (apl_size_t)1024, liSize);

        CMemoryBlock loMemoryBlockCont(128, &loMemoryBlock); 
        liSize = loMemoryBlockCont.GetTotalCapacity();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetTotalCapacity failed.", (apl_size_t)1152, liSize);
        loMemoryBlockCont.SetCont(NULL);
        
        //end environment
    }

    void testRead(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CMemoryBlock loMemoryBlock(128);
        char lsBuf[50];
        memset(lsBuf, 0, sizeof(lsBuf));

        //case
        apl_ssize_t liReadSize = loMemoryBlock.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Read failed(Should be -1).", (apl_ssize_t)-1, liReadSize);

        loMemoryBlock.Write("1234567890");
        liReadSize = loMemoryBlock.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Read failed.", (apl_ssize_t)11, liReadSize);
        CPPUNIT_ASSERT_MESSAGE("Read failed.", apl_strcmp(lsBuf, "1234567890") == 0);

        memset(lsBuf, 0, sizeof(lsBuf));
        loMemoryBlock.SetReadPtr(loMemoryBlock.GetBase());
        liReadSize = loMemoryBlock.Read(lsBuf,5);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Read failed.", (apl_ssize_t)5, liReadSize);   
        CPPUNIT_ASSERT_MESSAGE("Read failed.", apl_strcmp(lsBuf, "12345") == 0);

        //end environment
    }

    void testWriteStr(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        char lsBuf[50];                 
        memset(lsBuf, 0, sizeof(lsBuf));

        //case
        CMemoryBlock loMemoryBlock(2);
        apl_ssize_t liSize = loMemoryBlock.Write("1234567890");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Write failed.(Should be -1)",(apl_ssize_t)-1, liSize);

        CMemoryBlock loMemoryBlock2(20);
        liSize = loMemoryBlock2.Write("1234567890");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Write failed.", (apl_ssize_t)11, liSize);

        apl_ssize_t liReadSize = loMemoryBlock2.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Read failed.", (apl_ssize_t)11, liReadSize);   
        CPPUNIT_ASSERT_MESSAGE("Read failed.", apl_strcmp(lsBuf, "1234567890") == 0);

        //end environment
    }

    void testWriteBuf(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        const char *lpcIn = "1234567890";
        char lsBuf[50];
        memset(lsBuf, 0, sizeof(lsBuf)); 
        CMemoryBlock loMemoryBlock(20);
        //case
        apl_ssize_t liSize = loMemoryBlock.Write(lpcIn, 5);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Write failed.", (apl_ssize_t)5, liSize);

        apl_ssize_t liReadSize = loMemoryBlock.Read(lsBuf, sizeof(lsBuf));          
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Read failed.", (apl_ssize_t)5, liReadSize);   
        CPPUNIT_ASSERT_MESSAGE("Read failed.", apl_strcmp(lsBuf, "12345") == 0);

        //end environment
    }

    void testSetReadPtrPos(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char lsBuf[50];
        memset(lsBuf, 0, sizeof(lsBuf));
        CMemoryBlock loMemoryBlock(20);
        loMemoryBlock.Write("1234567890");

        //case
        loMemoryBlock.SetReadPtr(loMemoryBlock.GetBase()+3);
        apl_ssize_t liReadSize = loMemoryBlock.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetReadPtr failed.", (apl_ssize_t)8, liReadSize);
        CPPUNIT_ASSERT_MESSAGE("SetReadPtr failed.", apl_strcmp(lsBuf, "4567890") == 0);

        //end environment
    }

    void testSetReadPtrInt(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        char lsBuf[50];
        memset(lsBuf, 0, sizeof(lsBuf));
        CMemoryBlock loMemoryBlock(20);
        loMemoryBlock.Write("1234567890");

        //case
        loMemoryBlock.SetReadPtr(loMemoryBlock.GetBase());
        loMemoryBlock.SetReadPtr(3);
        apl_ssize_t liReadSize = loMemoryBlock.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetReadPtr failed.", (apl_ssize_t)8, liReadSize);
        CPPUNIT_ASSERT_MESSAGE("SetReadPtr failed.", apl_strcmp(lsBuf, "4567890") == 0);

        //end environment
    }

    void testSetWritePtrPos(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char lsBuf[50];                   
        memset(lsBuf, 0, sizeof(lsBuf));  
        CMemoryBlock loMemoryBlock(20);   
        loMemoryBlock.Write("1234567890");

        //case
        loMemoryBlock.SetWritePtr(loMemoryBlock.GetBase()+3);
        loMemoryBlock.Write("xyz");
        apl_ssize_t liReadSize = loMemoryBlock.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetWritePtr failed.", (apl_ssize_t)7, liReadSize);
        CPPUNIT_ASSERT_MESSAGE("SetWritePtr failed.", apl_strcmp(lsBuf, "123xyz") == 0);

        //end environment
    }

    void testSetWritePtrInt(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        char lsBuf[50];                   
        memset(lsBuf, 0, sizeof(lsBuf));  
        CMemoryBlock loMemoryBlock(20);   

        //case
        loMemoryBlock.SetWritePtr(5);
        loMemoryBlock.Write("1234567890");

        loMemoryBlock.SetReadPtr(5);
        apl_ssize_t liReadSize = loMemoryBlock.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetWritePtr failed.", (apl_ssize_t)11, liReadSize);
        CPPUNIT_ASSERT_MESSAGE("SetWritePtr failed.", apl_strcmp(lsBuf, "1234567890") == 0);

        //end environment
    }

    void testGetWritePtr(void)
    {
        PRINT_TITLE_2(__func__);        
        //satrt case environment
        char lsBuf[50];                 
        memset(lsBuf, 0, sizeof(lsBuf));
        CMemoryBlock loMemoryBlock(20); 
        loMemoryBlock.Write("1234567890");

        //case
        loMemoryBlock.SetReadPtr(loMemoryBlock.GetWritePtr());
        loMemoryBlock.Write("xyz");

        apl_ssize_t liReadSize = loMemoryBlock.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetWritePtr failed.", (apl_ssize_t)4, liReadSize);   
        CPPUNIT_ASSERT_MESSAGE("GetWritePtr failed.", apl_strcmp(lsBuf, "xyz") == 0);

        //end environment
    }

    void testReset(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char lsBuf[50];                   
        memset(lsBuf, 0, sizeof(lsBuf));  
        CMemoryBlock loMemoryBlock(20);   
        loMemoryBlock.Write("1234567890");

        //case
        loMemoryBlock.Reset();
        loMemoryBlock.Write("xyz");
        apl_ssize_t liReadSize = loMemoryBlock.Read(lsBuf, sizeof(lsBuf));              
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Reset failed.", (apl_ssize_t)4, liReadSize);
        CPPUNIT_ASSERT_MESSAGE("Reset failed.", apl_strcmp(lsBuf, "xyz") == 0);   

        //end environment
    }

    void testGetBase(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char lsBuf[50];                   
        memset(lsBuf, 0, sizeof(lsBuf));  
        CMemoryBlock loMemoryBlock(20);   
        loMemoryBlock.Write("1234567890");

        //case
        apl_ssize_t liReadSize = loMemoryBlock.Read(lsBuf, sizeof(lsBuf));
        loMemoryBlock.SetReadPtr(loMemoryBlock.GetBase());
        liReadSize = loMemoryBlock.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetBase failed.", (apl_ssize_t)11, liReadSize);
        CPPUNIT_ASSERT_MESSAGE("GetBase failed.", apl_strcmp(lsBuf, "1234567890") == 0);   

        //end environment
    }

    void testGetEnd(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        char lsBuf[50];    
        memset(lsBuf, 0, sizeof(lsBuf));
        CMemoryBlock loMemoryBlock(20);
          
        //case
        loMemoryBlock.Write("1234567890");
        loMemoryBlock.Reset();
        loMemoryBlock.SetWritePtr(loMemoryBlock.GetEnd()-10);
        loMemoryBlock.Write("abcd");
        apl_ssize_t liReadSize = loMemoryBlock.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetEnd failed.", (apl_ssize_t)15, liReadSize);
        CPPUNIT_ASSERT_MESSAGE("GetEnd failed.", apl_strcmp(lsBuf, "1234567890abcd") == 0);

        //end environment
    }

    void testSetBase(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CMemoryBlock loMemoryBlock(64);
        char const* lpcIn = "1234567890";

        //case
        loMemoryBlock.SetBase(lpcIn, 5, CMemoryBlock::DONT_DELETE);
        loMemoryBlock.SetReadPtr(loMemoryBlock.GetBase());
        loMemoryBlock.SetWritePtr(loMemoryBlock.GetEnd());
        char lsBuf[50];
        memset(lsBuf, 0, sizeof(lsBuf));
        loMemoryBlock.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("SetBase failed.", apl_strcmp(lsBuf, "12345") == 0);
        
        //case capacity
        CPPUNIT_ASSERT_MESSAGE("SetBase failed.", loMemoryBlock.GetCapacity() == loMemoryBlock.GetSize() );

        //end environment
    }

    void testClone(void)
    {
        PRINT_TITLE_2(__func__); 

        //start case environment
        char const* lsIn = "1234567890";
        char lsBuf[50];
        memset(lsBuf, 0, sizeof(lsBuf));

        //--- no cont
        CMemoryBlock loMemBlock1st(64);
        loMemBlock1st.Write(lsIn,10);

        CMemoryBlock *lpoMemBlock2nd = loMemBlock1st.Clone();
        apl_int_t liCnt = loMemBlock1st.GetReferCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Clone & GetReferCount failed.", 
                (apl_int_t)1, liCnt);
        liCnt = lpoMemBlock2nd->GetReferCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Clone & GetReferCount failed.",
                (apl_int_t)1, liCnt);

        lpoMemBlock2nd->Reset();
        lpoMemBlock2nd->Write("abcdefg");
        apl_ssize_t liReadSize = lpoMemBlock2nd->Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("Clone failed.", apl_strcmp(lsBuf, "abcdefg") == 0);

        memset(lsBuf, 0, sizeof(lsBuf)); 
        liReadSize = loMemBlock1st.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("Clone failed.", apl_strcmp(lsBuf, lsIn) == 0);

        //--- has cont
        loMemBlock1st.SetReadPtr(loMemBlock1st.GetBase());
        CMemoryBlock loMemBlock3rd(64, &loMemBlock1st);
        loMemBlock3rd.Write("abcde", 5);

        CMemoryBlock *lpoMemBlock4th = loMemBlock3rd.Clone();
        liCnt = loMemBlock3rd.GetReferCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Clone & GetReferCount failed.",
                (apl_int_t)1, liCnt);
        liCnt = lpoMemBlock4th->GetReferCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Clone & GetReferCount failed.",
                (apl_int_t)1, liCnt);

        // original one
        CMemoryBlock *lpoMemBlockCont = loMemBlock3rd.GetCont();
        lpoMemBlockCont->SetWritePtr(lpoMemBlockCont->GetBase());
        lpoMemBlockCont->Write("0987654321");
        memset(lsBuf, 0, sizeof(lsBuf));
        lpoMemBlockCont->Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("Cont Clone failed.", 
                apl_strcmp(lsBuf, "0987654321") == 0);

        loMemBlock3rd.Reset();
        loMemBlock3rd.Write("edcba");
        memset(lsBuf, 0, sizeof(lsBuf));
        loMemBlock3rd.Read(lsBuf, sizeof(lsBuf)); 
        CPPUNIT_ASSERT_MESSAGE("Cont Clone failed.", 
                apl_strcmp(lsBuf, "edcba") == 0);

        //cont one
        CMemoryBlock *lpoMemBlockContClone = lpoMemBlock4th->GetCont();
        memset(lsBuf, 0, sizeof(lsBuf));
        lpoMemBlockContClone->Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("Cont Clone failed.", 
                apl_strcmp(lsBuf, "1234567890") == 0);
        memset(lsBuf, 0, sizeof(lsBuf));
        lpoMemBlock4th->Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("Cont Clone failed.",
                apl_strcmp(lsBuf, "abcde") == 0);

        loMemBlock3rd.SetCont(NULL);
        lpoMemBlock4th->SetCont(NULL);
        //end environment
    }

    void testDuplicate(void)
    {
        PRINT_TITLE_2(__func__); 

        //start case environment
        char lsBuf[50];
        memset(lsBuf, 0, sizeof(lsBuf));

        CMemoryBlock loMemBlock1st(64);
        loMemBlock1st.Write("1234567890", 10);

        //case
        CMemoryBlock *lpoMemBlock2nd = loMemBlock1st.Duplicate();
        CMemoryBlock *lpoMemBlock3rd = loMemBlock1st.Duplicate();
        apl_int_t liCnt = loMemBlock1st.GetReferCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Duplicate failed.", (apl_int_t)3, liCnt);
        liCnt = lpoMemBlock2nd->GetReferCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Duplicate failed.", (apl_int_t)3, liCnt);
        liCnt = lpoMemBlock3rd->GetReferCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Duplicate failed.", (apl_int_t)3, liCnt);

        lpoMemBlock2nd->SetWritePtr(lpoMemBlock2nd->GetBase() + 3);
        lpoMemBlock2nd->Write("abcd",4);

        lpoMemBlock2nd->Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("lpoMemBlock2nd - Duplicate failed.", 
                apl_strcmp(lsBuf, "123abcd") == 0);

        loMemBlock1st.SetReadPtr(loMemBlock1st.GetBase());
        memset(lsBuf, 0, sizeof(lsBuf));
        loMemBlock1st.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("loMemBlock1st - Duplicate failed.", 
                apl_strcmp(lsBuf, "123abcd890") == 0);

        lpoMemBlock3rd->SetReadPtr(lpoMemBlock3rd->GetBase());
        memset(lsBuf, 0, sizeof(lsBuf));
        lpoMemBlock3rd->Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("lpoMemBlock3rd - Duplicate failed.", 
                apl_strcmp(lsBuf, "123abcd890") == 0);

        //end environment
    }

    void testGetReferCount(void)
    {
        PRINT_TITLE_2(__func__);        
        //start case environment
        CMemoryBlock loMemBlock1st(20); 
        loMemBlock1st.Write("1234567890");
        CMemoryBlock *lpoMemBlock2nd = loMemBlock1st.Duplicate();
        CMemoryBlock *lpoMemBlock3rd = loMemBlock1st.Duplicate();

        //case
        apl_int_t liCnt = loMemBlock1st.GetReferCount();                       
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Duplicate failed.", (apl_int_t)3, liCnt);
        liCnt = lpoMemBlock2nd->GetReferCount();                               
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Duplicate failed.", (apl_int_t)3, liCnt);
        liCnt = lpoMemBlock3rd->GetReferCount();                               
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Duplicate failed.", (apl_int_t)3, liCnt);

        //end environment
    }

    void testCont(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        char lsBuf[50];
        memset(lsBuf, 0, sizeof(lsBuf));
        CMemoryBlock loMBCont(20);
        loMBCont.Write("abcd");

        CMemoryBlock loMemoryBlock(20);
        loMemoryBlock.Write("12345");

        //case
        loMemoryBlock.SetCont(&loMBCont);
        CMemoryBlock *lpcMemoryBlock = loMemoryBlock.GetCont();
        apl_ssize_t liSize = lpcMemoryBlock->Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetCont and GetCont failed.",
                (apl_ssize_t)5, liSize);
        CPPUNIT_ASSERT_MESSAGE("SetCont and GetCont failed.",apl_strcmp(lsBuf, "abcd")==0);
        loMemoryBlock.SetCont(NULL);
        //end environment
    }

    void testDataBlock(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char lsBuf[50];
        memset(lsBuf, 0, sizeof(lsBuf));
        CDataBlock* loDataBlock = new CDataBlock("12345", 5, CMemoryBlock::DONT_DELETE);
        CMemoryBlock loMemoryBlock(50);

        //case
        loMemoryBlock.SetDataBlock(loDataBlock);

        loMemoryBlock.SetWritePtr(loMemoryBlock.GetEnd());
        apl_ssize_t liLen = loMemoryBlock.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetDataBlock failed.",                        
                (apl_ssize_t)5, liLen);                                                   
        CPPUNIT_ASSERT_MESSAGE("SetDataBlock failed.",apl_strcmp(lsBuf, "12345")==0);

        CDataBlock *lpcDataBlock = loMemoryBlock.GetDataBlock();
        apl_size_t liSize = lpcDataBlock->GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetDataBlock failed.", (apl_size_t)5, liSize);

        //end environment
    }

    void testSwapDataBlock(void)
    {
        PRINT_TITLE_2(__func__);

        CDataBlock* loDataBlock1 = new CDataBlock("12345", 5, CMemoryBlock::DONT_DELETE);
        CDataBlock* loDataBlock2 = new CDataBlock("54321", 5, CMemoryBlock::DONT_DELETE);

        CMemoryBlock loMemoryBlock1(loDataBlock1);
        CMemoryBlock loMemoryBlock2(loDataBlock2);

        //case
        {
            loMemoryBlock1.SwapDataBlock(loMemoryBlock2);

            ASSERT_MESSAGE( apl_strcmp(loMemoryBlock1.GetReadPtr(), "54321") == 0 );
            ASSERT_MESSAGE( apl_strcmp(loMemoryBlock2.GetReadPtr(), "12345") == 0 );
        }
        
        //case
        {
            loMemoryBlock1.SetReadPtr(1);
            loMemoryBlock1.SetWritePtr(2);
            loMemoryBlock2.SetReadPtr(3);
            loMemoryBlock2.SetWritePtr(4);
            loMemoryBlock1.SwapDataBlock(loMemoryBlock2);

            ASSERT_MESSAGE( apl_strcmp(loMemoryBlock1.GetReadPtr(), "45") == 0 );
            ASSERT_MESSAGE( apl_strcmp(loMemoryBlock1.GetWritePtr(), "5") == 0 );
            ASSERT_MESSAGE( apl_strcmp(loMemoryBlock2.GetReadPtr(), "4321") == 0 );
            ASSERT_MESSAGE( apl_strcmp(loMemoryBlock2.GetWritePtr(), "321") == 0 );
        }
    }

    void testRelease(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start case environment
        CMemoryBlock loMemoryBlock(50); 
        apl_int_t liCnt = loMemoryBlock.GetReferCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetReferCount failed.", (apl_int_t)1, liCnt);

        //case
        loMemoryBlock.Release();
        CDataBlock *lpoDataBlock = loMemoryBlock.GetDataBlock();
        if(lpoDataBlock != NULL)
        {
            CPPUNIT_FAIL("Release failed.");
        }

        //end environment
    }
};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclMemoryBlock);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

