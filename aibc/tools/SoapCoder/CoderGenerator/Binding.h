//=============================================================================
/**
 * \file    PortType.h
 * \brief
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: Binding.h,v 1.4 2009/03/09 07:23:29 daizh Exp $
 *
 * History
 * 2008.01.27 first release by DZH
 */
//=============================================================================
#ifndef __AI_BINDING_H__
#define __AI_BINDING_H__

#include "PortType.h"

class clsBinding : public Singleton<clsBinding>
{   
public:
    // Bingding operation type
    struct stFieldDesc
    {
        // Define for parameter pass will be convert to stField
        std::string   coStrMessage;
        std::vector<std::string> cvStrPart;
        std::string   coStrUse;
        std::string   coStrEncodingStyle;
        std::string   coStrNamespace;
    };
    
    struct stEntryDesc
    {
		std::vector<stFieldDesc> cvBody;
        std::vector<stFieldDesc> cvHeader;
    };
    
    // 
    struct stField
    {
        typedef std::vector<TMessagePart*>::iterator iterator;
        iterator Begin()                                { return cvpMessagePart.begin(); }
        iterator End()                                  { return cvpMessagePart.end(); }
        
        TMessageNode* cpoMessageNode;
        std::vector<TMessagePart*> cvpMessagePart;
        std::string   coStrUse;
        std::string   coStrEncodingStyle;
        std::string   coStrNamespace;
    };
    
    struct stEntry
    {
        typedef std::vector<stField>::iterator iterator;
        iterator BeginH()                                { return cvHeader.begin(); }
        iterator EndH()                                  { return cvHeader.end(); }
		iterator BeginB()                                { return cvBody.begin(); }
		iterator EndB()                                  { return cvBody.end(); }
        
		std::vector<stField> cvBody;
        std::vector<stField> cvHeader;
    };
    
    struct stOperation
    {
        stOperation() : cpoPortTypeOperation(NULL)
        {}
        
        // Attribute
        TPortTypeOperation* cpoPortTypeOperation;
        stEntry    coInput;
        stEntry    coOutput;
    };
    
    struct stBindingNode
    {
        typedef std::map<std::string, stOperation>::iterator iterator;
        iterator Begin()                                { return cmOperation.begin(); }
        iterator End()                                  { return cmOperation.end(); }
        
        std::string   coStrName;
        TPortTypeNode*    cpoPortType;
        std::map< std::string, stOperation > cmOperation;
    };
public:
    typedef std::map<std::string, stBindingNode>::iterator iterator;
     
public:
    stBindingNode* AddBinding( const char* apsName, const char* apsType )
    {
        if ( cmBindings.find(apsName) != cmBindings.end() )
        {
            throw clsException( clsException::eError, Format("[Binding]:binding name %s redefined.", apsName) ); 
        }
        
        TPortTypeNode* lpoPortType = clsPortType::Instance()->Find( apsType );
        if ( lpoPortType == NULL )
        {
            throw clsException( clsException::eError, Format("[Binding]:portType %s undefined.", apsType) ); 
        }
        
        stBindingNode* lpoBinding = &(cmBindings[apsName]);
        
        lpoBinding->coStrName   = apsName;
        lpoBinding->cpoPortType = lpoPortType;
        
        return lpoBinding;
    }
    
    stBindingNode* Find( const char* apsName )
    {
        iterator loIt;
        if ( ( loIt = cmBindings.find(apsName) ) == cmBindings.end() ) 
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
        if ( ( loIt = cmBindings.find(apsName) ) == cmBindings.end() ) 
        {
            return NULL;
        }
        
        stBindingNode::iterator loOptIter;
        if ( ( loOptIter = loIt->second.cmOperation.find(apsOptName) ) == loIt->second.cmOperation.end() )
        {
            return NULL;
        }
        
        return &(loOptIter->second);
    }
    
