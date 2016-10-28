#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include "AIMcbXML.h"

/////////////////////////////////////////////////////////////////////////
//
#define Mcb_GROWBY            (5)
#define Mcb_INDENTCHAR	      ('\t')
#define Mcb_LENSTR( str )     (str ? strlen(str) : 0)

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
	char				*cpcOpen;
	char				*cpcClose;
} McbClearTag;

typedef struct McbNextToken
{
	McbClearTag			*cpoClear;
	const char			*cpcStrBegin;
} McbNextToken;

typedef struct McbXML
{
	const char			*cpcXML;
	int					ciIndex;
	McbXMLError			ciErrCode;
	const char			*cpcEndTag;
	int					ciEndTagLen;
	const char			*cpcNewElement;
	int					ciNewElementLen;
	int					ciFirst;
	McbClearTag			*cpoClearTags;
} McbXML;

/////////////////////////////////////////////////////////////////////////
//
void McbInitElement( McbXMLElement *apoEntry, McbXMLElement *apoParent, char *apcName, int aiIsDeclaration )
{
	assert(apoEntry);

	apoEntry->ciMax = 0;
	apoEntry->ciCount = 0;
	apoEntry->cpoEntries = NULL;
	apoEntry->cpoParent = apoParent;
	apoEntry->ciIsDeclaration = aiIsDeclaration;
	apoEntry->cpcName = apcName;
}

McbXMLElement *McbCreateRoot( void )
{
	McbXMLElement	*lpoElement;

	AI_MALLOC_ASSERT( lpoElement, McbXMLElement, sizeof(McbXMLElement) );
	McbInitElement( lpoElement, NULL, 0, 0 );

	return  (lpoElement);
}

void McbDeleteRoot( McbXMLElement *apoElement )
{
    McbDeleteElement( apoElement );
	AI_FREE( apoElement );
}

void McbDeleteAttribute( McbXMLAttribute *apoEntry )
{
	assert(apoEntry);

	if( apoEntry->cpcName ) 
	{
		AI_FREE( apoEntry->cpcName );
		apoEntry->cpcName = NULL;
	}

	if( apoEntry->cpcValue ) 
	{
		AI_FREE( apoEntry->cpcValue );
		apoEntry->cpcValue=  NULL;
	}
}

void McbAttributeAttach( McbXMLAttribute *apoDst, McbXMLAttribute *apoSrc, int aiNum )
{
	for( int liIt = 0; liIt < aiNum; liIt++ )
	{
		apoDst[liIt].cpcName = apoSrc[liIt].cpcName;
		apoDst[liIt].cpcValue = apoSrc[liIt].cpcValue;

		apoSrc[liIt].cpcName = NULL;
		apoSrc[liIt].cpcValue = NULL;
	}
}

char McbGetNextChar( McbXML *apoXML )
{
	char		liChar;

	assert( apoXML );

	liChar = apoXML->cpcXML[ apoXML->ciIndex ];
	if( liChar ) apoXML->ciIndex++;

	return  (liChar);
}

