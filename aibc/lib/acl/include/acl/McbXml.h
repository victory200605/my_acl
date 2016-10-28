
#ifndef ACL_MCBXML_H
#define ACL_MCBXML_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

namespace xml
{

/**
 * nodetype of McbXml Enum
 */
enum EMcbXMLNodeType
{
    /**
     * NODE_EMPTY: 0
     */
    NODE_EMPTY = 0,
    /**
     * NODE_ATTRIBUTE: attribute type
     */
    NODE_ATTRIBUTE, 
    /**
     * NODE_ELEMENT: element type
     */
    NODE_ELEMENT,
    /**
     * NODE_TEXT: text type
     */
    NODE_TEXT,
    /**
     * NODE_CLEAR: clear type
     */
    NODE_CLEAR
};

/**
 * tagtype of mcbxml Enum
 */
enum ETagType
{
    /**
     * TAG_UNKNOWN: -1
     */
    TAG_UNKNOWN = -1,
    /**
     * TAG_CDATA: CDATA tag
     */
    TAG_CDATA,     //<![CDATA[ ... ]]>
    /**
     * TAG_PRE: PRE tag
     */
    TAG_PRE,       //<PRE> ... </PRE>
    /**
     * SCRIPT: SCRIPT tag
     */
    TAG_SCRIPT,    //<SCRIPT> ... </SCRIPT>
    /**
     * TAG_NODE: node tag
     */
    TAG_NOTE,      //<!-- ... -->
    /**
     * TAG_DOCTYPE: DOCTYPE tag
     */
    TAG_DOCTYPE,   //<!DOCTYPE .. >
    /**
     * TAG_DECLARE: DECLARE tag
     */
    TAG_DECLARE    //<? .. >
};

/**
 * error Enum
 */
enum EMcbXMLError
{
    /** 
     * ERROR_NODE : 0
     */
    ERROR_NONE = 0,
    /**
     * ERROR_EMPTY
     */
    ERROR_EMPTY,
    /**
     * ERROR_FIRST_NOT_START_TAG
     */
    ERROR_FIRST_NOT_START_TAG,
    /**
     * ERROR_MISSING_TAG_NAME
     */
    ERROR_MISSING_TAG_NAME,
    /**
     * ERROR_MISSING_END_TAG_NAME
     */
    ERROR_MISSING_END_TAG_NAME,
    /**
     * ERROR_NOMATCHING_QUOTE
     */
    ERROR_NOMATCHING_QUOTE,
    /**
     * ERROR_UNMATCHED_END_TAG
     */
    ERROR_UNMATCHED_END_TAG,
    /**
     * ERROR_UNEXPECTED_TOKEN
     */
    ERROR_UNEXPECTED_TOKEN,
    /**
     * ERROR_INVALIDTAG
     */
    ERROR_INVALIDTAG,
    /**
     * ERROR_NOELEMENTS
     */ 
    ERROR_NOELEMENTS
};

/**
 * struct McbXMLNode
 */
struct McbXMLNode
{
    /**
     * node of type
     */
    apl_int_t miType;

