/**
 * @file Xml.h
 */
#ifndef ACL_XML_H
#define ACL_XML_H

#include "acl/Utility.h"
#include "acl/McbXml.h"

ACL_NAMESPACE_START

/**
 * Xml node type enum value
 */
enum ENodeType
{
    XML_NODE = xml::NODE_EMPTY,
    XML_ELEMENT = xml::NODE_ELEMENT,
    XML_ATTRIBUTE = xml::NODE_ATTRIBUTE,
    XML_TEXT = xml::NODE_TEXT,
    XML_CLEAR = xml::NODE_CLEAR
};

/**
 * @class CXmlBaseNodeIterator
 * This is base node IteratorType and all xml IteratorType will inherit from it
 */
class CXmlBaseNodeIterator
{
public:
    /**
     * Constructor
     * @param [in]  apoParent    parent's element
     * @param [out] apoNode      child's Node
     */
    CXmlBaseNodeIterator( xml::McbXMLElement* apoParent, xml::McbXMLNode* apoNode );

    /**
     * Allow the use of compare operator with TXmlNodeIterator
     */
    bool operator == ( CXmlBaseNodeIterator const& aoRhs );

    /**
     * Allow the use of compare operator with TXmlNodeIterator
     */
    bool operator != ( CXmlBaseNodeIterator const& aoRhs );

protected:
    void SetParent( xml::McbXMLElement* apoParent )  { this->mpoParent = apoParent; }
        
    xml::McbXMLElement* GetParent(void) const        { return this->mpoParent; }
    
    void SetNode( xml::McbXMLNode* apoNode )         { this->mpoNode = apoNode; }
        
    xml::McbXMLNode* GetNode(void) const             { return this->mpoNode; }

private:
    xml::McbXMLElement* mpoParent;
    xml::McbXMLNode* mpoNode;
};

/**
 * @class CXmlEndIterator
 * This is all xml element end iterator
 */
class CXmlEndIterator : public CXmlBaseNodeIterator
{
public:
    /**
     * Constructor
     */
    CXmlEndIterator(void);
};

/**
 * @class TXmlNodeIterator
 * This xml node IteratorType which derive from CXmlBaseNodeIterator
 * That can be used to visit every xml node element
 */
template< typename TNodeType, apl_int_t TypeValue >
class TXmlNodeIterator : public CXmlBaseNodeIterator
{
public:

    /**
     * Constructor
     */
    TXmlNodeIterator(void)
        : CXmlBaseNodeIterator(NULL, NULL)
        , miType(TypeValue)
    {
    }
    
    /**
     * Constructor
     * @param [in]  apoParent    parent's element
     * @param [out] apoNode      child's node
     */
    TXmlNodeIterator( xml::McbXMLElement* apoParent, xml::McbXMLNode* apoNode )
        : CXmlBaseNodeIterator(apoParent, apoNode)
        , miType(TypeValue)
    {
    }
    
    /**
     * Constructor
     */
    TXmlNodeIterator( CXmlBaseNodeIterator const& aoOther )
        : CXmlBaseNodeIterator(aoOther)
        , miType(TypeValue)
    {
    }
    
    /**
     * Allow the use of assign operator with TXmlNodeIterator
     */
    TXmlNodeIterator& operator = ( CXmlEndIterator const& aoOther )
    {
        this->SetNode(NULL);
        return *this;
    }
    
    /**
     * Allow the use of assign operator with TXmlNodeIterator
     */
    TXmlNodeIterator& operator = ( TXmlNodeIterator const& aoOther )
    {
        this->SetParent( aoOther.GetParent() );
        this->SetNode( aoOther.GetNode() );
        return *this;
    }
    
    /**
     * Allow the use of add operator with TXmlNodeIterator
     */
    TXmlNodeIterator& operator ++ ()
    {
        if ( this->GetParent() != NULL && this->GetNode() != NULL )
        {
            apl_size_t liIndex = this->GetNode() - this->GetParent()->mpoEntries;
            
            this->SetNode(NULL);
            
            for ( ++liIndex; liIndex < this->GetParent()->muCount; liIndex++ )
            {
                if ( this->miType == XML_NODE
                    || this->GetParent()->mpoEntries[liIndex].miType == this->miType)
                {
                    this->SetNode(&this->GetParent()->mpoEntries[liIndex]);
                    break;
                }
            }
        }
        
        return *this;
    }
    
