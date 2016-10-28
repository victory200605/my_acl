
#include "acl/McbXml.h"

ACL_NAMESPACE_START

namespace xml
{

#define MCB_GROWBY            (5)
#define MCB_INDENTCHAR        ('\t')
#define MCB_LENSTR(str)       (str ? apl_strlen(str) : 0)

enum EMcbStatus
{
    INSIDE_TAG = 0,
    OUTSIDE_TAG
};

enum EMcbAttribute
{
    ATTRIBUTE_NAME = 0,
    ATTRIBUTE_EQUALS,
    ATTRIBUTE_VALUE
};

enum EMcbTokenType
{
    TOKEN_TEXT = 0,
    TOKEN_QUOTED_TEXT,
    TOKEN_TAG_START,          /* "<"   */
    TOKEN_TAG_END,            /* "</"  */
    TOKEN_CLOSE_TAG,          /* ">"   */
    TOKEN_EQUALS,             /* "="   */
    TOKEN_DECLARATION,        /* "<?"  */
    TOKEN_SHORT_HAND_CLOSE,   /* "/>"  */
    TOKEN_CLEAR,
    TOKEN_ERROR
};

struct McbClearTag
{
    char const* cpcOpen;
    char const* cpcClose;
};

struct McbNextToken
{
    McbClearTag* mpoClear;
    char const* cpcStrBegin;
};

struct McbXML
{
    char const*  cpcXML;
    apl_int_t    ciIndex;
    apl_int_t    miErrCode;
    char const*  cpcEndTag;
    apl_int_t    ciEndTagLen;
    char const*  cpcNewElement;
    apl_int_t    ciNewElementLen;
    apl_int_t    ciFirst;
    McbClearTag* mpoClearTags;
};

void McbStrCpy( char* apcDest, char const* apcSrc )
{
    while( *apcSrc != '\0' )
    {
        *apcDest++ = *apcSrc++;
    }
}

void McbInitElement( McbXMLElement* apoEntry, McbXMLElement* apoParent, char* apcName, apl_int_t aiIsDeclaration )
{
    apoEntry->muMax = 0;
    apoEntry->muCount = 0;
    apoEntry->mpoEntries = NULL;
    apoEntry->mpoParent = apoParent;
    apoEntry->muIsDeclaration = aiIsDeclaration;
    apoEntry->mpcName = apcName;
}

McbXMLElement* McbCreateRoot(void)
{
    McbXMLElement* lpoElement;

    ACL_MALLOC( lpoElement, McbXMLElement, sizeof(McbXMLElement) );
    
    McbInitElement(lpoElement, NULL, 0, 0);

    return lpoElement;
}

void McbDeleteRoot( McbXMLElement* apoElement )
{
    McbDeleteElement(apoElement);
    
    ACL_FREE(apoElement);
}

void McbDeleteAttribute( McbXMLAttribute* apoEntry )
{
    if(apoEntry->mpcName) 
    {
        ACL_FREE(apoEntry->mpcName);
        apoEntry->mpcName = NULL;
    }

    if(apoEntry->mpcValue) 
    {
        ACL_FREE(apoEntry->mpcValue);
        apoEntry->mpcValue=  NULL;
    }
}

void EMcbAttributeuteAttach( McbXMLAttribute* apoDst, McbXMLAttribute* apoSrc, apl_size_t aiNum )
{
    for ( apl_size_t liIt = 0; liIt < aiNum; liIt++ )
    {
        apoDst[liIt].mpcName = apoSrc[liIt].mpcName;
        apoDst[liIt].mpcValue = apoSrc[liIt].mpcValue;

        apoSrc[liIt].mpcName = NULL;
        apoSrc[liIt].mpcValue = NULL;
    }
}

char McbGetNextChar( McbXML* apoXML )
{
    char liChar;
    
    liChar = apoXML->cpcXML[apoXML->ciIndex];
    if(liChar)
    {
        apoXML->ciIndex++;
    }
    
    return liChar;
}

char McbFindNonWhiteSpace( McbXML* apoXML )
{
    char liChar = 0;
    int  liExitFlag = 0;

    while( (liExitFlag == 0) && ( liChar = McbGetNextChar(apoXML) ) )
    {
        switch(liChar)
        {
            case '\n':
            case ' ':
            case '\t':
            case '\r':
                continue;           

            default:
                liExitFlag = 1;
                break;
        }
    }

    return liChar;
}

char* McbStrdup( char const* apcData, apl_int_t aiSize)
{
    apl_int_t liSize;
    char*     lpcTemp;
        
    liSize = ( aiSize<=0 ? (apl_int_t)apl_strlen(apcData) : aiSize );
    ACL_MALLOC(lpcTemp, char, liSize + 1);

    apl_memcpy(lpcTemp, apcData, liSize);
    lpcTemp[liSize] = (char)0;

    return lpcTemp;
}

McbNextToken McbGetNextToken( McbXML* apoXML, apl_size_t* apiTokenLen, apl_int_t* apiType)
{
    apl_size_t      liSize;
    apl_size_t      liIndex;
    apl_int_t       liExitFlag;
    apl_int_t       liFoundMatch;
    apl_size_t      liOpenLength;
    apl_int_t       liIsText = 0;
    char            liChar = 0;
    char            liTempChar;
    char const*     lpcOpen;
    char const*     lpcXML;
    McbNextToken    loResult;

    liChar = McbFindNonWhiteSpace(apoXML);
    apl_memset(&loResult, 0, sizeof(McbNextToken));

    if(liChar == 0)
    {
        *apiTokenLen = 0;
        *apiType = TOKEN_ERROR;
        return loResult;
    }
    
    lpcXML = apoXML->cpcXML;
    loResult.cpcStrBegin = &lpcXML[apoXML->ciIndex-1];

    liIndex = 0;
    while (true)
    {
        lpcOpen = apoXML->mpoClearTags[liIndex].cpcOpen;
        if (lpcOpen == NULL)
        {
            break;
        }

        liOpenLength = apl_strlen(lpcOpen);
        if( apl_strncmp(lpcOpen, loResult.cpcStrBegin, liOpenLength) == 0 )
        {
            loResult.mpoClear = &apoXML->mpoClearTags[liIndex];
            apoXML->ciIndex += (liOpenLength-1);

            *apiType = TOKEN_CLEAR;
            return loResult;
        }

        liIndex++;
    }

    liTempChar = 0;
    lpcXML = apoXML->cpcXML;
        
    switch(liChar)
    {
        case '\'':
        case '\"':
            *apiType = TOKEN_QUOTED_TEXT;
            liTempChar = liChar;
    
            liSize = 1;
            liFoundMatch = 0;
    
            while((liChar = McbGetNextChar(apoXML)) != 0)
            {
                liSize++;
    
                if(liChar == liTempChar)
                {
                    liFoundMatch = 1;
                    break;
                }
            }
    
            if(liFoundMatch == 0)
            {
                apoXML->miErrCode = ERROR_NOMATCHING_QUOTE;               
                *apiType = TOKEN_ERROR;
            }

            if(McbFindNonWhiteSpace(apoXML))
            {
                apoXML->ciIndex--;
            }
    
            break;
    
        case '=':
            liSize = 1;
            *apiType = TOKEN_EQUALS;
            break;
    
        case '>':
            liSize = 1;
            *apiType = TOKEN_CLOSE_TAG;
            break;
    
        case '<':
            liTempChar = apoXML->cpcXML[apoXML->ciIndex];
    
            if(liTempChar == '/')
            {
                McbGetNextChar(apoXML);
                *apiType = TOKEN_TAG_END;
                liSize = 2;
            }
            else if(liTempChar == '?')
            {
                McbGetNextChar(apoXML);
                *apiType = TOKEN_DECLARATION;
                liSize = 2;
            }
            else 
            {
                *apiType = TOKEN_TAG_START;
                liSize = 1;
            }
            break;
    
        case '/':
            liTempChar = apoXML->cpcXML[apoXML->ciIndex];
    
            if(liTempChar == '>')
            {
                McbGetNextChar(apoXML);
                *apiType = TOKEN_SHORT_HAND_CLOSE;
                liSize = 2;
                break;
            }

        default:
            liIsText = 1;
            break;
    }

    if(liIsText)
    {
        liSize = 1;
        liExitFlag = 0;
        *apiType = TOKEN_TEXT;

        while((liExitFlag == 0) && (liChar = McbGetNextChar(apoXML)))
        {
            switch(liChar)
            {
                case '\n':
                case ' ':
                case '\t':
                case '\r':
                    liExitFlag = 1;
                    break;
    
                case '/':
                    liTempChar = apoXML->cpcXML[apoXML->ciIndex];
    
                    if(liTempChar == '>')
                    {
                        apoXML->ciIndex--;
                        liExitFlag = 1;
                    }
                    else
                    {
                        liSize++;
                    }
                    break;
    
                case '<':
                case '>':
                case '=':
                    apoXML->ciIndex--;                  
                    liExitFlag = 1;
                    break;
    
                case 0:
                    liExitFlag = 1;
                    break;
    
                default:
                    liSize++;
                    break;
            }
        }
    }

    *apiTokenLen = liSize;
    return loResult;
}

const char *McbGetError( apl_int_t aiErrCode )
{
    typedef struct McbErrorList
    {
        EMcbXMLError miErrCode;
        const char* cpcError;
    } McbErrorList;

    const char* lpcError = "Unknown";
    static McbErrorList _errorList[] = 
    {
        { ERROR_NONE,                 "No error"                  },
        { ERROR_EMPTY,                "No XML data"               },
        { ERROR_FIRST_NOT_START_TAG,  "First token not start tag" },
        { ERROR_MISSING_TAG_NAME,     "Missing start tag name"    },
        { ERROR_MISSING_END_TAG_NAME, "Missing end tag name"      },
        { ERROR_NOMATCHING_QUOTE,     "Unmatched quote"           },
        { ERROR_UNMATCHED_END_TAG,    "Unmatched end tag"         },
        { ERROR_UNEXPECTED_TOKEN,     "Unexpected token found"    },
        { ERROR_INVALIDTAG,           "Invalid tag found"         },
        { ERROR_NOELEMENTS,           "No elements found"         },
        { ERROR_NONE,                 NULL                        }
    };

    for(int liIt = 0; _errorList[liIt].cpcError; liIt++)
    {
        if(_errorList[liIt].miErrCode == aiErrCode)
        {
            lpcError = _errorList[liIt].cpcError;
            break;
        }
    }

    return lpcError; 
}

void McbDeleteText( McbXMLText* apoText )
{
    if(apoText->mpcValue)
    {
        ACL_FREE(apoText->mpcValue);
        apoText->mpcValue = NULL;
    }
}

void McbDeleteClear( McbXMLClear* apoClear )
{
    if(apoClear->mpcValue)
    {
        ACL_FREE(apoClear->mpcValue);
        apoClear->mpcValue = NULL;
    }
}

void McbDeleteNode(McbXMLNode* apoEntry)
{
    if(apoEntry)
    {
        if(apoEntry->miType == NODE_EMPTY)
        {
            return;
        }

        switch(apoEntry->miType)
        {
            case NODE_ATTRIBUTE:
                McbDeleteAttribute(apoEntry->mnNode.mpoAttrib);
                break;
                
            case NODE_ELEMENT:
                McbDeleteElement(apoEntry->mnNode.mpoElement);
                break;
    
            case NODE_TEXT:
                McbDeleteText(apoEntry->mnNode.mpoText);
                break;
    
            case NODE_CLEAR:
                McbDeleteClear(apoEntry->mnNode.mpoClear);
                break;
    
            default:
                assert(false);
                break;
        }   

        ACL_FREE(apoEntry->mnNode.mpoAttrib);
        apoEntry->miType = NODE_EMPTY;
    }
}

void McbDeleteElement( McbXMLElement* apoEntry )
{
    for ( apl_size_t liIt = 0; liIt < apoEntry->muCount; liIt++ )
    {
        McbDeleteNode(&apoEntry->mpoEntries[liIt]);
    }

    apoEntry->muMax = 0;
    apoEntry->muCount = 0;

    ACL_FREE(apoEntry->mpoEntries);
    apoEntry->mpoEntries = NULL;
    
    if(apoEntry->mpcName)
    {
        ACL_FREE(apoEntry->mpcName);
        apoEntry->mpcName = NULL;
    }
}

void McbAttachNodes( McbXMLNode* apoDst, McbXMLNode* apoSrc, apl_size_t aiNum )
{
    for ( apl_size_t liIt = 0; liIt < aiNum; liIt++ )
    {
        apoDst[liIt] = apoSrc[liIt];
        apoSrc[liIt].miType = NODE_EMPTY;
    }
}

void McbAllocNodes( McbXMLElement* apoEntry, apl_size_t aiGrowBy )
{
    apl_size_t  liMax;
    McbXMLNode* lpoNewNode;

    apoEntry->muMax += aiGrowBy;
    liMax = apoEntry->muMax;

    ACL_MALLOC(lpoNewNode, McbXMLNode, sizeof(McbXMLNode) * liMax);
    McbAttachNodes(lpoNewNode, apoEntry->mpoEntries, apoEntry->muCount);

    if(apoEntry->mpoEntries)
    {
        ACL_FREE(apoEntry->mpoEntries);
        apoEntry->mpoEntries = NULL;
    }

    apoEntry->mpoEntries = lpoNewNode;
}

McbClearTag* McbGetClearTags( ETagType aeType = TAG_UNKNOWN )
{
    static McbClearTag _tags[] =
    {
        /* cpcOpen      cpcClose    */
        { "<![CDATA[",  "]]>"       },
        { "<PRE>",      "</PRE>"    },
        { "<Script>",   "</Script>" },
        { "<!--",       "-->"       },
        { "<!DOCTYPE",  ">"         },
        { "<?",         "?>"        },
        { NULL,         NULL        }
    };
    
    if(aeType == TAG_UNKNOWN)
    {
        return _tags;
    }
    else
    {
        return &_tags[aeType];
    }
}

McbXMLElement* McbAddElement( McbXMLElement* apoEntry, char* apcName, apl_int_t aiIsDeclaration, apl_size_t aiGrowBy )
{
    McbXMLNode*    lpoNode;
    McbXMLElement* lpoElement;
    
    if(apoEntry->muCount == apoEntry->muMax)
    {
        McbAllocNodes(apoEntry, aiGrowBy);
    }

    lpoNode = &apoEntry->mpoEntries[apoEntry->muCount];
    lpoNode->miType = NODE_ELEMENT;

    ACL_MALLOC( lpoElement, McbXMLElement, sizeof(McbXMLElement) );
    McbInitElement(lpoElement, apoEntry, apcName, aiIsDeclaration);

    lpoNode->mnNode.mpoElement = lpoElement;
    apoEntry->muCount++;

    return lpoElement;
}

McbXMLAttribute* McbAddAttribute( McbXMLElement* apoEntry, char* apcName, char* apcValue, apl_size_t aiGrowBy)
{
    McbXMLNode*      lpoNode;
    McbXMLAttribute* lpoAttrib;

    if(apoEntry->muCount == apoEntry->muMax)
    {
        McbAllocNodes(apoEntry, aiGrowBy);
    }

    lpoNode = &apoEntry->mpoEntries[apoEntry->muCount];
    lpoNode->miType = NODE_ATTRIBUTE;

    ACL_MALLOC( lpoAttrib, McbXMLAttribute, sizeof(McbXMLAttribute) );
    lpoAttrib->mpcValue = apcValue;
    lpoAttrib->mpcName = apcName;

    lpoNode->mnNode.mpoAttrib = lpoAttrib;
    apoEntry->muCount++;

    return lpoAttrib;
}

McbXMLText* McbAddText( McbXMLElement* apoEntry, char* apcValue, apl_size_t aiGrowBy )
{
    McbXMLNode      *lpoNode;
    McbXMLText      *lpoText;

    if(apoEntry->muCount == apoEntry->muMax)
    {
        McbAllocNodes(apoEntry, aiGrowBy);
    }

    lpoNode = &apoEntry->mpoEntries[apoEntry->muCount];
    lpoNode->miType = NODE_TEXT;

    ACL_MALLOC( lpoText, McbXMLText, sizeof(McbXMLText) );
    lpoText->mpcValue = apcValue;

    lpoNode->mnNode.mpoText = lpoText;
    apoEntry->muCount++;

    return lpoText;
}

McbXMLClear* McbAddClear( McbXMLElement* apoEntry, char* apcValue, McbClearTag* apoClear, apl_size_t aiGrowBy )
{
    McbXMLNode      *lpoNode;
    McbXMLClear     *lpoNewClear;

    if(apoEntry->muCount == apoEntry->muMax)
    {
        McbAllocNodes(apoEntry, aiGrowBy);
    }

    lpoNode = &apoEntry->mpoEntries[apoEntry->muCount];
    lpoNode->miType = NODE_CLEAR;

    ACL_MALLOC(lpoNewClear, McbXMLClear, sizeof(McbXMLClear));
    lpoNewClear->mpcCloseTag = apoClear->cpcClose;
    lpoNewClear->mpcOpenTag = apoClear->cpcOpen;
    lpoNewClear->mpcValue = apcValue;

    lpoNode->mnNode.mpoClear = lpoNewClear;
    apoEntry->muCount++;

    return lpoNewClear;
}

McbXMLClear* McbAddClear( McbXMLElement* apoEntry, ETagType aeClearType, char* apcValue, apl_size_t aiGrowBy )
{
    McbClearTag* lpoClear = McbGetClearTags(aeClearType);
    
    return McbAddClear(apoEntry, apcValue, lpoClear, aiGrowBy);
}

McbXMLNode* McbEnumNodes( McbXMLElement* apoEntry, apl_size_t* apiIndex )
{   
    McbXMLNode      *lpoResult = NULL;

    if(*apiIndex < apoEntry->muCount)
    {
        lpoResult = &apoEntry->mpoEntries[*apiIndex];
        (*apiIndex)++;
    }

    return lpoResult;
}

McbXMLElement* McbEnumElements( McbXMLElement* apoEntry, apl_size_t* apiIndex )
{   
    apl_size_t      liIndex;
    McbXMLElement* lpoResult = NULL;

    liIndex = *apiIndex;
    for(; liIndex < apoEntry->muCount && !lpoResult; liIndex++)
    {
        if(apoEntry->mpoEntries[liIndex].miType == NODE_ELEMENT)
        {
            lpoResult = apoEntry->mpoEntries[liIndex].mnNode.mpoElement;
        }
    }

    *apiIndex = liIndex;
    
    return lpoResult;
}

McbXMLAttribute* McbEnumAttributes( McbXMLElement* apoEntry, apl_size_t* apiIndex )
{   
    apl_size_t liIndex;
    McbXMLAttribute* lpoResult = NULL;

    liIndex = *apiIndex;
    for(; liIndex < apoEntry->muCount && !lpoResult; liIndex++)
    {
        if(apoEntry->mpoEntries[liIndex].miType == NODE_ATTRIBUTE)
        {
            lpoResult = apoEntry->mpoEntries[liIndex].mnNode.mpoAttrib;
        }
    }

    *apiIndex = liIndex;
    
    return  (lpoResult);
}

void McbFindEndOfText( char const* apcToken, apl_size_t* apiText )
{
    char        liChar;
    apl_size_t  liTextLength;

    liTextLength = (*apiText)-1;    
    while(true)
    {
        liChar = apcToken[liTextLength];

        switch(liChar)
        {
            case '\r':
            case '\n':
            case '\t':
            case ' ':
                liTextLength--;
                break;
    
            default:
                *apiText = (liTextLength+1);
                return;
        }
    }
}

int McbParseClearTag(McbXML* apoXML, McbXMLElement* apoElement, McbClearTag* apoClear)
{
    apl_size_t  liLength = 0;
    char*       lpcTemp = NULL;
    const char* lpcXML = &apoXML->cpcXML[apoXML->ciIndex];

    lpcTemp = apl_strstr(lpcXML, apoClear->cpcClose);

    if(lpcTemp)
    {
        liLength = lpcTemp - lpcXML;

        apoXML->ciIndex += liLength;
        apoXML->ciIndex += apl_strlen(apoClear->cpcClose);
        
        lpcTemp = McbStrdup(lpcXML, liLength);
        McbAddClear(apoElement, lpcTemp, apoClear, MCB_GROWBY);

        return 1;
    }

    apoXML->miErrCode = ERROR_UNMATCHED_END_TAG;
    
    return 0;
}

int McbParseXMLElement( McbXML* apoXML, McbXMLElement* apoElement )
{
    apl_size_t  liTempLength;
    apl_size_t  liTokenLength;
    apl_size_t  liDeclaration;
    char*       lpcDupString;
    char const* lpcTemp = NULL;
    char const* lpcText = NULL;
    char const* lpcToken = NULL;

    EMcbStatus      liStatus;
    McbNextToken    loToken;
    apl_int_t       liTokenType;
    McbXMLElement*  lpoNewElement;
    EMcbAttribute   liAttrib = ATTRIBUTE_NAME;

    if(apoXML->ciFirst)
    {
        apoXML->ciFirst = 0;
        liStatus = OUTSIDE_TAG;
    }
    else 
    {
        liStatus = INSIDE_TAG;
    }

    while(true)
    {
        loToken = McbGetNextToken(apoXML, &liTokenLength, &liTokenType);

        if(liTokenType == TOKEN_ERROR)
        {
            //daizh 2010-3-11
            //case: <a>1<a><b>2</b>
            if (apoElement->mpcName != NULL)
            {
                apoXML->miErrCode = ERROR_UNMATCHED_END_TAG;
            }

            return 0;
        }

        if(liStatus == OUTSIDE_TAG)
        {
            switch(liTokenType)
            {
                case TOKEN_TEXT:
                case TOKEN_QUOTED_TEXT:
                case TOKEN_EQUALS:
                    if(lpcText == NULL)
                    {
                        lpcText = loToken.cpcStrBegin;
                    }                   
                    break;              
    
                case TOKEN_TAG_START:
                case TOKEN_DECLARATION:
                    liDeclaration = (liTokenType == TOKEN_DECLARATION);
                    if(lpcText)
                    {
                        liTempLength = loToken.cpcStrBegin - lpcText;
                        McbFindEndOfText(lpcText, &liTempLength);
        
                        lpcDupString = McbStrdup(lpcText, liTempLength);
                        McbAddText(apoElement, lpcDupString, MCB_GROWBY);
                        lpcText = NULL;
                    }
    
                    loToken = McbGetNextToken(apoXML, &liTokenLength, &liTokenType);
                    if(liTokenType != TOKEN_TEXT)
                    {
                        apoXML->miErrCode = ERROR_MISSING_TAG_NAME;
                        return 0;
                    }
    
                    if(apoElement->mpcName && apl_strcmp(apoElement->mpcName, loToken.cpcStrBegin) == 0)
                    {
                        //DANGER: if matched success, case <a>1<a><b>2</b> will be cause coredump
                        //FIXED: no
                        apoXML->cpcNewElement = loToken.cpcStrBegin;
                        apoXML->ciNewElementLen = liTokenLength;
        
                        return 1;
                    }
                    else
                    {
                        lpcDupString = McbStrdup(loToken.cpcStrBegin, liTokenLength);
                        lpoNewElement = McbAddElement(apoElement, lpcDupString, liDeclaration, MCB_GROWBY);
        
                        while(lpoNewElement)
                        {
                            if(McbParseXMLElement(apoXML, lpoNewElement) == 0)
                                return 0;

                            if(apoXML->ciEndTagLen)
                            {
                                if(apoElement->mpcName == NULL)
                                {
                                    apoXML->miErrCode = ERROR_UNMATCHED_END_TAG;
                                    return 0;
                                }
        
                                if(apl_strncmp(apoXML->cpcEndTag, apoElement->mpcName, \
                                        apl_strlen(apoElement->mpcName)) == 0)
                                {                                    
                                    apoXML->ciEndTagLen = 0;
                                }
        
                                return 1;
                            }
                            
                            if(apoXML->ciNewElementLen)
                            {
                                if(apl_strncmp(apoXML->cpcNewElement, apoElement->mpcName, 
                                        apl_strlen(apoElement->mpcName)) == 0)
                                {                                    
                                    return 1;
                                }
        
                                lpcDupString = McbStrdup(apoXML->cpcNewElement, apoXML->ciNewElementLen);
                                lpoNewElement = McbAddElement(apoElement, lpcDupString, 0, MCB_GROWBY);
        
                                apoXML->ciNewElementLen = 0;
                            }
                            else 
                            {
                                lpoNewElement = NULL;
                            }
                        }
                    }
                    break;
    
                case TOKEN_TAG_END:
                    if(lpcText)
                    {
                        liTempLength = loToken.cpcStrBegin - lpcText;
                        McbFindEndOfText(lpcText, &liTempLength);
        
                        lpcDupString = McbStrdup(lpcText, liTempLength);
                        McbAddText(apoElement, lpcDupString, MCB_GROWBY);
                        lpcText = NULL;
                    }
                            
                    loToken = McbGetNextToken(apoXML, &liTempLength, &liTokenType);                 
                    if(liTokenType != TOKEN_TEXT)
                    {
                        apoXML->miErrCode = ERROR_MISSING_END_TAG_NAME;
                        return 0;
                    }
        
                    lpcTemp = loToken.cpcStrBegin;
                    loToken = McbGetNextToken(apoXML, &liTokenLength, &liTokenType);
        
                    if(liTokenType != TOKEN_CLOSE_TAG)
                    {
                        apoXML->miErrCode = ERROR_MISSING_END_TAG_NAME;
                        return 0;
                    }
        
                    //added by max at 2006.03.31
                    //for the case: </abc><aa>123</aa>
                    if(apoElement->mpcName == NULL)
                    {
                        break;
                    }
                            
                    if(apl_strncmp(lpcTemp, apoElement->mpcName, apl_strlen(apoElement->mpcName)))
                    {
                        apoXML->cpcEndTag = lpcTemp;
                        apoXML->ciEndTagLen = liTempLength;
                        
                        //daizh 2010-3-11
                        //case <x><a>1<a><b>2</b></x>
                        apoXML->miErrCode = ERROR_MISSING_END_TAG_NAME;
                        return 0;
                    }
        
                    return 1;
    
                case TOKEN_CLEAR:
                    if(lpcText)
                    {
                        liTempLength = loToken.cpcStrBegin - lpcText;
                        McbFindEndOfText(lpcText, &liTempLength);

                        lpcDupString = McbStrdup(lpcText, liTempLength);
                        McbAddText(apoElement, lpcDupString, MCB_GROWBY);
                        lpcText = NULL;
                    }
    
                    if(McbParseClearTag(apoXML, apoElement, loToken.mpoClear) == 0)
                    {
                        return 0;
                    }
                    break;
    
                case TOKEN_CLOSE_TAG:        /* '>'   */
                case TOKEN_SHORT_HAND_CLOSE:  /* '/>'  */
                    apoXML->miErrCode = ERROR_UNEXPECTED_TOKEN;
                    return 0;
            
                case TOKEN_ERROR:
                default:
                    break;
            }
        }
        else if(liStatus == INSIDE_TAG)
        {
            switch(liAttrib)
            {
                case ATTRIBUTE_NAME:
                    switch(liTokenType)
                    {
                        case TOKEN_TEXT:
                            lpcTemp = loToken.cpcStrBegin;
                            liTempLength = liTokenLength;
                            liAttrib = ATTRIBUTE_EQUALS;
                            break;
            
                        case TOKEN_CLOSE_TAG:
                            liStatus = OUTSIDE_TAG;
                            break;
            
                        case TOKEN_SHORT_HAND_CLOSE:
                            return 1;
            
                        case TOKEN_QUOTED_TEXT:  /* '"SomeText"' */
                        case TOKEN_TAG_START:    /* '<'          */
                        case TOKEN_TAG_END:      /* '</'         */
                        case TOKEN_EQUALS:      /* '='          */
                        case TOKEN_DECLARATION: /* '<?'         */
                        case TOKEN_CLEAR:
                            apoXML->miErrCode = ERROR_UNEXPECTED_TOKEN;
                            return 0;
                                    
                        case TOKEN_ERROR:
                        default:
                            break;
                    }
                    break;
        
                case ATTRIBUTE_EQUALS:
                    switch(liTokenType)
                    {
                        case TOKEN_TEXT:
                            lpcDupString = McbStrdup(lpcTemp, liTempLength);
                            McbAddAttribute(apoElement, lpcDupString, NULL, MCB_GROWBY);
            
                            lpcTemp = loToken.cpcStrBegin;
                            liTempLength = liTokenLength;
                            break;
            
                        case TOKEN_SHORT_HAND_CLOSE:
                        case TOKEN_CLOSE_TAG:
                            if(apoElement->muIsDeclaration && (lpcTemp[liTempLength-1]) == '?')
                            {
                                liTempLength--;
                            }
            
                            if(liTempLength)
                            {
                                lpcDupString = McbStrdup(lpcTemp, liTempLength);
                                McbAddAttribute(apoElement, lpcDupString, NULL, MCB_GROWBY);
                            }
            
                            if(liTokenType == TOKEN_SHORT_HAND_CLOSE)
                            {
                                return 1;
                            }
                            
                            liStatus = OUTSIDE_TAG;
                            break;
            
                        case TOKEN_EQUALS:
                            liAttrib = ATTRIBUTE_VALUE;
                            break;
            
                        case TOKEN_QUOTED_TEXT:  /* 'Attribute "InvalidAttr"'*/
                        case TOKEN_TAG_START:    /* 'Attribute <'            */
                        case TOKEN_TAG_END:      /* 'Attribute </'           */
                        case TOKEN_DECLARATION: /* 'Attribute <?'           */
                        case TOKEN_CLEAR:
                            apoXML->miErrCode = ERROR_UNEXPECTED_TOKEN;
                            return 0;
        
                        case TOKEN_ERROR:
                        default:
                            break;  
                    }
                    break;
        
                case ATTRIBUTE_VALUE:
                    switch(liTokenType)
                    {
                        case TOKEN_TEXT:
                            if(apoElement->muIsDeclaration && 
                                (loToken.cpcStrBegin[liTokenLength-1]) == '?')
                            {
                                liTokenLength--;
                            }
            
                            if(liTempLength)
                            {
                                lpcToken = McbStrdup(loToken.cpcStrBegin, liTokenLength);
                            }
                            else
                            {
                                lpcToken = NULL;
                            }
            
                            lpcDupString = McbStrdup(lpcTemp, liTempLength);
                            McbAddAttribute(apoElement, lpcDupString, (char *)lpcToken, MCB_GROWBY);
            
                            liAttrib = ATTRIBUTE_NAME;
                            break;
                        ///FIXED by daizh
                        case TOKEN_QUOTED_TEXT:
                            if(apoElement->muIsDeclaration && 
                                (loToken.cpcStrBegin[liTokenLength-1]) == '?')
                            {
                                liTokenLength--;
                            }
            
                            if(liTempLength)
                            {
                                if (liTokenLength == 2)
                                {
                                    lpcToken = McbStrdup("", 0);
                                }
                                else
                                {
                                    lpcToken = McbStrdup(loToken.cpcStrBegin + 1, liTokenLength - 2);
                                }
                            }
                            else
                            {
                                lpcToken = NULL;
                            }
            
                            lpcDupString = McbStrdup(lpcTemp, liTempLength);
                            McbAddAttribute(apoElement, lpcDupString, (char *)lpcToken, MCB_GROWBY);
            
                            liAttrib = ATTRIBUTE_NAME;
                            break;
            
                        case TOKEN_TAG_START:        /* 'Attr = <'   */
                        case TOKEN_TAG_END:          /* 'Attr = </'  */
                        case TOKEN_CLOSE_TAG:        /* 'Attr = >'   */
                        case TOKEN_SHORT_HAND_CLOSE:  /* "Attr = />"  */
                        case TOKEN_EQUALS:          /* 'Attr = ='   */
                        case TOKEN_DECLARATION:     /* 'Attr = <?'  */
                        case TOKEN_CLEAR:
                            apoXML->miErrCode = ERROR_UNEXPECTED_TOKEN;
                            return 0;
                            break;
                                
                        case TOKEN_ERROR:
                        default:
                            break;
                }
            }
        }
    }
}

void McbCountLinesAndColumns( char const* apcXML, apl_int_t liUpto, McbXMLResults* apoResults )
{
    char liChar;

    apoResults->miLine = 1;
    apoResults->miColumn = 1;

    for(int liIt = 0; liIt < liUpto; liIt++)
    {
        liChar = apcXML[liIt];
        assert(liChar);

        if(liChar == '\n')
        {
            apoResults->miLine++;
            apoResults->miColumn = 1;
        }
        else
        {
            apoResults->miColumn++;
        }
    }
}

McbXMLElement* McbParseXML( char const* apcXML, McbXMLResults* apoResults )
{
    apl_int_t      liErrCode;
    McbXMLElement* lpoRootElement = NULL;
    McbXML loInitXML = { NULL, 0, ERROR_NONE, NULL, 0, NULL, 0, 1, NULL };

    loInitXML.cpcXML = apcXML;
    loInitXML.mpoClearTags = McbGetClearTags();

    lpoRootElement = McbCreateRoot();
    McbParseXMLElement(&loInitXML, lpoRootElement);
    liErrCode = loInitXML.miErrCode;

    if(liErrCode != ERROR_NONE)
    {
        McbDeleteRoot(lpoRootElement);
        lpoRootElement = NULL;
    }

    if(apoResults)
    {
        apoResults->miErrCode = liErrCode;

        if(liErrCode != ERROR_NONE)
        {
            McbCountLinesAndColumns(loInitXML.cpcXML, loInitXML.ciIndex, apoResults);
        }
    }

    return lpoRootElement;
}

McbXMLElement* McbFindElement( McbXMLElement* apoEntry, char const* apcPath )
{
    apl_size_t     liIndex;
    apl_size_t     liNameLength;
    apl_size_t     liComResult = 0;
    char const*    lpcName = NULL;
    char const*    lpcNext = NULL;
    McbXMLElement* lpoChild = NULL;

    lpcNext = apl_strchr(apcPath, '/');

    if(lpcNext == NULL)
    {
        liNameLength = apl_strlen(apcPath);
    }
    else
    {
        liNameLength = lpcNext - apcPath;

        if(lpcNext[1])
        {
            lpcNext++;
        }
        else
        {
            lpcNext = NULL;
        }
    }

    if(liNameLength)
    {
        liIndex = 0;
        while( ( lpoChild = McbEnumElements(apoEntry, &liIndex) ) != NULL)
        {
            lpcName = lpoChild->mpcName;

            if(lpcName)
            {
                if(apl_strchr(apcPath, '/') == NULL)
                {
                    liComResult = apl_strcmp(apcPath, lpcName);
                }
                else
                {
                    liComResult = apl_strncmp(apcPath, lpcName, liNameLength);
                }

                if(liComResult == 0)
                {
                    if(lpcNext == NULL)
                    {
                        return lpoChild;
                    }
                    else 
                    {
                        return McbFindElement(lpoChild, lpcNext);
                    }
                }
            }
        }
    }

    return NULL;
}

apl_size_t McbFindElements( McbXMLElement* apoHead, char const* apcName, McbXMLElement** apoElemArray, apl_size_t aiCount )
{
    apl_size_t      liIndex      = 0;
    apl_size_t      liNameLen    = apl_strlen(apcName) + 1;
    apl_size_t      liElemCount  = 0;
    char*           lpcTmpName   = NULL;
    char*           lpcLeafName  = NULL;
    McbXMLElement*  lpoElemEntry = apoHead;
    McbXMLElement*  lpoChild     = NULL;
    
    ACL_MALLOC_ASSERT( lpcTmpName, char, liNameLen );
        
    apl_strncpy( lpcTmpName, apcName, liNameLen );
	apl_memset( apoElemArray, 0, sizeof(McbXMLElement *) * aiCount );
    
    do
    {
        // Skip redundant '/' last, e.g "root_name/next_name//" 
        while( (lpcLeafName = apl_strrchr( lpcTmpName, '/' )) != NULL )
        {
            if ( apl_strlen(lpcLeafName) > 1 ) break;
            
            *lpcLeafName = '\0';
        }
        
        if ( lpcLeafName != NULL )
        {
            // Yes, it's multi-directory path, e.g "root_name/next_name/last_name" 
            *lpcLeafName = '\0';
            lpcLeafName++;
            lpoElemEntry = McbFindElement( apoHead, lpcTmpName );
        }
        else
        {
            lpcLeafName = lpcTmpName;
        }
        
        // No, can't find entry element, break it
        if ( lpoElemEntry == NULL ) break;
        
        // Poll to find target element
        while( (lpoChild = McbEnumElements(lpoElemEntry, &liIndex)) != NULL )
        {          
            if ( liElemCount >= aiCount ) break;

            if( apl_strcmp(lpcLeafName, lpoChild->mpcName) == 0 )
            {
                apoElemArray[liElemCount++] = lpoChild;
            }
        }
        
    }while(false);
    
    ACL_FREE( lpcTmpName );
    
    return liElemCount;
}

McbXMLAttribute* McbFindAttribute( McbXMLElement* apoEntry, char const* apcAttribute )
{
    apl_size_t       liIndex;
    apl_size_t       liAttribLength;
    McbXMLAttribute* lpoAttribute = NULL;

    liIndex = 0;
    liAttribLength = apl_strlen(apcAttribute);

    while( ( lpoAttribute = McbEnumAttributes(apoEntry, &liIndex) ) != NULL)
    {
        if(apl_strncmp(lpoAttribute->mpcName, apcAttribute, liAttribLength) == 0)
        {
            return lpoAttribute;
        }
    }

    return NULL;
}

McbXMLElement* McbCreateElements( McbXMLElement* apoEntry, char const* apcPath)
{
    apl_size_t     liIndex;
    apl_size_t     liNameLength;
    apl_size_t     liComResult = 0;
    char*          lpcTemp = NULL;
    char const*    lpcName = NULL;
    char const*    lpcNext = NULL;
    McbXMLElement* lpoChild = NULL;
    McbXMLElement* lpoNewElement = NULL;
    
    if(lpcNext = apl_strchr(apcPath, '/'), lpcNext == NULL)
    {
        liNameLength = apl_strlen(apcPath);     
    }
    else
    {
        liNameLength = lpcNext - apcPath;

        if(lpcNext[1])
        {
            lpcNext++;
        }
        else
        {
            lpcNext = NULL;
        }
    }

    if(liNameLength)
    {
        liIndex = 0;
        while( ( lpoChild = McbEnumElements(apoEntry, &liIndex) ) != NULL)
        {
            lpcName = lpoChild->mpcName;

            if(lpcName)
            {
                if(apl_strchr(apcPath, '/') == NULL)
                {
                    liComResult = apl_strcmp(apcPath, lpcName);
                }
                else
                {
                    liComResult = apl_strncmp(apcPath, lpcName, liNameLength);
                }

                if(liComResult == 0)
                {
                    if(lpcNext == NULL)
                    {
                        return lpoChild;
                    }
                    else 
                    {
                        return McbCreateElements(lpoChild, lpcNext);
                    }
                }
            }
        }

        lpcTemp = McbStrdup(apcPath, liNameLength);
        lpoNewElement = McbAddElement(apoEntry, lpcTemp, 0, MCB_GROWBY);

        if(lpcNext == NULL)
        {
            return lpoNewElement;
        }
        else 
        {
            return McbCreateElements(lpoNewElement, lpcNext);
        }
    }

    return NULL;
}

apl_size_t McbCreateXMLStringR( McbXMLElement* apoEntry, char* apcMarker, apl_int_t aiFormat)
{
    apl_size_t       liIndex;
    apl_size_t       liLength;
    apl_size_t       liResult = 0;
    apl_int_t        liChildFormat;
    apl_size_t       liElementLength;
    apl_size_t       liHasChildren = 0;
    apl_size_t       liHasTextChildren = 0;
    McbXMLNode*      lpoChildNode = NULL;
    McbXMLAttribute* lpoAttrib = NULL;

    liElementLength = MCB_LENSTR(apoEntry->mpcName);

    if(liElementLength)
    {
        liLength = (aiFormat == -1 ? 0 : aiFormat);

        if(apcMarker)
        {
            if(liLength)
            {
                apl_memset(apcMarker, MCB_INDENTCHAR, liLength);
            }

            liResult = liLength;
            apcMarker[liResult++] = '<';        
            McbStrCpy(&apcMarker[liResult], apoEntry->mpcName);
            
            liResult += liElementLength;
            apcMarker[liResult++] = ' ';        
        }                
        else
        {
            liResult += (liLength + liElementLength + 2);
        }
        
        liIndex = 0;
        while((lpoChildNode = McbEnumNodes(apoEntry, &liIndex)) != NULL)
        {
            switch(lpoChildNode->miType)
            {
                case NODE_ATTRIBUTE:
                {
                    lpoAttrib = lpoChildNode->mnNode.mpoAttrib;
                    liLength = MCB_LENSTR(lpoAttrib->mpcName);
    
                    if(liLength)
                    {
                        if(apcMarker)
                        {
                            McbStrCpy(&apcMarker[liResult], lpoAttrib->mpcName);             
                        }
    
                        liResult += liLength;
                        liLength = MCB_LENSTR(lpoAttrib->mpcValue);
    
                        if(liLength)
                        {
                            if (lpoAttrib->mpcValue[0] == '\"'
                                || lpoAttrib->mpcValue[0] == '\'')
                            {
                                if(apcMarker)
                                {
                                    apcMarker[liResult] = (char)'=';
                                    McbStrCpy(&apcMarker[liResult + 1], lpoAttrib->mpcValue);
                                }
                                liResult += (liLength + 1);
                            }
                            else
                            {
                                if(apcMarker)
                                {
                                    apcMarker[liResult] = (char)'=';
                                    apcMarker[liResult + 1] = (char)'\"';
                                    McbStrCpy(&apcMarker[liResult + 2], lpoAttrib->mpcValue);
                                    apcMarker[liResult + 2 + liLength] = (char)'\"';
                                }
                                liResult += (liLength + 3);
                            }
                        }
                        else
                        {
                            if(apcMarker)
                            {
                                apcMarker[liResult] = (char)'=';
                                apcMarker[liResult + 1] = (char)'\"';
                                apcMarker[liResult + 2] = (char)'\"';
                            }
                            liResult += 3;
                        }
    
                        if(apcMarker)
                        {
                            apcMarker[liResult] = (char)' ';                              
                        }
    
                        liResult++;
                    }
                }
                break;
    
                case NODE_TEXT:
                    liHasChildren = 1;
                    liHasTextChildren = 1;
                    break;

                case NODE_EMPTY:
                    continue;
    
                default:
                    liHasChildren = 1;
                    break;
            }
        }
        
        if(liHasChildren)
        {   
            if(apcMarker)
            {
                apcMarker[liResult-1] = (char)'>';
            }

            if(aiFormat != (-1) && !liHasTextChildren)//text node needn't format
            {
                if(apcMarker)
                {
                    apcMarker[liResult] = (char)'\n';
                }

                liResult++;
            }
        }
        else
        {
            liResult--;
        }
    }

    if(aiFormat == -1)
    {
        liChildFormat = -1;
    }
    else
    {
        if(liElementLength) 
        {
            liChildFormat = aiFormat + 1;
        }
        else
        {
            liChildFormat = aiFormat;
        }   
    }

    liIndex = 0;
    while((lpoChildNode = McbEnumNodes(apoEntry, &liIndex)) != NULL)
    {
        switch(lpoChildNode->miType)
        {
            case NODE_TEXT:
            {
                liLength = MCB_LENSTR(lpoChildNode->mnNode.mpoText->mpcValue);
    
                if(liLength)
                {
                    if(false/*aiFormat != -1*/)//text node needn't format
                    {
                        if(apcMarker)
                        {
                            apl_memset(&apcMarker[liResult], MCB_INDENTCHAR, aiFormat + 1);
                            McbStrCpy(&apcMarker[liResult + aiFormat + 1], lpoChildNode->mnNode.mpoText->mpcValue);
                            apcMarker[ liResult + aiFormat + 1 + liLength] = (char)'\n';
                        }
    
                        liResult += (liLength + aiFormat + 2);
                    }
                    else
                    {
                        if(apcMarker)
                        {
                            McbStrCpy(&apcMarker[liResult], lpoChildNode->mnNode.mpoText->mpcValue);
                        }
    
                        liResult += liLength;
                    }               
                }
            }
            break;
    
            case NODE_CLEAR:
            {
                liLength = MCB_LENSTR(lpoChildNode->mnNode.mpoClear->mpcOpenTag);
            
                if(liLength)
                {
                    if(aiFormat != -1)
                    {
                        if(apcMarker)
                        {
                            apl_memset(&apcMarker[liResult], MCB_INDENTCHAR, aiFormat + 1);
                            McbStrCpy(&apcMarker[liResult + aiFormat + 1], lpoChildNode->mnNode.mpoClear->mpcOpenTag);
            
                            /* apcMarker[liResult + aiFormat + 1 + liLength] = (char)'\n'; */
                        }
            
                        /* liResult += (liLength + aiFormat + 2); */
                        liResult += (liLength + aiFormat + 1);
                    }
                    else
                    {
                        if(apcMarker)
                        {
                            McbStrCpy(&apcMarker[liResult], lpoChildNode->mnNode.mpoClear->mpcOpenTag);
                        }
            
                        liResult += liLength;
                    }               
                }
            
                liLength = MCB_LENSTR(lpoChildNode->mnNode.mpoClear->mpcValue);
            
                if(liLength)
                {
                    if(apcMarker)
                    {
                        McbStrCpy(&apcMarker[liResult], lpoChildNode->mnNode.mpoClear->mpcValue);
                    }
            
                    liResult += liLength;
                }
            
                liLength = MCB_LENSTR(lpoChildNode->mnNode.mpoClear->mpcCloseTag);
            
                if(liLength)
                {
                    if(apcMarker)
                    {
                        McbStrCpy(&apcMarker[liResult], lpoChildNode->mnNode.mpoClear->mpcCloseTag);
                    }
            
                    liResult += liLength;
                }
            
                if(aiFormat != -1)
                {
                    if(apcMarker)
                    {
                        apcMarker[liResult] = (char)'\n';
                    }
            
                    liResult++;
                }
            }
            break;
                
            case NODE_ELEMENT:
            {
                liResult += McbCreateXMLStringR(lpoChildNode->mnNode.mpoElement, 
                    (apcMarker ? apcMarker + liResult : NULL), liChildFormat);
            }
    
            default:
                break;
        }
    }

    if(liElementLength)
    {
        if(liHasChildren)
        {
            if(apcMarker)
            {
                if(aiFormat != -1 && !liHasTextChildren)//text node needn't format
                {
                    if(aiFormat)
                    {
                        apl_memset(&apcMarker[liResult], MCB_INDENTCHAR, aiFormat);
                        liResult += aiFormat;
                    }
                }       

                McbStrCpy(&apcMarker[liResult], "</");        
                liResult += 2;
                
                McbStrCpy(&apcMarker[liResult], apoEntry->mpcName);
                liResult += liElementLength;
                
                if(aiFormat == -1)
                {
                    McbStrCpy(&apcMarker[liResult], ">");
                    liResult++;
                }
                else
                {
                    McbStrCpy(&apcMarker[liResult], ">\n");
                    liResult += 2;
                }
            }
            else
            {
                if(aiFormat != -1 )
                {
                    if (!liHasTextChildren)//text node needn't format
                    {
                        liResult += (liElementLength + 4 + aiFormat);
                
                    }
                    else
                    {
                        liResult += (liElementLength + 4);
                    }
                }
                else
                {
                    liResult += (liElementLength + 3);
                }
            }
        }
        else 
        {
            if(apcMarker)
            {
                if(aiFormat == -1)
                {
                    McbStrCpy(&apcMarker[liResult], "/>"); 
                    liResult += 2;
                }
                else
                {
                    McbStrCpy(&apcMarker[liResult], "/>\n"); 
                    liResult += 3;
                }
            }
            else
            {
                liResult += (aiFormat == -1 ? 2 : 3);
            }
        }
    }

    return liResult;
}

char *McbCreateXMLString( McbXMLElement* apoHead, apl_int_t aiFormat, apl_size_t* apiSize)
{
    apl_int_t  liFormat;
    apl_size_t liStrLength;
    char*      lpcResult = NULL;

    if(apoHead)
    {
        liFormat = (aiFormat ? 0 : -1);
        
        if( ( liStrLength = McbCreateXMLStringR(apoHead, NULL, liFormat) ) <= 0 )
        {
            return NULL;
        }

        ACL_MALLOC(lpcResult, char, (liStrLength+1));

        McbCreateXMLStringR(apoHead, lpcResult, liFormat);
        lpcResult[liStrLength] = '\0';
        
        if(apiSize)
        {
            *apiSize = liStrLength;
        }
        
        return lpcResult;
    }

    return NULL;
}

}

ACL_NAMESPACE_END