    /**
     * pointer of nodetype
     */
    union
    {
        struct McbXMLAttribute* mpoAttrib;
        struct McbXMLElement*   mpoElement;
        struct McbXMLText*      mpoText;
        struct McbXMLClear*     mpoClear;
    } mnNode;
    
};

/**
 * struct McbXMLElement
 */
struct McbXMLElement
{
    /**
     * Element name
     */
    char*                 mpcName;
    /**
     * Num of child nodes
     */
    apl_size_t            muCount;
    /**
     * Num of allocated nodes
     */
    apl_size_t            muMax;
    /**
     * Whether node is an XML declaration -'<?xml ?>'
     */
    apl_int_t             muIsDeclaration;
    /**
     * Array of child nodes
     */ 
    struct McbXMLNode*    mpoEntries;
    /**
     * Pointer to parent element
     */
    struct McbXMLElement* mpoParent;
};

/**
 * struct McbXMLText
 */
struct McbXMLText
{
    /**
     * the value of text
     */
    char* mpcValue;
};

/**
 * struct McbXMLClear
 */
struct McbXMLClear
{
    /**
     * the Open tag of clear
     */
    char const* mpcOpenTag;
    /**
     * the value of clear
     */
    char* mpcValue;
    /**
     * the Close tag of clear
     */
    char const* mpcCloseTag;
};

/**
 * struct McbXMLAttribute
 */
struct McbXMLAttribute
{
    /**
     * name of attribute
     */
    char* mpcName;
    /**
     * value of attribute
     */
    char* mpcValue;
};

/**
 * struct McbXMLResults
 */
struct McbXMLResults
{
    /**
     * the linenumber
     */
    apl_int_t miLine;
    /**
     * the columns
     */
    apl_int_t miColumn;
    /**
     * error code
     */
    apl_int_t miErrCode;
};

/**
 * Construct root element
 */
McbXMLElement* McbCreateRoot(void);

/**
 * Delete root element
 */
void McbDeleteRoot( McbXMLElement* apoElement );

/**
 * Obtain error information in a string.
 */
char const* McbGetError( apl_int_t aiErrCode );

/**
 * Parse XML string into elements.  This returns a pointer to the first 
 * element (created on the heap) if successful.  This must be deleted by 
 * first calling McbDeleteElement() to recursively delete child nodes then 
 * calling free on the element to cleanup the heap.
 * If the function fails then 0 will be returned.  If the results pointer
 * given to the function was not 0 the error, line and column can be 
 * obtained.
 */
McbXMLElement* McbParseXML( char const* apcXML, McbXMLResults* apoResults );

/**
 * Clears an element (deletes its children and the memory which belongs to it).
 */
void McbDeleteElement( McbXMLElement* apoEntry );

/**
 * Enumerate nodes in the list returning a pointer to a node.  The index 
 * pointer should be initialised to zero initially - this will be incremented 
 * for each subsequent node that is obtained.
 * 0 will be returned when all nodes have been obtained.
 */
McbXMLNode* McbEnumNodes( McbXMLElement* apoEntry, apl_size_t* apiIndex );

/**
 * Recursively search the tree for the required element based on the given
 * path.
 */
McbXMLElement* McbFindElement( McbXMLElement* apoHead, char const* apcName );

/**
 * Search the tree for the required elements based on the given path.
 */
apl_size_t McbFindElements( McbXMLElement* apoHead, char const* apcName, McbXMLElement** apoElemArray, apl_size_t aiCount );

/**
 * Search the given element for an attribute.
 */
McbXMLAttribute* McbFindAttribute( McbXMLElement* apoEntry, char const* apcAttribute );

/**
 * Enumerate elements on the given element.
 */
McbXMLElement* McbEnumElements( McbXMLElement* apoEntry, apl_size_t* apiIndex );

/**
 * Add element to the node.
 */
McbXMLElement *McbAddElement( McbXMLElement* apoEntry, char* apcName, apl_int_t aiIsDeclaration, apl_size_t aiGrowBy );

/**
 * Add an attribute to the element
 */
McbXMLAttribute* McbAddAttribute( McbXMLElement* apoEntry, char* apcName, char* apcValue, apl_size_t aiGrowBy );

/**
 * Add an text to the element
 */
McbXMLText* McbAddText( McbXMLElement* apoEntry, char* apcValue, apl_size_t aiGrowBy );

/**
 * Add clear unformatted data to the node.
 */
McbXMLClear* McbAddClear( McbXMLElement* apoEntry, ETagType aeClearType, char* apcValue, apl_size_t aiGrowBy );

/**
 * Enumerate attributes on the given element.
 */
McbXMLAttribute* McbEnumAttributes( McbXMLElement* apoEntry, apl_size_t* apiIndex );

/**
 * Create elements in the list based on the path, returning the final node.
 */
McbXMLElement* McbCreateElements( McbXMLElement* apoEntry, char const* apcPath );

/**
 * Create an XML string from the head element.
 * If successful this returns the XML string representation of the specified 
 * XML element node and its subnodes.  This string must be deleted with 
 * free().
 *
 * If the size pointer is not 0 then the total size of the returned string
 * NOT INCLUDING THE NULL TERMINATOR will be updated.
 */
char* McbCreateXMLString( McbXMLElement* apoHead, apl_int_t aiFormat, apl_size_t* apiSize );
apl_size_t McbCreateXMLStringR( McbXMLElement* apoEntry, char* apcMarker, apl_int_t aiFormat);

}

ACL_NAMESPACE_END

#endif//ACL_MCBXML_H