    /**
     * Allow the use of add operator with TXmlNodeIterator
     */
    TXmlNodeIterator operator ++ (int)
    {
        TXmlNodeIterator loTmp(*this);
        ++*this;
        return loTmp;
    }

    /**
     * Allow the use of pointer operator with TXmlNodeIterator
     */
    TNodeType* operator -> ()
    {
        return &(*(*this));
    }
    
    TNodeType& operator * ()
    {
        this->moNode.SetNode( this->GetNode() );
        
        return this->moNode;
    }

private:
    apl_int_t miType;
    TNodeType moNode;
};


/**
 * @class CXmlNode
 * This base xml node, and all other node will inherit from it
 */
class CXmlNode
{
    template <typename, apl_int_t> friend class TXmlNodeIterator;
    
public:
    /**
     * typedef TXmlNodeIterator<CXmlNode, XML_NODE> IteratorType;
     */
    typedef TXmlNodeIterator<CXmlNode, XML_NODE> IteratorType;

public:
    /**
     * Constructor
     */
    CXmlNode(void);
    
    /**
     * Check the type of xmlnode is element
     */
    bool IsElement(void);

    /**
     * Check the type of xmlnode is attribute
     */   
    bool IsAttribute(void);

    /**
     * Check the type of xmlnode is text 
     */
    bool IsText(void);

    /**
     * Check the type of xmlnode is clear
     */
    bool IsClear(void);
        
protected:
    void SetNode( xml::McbXMLNode* apoNode );
        
    xml::McbXMLNode* GetNode(void);
        
    xml::McbXMLElement* ToElement(void);
        
    xml::McbXMLAttribute* ToAttribute(void);
        
    xml::McbXMLText* ToText(void);
        
    xml::McbXMLClear* ToClear(void);

private:
    xml::McbXMLNode* mpoNode;
};

/**
 * @class CXmlAttribute
 * This xml attribute node and derive from CXmlNode
 */
class CXmlAttribute : public CXmlNode
{
    friend class CXmlElement;
public:
    /**
     * typedef TXmlNodeIterator<CXmlAttribute, XML_ATTRIBUTE> IteratorType;
     */
    typedef TXmlNodeIterator<CXmlAttribute, XML_ATTRIBUTE> IteratorType;
    
public:
    /**
     * set the name of xmlattribute
     *
     * @param [in] apcName  Attribute Name
     */
    void SetName( const char* apcName );
    
    /**
     * set the value of xmlattribute
     *
     * @param [in] apcValue Attribute Value
     */ 
    void SetValue( const char* apcValue );
    
    /**
     * get the name of xmlattribute
     */
    const char* GetName();
    
    /**
     * get the value of xmlattribute
     */
    const char* GetValue();
};

/**
 * @class CXmlText
 * This xml text node and derive from CXmlNode
 */
class CXmlText : public CXmlNode
{
public:
    /**
     * typedef TXmlNodeIterator<CXmlText, XML_TEXT> IteratorType;
     */
    typedef TXmlNodeIterator<CXmlText, XML_TEXT> IteratorType;
    
public:
    /**
     * set the value of xmltext
     *
     * @param [in] apcValue  Text value
     */
    void SetValue( const char* apcValue );
    
    /**
     * get the value of text
     */
    const char* GetValue();
};

/**
 * @class CXmlClear
 * This xml clear node and derive from CXmlNode
 */
class CXmlClear : public CXmlNode
{
public:
    /**
     * typedef TXmlNodeIterator<CXmlClear, XML_CLEAR> IteratorType;
     */
    typedef TXmlNodeIterator<CXmlClear, XML_CLEAR> IteratorType;
    
    /**
     * tagtype of xmlClear enumeration
     */
    enum ETagType
    {
        TAG_UNKNOWN = xml::TAG_UNKNOWN,
        TAG_CDATA = xml::TAG_CDATA,       //<![CDATA[ ... ]]>
        TAG_PRE = xml::TAG_PRE,           //<PRE> ... </PRE>
        TAG_SCRIPT = xml::TAG_SCRIPT,     //<SCRIPT> ... </SCRIPT>
        TAG_NOTE = xml::TAG_NOTE,         //<!-- ... -->
        TAG_DOCTYPE = xml::TAG_DOCTYPE,   //<!DOCTYPE .. >
        TAG_DECLARE = xml::TAG_DECLARE    //<? .. >
    };
    
public:
    /**
     * get the tagtype of xmlClear 
     */
    ETagType GetType();
    