char McbFindNonWhiteSpace( McbXML *apoXML )
{
	char		liChar = 0;
	int			liExitFlag = 0;

	assert( apoXML );

	while( (liExitFlag == 0) && (liChar = McbGetNextChar(apoXML)) )
	{
		switch( liChar )
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

	return  (liChar);
}

char *McbStrdup( const char *apcData, int aiSize )
{
	int			liSize;
	char		*lpcTemp;

	assert(apcData);
	
	liSize = (aiSize<=0?(int)strlen(apcData):aiSize);
	AI_MALLOC_ASSERT( lpcTemp, char, liSize + 1 );

	memcpy( lpcTemp, apcData, liSize );
	lpcTemp[ liSize ] = (char)0;

	return  (lpcTemp);
}

McbNextToken McbGetNextToken( McbXML *apoXML, int *apiTokenLen, McbTokenType *apiType )
{
	int				liSize;
	int				liIndex;
	int				liExitFlag;
	int				liFoundMatch;
	int				liOpenLength;
	int				liIsText = 0;
	char			liChar = 0;
	char			liTempChar;
	const char		*lpcOpen;
	const char		*lpcXML;
	McbNextToken	loResult;

	liChar = McbFindNonWhiteSpace( apoXML );
	memset( &loResult, 0, sizeof(McbNextToken) );

	if( liChar == 0 )
	{
   		*apiTokenLen = 0;
		*apiType = eTokenError;
		return  (loResult);
	}
	
	lpcXML = apoXML->cpcXML;
	loResult.cpcStrBegin = &lpcXML[apoXML->ciIndex-1];

	liIndex = 0;
	while( 1 )
	{
		lpcOpen = apoXML->cpoClearTags[liIndex].cpcOpen;
		if( lpcOpen == NULL )  break;

		liOpenLength = strlen(lpcOpen);
		if( strncmp(lpcOpen, loResult.cpcStrBegin, liOpenLength) == 0 )
		{
			loResult.cpoClear = &apoXML->cpoClearTags[liIndex];
			apoXML->ciIndex += (liOpenLength-1);

			*apiType = eTokenClear;
			return  (loResult);
		}

		liIndex++;
	}

	liTempChar = 0;
	lpcXML = apoXML->cpcXML;
		
	switch( liChar )
	{
		case '\'':
		case '\"':
			*apiType = eTokenQuotedText;
			liTempChar = liChar;
	
			liSize = 1;
			liFoundMatch = 0;
	
			while( (liChar = McbGetNextChar(apoXML)) != 0 )
			{
				liSize++;
	
				if( liChar == liTempChar )
				{
					liFoundMatch = 1;
					break;
				}
			}
	
			if( liFoundMatch == 0 )
			{
				apoXML->ciErrCode = eXMLErrorNoMatchingQuote;				
				*apiType = eTokenError;
			}

			if( McbFindNonWhiteSpace(apoXML) )
			{
				apoXML->ciIndex--;
			}
	
			break;
	
		case '=':
			liSize = 1;
			*apiType = eTokenEquals;
			break;
	
		case '>':
			liSize = 1;
			*apiType = eTokenCloseTag;
			break;
	
		case '<':
			liTempChar = apoXML->cpcXML[apoXML->ciIndex];
	
			if( liTempChar == '/' )
			{
				McbGetNextChar( apoXML );
				*apiType = eTokenTagEnd;
				liSize = 2;
			}
			else if( liTempChar == '?' )
			{
				McbGetNextChar( apoXML );
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
	
			if( liTempChar == '>' )
			{
				McbGetNextChar( apoXML );
				*apiType = eTokenShortHandClose;
				liSize = 2;
				break;
			}

		default:
			liIsText = 1;
			break;
	}

	if( liIsText )
	{
		liSize = 1;
		liExitFlag = 0;
		*apiType = eTokenText;

		while( (liExitFlag == 0) && (liChar = McbGetNextChar(apoXML)) )
		{
			switch( liChar )
            {
	            case '\n':
	            case ' ':
	            case '\t':
	            case '\r':
					liExitFlag = 1;
	                break;
	
				case '/':
					liTempChar = apoXML->cpcXML[apoXML->ciIndex];
	
					if( liTempChar == '>' )
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
	return  (loResult);
}

const char *McbGetError( McbXMLError aiErrCode )
{
	typedef struct McbErrorList
	{
		McbXMLError			ciErrCode;
		const char			*cpcError;
	} McbErrorList;

	const char			*lpcError = "Unknown";
	static McbErrorList _errorList[ ] = 
	{
		{ eXMLErrorNone,              "No error"                  },
		{ eXMLErrorEmpty,             "No XML data"               },
		{ eXMLErrorFirstNotStartTag,  "First token not start tag" },
		{ eXMLErrorMissingTagName,    "Missing start tag name"    },
		{ eXMLErrorMissingEndTagName, "Missing end tag name"      },
		{ eXMLErrorNoMatchingQuote,   "Unmatched quote"           },
		{ eXMLErrorUnmatchedEndTag,   "Unmatched end tag"         },
		{ eXMLErrorUnexpectedToken,   "Unexpected token found"    },
		{ eXMLErrorInvalidTag,        "Invalid tag found"         },
		{ eXMLErrorNoElements,        "No elements found"         },
		{ eXMLErrorNone,              NULL                        }
	};

	for( int liIt = 0; _errorList[liIt].cpcError; liIt++ )
	{
		if( _errorList[liIt].ciErrCode == aiErrCode )
		{
			lpcError = _errorList[liIt].cpcError;
			break;
		}
	}

	return  (lpcError);	
}

void McbDeleteText( McbXMLText *apoText )
{
	assert(apoText);

	if( apoText->cpcValue )
	{
		AI_FREE( apoText->cpcValue );
		apoText->cpcValue = NULL;
	}
}

void McbDeleteClear( McbXMLClear *apoClear )
{
	assert(apoClear);

	if( apoClear->cpcValue )
	{
		AI_FREE( apoClear->cpcValue );
		apoClear->cpcValue = NULL;
	}
}

void McbDeleteNode( McbXMLNode *apoEntry )
{
	if( apoEntry )
	{
		if( apoEntry->ciType == eNodeEmpty )
			return;

		switch( apoEntry->ciType )
		{
			case eNodeAttribute:
				McbDeleteAttribute( apoEntry->coNode.cpoAttrib );
				break;
				
			case eNodeElement:
				McbDeleteElement( apoEntry->coNode.cpoElement );
				break;
	
			case eNodeText:
				McbDeleteText( apoEntry->coNode.cpoText );
				break;
	
			case eNodeClear:
				McbDeleteClear( apoEntry->coNode.cpoClear );
				break;
	
			default:
				assert( 1 );
				break;
		}	

		AI_FREE( apoEntry->coNode.cpoAttrib );
		apoEntry->ciType = eNodeEmpty;
	}
}

void McbDeleteElement( McbXMLElement *apoEntry )
{
	assert(apoEntry);

	for( int liIt = 0; liIt < apoEntry->ciCount; liIt++ )
	{
		McbDeleteNode( &apoEntry->cpoEntries[liIt] );
	}

	apoEntry->ciMax = 0;
	apoEntry->ciCount = 0;

	AI_FREE( apoEntry->cpoEntries );
	apoEntry->cpoEntries = NULL;
	
	if( apoEntry->cpcName )
	{
		AI_FREE( apoEntry->cpcName );
		apoEntry->cpcName = NULL;
	}
}

void McbAttachNodes( McbXMLNode *apoDst, McbXMLNode *apoSrc, int aiNum )
{
	for( int liIt = 0; liIt < aiNum; liIt++ )
	{
		apoDst[liIt] = apoSrc[liIt];
		apoSrc[liIt].ciType = eNodeEmpty;
	}
}

void McbAllocNodes( McbXMLElement *apoEntry, int aiGrowBy )
{
	int				liMax;
	McbXMLNode		*lpoNewNode;

	assert(apoEntry);
	assert(aiGrowBy > 0);

    apoEntry->ciMax += aiGrowBy;
    liMax = apoEntry->ciMax;

    AI_MALLOC_ASSERT( lpoNewNode, McbXMLNode, sizeof(McbXMLNode) * liMax );
    McbAttachNodes( lpoNewNode, apoEntry->cpoEntries, apoEntry->ciCount );

	if( apoEntry->cpoEntries )
	{
		AI_FREE( apoEntry->cpoEntries );
		apoEntry->cpoEntries = NULL;
	}

    apoEntry->cpoEntries = lpoNewNode;
}

McbXMLElement *McbAddElement( McbXMLElement *apoEntry, char *apcName, int aiIsDeclaration, int aiGrowBy )
{
	McbXMLNode		*lpoNode;
	McbXMLElement	*lpoElement;

	assert(apoEntry);
	assert(aiGrowBy > 0);

	if( apoEntry->ciCount == apoEntry->ciMax )
	{
		McbAllocNodes( apoEntry, aiGrowBy );
	}

	lpoNode = &apoEntry->cpoEntries[apoEntry->ciCount];
	lpoNode->ciType = eNodeElement;

	AI_MALLOC_ASSERT( lpoElement, McbXMLElement, sizeof(McbXMLElement) );
	McbInitElement( lpoElement, apoEntry, apcName, aiIsDeclaration );

	lpoNode->coNode.cpoElement = lpoElement;
	apoEntry->ciCount++;

	return  (lpoElement);
}

McbXMLAttribute *McbAddAttribute( McbXMLElement *apoEntry, char *apcName, char *apcValue, int aiGrowBy )
{
	McbXMLNode			*lpoNode;
	McbXMLAttribute		*lpoAttrib;

	assert(apoEntry);
	assert(aiGrowBy > 0);

	if( apoEntry->ciCount == apoEntry->ciMax )
	{
		McbAllocNodes( apoEntry, aiGrowBy );
	}

	lpoNode = &apoEntry->cpoEntries[apoEntry->ciCount];
	lpoNode->ciType = eNodeAttribute;

	AI_MALLOC_ASSERT( lpoAttrib, McbXMLAttribute, sizeof(McbXMLAttribute) );
	lpoAttrib->cpcValue = apcValue;
	lpoAttrib->cpcName = apcName;

	lpoNode->coNode.cpoAttrib = lpoAttrib;
	apoEntry->ciCount++;

	return  (lpoAttrib);
}

McbXMLText *McbAddText( McbXMLElement *apoEntry, char *apcValue, int aiGrowBy )
{
	McbXMLNode		*lpoNode;
	McbXMLText		*lpoText;

	assert(apoEntry);
	assert(aiGrowBy > 0);

	if( apoEntry->ciCount == apoEntry->ciMax )
	{
		McbAllocNodes( apoEntry, aiGrowBy );
	}

	lpoNode = &apoEntry->cpoEntries[apoEntry->ciCount];
	lpoNode->ciType = eNodeText;

	AI_MALLOC_ASSERT( lpoText, McbXMLText, sizeof(McbXMLText) );
	lpoText->cpcValue = apcValue;

	lpoNode->coNode.cpoText = lpoText;
	apoEntry->ciCount++;

	return  (lpoText);
}

McbXMLClear *McbAddClear( McbXMLElement *apoEntry, char *apcValue, McbClearTag *apoClear, int aiGrowBy )
{
	McbXMLNode		*lpoNode;
	McbXMLClear		*lpoNewClear;

	assert(apoEntry);
	assert(aiGrowBy > 0);

	if( apoEntry->ciCount == apoEntry->ciMax )
	{
		McbAllocNodes( apoEntry, aiGrowBy );
	}

	lpoNode = &apoEntry->cpoEntries[apoEntry->ciCount];
	lpoNode->ciType = eNodeClear;

	AI_MALLOC_ASSERT( lpoNewClear, McbXMLClear, sizeof(McbXMLClear) );
	lpoNewClear->cpcCloseTag = apoClear->cpcClose;
	lpoNewClear->cpcOpenTag = apoClear->cpcOpen;
	lpoNewClear->cpcValue = apcValue;

	lpoNode->coNode.cpoClear = lpoNewClear;
	apoEntry->ciCount++;

	return  (lpoNewClear);
}

McbXMLNode *McbEnumNodes( McbXMLElement *apoEntry, int *apiIndex )
{	
	McbXMLNode		*lpoResult = NULL;

	assert(apiIndex);
	assert(apoEntry);

	if( *apiIndex < apoEntry->ciCount )
	{
		lpoResult = &apoEntry->cpoEntries[*apiIndex];
		(*apiIndex)++;
	}

	return  (lpoResult);
}

McbXMLElement *McbEnumElements( McbXMLElement *apoEntry, int *apiIndex )
{	
	int				liIndex;
	McbXMLElement	*lpoResult = NULL;

	assert(apiIndex);
	assert(apoEntry);

	liIndex = *apiIndex;
	for( ; liIndex < apoEntry->ciCount && !lpoResult; liIndex++ )
	{
		if( apoEntry->cpoEntries[liIndex].ciType == eNodeElement )
		{
			lpoResult = apoEntry->cpoEntries[liIndex].coNode.cpoElement;
		}
	}

	*apiIndex = liIndex;
	return  (lpoResult);
}

McbXMLAttribute *McbEnumAttributes( McbXMLElement *apoEntry, int *apiIndex )
{	
	int					liIndex;
	McbXMLAttribute		*lpoResult = NULL;

	assert(apiIndex);
	assert(apoEntry);

	liIndex = *apiIndex;
	for( ; liIndex < apoEntry->ciCount && !lpoResult; liIndex++ )
	{
		if( apoEntry->cpoEntries[liIndex].ciType == eNodeAttribute )
		{
			lpoResult = apoEntry->cpoEntries[liIndex].coNode.cpoAttrib;
		}
	}

	*apiIndex = liIndex;
	return  (lpoResult);
}

void McbFindEndOfText( const char *apcToken, int *apiText )
{
	char		liChar;
	int			liTextLength;

	assert(apiText);
	assert(apcToken);

	liTextLength = (*apiText)-1;	
	while( 1 )
	{
		assert( liTextLength >= 0 );
		liChar = apcToken[liTextLength];

		switch( liChar )
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

int McbParseClearTag( McbXML *apoXML, McbXMLElement *apoElement, McbClearTag *apoClear )
{
	int			liLength = 0;
	char		*lpcTemp = NULL;
	const char	*lpcXML = &apoXML->cpcXML[apoXML->ciIndex];

	lpcTemp = strstr( lpcXML, apoClear->cpcClose );

    if( lpcTemp )
    {
        liLength = lpcTemp - lpcXML;

		apoXML->ciIndex += liLength;
		apoXML->ciIndex += strlen( apoClear->cpcClose );
        
        lpcTemp = McbStrdup( lpcXML, liLength );
        McbAddClear( apoElement, lpcTemp, apoClear, Mcb_GROWBY );

        return  (1);
    }

	apoXML->ciErrCode = eXMLErrorUnmatchedEndTag;
	return  (0);
}

int McbParseXMLElement( McbXML *apoXML, McbXMLElement *apoElement )
{
	int				liTempLength;
	int				liTokenLength;
	int				liDeclaration;
	char			*lpcDupString;
	const char		*lpcTemp = NULL;
	const char		*lpcText = NULL;
	const char		*lpcToken = NULL;

	McbStatus		liStatus;
	McbNextToken	loToken;
	McbTokenType	liTokenType;
	McbXMLElement	*lpoNewElement;
	McbAttrib		liAttrib = eAttribName;

	assert( apoXML );
	assert( apoElement );

	if( apoXML->ciFirst )
	{
		apoXML->ciFirst = 0;
		liStatus = eOutsideTag;
	}
    else 
	{
		liStatus = eInsideTag;
	}

	while( 1 )
	{
		loToken = McbGetNextToken( apoXML, &liTokenLength, &liTokenType );

 		if( liTokenType == eTokenError )
 			return  (0);

		if( liStatus == eOutsideTag )
		{
			switch( liTokenType )
			{
				case eTokenText:
				case eTokenQuotedText:
				case eTokenEquals:
					if( lpcText == NULL )
					{
						lpcText = loToken.cpcStrBegin;
					}					
					break;				
	
				case eTokenTagStart:
				case eTokenDeclaration:
					liDeclaration = (liTokenType == eTokenDeclaration);
					if( lpcText )
					{
						liTempLength = loToken.cpcStrBegin - lpcText;
						McbFindEndOfText( lpcText, &liTempLength );
		
						lpcDupString = McbStrdup(lpcText, liTempLength);
						McbAddText(apoElement, lpcDupString, Mcb_GROWBY);
						lpcText = NULL;
					}
	
					loToken = McbGetNextToken(apoXML, &liTokenLength, &liTokenType);
					if( liTokenType != eTokenText )
					{
						apoXML->ciErrCode = eXMLErrorMissingTagName;
						return  (0);
					}
	
					if( apoElement->cpcName && strcmp(apoElement->cpcName, loToken.cpcStrBegin) == 0)
					{
                        apoXML->cpcNewElement = loToken.cpcStrBegin;
                        apoXML->ciNewElementLen = liTokenLength;
		
						return  (1);
					}
					else
					{
						lpcDupString = McbStrdup( loToken.cpcStrBegin, liTokenLength );
                        lpoNewElement = McbAddElement( apoElement, lpcDupString, liDeclaration, Mcb_GROWBY );
		
						while( lpoNewElement )
                        {
                            if( McbParseXMLElement(apoXML, lpoNewElement) == 0 )
                                return  (0);

							if( apoXML->ciEndTagLen )
							{
								if( apoElement->cpcName == NULL )
								{
									apoXML->ciErrCode = eXMLErrorUnmatchedEndTag;
									return  (0);
								}
		
								if( strncmp(apoXML->cpcEndTag, apoElement->cpcName, \
										strlen(apoElement->cpcName) ) == 0)
								{                                    
									apoXML->ciEndTagLen = 0;
								}
		
								return  (1);
							}
							
							if( apoXML->ciNewElementLen )
							{
								if( strncmp(apoXML->cpcNewElement, apoElement->cpcName, 
										strlen(apoElement->cpcName)) == 0 )
								{                                    
									return  (1);
								}
		
								lpcDupString = McbStrdup(apoXML->cpcNewElement, apoXML->ciNewElementLen);
								lpoNewElement = McbAddElement( apoElement, lpcDupString, 0, Mcb_GROWBY );
		
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
					if( lpcText )
					{
						liTempLength = loToken.cpcStrBegin - lpcText;
						McbFindEndOfText( lpcText, &liTempLength );
		
						lpcDupString = McbStrdup(lpcText, liTempLength);
						McbAddText(apoElement, lpcDupString, Mcb_GROWBY);
						lpcText = NULL;
					}
							
					loToken = McbGetNextToken(apoXML, &liTempLength, &liTokenType);					
					if( liTokenType != eTokenText )
					{
						apoXML->ciErrCode = eXMLErrorMissingEndTagName;
						return  (0);
					}
		
					lpcTemp = loToken.cpcStrBegin;
					loToken = McbGetNextToken(apoXML, &liTokenLength, &liTokenType);
		
					if( liTokenType != eTokenCloseTag )
					{
						apoXML->ciErrCode = eXMLErrorMissingEndTagName;
						return  (0);
					}
		
					//added by max at 2006.03.31
					//for the case: </abc><aa>123</aa>
					if( apoElement->cpcName == NULL )
						break;
							
					if( strncmp(lpcTemp, apoElement->cpcName, strlen(apoElement->cpcName)) )
					{
						apoXML->cpcEndTag = lpcTemp;
						apoXML->ciEndTagLen = liTempLength;
					}
		
					return  (1);
	
				case eTokenClear:
					if( lpcText )
					{
						liTempLength = loToken.cpcStrBegin - lpcText;
						McbFindEndOfText( lpcText, &liTempLength );

						lpcDupString = McbStrdup(lpcText, liTempLength);
						McbAddText(apoElement, lpcDupString, Mcb_GROWBY);
						lpcText = NULL;
					}
	
					if( McbParseClearTag(apoXML, apoElement, loToken.cpoClear) == 0 )
						return  (0);
					break;
	
				case eTokenCloseTag:		/* '>'   */
				case eTokenShortHandClose:	/* '/>'  */
					apoXML->ciErrCode = eXMLErrorUnexpectedToken;
					return  (0);
			
				case eTokenError:
				default:
					break;
			}
		}
		else if( liStatus == eInsideTag )
		{
			switch( liAttrib )
			{
				case eAttribName:
					switch( liTokenType )
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
							return  (1);
			
						case eTokenQuotedText:	/* '"SomeText"' */
						case eTokenTagStart:	/* '<'          */
						case eTokenTagEnd:		/* '</'         */
						case eTokenEquals:		/* '='          */
						case eTokenDeclaration:	/* '<?'         */
						case eTokenClear:
							apoXML->ciErrCode = eXMLErrorUnexpectedToken;
							return  (0);
									
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
							McbAddAttribute(apoElement, lpcDupString, NULL, Mcb_GROWBY);
			
							lpcTemp = loToken.cpcStrBegin;
							liTempLength = liTokenLength;
							break;
			
						case eTokenShortHandClose:
						case eTokenCloseTag:
							if( apoElement->ciIsDeclaration && (lpcTemp[liTempLength-1]) == '?')
							{
								liTempLength--;
							}
			
							if( liTempLength )
							{
								lpcDupString = McbStrdup(lpcTemp, liTempLength);
								McbAddAttribute(apoElement, lpcDupString, NULL, Mcb_GROWBY);
							}
			
							if( liTokenType == eTokenShortHandClose )
								return  (1);
			
							liStatus = eOutsideTag;
							break;
			
						case eTokenEquals:
							liAttrib = eAttribValue;
							break;
			
						case eTokenQuotedText:	/* 'Attribute "InvalidAttr"'*/
						case eTokenTagStart:	/* 'Attribute <'            */
						case eTokenTagEnd:		/* 'Attribute </'           */
						case eTokenDeclaration:	/* 'Attribute <?'           */
						case eTokenClear:
							apoXML->ciErrCode = eXMLErrorUnexpectedToken;
							return  (0);
		
						case eTokenError:
						default:
							break;	
					}
					break;
		
				case eAttribValue:
					switch( liTokenType )
					{
						case eTokenText:
						case eTokenQuotedText:
							if( apoElement->ciIsDeclaration && 
								(loToken.cpcStrBegin[liTokenLength-1]) == '?')
							{
								liTokenLength--;
							}
			
							if( liTempLength )
							{
								lpcToken = McbStrdup(loToken.cpcStrBegin, liTokenLength);
							}
							else
							{
								lpcToken = NULL;
							}
			
							lpcDupString = McbStrdup(lpcTemp, liTempLength);
							McbAddAttribute( apoElement, lpcDupString, (char *)lpcToken, Mcb_GROWBY);
			
							liAttrib = eAttribName;
							break;
			
						case eTokenTagStart:		/* 'Attr = <'   */
						case eTokenTagEnd:			/* 'Attr = </'  */
						case eTokenCloseTag:		/* 'Attr = >'   */
						case eTokenShortHandClose:	/* "Attr = />"  */
						case eTokenEquals:			/* 'Attr = ='   */
						case eTokenDeclaration:		/* 'Attr = <?'  */
						case eTokenClear:
							apoXML->ciErrCode = eXMLErrorUnexpectedToken;
							return  (0);
							break;
								
						case eTokenError:
						default:
							break;
				}
			}
		}
	}
}

void McbCountLinesAndColumns( const char *apcXML, int liUpto, McbXMLResults *apoResults )
{
	char		liChar;

	assert(apcXML);	
	assert(apoResults);

	apoResults->ciLine = 1;
	apoResults->ciColumn = 1;

	for( int liIt = 0; liIt < liUpto; liIt++ )
	{
		liChar = apcXML[liIt];
		assert( liChar );

		if( liChar == '\n' )
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

McbClearTag	*McbGetClearTags( void )
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

	return  _tags;
}

McbXMLElement *McbParseXML( const char *apcXML, McbXMLResults *apoResults )
{
	McbXMLError		liErrCode;
	McbXMLElement	*lpoRootElement = NULL;
	McbXML loInitXML = { NULL, 0, eXMLErrorNone, NULL, 0, NULL, 0, 1, NULL };

	loInitXML.cpcXML = apcXML;
	loInitXML.cpoClearTags = McbGetClearTags();

	lpoRootElement = McbCreateRoot();
	McbParseXMLElement( &loInitXML, lpoRootElement );
	liErrCode = loInitXML.ciErrCode;

	if( liErrCode != eXMLErrorNone )
	{
		McbDeleteRoot( lpoRootElement );
		lpoRootElement = NULL;
	}

	if( apoResults )
	{
		apoResults->ciErrCode = liErrCode;

		if( liErrCode != eXMLErrorNone )
		{
			McbCountLinesAndColumns( loInitXML.cpcXML, loInitXML.ciIndex, apoResults );
		}
	}

	return  (lpoRootElement);
}

McbXMLElement *McbFindElement( McbXMLElement *apoEntry, const char *apcPath )
{
	int					liIndex;
	int					liNameLength;
	int					liComResult = 0;
	const char			*lpcName = NULL;
	const char			*lpcNext = NULL;
	McbXMLElement		*lpoChild = NULL;

	assert(apcPath);
	assert(apoEntry);
	lpcNext = strchr(apcPath, '/');

	if( lpcNext == NULL )
	{
		liNameLength = strlen(apcPath);		
	}
	else
	{
		liNameLength = lpcNext - apcPath;

		if( lpcNext[1] )
		{
			lpcNext++;
		}
		else
		{
			lpcNext = NULL;
		}
	}

     if( liNameLength )
     {
          liIndex = 0;
          while( (lpoChild = McbEnumElements(apoEntry, &liIndex)) != NULL )
          {
               lpcName = lpoChild->cpcName;

               if( lpcName )
               {
                    if( strchr(apcPath, '/') == NULL )
                    {
                         liComResult = strcmp(apcPath, lpcName);
                    }
                    else
                    {
                         liComResult = strncmp(apcPath, lpcName, liNameLength);
                    }

                    if( liComResult == 0 )
                    {
                         if( lpcNext == NULL )
                         {
                              return  (lpoChild);
                         }
                         else 
                         {
                              return  McbFindElement(lpoChild, lpcNext);
                         }
                    }
               }
          }
	}

	return  (NULL);
}

size_t McbFindElements( McbXMLElement *apoHead, const char *apcName, McbXMLElement **apoElemArray, size_t aiCount )
{
    assert( apcName != NULL && apoElemArray != NULL );

    int                  liIndex      = 0;
    int                  liNameLen    = strlen(apcName) + 1;
    size_t               liElemCount  = 0;
    char                *lpcTmpName   = NULL;
    char                *lpcLeafName  = NULL;
    McbXMLElement       *lpoElemEntry = apoHead;
    McbXMLElement       *lpoChild     = NULL;
    
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

            if( strcmp(lpcLeafName, lpoChild->cpcName) == 0 )
            {
                apoElemArray[liElemCount++] = lpoChild;
            }
        }
        
    }while(false);
    
    AI_FREE( lpcTmpName );
    
    return liElemCount;
}

McbXMLAttribute *McbFindAttribute( McbXMLElement *apoEntry, const char *apcAttrib )
{
	int					liIndex;
	McbXMLAttribute		*lpoAttrib = NULL;

	assert(apoEntry);
	assert(apcAttrib);

	liIndex = 0;

	while( (lpoAttrib = McbEnumAttributes(apoEntry, &liIndex)) != NULL )
	{
		if( strcmp(lpoAttrib->cpcName, apcAttrib ) == 0 )
		{
			return  (lpoAttrib);
		}
	}

	return  (NULL);
}

McbXMLClear *McbAddCData( McbXMLElement *apoEntry, char *apcValue, int aiGrowBy )
{
	#define Mcb_GETCDATA()  &McbGetClearTags()[0]

	return  McbAddClear( apoEntry, apcValue, Mcb_GETCDATA(), aiGrowBy );
}

McbXMLElement *McbCreateElements( McbXMLElement *apoEntry, const char *apcPath )
{
	int					liIndex;
	int					liNameLength;
	int					liComResult = 0;
	char				*lpcTemp = NULL;
	const char			*lpcName = NULL;
	const char			*lpcNext = NULL;
	McbXMLElement		*lpoChild = NULL;
	McbXMLElement		*lpoNewElement = NULL;

	assert(apcPath);
	assert(apoEntry);

	if( lpcNext = strchr(apcPath, '/'), lpcNext == NULL )
	{
		liNameLength = strlen(apcPath);		
	}
	else
	{
		liNameLength = lpcNext - apcPath;

		if( lpcNext[1] )
		{
			lpcNext++;
		}
		else
		{
			lpcNext = NULL;
		}
	}

	if( liNameLength )
	{
        liIndex = 0;
        while( (lpoChild = McbEnumElements(apoEntry, &liIndex)) != NULL )
        {
            lpcName = lpoChild->cpcName;

            if( lpcName )
            {
                if( strchr(apcPath, '/') == NULL )
				{
					liComResult = strcmp(apcPath, lpcName);
				}
				else
				{
					liComResult = strncmp(apcPath, lpcName, liNameLength);
				}

				if( liComResult == 0 )
                {
                    if( lpcNext == NULL )
                    {
                        return  (lpoChild);
                    }
                    else 
                    {
                        return  McbCreateElements( lpoChild, lpcNext );
                    }
                }
            }
        }

		lpcTemp = McbStrdup( apcPath, liNameLength );
		lpoNewElement = McbAddElement( apoEntry, lpcTemp, 0, Mcb_GROWBY );

		if( lpcNext == NULL )
		{
			return  (lpoNewElement);
		}
        else 
		{
			return  McbCreateElements( lpoNewElement, lpcNext );
		}
	}

	return  (NULL);
}

int McbCreateXMLStringR( McbXMLElement *apoEntry, char *apcMarker, int aiFormat )
{
	int					liIndex;
	int					liLength;
	int					liResult = 0;
	int					liChildFormat;
	int					liElementLength;
	int					liHasChildren = 0;
	McbXMLNode			*lpoChildNode = NULL;
	McbXMLAttribute		*lpoAttrib = NULL;

	assert( apoEntry );
	liElementLength = Mcb_LENSTR( apoEntry->cpcName );

	if( liElementLength )
	{
		liLength = (aiFormat == -1 ? 0 : aiFormat);

        if( apcMarker )
        {
            if( liLength )
            {
                memset( apcMarker, Mcb_INDENTCHAR, liLength );
            }

            liResult = liLength;
            apcMarker[liResult++] = '<';        
            strcpy( &apcMarker[liResult], apoEntry->cpcName );
            
            liResult += liElementLength;
            apcMarker[liResult++] = ' ';        
        }                
        else
        {
            liResult += (liLength + liElementLength + 2 );
        }
        
        liIndex = 0;
        while( (lpoChildNode = McbEnumNodes( apoEntry, &liIndex )) != NULL )
        {
            switch( lpoChildNode->ciType )
            {
	            case eNodeAttribute:
	                {
	                    lpoAttrib = lpoChildNode->coNode.cpoAttrib;
	                    liLength = Mcb_LENSTR( lpoAttrib->cpcName );
	
	                    if( liLength )
	                    {
	                        if( apcMarker )
	                        {
	                            strcpy( &apcMarker[liResult], lpoAttrib->cpcName );             
	                        }
	
	                        liResult += liLength;
	                        liLength = Mcb_LENSTR( lpoAttrib->cpcValue );
	
	                        if( liLength )
	                        {
	                            if( apcMarker )
	                            {
	                                apcMarker[liResult] = (char)'=';
	                                strcpy( &apcMarker[liResult+1], lpoAttrib->cpcValue );
	                            }
	                            
	                            liResult += (liLength + 1);
	                        }
	
	                        if( apcMarker )
	                        {
	                            apcMarker[liResult] = (char)' ';                              
	                        }
	
	                        liResult++;
	                    }
					}
					break;
	
	            case eNodeEmpty:
	                continue;
	
	            default:
	                liHasChildren = 1;
	                break;
            }
        }
        
        if( liHasChildren )
        {   
            if( apcMarker )
            {
                apcMarker[liResult-1] = (char)'>';
            }

            if( aiFormat != (-1) )
            {
                if( apcMarker )
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

	if( aiFormat == (-1) )
	{
		liChildFormat = -1;
	}
	else
	{
		if( liElementLength ) 
		{
			liChildFormat = aiFormat + 1;
		}
		else
		{
			liChildFormat = aiFormat;
		}	
	}
     bool lbIsText = false;
	liIndex = 0;
	while( (lpoChildNode = McbEnumNodes(apoEntry, &liIndex)) != NULL )
	{
          lbIsText = false;
		switch( lpoChildNode->ciType )
		{
			case eNodeText:
				{
	                liLength = Mcb_LENSTR(lpoChildNode->coNode.cpoText->cpcValue);
	
	                if( liLength )
	                {
						if( aiFormat != (-1) )
						{
                                   if( apcMarker )
                                   {
                                        // memset( &apcMarker[liResult], Mcb_INDENTCHAR, aiFormat + 1);
                                        //strcpy( &apcMarker[liResult + aiFormat + 1], lpoChildNode->coNode.cpoText->cpcValue);
                                        //	apcMarker[ liResult + aiFormat + 1 + liLength] = (char)'\n';
                                        strcpy( &apcMarker[liResult-1], lpoChildNode->coNode.cpoText->cpcValue);
                                   }

                                   //liResult += (liLength + aiFormat + 2);
                                   liResult += (liLength - 1);
                                   lbIsText = true;
						}
						else
						{
                                   if( apcMarker )
                                   {
                                        strcpy( &apcMarker[liResult], lpoChildNode->coNode.cpoText->cpcValue);
                                   }

                                   liResult += liLength;
						}               
	                }
				}
				break;
	
			case eNodeClear:
				{
	                liLength = Mcb_LENSTR(lpoChildNode->coNode.cpoClear->cpcOpenTag);
	
	                if( liLength )
	                {
						if( aiFormat != (-1) )
						{
	                        if( apcMarker )
	                        {
	                            memset( &apcMarker[liResult], Mcb_INDENTCHAR, aiFormat + 1);
	                            strcpy( &apcMarker[liResult + aiFormat + 1], lpoChildNode->coNode.cpoClear->cpcOpenTag);
	
								/* apcMarker[liResult + aiFormat + 1 + liLength] = (char)'\n'; */
	                        }
	
	                        /* liResult += (liLength + aiFormat + 2); */
							liResult += (liLength + aiFormat + 1);
						}
						else
						{
							if( apcMarker )
	                        {
								strcpy( &apcMarker[liResult], lpoChildNode->coNode.cpoClear->cpcOpenTag);
							}
	
							liResult += liLength;
						}               
	                }
	
					liLength = Mcb_LENSTR(lpoChildNode->coNode.cpoClear->cpcValue);

	                if( liLength )
	                {
						if( apcMarker )
	                    {
							strcpy( &apcMarker[liResult], lpoChildNode->coNode.cpoClear->cpcValue);
						}
	
						liResult += liLength;
					}
	
					liLength = Mcb_LENSTR(lpoChildNode->coNode.cpoClear->cpcCloseTag);
	
	                if( liLength )
	                {
						if( apcMarker )
	                    {
							strcpy( &apcMarker[liResult], lpoChildNode->coNode.cpoClear->cpcCloseTag);
						}
	
						liResult += liLength;
					}
	
					if( aiFormat != (-1) )
					{
						if( apcMarker )
						{
							apcMarker[liResult] = (char)'\n';
						}
	
						liResult++;
					}
				}
				break;
				
			case eNodeElement:
				{
					liResult += McbCreateXMLStringR( lpoChildNode->coNode.cpoElement, 
						(apcMarker ? apcMarker + liResult : NULL), liChildFormat );
				}
	
			default:
				break;
		}
	}

	if( liElementLength )
	{
        if( liHasChildren )
        {
            if( apcMarker )
            {
                if( aiFormat != (-1) && lbIsText == false )
                {
                    if( aiFormat )
                    {
                        memset( &apcMarker[liResult], Mcb_INDENTCHAR, aiFormat );
                        liResult += aiFormat;
                    }
                }       

                strcpy( &apcMarker[liResult], "</" );        
                liResult += 2;
                
                strcpy( &apcMarker[liResult], apoEntry->cpcName );
                liResult += liElementLength;
                
                if( aiFormat == (-1) )
                {
                    strcpy( &apcMarker[liResult], ">" );
                    liResult++;
                }
                else
                {
                    strcpy( &apcMarker[liResult], ">\n" );
                    liResult += 2;
                }
            }
            else
            {
                if( aiFormat != (-1) )
                {
                    liResult += (liElementLength + 4 + aiFormat);
                }
                else
                {
                    liResult += (liElementLength + 3);
                }
            }
        }
        else 
        {
            if( apcMarker )
            {
                if( aiFormat == (-1) )
                {
                    strcpy( &apcMarker[liResult], "/>" ); 
                    liResult += 2;
                }
                else
                {
                    strcpy( &apcMarker[liResult], "/>\n" ); 
                    liResult += 3;
                }
            }
            else
            {
                liResult += (aiFormat == -1 ? 2 : 3);
            }
        }
	}

	return  (liResult);
}

char *McbCreateXMLString( McbXMLElement *apoHead, int aiFormat, int *apiSize )
{
	int			liFormat;
	int			liStrLength;
	char		*lpcResult = NULL;

	if( apoHead )
	{
		liFormat = (aiFormat ? 0 : -1);
		
		if( (liStrLength = McbCreateXMLStringR( apoHead, NULL, liFormat )) <= 0 )
			return  (NULL);

		AI_MALLOC_ASSERT( lpcResult, char, (liStrLength+1) );

		McbCreateXMLStringR( apoHead, lpcResult, liFormat );
		if(apiSize)  *apiSize = liStrLength;
	
		return  (lpcResult);
	}

	return  (NULL);
}
