
#include "Test.h"
#include "acl/ThreadManager.h"
#include "acl/SmartPtr.h"

///////////////////////////////////////////////////////////////////////////////////////////
class CTestParent
{
public:
    CTestParent(void)
    {
        ++CTestParent::ciCount;
    }
    
    virtual ~CTestParent(void)
    {
        --CTestParent::ciCount;
    }
    
    virtual apl_int_t Who(void) const
    {
        return 0;
    }
    
    static apl_int_t GetCount()
    {
        return CTestParent::ciCount;
    }
    
private:
    static acl::TNumber<apl_int_t, acl::CLock> ciCount;
};

acl::TNumber<apl_int_t, acl::CLock> CTestParent::ciCount(0);

/////////////////////////////////////////////////////////////////////////////////////////
class CTestChild : public CTestParent
{
public:
    virtual ~CTestChild(void)
    {
    }
    
    virtual apl_int_t Who(void) const
    {
        return 1;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////
class CTestCounter
{
public:
    CTestCounter(void)
    {
        ++CTestCounter::ciCount;
    }
    
    virtual ~CTestCounter(void)
    {
        --CTestCounter::ciCount;
    }

    static apl_int_t GetCount()
    {
        return CTestCounter::ciCount;
    }
    
private:
    static apl_int_t ciCount;
};

apl_int_t CTestCounter::ciCount = 0;

//////////////////////////////////////////////////////////////////////////////////////
class CTestAclShartPtr: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclShartPtr);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testAssign);
    CPPUNIT_TEST(testSwap);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST(testGetUseCount);
    CPPUNIT_TEST(testGet);
    CPPUNIT_TEST(testOperatorNON);
    CPPUNIT_TEST(testPointer);
    CPPUNIT_TEST(testReference);
    CPPUNIT_TEST(testCompare);
    CPPUNIT_TEST(testPerformance1);
    CPPUNIT_TEST(testPerformance2);
    CPPUNIT_TEST(testPerformance3);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void){}
    void tearDown(void){}
    
    void testConstructor(void)
    {
        {
            acl::TSharePtr<CTestParent> loPtr1(new CTestParent);
            acl::TSharePtr<CTestParent> loPtr2(loPtr1);
            acl::TSharePtr<CTestParent> loPtr3(loPtr2, acl::STATIC_CAST_TAG);
            acl::TSharePtr<CTestParent> loPtr4(loPtr2, acl::REINTERPRET_CAST_TAG);
            acl::TSharePtr<const CTestParent> loPtr5(new CTestParent);
            acl::TSharePtr<CTestParent> loPtr6(loPtr5, acl::CONST_CAST_TAG);
            acl::TSharePtr<CTestChild>  loPtr7(new CTestChild);
            acl::TSharePtr<CTestParent> loPtr8(loPtr7, acl::DYNAMIC_CAST_TAG);
            
            ASSERT_MESSAGE(loPtr1 != NULL && loPtr1->Who() == 0);
            ASSERT_MESSAGE(loPtr2 != NULL && loPtr2->Who() == 0);
            ASSERT_MESSAGE(loPtr3 != NULL && loPtr3->Who() == 0);
            ASSERT_MESSAGE(loPtr4 != NULL && loPtr4->Who() == 0);
            ASSERT_MESSAGE(loPtr5 != NULL && loPtr5->Who() == 0);
            ASSERT_MESSAGE(loPtr6 != NULL && loPtr6->Who() == 0);
            ASSERT_MESSAGE(loPtr7 != NULL && loPtr7->Who() == 1);
            ASSERT_MESSAGE(loPtr8 != NULL && loPtr8->Who() == 1);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testAssign(void)
    {
        {
            acl::TSharePtr<CTestParent> loPtr1(new CTestParent);
            acl::TSharePtr<CTestParent> loPtr2 = loPtr1;
            acl::TSharePtr<CTestChild>  loPtr3(new CTestChild);
            acl::TSharePtr<CTestParent> loPtr4 = loPtr3;
            
            ASSERT_MESSAGE(loPtr1 != NULL && loPtr1->Who() == 0);
            ASSERT_MESSAGE(loPtr2 != NULL && loPtr2->Who() == 0);
            ASSERT_MESSAGE(loPtr3 != NULL && loPtr3->Who() == 1);
            ASSERT_MESSAGE(loPtr4 != NULL && loPtr4->Who() == 1);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testSwap(void)
    {
        {
            acl::TSharePtr<CTestParent> loPtr1(new CTestParent);
            acl::TSharePtr<CTestParent> loPtr2(new CTestChild);
            
            loPtr1.Swap(loPtr2);
            
            ASSERT_MESSAGE(loPtr1 != NULL && loPtr1->Who() == 1);
            ASSERT_MESSAGE(loPtr2 != NULL && loPtr2->Who() == 0);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testReset(void)
    {
        {
            acl::TSharePtr<CTestParent> loPtr1(new CTestParent);
            
            loPtr1.Reset();
            
            ASSERT_MESSAGE(loPtr1 == NULL);
        }
        
        {
            acl::TSharePtr<CTestParent> loPtr1(new CTestParent);
            
            loPtr1.Reset(new CTestChild);
            
            ASSERT_MESSAGE(loPtr1 != NULL && loPtr1->Who() == 1);
        }
        
        {
            acl::TSharePtr<CTestParent> loPtr1(new CTestParent);
            
            loPtr1.Reset(new CTestChild);
            
            ASSERT_MESSAGE(loPtr1 != NULL && loPtr1->Who() == 1);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testGetUseCount(void)
    {
        {
            acl::TSharePtr<CTestParent> loPtr1(new CTestParent);
            
            ASSERT_MESSAGE(loPtr1.GetUseCount() == 1);
            
            acl::TSharePtr<CTestParent> loPtr2 = loPtr1;
                
            ASSERT_MESSAGE(loPtr1.GetUseCount() == 2);
            
            loPtr1.Reset();
            
            ASSERT_MESSAGE(loPtr1 == NULL);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }

    void testGet(void)
    {
        {
            CTestParent* lpoParent = new CTestParent;
            acl::TSharePtr<CTestParent> loPtr1(lpoParent);
            
            CTestParent* lpoParent1 = loPtr1.Get();
            
            ASSERT_MESSAGE(lpoParent1 == lpoParent);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testOperatorNON(void)
    {
        {
            acl::TSharePtr<CTestParent> loPtr1(new CTestParent);
            acl::TSharePtr<CTestParent> loPtr2;
            
            ASSERT_MESSAGE(!loPtr1 == true);
            ASSERT_MESSAGE(!loPtr2 == false);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testPointer(void)
    {
        struct CTraitPointer
        {
            void DoPointer(acl::TSharePtr<CTestParent>& aoPtr)
            {
                ASSERT_MESSAGE(aoPtr->Who() == 0);
            }
            
            void DoPointerByConst(acl::TSharePtr<CTestParent> const& aoPtr)
            {
                ASSERT_MESSAGE(aoPtr->Who() == 0);
            }
        };
        
        {
            CTraitPointer loTraitPointer;
            acl::TSharePtr<CTestParent> loPtr1(new CTestParent);
            
            loTraitPointer.DoPointer(loPtr1);
            loTraitPointer.DoPointerByConst(loPtr1);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }

    void testReference(void)
    {
        struct CTraitReference
        {
            void DoReference(acl::TSharePtr<CTestParent>& aoPtr)
            {
                ASSERT_MESSAGE((*aoPtr).Who() == 0);
            }
            
            void DoReferenceByConst(acl::TSharePtr<CTestParent> const& aoPtr)
            {
                ASSERT_MESSAGE((*aoPtr).Who() == 0);
            }
        };
        
        {
            CTraitReference loTraitReference;
            acl::TSharePtr<CTestParent> loPtr1(new CTestParent);
            
            loTraitReference.DoReference(loPtr1);
            loTraitReference.DoReferenceByConst(loPtr1);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testCompare(void)
    {     
        {
            acl::TSharePtr<CTestParent> loPtr1(new CTestParent);
            acl::TSharePtr<CTestParent> loPtr2;
            
            ASSERT_MESSAGE(loPtr1 != NULL);
            ASSERT_MESSAGE(!(loPtr1 == NULL) );
            ASSERT_MESSAGE(!(loPtr1 < NULL) );
            ASSERT_MESSAGE((loPtr1 > NULL) );
            ASSERT_MESSAGE((loPtr1 >= NULL) );
            ASSERT_MESSAGE(!(loPtr1 <= NULL) );
            
            ASSERT_MESSAGE(loPtr1 != loPtr2);
            ASSERT_MESSAGE(!(loPtr1 == loPtr2) );
            ASSERT_MESSAGE(!(loPtr1 < loPtr2) );
            ASSERT_MESSAGE((loPtr1 > loPtr2) );
            ASSERT_MESSAGE((loPtr1 >= loPtr2) );
            ASSERT_MESSAGE(!(loPtr1 <= loPtr2) );
        }
          
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testPerformance1(void)
    {
        PRINT_TITLE_2(__func__);
        {
            acl::TSharePtr<CTestCounter> loPtr1(new CTestCounter);
                
            START_LOOP(1000000)
            {
                acl::TSharePtr<CTestCounter> loPtr2(loPtr1);
            }
            END_LOOP();
            
            START_LOOP(1000000)
            {
                acl::TSharePtr<CTestCounter> loPtr2 = loPtr1;
            }
            END_LOOP();
            
            acl::TSharePtr<CTestCounter> loPtr3;
            START_LOOP(1000000)
            {
                loPtr3 = loPtr1;
            }
            END_LOOP();
        }
        
        ASSERT_MESSAGE(CTestCounter::GetCount() == 0);
    }
    
    void testPerformance2(void)
    {
        PRINT_TITLE_2(__func__);
        {
            acl::TSharePtr<CTestCounter, acl::CLock> loPtr1(new CTestCounter);
                
            START_LOOP(1000000)
            {
                acl::TSharePtr<CTestCounter, acl::CLock> loPtr2(loPtr1);
            }
            END_LOOP();
            
            START_LOOP(1000000)
            {
                acl::TSharePtr<CTestCounter, acl::CLock> loPtr2 = loPtr1;
            }
            END_LOOP();
            
            acl::TSharePtr<CTestCounter, acl::CLock> loPtr3;
            START_LOOP(1000000)
            {
                loPtr3 = loPtr1;
            }
            END_LOOP();
        }
        
        ASSERT_MESSAGE(CTestCounter::GetCount() == 0);
    }
    
    void Func1( acl::TSharePtr<CTestCounter, acl::CLock>& aoPtr )
    {
        aoPtr = new CTestCounter;
    }

    acl::TSharePtr<CTestCounter, acl::CLock> Func2(void)
    {
        acl::TSharePtr<CTestCounter, acl::CLock> loPtr(new CTestCounter);
            
        return loPtr;
    }
    
    void Func3(void)
    {
        acl::TSharePtr<CTestCounter, acl::CLock> aoPtr(new CTestCounter);
    }
    
    void testPerformance3(void)
    {
        PRINT_TITLE_2(__func__);
        {
            START_LOOP(1000000)
            {
                acl::TSharePtr<CTestCounter, acl::CLock> loPtr2;
                
                Func1(loPtr2);
            }
            END_LOOP();

            START_LOOP(1000000)
            {
                acl::TSharePtr<CTestCounter, acl::CLock> loPtr2 = Func2();
                
                loPtr2->GetCount();
            }
            END_LOOP();

            START_LOOP(1000000)
            {
                Func2();
            }
            END_LOOP();

            START_LOOP(1000000)
            {
                Func3();
            }
            END_LOOP();
        }
        
        ASSERT_MESSAGE(CTestCounter::GetCount() == 0);
    }
};

//////////////////////////////////////////////////////////////////////////////////////
class CTestAclShareArray: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclShareArray);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testAssign);
    CPPUNIT_TEST(testSwap);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST(testGetUseCount);
    CPPUNIT_TEST(testGet);
    CPPUNIT_TEST(testOperatorNON);
    CPPUNIT_TEST(testCompare);
    CPPUNIT_TEST(testPerformance1);
    CPPUNIT_TEST(testPerformance2);
    CPPUNIT_TEST(testPerformance3);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void){}
    void tearDown(void){}
    
    void testConstructor(void)
    {
        {
            CTestParent* lpoParent = new CTestParent[1];
            acl::TShareArray<CTestParent> loPtr1(lpoParent);
            acl::TShareArray<CTestParent> loPtr2(loPtr1);
            
            ASSERT_MESSAGE(loPtr1 == lpoParent);
            ASSERT_MESSAGE(loPtr1 == lpoParent);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testAssign(void)
    {
        {
            CTestParent* lpoParent = new CTestParent[1];
            acl::TShareArray<CTestParent> loPtr1(lpoParent);
            acl::TShareArray<CTestParent> loPtr2 = loPtr1;
            acl::TShareArray<CTestParent> loPtr3;
            
            CTestParent* lpoParent1 = new CTestParent[1];
            loPtr3 = lpoParent1;
            
            ASSERT_MESSAGE(loPtr1 == lpoParent);
            ASSERT_MESSAGE(loPtr2 == lpoParent);
            ASSERT_MESSAGE(loPtr3 == lpoParent1);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testSwap(void)
    {
        {
            CTestParent* lpoParent1 = new CTestParent[1];
            CTestParent* lpoParent2 = new CTestParent[1];
            acl::TShareArray<CTestParent> loPtr1(lpoParent1);
            acl::TShareArray<CTestParent> loPtr2(lpoParent2);
            
            loPtr1.Swap(loPtr2);
            
            ASSERT_MESSAGE(loPtr1 == lpoParent2);
            ASSERT_MESSAGE(loPtr2 == lpoParent1);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testReset(void)
    {
        {
            acl::TShareArray<CTestParent> loPtr1(new CTestParent[1]);
            
            loPtr1.Reset();
            
            ASSERT_MESSAGE(loPtr1 == NULL);
        }
        
        {
            acl::TShareArray<CTestParent> loPtr1(new CTestParent[1]);
            
            CTestParent* lpoParent = new CTestParent[1];
            loPtr1.Reset(lpoParent);
            
            ASSERT_MESSAGE(loPtr1 == lpoParent);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testGetUseCount(void)
    {
        {
            acl::TShareArray<CTestParent> loPtr1(new CTestParent[1]);
            
            ASSERT_MESSAGE(loPtr1.GetUseCount() == 1);
            
            acl::TShareArray<CTestParent> loPtr2 = loPtr1;
                
            ASSERT_MESSAGE(loPtr1.GetUseCount() == 2);
            
            loPtr1.Reset();
            
            ASSERT_MESSAGE(loPtr1 == NULL);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }

    void testGet(void)
    {
        {
            CTestParent* lpoParent = new CTestParent[1];
            acl::TShareArray<CTestParent> loPtr1(lpoParent);
            
            CTestParent* lpoParent1 = loPtr1.Get();
            
            ASSERT_MESSAGE(lpoParent1 == lpoParent);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testOperatorNON(void)
    {
        {
            acl::TShareArray<CTestParent> loPtr1(new CTestParent[1]);
            acl::TShareArray<CTestParent> loPtr2;
            
            ASSERT_MESSAGE(!loPtr1 == true);
            ASSERT_MESSAGE(!loPtr2 == false);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testOperatorArray(void)
    {
        {
            CTestParent* lpoParant = new CTestParent[10];
            acl::TShareArray<CTestParent> loPtr1(lpoParant);
            
            for (apl_int_t n = 0; n < 10; n++)
            {
                ASSERT_MESSAGE(&(loPtr1[n]) == &(lpoParant[n]) );
            }
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }

    void testCompare(void)
    {     
        {
            acl::TShareArray<CTestParent> loPtr1(new CTestParent[1]);
            acl::TShareArray<CTestParent> loPtr2;
            
            ASSERT_MESSAGE(loPtr1 != NULL);
            ASSERT_MESSAGE(!(loPtr1 == NULL) );
            ASSERT_MESSAGE(NULL != loPtr1);
            ASSERT_MESSAGE(!(NULL == loPtr1) );
            
            ASSERT_MESSAGE(loPtr1 != loPtr2);
            ASSERT_MESSAGE(!(loPtr1 == loPtr2) );
        }
          
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testPerformance1(void)
    {
        PRINT_TITLE_2(__func__);
        {
            acl::TShareArray<CTestCounter> loPtr1(new CTestCounter[1]);
                
            START_LOOP(1000000)
            {
                acl::TShareArray<CTestCounter> loPtr2(loPtr1);
            }
            END_LOOP();
            
            START_LOOP(1000000)
            {
                acl::TShareArray<CTestCounter> loPtr2 = loPtr1;
            }
            END_LOOP();
            
            acl::TShareArray<CTestCounter> loPtr3;
            START_LOOP(1000000)
            {
                loPtr3 = loPtr1;
            }
            END_LOOP();
        }
        
        ASSERT_MESSAGE(CTestCounter::GetCount() == 0);
    }
    
    void testPerformance2(void)
    {
        PRINT_TITLE_2(__func__);
        {
            acl::TShareArray<CTestCounter, acl::CLock> loPtr1(new CTestCounter[1]);
                
            START_LOOP(1000000)
            {
                acl::TShareArray<CTestCounter, acl::CLock> loPtr2(loPtr1);
            }
            END_LOOP();
            
            START_LOOP(1000000)
            {
                acl::TShareArray<CTestCounter, acl::CLock> loPtr2 = loPtr1;
            }
            END_LOOP();
            
            acl::TShareArray<CTestCounter, acl::CLock> loPtr3;
            START_LOOP(1000000)
            {
                loPtr3 = loPtr1;
            }
            END_LOOP();
        }
        
        ASSERT_MESSAGE(CTestCounter::GetCount() == 0);
    }
    
    void Func1( acl::TShareArray<CTestCounter, acl::CLock>& aoPtr )
    {
        aoPtr = new CTestCounter[1];
    }

    acl::TShareArray<CTestCounter, acl::CLock> Func2(void)
    {
        acl::TShareArray<CTestCounter, acl::CLock> loPtr(new CTestCounter[1]);
            
        return loPtr;
    }
    
    void Func3(void)
    {
        acl::TShareArray<CTestCounter, acl::CLock> aoPtr(new CTestCounter[1]);
    }
    
    void testPerformance3(void)
    {
        PRINT_TITLE_2(__func__);
        {
            START_LOOP(1000000)
            {
                acl::TShareArray<CTestCounter, acl::CLock> loPtr2;
                
                Func1(loPtr2);
            }
            END_LOOP();

            START_LOOP(1000000)
            {
                acl::TShareArray<CTestCounter, acl::CLock> loPtr2 = Func2();
                
                loPtr2[0].GetCount();
            }
            END_LOOP();

            START_LOOP(1000000)
            {
                Func2();
            }
            END_LOOP();

            START_LOOP(1000000)
            {
                Func3();
            }
            END_LOOP();
        }
        
        ASSERT_MESSAGE(CTestCounter::GetCount() == 0);
    }
};


//////////////////////////////////////////////////////////////////////////////////////
class CTestAclScopePtr: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclScopePtr);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testSwap);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST(testGet);
    CPPUNIT_TEST(testOperatorNON);
    CPPUNIT_TEST(testPointer);
    CPPUNIT_TEST(testReference);
    CPPUNIT_TEST(testCompare);
    CPPUNIT_TEST(testPerformance1);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void){}
    void tearDown(void){}
    
    void testConstructor(void)
    {
        {
            CTestParent* lpoParent1 = new CTestParent;
            CTestParent* lpoParent2 = new CTestParent;
            acl::TScopePtr<CTestParent> loPtr1(lpoParent1);
            acl::TScopePtr<CTestParent> loPtr2(lpoParent2);
            
            ASSERT_MESSAGE(loPtr1 == lpoParent1);
            ASSERT_MESSAGE(loPtr2 == lpoParent2);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }

    void testSwap(void)
    {
        {
            CTestParent* lpoParent1 = new CTestParent;
            CTestParent* lpoParent2 = new CTestParent;
            acl::TScopePtr<CTestParent> loPtr1(lpoParent1);
            acl::TScopePtr<CTestParent> loPtr2(lpoParent2);
            
            loPtr1.Swap(loPtr2);
            
            ASSERT_MESSAGE(loPtr1 == lpoParent2);
            ASSERT_MESSAGE(loPtr2 == lpoParent1);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testReset(void)
    {
        {
            acl::TScopePtr<CTestParent> loPtr1(new CTestParent);
            
            loPtr1.Reset();
            
            ASSERT_MESSAGE(loPtr1 == NULL);
        }
        
        {
            acl::TScopePtr<CTestParent> loPtr1(new CTestParent);
            
            CTestParent* lpoParent = new CTestParent;
            loPtr1.Reset(lpoParent);
            
            ASSERT_MESSAGE(loPtr1 == lpoParent);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testGet(void)
    {
        {
            CTestParent* lpoParent = new CTestParent[1];
            acl::TScopePtr<CTestParent> loPtr1(lpoParent);
            
            CTestParent* lpoParent1 = loPtr1.Get();
            
            ASSERT_MESSAGE(lpoParent1 == lpoParent);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testOperatorNON(void)
    {
        {
            acl::TScopePtr<CTestParent> loPtr1(new CTestParent[1]);
            acl::TScopePtr<CTestParent> loPtr2;
            
            ASSERT_MESSAGE(!loPtr1 == true);
            ASSERT_MESSAGE(!loPtr2 == false);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testPointer(void)
    {
        struct CTraitPointer
        {
            void DoPointer(acl::TScopePtr<CTestParent>& aoPtr)
            {
                ASSERT_MESSAGE(aoPtr->Who() == 0);
            }
            
            void DoPointerByConst(acl::TScopePtr<CTestParent> const& aoPtr)
            {
                ASSERT_MESSAGE(aoPtr->Who() == 0);
            }
        };
        
        {
            CTraitPointer loTraitPointer;
            acl::TScopePtr<CTestParent> loPtr1(new CTestParent);
            
            loTraitPointer.DoPointer(loPtr1);
            loTraitPointer.DoPointerByConst(loPtr1);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }

    void testReference(void)
    {
        struct CTraitReference
        {
            void DoReference(acl::TScopePtr<CTestParent>& aoPtr)
            {
                ASSERT_MESSAGE( (*aoPtr).Who() == 0);
            }
            
            void DoReferenceByConst(acl::TScopePtr<CTestParent> const& aoPtr)
            {
                ASSERT_MESSAGE( (*aoPtr).Who() == 0);
            }
        };
        
        {
            CTraitReference loTraitReference;
            acl::TScopePtr<CTestParent> loPtr1(new CTestParent);
            
            loTraitReference.DoReference(loPtr1);
            loTraitReference.DoReferenceByConst(loPtr1);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testCompare(void)
    {     
        {
            acl::TScopePtr<CTestParent> loPtr1(new CTestParent);
            acl::TScopePtr<CTestParent> loPtr2;
            
            ASSERT_MESSAGE(loPtr1 != NULL);
            ASSERT_MESSAGE(!(loPtr1 == NULL) );
            ASSERT_MESSAGE(!(loPtr1 < NULL) );
            ASSERT_MESSAGE((loPtr1 > NULL) );
            ASSERT_MESSAGE((loPtr1 >= NULL) );
            ASSERT_MESSAGE(!(loPtr1 <= NULL) );
            
            ASSERT_MESSAGE(loPtr1 != loPtr2);
            ASSERT_MESSAGE(!(loPtr1 == loPtr2) );
            ASSERT_MESSAGE(!(loPtr1 < loPtr2) );
            ASSERT_MESSAGE((loPtr1 > loPtr2) );
            ASSERT_MESSAGE((loPtr1 >= loPtr2) );
            ASSERT_MESSAGE(!(loPtr1 <= loPtr2) );
        }
          
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testPerformance1(void)
    {
        PRINT_TITLE_2(__func__);
        {  
            START_LOOP(1000000)
            {
                acl::TScopePtr<CTestCounter> loPtr1(new CTestCounter);
            }
            END_LOOP();
        }
        
        ASSERT_MESSAGE(CTestCounter::GetCount() == 0);
    }
};

//////////////////////////////////////////////////////////////////////////////////////
class CTestAclScopeArray: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclScopeArray);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testSwap);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST(testGet);
    CPPUNIT_TEST(testOperatorNON);
    CPPUNIT_TEST(testCompare);
    CPPUNIT_TEST(testPerformance1);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void){}
    void tearDown(void){}
    
    void testConstructor(void)
    {
        {
            CTestParent* lpoParent = new CTestParent[1];
            acl::TScopeArray<CTestParent> loPtr1(lpoParent);

            ASSERT_MESSAGE(loPtr1 == lpoParent);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testSwap(void)
    {
        {
            CTestParent* lpoParent1 = new CTestParent[1];
            CTestParent* lpoParent2 = new CTestParent[1];
            acl::TScopeArray<CTestParent> loPtr1(lpoParent1);
            acl::TScopeArray<CTestParent> loPtr2(lpoParent2);
            
            loPtr1.Swap(loPtr2);
            
            ASSERT_MESSAGE(loPtr1 == lpoParent2);
            ASSERT_MESSAGE(loPtr2 == lpoParent1);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testReset(void)
    {
        {
            acl::TScopeArray<CTestParent> loPtr1(new CTestParent[1]);
            
            loPtr1.Reset();
            
            ASSERT_MESSAGE(loPtr1 == NULL);
        }
        
        {
            acl::TScopeArray<CTestParent> loPtr1(new CTestParent[1]);
            
            CTestParent* lpoParent = new CTestParent[1];
            loPtr1.Reset(lpoParent);
            
            ASSERT_MESSAGE(loPtr1 == lpoParent);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }

    void testGet(void)
    {
        {
            CTestParent* lpoParent = new CTestParent[1];
            acl::TShareArray<CTestParent> loPtr1(lpoParent);
            
            CTestParent* lpoParent1 = loPtr1.Get();
            
            ASSERT_MESSAGE(lpoParent1 == lpoParent);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testOperatorNON(void)
    {
        {
            acl::TScopeArray<CTestParent> loPtr1(new CTestParent[1]);
            acl::TScopeArray<CTestParent> loPtr2;
            
            ASSERT_MESSAGE(!loPtr1 == true);
            ASSERT_MESSAGE(!loPtr2 == false);
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testOperatorArray(void)
    {
        {
            CTestParent* lpoParant = new CTestParent[10];
            acl::TScopeArray<CTestParent> loPtr1(lpoParant);
            
            for (apl_int_t n = 0; n < 10; n++)
            {
                ASSERT_MESSAGE(&(loPtr1[n]) == &(lpoParant[n]) );
            }
        }
        
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }

    void testCompare(void)
    {     
        {
            acl::TScopeArray<CTestParent> loPtr1(new CTestParent[1]);
            acl::TScopeArray<CTestParent> loPtr2;
            
            ASSERT_MESSAGE(loPtr1 != NULL);
            ASSERT_MESSAGE(!(loPtr1 == NULL) );
            ASSERT_MESSAGE(NULL != loPtr1);
            ASSERT_MESSAGE(!(NULL == loPtr1) );
            
            ASSERT_MESSAGE(loPtr1 != loPtr2);
            ASSERT_MESSAGE(!(loPtr1 == loPtr2) );
        }
          
        ASSERT_MESSAGE(CTestParent::GetCount() == 0);
    }
    
    void testPerformance1(void)
    {
        PRINT_TITLE_2(__func__);
        {
            START_LOOP(1000000)
            {
                acl::TShareArray<CTestCounter> loPtr1(new CTestCounter[1]);
            }
            END_LOOP();
        }
        
        ASSERT_MESSAGE(CTestCounter::GetCount() == 0);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclShartPtr);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclShareArray);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclScopePtr);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclScopeArray);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
