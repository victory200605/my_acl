//=============================================================================
/**
 * \file    CodeBuilder.h
 * \brief
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: CodeBuilder.h,v 1.4 2009/03/10 11:27:53 daizh Exp $
 *
 * History
 * 2008.01.27 first release by DZH
 */
//=============================================================================
#ifndef __AI_CODEBUILDER_H__
#define __AI_CODEBUILDER_H__

#include "Context.h"
#include "Service.h"

class clsCodeBuilder
{
public:
	clsCodeBuilder( clsContext& aoContext ): coContext( aoContext )
	{}

	virtual ~clsCodeBuilder( )
	{}
	
	clsContext* GetContext()
	{
	    return &coContext;
	}

	virtual const char* BuildHCoder( )
	{
		// header
		coStrResult = GetContext()->CallFunc( "__H__Header", "" );

		coStrResult += this->BuildComplexTypes( "__H__ComplexType" );
		
		coStrResult += this->BuildElementTypes( "__H__Element" );

		coStrResult += this->BindingMessage( "__H__Message" );
		
		coStrResult += this->BuildOperation( "__H__PortType" );

		coStrResult += GetContext()->CallFunc( "__H__Tail", "" );

		return coStrResult.c_str();
	}
	
	virtual const char* BuildCppCoder( )
	{
		// header
		coStrResult = GetContext()->CallFunc( "__CPP__Header", "" );

		coStrResult += this->BuildComplexTypes( "__CPP__ComplexType" );
		
		coStrResult += this->BuildElementTypes( "__CPP__Element" );

		coStrResult += this->BindingMessage( "__CPP__Message" );
		
		coStrResult += this->BuildOperation( "__CPP__PortType" );

		coStrResult += GetContext()->CallFunc( "__CPP__Tail", "" );

		return coStrResult.c_str();
	}

protected:
	std::string BuildComplexTypes( const char* apcFuncName )
	{
		std::string loStrCode;

		for ( clsTypes::iterator loIt = clsTypes::Instance()->Begin(); loIt != clsTypes::Instance()->End(); loIt++ )
		{
			if ( (*loIt)->GetType() == clsElement::eComplex )
			{
				loStrCode += this->GenComplexCode( apcFuncName, (clsComplexElement*)(*loIt) );
			}
		}
		
		return loStrCode;
	}
	
	std::string BuildElementTypes( const char* apcFuncName )
	{
		std::string loStrCode;
		    
		for ( clsTypes::iterator loIt = clsTypes::Instance()->Begin(); loIt != clsTypes::Instance()->End(); loIt++ )
		{
			if ( (*loIt)->GetType() == clsElement::eElement )
			{
				loStrCode += this->GenElementCode( apcFuncName, (clsSingleElement*)(*loIt) );
			}
		}

		return loStrCode;
	}

	std::string BindingMessage( const char* apcFuncName )
	{
		std::string loStrCode;

		for ( clsMessage::iterator loIt = clsMessage::Instance()->Begin(); 
			loIt != clsMessage::Instance()->End(); loIt++ )
		{
			loStrCode += this->GenMessage( apcFuncName, loIt->second );
		}

		return loStrCode;
	}

