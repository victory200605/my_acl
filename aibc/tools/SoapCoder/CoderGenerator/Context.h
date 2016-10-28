//=============================================================================
/**
 * \file    Context.h
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: Context.h,v 1.2 2009/03/09 02:34:56 daizh Exp $
 *
 * History
 * 2008.01.29 first release by DZH
 */
//=============================================================================

#ifndef __AI_CONTEXT_H__
#define __AI_CONTEXT_H__

#include "../../LuaPlus/LuaPlus.h"
using namespace LuaPlus;

#include <assert.h>
#include <algorithm>
#include <map>

class clsContext
{
public:
    typedef std::map<std::string, std::string>::iterator TNatureTypeIter;
    
public:
    virtual void LoadFormFile( const char* apsConfigFile )
    {
        if ( coState->LoadFile( apsConfigFile ) || coState->PCall(0,0,0) )
        {
             throw clsException( lua_tostring( coState->GetCState(), -1 ) );
        }

        this->LoadTypeMap( );
    }
    
    void SetVariable( const char* apsName, const char* apsValue )
    {
        coState->GetGlobals().SetString( apsName, apsValue );
    }
    
    void SetNamespace( const char* apcKey, const char* apcAlias, const char* apcValue )
    {
        LuaObject loNamespace = coState->GetGlobal( "XML_NAMESPACE" );
		
		assert( loNamespace.IsTable() );

		if ( !loNamespace.GetByName( apcKey ).IsTable() )
		{
			LuaObject loTmp;
			loTmp.AssignNewTable(coState);
			loTmp.SetString( "alias", apcAlias );
			loTmp.SetString( "value", apcValue );
        
			loNamespace.SetObject( apcKey, loTmp );
		}
    }
     
    void SetNatureType( const char* apsWSDLType, const char* apsNatureType )
    {
        cmNature[apsWSDLType] = apsNatureType;
    }
    
    const char* LookUpNatureType( const char* apsWSDLType )
    {
        std::map<std::string, std::string>::iterator liIt = cmNature.find( apsWSDLType );
        if ( liIt != cmNature.end() )
        {
             return liIt->second.c_str();
        }
        return NULL;
    }
    
    TNatureTypeIter BeginNatureType()
    {
        return cmNature.begin();
    }
    
    TNatureTypeIter EndNatureType()
    {
        return cmNature.end();
    }

	template< class T >
	std::string CallFunc( const char* apsFuncName, T& aoParam )
	{
		std::string loRet;

		if ( !coState->GetGlobal(apsFuncName).IsFunction() ) return loRet;

		LuaFunction<const char*> lfNode = coState->GetGlobal(apsFuncName);
		loRet = lfNode(aoParam);

		return loRet;
	}
	
	std::string GetOutputFileName()
	{
	    LuaObject loLuaString = coState->GetGlobal("OUTPUT_FILENAME");
	    
	    if ( loLuaString.IsString() )
	    {
	        return loLuaString.ToString();
	    }
	    else
	    {
	        return "";
	    }
	}
	
	std::string GetOutputDirectory()
	{
	    LuaObject loLuaString = coState->GetGlobal("OUTPUT_DIRECTORY");
	    
	    if ( loLuaString.IsString() )
	    {
	        return loLuaString.ToString();
	    }
	    else
	    {
	        return "";
	    }
	}

	LuaStateOwner& GetLuaState( )              { return coState; }

protected:
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

			cmNature[ loSubTable.GetByIndex(1).GetString() ] = loSubTable.GetByIndex(2).GetString();
		}
		return true;
	}

protected:
    std::map<std::string, std::string> cmNature;
    std::map<std::string, std::string> cmNamespace;
    
    LuaStateOwner coState;
};

#endif //__AI_CONTEXT_H__
