#include "Test.h"
#include "acl/Xml.h"

using namespace acl;

class CTestAclXml: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclXml);
    CPPUNIT_TEST( testAdd );
    CPPUNIT_TEST( testXml );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}
    void testAdd(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CXmlElement::IteratorType loElemIter;
        CXmlElement::IteratorType loElemIterRe;
        CXmlAttribute::IteratorType loAttrIter;
        CXmlText::IteratorType loTextIter;
        CXmlClear::IteratorType loClearIter;
        CXmlNode::IteratorType loIter;
        apl_int_t liN= 0;
        CXmlRootElement loRootElement;
        
        //case
        loRootElement.SetName("root");
        if ( apl_memcmp( loRootElement.GetName(), "root", 4) != 0 )
        {
            CPPUNIT_FAIL("Element GetName fail");
        }
        for(liN= 0; liN<10; liN++)
        {
            //Element
            loElemIter= loRootElement.AddElement("name");
            if ( loElemIter == loRootElement.End() )
            {
                CPPUNIT_FAIL("Add Element fail");
            }
            if ( apl_memcmp( loElemIter->GetName(), "name", 4) != 0 )
            {
                CPPUNIT_FAIL("Element GetName fail");
            }
            loElemIter->SetName("ElemName");
            if ( apl_memcmp( loElemIter->GetName(), "ElemName", 8) != 0 )
            {
                CPPUNIT_FAIL("Element GetName fail");
            }
            CPPUNIT_ASSERT_MESSAGE("Element fail", loElemIter->IsElement() );
            loElemIterRe= loElemIter->AddElement("secName1");
            if ( loElemIterRe == loElemIter->End() )
            {
                CPPUNIT_FAIL("Add Element fail");
            }

            // Attribute
            loAttrIter= loElemIter->AddAttribute("Attr", "Attri");
            if ( loAttrIter == loElemIter->End() )
            {
                CPPUNIT_FAIL("AddAttribute fail");
            }
            if ( apl_memcmp( loAttrIter->GetName(), "Attr", 4) != 0 )
            {
                CPPUNIT_FAIL("Attribute GetName fail");
            }
            loAttrIter->SetName("AttrName");
            if ( apl_memcmp( loAttrIter->GetName(), "AttrName", 8) != 0 )
            {
                CPPUNIT_FAIL("Attribute GetName fail");
            }
            if ( apl_memcmp( loAttrIter->GetValue(), "Attri", 5) != 0 )
            {
                CPPUNIT_FAIL("Attribute GetValue fail");
            }
            loAttrIter->SetValue("AttrValue");
            if ( apl_memcmp( loAttrIter->GetValue(), "AttrValue", 9) != 0 )
            {
                CPPUNIT_FAIL("Attribute GetValue fail");
            }
            CPPUNIT_ASSERT_MESSAGE("Attribute fail", loAttrIter->IsAttribute() );
            loAttrIter= loElemIter->AddAttribute("Attrname1", "Attrvalue1");
            if ( loAttrIter == loElemIter->End() )
            {
                CPPUNIT_FAIL("AddAttribute fail");
            }
            CPPUNIT_ASSERT_MESSAGE("Attribute fail", loAttrIter->IsAttribute() );

            //Text
            loTextIter= loElemIter->AddText("Text");
            if ( loTextIter == loElemIter->End() )
            {
                CPPUNIT_FAIL("AddText fail");
            }
            CPPUNIT_ASSERT_MESSAGE("Text fail", loTextIter->IsText() );
            if ( apl_memcmp(loTextIter->GetValue(), "Text", 4) != 0 )
            {
                CPPUNIT_FAIL("Text fail");
            }
            loTextIter->SetValue("ValueText");
            if ( apl_memcmp(loTextIter->GetValue(), "ValueText", 9) != 0 )
            {
                CPPUNIT_FAIL("Text fail");
            }

            //Clear
            loClearIter= loElemIter->AddClear(CXmlClear::TAG_NOTE, "Clear");
            if ( loClearIter == loElemIter->End() )
            {
                CPPUNIT_FAIL("AddClear fail");
            }
            CPPUNIT_ASSERT_MESSAGE("Clear fail", loClearIter->IsClear() );
            if ( apl_memcmp(loClearIter->GetValue(), "Clear", 5) != 0 )
            {
                CPPUNIT_FAIL("Clear GetValue fail");
            }
            if ( loClearIter->GetType() != CXmlClear::TAG_NOTE )
            {
                CPPUNIT_FAIL("Clear getType fail");
            }
            loClearIter->SetValue("ValueClear");
            if ( apl_memcmp(loClearIter->GetValue(), "ValueClear", 10) != 0 )
            {
                CPPUNIT_FAIL("Clear GetValue fail");
            }
            loClearIter= loElemIter->AddClear(CXmlClear::TAG_PRE, "ValueClear");
            if ( loClearIter == loElemIter->End() )
            {
                CPPUNIT_FAIL("AddClear fail");
            }
            CPPUNIT_ASSERT_MESSAGE("Clear fail", loClearIter->IsClear() );
            if ( loClearIter->GetType() != CXmlClear::TAG_PRE )
            {
                CPPUNIT_FAIL("Clear getType fail");
            }
        }
    }
    void testXml(void)
    {
        //start environment
        CreateXml();
        //case
        testParseXml(); 
        testFindXml();

        //end environment
    }
    void CreateXml(void)
    {
        CXmlElement::IteratorType loElemIter;
        CXmlElement::IteratorType loElemIterRe;
        ciElem= 0;
        ciElemParse= 0;
        ciAttr= 0;
        ciAttrParse= 0;
        ciText= 0;
        ciTextParse= 0;
        ciClear= 0;
        ciClearParse= 0;
        coRootElement.SetName("root");
        for(apl_int_t liN= 0; liN<10; liN++)
        {
            coElemIter= coRootElement.AddElement("ElemName");
            ++ciElem;
            //For find
            loElemIterRe= coElemIter->AddElement("secName1");
            loElemIterRe->AddAttribute("Attr", "Attri");
            loElemIterRe->AddAttribute("Attr1", "Attri1");
            loElemIterRe->AddAttribute("Attr2", "Attri2");
            loElemIterRe->AddElement("trdName");
            loElemIterRe->AddElement("trdName");
            loElemIterRe->AddElement("trdName");
            loElemIterRe= coElemIter->AddElement("secName2");
            loElemIterRe->AddAttribute("Attr", "Attri");
            loElemIterRe->AddAttribute("Attr1", "Attri1");
            loElemIterRe->AddAttribute("Attr2", "Attri2");
            loElemIterRe->AddElement("trdName");
            loElemIterRe->AddElement("trdName");
            loElemIterRe->AddElement("trdName");
            loElemIterRe= coElemIter->AddElement("secName3");
            loElemIterRe->AddAttribute("Attr", "Attri");
            loElemIterRe->AddAttribute("Attr1", "Attri1");
            loElemIterRe->AddAttribute("Attr2", "Attri2");
            loElemIterRe->AddElement("trdName");
            loElemIterRe->AddElement("trdName");
            loElemIterRe->AddElement("trdName");

            coAttrIter= coElemIter->AddAttribute("AttrName", "AttrValue");
            ++ciAttr;
            coAttrIter->SetValue("AttrValue");
            coAttrIter= coElemIter->AddAttribute("Attrname1", "Attrvalue1");
            ++ciAttr;
            CPPUNIT_ASSERT_MESSAGE("Attribute fail", coAttrIter->IsAttribute() );

            coTextIter= coElemIter->AddText("ValueText");
            ++ciText;

            coClearIter= coElemIter->AddClear(CXmlClear::TAG_NOTE, "NoteClear");
            ++ciClear;
            coClearIter= coElemIter->AddClear(CXmlClear::TAG_PRE, "PREClear");
            ++ciClear;
        }
    }
    void testParseXml(void)
    {
        PRINT_TITLE_2(__func__);
        CXmlElement::IteratorType loElemIter;
        coCreator.Create(coRootElement, CXmlCreator::OPT_DEFFORMAT);
//        printf("%s\n", coCreator.GetXml() );
        if ( coParse.Parse( coCreator.GetXml(), coRootElement ) != 0 )
        {
            CPPUNIT_FAIL("xml parse fail");
        }
        loElemIter= coRootElement.BeginElement(); //get root
        coElemIter= loElemIter->BeginElement(); 
        for( ; coElemIter != loElemIter->End(); ++coElemIter)
        {
            ++ciElemParse;
            coAttrIter= coElemIter->BeginAttribute();
            for( ; coAttrIter != coElemIter->End(); ++coAttrIter)
            {
                ++ciAttrParse;
            }
            coTextIter= coElemIter->BeginText();
            for( ; coTextIter != coElemIter->End(); ++coTextIter)
            {
                ++ciTextParse;
            }
            coClearIter= coElemIter->BeginClear();
            for( ; coClearIter != coElemIter->End(); ++coClearIter)
            {
                ++ciClearParse;
            }
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Element parse fail", ciElem, ciElemParse);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Attribute parse fail", ciAttr, ciAttrParse);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Text parse fail", ciText, ciTextParse);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Clear parse fail", ciClear, ciClearParse);

        ciElemParse= 0;
        ciAttrParse= 0;
        ciTextParse= 0;
        ciClearParse= 0;
        loElemIter= coRootElement.BeginElement(); //get root
        CXmlNode::IteratorType loIter;
        loIter= loElemIter->Begin();
        coElemIter= loElemIter->BeginElement();
        for( ; loIter != loElemIter->End(); ++loIter)
        {
            if ( loIter->IsElement() )
            {
                ++ciElemParse;
                CXmlNode::IteratorType loNodeIter;
                for(loNodeIter= coElemIter->Begin() ; loNodeIter != coElemIter->End(); ++loNodeIter)
                {
                    if ( loNodeIter->IsAttribute() )
                    {
                        ++ciAttrParse;
                    }
                    if ( loNodeIter->IsText() )
                    {
                        ++ciTextParse;
                    }
                    if ( loNodeIter->IsClear() )
                    {
                        ++ciClearParse;
                    }
                }
            }
            ++coElemIter;
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Element parse fail", ciElem, ciElemParse);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Attribute parse fail", ciAttr, ciAttrParse);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Text parse fail", ciText, ciTextParse);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Clear parse fail", ciClear, ciClearParse);

    }

    void testFindXml(void)
    {
        PRINT_TITLE_2(__func__);
        //Find Elem
        CXmlAttribute laoAttr[10];
        CXmlElement laoElem[10];
        apl_size_t  luRet= 0;
        coElemIter= coRootElement.FindElement("root/ElemXie");
        if ( coElemIter != coRootElement.End() )
        {
            CPPUNIT_FAIL("xml find elem fail");
        }
        coElemIter= coRootElement.FindElement("root/ElemName/secName1");
        if ( coElemIter == coRootElement.End() )
        {
            CPPUNIT_FAIL("xml find elem fail");
        }
        if ( apl_memcmp( coElemIter->GetName(), "secName1", 8) != 0 )
        {
            CPPUNIT_FAIL("Element GetName fail");
        }
        luRet= coRootElement.FindElement("root/ElemName/secName2/trdName1", laoElem, 10);
        if ( luRet != 0 )
        {
            CPPUNIT_FAIL("xml find elem fail");
        }
        luRet= coRootElement.FindElement("root/ElemName/secName2/trdName", laoElem, 2);
        if ( luRet != 2 )
        {
            CPPUNIT_FAIL("xml find elem fail");
        }
        luRet= coRootElement.FindElement("root/ElemName/secName2", laoElem, 10);
        if ( luRet != 1 )
        {
            CPPUNIT_FAIL("xml find elem fail");
        }
        if ( apl_memcmp( laoElem[0].GetName(), "secName2", 8) != 0 )
        {
            CPPUNIT_FAIL("Element GetName fail");
        }
        coElemIter= laoElem[0].FindElement("trdName");
        if ( coElemIter == coRootElement.End() )
        {
            CPPUNIT_FAIL("xml find elem fail");
        }
        luRet= laoElem[0].FindElement("trdName", laoElem, 6);
        if ( luRet != 3 )
        {
            CPPUNIT_FAIL("xml find elem fail");
        }

        //Find Attribute
        coElemIter= coRootElement.FindElement("root/ElemName/secName1");
        coAttrIter= coElemIter->FindAttribute("Attribute");
        if ( coAttrIter != coElemIter->End() )
        {
            CPPUNIT_FAIL("find attribute fail");
        }
        coAttrIter= coElemIter->FindAttribute("Attr");
        if ( coAttrIter == coElemIter->End() )
        {
            CPPUNIT_FAIL("find attribute fail");
        }
        if ( apl_memcmp( coAttrIter->GetName(), "Attr", 4) != 0 )
        {
            CPPUNIT_FAIL("Attribute GetName fail");
        }
        luRet= coElemIter->FindAttribute("Attr", laoAttr, 10);
        if ( luRet != 1)
        {
            CPPUNIT_FAIL("find attribute fail");
        }
        if ( apl_memcmp( laoAttr[0].GetName(), "Attr", 4) != 0 )
        {
            CPPUNIT_FAIL("Attribute GetName fail");
        }
        luRet= coElemIter->FindAttribute("Attribute", laoAttr+1, 9);
        if ( luRet != 0)
        {
            CPPUNIT_FAIL("find attribute fail");
        }
        coAttrIter= coRootElement.FindAttribute("root/ElemName/secName1/Attribute");
        if ( coAttrIter != coRootElement.End() )
        {
            CPPUNIT_FAIL("find attribute fail");
        }
        luRet= coRootElement.FindAttribute("root/ElemName/secName1/Attribute", laoAttr, 10);
        if ( luRet != 0 )
        {
            CPPUNIT_FAIL("find attribute fail");
        }
        coAttrIter= coRootElement.FindAttribute("root/ElemName/secName1/Attr1");
        if ( coAttrIter == coRootElement.End() )
        {
            CPPUNIT_FAIL("find attribute fail");
        }
        luRet= coRootElement.FindAttribute("root/ElemName/secName1/Attr1", laoAttr, 10);
        if ( luRet != 1 )
        {
            CPPUNIT_FAIL("find attribute fail");
        }
        if ( apl_memcmp( laoAttr[0].GetName(), "Attr1", 5) != 0 )
        {
            CPPUNIT_FAIL("Attribute GetName fail");
        }    
    }
private:
    CXmlRootElement coRootElement;
    CXmlCreator coCreator;
    CXmlElement::IteratorType coElemIter;
    CXmlAttribute::IteratorType coAttrIter;
    CXmlText::IteratorType coTextIter;
    CXmlClear::IteratorType coClearIter;
    CXmlNode::IteratorType coIter;
    CXmlParser coParse;
    apl_int_t ciElem;
    apl_int_t ciElemParse;
    apl_int_t ciAttr;
    apl_int_t ciAttrParse;
    apl_int_t ciText;
    apl_int_t ciTextParse;
    apl_int_t ciClear;
    apl_int_t ciClearParse;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclXml);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

