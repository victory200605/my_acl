#ifndef __AILIBEX__AIMCBXML_H__
#define __AILIBEX__AIMCBXML_H__

#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

/////////////////////////////////////////////////////////////////////////
//
typedef enum McbXMLNodeType
{
    eNodeEmpty = 0,
    eNodeAttribute, 
    eNodeElement,
    eNodeText,
    eNodeClear
} McbXMLNodeType;

typedef struct McbXMLNode
{
    McbXMLNodeType ciType;

    union
    {
        struct McbXMLAttribute  *cpoAttrib;
        struct McbXMLElement    *cpoElement;
        struct McbXMLText       *cpoText;
        struct McbXMLClear      *cpoClear;
    } coNode;
} McbXMLNode;

/////////////////////////////////////////////////////////////////////////
//
typedef struct McbXMLElement
{
    char                    *cpcName;        /* Element name              */
    int                     ciCount;         /* Num of child nodes        */
    int                     ciMax;           /* Num of allocated nodes    */
    int                     ciIsDeclaration; /* Whether node is an XML    */
                                             /* declaration - '<?xml ?>'  */
    struct McbXMLNode       *cpoEntries;     /* Array of child nodes      */
    struct McbXMLElement    *cpoParent;      /* Pointer to parent element */
} McbXMLElement;

typedef struct McbXMLText
{
    char    *cpcValue;
} McbXMLText;

typedef struct McbXMLClear
{
    const char    *cpcOpenTag;
    char          *cpcValue;
    const char    *cpcCloseTag;
} McbXMLClear;

typedef struct McbXMLAttribute
{
    char    *cpcName;
    char    *cpcValue;
} McbXMLAttribute;

/////////////////////////////////////////////////////////////////////////
//
typedef enum McbXMLError
{
    eXMLErrorNone = 0,
    eXMLErrorEmpty,
    eXMLErrorFirstNotStartTag,
    eXMLErrorMissingTagName,
    eXMLErrorMissingEndTagName,
    eXMLErrorNoMatchingQuote,
    eXMLErrorUnmatchedEndTag,
    eXMLErrorUnexpectedToken,
    eXMLErrorInvalidTag,
    eXMLErrorNoElements
} McbXMLError;

typedef struct McbXMLResults
{
    int             ciLine;
    int             ciColumn;
    McbXMLError     ciErrCode;
} McbXMLResults;

/*
 ****************************************************************************
 * Construct/delete root element
 ****************************************************************************
 */
McbXMLElement *McbCreateRoot( void );
void McbDeleteRoot( McbXMLElement *apoElement );

/*
 ****************************************************************************
 * Obtain error information in a string.
 ****************************************************************************
 */
const char *McbGetError( McbXMLError aiErrCode );

/*
 ****************************************************************************
 * Parse XML string into elements.  This returns a pointer to the first 
 * element (created on the heap) if successful.  This must be deleted by 
 * first calling McbDeleteElement() to recursively delete child nodes then 
 * calling free on the element to cleanup the heap.
 * If the function fails then 0 will be returned.  If the results pointer
 * given to the function was not 0 the error, line and column can be 
 * obtained.
 ****************************************************************************
 */
McbXMLElement *McbParseXML( const char *apcXML, McbXMLResults *apoResults );

/*
 ****************************************************************************
 * Clears an element (deletes its children and the memory which belongs to 
 * it).
 ****************************************************************************
 */
void McbDeleteElement( McbXMLElement *apoEntry );

/*
 ****************************************************************************
 * Enumerate nodes in the list returning a pointer to a node.  The index 
 * pointer should be initialised to zero initially - this will be incremented 
 * for each subsequent node that is obtained.
 * 0 will be returned when all nodes have been obtained.
 ****************************************************************************
 */
McbXMLNode *McbEnumNodes( McbXMLElement *apoEntry, int *apiIndex );

/*
 ****************************************************************************
 * Recursively search the tree for the required element based on the given
 * path.
 ****************************************************************************
 */
McbXMLElement *McbFindElement( McbXMLElement *apoHead, const char *apcName );

/*
 ****************************************************************************
 * Search the tree for the required elements based on the given
 * path.
 ****************************************************************************
 */
size_t McbFindElements( McbXMLElement *apoHead, const char *apcName, McbXMLElement **apoElemArray, size_t aiCount );

/*
 ****************************************************************************
 * Search the given element for an attribute.
 ****************************************************************************
 */
McbXMLAttribute *McbFindAttribute( McbXMLElement *apoEntry, const char *apcAttrib );

/*
 ****************************************************************************
 * Enumerate elements on the given element.
 ****************************************************************************
 */
McbXMLElement *McbEnumElements( McbXMLElement *apoEntry, int *apiIndex );

/*
 ****************************************************************************
 * Add an attribute to the element
 ****************************************************************************
 */
McbXMLAttribute *McbAddAttribute( McbXMLElement *apoEntry, char *apcName, char *apcValue, int aiGrowBy );
McbXMLText *McbAddText( McbXMLElement *apoEntry, char *apcValue, int aiGrowBy );

/*
 ****************************************************************************
 * Enumerate attributes on the given element.
 ****************************************************************************
 */
McbXMLAttribute *McbEnumAttributes( McbXMLElement *apoEntry, int *apiIndex );

/*
 ****************************************************************************
 * Create elements in the list based on the path, returning the final node.
 ****************************************************************************
 */
McbXMLElement *McbCreateElements( McbXMLElement *apoEntry, const char *apcPath );

/*
 ****************************************************************************
 * Duplicate a string.
 ****************************************************************************
 */
char *McbStrdup( const char *apcData, int aiSize );

/*
 ****************************************************************************
 * Create an XML string from the head element.
 * If successful this returns the XML string representation of the specified 
 * XML element node and its subnodes.  This string must be deleted with 
 * free().
 *
 * If the size pointer is not 0 then the total size of the returned string
 * NOT INCLUDING THE NULL TERMINATOR will be updated.
 ****************************************************************************
 */
char *McbCreateXMLString( McbXMLElement *apoHead, int aiFormat, int *apiSize );
int McbCreateXMLStringR( McbXMLElement *apoEntry, char *apcMarker, int aiFormat );

/*
 ****************************************************************************
 * Add clear unformatted data to the node.
 ****************************************************************************
 */
McbXMLClear *McbAddCData( McbXMLElement *apoEntry, char *apcValue, int aiGrowBy );
McbXMLElement *McbAddElement( McbXMLElement *apoEntry, char *apcName, int aiIsDeclaration, int aiGrowBy );

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AIMCBXML_H__

