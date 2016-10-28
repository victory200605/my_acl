#include "Test.h"
#include "acl/IniConfig.h"
#include "acl/XmlConfig.h"

#define INIPATH  "./My.ini"

using namespace acl;

void TestSetValue(CBaseConfig *aoConfig)
{
    //SetValue
    apl_int_t liRet = -1;
    liRet = aoConfig->SetValue("FIELD_1", "f1_int",  (apl_int_t)1, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
    liRet = aoConfig->SetValue("FIELD_1", "f1_bool", true, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
    liRet = aoConfig->SetValue("FIELD_2", "f2_double", 3.3, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
    liRet = aoConfig->SetValue("FIELD_3", "f3_str", "value4", true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
    liRet = aoConfig->SetValue("FIELD_3", "f3_str", "newvalue4", false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);

    liRet = aoConfig->SetValue("FIELD_3", "key5", "value5", false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)-1, liRet);
    liRet = aoConfig->SetValue("FIELD_4", "key4", "new value4", false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)-1, liRet);
}

void TestGetValue1(CBaseConfig *aoConfig)
{
    //GetValue
    CIniConfig::KeyIterType loIter1 = aoConfig->GetValue("FIELD_1", "f1_int", "0");
    if(!loIter1.IsEmpty())
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue int failed.", (apl_int_t)1, loIter1.ToInt());
    }

    CIniConfig::KeyIterType loIter2 = aoConfig->GetValue("FIELD_1", "f1_null", "-1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue int failed.", (apl_int_t)-1, loIter2.ToInt());

    CIniConfig::KeyIterType loIter3 = aoConfig->GetValue("FIELD_1", "f1_bool", "true");
    if(!loIter3.IsEmpty())
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue bool failed.", true, loIter3.ToBool());
    }
    CIniConfig::KeyIterType loIter4 = aoConfig->GetValue("FIELD_2", "f2_double", "1.1");
    if(!loIter4.IsEmpty())
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue float failed.", 3.3, loIter4.ToFloat());
    }
    CIniConfig::KeyIterType loIter5 = aoConfig->GetValue("FIELD_3", "f3_str", "default str");
    if(!loIter5.IsEmpty())
    {
        CPPUNIT_ASSERT_MESSAGE("GetValue str failed.", apl_strcmp(loIter5.ToString(), "newvalue4")==0);
    }

}

void TestGetValue2(CBaseConfig *aoConfig)
{
    CIniConfig::KeyIterType loIter1 = aoConfig->GetValue("FIELD_1", (apl_size_t)0, "0");
    if(!loIter1.IsEmpty())
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue int failed.", (apl_int_t)1, loIter1.ToInt());
    }

    CIniConfig::KeyIterType loIter3 = aoConfig->GetValue("FIELD_1", (apl_size_t)1, NULL);
    if(!loIter3.IsEmpty())
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue bool failed.", true, loIter3.ToBool());
    }
    CIniConfig::KeyIterType loIter4 = aoConfig->GetValue("FIELD_2", (apl_size_t)0, "1.1");
    if(!loIter4.IsEmpty())
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue float failed.", 3.3, loIter4.ToFloat());
    }
    CIniConfig::KeyIterType loIter5 = aoConfig->GetValue("FIELD_3", (apl_size_t)0, "default str");
    if(!loIter5.IsEmpty())
    {
        CPPUNIT_ASSERT_MESSAGE("GetValue str failed.", apl_strcmp(loIter5.ToString(), "newvalue4")==0);
    }

    CIniConfig::KeyIterType loIter2 = aoConfig->GetValue("FIELD_1", (apl_size_t)8, "-1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue int failed.", (apl_int_t)-1, loIter2.ToInt());

}
void TestIterator(CBaseConfig *aoConfig)
{
    char const* lpsField[] = {"SERVER1", "SERVER2", "SERVER3"};

    apl_int_t liFlag = 0;
    for(CBaseConfig::FieldIterType loIter = aoConfig->Begin(); loIter != aoConfig->End(); ++loIter)
    {
        char const*  lpcTmp = loIter.GetFieldName();
        CPPUNIT_ASSERT_MESSAGE("GetFieldName failed.", apl_strcmp(lpcTmp, lpsField[liFlag++]) == 0);
    }

    CBaseConfig::FieldIterType loIter = aoConfig->Begin();
    if(loIter == aoConfig->Begin())
    {
        CIniConfig::KeyIterType loValue1 = loIter.GetValue("key1","default");
        if(!loValue1.IsEmpty())
        {
            CPPUNIT_ASSERT_MESSAGE("CFieldIterator GetValue failed.", apl_strcmp(loValue1.ToString(), "value1") == 0);
        }

        CIniConfig::KeyIterType loValue2 = loIter.GetValue(2, "default");
        if(!loValue1.IsEmpty())
        {            
            CPPUNIT_ASSERT_MESSAGE("CFieldIterator GetValue failed.", apl_strcmp(loValue2.ToString(), "value12") == 0);
        }

        apl_size_t liSize = loIter.GetKeyCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("CFieldIterator GetKeyCount failed.", (apl_size_t)3, liSize);

        char const* lsValue[] = {"value1", "value11", "value12"};
        apl_int_t liFlag = 0;
        for(CIniConfig::KeyIterType loKeyIter = loIter.Begin(); loKeyIter != loIter.End(); ++loKeyIter)
        {
            CPPUNIT_ASSERT_MESSAGE("CFieldIterator Begin End failed.", 
                    apl_strcmp(loKeyIter.ToString(), lsValue[liFlag++]) == 0);
        }
    }
}

