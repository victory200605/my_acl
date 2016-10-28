//=============================================================================
/**
 * \file    Service.h
 * \brief
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: Service.h,v 1.1 2009/02/23 03:48:19 daizh Exp $
 *
 * History
 * 2008.01.27 first release by DZH
 */
//=============================================================================
#ifndef __AI_SERVICE_H__
#define __AI_SERVICE_H__

#include "Binding.h"

class clsService : public Singleton<clsService>
{   
public:
    // Service Port type
    struct stPort
    {
        stPort() : cpoBindingNode(NULL)
        {}
        stPort( const char* apsName, TBindingNode* apsBinding ) : 
            coStrName(apsName), cpoBindingNode(apsBinding)
        {}
        
        // Attribute
        std::string   coStrName;
        TBindingNode* cpoBindingNode;
    };
    
    struct stServiceNode
    {
        typedef std::map<std::string, stPort>::iterator iterator;
        iterator Begin()                                { return cmPort.begin(); }
        iterator End()                                  { return cmPort.end(); }
        
        std::string   coStrName;
        std::map< std::string, stPort > cmPort;
    };
public:
    typedef std::map<std::string, stServiceNode>::iterator iterator;
     
public:
    stServiceNode* AddService( const char* apsName )
    {
        if ( cmServices.find(apsName) != cmServices.end() )
        {
            throw clsException( clsException::eError, Format("[Service]:Service name %s redefined.", apsName) ); 
        }

        stServiceNode* lpoService = &(cmServices[apsName]);
        
        lpoService->coStrName   = apsName;
        
        return lpoService;
    }
    
    stServiceNode* Find( const char* apsName )
    {
        iterator loIt;
        if ( ( loIt = cmServices.find(apsName) ) == cmServices.end() ) 
        {
            return NULL;
        }
        else
        {
            return &(loIt->second);
        }
    }
    stPort* Find( const char* apsName, const char* apsPortName )
    {
        iterator loIt;
        if ( ( loIt = cmServices.find(apsName) ) == cmServices.end() ) 
        {
            return NULL;
        }
        
        stServiceNode::iterator loOptIter;
        if ( ( loOptIter = loIt->second.cmPort.find(apsPortName) ) == loIt->second.cmPort.end() )
        {
            return NULL;
        }
        
        return &(loOptIter->second);
    }
    stPort* Find( stServiceNode* apsServiceNode, const char* apsPortName )
    {       
        stServiceNode::iterator loIter;
        if ( ( loIter = apsServiceNode->cmPort.find(apsPortName) ) == apsServiceNode->cmPort.end() )
        {
            return NULL;
        }
        
        return &(loIter->second);
    }
    
    void AddPort( stServiceNode* apoService, const char* apsName, const char* apsBinding )
    {
        TBindingNode* lpoBindingNode = clsBinding::Instance()->Find( apsBinding );
        if ( lpoBindingNode == NULL )
        {
            throw clsException( clsException::eError, Format("[Service]:binding %s undefined.", apsName) );
        }
        
        apoService->cmPort[apsName] = stPort( apsName, lpoBindingNode );
    }
    
    iterator Begin()
    {
        return cmServices.begin();
    }
    
    iterator End()
    {
        return cmServices.end();
    }

protected:
    std::map<std::string, stServiceNode> cmServices;
};

typedef clsService::stServiceNode  TServiceNode;
typedef clsService::stPort         TServicePort;

#endif //__AI_PORTTYPE_H__
