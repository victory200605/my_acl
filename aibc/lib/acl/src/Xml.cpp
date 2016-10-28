
#include "acl/Xml.h"

ACL_NAMESPACE_START

CXmlBaseNodeIterator::CXmlBaseNodeIterator( xml::McbXMLElement* apoParent, xml::McbXMLNode* apoNode )
    : mpoParent(apoParent)
    , mpoNode(apoNode)
{
}

bool CXmlBaseNodeIterator::operator == ( CXmlBaseNodeIterator const& aoRhs )
{
    return this->mpoNode == aoRhs.mpoNode;
}

bool CXmlBaseNodeIterator::operator != ( CXmlBaseNodeIterator const& aoRhs )
{
    return this->mpoNode != aoRhs.mpoNode;
}

CXmlEndIterator::CXmlEndIterator(void)
    : CXmlBaseNodeIterator(NULL, NULL)
{
}

/////////////////////////////////////////////////////////////////////////////////////
CXmlNode::CXmlNode(void)
    : mpoNode(NULL)
{
}

xml::McbXMLNode* CXmlNode::GetNode(void)
{
    return this->mpoNode;
}
    
void CXmlNode::SetNode( xml::McbXMLNode* apoNode )
{
    this->mpoNode = apoNode;
}
    
bool CXmlNode::IsElement(void)
{
    return this->mpoNode == NULL ? false : this->mpoNode->miType == (apl_int_t)XML_ELEMENT;
}

bool CXmlNode::IsAttribute(void)
{
    return this->mpoNode == false ? false : this->mpoNode->miType == (apl_int_t)XML_ATTRIBUTE;
}

bool CXmlNode::IsText(void)
{
    return this->mpoNode == NULL ? false : this->mpoNode->miType == (apl_int_t)XML_TEXT;
}

bool CXmlNode::IsClear(void)
{
    return this->mpoNode == NULL ? false : this->mpoNode->miType == (apl_int_t)XML_CLEAR;
}
    
xml::McbXMLElement* CXmlNode::ToElement(void)
{
    return this->mpoNode == NULL ? NULL : this->mpoNode->mnNode.mpoElement;
}

xml::McbXMLAttribute* CXmlNode::ToAttribute(void)
{
    return this->mpoNode == NULL ? NULL : this->mpoNode->mnNode.mpoAttrib;
}

xml::McbXMLText* CXmlNode::ToText(void)
{
    return this->mpoNode == NULL ? NULL : this->mpoNode->mnNode.mpoText;
}

xml::McbXMLClear* CXmlNode::ToClear(void)
{
    return this->mpoNode == NULL ? NULL : this->mpoNode->mnNode.mpoClear;
}

/////////////////////////////////////////////////////////////////////////////////////
CXmlElement::CXmlElement(void)
{
}

void CXmlElement::SetName( const char* apcName )
{
    if (this->ToElement() != NULL)
    {
        char* lpTmp = this->ToElement()->mpcName;
        this->ToElement()->mpcName = apl_strdup(apcName);
        ACL_FREE(lpTmp);
    }
}

const char* CXmlElement::GetName()
{
    return this->ToElement()->mpcName;
}

CXmlNode::IteratorType CXmlElement::Begin()
{
    apl_size_t liN = 0;
    
    xml::McbXMLNode* lpoNode = xml::McbEnumNodes(this->ToElement(), &liN);
    
    return CXmlNode::IteratorType(this->ToElement(), lpoNode);
}

CXmlElement::IteratorType CXmlElement::BeginElement()
{
    apl_size_t liN = 0;
    
    while(true)
    {
        xml::McbXMLNode* lpoNode = xml::McbEnumNodes(this->ToElement(), &liN);
        if (lpoNode == NULL)
        {
            break;
        }
        else if (lpoNode->miType == (apl_int_t)XML_ELEMENT)
        {
            return CXmlElement::IteratorType(this->ToElement(), lpoNode);
        }
    }
    
    return CXmlElement::IteratorType(NULL, NULL);
}

