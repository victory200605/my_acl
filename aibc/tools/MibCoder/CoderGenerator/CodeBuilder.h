
#ifndef __CODE_BUILDER_H__
#define __CODE_BUILDER_H__

#include "ObjectType.h"

class clsHCodeBuilder
{
public:
	clsHCodeBuilder(){}

	const char* Build( const TObjectList& aoObjectList, clsContext& aoContext )
	{
		// Gen initialize function
		std::string loStrTmp;
		std::string loStrMacro;

		aoContext.GetFileNote( loStrTmp );
		coStrBuffer += loStrTmp;

		// include macro
		if ( aoContext.GetIncludeMacro(loStrTmp) )
		{
			coStrBuffer += ::Format( "#ifndef %s  \n#define %s \n", loStrTmp.c_str(), loStrTmp.c_str() );
			loStrMacro  = ::Format( "#endif //%s \n", loStrTmp.c_str() );
		}

		// include file
		if ( aoContext.GetIncludeFile( loStrTmp ) )
		{
			coStrBuffer += "\n";
			coStrBuffer += loStrTmp;
		}

		for( TObjectList::const_iterator liIt = aoObjectList.begin(); liIt != aoObjectList.end(); liIt++ )
		{
			aoContext.GetFuncDeclare( (*liIt)->GetName(), (*liIt)->GetOid(), (*liIt)->GetTypeName(), loStrTmp );
			coStrBuffer += loStrTmp;
		}

		// Initialize function declare
		aoContext.GetInitFuncDeclare( loStrTmp );
		coStrBuffer += loStrTmp;

		coStrBuffer += ::Format( "\n%s\n", loStrMacro.c_str() );;

		return coStrBuffer.c_str();
	}

protected:
	std::string coStrBuffer;
};

class clsCppCodeBuilder
{
public:
	clsCppCodeBuilder(){}

	const char* Build( const TObjectList& aoObjectList, clsCppContext& aoContext )
	{
		// Gen initialize function
		std::string loStrTmp;
		std::string loStrRegister;
		std::string loStrDefine;
		std::string loStrBody;

		aoContext.GetFileNote( loStrTmp );
		coStrBuffer += loStrTmp;

		// include file
		if ( aoContext.GetIncludeFile( loStrTmp ) )
		{
			coStrBuffer += "\n";
			coStrBuffer += loStrTmp;
		}

		for( TObjectList::const_iterator liIt = aoObjectList.begin(); liIt != aoObjectList.end(); liIt++ )
		{
			if ( (*liIt)->GetType() == clsObjectType::SIMPLE )
			{
				aoContext.GetObjectType( (*liIt)->GetName(), (*liIt)->GetOid(), (*liIt)->GetTypeName(), loStrTmp );
			}
			else
			{
				std::string loRow;
				for ( clsTable::iterator liCol = ((clsTable*)(*liIt))->Begin();
					liCol != ((clsTable*)(*liIt))->End(); liCol++ )
				{
					aoContext.GetColumnType( (*liCol)->GetName(), (*liCol)->GetOid(), (*liCol)->GetTypeName(), loStrTmp );
					loRow += loStrTmp;
				}
				aoContext.GetTableType( (*liIt)->GetName(), (*liIt)->GetOid(), (*liIt)->GetTypeName(), loRow.c_str(), loStrTmp );
			}
			loStrBody += loStrTmp;

			aoContext.GetRegisterObject( (*liIt)->GetName(), (*liIt)->GetOid(), (*liIt)->GetTypeName(), loStrTmp );
			loStrRegister += loStrTmp;

			aoContext.GetDefineObject( (*liIt)->GetName(), (*liIt)->GetOid(), (*liIt)->GetTypeName(), loStrTmp );
			loStrDefine += loStrTmp;
		}

		aoContext.GetInitialize( loStrRegister.c_str(), loStrTmp );
		coStrBuffer += loStrDefine + loStrTmp + loStrBody;

		return coStrBuffer.c_str();
	}

protected:
	std::string coStrBuffer;
};

#endif //__CODE_BUILDER_H__
