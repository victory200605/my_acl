//=============================================================================
/**
 * \file    PortType.h
 * \brief
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: PortType.h,v 1.3 2009/03/09 06:53:32 daizh Exp $
 *
 * History
 * 2008.01.27 first release by DZH
 */
//=============================================================================
#ifndef __AI_PORTTYPE_H__
#define __AI_PORTTYPE_H__

#include "Message.h"

class clsPortType : public Singleton<clsPortType>
{   
public:
    // PortType operation type
    struct stOperation
    {
        stOperation() : cpoInputMessage(NULL),cpoOutputMessage(NULL)
        {}
        stOperation( const char* apsName, const char* apsInput, TMessageNode* apoInputMsg, 
            const char* apsOutput, TMessageNode* apoOutputMsg ) :
			coStrName(apsName),
            coStrInput(apsInput),
            cpoInputMessage(apoInputMsg),
            coStrOutput(apsOutput),
            cpoOutputMessage(apoOutputMsg)
        {}
        
        // Attribute
        std::string   coStrName;
        std::string   coStrInput;
        TMessageNode* cpoInputMessage; 
        std::string   coStrOutput;
        TMessageNode* cpoOutputMessage;
    };
    
    struct stPortTypeNode
    {
        typedef std::map<std::string, stOperation>::iterator iterator;
        iterator Begin()                                { return cmOperation.begin(); }
        iterator End()                                  { return cmOperation.end(); }
        
        std::string   coStrName;
        std::map< std::string, stOperation > cmOperation;
    };
public:
    typedef std::map<std::string, stPortTypeNode>::iterator iterator;
     
public:
    stPortTypeNode* AddPortType( const char* apsName )
    {
        if ( cmPortTypes.find(apsName) != cmPortTypes.end() ) return NULL;
        
        stPortTypeNode* lpoPortType = &(cmPortTypes[apsName]);
        
        lpoPortType->coStrName = apsName;
        
        return lpoPortType;
    }
    
    stPortTypeNode* Find( const char* apsName )
    {
        iterator loIt;
        if ( ( loIt = cmPortTypes.find(apsName) ) == cmPortTypes.end() ) 
        {
            return NULL;
        }
        else
        {
            return &(loIt->second);
        }
    }
    stOperation* Find( const char* apsName, const char* apsOptName )
    {
        iterator loIt;
        if ( ( loIt = cmPortTypes.find(apsName) ) == cmPortTypes.end() ) 
        {
            return NULL;
        }
        
        stPortTypeNode::iterator loOptIter;
        if ( ( loOptIter = loIt->second.cmOperation.find(apsOptName) ) == loIt->second.cmOperation.end() )
        {
            return NULL;
        }
        
        return &(loOptIter->second);
    }
    stOperation* Find( stPortTypeNode* apoPortTypeNode, const char* apsOptName )
    {
        stPortTypeNode::iterator loOptIter;
        if ( ( loOptIter = apoPortTypeNode->cmOperation.find(apsOptName) ) == apoPortTypeNode->cmOperation.end() )
        {
            return NULL;
        }
        
        return &(loOptIter->second);
    }
    
    void AddOperation( stPortTypeNode* apoPortTypeNode, const char* apsName, const char* apsInput, const char* apsOutput )
    {
		TMessageNode* lpoInputMessage  = NULL;
		TMessageNode* lpoOutputMessage = NULL;

		if ( strlen(apsInput) > 1 )
		{
			lpoInputMessage = clsMessage::Instance()->Find( apsInput );
			if ( lpoInputMessage == NULL )
			{
				throw clsException( clsException::eError, Format("[portType]:%s undefined.", apsInput) );
			}
		}
        
		if ( strlen(apsOutput) > 1 )
		{
			lpoOutputMessage = clsMessage::Instance()->Find( apsOutput );
			if ( lpoOutputMessage == NULL )
			{
				throw clsException( clsException::eError, Format("[portType]:%s undefined.", apsOutput) );
			}
		}
        
        apoPortTypeNode->cmOperation[apsName] = stOperation( apsName, apsInput, lpoInputMessage,
            apsOutput, lpoOutputMessage );
    }
    
    iterator Begin()
    {
        cmPortTypes.begin();
    }
    
    iterator End()
    {
        cmPortTypes.end();
    }
    
protected:
    std::map<std::string, stPortTypeNode> cmPortTypes;
};

typedef clsPortType::stPortTypeNode TPortTypeNode;
typedef clsPortType::stOperation    TPortTypeOperation;

#endif //__AI_PORTTYPE_H__