CXmlAttribute::IteratorType CXmlElement::BeginAttribute()
{
    apl_size_t liN = 0;
    
    while(true)
    {
        xml::McbXMLNode* lpoNode = xml::McbEnumNodes(this->ToElement(), &liN);
        if (lpoNode == NULL)
        {
            break;
        }
        else if (lpoNode->miType == (apl_int_t)XML_ATTRIBUTE)
        {
            return CXmlAttribute::IteratorType(this->ToElement(), lpoNode);
        }
    }
    
    return CXmlAttribute::IteratorType(NULL, NULL);
}

CXmlText::IteratorType CXmlElement::BeginText()
{
    apl_size_t liN = 0;
    
    while(true)
    {
        xml::McbXMLNode* lpoNode = xml::McbEnumNodes(this->ToElement(), &liN);
        if (lpoNode == NULL)
        {
            break;
        }
        else if (lpoNode->miType == (apl_int_t)XML_TEXT)
        {
            return CXmlText::IteratorType(this->ToElement(), lpoNode);
        }
    }
    
    return CXmlText::IteratorType(NULL, NULL);
}

CXmlClear::IteratorType CXmlElement::BeginClear()
{
    apl_size_t liN = 0;
    
    while(true)
    {
        xml::McbXMLNode* lpoNode = xml::McbEnumNodes(this->ToElement(), &liN);
        if (lpoNode == NULL)
        {
            break;
        }
        else if (lpoNode->miType == (apl_int_t)XML_CLEAR)
        {
            return CXmlClear::IteratorType(this->ToElement(), lpoNode);
        }
    }
    
    return CXmlClear::IteratorType(NULL, NULL);
}

CXmlEndIterator CXmlElement::End()
{
    return CXmlEndIterator();
}

CXmlElement::IteratorType CXmlElement::FindElement( char const* apcPath )
{
    char const* lpcName = NULL;
    xml::McbXMLElement* lpoElement = FindRootElement(apcPath, &lpcName);
    
    if (lpoElement == NULL || lpcName == NULL)
    {
        return CXmlElement::IteratorType(NULL, NULL);
    }
    
    for ( apl_size_t liN = 0; liN < lpoElement->muCount; liN++ )
    {
        if (lpoElement->mpoEntries[liN].miType == (apl_int_t)XML_ELEMENT
            && apl_strcmp(lpoElement->mpoEntries[liN].mnNode.mpoElement->mpcName, lpcName) == 0 )
        {
            return CXmlElement::IteratorType(this->ToElement(), &lpoElement->mpoEntries[liN]);
        }
    }
    
    return CXmlElement::IteratorType(NULL, NULL);
}

apl_size_t CXmlElement::FindElement( char const* apcPath, CXmlElement aoElement[], apl_size_t aiCount )
{
    apl_size_t  liSize = 0;
    char const* lpcName = NULL;
    xml::McbXMLElement* lpoElement = FindRootElement(apcPath, &lpcName);
    
    if (lpoElement == NULL || lpcName == NULL)
    {
        return liSize;
    }
    
    for ( apl_size_t liN = 0; liN < lpoElement->muCount; liN++ )
    {
        if ( liSize < aiCount
            && lpoElement->mpoEntries[liN].miType == (apl_int_t)XML_ELEMENT
            && apl_strcmp(lpoElement->mpoEntries[liN].mnNode.mpoElement->mpcName, lpcName) == 0 )
        {
            aoElement[liSize++].SetNode(&lpoElement->mpoEntries[liN]);
        }
    }
    
    return liSize;
}