    /**
     * set the value of xmlattribute
     *
     * @param [in] apcValue  Clear value
     */
    void SetValue( const char* apcValue );
    
    /**
     * get the value of xmlattribute
     */
    const char* GetValue();
};

/**
 * @class CXmlElement
 * This xml element node and derive from CXmlNode
 */
class CXmlElement : public CXmlNode
{
public:
    /**
     * typedef TXmlNodeIterator<CXmlElement, XML_ELEMENT> IteratorType;
     */
    typedef TXmlNodeIterator<CXmlElement, XML_ELEMENT> IteratorType;
    
public:
    /**
     * Constructor
     */
    CXmlElement(void);

    /**
     * set the name of xmlelement
     *
     * @param [in] apcName  Eelement name
     */
    void SetName( const char* apcName );
    
    /**
     * get the name of xmlelement
     */
    const char* GetName();
    
    /**
     * get the begin IteratorType of child and type is node
     */
    CXmlNode::IteratorType Begin();
    
    /**
     * get the begin IteratorType of child and type is element
     */
    CXmlElement::IteratorType BeginElement();
    
    /**
     * get the begin IteratorType of child and type is attribute
     */
    CXmlAttribute::IteratorType BeginAttribute();
    
    /**
     * get the begin IteratorType of child and type is text
     */
    CXmlText::IteratorType BeginText();

    /**
     * get the begin IteratorType of child and type is clear
     */
    CXmlClear::IteratorType BeginClear();
    
    /**
     * get the end IteratorType
     */
    CXmlEndIterator End();
    
    /**
     * find the specified the xml element, return the first element
     * @param [in] apcPath  the path of xml element
     *     exp:<element1>          <-- current element
     *             <element2>
     *                 <element3/> <-- FindElement("element2/element3")
     *             </element2>
     *         </element1>
     * @return    If find successful, the IteratorType of element will return, 
     *            otherwise, the end IteratorType will return.
     */
    CXmlElement::IteratorType FindElement( char const* apcPath );
    
    /**
     * find the not more than aiCount xml element
     *
     * @param [in] apcPath  the path of xml element
     *     exp:<element1>          <-- current element
     *             <element2>
     *                 <element3/> <-- FindElement("element2/element3")
     *             </element2>
     *         </element1>
     * @param [out] aoElement[]  storage result element
     * @param [in]  aiCount      storage buffer size
     * @return     the number suceessful finded.
     */
    apl_size_t FindElement( char const* apcPath, CXmlElement aoElement[], apl_size_t aiCount );
    
    /**
     * find the specified the xml attribute, return the first attribute
     * @param [in] apcPath  the path of xml element
     *     exp:<element1>          <-- current element
     *             <element2>
     *                 <element3/> <-- FindElement("element2/element3")
     *             </element2>
     *         </element1>
     * @return    If find successful, the IteratorType of element will return, 
     *            otherwise, the end IteratorType will return.
     */
    CXmlAttribute::IteratorType FindAttribute( char const* apcPath );
    
    /**
     * find the not more than aiCount xml attribute
     *
     * @param [in] apcPath  the path of xml element
     *     exp:<element1>          <-- current element
     *             <element2>
     *                 <element3/> <-- FindElement("element2/element3")
     *             </element2>
     *         </element1>
     * @param [out] aoAttribute[]  storage result element
     * @param [in]  aiCount        storage buffer size
     * @return     the number suceessful finded.
     */
    apl_size_t FindAttribute( char const* apcPath, CXmlAttribute aoAttribute[], apl_size_t aiCount );
    
    /**
     * add the xml element node, and return the element node IteratorType
     *
     * @param [in] apcName    the name of xml element
     * @return    If  add successful, the IteratorType of XmlElement will return, 
     *            otherwise, the end IteratorType will return.
     */
    CXmlElement::IteratorType AddElement( char const* apcName );
        
    /**
     * add the xml element node, and return the element node IteratorType
     *
     * @param [in] apcName    the name of xml element
     * @param [in] auGrowBy   xml element array size will be grow by @auGrowBy
     *
     * @return    If  add successful, the IteratorType of XmlElement will return, 
     *            otherwise, the end IteratorType will return.
     */
    CXmlElement::IteratorType AddElement( char const* apcName, apl_size_t auGrowBy );
    
