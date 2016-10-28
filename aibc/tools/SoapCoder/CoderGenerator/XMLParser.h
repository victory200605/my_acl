//=============================================================================
/**
 * \file    XMLParser.h
 * \brief  xml parser
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: XMLParser.h,v 1.8 2010/07/17 13:49:11 daizh Exp $
 *
 * History
 * 2008.01.28 first release by DZH
 */
//=============================================================================

#ifndef __AI_XMLPARSER_H__
#define __AI_XMLPARSER_H__

#include <string>
#include <map>
using namespace std;

#include "AIMcbXML.h"
#include "Element.h"
#include "Context.h"
#include "AIFile.h"

inline const char* StrcharLast( const char* apsName, char ach )
{
    const char* lpsName = apsName;
    const char* lpsNextName = apsName;
    while ( ( lpsNextName = strchr( lpsNextName, ach ) ) != NULL )
    {
        lpsNextName++;
        lpsName = lpsNextName;
    }
    return lpsName;
}

inline const char* EraseNs( const char* apsName )
{
    return StrcharLast( apsName, ':' );
}

inline const char* EraseNs( const char* apsName, std::string& aoStrNamespace )
{
    const char* lpsStart = apsName;
    const char* lpsColon = StrcharLast( apsName, ':' );
    
    aoStrNamespace.assign( lpsStart, 0, lpsColon - lpsStart - 1 );
    
    return lpsColon;
}