CXmlAttribute::IteratorType CXmlElement::FindAttribute( char const* apcPath )
{
    char const* lpcName = NULL;
    xml::McbXMLElement* lpoElement = FindRootElement(apcPath, &lpcName);
    
    if (lpoElement == NULL || lpcName == NULL)
    {
        return CXmlAttribute::IteratorType(NULL, NULL);
    }
    
    for ( apl_size_t liN = 0; liN < lpoElement->muCount; liN++ )
    {
        if (lpoElement->mpoEntries[liN].miType == (apl_int_t)XML_ATTRIBUTE
            && apl_strcmp(lpoElement->mpoEntries[liN].mnNode.mpoAttrib->mpcName, lpcName) == 0 )
        {
            return CXmlAttribute::IteratorType(this->ToElement(), &lpoElement->mpoEntries[liN]);
        }
    }
    
    return CXmlAttribute::IteratorType(NULL, NULL);
}

apl_size_t CXmlElement::FindAttribute( char const* apcPath, CXmlAttribute aoAttribute[], apl_size_t aiCount )
{
    apl_size_t  liSize = 0;
    char const* lpcName = NULL;
    xml::McbXMLElement* lpoElement = FindRootElement(apcPath, &lpcName);
    
    if (lpoElement == NULL || lpcName == NULL)
    {
        return liSize;
    }
    
    for ( apl_size_t liN = 0; liN < lpoElement->muCount; liN++ )
    {
        if ( liSize < aiCount
            && lpoElement->mpoEntries[liN].miType == (apl_int_t)XML_ATTRIBUTE
            && apl_strcmp(lpoElement->mpoEntries[liN].mnNode.mpoAttrib->mpcName, lpcName) == 0 )
        {
            aoAttribute[liSize++].SetNode(&lpoElement->mpoEntries[liN]);
        }
    }
    
    return liSize;
}

CXmlElement::IteratorType CXmlElement::AddElement( char const* apcName )
{
    return this->AddElement(apcName, 1);
}

CXmlElement::IteratorType CXmlElement::AddElement( char const* apcName, apl_size_t auGrowBy )
{
    char* lpcName = apl_strdup(apcName);
    xml::McbXMLElement* lpoElement = xml::McbAddElement(this->ToElement(), lpcName, 0, auGrowBy);
    if (lpoElement == NULL)
    {
        return CXmlElement::IteratorType(NULL, NULL);
    }
    else
    {
        return CXmlElement::IteratorType(this->ToElement(), &this->ToElement()->mpoEntries[this->ToElement()->muCount-1]);
    }
}

CXmlAttribute::IteratorType CXmlElement::AddAttribute( char const* apcName, char const* apcValue )
{
    return this->AddAttribute(apcName, apcValue, 1);
}

CXmlAttribute::IteratorType CXmlElement::AddAttribute( char const* apcName, char const* apcValue, apl_size_t auGrowBy )
{
    char* lpcName  = apl_strdup(apcName);
    char* lpcValue = apl_strdup(apcValue);
    xml::McbXMLAttribute* lpoAttr = xml::McbAddAttribute(this->ToElement(), lpcName, lpcValue, auGrowBy);
    if (lpoAttr == NULL)
    {
        return CXmlAttribute::IteratorType(NULL, NULL);
    }
    else
    {
        return CXmlAttribute::IteratorType(this->ToElement(), &this->ToElement()->mpoEntries[this->ToElement()->muCount-1]);
    }
}

CXmlText::IteratorType CXmlElement::AddText( char const* apcValue )
{
    char* lpcValue = apl_strdup(apcValue);
    xml::McbXMLText* lpoText = xml::McbAddText(this->ToElement(), lpcValue, 1);
    if (lpoText == NULL)
    {
        return CXmlText::IteratorType(NULL, NULL);
    }
    else
    {
        return CXmlText::IteratorType(this->ToElement(), &this->ToElement()->mpoEntries[this->ToElement()->muCount-1]);
    }
}

CXmlClear::IteratorType CXmlElement::AddClear( CXmlClear::ETagType aeType, char const* apcValue )
{
    char* lpcValue = apl_strdup(apcValue);
    xml::McbXMLClear* lpoClear = xml::McbAddClear(this->ToElement(), (xml::ETagType)aeType, lpcValue, 1);
    if (lpoClear == NULL)
    {
        return CXmlClear::IteratorType(NULL, NULL);
    }
    else
    {
        return CXmlClear::IteratorType(this->ToElement(), &this->ToElement()->mpoEntries[this->ToElement()->muCount-1]);
    }
}

