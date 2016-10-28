#include "Test.h"
#include "acl/ObjectPool.h"
#include "acl/ThreadManager.h"

using namespace acl;

class CTestObject
{
public:
    static apl_int_t ciCount;
    
public:
    CTestObject(void) : miValue(-1)                                      { ciCount++; }
    CTestObject( apl_int_t aiValue ) : miValue(aiValue)                  { ciCount++; }
    CTestObject( CTestObject const& aoOther ) : miValue(aoOther.miValue) { ciCount++; }
    ~CTestObject(void)                                                   { ciCount--; }
    
    void SetValue( apl_int_t aiValue )                                   { this->miValue = aiValue; }
    apl_int_t GetValue(void)                                             { return this->miValue; }
    
private:
    apl_int_t miValue;
};

class CTestConstructor
{
public:
    CTestConstructor( apl_int_t aiValue ) : miValue(aiValue) {}
    apl_int_t operator () ( CTestObject& aoObj )             { aoObj.SetValue(this->miValue); return 0; }

private:
    apl_int_t miValue;
};

class CTestDestructor
{
public:
    CTestDestructor( apl_int_t aiN )                         { ciCount = aiN; }
    ~CTestDestructor(void)                                   { ASSERT_MESSAGE(ciCount == 0); }
    void operator () ( CTestObject& aoObj )                  { ciCount--; }

private:
    static apl_int_t ciCount;
};

apl_int_t CTestObject::ciCount = 0;
apl_int_t CTestDestructor::ciCount = 0;

