//=============================================================================
/**
 * \file    Message.h
 * \brief
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: Message.h,v 1.2 2009/03/09 02:34:56 daizh Exp $
 *
 * History
 * 2008.01.27 first release by DZH
 */
//=============================================================================
#ifndef __AI_MESSAGE_H__
#define __AI_MESSAGE_H__

#include "Singleton.h"
#include "Types.h"
#include <vector>
#include <string>
#include <map>

class clsMessage : public Singleton<clsMessage>
{ 
public:
    // Message part type
    struct stPart
    {
        stPart() : cpoElement(NULL)
        {}
        stPart( const char* apsName, const char* apsNamespace, const char* apsElementName, clsElement* apoElement ) :
            coStrName(apsName),
            coStrNamespace(apsNamespace),
            coStrElement(apsElementName),
            cpoElement(apoElement)
        {}
        
        // Attribute
        std::string coStrName;
        std::string coStrNamespace;
        std::string coStrElement;
        clsElement* cpoElement;
    };
    
    struct stMessageNode
    {
        typedef  std::vector<stPart>::iterator iterator;
        iterator Begin()                                { return cvPart.begin(); }
        iterator End()                                  { return cvPart.end(); }
        
        std::string         coStrName;
        std::vector<stPart> cvPart;
    };

public:
    typedef std::map<std::string, stMessageNode>::iterator iterator;
       
public:
    stMessageNode* AddMessage( const char* apsName )
    {
        if ( cmMessages.find(apsName) != cmMessages.end() ) return NULL;
        
        stMessageNode* lpoMessage = &(cmMessages[apsName]);
        
        lpoMessage->coStrName = apsName;
        
        return lpoMessage;
    }
    
    stMessageNode* Find( const char* apsName )
    {
        iterator loIt;
        if ( ( loIt = cmMessages.find(apsName) ) == cmMessages.end() ) 
        {
            return NULL;
        }
        else
        {
            return &(loIt->second);
        }
    }
    stPart* Find( stMessageNode* apoMessageNode, const char* apsPart )
    {
		for( std::vector<stPart>::iterator loIt = apoMessageNode->cvPart.begin();
             loIt != apoMessageNode->cvPart.end(); loIt++ )
        {
            if ( loIt->coStrName == apsPart ) return &(*loIt);
        }
        return NULL;
    }
    
    void AddPart( stMessageNode* apoMessageNode, const char* apsName, const char* apsNamespace, const char* apsElement )
    {
        clsElement* lpoElement = clsTypes::Instance()->Find( apsElement );
        if ( lpoElement == NULL )
        {
            throw clsException( clsException::eError, Format("[message]:%s undefined.", apsElement) );
        }
        
        apoMessageNode->cvPart.push_back( stPart(apsName, apsNamespace, apsElement, lpoElement) );
    }
    
    iterator Begin()
    {
        return cmMessages.begin();
    }
    
    iterator End()
    {
        return cmMessages.end();
    }
    
protected:
    std::map<std::string, stMessageNode> cmMessages;
};

typedef clsMessage::stMessageNode TMessageNode;
typedef clsMessage::stPart        TMessagePart;
#endif