xml::McbXMLElement* CXmlElement::FindRootElement( char const* apcPath, char const** appcName )
{
    bool        lbIsHit = false;
    char        lacNext[APL_NAME_MAX] = {0};
    char const* lpcSlash = NULL;
    xml::McbXMLElement* lpoCurrElement = this->ToElement();
        
    if (apcPath == NULL)
    {
        return NULL;
    }
    
    for ( lpcSlash = apl_strchr(apcPath, '/'); lpcSlash != NULL; lpcSlash = apl_strchr(apcPath, '/') )
    {
        if ( (apl_size_t)(lpcSlash - apcPath) < APL_NAME_MAX)
        {
            apl_memcpy(lacNext, apcPath, lpcSlash - apcPath);
            lacNext[lpcSlash - apcPath] = '\0';
        }
        else
        {
            return NULL;
        }
        
        lbIsHit = false;
        for ( apl_size_t liN = 0; liN < lpoCurrElement->muCount; liN++ )
        {
            if ( lpcSlash != NULL
                && lpoCurrElement->mpoEntries[liN].miType == (apl_int_t)XML_ELEMENT
                && apl_strcmp(lpoCurrElement->mpoEntries[liN].mnNode.mpoElement->mpcName, lacNext) == 0 )
            {
                lpoCurrElement = lpoCurrElement->mpoEntries[liN].mnNode.mpoElement;
                lbIsHit = true;
                break;
            }
        }
        
        if (!lbIsHit)
        {
            return NULL;
        }
        
        apcPath = lpcSlash + 1;
    }
    
    if (appcName != NULL)
    {
        *appcName = apcPath;
    }
    
    return lpoCurrElement;
}

////////////////////////////////////////////////////////////////////////////////////////
void CXmlAttribute::SetName( const char* apcName )
{
    if ( this->ToAttribute() != NULL )
    {
        char* lpTmp = this->ToAttribute()->mpcName;
        this->ToAttribute()->mpcName = apl_strdup(apcName);
        ACL_FREE(lpTmp);
    }
}
    
void CXmlAttribute::SetValue( const char* apcValue )
{
    if ( this->ToAttribute() != NULL )
    {
        char* lpTmp = this->ToAttribute()->mpcValue;
        this->ToAttribute()->mpcValue = apl_strdup(apcValue);
        ACL_FREE(lpTmp);
    }
}
    
const char* CXmlAttribute::GetName()
{
    return this->ToAttribute() != NULL ? this->ToAttribute()->mpcName : NULL;
}
    
const char* CXmlAttribute::GetValue()
{
    return this->ToAttribute() != NULL ? this->ToAttribute()->mpcValue : NULL;
}

//////////////////////////////////////////////////////////////////////////////////
void CXmlText::SetValue( const char* apcValue )
{
    if ( this->ToText() != NULL )
    {
        char* lpTmp = this->ToText()->mpcValue;
        this->ToText()->mpcValue = apl_strdup(apcValue);
        ACL_FREE(lpTmp);
    }
}
    
const char* CXmlText::GetValue()
{
    return this->ToText() != NULL ? this->ToText()->mpcValue : NULL;
}

////////////////////////////////////////////////////////////////////////////////////
CXmlClear::ETagType CXmlClear::GetType()
{
    if ( apl_strcmp( this->ToClear()->mpcOpenTag, "<![CDATA[" ) == 0 )
    {
        return TAG_CDATA;
    }
    else if ( apl_strcmp( this->ToClear()->mpcOpenTag, "<PRE>" ) == 0 )
    {
        return TAG_PRE;
    }
    else if ( apl_strcmp( this->ToClear()->mpcOpenTag, "<SCRIPT>" ) == 0 )
    {
        return TAG_SCRIPT;
    }
    else if ( apl_strcmp( this->ToClear()->mpcOpenTag, "<!--" ) == 0 )
    {
        return TAG_NOTE;
    }
    else if ( apl_strcmp( this->ToClear()->mpcOpenTag, "<!DOCTYPE" ) == 0 )
    {
        return TAG_DOCTYPE;
    }
    else if ( apl_strcmp( this->ToClear()->mpcOpenTag, "<?" ) == 0 )
    {
        return TAG_DECLARE;
    }
    else
    {
        return TAG_UNKNOWN;
    }
}