    /**
     * add the xml attribute node, and return the attribute node IteratorType
     *
     * @param [in] apcName     the name  of xml attribute
     * @param [in] apcValue    the Value of xmlAttribute
     * @return    If add successful, the IteratorType of XmlAttribute will return, 
     *            otherwise, the end IteratorType will return.
     */
    CXmlAttribute::IteratorType AddAttribute( char const* apcName, char const* apcValue );
    
    /**
     * add the xml attribute node, and return the attribute node IteratorType
     *
     * @param [in] apcName     the name  of xml attribute
     * @param [in] apcValue    the Value of xmlAttribute
     * @param [in] auGrowBy    xml element array size will be grow by @auGrowBy
     *
     * @return    If add successful, the IteratorType of XmlAttribute will return, 
     *            otherwise, the end IteratorType will return.
     */
    CXmlAttribute::IteratorType AddAttribute( char const* apcName, char const* apcValue, apl_size_t auGrowBy );
        
    /**
     * add the xml text node, and return the text node IteratorType
     *
     * @param [in] apcValue    the Value of xml text
     * @return    If add successful, the IteratorType of XmlText will return, 
     *            otherwise, the end IteratorType will return.
     */
    CXmlText::IteratorType AddText( char const* apcValue );
    
    /**
     * add the xml clear node, and return the clear node IteratorType
     *
     * @param [in] aeType      the tagtype of xml clear
     * @param [in] apcValue    the value of xml clear
     * @return    If add successful, the IteratorType of XmlClear will return, 
     *            otherwise, the end IteratorType will return.
     */
    CXmlClear::IteratorType AddClear( CXmlClear::ETagType aeType, char const* apcValue );

protected:
    xml::McbXMLElement* FindRootElement( char const* apcPath, char const** appcName );
};

/**
 * @class CXmlParser
 * Xml parser and derive from CXmlElement
 */
class CXmlRootElement : public CXmlElement
{
    friend class CXmlParser;
    friend class CXmlCreator;
    
public:
    /**
     * Constructor
     */
    CXmlRootElement(void);
    
    /**
     * Destructor 
     */
    ~CXmlRootElement(void);

protected:
    void SetElement( xml::McbXMLElement* apoElement );
    
private:
    xml::McbXMLNode moNode;
};

/**
 * @class CXmlParser
 * Xml parser and derive from CXmlElement
 */
class CXmlParser
{
public:
    /**
     * Constructor
     */
    CXmlParser(void);
    
    /**
     * Destructor 
     */
    ~CXmlParser(void);
    
    /**
     * parse the text xml
     * @param [in] apcXml  text xml
     * @return    0 if successful, -1 if not.
     */
    apl_int_t Parse( char const* apcXml, CXmlRootElement& aoRootElement );
    
    /**
     * get the error line number where string of xml are parsed
     * @return   the lineNumber.
     */ 
    apl_size_t GetErrorLine(void);
    
    /**
     * get the error column number where string of xml parse
     * @return    columnNumber.
     */
    apl_size_t GetErrorColumn(void);
    
    /**
     * get the error info when string of xml are parsed
     * @return   a pointer pointed to string
     */
    char const* GetStrError(void);

private:
    xml::McbXMLResults moResult;
};

/**
 * @class CXmlCreator
 * This is xml creator that can be used to construct xml node tree and created text xml
 */
class CXmlCreator
{
public:
    enum EFormatOption
    {
        OPT_UNFORMAT  = 0,
        OPT_DEFFORMAT = 1
    };
    
public:
    /**
     * Constructor
     */
    CXmlCreator(void);
    
    /**
     * destructor
     */
    ~CXmlCreator(void);
    
    /**
     * create the text xml
     * @param [in] abIsFormat   if format context
     * @return   0 if successful, -1 if not.
     */
    apl_int_t Create( CXmlRootElement& aoRootElement, EFormatOption aeFormat = OPT_UNFORMAT );
    
    /**
     * get the length of xml
     */
    apl_size_t GetLength();
    
    /**
     * get the string of xml
     */
    char const* GetXml();

private:
    char* mpcXml;
    apl_size_t muLength;
};

ACL_NAMESPACE_END

#endif//ACL_XML_H