	std::string BuildOperation( const char* apcFuncName )
	{
		std::string loStrCode;
		std::vector<TBindingNode*> lvoBindingNodes;

		for ( clsService::iterator loIt = clsService::Instance()->Begin(); 
			loIt != clsService::Instance()->End(); loIt++ )
		{
			for ( TServiceNode::iterator loPortIter = loIt->second.Begin(); 
				loPortIter != loIt->second.End(); loPortIter++ )
			{
				lvoBindingNodes.push_back( loPortIter->second.cpoBindingNode );
			}
		}

		for ( std::vector<TBindingNode*>::iterator loIt = lvoBindingNodes.begin();
			loIt != lvoBindingNodes.end(); loIt++ )
		{
			for ( TBindingNode::iterator loOptIter = (*loIt)->Begin(); 
				loOptIter != (*loIt)->End(); loOptIter++ )
			{
				LuaObject loTable = GetContext()->GetLuaState()->GetGlobals().CreateTable( 
					loOptIter->second.cpoPortTypeOperation->coStrName.c_str() );

				loTable.SetString( "name", loOptIter->second.cpoPortTypeOperation->coStrName.c_str() );

				LuaObject loInput;
				loInput.AssignNewTable(GetContext()->GetLuaState());
				MakeProtTypeTable( loOptIter->second.coInput, loInput );
				loTable.SetObject( "input", loInput );

				LuaObject loOutput;
				loOutput.AssignNewTable(GetContext()->GetLuaState());
				MakeProtTypeTable( loOptIter->second.coOutput, loOutput );
				loTable.SetObject( "output", loOutput );

				loStrCode += GetContext()->CallFunc( apcFuncName, loTable );
			}
		}

		return loStrCode;
	}

	std::string GenComplexCode( const char* apcFuncName, clsComplexElement* apoElement )
	{
		LuaObject loTable = GetContext()->GetLuaState()->GetGlobals().CreateTable( apoElement->GetName() );

		loTable.SetString( "name", apoElement->GetName() );

		LuaObject loBody;
		int liIdx = 1;
		loBody.AssignNewTable(GetContext()->GetLuaState());
		for ( clsComplexElement::iterator loIter = apoElement->Begin(); loIter != apoElement->End(); loIter++ )
		{
			LuaObject loRow;
			loRow.AssignNewTable(GetContext()->GetLuaState());
			loRow.SetString( "type", (*loIter)->GetTypeName() );
			loRow.SetString( "typet", (*loIter)->GetTypet() );
			loRow.SetString( "ptype", (*loIter)->GetRootElement()->GetTypeName() );
			loRow.SetString( "ptypet", (*loIter)->GetRootElement()->GetTypet() );
			loRow.SetString( "name", (*loIter)->GetName() );
			loRow.SetString( "namespace", (*loIter)->GetNamespace() );
			loRow.SetInteger( "maxOccurs", (*loIter)->GetMaxOccurs() );
			loRow.SetInteger( "minOccurs", (*loIter)->GetMinOccurs() );

			loBody.SetObject( liIdx++, loRow );
		}

		loTable.SetObject( "body", loBody );

		return GetContext()->CallFunc( apcFuncName, loTable );
	}

	std::string GenElementCode( const char* apcFuncName, clsSingleElement* apoElement )
	{
		LuaObject loTable = GetContext()->GetLuaState()->GetGlobals().CreateTable( apoElement->GetName() );

		loTable.SetString( "name", apoElement->GetName() );
		loTable.SetString( "type", apoElement->GetTypeName() );
		loTable.SetString( "typet", apoElement->GetTypet() );
		loTable.SetString( "ptype", apoElement->GetRootElement()->GetTypeName() );
		loTable.SetString( "ptypet", apoElement->GetRootElement()->GetTypet() );
		loTable.SetString( "namespace", apoElement->GetNamespace() );
		loTable.SetInteger( "maxOccurs", apoElement->GetMaxOccurs() );
		loTable.SetInteger( "minOccurs", apoElement->GetMinOccurs() );

		return GetContext()->CallFunc( apcFuncName, loTable );
	}

