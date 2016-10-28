
#include "AIMcbXML.h"

AIBC_NAMESPACE_START

#define MCB_GROWBY            (5)
#define MCB_INDENTCHAR        ('\t')
#define MCB_LENSTR(str)       (str ? strlen(str) : 0)

/////////////////////////////////////////////////////////////////////////
//
typedef enum McbStatus
{
    eInsideTag = 0,
    eOutsideTag
} McbStatus;

typedef enum McbAttrib
{
    eAttribName = 0,
    eAttribEquals,
    eAttribValue
} McbAttrib;

typedef enum McbTokenType
{
    eTokenNone = -1,
    eTokenText = 0,
    eTokenQuotedText,
    eTokenTagStart,         /* "<"   */
    eTokenTagEnd,           /* "</"  */
    eTokenCloseTag,         /* ">"   */
    eTokenEquals,           /* "="   */
    eTokenDeclaration,      /* "<?"  */
    eTokenShortHandClose,   /* "/>"  */
    eTokenClear,
    eTokenError
} McbTokenType;

/////////////////////////////////////////////////////////////////////////
//
typedef struct McbClearTag
{
    const char          *cpcOpen;
    const char          *cpcClose;
} McbClearTag;

typedef struct McbNextToken
{
    McbClearTag         *cpoClear;
    const char          *cpcStrBegin;
} McbNextToken;

typedef struct McbXML
{
    const char          *cpcXML;
    int                 ciIndex;
    McbXMLError         ciErrCode;
    const char          *cpcEndTag;
    int                 ciEndTagLen;
    const char          *cpcNewElement;
    int                 ciNewElementLen;
    int                 ciFirst;
    McbClearTag         *cpoClearTags;
} McbXML;

void McbStrCpy( char* apcDest, char const* apcSrc )
{
    while( *apcSrc != '\0' )
    {
        *apcDest++ = *apcSrc++;
    }
}

void McbInitElement( McbXMLElement* apoEntry, McbXMLElement* apoParent, char* apcName, int aiIsDeclaration )
{
    apoEntry->ciMax = 0;
    apoEntry->ciCount = 0;
    apoEntry->cpoEntries = NULL;
    apoEntry->cpoParent = apoParent;
    apoEntry->ciIsDeclaration = aiIsDeclaration;
    apoEntry->cpcName = apcName;
}

McbXMLElement* McbCreateRoot(void)
{
    McbXMLElement* lpoElement;

    AI_MALLOC( lpoElement, McbXMLElement, sizeof(McbXMLElement) );
    
    McbInitElement(lpoElement, NULL, 0, 0);

    return lpoElement;
}

void McbDeleteRoot( McbXMLElement* apoElement )
{
    McbDeleteElement(apoElement);
    
    AI_FREE(apoElement);
}

void McbDeleteAttribute( McbXMLAttribute* apoEntry )
{
    if(apoEntry->cpcName) 
    {
        AI_FREE(apoEntry->cpcName);
        apoEntry->cpcName = NULL;
    }

    if(apoEntry->cpcValue) 
    {
        AI_FREE(apoEntry->cpcValue);
        apoEntry->cpcValue=  NULL;
    }
}

