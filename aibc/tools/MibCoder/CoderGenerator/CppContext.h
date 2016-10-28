//=============================================================================
/**
* \file    CppContext.h
* \brief 
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: CppContext.h,v 1.1 2009/02/23 03:41:40 daizh Exp $
*
* History
* 2008.01.29 first release by DZH
*/
//=============================================================================

#ifndef __AI_CPPCONTEXT_H__
#define __AI_CPPCONTEXT_H__

#include "Context.h"

class clsCppContext : public clsContext
{
public:
	virtual void LoadFormFile( const char* apsConfigFile )
	{
		if ( coState->LoadFile( apsConfigFile ) || coState->PCall(0,0,0) )
		{
			throw clsException( lua_tostring( coState->GetCState(), -1 ) );
		}
	}
	bool GetObjectType( const char* apcName, const char* apcOid, const char* apcTypeName, std::string& aoStrValue )
	{
		std::string loStrFuncName = "FUNC_OBJECT_TEMPLATE";

		if ( !coState->GetGlobal( loStrFuncName.c_str()).IsFunction() ) return false;

		LuaFunction<const char*> lfHeader = coState->GetGlobal(loStrFuncName.c_str());
		aoStrValue = lfHeader( apcName, apcOid, apcTypeName );
		return true;
	}
	bool GetTableType( const char* apcName, const char* apcOid, const char* apcTypeName, const char* apcRow, std::string& aoStrValue )
	{
		if ( !coState->GetGlobal("FUNC_OBJECT_TEMPLATE_TABLE").IsFunction() ) return false;

		LuaFunction<const char*> lfHeader = coState->GetGlobal("FUNC_OBJECT_TEMPLATE_TABLE");
		aoStrValue = lfHeader( apcName, apcOid, apcTypeName, apcRow );
		return true;
	}
	bool GetColumnType( const char* apcName, const char* apcOid, const char* apcTypeName, std::string& aoStrValue )
	{
		if ( !coState->GetGlobal("FUNC_OBJECT_TEMPLATE_COLUMN").IsFunction() ) return false;

		LuaFunction<const char*> lfHeader = coState->GetGlobal("FUNC_OBJECT_TEMPLATE_COLUMN");
		aoStrValue = lfHeader( apcName, apcOid, apcTypeName );
		return true;
	}
	bool GetDefineObject( const char* apcName, const char* apcOid, const char* apcTypeName, std::string& aoStrValue )
	{
		if ( !coState->GetGlobal("FUNC_DEFINE_OBJECT_TEMPLATE").IsFunction() ) return false;

		LuaFunction<const char*> lfHeader = coState->GetGlobal("FUNC_DEFINE_OBJECT_TEMPLATE");
		aoStrValue = lfHeader( apcName, apcOid, apcTypeName );
		return true;
	}
	bool GetRegisterObject( const char* apcName, const char* apcOid, const char* apcTypeName, std::string& aoStrValue )
	{
		if ( !coState->GetGlobal("FUNC_REG_OBJECT_TEMPLATE").IsFunction() ) return false;

		LuaFunction<const char*> lfHeader = coState->GetGlobal("FUNC_REG_OBJECT_TEMPLATE");
		aoStrValue = lfHeader( apcName, apcOid, apcTypeName );
		return true;
	}
	bool GetInitialize( const char* apcRegCoder, std::string& aoStrValue )
	{
		if ( !coState->GetGlobal("FUNC_INITIALIZ_TEMPLATE").IsFunction() ) return false;

		LuaFunction<const char*> lfHeader = coState->GetGlobal("FUNC_INITIALIZ_TEMPLATE");
		aoStrValue = lfHeader( apcRegCoder );
		return true;
	}
};

#endif //__AI_CONTEXT_H__