void TestComment(CBaseConfig *aoConfig)
{
    apl_int_t liRet = -1;                                                   

    liRet = aoConfig->AddComment("SERVER1", "server field...1");             
    CPPUNIT_ASSERT_EQUAL_MESSAGE("AddComment failed.", (apl_int_t)0, liRet);
    liRet = aoConfig->AddComment("SERVER1", "server field...2");             
    liRet = aoConfig->AddComment("SERVER1", "key1", "key1 comment");         
    CPPUNIT_ASSERT_EQUAL_MESSAGE("AddComment failed.", (apl_int_t)0, liRet);

    liRet = aoConfig->DelComment("SERVER1");                                 
    CPPUNIT_ASSERT_EQUAL_MESSAGE("DelComment failed.", (apl_int_t)2, liRet);
    liRet = aoConfig->DelComment("SERVER1", "key1");                         
    CPPUNIT_ASSERT_EQUAL_MESSAGE("DelComment failed.", (apl_int_t)1, liRet);
}

/* ----------------------------------------------------------------- */

class CTestAclIniConfig:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclIniConfig);
    CPPUNIT_TEST(testOpen);
    CPPUNIT_TEST(testSync);
    CPPUNIT_TEST(testGetKeyCount);
    CPPUNIT_TEST(testGetFieldCount);
    CPPUNIT_TEST(testGetValue1);
    CPPUNIT_TEST(testGetValue2);
    CPPUNIT_TEST(testSetValueInt);
    CPPUNIT_TEST(testSetValueBool);
    CPPUNIT_TEST(testSetValueDouble);
    CPPUNIT_TEST(testSetValueStr);
    CPPUNIT_TEST(testIterator);
    CPPUNIT_TEST(testComment);
    CPPUNIT_TEST(testReload);
    CPPUNIT_TEST_SUITE_END();

private:    
    apl_handle_t ciFd;
    CIniConfig coConfig;

