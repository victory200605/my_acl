
#ifndef __XMLPARSE_H__
#define __XMLPARSE_H__

#include "AIMcbXML.h"
#include "ObjectType.h"

class clsXMLParser
{
public:
	clsXMLParser( ) : cpoRootElement(NULL)
	{
		
	}
	
	static Recycle( TObjectList& aoObjectType )
	{
	    for ( TObjectList::iterator liIt = aoObjectType.begin(); liIt != aoObjectType.end(); liIt++ )
	    {
	        delete (*liIt);
	    }
	    aoObjectType.clear();
	}
	
	int Parser( const char* apcXML, TObjectList& aoObjectType )
	{
		cpoRootElement = ::McbParseXML( apcXML, &coStat );
		if ( cpoRootElement == NULL )
		{
			throw clsException( "Parser Xml fail" );
		}

		McbXMLElement* lpoSubElemen = NULL;
		McbXMLElement* lpoCurrElement = McbFindElement( cpoRootElement, "smi/nodes" );
		if ( lpoCurrElement == NULL )
		{
			throw clsException( "Can't find path `smi/nodes`" );
		}

		int liIndex = 0;
		while ( ( lpoSubElemen = McbEnumElements( lpoCurrElement, &liIndex ) ) != NULL )
		{
			if ( strcmp(lpoSubElemen->cpcName, "scalar" ) == 0 )
			{
				clsObjectType* lpoObjectType = this->GetObjectType( lpoSubElemen);

				aoObjectType.push_back( lpoObjectType );
			}
			if ( strcmp(lpoSubElemen->cpcName, "table" ) == 0 )
			{
				clsTable* lpoTable = this->GetTable( lpoSubElemen);

				aoObjectType.push_back( lpoTable );
			}
		}

		return 0;
	}

protected:
	clsObjectType* GetObjectType( McbXMLElement* apoCurrElement )
	{
		//get attribute
		McbXMLAttribute* lpoName = McbFindAttribute( apoCurrElement, "name" );
		McbXMLAttribute* lpoOid = McbFindAttribute( apoCurrElement, "oid" );
		const char* lpcType = GetElementType( apoCurrElement );

		if ( lpoName == NULL || lpoOid == NULL || lpcType == NULL )
		{
		    throw clsException( "Can't parser scalar for object, maybe name or oid attribute excep" );
		}

		std::string loStrName = this->EraseInvalidChar( lpoName->cpcValue );
		std::string loStrOid  = this->EraseInvalidChar( lpoOid->cpcValue );
		std::string loStrType = this->EraseInvalidChar( lpcType );

		return new clsObjectType( loStrName.c_str(), loStrOid.c_str(), loStrType.c_str() );
	}

	clsTable* GetTable( McbXMLElement* apoCurrElement )
	{
		//get attribute
		McbXMLAttribute* lpoName = McbFindAttribute( apoCurrElement, "name" );
		McbXMLAttribute* lpoOid = McbFindAttribute( apoCurrElement, "oid" );
		
        if ( lpoName == NULL || lpoOid == NULL )
		{
		    throw clsException( "Can't parser table, maybe name or oid attribute excep" );
		}
		
		std::string loStrName = this->EraseInvalidChar( lpoName->cpcValue );
		std::string loStrOid  = this->EraseInvalidChar( lpoOid->cpcValue );

		clsTable* lpoTable = new clsTable( loStrName.c_str(), loStrOid.c_str() );

		if ( ( apoCurrElement = McbFindElement( apoCurrElement, "row" ) ) == NULL )
		{
		    throw clsException( Format( "Can't find row for table %s", loStrName.c_str() ) );
		}

		int liIndex = 0;
		McbXMLElement* lpoSubElement = NULL;
		while ( ( lpoSubElement = McbEnumElements( apoCurrElement, &liIndex ) ) != NULL )
		{
			if ( strcmp( lpoSubElement->cpcName, "column" ) == 0 )
			{
				//get attribute
				clsObjectType* lpoColumn = GetObjectType( lpoSubElement );

				lpoTable->AddColumn( lpoColumn );
			}
		}

		return lpoTable;
	}

protected:
	const char* GetElementType( McbXMLElement* apoElement )
	{
		McbXMLElement* lpoCurrElement = NULL;

		lpoCurrElement = McbFindElement( apoElement, "syntax/typedef" );
		if ( lpoCurrElement != NULL )
		{
			McbXMLAttribute * loAttrType = McbFindAttribute( lpoCurrElement, "basetype" );
			if ( loAttrType == NULL ) return NULL;
			return loAttrType->cpcValue;
		}
		
		lpoCurrElement = McbFindElement( apoElement, "syntax/type" );
		if ( lpoCurrElement != NULL )
		{
			McbXMLAttribute * loAttrType = McbFindAttribute( lpoCurrElement, "name" );
			if ( loAttrType == NULL ) return NULL;
			return loAttrType->cpcValue;
		}
		
		throw clsException( Format( "Can't find `syntax/typedef` or `syntax/type` for element %s", apoElement->cpcName ) );
		
		return NULL;
	}

	char* EraseInvalidChar( const char* apsName )
	{
		static char slsBuff[4096];
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

protected:
	McbXMLResults  coStat;
	McbXMLElement* cpoRootElement;
};

#endif // __XMLPARSE_H__