inline char* EraseInvalidChar( const char* apsName )
{
    static char slsBuff[AI_MAX_BUFF_SIZE];
    int liInd = 0;
    while ( *apsName++ )
    {
        if ( *apsName == '\"' || *apsName == ' ' )
        {
            continue;
        }
        slsBuff[liInd++] = *apsName;
    }
    slsBuff[liInd] = '\0';

    return slsBuff;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void InitializeNatureType( clsContext& aoContext )
{
    for ( clsContext::TNatureTypeIter loIter = aoContext.BeginNatureType();
          loIter != aoContext.EndNatureType(); loIter++ )
    {
        clsElement* lpoElement  = new clsSingleElement( loIter->second.c_str(), loIter->first.c_str() );
        lpoElement->SetType(clsElement::eNature);
        clsTypes::Instance()->AddElement( lpoElement->GetName(), lpoElement );
    }
}

//////////////////////////////////////////////////// schema - parser ///////////////////////////////////////////
class clsXmlSchemaParser
{
public:
    clsXmlSchemaParser() 
    {
    }
    ~clsXmlSchemaParser()
    {
    }
    
    bool ParserFromFile( const char* apcFileName )
    {
        clsFile        loFile;
        std::string    loStrBuffer;
        const char*    lpcLine = NULL;
        
        if ( !loFile.Open(apcFileName, "r") )
        {
            return false;
        }
        
        while ( ( lpcLine = loFile.GetLine() ) != NULL )
        {
            loStrBuffer += lpcLine;
        }

        return this->ParserFromBuffer( loStrBuffer.c_str() );
    }
    
    virtual bool ParserFromBuffer( const char* apcXml )
    {
        McbXMLResults  loStat;
        McbXMLElement* lpoRootElement = NULL;
        McbXMLElement* lpoCurrElement = NULL;
        McbXMLElement* lpoSubElement  = NULL;

        if ( ( lpoRootElement = McbParseXML( apcXml, &loStat ) ) == NULL )
        {
            return false;
        }

        int liIndex = 0;
        lpoCurrElement = lpoRootElement;
        while ( ( lpoSubElement = McbEnumElements( lpoCurrElement, &liIndex ) ) != NULL )
        {
            if ( ::stricmp( "definitions", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
                lpoCurrElement = lpoSubElement;
                liIndex = 0;
                continue;
            }
            else if ( ::stricmp( "types", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
                lpoCurrElement = lpoSubElement;
                liIndex = 0;
                continue;
            }
            else if ( ::stricmp( "schema", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
                int liTemp = 0;
                McbXMLElement* lpoTemp = NULL;
                while ( (lpoTemp = McbEnumElements(lpoSubElement, &liTemp ) ) != NULL )
                {
                    if ( stricmp( "complexType", EraseNs(lpoTemp->cpcName) ) == 0 )
                    {
                        clsElement* lpoElement = this->ComplexElement( lpoTemp );
                        clsTypes::Instance()->AddElement( lpoElement->GetName(), lpoElement );
                    }
                    else if ( stricmp( "element", EraseNs(lpoTemp->cpcName) ) == 0 )
                    {
                        clsElement* lpoElement = this->SingleElement( lpoTemp );
                        if (lpoElement != NULL)
                        {
                            clsTypes::Instance()->AddElement( lpoElement->GetName(), lpoElement );
                        }
                    }
        			else if ( stricmp( "simpleType", EraseNs(lpoTemp->cpcName) ) == 0 )
        			{
        				clsElement* lpoElement = this->SimpleElement( lpoTemp );
        				clsTypes::Instance()->AddElement( lpoElement->GetName(), lpoElement );
        			}
                }
            }
        }

        clsTypes::Instance()->ProcessTypeLink();
        clsTypes::Instance()->ProcessExtendComplexType();
        clsTypes::Instance()->ProcessForwardDeclare();
            
        McbDeleteRoot( lpoRootElement );
        return true;
    }
    
protected:
    clsElement* ComplexElement( McbXMLElement* lpoMcbElement )
    {
        int            liIndex = 0;
        McbXMLElement* lpoCurrElement = lpoMcbElement;
        McbXMLElement* lpoSubElement  = NULL;
        clsComplexElement* lpoComplexElement  = new clsComplexElement;

        McbXMLAttribute * lpoAttr = McbFindAttribute( lpoMcbElement, "name" );
        if ( lpoAttr == NULL && ( lpoAttr = McbFindAttribute( lpoMcbElement->cpoParent, "name" ) ) == NULL )
        {
            return NULL;
        }
        
        lpoComplexElement->SetTypeName( EraseInvalidChar( lpoAttr->cpcValue ) );
        lpoComplexElement->SetName( EraseInvalidChar( lpoAttr->cpcValue ) );

        while ( ( lpoSubElement = McbEnumElements( lpoCurrElement, &liIndex ) ) != NULL )
        {
            if ( ::stricmp( "complexContent", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
                lpoCurrElement = lpoSubElement;
                liIndex = 0;
                continue;
            }
            else if ( ::stricmp( "simpleContent", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
                lpoCurrElement = lpoSubElement;
                liIndex = 0;
                continue;
            }
            else if ( ::stricmp( "extension", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
                lpoCurrElement = lpoSubElement;
                liIndex = 0;
                McbXMLAttribute * lpoAttr = McbFindAttribute( lpoSubElement, "base" );
                lpoComplexElement->SetExtension( EraseNs( EraseInvalidChar( lpoAttr->cpcValue ) ) );
                continue;
            }
            else if ( ::stricmp( "sequence", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
                lpoCurrElement = lpoSubElement;
                liIndex = 0;
                continue;
            }
            else if ( ::stricmp( "element", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
                lpoComplexElement->SetSubElement( SingleElement( lpoSubElement ) );
            }
        }

        return lpoComplexElement;
    }

	clsElement* SimpleElement( McbXMLElement* lpoMcbElement )
	{
		int            liIndex = 0;
		McbXMLElement* lpoCurrElement = lpoMcbElement;
		McbXMLElement* lpoSubElement  = NULL;
		std::string    loStrName;
		std::string    loStrType;

		McbXMLAttribute * lpoAttr = McbFindAttribute( lpoMcbElement, "name" );
		if ( lpoAttr == NULL && ( lpoAttr = McbFindAttribute( lpoMcbElement->cpoParent, "name" ) ) == NULL )
		{
			return NULL;
		}

		loStrName = EraseInvalidChar( lpoAttr->cpcValue );
		loStrType = EraseInvalidChar( lpoAttr->cpcValue );

		while ( ( lpoSubElement = McbEnumElements( lpoCurrElement, &liIndex ) ) != NULL )
		{
			if ( ::stricmp( "complexContent", EraseNs(lpoSubElement->cpcName) ) == 0 )
			{
				lpoCurrElement = lpoSubElement;
				liIndex = 0;
				continue;
			}
			else if ( ::stricmp( "simpleContent", EraseNs(lpoSubElement->cpcName) ) == 0 )
			{
				lpoCurrElement = lpoSubElement;
				liIndex = 0;
				continue;
			}
			else if ( ::stricmp( "restriction", EraseNs(lpoSubElement->cpcName) ) == 0 )
			{
				lpoCurrElement = lpoSubElement;
				liIndex = 0;
				McbXMLAttribute * lpoAttr = McbFindAttribute( lpoSubElement, "base" );
				loStrType = EraseNs( EraseInvalidChar( lpoAttr->cpcValue ) );

				return new clsSingleElement( loStrType.c_str(), loStrName.c_str() );
			}
		}

		return NULL;
	}

    clsElement* SingleElement( McbXMLElement* lpoMcbElement )
    {
        McbXMLAttribute* lpoAttrName  = McbFindAttribute( lpoMcbElement, "name" );
        McbXMLAttribute* lpoAttrType  = McbFindAttribute( lpoMcbElement, "type" );
        McbXMLAttribute* lpoMinOccurs = McbFindAttribute( lpoMcbElement, "minOccurs" );
        McbXMLAttribute* lpoMaxOccurs = McbFindAttribute( lpoMcbElement, "maxOccurs" );

        std::string loStrName( EraseInvalidChar( lpoAttrName->cpcValue ) );
        std::string loStrType( EraseNs( EraseInvalidChar( lpoAttrType->cpcValue ) ) );
            
        if (loStrName == loStrType)
        {
            return NULL;
        }

        clsElement* lpoElement  = new clsSingleElement( loStrType.c_str(), loStrName.c_str() );
        if ( lpoMinOccurs != NULL )
        {
           lpoElement->SetMinOccurs( atoi(lpoMinOccurs->cpcValue) );
        }
        
        if ( lpoMaxOccurs != NULL && stricmp( "unbounded", EraseInvalidChar(lpoMaxOccurs->cpcValue) ) == 0 )
        {
           lpoElement->SetMaxOccurs( 2000000000 ); //max
        }
        else if ( lpoMaxOccurs != NULL )
        {
           lpoElement->SetMaxOccurs( atoi(lpoMaxOccurs->cpcValue) );
        }
        
        return lpoElement;
    }
    
protected:
    clsContext* cpoContext;
};

//////////////////////////////////////////////////// schema - parser ///////////////////////////////////////////
class clsXmlWSDLParser : public clsXmlSchemaParser
{
public:
	clsXmlWSDLParser( clsContext& aoContext ) : coContext(aoContext)
    {
    }
    ~clsXmlWSDLParser()
    {
    }
    virtual bool ParserFromBuffer( const char* apcXml )
    {
        McbXMLResults  loStat;
        McbXMLElement* lpoRootElement = NULL;
        McbXMLElement* lpoCurrElement = NULL;
        McbXMLElement* lpoSubElement  = NULL;

        if ( ( lpoRootElement = McbParseXML( apcXml, &loStat ) ) == NULL )
        {
            return false;
        }

        int liIndex = 0;
        lpoCurrElement = lpoRootElement;
        while ( ( lpoSubElement = McbEnumElements( lpoCurrElement, &liIndex ) ) != NULL )
        {
            if ( ::stricmp( "definitions", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
 				GetNamespace( lpoSubElement );
                lpoCurrElement = lpoSubElement;
                liIndex = 0;
                continue;
            }
            else if ( ::stricmp( "message", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
                if ( this->GetMessageElement( lpoSubElement ) == 0 )
                {
                    ///Error
                }
            }
            else if ( ::stricmp( "portType", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
                if ( this->GetPortTypeElement( lpoSubElement ) == 0 )
                {
                    ///Error
                }
            }
            else if ( ::stricmp( "binding", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
                if ( this->GetBindingElement( lpoSubElement ) == 0 )
                {
                    ///Error
                }
            }
            else if ( ::stricmp( "service", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
                if ( this->GetServiceElement( lpoSubElement ) == 0 )
                {
                    ///Error
                }
            }
        }
        
        McbDeleteRoot( lpoRootElement );
        
        return true;
    }

protected:
	void GetNamespace( McbXMLElement* lpoMcbElement )
	{
		int liIndex = 0;
		McbXMLAttribute* lpoAttr = NULL;
		std::map<std::string, std::string> loMap;
		std::string loTargetNamespace;
		std::string loXsd = "http://www.w3.org/2001/XMLSchema";
		std::string loXsi = "http://www.w3.org/2001/XMLSchema-instance";

		while ( (lpoAttr = McbEnumAttributes(lpoMcbElement, &liIndex)) != NULL )
		{
			if ( strcmp(lpoAttr->cpcName, "targetNamespace" ) == 0 )
			{
				loTargetNamespace = EraseInvalidChar( lpoAttr->cpcValue);
				continue;
			}

			loMap[EraseInvalidChar( lpoAttr->cpcValue)] = EraseNs(lpoAttr->cpcName);
		}

		///xsd
		if ( loMap.find(loXsd) != loMap.end() )
		{
			coContext.SetNamespace( "xsd", loMap.find(loXsd)->second.c_str(), loXsd.c_str() );
		}

		///xsi
		if ( loMap.find(loXsi) != loMap.end() )
		{
			coContext.SetNamespace( "xsi", loMap.find(loXsi)->second.c_str(), loXsi.c_str() );
		}

		///targetName
		if ( loMap.find(loTargetNamespace) != loMap.end() )
		{
			coContext.SetNamespace( "targetNamespace", loMap.find(loTargetNamespace)->second.c_str(), loTargetNamespace.c_str() );
		}
	}

    /* get message element */
    TMessageNode* GetMessageElement( McbXMLElement* lpoMcbElement )
    {  
        McbXMLAttribute * lpoAttr = McbFindAttribute( lpoMcbElement, "name" );
        if ( lpoAttr == NULL  )
        {
            return NULL;
        }
        TMessageNode* lpoMessageNode  = clsMessage::Instance()->AddMessage( EraseNs( EraseInvalidChar( lpoAttr->cpcValue ) ) );
    
        McbXMLElement* lpoSubElement = NULL;
        int liIndex = 0;
        while ( ( lpoSubElement = McbEnumElements( lpoMcbElement, &liIndex ) ) != NULL )
        {
			if ( ::stricmp( "part", EraseNs(lpoSubElement->cpcName) ) != 0 )
			{
				continue;
			}

            McbXMLAttribute * lpoAttrName = McbFindAttribute( lpoSubElement, "name" );
            McbXMLAttribute * lpoAttrElem = McbFindAttribute( lpoSubElement, "element" );
            McbXMLAttribute * lpoAttrType = McbFindAttribute( lpoSubElement, "type" );
            if ( lpoAttrName == NULL || ( lpoAttrElem == NULL && lpoAttrType == NULL ) )
            {
                 return NULL;
            }
            
            if ( lpoAttrType == NULL ) lpoAttrType = lpoAttrElem;
            
            std::string loStrNamespace;
            std::string loStrName = EraseNs( EraseInvalidChar( lpoAttrName->cpcValue ) );
            std::string loStrElemName = EraseNs( EraseInvalidChar( lpoAttrType->cpcValue ), loStrNamespace );

            clsMessage::Instance()->AddPart( lpoMessageNode, loStrName.c_str(), loStrNamespace.c_str(), loStrElemName.c_str() );
        }
    
        return lpoMessageNode;
    }
    /* end for message element*/
    
    /* start for porttype element*/
    TPortTypeNode* GetPortTypeElement( McbXMLElement* lpoMcbElement )
    {
        McbXMLAttribute * lpoAttr = McbFindAttribute( lpoMcbElement, "name" );
        if ( lpoAttr == NULL  )
        {
            return NULL;
        }
        TPortTypeNode* lpoPortTypeNode = clsPortType::Instance()->AddPortType( EraseNs( EraseInvalidChar( lpoAttr->cpcValue ) ) );
        
        McbXMLElement* lpoSubElement = NULL;
        int liIndex = 0;
        while ( ( lpoSubElement = McbEnumElements( lpoMcbElement, &liIndex ) ) != NULL )
        {
			std::string loOptName;
			std::string loInputMessage;
			std::string loOutputMessage;

            if ( ::stricmp( "operation", EraseNs(lpoSubElement->cpcName) ) == 0 )
            {
                // Operation Name
                McbXMLAttribute* lpoOptName = McbFindAttribute( lpoSubElement, "name" );
                if ( lpoOptName == NULL ) 
                {
                    return NULL;
                }
                loOptName = EraseNs( EraseInvalidChar( lpoOptName->cpcValue ) );
                
                int liOptIndex = 0;
                McbXMLElement* lpoOptElement = NULL;
                while ( ( lpoOptElement = McbEnumElements( lpoSubElement, &liOptIndex ) ) != NULL )
                {
                    if ( ::stricmp( "input", EraseNs(lpoOptElement->cpcName) ) == 0 )
                    {
                        McbXMLAttribute* lpoAttrMessage = McbFindAttribute( lpoOptElement, "message" );
                        if ( lpoAttrMessage != NULL )
                        {
                            loInputMessage = EraseNs( EraseInvalidChar( lpoAttrMessage->cpcValue ) );
                        }
                    }
                    else if ( ::stricmp( "output", EraseNs(lpoOptElement->cpcName) ) == 0 )
                    {
                        McbXMLAttribute* lpoAttrMessage = McbFindAttribute( lpoOptElement, "message" );
                        if ( lpoAttrMessage != NULL )
                        {
                            loOutputMessage = EraseNs( EraseInvalidChar( lpoAttrMessage->cpcValue ) );
                        }
                    }
                }
                
                clsPortType::Instance()->AddOperation( lpoPortTypeNode, loOptName.c_str(), loInputMessage.c_str(), loOutputMessage.c_str() );
            }
        }
    
        return lpoPortTypeNode;
    }
    /* end for porttype element*/
    
    /* start for porttype element*/
    TBindingNode* GetBindingElement( McbXMLElement* lpoMcbElement )
    {
        McbXMLAttribute * lpoAttrName = McbFindAttribute( lpoMcbElement, "name" );
        McbXMLAttribute * lpoAttrType = McbFindAttribute( lpoMcbElement, "type" );
        if ( lpoAttrName == NULL || lpoAttrType == NULL )
        {
            return NULL;
        }
        std::string loStrName = EraseNs( EraseInvalidChar( lpoAttrName->cpcValue ) );
        std::string loStrType = EraseNs( EraseInvalidChar( lpoAttrType->cpcValue ) );
        TBindingNode* lpoBindingNode = clsBinding::Instance()->AddBinding( loStrName.c_str(), loStrType.c_str() );
        
        McbXMLElement* lpoSubElement = NULL;
        int liIndex = 0;
        while ( ( lpoSubElement = McbEnumElements( lpoMcbElement, &liIndex ) ) != NULL )
        {
            if ( ::stricmp( "operation", EraseNs(lpoSubElement->cpcName) ) != 0 )
            {
                continue;
            }
			
            std::string loOptName;
            clsBinding::stEntryDesc loInput;
            clsBinding::stEntryDesc loOutput;
            
            // Operation Name
            McbXMLAttribute * lpoAttrOptName = McbFindAttribute( lpoSubElement, "name" );
            if ( lpoAttrOptName != NULL )
            {
                loOptName = EraseNs( EraseInvalidChar( lpoAttrOptName->cpcValue ) );
            }
            
            int liOptIndex = 0;
            McbXMLElement* lpoOptElement = NULL;
            while ( ( lpoOptElement = McbEnumElements( lpoSubElement, &liOptIndex ) ) != NULL )
            {
                if ( ::stricmp( "input", EraseNs(lpoOptElement->cpcName) ) == 0 )
                {
                    // Input/body
					McbXMLElement* lpoElemArray[1000];
					int liSize = FindMcbElements( lpoOptElement, "body", lpoElemArray, 1000 );
					loInput.cvBody.resize(liSize);
					for( int liIt = 0; liIt < liSize; liIt++ )
					{
						ParseBindingOpt( lpoElemArray[liIt], loInput.cvBody[liIt] );
					}

                    // Input/header
                    liSize = FindMcbElements( lpoOptElement, "header", lpoElemArray, 1000 );
                    loInput.cvHeader.resize(liSize);
                    for( int liIt = 0; liIt < liSize; liIt++ )
                    {
                        ParseBindingOpt( lpoElemArray[liIt], loInput.cvHeader[liIt] );
                    }
                }
                else if ( ::stricmp( "output", EraseNs(lpoOptElement->cpcName) ) == 0 )
                {
                    // Output/body
					McbXMLElement* lpoElemArray[1000];
					int liSize = FindMcbElements( lpoOptElement, "body", lpoElemArray, 1000 );
					loOutput.cvBody.resize(liSize);
					for( int liIt = 0; liIt < liSize; liIt++ )
					{
						ParseBindingOpt( lpoElemArray[liIt], loOutput.cvBody[liIt] );
					}

                    // Output/header
                    liSize = FindMcbElements( lpoOptElement, "header", lpoElemArray, 1000 );
                    loOutput.cvHeader.resize(liSize);
                    for( int liIt = 0; liIt < liSize; liIt++ )
                    {
                        ParseBindingOpt( lpoElemArray[liIt], loOutput.cvHeader[liIt] );
                    }
                }
            }
            clsBinding::Instance()->AddOperation( lpoBindingNode, loOptName.c_str(), loInput, loOutput );
        }
    
        return lpoBindingNode;
    }
    void ParseBindingOpt( McbXMLElement* lpoMcbElement, clsBinding::stFieldDesc& aoRet )
    {
        McbXMLAttribute* lpoAttr = NULL;
        
        // Operation Name
        lpoAttr = McbFindAttribute( lpoMcbElement, "message" );
        if ( lpoAttr != NULL ) 
        {
            aoRet.coStrMessage = EraseNs( EraseInvalidChar( lpoAttr->cpcValue ) );
        }
        
        lpoAttr = McbFindAttribute( lpoMcbElement, "parts" );
        if ( lpoAttr != NULL ) 
        {
            aoRet.cvStrPart.push_back( EraseNs( EraseInvalidChar( lpoAttr->cpcValue ) ) );
        }
        lpoAttr = McbFindAttribute( lpoMcbElement, "part" );
        if ( lpoAttr != NULL ) 
        {
            aoRet.cvStrPart.push_back( EraseNs( EraseInvalidChar( lpoAttr->cpcValue ) ) );
        }
        
        lpoAttr = McbFindAttribute( lpoMcbElement, "use" );
        if ( lpoAttr != NULL ) 
        {
            aoRet.coStrUse = EraseInvalidChar( lpoAttr->cpcValue );
        }
        
        lpoAttr = McbFindAttribute( lpoMcbElement, "encodingStyle" );
        if ( lpoAttr != NULL ) 
        {
            aoRet.coStrEncodingStyle = EraseInvalidChar( lpoAttr->cpcValue );
        }
        
        lpoAttr = McbFindAttribute( lpoMcbElement, "namespace" );
        if ( lpoAttr != NULL ) 
        {
            aoRet.coStrNamespace = EraseInvalidChar( lpoAttr->cpcValue );
        }
    }

	size_t FindMcbElements( McbXMLElement* apoMcbElement, const char* apcName, McbXMLElement** appoRetBuffer, size_t aiCount )
	{
		int liOptIndex = 0;
		int liRetCount = 0;
		McbXMLElement* lpoSubElement = NULL;
		while ( ( lpoSubElement = McbEnumElements( apoMcbElement, &liOptIndex ) ) != NULL )
		{
			if ( ::stricmp( apcName, EraseNs(lpoSubElement->cpcName) ) == 0 )
			{
				appoRetBuffer[liRetCount++] = lpoSubElement;
			}
			if ( liRetCount >= aiCount ) break;
		}

		return liRetCount;
	}
    /* end for porttype element*/
    
    /* start for porttype element*/
    TServiceNode* GetServiceElement( McbXMLElement* lpoMcbElement )
    {
        McbXMLAttribute * lpoAttrName = McbFindAttribute( lpoMcbElement, "name" );
        if ( lpoAttrName == NULL )
        {
            return NULL;
        }
        std::string loStrName = EraseNs( EraseInvalidChar( lpoAttrName->cpcValue ) );
            
        TServiceNode* lpoServiceNode = clsService::Instance()->AddService( loStrName.c_str() );
        
        McbXMLElement* lpoSubElement = NULL;
        int liIndex = 0;
        while ( ( lpoSubElement = McbEnumElements( lpoMcbElement, &liIndex ) ) != NULL )
        {
			if ( ::stricmp( "port", EraseNs(lpoSubElement->cpcName) ) != 0 )
			{
				continue;
			}

            std::string loStrPortName;
            std::string loStrBinding;
                
            McbXMLAttribute * liTmpAttr = McbFindAttribute( lpoSubElement, "name" );
            if ( liTmpAttr != NULL )
            {
                loStrPortName = EraseNs( EraseInvalidChar( liTmpAttr->cpcValue ) );
            }
            liTmpAttr = McbFindAttribute( lpoSubElement, "binding" );
            if ( liTmpAttr != NULL )
            {
                loStrBinding = EraseNs( EraseInvalidChar( liTmpAttr->cpcValue ) );
            }
            
            clsService::Instance()->AddPort( lpoServiceNode, loStrPortName.c_str(), loStrBinding.c_str() );
        }
        return lpoServiceNode;
    }
    /* end for porttype element*/

protected:
	clsContext& coContext;
};

#endif