void CXmlClear::SetValue( const char* apcValue )
{
    if ( this->ToClear() != NULL )
    {
        char* lpTmp = this->ToClear()->mpcValue;
        this->ToClear()->mpcValue = apl_strdup(apcValue);
        ACL_FREE(lpTmp);
    }
}

const char* CXmlClear::GetValue()
{
    return this->ToClear() != NULL ? this->ToClear()->mpcValue : NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////
CXmlRootElement::CXmlRootElement(void)
{
    apl_memset( &this->moNode, 0, sizeof(this->moNode) );
    this->moNode.miType = XML_ELEMENT;
    this->SetNode(&this->moNode);
    
    xml::McbXMLElement* lpoElement = xml::McbCreateRoot();
    ACL_ASSERT(lpoElement != NULL);
    
    this->SetElement(lpoElement);
}

CXmlRootElement::~CXmlRootElement(void)
{
    this->SetElement(NULL);
}

void CXmlRootElement::SetElement( xml::McbXMLElement* apoElement )
{
    if (this->moNode.mnNode.mpoElement != NULL)
    {
        xml::McbDeleteRoot(this->moNode.mnNode.mpoElement);
    }
 
    this->moNode.mnNode.mpoElement = apoElement;
}

//////////////////////////////////////////////////////////////////////////////////////////
CXmlParser::CXmlParser(void)
{
    apl_memset( &this->moResult, 0, sizeof(this->moResult) );
}
    
CXmlParser::~CXmlParser(void)
{
}
       
apl_int_t CXmlParser::Parse( char const* apcXml, CXmlRootElement& aoRootElement )
{
    apl_memset( &this->moResult, 0, sizeof(this->moResult) );
    
    xml::McbXMLElement* lpoElement = xml::McbParseXML(apcXml, &this->moResult);
    
    if (lpoElement != NULL)
    {
        aoRootElement.SetElement(lpoElement);
    }
    
    return lpoElement == NULL ? -1 : 0;
}
    
apl_size_t CXmlParser::GetErrorLine()
{
    return this->moResult.miLine;
}
    
apl_size_t CXmlParser::GetErrorColumn()
{
    return this->moResult.miColumn;
}

char const* CXmlParser::GetStrError(void)
{
    return xml::McbGetError( this->moResult.miErrCode);
}

/////////////////////////////////////////////////////////////////////////////////////////
CXmlCreator::CXmlCreator(void)
    : mpcXml(NULL)
    , muLength(0)
{
}

CXmlCreator::~CXmlCreator(void)
{
    ACL_FREE(this->mpcXml);
    this->muLength = 0;
}
    
apl_int_t CXmlCreator::Create(CXmlRootElement& aoRootElement, EFormatOption aeFormat)
{     
    if ( aoRootElement.ToElement() == NULL )
    {
        return -1;
    }
    
    if (this->mpcXml != NULL)
    {
        ACL_FREE(this->mpcXml);
    }
    
    this->mpcXml = xml::McbCreateXMLString(aoRootElement.ToElement(), aeFormat != OPT_UNFORMAT ? 1 : 0, &this->muLength);
    
    return this->mpcXml == NULL ? -1 : 0;
}

apl_size_t CXmlCreator::GetLength()
{
    return this->muLength;
}
    
char const* CXmlCreator::GetXml()
{
    return this->mpcXml;
}
    
ACL_NAMESPACE_END