    void AddOperation( stBindingNode* apoBinding, const char* apsName, stEntryDesc& aoInput, stEntryDesc& aoOutput )
    {
        TPortTypeOperation* lpoOperation = clsPortType::Instance()->Find( apoBinding->cpoPortType, apsName);
        if ( lpoOperation == NULL )
        {
            throw clsException( clsException::eError, Format("[Binding]:portType operation %s undefined.", apsName) );
        }
        
        stOperation loOpt;
        loOpt.cpoPortTypeOperation = lpoOperation;
        
        // Do Input
		loOpt.coInput.cvBody.resize( aoInput.cvBody.size() );
		for ( size_t liIt = 0; liIt < aoInput.cvBody.size(); liIt++ )
		{
			this->LinkEntryField( lpoOperation, aoInput.cvBody[liIt], loOpt.coInput.cvBody[liIt], true );
		}
        
        loOpt.coInput.cvHeader.resize( aoInput.cvHeader.size() );
        for ( size_t liIt = 0; liIt < aoInput.cvHeader.size(); liIt++ )
        {
            this->LinkEntryField( lpoOperation, aoInput.cvHeader[liIt], loOpt.coInput.cvHeader[liIt], true );
        }
        
        // Do Output
		loOpt.coOutput.cvBody.resize( aoOutput.cvBody.size() );
		for ( size_t liIt = 0; liIt < aoOutput.cvBody.size(); liIt++ )
		{
			this->LinkEntryField( lpoOperation, aoOutput.cvBody[liIt], loOpt.coOutput.cvBody[liIt], false );
		}
        
        
        loOpt.coOutput.cvHeader.resize( aoOutput.cvHeader.size() );
        for ( size_t liIt = 0; liIt < aoOutput.cvHeader.size(); liIt++ )
        {
            this->LinkEntryField( lpoOperation, aoOutput.cvHeader[liIt], loOpt.coOutput.cvHeader[liIt], false );
        }
        
        apoBinding->cmOperation[apsName] = loOpt;
    }
    
    iterator Begin()
    {
        return cmBindings.begin();
    }
    
    iterator End()
    {
        return cmBindings.end();
    }

protected:
    void LinkEntryField( TPortTypeOperation* apoOperation, stFieldDesc& aoFieldDesc, stField& aoField, bool abIsInput )
    {
        TMessageNode* lpoMessageNode = NULL;
        TMessagePart* lpoMessagePart = NULL;
        // Do input
        if ( aoFieldDesc.coStrMessage.length() > 0 )
        {
            // Point to message
            lpoMessageNode = clsMessage::Instance()->Find(aoFieldDesc.coStrMessage.c_str());
        }
        else
        {
            // Default message
            if ( abIsInput )
            {
                lpoMessageNode = apoOperation->cpoInputMessage;
            }
            else
            {
                lpoMessageNode = apoOperation->cpoOutputMessage;
            }
        }
        
        aoField.cpoMessageNode = lpoMessageNode;
        
        // Message part link
        for ( size_t liIdx = 0; liIdx < aoFieldDesc.cvStrPart.size(); liIdx++ )
        {
            lpoMessagePart = clsMessage::Instance()->Find( lpoMessageNode, aoFieldDesc.cvStrPart[liIdx].c_str() );
            
            if ( lpoMessagePart == NULL )
            {
                throw clsException( clsException::eError, Format("[Binding]:message part %s undefined.", aoFieldDesc.cvStrPart[liIdx].c_str()) );
            }
            aoField.cvpMessagePart.push_back(lpoMessagePart);
        }
        if ( lpoMessagePart == NULL )
        {
            for ( size_t liN = 0; liN < lpoMessageNode->cvPart.size(); liN++ )
            {
                lpoMessagePart = &(lpoMessageNode->cvPart[liN]);
                aoField.cvpMessagePart.push_back(lpoMessagePart);
            }
        }
        //else if ( lpoMessagePart == NULL )
        //{
        //    throw clsException( clsException::eError, Format("[Binding]:message %s part incertitude.", lpoMessageNode->coStrName.c_str() ) );
        //}
        
        aoField.coStrUse           = aoFieldDesc.coStrUse;
        aoField.coStrEncodingStyle = aoFieldDesc.coStrEncodingStyle;
        aoField.coStrNamespace     = aoFieldDesc.coStrNamespace;
    }
    
protected:
    std::map<std::string, stBindingNode> cmBindings;
};

typedef clsBinding::stBindingNode  TBindingNode;
typedef clsBinding::stOperation    TBindingOperation;

#endif //__AI_PORTTYPE_H__