void McbAttributeAttach( McbXMLAttribute* apoDst, McbXMLAttribute* apoSrc, int aiNum )
{
    for ( int liIt = 0; liIt < aiNum; liIt++ )
    {
        apoDst[liIt].cpcName = apoSrc[liIt].cpcName;
        apoDst[liIt].cpcValue = apoSrc[liIt].cpcValue;

        apoSrc[liIt].cpcName = NULL;
        apoSrc[liIt].cpcValue = NULL;
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

char* McbStrdup( char const* apcData, int aiSize)
{
    int liSize;
    char*     lpcTemp;
        
    liSize = ( aiSize<=0 ? (int)strlen(apcData) : aiSize );
    AI_MALLOC(lpcTemp, char, liSize + 1);

    memcpy(lpcTemp, apcData, liSize);
    lpcTemp[liSize] = (char)0;

    return lpcTemp;
}

McbNextToken McbGetNextToken( McbXML* apoXML, int* apiTokenLen, McbTokenType* apiType, McbStatus aiStatus)
{
    int      liSize;
    int      liIndex;
    int       liExitFlag;
    int       liFoundMatch;
    int      liOpenLength;
    int       liIsText = 0;
    char            liChar = 0;
    char            liTempChar;
    char const*     lpcOpen;
    char const*     lpcXML;
    McbNextToken    loResult;

    liChar = McbGetNextChar(apoXML);
    memset(&loResult, 0, sizeof(McbNextToken));

    if(liChar == 0)
    {
        *apiTokenLen = 0;
        *apiType = eTokenError;
        return loResult;
    }
    
    lpcXML = apoXML->cpcXML;
    loResult.cpcStrBegin = &lpcXML[apoXML->ciIndex-1];

    liIndex = 0;
    while (true)
    {
        lpcOpen = apoXML->cpoClearTags[liIndex].cpcOpen;
        if (lpcOpen == NULL)
        {
            break;
        }

        liOpenLength = strlen(lpcOpen);
        if( strncmp(lpcOpen, loResult.cpcStrBegin, liOpenLength) == 0 )
        {
            loResult.cpoClear = &apoXML->cpoClearTags[liIndex];
            apoXML->ciIndex += (liOpenLength-1);

            *apiType = eTokenClear;
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
            if (aiStatus == eInsideTag)
            {
                *apiType = eTokenQuotedText;
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
                    apoXML->ciErrCode = eXMLErrorNoMatchingQuote;               
                    *apiType = eTokenError;
                }

                if(McbFindNonWhiteSpace(apoXML))
                {
                    apoXML->ciIndex--;
                }
            }
            else
            {
                liIsText = 1;
            }
            break;
    
        case '=':
            if (aiStatus == eInsideTag)
            {
                liSize = 1;
                *apiType = eTokenEquals;
            }
            else
            {
                liIsText = 1;
            }
            break;
    
        case '>':
            if (aiStatus == eInsideTag || *apiType == eTokenTagEnd)
            {
                liSize = 1;
                *apiType = eTokenCloseTag;
            }
            else
            {
                liIsText = 1;
            }
            break;
    
        case '<':
            liTempChar = apoXML->cpcXML[apoXML->ciIndex];
    
            if(liTempChar == '/')
            {
                McbGetNextChar(apoXML);
                *apiType = eTokenTagEnd;
                liSize = 2;
            }
            else if(liTempChar == '?')
            {
                McbGetNextChar(apoXML);
                *apiType = eTokenDeclaration;
                liSize = 2;
            }
            else 
            {
                *apiType = eTokenTagStart;
                liSize = 1;
            }
            break;
    
        case '/':
            liTempChar = apoXML->cpcXML[apoXML->ciIndex];
    
            if(liTempChar == '>')
            {
                McbGetNextChar(apoXML);
                *apiType = eTokenShortHandClose;
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

        while((liExitFlag == 0) && (liChar = McbGetNextChar(apoXML)))
        {
            if (*apiType == eTokenCloseTag || *apiType == eTokenText)
            {
                switch(liChar)
                {
                    case '<':
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
            else
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

        *apiType = eTokenText;
    }

    *apiTokenLen = liSize;
    return loResult;
}

void PrintParsingStatus(McbNextToken const& aoNextToken, int aiTokenLen, McbTokenType aiType, McbStatus aiStatus, McbAttrib aiAttribute)
{
    static char const* TOKEN_TYPE_STR[] = 
    {
        "eTokenText",
        "eTokenQuotedText",
        "eTokenTagStart",         
        "eTokenTagEnd",           
        "eTokenCloseTag",         
        "eTokenEquals",           
        "eTokenDeclaration",     
        "eTokenShortHandClose",   
        "eTokenClear",
        "eTokenError"
    };

    static char const* STATUS_STR[] =
    {
        "eInsideTag",
        "eOutsideTag"
    };

    static char const* ATTRIBUTE_STR[] =
    {
        "eAttribName",
        "eAttribEquals",
        "eAttribValue"
    };

    printf("%12s:%18s:%16s:[%u]%.*s\n", 
        STATUS_STR[aiStatus],
        aiStatus == eInsideTag ? ATTRIBUTE_STR[aiAttribute] : "",
        TOKEN_TYPE_STR[aiType], 
        aiTokenLen, 
        aiTokenLen, 
        aoNextToken.cpcStrBegin
    );
}

const char *McbGetError( McbXMLError aiErrCode )
{
    typedef struct McbErrorList
    {
        McbXMLError ciErrCode;
        const char* cpcError;
    } McbErrorList;

    const char* lpcError = "Unknown";
    static McbErrorList _errorList[] = 
    {
        { eXMLErrorNone,                 "No error"                  },
        { eXMLErrorEmpty,                "No XML data"               },
        { eXMLErrorFirstNotStartTag,  "First token not start tag" },
        { eXMLErrorMissingTagName,     "Missing start tag name"    },
        { eXMLErrorMissingEndTagName, "Missing end tag name"      },
        { eXMLErrorNoMatchingQuote,     "Unmatched quote"           },
        { eXMLErrorUnmatchedEndTag,    "Unmatched end tag"         },
        { eXMLErrorUnexpectedToken,     "Unexpected token found"    },
        { eXMLErrorInvalidTag,           "Invalid tag found"         },
        { eXMLErrorNoElements,           "No elements found"         },
        { eXMLErrorNone,                 NULL                        }
    };

    for(int liIt = 0; _errorList[liIt].cpcError; liIt++)
    {
        if(_errorList[liIt].ciErrCode == aiErrCode)
        {
            lpcError = _errorList[liIt].cpcError;
            break;
        }
    }

    return lpcError; 
}

void McbDeleteText( McbXMLText* apoText )
{
    if(apoText->cpcValue)
    {
        AI_FREE(apoText->cpcValue);
        apoText->cpcValue = NULL;
    }
}

void McbDeleteClear( McbXMLClear* apoClear )
{
    if(apoClear->cpcValue)
    {
        AI_FREE(apoClear->cpcValue);
        apoClear->cpcValue = NULL;
    }
}

void McbDeleteNode(McbXMLNode* apoEntry)
{
    if(apoEntry)
    {
        if(apoEntry->ciType == eNodeEmpty)
        {
            return;
        }

        switch(apoEntry->ciType)
        {
            case eNodeAttribute:
                McbDeleteAttribute(apoEntry->coNode.cpoAttrib);
                break;
                
            case eNodeElement:
                McbDeleteElement(apoEntry->coNode.cpoElement);
                break;
    
            case eNodeText:
                McbDeleteText(apoEntry->coNode.cpoText);
                break;
    
            case eNodeClear:
                McbDeleteClear(apoEntry->coNode.cpoClear);
                break;
    
            default:
                assert(false);
                break;
        }   

        AI_FREE(apoEntry->coNode.cpoAttrib);
        apoEntry->ciType = eNodeEmpty;
    }
}

void McbDeleteElement( McbXMLElement* apoEntry )
{
    for ( int liIt = 0; liIt < apoEntry->ciCount; liIt++ )
    {
        McbDeleteNode(&apoEntry->cpoEntries[liIt]);
    }

    apoEntry->ciMax = 0;
    apoEntry->ciCount = 0;

    AI_FREE(apoEntry->cpoEntries);
    apoEntry->cpoEntries = NULL;
    
    if(apoEntry->cpcName)
    {
        AI_FREE(apoEntry->cpcName);
        apoEntry->cpcName = NULL;
    }
}

void McbAttachNodes( McbXMLNode* apoDst, McbXMLNode* apoSrc, int aiNum )
{
    for ( int liIt = 0; liIt < aiNum; liIt++ )
    {
        apoDst[liIt] = apoSrc[liIt];
        apoSrc[liIt].ciType = eNodeEmpty;
    }
}

void McbAllocNodes( McbXMLElement* apoEntry, int aiGrowBy )
{
    int  liMax;
    McbXMLNode* lpoNewNode;

    apoEntry->ciMax += aiGrowBy;
    liMax = apoEntry->ciMax;

    AI_MALLOC(lpoNewNode, McbXMLNode, sizeof(McbXMLNode) * liMax);
    McbAttachNodes(lpoNewNode, apoEntry->cpoEntries, apoEntry->ciCount);

    if(apoEntry->cpoEntries)
    {
        AI_FREE(apoEntry->cpoEntries);
        apoEntry->cpoEntries = NULL;
    }

    apoEntry->cpoEntries = lpoNewNode;
}

McbClearTag *McbGetClearTags(void)
{
    static McbClearTag _tags[] =
    {
        /* cpcOpen      cpcClose    */
        { "<![CDATA[",  "]]>"       },
        //{ "<PRE>",      "</PRE>"    },
        //{ "<Script>",   "</Script>" },
        { "<!--",       "-->"       },
        { "<!DOCTYPE",  ">"         },
        { "<?",         "?>"        },
        { NULL,         NULL        }
    };

    return  _tags;
}

McbXMLElement* McbAddElement( McbXMLElement* apoEntry, char* apcName, int aiIsDeclaration, int aiGrowBy )
{
    McbXMLNode*    lpoNode;
    McbXMLElement* lpoElement;
    
    if(apoEntry->ciCount == apoEntry->ciMax)
    {
        McbAllocNodes(apoEntry, aiGrowBy);
    }

    lpoNode = &apoEntry->cpoEntries[apoEntry->ciCount];
    lpoNode->ciType = eNodeElement;

    AI_MALLOC( lpoElement, McbXMLElement, sizeof(McbXMLElement) );
    McbInitElement(lpoElement, apoEntry, apcName, aiIsDeclaration);

    lpoNode->coNode.cpoElement = lpoElement;
    apoEntry->ciCount++;

    return lpoElement;
}

McbXMLAttribute* McbAddAttribute( McbXMLElement* apoEntry, char* apcName, char* apcValue, int aiGrowBy)
{
    McbXMLNode*      lpoNode;
    McbXMLAttribute* lpoAttrib;

    if(apoEntry->ciCount == apoEntry->ciMax)
    {
        McbAllocNodes(apoEntry, aiGrowBy);
    }

    lpoNode = &apoEntry->cpoEntries[apoEntry->ciCount];
    lpoNode->ciType = eNodeAttribute;

    AI_MALLOC( lpoAttrib, McbXMLAttribute, sizeof(McbXMLAttribute) );
    lpoAttrib->cpcValue = apcValue;
    lpoAttrib->cpcName = apcName;

    lpoNode->coNode.cpoAttrib = lpoAttrib;
    apoEntry->ciCount++;

    return lpoAttrib;
}

McbXMLText* McbAddText( McbXMLElement* apoEntry, char* apcValue, int aiGrowBy )
{
    McbXMLNode      *lpoNode;
    McbXMLText      *lpoText;

    if(apoEntry->ciCount == apoEntry->ciMax)
    {
        McbAllocNodes(apoEntry, aiGrowBy);
    }

    lpoNode = &apoEntry->cpoEntries[apoEntry->ciCount];
    lpoNode->ciType = eNodeText;

    AI_MALLOC( lpoText, McbXMLText, sizeof(McbXMLText) );
    lpoText->cpcValue = apcValue;

    lpoNode->coNode.cpoText = lpoText;
    apoEntry->ciCount++;

    return lpoText;
}

McbXMLClear* McbAddClear( McbXMLElement* apoEntry, char* apcValue, McbClearTag* apoClear, int aiGrowBy )
{
    McbXMLNode      *lpoNode;
    McbXMLClear     *lpoNewClear;

    if(apoEntry->ciCount == apoEntry->ciMax)
    {
        McbAllocNodes(apoEntry, aiGrowBy);
    }

    lpoNode = &apoEntry->cpoEntries[apoEntry->ciCount];
    lpoNode->ciType = eNodeClear;

    AI_MALLOC(lpoNewClear, McbXMLClear, sizeof(McbXMLClear));
    lpoNewClear->cpcCloseTag = const_cast<char*>(apoClear->cpcClose);
    lpoNewClear->cpcOpenTag = const_cast<char*>(apoClear->cpcOpen);
    lpoNewClear->cpcValue = apcValue;

    lpoNode->coNode.cpoClear = lpoNewClear;
    apoEntry->ciCount++;

    return lpoNewClear;
}


McbXMLNode* McbEnumNodes( McbXMLElement* apoEntry, int* apiIndex )
{   
    McbXMLNode      *lpoResult = NULL;

    if(*apiIndex < apoEntry->ciCount)
    {
        lpoResult = &apoEntry->cpoEntries[*apiIndex];
        (*apiIndex)++;
    }

    return lpoResult;
}

McbXMLElement* McbEnumElements( McbXMLElement* apoEntry, int* apiIndex )
{   
    int      liIndex;
    McbXMLElement* lpoResult = NULL;

    liIndex = *apiIndex;
    for(; liIndex < apoEntry->ciCount && !lpoResult; liIndex++)
    {
        if(apoEntry->cpoEntries[liIndex].ciType == eNodeElement)
        {
            lpoResult = apoEntry->cpoEntries[liIndex].coNode.cpoElement;
        }
    }

    *apiIndex = liIndex;
    
    return lpoResult;
}

McbXMLAttribute* McbEnumAttributes( McbXMLElement* apoEntry, int* apiIndex )
{   
    int liIndex;
    McbXMLAttribute* lpoResult = NULL;

    liIndex = *apiIndex;
    for(; liIndex < apoEntry->ciCount && !lpoResult; liIndex++)
    {
        if(apoEntry->cpoEntries[liIndex].ciType == eNodeAttribute)
        {
            lpoResult = apoEntry->cpoEntries[liIndex].coNode.cpoAttrib;
        }
    }

    *apiIndex = liIndex;
    
    return  (lpoResult);
}

void McbFindEndOfText( char const* apcToken, int* apiText )
{
    char        liChar;
    int  liTextLength;

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
    int  liLength = 0;
    char*       lpcTemp = NULL;
    const char* lpcXML = &apoXML->cpcXML[apoXML->ciIndex];

    lpcTemp = const_cast<char*>(strstr(lpcXML, apoClear->cpcClose));

    if(lpcTemp)
    {
        liLength = lpcTemp - lpcXML;

        apoXML->ciIndex += liLength;
        apoXML->ciIndex += strlen(apoClear->cpcClose);
        
        lpcTemp = McbStrdup(lpcXML, liLength);
        McbAddClear(apoElement, lpcTemp, apoClear, MCB_GROWBY);

        return 1;
    }

    apoXML->ciErrCode = eXMLErrorUnmatchedEndTag;
    
    return 0;
}

int McbParseXMLElement( McbXML* apoXML, McbXMLElement* apoElement )
{
    int  liTempLength;
    int  liTokenLength;
    int  liDeclaration;
    char*       lpcDupString;
    char const* lpcTemp = NULL;
    char const* lpcText = NULL;
    char const* lpcToken = NULL;

    McbStatus      liStatus;
    McbNextToken    loToken;
    McbTokenType   liTokenType = eTokenNone;
    McbXMLElement*  lpoNewElement;
    McbAttrib   liAttrib = eAttribName;

    if(apoXML->ciFirst)
    {
        apoXML->ciFirst = 0;
        liStatus = eOutsideTag;
    }
    else 
    {
        liStatus = eInsideTag;
    }

//#define McbGetNextToken(x, l, t, s) McbGetNextToken_(x, l, t, s); PrintParsingStatus(loToken, *l, *t, s, liAttrib)
    while(true)
    {
        loToken = McbGetNextToken(apoXML, &liTokenLength, &liTokenType, liStatus);

        if(liTokenType == eTokenError)
        {
            //daizh 2010-3-11
            //case: <a>1<a><b>2</b>
            if (apoElement->cpcName != NULL)
            {
                apoXML->ciErrCode = eXMLErrorUnmatchedEndTag;
            }

            return 0;
        }


        if(liStatus == eOutsideTag)
        {
            switch(liTokenType)
            {
                case eTokenText:
                case eTokenQuotedText:
                case eTokenEquals:
                    if(lpcText == NULL)
                    {
                        lpcText = loToken.cpcStrBegin;
                    }                   
                    break;              
    
                case eTokenTagStart:
                case eTokenDeclaration:
                    liDeclaration = (liTokenType == eTokenDeclaration);
                    if(lpcText)
                    {
                        liTempLength = loToken.cpcStrBegin - lpcText;
                        //McbFindEndOfText(lpcText, &liTempLength);
        
                        lpcDupString = McbStrdup(lpcText, liTempLength);
                        McbAddText(apoElement, lpcDupString, MCB_GROWBY);
                        lpcText = NULL;
                    }
    
                    loToken = McbGetNextToken(apoXML, &liTokenLength, &liTokenType, liStatus);
                    if(liTokenType != eTokenText)
                    {
                        apoXML->ciErrCode = eXMLErrorMissingTagName;
                        return 0;
                    }
    
                    if(apoElement->cpcName && strcmp(apoElement->cpcName, loToken.cpcStrBegin) == 0)
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
                                if(apoElement->cpcName == NULL)
                                {
                                    apoXML->ciErrCode = eXMLErrorUnmatchedEndTag;
                                    return 0;
                                }
        
                                if(strncmp(apoXML->cpcEndTag, apoElement->cpcName, \
                                        strlen(apoElement->cpcName)) == 0)
                                {                                    
                                    apoXML->ciEndTagLen = 0;
                                }
        
                                return 1;
                            }
                            
                            if(apoXML->ciNewElementLen)
                            {
                                if(strncmp(apoXML->cpcNewElement, apoElement->cpcName, 
                                        strlen(apoElement->cpcName)) == 0)
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
    
                case eTokenTagEnd:
                    if(lpcText)
                    {
                        liTempLength = loToken.cpcStrBegin - lpcText;
                        //McbFindEndOfText(lpcText, &liTempLength);
        
                        lpcDupString = McbStrdup(lpcText, liTempLength);
                        McbAddText(apoElement, lpcDupString, MCB_GROWBY);
                        lpcText = NULL;
                    }
                            
                    loToken = McbGetNextToken(apoXML, &liTempLength, &liTokenType, liStatus);                 
                    if(liTokenType != eTokenText)
                    {
                        apoXML->ciErrCode = eXMLErrorMissingEndTagName;
                        return 0;
                    }
        
                    lpcTemp = loToken.cpcStrBegin;
                    liTokenType = eTokenTagEnd; // support > in text
                    loToken = McbGetNextToken(apoXML, &liTokenLength, &liTokenType, liStatus);
        
                    if(liTokenType != eTokenCloseTag)
                    {
                        apoXML->ciErrCode = eXMLErrorMissingEndTagName;
                        return 0;
                    }
        
                    //added by max at 2006.03.31
                    //for the case: </abc><aa>123</aa>
                    if(apoElement->cpcName == NULL)
                    {
                        break;
                    }
                            
                    if(strncmp(lpcTemp, apoElement->cpcName, strlen(apoElement->cpcName)))
                    {
                        apoXML->cpcEndTag = lpcTemp;
                        apoXML->ciEndTagLen = liTempLength;
                        
                        //daizh 2010-3-11
                        //case <x><a>1<a><b>2</b></x>
                        apoXML->ciErrCode = eXMLErrorMissingEndTagName;
                        return 0;
                    }
        
                    return 1;
    
                case eTokenClear:
                    if(lpcText)
                    {
                        liTempLength = loToken.cpcStrBegin - lpcText;
                        //McbFindEndOfText(lpcText, &liTempLength);

                        lpcDupString = McbStrdup(lpcText, liTempLength);
                        McbAddText(apoElement, lpcDupString, MCB_GROWBY);
                        lpcText = NULL;
                    }
    
                    if(McbParseClearTag(apoXML, apoElement, loToken.cpoClear) == 0)
                    {
                        return 0;
                    }
                    break;
    
                case eTokenCloseTag:        /* '>'   */
                case eTokenShortHandClose:  /* '/>'  */
                    apoXML->ciErrCode = eXMLErrorUnexpectedToken;
                    return 0;
            
                case eTokenError:
                default:
                    break;
            }
        }
        else if(liStatus == eInsideTag)
        {
            switch(liAttrib)
            {
                case eAttribName:
                    switch(liTokenType)
                    {
                        case eTokenText:
                            lpcTemp = loToken.cpcStrBegin;
                            liTempLength = liTokenLength;
                            liAttrib = eAttribEquals;
                            break;
            
                        case eTokenCloseTag:
                            liStatus = eOutsideTag;
                            break;
            
                        case eTokenShortHandClose:
                            return 1;
            
                        case eTokenQuotedText:  /* '"SomeText"' */
                        case eTokenTagStart:    /* '<'          */
                        case eTokenTagEnd:      /* '</'         */
                        case eTokenEquals:      /* '='          */
                        case eTokenDeclaration: /* '<?'         */
                        case eTokenClear:
                            apoXML->ciErrCode = eXMLErrorUnexpectedToken;
                            return 0;
                                    
                        case eTokenError:
                        default:
                            break;
                    }
                    break;
        
                case eAttribEquals:
                    switch(liTokenType)
                    {
                        case eTokenText:
                            lpcDupString = McbStrdup(lpcTemp, liTempLength);
                            McbAddAttribute(apoElement, lpcDupString, NULL, MCB_GROWBY);
            
                            lpcTemp = loToken.cpcStrBegin;
                            liTempLength = liTokenLength;
                            break;
            
                        case eTokenShortHandClose:
                        case eTokenCloseTag:
                            if(apoElement->ciIsDeclaration && (lpcTemp[liTempLength-1]) == '?')
                            {
                                liTempLength--;
                            }
            
                            if(liTempLength)
                            {
                                lpcDupString = McbStrdup(lpcTemp, liTempLength);
                                McbAddAttribute(apoElement, lpcDupString, NULL, MCB_GROWBY);
                            }
            
                            if(liTokenType == eTokenShortHandClose)
                            {
                                return 1;
                            }
                            
                            liStatus = eOutsideTag;
                            break;
            
                        case eTokenEquals:
                            liAttrib = eAttribValue;
                            break;
            
                        case eTokenQuotedText:  /* 'Attribute "InvalidAttr"'*/
                        case eTokenTagStart:    /* 'Attribute <'            */
                        case eTokenTagEnd:      /* 'Attribute </'           */
                        case eTokenDeclaration: /* 'Attribute <?'           */
                        case eTokenClear:
                            apoXML->ciErrCode = eXMLErrorUnexpectedToken;
                            return 0;
        
                        case eTokenError:
                        default:
                            break;  
                    }
                    break;
        
                case eAttribValue:
                    switch(liTokenType)
                    {
                        case eTokenText:
                            if(apoElement->ciIsDeclaration && 
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
            
                            liAttrib = eAttribName;
                            break;
                        ///FIXED by daizh
                        case eTokenQuotedText:
                            if(apoElement->ciIsDeclaration && 
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
            
                            liAttrib = eAttribName;
                            break;
            
                        case eTokenTagStart:        /* 'Attr = <'   */
                        case eTokenTagEnd:          /* 'Attr = </'  */
                        case eTokenCloseTag:        /* 'Attr = >'   */
                        case eTokenShortHandClose:  /* "Attr = />"  */
                        case eTokenEquals:          /* 'Attr = ='   */
                        case eTokenDeclaration:     /* 'Attr = <?'  */
                        case eTokenClear:
                            apoXML->ciErrCode = eXMLErrorUnexpectedToken;
                            return 0;
                            break;
                                
                        case eTokenError:
                        default:
                            break;
                }
            }
        }
    }
}

void McbCountLinesAndColucos( char const* apcXML, int liUpto, McbXMLResults* apoResults )
{
    char liChar;

    apoResults->ciLine = 1;
    apoResults->ciColumn = 1;

    for(int liIt = 0; liIt < liUpto; liIt++)
    {
        liChar = apcXML[liIt];
        assert(liChar);

        if(liChar == '\n')
        {
            apoResults->ciLine++;
            apoResults->ciColumn = 1;
        }
        else
        {
            apoResults->ciColumn++;
        }
    }
}

McbXMLElement* McbParseXML( char const* apcXML, McbXMLResults* apoResults )
{
    McbXMLError      liErrCode;
    McbXMLElement* lpoRootElement = NULL;
    McbXML loInitXML = { NULL, 0, eXMLErrorNone, NULL, 0, NULL, 0, 1, NULL };

    loInitXML.cpcXML = apcXML;
    loInitXML.cpoClearTags = McbGetClearTags();

    lpoRootElement = McbCreateRoot();
    McbParseXMLElement(&loInitXML, lpoRootElement);
    liErrCode = static_cast<McbXMLError>(loInitXML.ciErrCode);

    if(liErrCode != eXMLErrorNone)
    {
        McbDeleteRoot(lpoRootElement);
        lpoRootElement = NULL;
    }

    if(apoResults)
    {
        apoResults->ciErrCode = liErrCode;

        if(liErrCode != eXMLErrorNone)
        {
            McbCountLinesAndColucos(loInitXML.cpcXML, loInitXML.ciIndex, apoResults);
        }
    }

    return lpoRootElement;
}

McbXMLElement* McbFindElement( McbXMLElement* apoEntry, char const* apcPath )
{
    int     liIndex;
    int     liNameLength;
    int     liComResult = 0;
    char const*    lpcName = NULL;
    char const*    lpcNext = NULL;
    McbXMLElement* lpoChild = NULL;

    lpcNext = strchr(apcPath, '/');

    if(lpcNext == NULL)
    {
        liNameLength = strlen(apcPath);
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
            lpcName = lpoChild->cpcName;

            if(lpcName)
            {
                if(strchr(apcPath, '/') == NULL)
                {
                    liComResult = strcmp(apcPath, lpcName);
                }
                else
                {
                    liComResult = strncmp(apcPath, lpcName, liNameLength);
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

size_t McbFindElements( McbXMLElement* apoHead, char const* apcName, McbXMLElement** apoElemArray, size_t aiCount )
{
    int             liIndex      = 0;
    int             liNameLen    = strlen(apcName) + 1;
    size_t          liElemCount  = 0;
    char*           lpcTmpName   = NULL;
    char*           lpcLeafName  = NULL;
    McbXMLElement*  lpoElemEntry = apoHead;
    McbXMLElement*  lpoChild     = NULL;
    
    AI_MALLOC_ASSERT( lpcTmpName, char, liNameLen );
        
    strncpy( lpcTmpName, apcName, liNameLen );
	memset( apoElemArray, 0, sizeof(McbXMLElement *) * aiCount );
    
    do
    {
        // Skip redundant '/' last, e.g "root_name/next_name//" 
        while( (lpcLeafName = strrchr( lpcTmpName, '/' )) != NULL )
        {
            if ( strlen(lpcLeafName) > 1 ) break;
            
            *lpcLeafName = '\0';
        }
        
        if ( lpcLeafName != NULL )
        {
            // Yes, it's cilti-directory path, e.g "root_name/next_name/last_name" 
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

            if( strcmp(lpcLeafName, lpoChild->cpcName) == 0 )
            {
                apoElemArray[liElemCount++] = lpoChild;
            }
        }
        
    }while(false);
    
    AI_FREE( lpcTmpName );
    
    return liElemCount;
}

McbXMLAttribute* McbFindAttribute( McbXMLElement* apoEntry, char const* apcAttribute )
{
    int       liIndex;
    int       liAttribLength;
    McbXMLAttribute* lpoAttribute = NULL;

    liIndex = 0;
    liAttribLength = strlen(apcAttribute);

    while( ( lpoAttribute = McbEnumAttributes(apoEntry, &liIndex) ) != NULL)
    {
        if(strncmp(lpoAttribute->cpcName, apcAttribute, liAttribLength) == 0)
        {
            return lpoAttribute;
        }
    }

    return NULL;
}

McbXMLElement* McbCreateElements( McbXMLElement* apoEntry, char const* apcPath)
{
    int     liIndex;
    int     liNameLength;
    int     liComResult = 0;
    char*          lpcTemp = NULL;
    char const*    lpcName = NULL;
    char const*    lpcNext = NULL;
    McbXMLElement* lpoChild = NULL;
    McbXMLElement* lpoNewElement = NULL;
    
    if(lpcNext = strchr(apcPath, '/'), lpcNext == NULL)
    {
        liNameLength = strlen(apcPath);     
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
            lpcName = lpoChild->cpcName;

            if(lpcName)
            {
                if(strchr(apcPath, '/') == NULL)
                {
                    liComResult = strcmp(apcPath, lpcName);
                }
                else
                {
                    liComResult = strncmp(apcPath, lpcName, liNameLength);
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

int McbCreateXMLStringR( McbXMLElement* apoEntry, char* apcMarker, int aiFormat)
{
    int       liIndex;
    int       liLength;
    int       liResult = 0;
    int        liChildFormat;
    int       liElementLength;
    int       liHasChildren = 0;
    int       liHasTextChildren = 0;
    McbXMLNode*      lpoChildNode = NULL;
    McbXMLAttribute* lpoAttrib = NULL;

    liElementLength = MCB_LENSTR(apoEntry->cpcName);

    if(liElementLength)
    {
        liLength = (aiFormat == -1 ? 0 : aiFormat);

        if(apcMarker)
        {
            if(liLength)
            {
                memset(apcMarker, MCB_INDENTCHAR, liLength);
            }

            liResult = liLength;
            apcMarker[liResult++] = '<';        
            McbStrCpy(&apcMarker[liResult], apoEntry->cpcName);
            
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
            switch(lpoChildNode->ciType)
            {
                case eNodeAttribute:
                {
                    lpoAttrib = lpoChildNode->coNode.cpoAttrib;
                    liLength = MCB_LENSTR(lpoAttrib->cpcName);
    
                    if(liLength)
                    {
                        if(apcMarker)
                        {
                            McbStrCpy(&apcMarker[liResult], lpoAttrib->cpcName);             
                        }
    
                        liResult += liLength;
                        liLength = MCB_LENSTR(lpoAttrib->cpcValue);
    
                        if(liLength)
                        {
                            if (lpoAttrib->cpcValue[0] == '\"'
                                || lpoAttrib->cpcValue[0] == '\'')
                            {
                                if(apcMarker)
                                {
                                    apcMarker[liResult] = (char)'=';
                                    McbStrCpy(&apcMarker[liResult + 1], lpoAttrib->cpcValue);
                                }
                                liResult += (liLength + 1);
                            }
                            else
                            {
                                if(apcMarker)
                                {
                                    apcMarker[liResult] = (char)'=';
                                    apcMarker[liResult + 1] = (char)'\"';
                                    McbStrCpy(&apcMarker[liResult + 2], lpoAttrib->cpcValue);
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
    
                case eNodeText:
                    liHasChildren = 1;
                    liHasTextChildren = 1;
                    break;

                case eNodeEmpty:
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
        switch(lpoChildNode->ciType)
        {
            case eNodeText:
            {
                liLength = MCB_LENSTR(lpoChildNode->coNode.cpoText->cpcValue);
    
                if(liLength)
                {
                    if(false/*aiFormat != -1*/)//text node needn't format
                    {
                        if(apcMarker)
                        {
                            memset(&apcMarker[liResult], MCB_INDENTCHAR, aiFormat + 1);
                            McbStrCpy(&apcMarker[liResult + aiFormat + 1], lpoChildNode->coNode.cpoText->cpcValue);
                            apcMarker[ liResult + aiFormat + 1 + liLength] = (char)'\n';
                        }
    
                        liResult += (liLength + aiFormat + 2);
                    }
                    else
                    {
                        if(apcMarker)
                        {
                            McbStrCpy(&apcMarker[liResult], lpoChildNode->coNode.cpoText->cpcValue);
                        }
    
                        liResult += liLength;
                    }               
                }
            }
            break;
    
            case eNodeClear:
            {
                liLength = MCB_LENSTR(lpoChildNode->coNode.cpoClear->cpcOpenTag);
            
                if(liLength)
                {
                    if(aiFormat != -1)
                    {
                        if(apcMarker)
                        {
                            memset(&apcMarker[liResult], MCB_INDENTCHAR, aiFormat + 1);
                            McbStrCpy(&apcMarker[liResult + aiFormat + 1], lpoChildNode->coNode.cpoClear->cpcOpenTag);
            
                            /* apcMarker[liResult + aiFormat + 1 + liLength] = (char)'\n'; */
                        }
            
                        /* liResult += (liLength + aiFormat + 2); */
                        liResult += (liLength + aiFormat + 1);
                    }
                    else
                    {
                        if(apcMarker)
                        {
                            McbStrCpy(&apcMarker[liResult], lpoChildNode->coNode.cpoClear->cpcOpenTag);
                        }
            
                        liResult += liLength;
                    }               
                }
            
                liLength = MCB_LENSTR(lpoChildNode->coNode.cpoClear->cpcValue);
            
                if(liLength)
                {
                    if(apcMarker)
                    {
                        McbStrCpy(&apcMarker[liResult], lpoChildNode->coNode.cpoClear->cpcValue);
                    }
            
                    liResult += liLength;
                }
            
                liLength = MCB_LENSTR(lpoChildNode->coNode.cpoClear->cpcCloseTag);
            
                if(liLength)
                {
                    if(apcMarker)
                    {
                        McbStrCpy(&apcMarker[liResult], lpoChildNode->coNode.cpoClear->cpcCloseTag);
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
                
            case eNodeElement:
            {
                liResult += McbCreateXMLStringR(lpoChildNode->coNode.cpoElement, 
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
                        memset(&apcMarker[liResult], MCB_INDENTCHAR, aiFormat);
                        liResult += aiFormat;
                    }
                }       

                McbStrCpy(&apcMarker[liResult], "</");        
                liResult += 2;
                
                McbStrCpy(&apcMarker[liResult], apoEntry->cpcName);
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

char *McbCreateXMLString( McbXMLElement* apoHead, int aiFormat, int* apiSize)
{
    int  liFormat;
    int liStrLength;
    char*      lpcResult = NULL;

    if(apoHead)
    {
        liFormat = (aiFormat ? 0 : -1);
        
        if( ( liStrLength = McbCreateXMLStringR(apoHead, NULL, liFormat) ) <= 0 )
        {
            return NULL;
        }

        AI_MALLOC(lpcResult, char, (liStrLength+1));

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


AIBC_NAMESPACE_END
