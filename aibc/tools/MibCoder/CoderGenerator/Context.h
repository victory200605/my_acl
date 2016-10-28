//=============================================================================
/**
* \file    Context.h
* \brief 
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: Context.h,v 1.2 2009/02/23 03:43:46 daizh Exp $
*
* History
* 2008.01.29 first release by DZH
*/
//=============================================================================

#ifndef __AI_CONTEXT_H__
#define __AI_CONTEXT_H__

#include "../../LuaPlus/LuaPlus.h"
using namespace LuaPlus;

#include "Exception.h"

#include <assert.h>
#include <algorithm>
#include <map>

class clsContext
{
public:
	// variable name suffix or perfix format define
	struct stNameFix
	{
		stNameFix() {}
		stNameFix( const char* apsName, const char* apsTypePrefix,  const char* apsVarPrefix ):
		coStrName( apsName ),
			coStrTypePrefix( apsTypePrefix ),
			coStrVarPrefix( apsVarPrefix )
		{}
		std::string coStrName;
		std::string coStrTypePrefix;
		std::string coStrVarPrefix;
	};

	// style 
	struct stStyle 
	{
		typedef std::map<std::string, stNameFix>::iterator TNameFixIter;

		stStyle(){}

		// name fix map
		std::map<std::string, stNameFix> cmNameFix;
	};

public:
	virtual void LoadFormFile( const char* apsConfigFile )
	{
		if ( coState->LoadFile( apsConfigFile ) || coState->PCall(0,0,0) )
		{
			throw clsException( lua_tostring( coState->GetCState(), -1 ) );
		}
	}

	void SetVariable( const char* apsName, const char* apsValue )
	{
		coState->GetGlobals().SetString( apsName, apsValue );
	}
	
	int LookUpNatureType( const char* apsWSDLType )
	{
		std::map<std::string, int>::iterator liIt = cmNature.find( Toupper( apsWSDLType ).c_str() );
		if ( liIt != cmNature.end() )
		{
			return liIt->second;
		}
		return -1;
	}

	std::string FormatName( const char* apsPrefix, const char* apsName, const char* apsSuffix = NULL )
	{
		return FormatNameVC( apsPrefix, apsName, apsSuffix );
	}

	std::string GetTypeNamePrefix( const char* apsName ) 
	{
		stStyle::TNameFixIter liIt = cstStyle.cmNameFix.find( Toupper( apsName ).c_str() );
		if ( liIt != cstStyle.cmNameFix.end() )
		{
			return liIt->second.coStrTypePrefix;
		}
		return std::string("");
	}

	std::string GetVarNamePrefix( const char* apsName ) 
	{
		stStyle::TNameFixIter liIt = cstStyle.cmNameFix.find( Toupper( apsName ).c_str() );
		if ( liIt != cstStyle.cmNameFix.end() )
		{
			return liIt->second.coStrVarPrefix;
		}
		return std::string("");
	}

	bool GetOutputFileName( std::string& aoValue, const char* apsDefaultDir )
	{
		std::string loStrFileName;
		bool lbRet = true;
		lbRet = this->GetString( "OUTPUT_FILENAME", loStrFileName ); 
		if ( !lbRet ) return false;
		aoValue = apsDefaultDir;
		aoValue += loStrFileName.c_str();
		return true;
	}

	bool GetFileNote( std::string& coValue ) 
	{
		return this->GetString( "FILE_NOTE", coValue ); 
	}

	bool GetIncludeFile( std::string& coValue )  
	{ 
		return this->GetString( "INCLUDE_FILE", coValue ); 
	}

	bool GetIncludeMacro( std::string& coValue )
	{
		return this->GetString( "INCLUDE_MACRO_DEFINE", coValue ); 
	}

	bool GetFuncDeclare( const char* apcName, const char* apcOid, const char* apcTypeName, std::string& coValue )
	{
		if ( !coState->GetGlobal("FUNC_DECLARE").IsFunction() ) return false;

		LuaFunction<const char*> lfNode = coState->GetGlobal("FUNC_DECLARE");
		coValue = lfNode( apcName, apcOid, apcTypeName );
		return true;
	}
	
	bool GetInitFuncDeclare( std::string& coValue )
	{
		if ( !coState->GetGlobal("FUNC_INITIALIZE_DECLARE").IsFunction() ) return false;

		LuaFunction<const char*> lfNode = coState->GetGlobal("FUNC_INITIALIZE_DECLARE");
		coValue = lfNode();
		return true;
	}


	LuaStateOwner& GetLuaState( )              { return coState; }

protected:
	std::string Toupper( const char* apsString )
	{
		std::string loString(apsString);
		transform( loString.begin(), loString.end(), loString.begin(), toupper );
		return loString;
	}

	bool GetString( const char* apsName, std::string& aoStrValue )
	{
		if ( !coState->GetGlobal( apsName ).IsString() ) return false;

		aoStrValue = coState->GetGlobal( apsName ).ToString();
		return true;
	}

	bool GetNumber( const char* apsName, double& adValue )
	{
		if ( !coState->GetGlobal( apsName ).IsNumber() ) return false;

		adValue = coState->GetGlobal( apsName ).ToNumber();
		return true;
	}

	static std::string FormatNameVC( const char* apsPrefix, const char* apsName, const char* apsSuffix = NULL )
	{
		assert( apsName != NULL );

		std::string loStrBuff; 

		size_t liLen = strlen(apsName);
		loStrBuff = toupper(apsName[0]);
		bool lbIsUnderline = false;
		while ( *apsName++ != '\0')
		{
			if ( *apsName == '_' )
			{
				lbIsUnderline = true;
				continue;
			}
			if ( lbIsUnderline )
			{
				loStrBuff += toupper( *apsName );
				lbIsUnderline = false;
			}
			else loStrBuff += *apsName;
		}

		if ( apsPrefix != NULL ) loStrBuff = apsPrefix + loStrBuff;
		if ( apsSuffix != NULL ) loStrBuff += apsSuffix;
		return loStrBuff;
	}

protected:
	bool LoadStyle( )
	{
		/* 读取名字风格 */
		LuaObject loTable = coState->GetGlobals()["NAME_PREFIX"];
		if ( !loTable.IsTable() )
		{
			return false;
		}
		for ( LuaTableIterator liIt( loTable ); liIt; liIt.Next() )
		{
			LuaObject loSubTable = liIt.GetValue();
			cstStyle.cmNameFix[ loSubTable.GetByIndex(1).GetString() ] = stNameFix( 
				loSubTable.GetByIndex(1).GetString(), 
				loSubTable.GetByIndex(2).GetString(),
				loSubTable.GetByIndex(3).GetString() );

		}

		return true;
	}
	bool LoadTypeMap()
	{
		/* 读取型别映射配置表 */
		LuaObject loTable = coState->GetGlobals()["TYPE_MAP"];
		if ( !loTable.IsTable() )
		{
			return false;
		}

		for ( LuaTableIterator liIt( loTable ); liIt; liIt.Next() )
		{
			LuaObject loSubTable = liIt.GetValue();

			cmNature[ Toupper( loSubTable.GetByIndex(1).GetString() ) ] = loSubTable.GetByIndex(2).GetInteger();
		}
		return true;
	}

protected:
	std::map<std::string, int> cmNature;

	LuaStateOwner coState;

	stStyle cstStyle;
};

#endif //__AI_CONTEXT_H__