public:
    void setUp(void) 
    {
        ciFd =open(INIPATH, APL_O_WRONLY|APL_O_CREAT|APL_O_TRUNC, 0666);
        if(ciFd < 0)
        {
            CPPUNIT_FAIL("SetUp open file failed.");
        }
        //Open
        apl_int_t liRet = coConfig.Open( INIPATH );                                 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open failed.", (apl_int_t)0, liRet);

        liRet = coConfig.SetValue("SERVER1", "key1", "value1", true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
        liRet = coConfig.SetValue("SERVER1", "key11", "value11", true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
        liRet = coConfig.SetValue("SERVER1", "key12", "value12", true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
        liRet = coConfig.SetValue("SERVER2", "key2", "value2", true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
        liRet = coConfig.SetValue("SERVER3", "key3", "value3", true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
        coConfig.Sync();
    }
    void tearDown(void) 
    {
        unlink(INIPATH);
        apl_close(ciFd);
        coConfig.Close();
    }

    void testOpen(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_handle_t liHandle = apl_open("tmp.txt", APL_O_WRONLY|APL_O_CREAT, 0666);
        PrintErrMsg(apl_strerror(apl_get_errno()), liHandle);

        //case
        apl_int_t liRet = coConfig.Open( "tmp.txt" );                       
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open failed.", (apl_int_t)0, liRet);

        //end environment
        apl_close(liHandle);
        unlink("tmp.txt");
    }

    void testSync(void)
    {
        //start case environment
        CIniConfig loConfig;
        char lsBuf[50];
        char *lpcPos = NULL;
        apl_handle_t liHandle = apl_open("tmp.txt", APL_O_RDWR|APL_O_CREAT|APL_O_TRUNC, 0666);
        PrintErrMsg(apl_strerror(apl_get_errno()), liHandle);                        

        apl_int_t liRet = loConfig.Open( "tmp.txt" );                               
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open failed.", (apl_int_t)0, liRet);           
        liRet = loConfig.SetValue("Sync", "key1", "value1", true);         
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);

        //case
        apl_lseek(liHandle, 0, APL_SEEK_SET);
        memset(lsBuf, 0, sizeof(lsBuf));
        apl_read(liHandle, lsBuf, sizeof(lsBuf));
        lpcPos = apl_strstr(lsBuf, "Sync");
        if(lpcPos != NULL)
        {
            CPPUNIT_FAIL("Sync failed.");
        }
        liRet = loConfig.Sync();    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sync failed.", (apl_int_t)0, liRet);
        apl_lseek(liHandle, 0, APL_SEEK_SET);
        memset(lsBuf, 0, sizeof(lsBuf));
        apl_read(liHandle, lsBuf, sizeof(lsBuf));
        lpcPos = apl_strstr(lsBuf, "Sync");
        if(lpcPos == NULL)
        {
            CPPUNIT_FAIL("Sync failed.");
        }

        //end environment
        apl_close(liHandle);                                                         
        unlink("tmp.txt");                                                          
    }

    void testGetKeyCount(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment

        //case
        apl_size_t liCnt = coConfig.GetKeyCount("SERVER1");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetKeyCount failed.", (apl_size_t)3, liCnt);

        //end environment
    }

    void testGetFieldCount(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        
        //case
        apl_size_t liCnt = 0;
        liCnt = coConfig.GetFieldCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetFieldCount failed.", (apl_size_t)3, liCnt);
        liCnt = coConfig.GetFieldCount("S.*");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetFieldCount failed.", (apl_size_t)3, liCnt);

        //end environment
    }

    void testGetValue1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TestSetValue(&coConfig);
        apl_int_t liRet = coConfig.Sync();    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sync failed.", (apl_int_t)0, liRet);

        //case
        TestGetValue1(&coConfig);

        //end environment
    }

    void testGetValue2(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        TestSetValue(&coConfig);
        apl_int_t liRet = coConfig.Sync();    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sync failed.", (apl_int_t)0, liRet);

        //case
        TestGetValue2(&coConfig);

        //end environment
    }

    void testSetValueInt(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment                  

        //case
        apl_int_t liRet = coConfig.SetValue("FIELD_1", "f1_int",  (apl_int_t)1, true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);

        //end environment
    }

    void testSetValueBool(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment                  

        //case
        apl_int_t liRet = coConfig.SetValue("FIELD_1", "f1_bool", true, true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);

        //end environment
    }

    void testSetValueDouble(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment                  

        //case
        apl_int_t liRet = coConfig.SetValue("FIELD_2", "f2_double", 3.3, true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
        //end environment
    }

    void testSetValueStr(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment                  

        //case
        apl_int_t liRet = coConfig.SetValue("FIELD_3", "f3_str", "value4", true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);

        //end environment
    }

    void testIterator(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment

        //case
        TestIterator(&coConfig);

        //end environment
    }

    void testComment(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment

        //case
        TestComment(&coConfig);
        apl_int_t liRet = coConfig.Sync();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sync failed.", (apl_int_t)0, liRet);

        //end environment
    }

    void testReload(void)
    {
        PRINT_TITLE_2(__func__); 

        //start case environment
        CIniConfig::KeyIterType loIter1 = coConfig.GetValue("SERVER1", "key1", "default");     
        if(!loIter1.IsEmpty())                                                                  
        {                                                                                       
            CPPUNIT_ASSERT_MESSAGE("Get value failed.",
                    apl_strcmp(loIter1.ToString(), "value1")==0);
        }                                                                                       

        //case
        apl_int_t liRet = coConfig.Reload();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Reload failed.", (apl_int_t)0, liRet);
        CIniConfig::KeyIterType loIter2 = coConfig.GetValue("SERVER1", "key1", "default");     
        if(!loIter2.IsEmpty())                                                                  
        {                                                                                       
            CPPUNIT_ASSERT_MESSAGE("Reload failed.",
                    apl_strcmp(loIter2.ToString(), "value1")==0);
        } 

        //end environment
    }
};  

class CTestAclXmlConfig:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclXmlConfig);
    CPPUNIT_TEST(testOpen);
    CPPUNIT_TEST(testSync);
    CPPUNIT_TEST(testGetKeyCount);
    CPPUNIT_TEST(testGetFieldCount);
    CPPUNIT_TEST(testGetValue1);
    CPPUNIT_TEST(testGetValue2);
    CPPUNIT_TEST(testSetValueInt);
    CPPUNIT_TEST(testSetValueBool);
    CPPUNIT_TEST(testSetValueDouble);
    CPPUNIT_TEST(testSetValueStr);
    CPPUNIT_TEST(testIterator);
    CPPUNIT_TEST(testReload);
    CPPUNIT_TEST_SUITE_END();

private:    
    apl_handle_t ciFd;
    CXmlConfig coConfig;

public:
    void setUp(void) 
    {
        ciFd =open(INIPATH, APL_O_WRONLY|APL_O_CREAT|APL_O_TRUNC, 0666); 
        if(ciFd < 0)
        {
            CPPUNIT_FAIL("SetUp open file failed.");
        }
        apl_int_t liRet = coConfig.Open( INIPATH );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open failed.", (apl_int_t)0, liRet);
        
        liRet = coConfig.SetValue("SERVER1", "key1", "value1", true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
        liRet = coConfig.SetValue("SERVER1", "key11", "value11", true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
        liRet = coConfig.SetValue("SERVER1", "key12", "value12", true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
        liRet = coConfig.SetValue("SERVER2", "key2", "value2", true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
        liRet = coConfig.SetValue("SERVER3", "key3", "value3", true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
        coConfig.Sync();
    }
    void tearDown(void) 
    {
        unlink(INIPATH);
        apl_close(ciFd);
        coConfig.Close();
    }

    void testOpen(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_handle_t liHandle = apl_open("tmp.txt", APL_O_WRONLY|APL_O_CREAT, 0666);
        PrintErrMsg(apl_strerror(apl_get_errno()), liHandle);

        //case
        apl_int_t liRet = coConfig.Open( "tmp.txt" );                       
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open failed.", (apl_int_t)0, liRet);

        //end environment
        apl_close(liHandle);
        unlink("tmp.txt");
    }

    void testSync(void)
    {
        //start case environment
        CIniConfig loConfig;
        char lsBuf[50];
        char *lpcPos = NULL;
        apl_handle_t liHandle = apl_open("tmp.txt", APL_O_RDWR|APL_O_CREAT|APL_O_TRUNC, 0666);
        PrintErrMsg(apl_strerror(apl_get_errno()), liHandle);                        

        apl_int_t liRet = loConfig.Open( "tmp.txt" );                               
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open failed.", (apl_int_t)0, liRet);           
        liRet = loConfig.SetValue("Sync", "key1", "value1", true);         
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);

        //case
        apl_lseek(liHandle, 0, APL_SEEK_SET);
        memset(lsBuf, 0, sizeof(lsBuf));
        apl_read(liHandle, lsBuf, sizeof(lsBuf));
        lpcPos = apl_strstr(lsBuf, "Sync");
        if(lpcPos != NULL)
        {
            CPPUNIT_FAIL("Sync failed.");
        }
        liRet = loConfig.Sync();    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sync failed.", (apl_int_t)0, liRet);
        apl_lseek(liHandle, 0, APL_SEEK_SET);
        memset(lsBuf, 0, sizeof(lsBuf));
        apl_read(liHandle, lsBuf, sizeof(lsBuf));
        lpcPos = apl_strstr(lsBuf, "Sync");
        if(lpcPos == NULL)
        {
            CPPUNIT_FAIL("Sync failed.");
        }

        //end environment
        apl_close(liHandle);                                                         
        unlink("tmp.txt");                                                          
    }

    void testGetKeyCount(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment

        //case
        apl_size_t liCnt = coConfig.GetKeyCount("SERVER1");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetKeyCount failed.", (apl_size_t)3, liCnt);

        //end environment
    }

    void testGetFieldCount(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        
        //case
        apl_size_t liCnt = 0;
        liCnt = coConfig.GetFieldCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetFieldCount failed.", (apl_size_t)3, liCnt);
        liCnt = coConfig.GetFieldCount("S.*");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetFieldCount failed.", (apl_size_t)3, liCnt);

        //end environment
    }

    void testGetValue1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TestSetValue(&coConfig);
        apl_int_t liRet = coConfig.Sync();    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sync failed.", (apl_int_t)0, liRet);

        //case
        TestGetValue1(&coConfig);

        //end environment
    }

    void testGetValue2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TestSetValue(&coConfig);
        apl_int_t liRet = coConfig.Sync();    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sync failed.", (apl_int_t)0, liRet);

        //case
        TestGetValue2(&coConfig);

        //end environment
    }

    void testSetValueInt(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment                  

        //case
        apl_int_t liRet = coConfig.SetValue("FIELD_1", "f1_int",  (apl_int_t)1, true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);

        //end environment
    }

    void testSetValueBool(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment                  

        //case
        apl_int_t liRet = coConfig.SetValue("FIELD_1", "f1_bool", true, true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);

        //end environment
    }

    void testSetValueDouble(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment                  

        //case
        apl_int_t liRet = coConfig.SetValue("FIELD_2", "f2_double", 3.3, true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);
        //end environment
    }

    void testSetValueStr(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment                  

        //case
        apl_int_t liRet = coConfig.SetValue("FIELD_3", "f3_str", "value4", true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SetValue failed.", (apl_int_t)0, liRet);

        //end environment
    }

    void testIterator(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment

        //case
        TestIterator(&coConfig);

        //end environment
    }

    void testReload(void)
    {
        PRINT_TITLE_2(__func__); 

        //start case environment
        CIniConfig::KeyIterType loIter1 = coConfig.GetValue("SERVER1", "key1", "default");     
        if(!loIter1.IsEmpty())                                                                  
        {                                                                                       
            CPPUNIT_ASSERT_MESSAGE("Get value failed.",
                    apl_strcmp(loIter1.ToString(), "value1")==0);
        }                                                                                       

        //case
        apl_int_t liRet = coConfig.Reload();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Reload failed.", (apl_int_t)0, liRet);

        CIniConfig::KeyIterType loIter2 = coConfig.GetValue("SERVER1", "key1", "default");     
        if(!loIter2.IsEmpty())                                                                  
        {                                                                                       
            CPPUNIT_ASSERT_MESSAGE("Reload failed.",
                    apl_strcmp(loIter2.ToString(), "value1")==0);
        }                                                                                       

        //end environment
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclIniConfig);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclXmlConfig);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