class CTestAclObjPool: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclObjPool);
    CPPUNIT_TEST( testInitialize1 );
    CPPUNIT_TEST( testInitialize2 );
    CPPUNIT_TEST( testInitialize3 );
    CPPUNIT_TEST( testInitialize4 );
    CPPUNIT_TEST( testInitialize5 );
    CPPUNIT_TEST( testInitialize6 );
    CPPUNIT_TEST( testGive1 );
    CPPUNIT_TEST( TestResize1 );
    CPPUNIT_TEST( TestResize2 );
    CPPUNIT_TEST( testGetIdle );
    CPPUNIT_TEST( testRelease );
    CPPUNIT_TEST( testDestroy1 );
    CPPUNIT_TEST( testDestroy2 );
    CPPUNIT_TEST( testClose1 );
    CPPUNIT_TEST( testClose2 );
    CPPUNIT_TEST( testClose3 );
    CPPUNIT_TEST( testStability );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}
    void testInitialize1(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        TObjectPool<CTestObject> loPool;
        
        //case
        ASSERT_MESSAGE(loPool.Initialize(3, true) == 0);//dynamic-create
        ASSERT_MESSAGE(loPool.GetSize() == 0);//dynamic-create
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);//dynamic-create
        {
            TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE( loPtr1->GetValue() == -1 );
            ASSERT_MESSAGE(loPool.GetSize() == 1);//dynamic-create
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);//dynamic-create
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);//dynamic-create
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);//dynamic-create
            
            TObjectPool<CTestObject>::PointerType loPtr2 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr2.IsEmpty() );
            ASSERT_MESSAGE( loPtr2->GetValue() == -1 );
            ASSERT_MESSAGE(loPool.GetSize() == 2);//dynamic-create
            ASSERT_MESSAGE(loPool.GetBusySize() == 2);//dynamic-create
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);//dynamic-create
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);//dynamic-create
            
            TObjectPool<CTestObject>::PointerType loPtr3 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr3.IsEmpty() );
            ASSERT_MESSAGE( loPtr3->GetValue() == -1 );
            ASSERT_MESSAGE(loPool.GetSize() == 3);//dynamic-create
            ASSERT_MESSAGE(loPool.GetBusySize() == 3);//dynamic-create
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);//dynamic-create
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);//dynamic-create
        }
        //end environment
        loPool.Close();
    }
    
    void testInitialize2(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        TObjectPool<CTestObject> loPool;
        
        //case
        ASSERT_MESSAGE(loPool.Initialize(3, false) == 0);
        ASSERT_MESSAGE(loPool.GetSize() == 3);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        {
            TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE( loPtr1->GetValue() == -1 );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 2);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject>::PointerType loPtr2 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr2.IsEmpty() );
            ASSERT_MESSAGE( loPtr2->GetValue() == -1 );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 2);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 1);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject>::PointerType loPtr3 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr3.IsEmpty() );
            ASSERT_MESSAGE( loPtr3->GetValue() == -1 );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 3);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        }
        //end environment
        loPool.Close();
    }
    
    void testInitialize3(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        TObjectPool<CTestObject, CTestConstructor> loPool;
        
        //case
        ASSERT_MESSAGE(loPool.Initialize(3, true, CTestConstructor(1) ) == 0);
        ASSERT_MESSAGE(loPool.GetSize() == 0);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        {
            TObjectPool<CTestObject, CTestConstructor>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE( loPtr1->GetValue() == 1 );
            ASSERT_MESSAGE(loPool.GetSize() == 1);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject, CTestConstructor>::PointerType loPtr2 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr2.IsEmpty() );
            ASSERT_MESSAGE( loPtr2->GetValue() == 1 );
            ASSERT_MESSAGE(loPool.GetSize() == 2);
            ASSERT_MESSAGE(loPool.GetBusySize() == 2);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject, CTestConstructor>::PointerType loPtr3 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr3.IsEmpty() );
            ASSERT_MESSAGE( loPtr3->GetValue() == 1 );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 3);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        }
        //end environment
        loPool.Close();
    }
    
    void testInitialize4(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        TObjectPool<CTestObject, CTestConstructor> loPool;
        
        //case
        ASSERT_MESSAGE(loPool.Initialize(3, false, CTestConstructor(1) ) == 0);
        ASSERT_MESSAGE(loPool.GetSize() == 3);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        {
            TObjectPool<CTestObject, CTestConstructor>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE( loPtr1->GetValue() == 1 );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 2);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject, CTestConstructor>::PointerType loPtr2 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr2.IsEmpty() );
            ASSERT_MESSAGE( loPtr2->GetValue() == 1 );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 2);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 1);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject, CTestConstructor>::PointerType loPtr3 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr3.IsEmpty() );
            ASSERT_MESSAGE( loPtr3->GetValue() == 1 );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 3);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        }
        //end environment
        loPool.Close();
    }
    
    void testInitialize5(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        TObjectPool<CTestObject, CTestConstructor, CTestDestructor> loPool;
        CTestDestructor loDestructor(3);
        
        //case
        ASSERT_MESSAGE(loPool.Initialize(3, true, CTestConstructor(1), loDestructor ) == 0);
        ASSERT_MESSAGE(loPool.GetSize() == 0);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        {
            TObjectPool<CTestObject, CTestConstructor, CTestDestructor>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE( loPtr1->GetValue() == 1 );
            ASSERT_MESSAGE(loPool.GetSize() == 1);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject, CTestConstructor, CTestDestructor>::PointerType loPtr2 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr2.IsEmpty() );
            ASSERT_MESSAGE( loPtr2->GetValue() == 1 );
            ASSERT_MESSAGE(loPool.GetSize() == 2);
            ASSERT_MESSAGE(loPool.GetBusySize() == 2);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject, CTestConstructor, CTestDestructor>::PointerType loPtr3 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr3.IsEmpty() );
            ASSERT_MESSAGE( loPtr3->GetValue() == 1 );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 3);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        }
        //end environment
        loPool.Close();
    }
    
    void testInitialize6(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        TObjectPool<CTestObject, CTestConstructor, CTestDestructor> loPool;
        CTestDestructor loDestructor(3);
        
        //case
        ASSERT_MESSAGE(loPool.Initialize(3, false, CTestConstructor(1), loDestructor ) == 0);
        ASSERT_MESSAGE(loPool.GetSize() == 3);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        {
            TObjectPool<CTestObject, CTestConstructor, CTestDestructor>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE( loPtr1->GetValue() == 1 );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 2);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject, CTestConstructor, CTestDestructor>::PointerType loPtr2 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr2.IsEmpty() );
            ASSERT_MESSAGE( loPtr2->GetValue() == 1 );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 2);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 1);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject, CTestConstructor, CTestDestructor>::PointerType loPtr3 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr3.IsEmpty() );
            ASSERT_MESSAGE( loPtr3->GetValue() == 1 );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 3);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        }
        //end environment
        loPool.Close();
    }
    
    void testGive1(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        TObjectPool<CTestObject> loPool;
        
        //case
        ASSERT_MESSAGE(loPool.Initialize(3, true) == 0);
        ASSERT_MESSAGE(loPool.GetSize() == 0);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);

        {
            ////
            ASSERT_MESSAGE(loPool.Give(new CTestObject(0) ) == 0);
            ASSERT_MESSAGE(loPool.GetSize() == 1);
            ASSERT_MESSAGE(loPool.GetBusySize() == 0);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 1);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE( loPtr1->GetValue() == 0 );
            ASSERT_MESSAGE(loPool.GetSize() == 1);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            ////
            ASSERT_MESSAGE(loPool.Give(new CTestObject(1) ) == 0);
            ASSERT_MESSAGE(loPool.GetSize() == 2);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 1);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject>::PointerType loPtr2 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr2.IsEmpty() );
            ASSERT_MESSAGE( loPtr2->GetValue() == 1 );
            ASSERT_MESSAGE(loPool.GetSize() == 2);
            ASSERT_MESSAGE(loPool.GetBusySize() == 2);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            ////
            ASSERT_MESSAGE(loPool.Give(new CTestObject(2) ) == 0);
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 2);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 1);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject>::PointerType loPtr3 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr3.IsEmpty() );
            ASSERT_MESSAGE( loPtr3->GetValue() == 2 );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 3);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);

            ASSERT_MESSAGE(loPool.Give(NULL) == -1);
        }
        //end environment
        loPool.Close();
    }
    
    void TestResize1(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        TObjectPool<CTestObject> loPool;
        
        //case
        ASSERT_MESSAGE(loPool.Initialize(3, true) == 0);
        ASSERT_MESSAGE(loPool.GetSize() == 0);
        ASSERT_MESSAGE(loPool.GetBusySize() == 0);
        ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        
        {
            TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE(loPool.GetSize() == 1);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            ASSERT_MESSAGE( loPtr1->GetValue() == -1 );
            
            ASSERT_MESSAGE(loPool.Resize(1) == 0);
            ASSERT_MESSAGE(loPool.GetSize() == 1);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 1);
            
            ASSERT_MESSAGE(loPool.Resize(3) == 0);
            ASSERT_MESSAGE(loPool.GetSize() == 1);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject>::PointerType loPtr2 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr2.IsEmpty() );
            ASSERT_MESSAGE(loPool.GetSize() == 2);
            ASSERT_MESSAGE(loPool.GetBusySize() == 2);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            ASSERT_MESSAGE( loPtr2->GetValue() == -1 );
            
            ASSERT_MESSAGE(loPool.Resize(1) == 0);
            ASSERT_MESSAGE(loPool.GetSize() == 2);
            ASSERT_MESSAGE(loPool.GetBusySize() == 2);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 1);
            
            loPtr2.Release();
            
            ASSERT_MESSAGE(loPool.GetSize() == 1);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 1);
        }
        
        //end environment
        loPool.Close();
    }
    
    void TestResize2(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        TObjectPool<CTestObject> loPool;
        
        //case
        ASSERT_MESSAGE(loPool.Initialize(3, false) == 0);
        ASSERT_MESSAGE(loPool.GetSize() == 3);
        ASSERT_MESSAGE(loPool.GetBusySize() == 0);
        ASSERT_MESSAGE(loPool.GetIdleSize() == 3);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        
        {
            TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 2);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            ASSERT_MESSAGE( loPtr1->GetValue() == -1 );
            
            ASSERT_MESSAGE(loPool.Resize(1) == 0);
            ASSERT_MESSAGE(loPool.GetSize() == 1);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 1);
            
            ASSERT_MESSAGE(loPool.Resize(3) == 0);
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 2);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            TObjectPool<CTestObject>::PointerType loPtr2 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr2.IsEmpty() );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 2);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 1);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            ASSERT_MESSAGE( loPtr2->GetValue() == -1 );
            
            ASSERT_MESSAGE(loPool.Resize(1) == 0);
            ASSERT_MESSAGE(loPool.GetSize() == 2);
            ASSERT_MESSAGE(loPool.GetBusySize() == 2);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 1);
            
            loPtr2.Release();
            
            ASSERT_MESSAGE(loPool.GetSize() == 1);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 1);
            
            ASSERT_MESSAGE(loPool.Resize(4) == 0);
            ASSERT_MESSAGE(loPool.GetSize() == 4);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 3);
            ASSERT_MESSAGE(loPool.GetCapacity() == 4);
        }
        
        //end environment
        loPool.Close();
    }
    
    void testGetIdle(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        struct CParam {
            CParam(TObjectPool<CTestObject>& aoPool, apl_int_t& aiCount)
                : loPool(aoPool)
                , liCount(aiCount)
            {}
            
            TObjectPool<CTestObject>& loPool;
            apl_int_t& liCount;
        };
        
        TObjectPool<CTestObject> loPool;
        apl_int_t liCount = 0;
        CParam loParam(loPool, liCount);
        
        
        START_THREAD_BODY(mybody1, CParam, loParam)
            acl::CTimestamp loStart, loEnd;
            loStart.Update(acl::CTimestamp::PRC_SEC);
            TObjectPool<CTestObject>::PointerType loPtr= loParam.loPool.GetIdle( acl::CTimeValue(5) );
            ASSERT_MESSAGE( loPtr.IsEmpty() );
            loEnd.Update(acl::CTimestamp::PRC_SEC);
            ASSERT_MESSAGE(loEnd.Sec() - loStart.Sec() >= 4);
            loParam.liCount = 1;
        END_THREAD_BODY(mybody1);
        
        START_THREAD_BODY(mybody2, CParam, loParam)
            TObjectPool<CTestObject>::PointerType loPtr= loParam.loPool.GetIdle( acl::CTimeValue(5) );
            ASSERT_MESSAGE( !loPtr.IsEmpty() )
            loParam.liCount = 3;
        END_THREAD_BODY(mybody2);
        
        //case
        ASSERT_MESSAGE(loPool.Initialize(1, false) == 0);
        ASSERT_MESSAGE(loPool.GetSize() == 1);
        ASSERT_MESSAGE(loPool.GetBusySize() == 0);
        ASSERT_MESSAGE(loPool.GetIdleSize() == 1);
        ASSERT_MESSAGE(loPool.GetCapacity() == 1);
        
        {
            TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE(loPool.GetSize() == 1);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 1);
            ASSERT_MESSAGE( loPtr1->GetValue() == -1 );
            
            TObjectPool<CTestObject>::PointerType loPtr2 = loPool.GetIdle( acl::CTimeValue(1) );
            ASSERT_MESSAGE( loPtr2.IsEmpty() );
            
            RUN_THREAD_BODY(mybody1);
            WAIT_EXP(liCount == 1);
            
            RUN_THREAD_BODY(mybody2);
            loPool.Resize(2);
            WAIT_EXP(liCount == 3);
        }
        
        //end environment
        loPool.Close();
    }
    
    void testRelease(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        TObjectPool<CTestObject> loPool;
        
        //case
        ASSERT_MESSAGE(loPool.Initialize(3, false) == 0);
        ASSERT_MESSAGE(loPool.GetSize() == 3);
        ASSERT_MESSAGE(loPool.GetBusySize() == 0);
        ASSERT_MESSAGE(loPool.GetIdleSize() == 3);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        
        {
            TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 2);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            loPtr1.Release();
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 0);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 3);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        }
        
        ASSERT_MESSAGE(loPool.GetSize() == 3);
        ASSERT_MESSAGE(loPool.GetBusySize() == 0);
        ASSERT_MESSAGE(loPool.GetIdleSize() == 3);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        
        {
            TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 2);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        }
        
        ASSERT_MESSAGE(loPool.GetSize() == 3);
        ASSERT_MESSAGE(loPool.GetBusySize() == 0);
        ASSERT_MESSAGE(loPool.GetIdleSize() == 3);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        
        loPool.Close();
    }
    
    void testDestroy1(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        TObjectPool<CTestObject> loPool;
        
        //case
        ASSERT_MESSAGE(loPool.Initialize(3, false) == 0);
        ASSERT_MESSAGE(loPool.GetSize() == 3);
        ASSERT_MESSAGE(loPool.GetBusySize() == 0);
        ASSERT_MESSAGE(loPool.GetIdleSize() == 3);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        
        {
            TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 2);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            loPtr1.Destroy();
            loPtr1.Release();
            
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 0);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 3);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        }
        
        ASSERT_MESSAGE(loPool.GetSize() == 3);
        ASSERT_MESSAGE(loPool.GetBusySize() == 0);
        ASSERT_MESSAGE(loPool.GetIdleSize() == 3);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        
        {
            TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE(loPool.GetSize() == 3);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 2);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            loPtr1.Destroy();
        }
        
        ASSERT_MESSAGE(loPool.GetSize() == 3);
        ASSERT_MESSAGE(loPool.GetBusySize() == 0);
        ASSERT_MESSAGE(loPool.GetIdleSize() == 3);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        
        loPool.Close();
    }
    
    void testDestroy2(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        TObjectPool<CTestObject> loPool;
        
        //case
        ASSERT_MESSAGE(loPool.Initialize(3, true) == 0);
        ASSERT_MESSAGE(loPool.GetSize() == 0);
        ASSERT_MESSAGE(loPool.GetBusySize() == 0);
        ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        
        {
            TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE(loPool.GetSize() == 1);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            loPtr1.Destroy();
            loPtr1.Release();
            
            ASSERT_MESSAGE(loPool.GetSize() == 0);
            ASSERT_MESSAGE(loPool.GetBusySize() == 0);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        }
        
        ASSERT_MESSAGE(loPool.GetSize() == 0);
        ASSERT_MESSAGE(loPool.GetBusySize() == 0);
        ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        
        {
            TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
            ASSERT_MESSAGE( !loPtr1.IsEmpty() );
            ASSERT_MESSAGE(loPool.GetSize() == 1);
            ASSERT_MESSAGE(loPool.GetBusySize() == 1);
            ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
            ASSERT_MESSAGE(loPool.GetCapacity() == 3);
            
            loPtr1.Destroy();
        }
        
        ASSERT_MESSAGE(loPool.GetSize() == 0);
        ASSERT_MESSAGE(loPool.GetBusySize() == 0);
        ASSERT_MESSAGE(loPool.GetIdleSize() == 0);
        ASSERT_MESSAGE(loPool.GetCapacity() == 3);
        
        loPool.Close();
    }
    
    void testClose1(void)
    {
        PRINT_TITLE_2(__func__);
        
        {
            //start environment
            TObjectPool<CTestObject> loPool;
            ASSERT_MESSAGE(loPool.Initialize(3, true) == 0);
            
            TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
            TObjectPool<CTestObject>::PointerType loPtr2 = loPool.GetIdle();
            TObjectPool<CTestObject>::PointerType loPtr3 = loPool.GetIdle();
        }
        
        ASSERT_MESSAGE(CTestObject::ciCount == 0);
    }
    
    void testClose2(void)
    {
        PRINT_TITLE_2(__func__);
        
        {
            //start environment
            TObjectPool<CTestObject> loPool;
            {
                ASSERT_MESSAGE(loPool.Initialize(3, true) == 0);
                
                TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
                TObjectPool<CTestObject>::PointerType loPtr2 = loPool.GetIdle();
                TObjectPool<CTestObject>::PointerType loPtr3 = loPool.GetIdle();
            }
            loPool.Close();
        }
        
        ASSERT_MESSAGE(CTestObject::ciCount == 0);
    }
    
    void testClose3(void)
    {
        PRINT_TITLE_2(__func__);
        
        {   
            {
                //start environment
                TObjectPool<CTestObject> loPool;
                ASSERT_MESSAGE(loPool.Initialize(3, true) == 0);
                
                TObjectPool<CTestObject>::PointerType loPtr1 = loPool.GetIdle();
                TObjectPool<CTestObject>::PointerType loPtr2 = loPool.GetIdle();
                TObjectPool<CTestObject>::PointerType loPtr3 = loPool.GetIdle();
                
                loPool.Close(acl::CTimeValue(1) );
            }
        }
        
        ASSERT_MESSAGE(CTestObject::ciCount == 0);
    }
    
    void testStability(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        struct CParam {
            CParam(TObjectPool<CTestObject>& aoPool, apl_int_t& aiCount)
                : loPool(aoPool)
                , liCount(aiCount)
            {}
            
            TObjectPool<CTestObject>& loPool;
            apl_int_t& liCount;
        };
        
        TObjectPool<CTestObject> loPool;
        apl_int_t liCount = 0;
        CParam loParam(loPool, liCount);
        
        printf("\n");
        START_THREAD_BODY(mybody1, CParam, loParam)
            START_LOOP(100000)
            {
                TObjectPool<CTestObject>::PointerType loPtr= loParam.loPool.GetIdle( acl::CTimeValue(5) );
                assert(!loPtr.IsEmpty());
                //ASSERT_MESSAGE( !loPtr.IsEmpty() );
            }
            END_LOOP();
            loParam.liCount = 1;
        END_THREAD_BODY(mybody1);
        
        START_THREAD_BODY(mybody2, CParam, loParam)
            START_LOOP(100000)
            {
                TObjectPool<CTestObject>::PointerType loPtr= loParam.loPool.GetIdle( acl::CTimeValue(5) );
                assert(!loPtr.IsEmpty());
                //ASSERT_MESSAGE( !loPtr.IsEmpty() );
            }
            END_LOOP();
            WAIT_EXP(loParam.liCount == 1);
            loParam.liCount = 2;
        END_THREAD_BODY(mybody2);
        
        ASSERT_MESSAGE(loPool.Initialize(3, false) == 0);
        
        RUN_THREAD_BODY(mybody1);
        RUN_THREAD_BODY(mybody2);
        WAIT_EXP(liCount == 2);
        
        loPool.Close();
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclObjPool);

int main()
{    
    RUN_ALL_TEST(__FILE__);
}