	virtual std::string GenMessage( const char* apcFuncName, TMessageNode& aoMessage )
	{
		LuaObject loTable = GetContext()->GetLuaState()->GetGlobals().CreateTable( aoMessage.coStrName.c_str() );

		loTable.SetString( "name", aoMessage.coStrName.c_str() );

		//part
		int liPartIdx = 1;
		LuaObject loPart;
		loPart.AssignNewTable(GetContext()->GetLuaState());
		for ( TMessageNode::iterator loIt = aoMessage.Begin(); loIt != aoMessage.End(); loIt++ )
		{
			LuaObject loRow;
			loRow.AssignNewTable(GetContext()->GetLuaState());
			loRow.SetString( "type", loIt->coStrElement.c_str() );
			loRow.SetString( "typet", loIt->cpoElement->GetTypet() );
			loRow.SetString( "ptype", loIt->cpoElement->GetRootElement()->GetTypeName() );
			loRow.SetString( "ptypet", loIt->cpoElement->GetRootElement()->GetTypet() );
			loRow.SetString( "name", loIt->coStrName.c_str() );
			loRow.SetString( "namespace", loIt->coStrNamespace.c_str() );

			loPart.SetObject( liPartIdx++, loRow );
		}

		loTable.SetObject( "part", loPart );

		return GetContext()->CallFunc( apcFuncName, loTable );
	}

	void MakeProtTypeTable(  clsBinding::stEntry& aoEntry, LuaObject& aoTable )
	{
		// Header
		int liHeaderIdx = 1;
		LuaObject loHeader;
		loHeader.AssignNewTable(GetContext()->GetLuaState());
		for ( clsBinding::stEntry::iterator loFIt = aoEntry.BeginH(); loFIt != aoEntry.EndH(); loFIt++ )
		{
			for ( clsBinding::stField::iterator loIt = loFIt->Begin(); loIt != loFIt->End(); loIt++ )
			{
				LuaObject loRow;
				loRow.AssignNewTable(GetContext()->GetLuaState());
				loRow.SetString( "type", (*loIt)->coStrElement.c_str() );
				loRow.SetString( "ptype", (*loIt)->cpoElement->GetRootElement()->GetTypeName() );
				loRow.SetString( "ptypet", (*loIt)->cpoElement->GetRootElement()->GetTypet() );
				loRow.SetString( "typet", (*loIt)->cpoElement->GetTypet() );
				loRow.SetString( "name", (*loIt)->coStrName.c_str() );
				loRow.SetString( "namespace", (*loIt)->coStrNamespace.c_str() );
				loRow.SetString( "use", loFIt->coStrUse.c_str() );
				loRow.SetString( "encodingStyle", loFIt->coStrEncodingStyle.c_str() );
				loRow.SetString( "message", loFIt->cpoMessageNode->coStrName.c_str() );

				loHeader.SetObject( liHeaderIdx++, loRow );
			}
		}

		aoTable.SetObject( "header", loHeader );

		//body
		int liBodyIdx = 1;
		LuaObject loBody;
		loBody.AssignNewTable(GetContext()->GetLuaState());
		for ( clsBinding::stEntry::iterator loFIt = aoEntry.BeginB(); loFIt != aoEntry.EndB(); loFIt++ )
		{
			for ( clsBinding::stField::iterator loIt = loFIt->Begin(); loIt != loFIt->End(); loIt++ )
			{
				LuaObject loRow;
				loRow.AssignNewTable(GetContext()->GetLuaState());
				loRow.SetString( "type", (*loIt)->coStrElement.c_str() );
				loRow.SetString( "ptype", (*loIt)->cpoElement->GetRootElement()->GetTypeName() );
				loRow.SetString( "ptypet", (*loIt)->cpoElement->GetRootElement()->GetTypet() );
				loRow.SetString( "typet", (*loIt)->cpoElement->GetTypet() );
				loRow.SetString( "name", (*loIt)->coStrName.c_str() );
				loRow.SetString( "namespace", (*loIt)->coStrNamespace.c_str() );
				loRow.SetString( "use", loFIt->coStrUse.c_str() );
				loRow.SetString( "encodingStyle", loFIt->coStrEncodingStyle.c_str() );
				loRow.SetString( "message", loFIt->cpoMessageNode->coStrName.c_str() );

				loBody.SetObject( liBodyIdx++, loRow );
			}
		}

		aoTable.SetObject( "body", loBody );
	}

protected:
	std::string coStrResult;
	    
	clsContext& coContext;
};

#endif //__AI_CODEBUILDER_H__
