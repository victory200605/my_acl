//=============================================================================
/**
 * \file    Element.h
 * \brief
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: Element.h,v 1.2 2009/03/09 02:34:56 daizh Exp $
 *
 * History
 * 2008.01.27 first release by DZH
 */
//=============================================================================
#ifndef __AI_ELEMENT_H__
#define __AI_ELEMENT_H__

#include "Exception.h"
#include <assert.h>
#include <string>
#include <deque>
#include <set>

/** 
 *	\brief element base class
 */
class clsElement
{
public:
    /* element node type */
    enum { eNature = 0, eElement, eComplex };

    clsElement( void ):
        ciDepth(0),
        ciType(eNature),
        ciMinOccurs(-1),
        ciMaxOccurs(-1),
		cpoTypeElement( NULL )
    {
    }
    
    clsElement( int aiType, const char* apsTypeName, const char* apsName, const char* apsNamespace ):
        ciDepth(0),
        ciType(aiType),
        ciMinOccurs(-1),
        ciMaxOccurs(-1),
        coStrTypeName(apsTypeName),
        coStrName(apsName),
        coStrNamespace(apsNamespace),
        cpoTypeElement(NULL)
    {
    }
    
    virtual ~clsElement()
    {
    }

    /* set operator define */
    void SetType( int aiType )                   { ciType = aiType; }
    void SetMinOccurs( int aiMin )               { ciMinOccurs = aiMin; }                                      
    void SetMaxOccurs( int aiMax )               { ciMaxOccurs = aiMax; }                                      
    void SetTypeName( const char* apsName )      { coStrTypeName = apsName; }                                       
    void SetName( const char* apsName )          { coStrName = apsName; }
    void SetNamespace( const char* apsName )     { coStrNamespace = apsName; }                                      
    void SetTypeElement( clsElement* apoElem )   { cpoTypeElement = apoElem; }
    void SetDepth( int aiN )                     { ciDepth = aiN; }
                                                 
    /* get operator */                           
    int GetType()                                { return ciType; }
    int GetMinOccurs()                           { return ciMinOccurs; }
    int GetMaxOccurs()                           { return ciMaxOccurs; }
    int GetDepth() const                         { return ciDepth; }
                                                 
    const char* GetTypeName( void )              { return coStrTypeName.c_str(); }
    const char* GetName( void )                  { return coStrName.c_str(); }
    const char* GetNamespace( void )             { return coStrNamespace.c_str(); }
    clsElement* GetTypeElement( void )           { return cpoTypeElement; }
    const char* GetTypet( void )                 
    { 
        switch(this->GetType())
        { 
            case eNature: return "nature";
            case eElement:return "element";
            case eComplex:return "complex";
            default:      return "unknow";
        };
    }
    
    clsElement* GetRootElement( void )
    {
        clsElement* loTmp = this;
        while ( loTmp->GetType() == eElement )
        {
            loTmp = loTmp->GetTypeElement();
        }
        return loTmp;
    }
    
    /* clone */
    virtual clsElement* Clone() 
    {
		throw clsException( clsException::eError, "Unimplement" );
    }
    /* sub element */
    virtual void SetSubElement( clsElement* apoNode )
    {
        throw clsException( clsException::eError, "Unimplement" );
    }
    
protected:
    int         ciDepth; // for declare sequence
    int         ciType;  // element type
    int         ciMinOccurs;
    int         ciMaxOccurs;
    
    std::string coStrName;
    std::string coStrTypeName;
    std::string coStrNamespace;

    clsElement* cpoTypeElement;
};

class clsSingleElement : public clsElement
{
public:
	clsSingleElement( const char* apsTypeName, const char* apsName, const char* apsNamespace = "" ) : 
	    clsElement( eElement, apsTypeName, apsName, apsNamespace )
    {
    };
};

class clsComplexElement : public clsElement
{
public:
    typedef std::deque<clsElement*>::iterator iterator;
	typedef std::deque<clsElement*>::reverse_iterator iterator_r;
    enum EXTENSTAT { eExtenEmpty = 0, eExtenPanding, eExtenCompleted };

public:
	clsComplexElement() 
	    : clsElement( eComplex, "struct", "", "" )
	    , ciExtensionStat(eExtenEmpty)
	{
	}
    
    virtual void SetSubElement( clsElement* apoElement )
    {
        PutDup( apoElement->GetName() );
        cvChilds.push_back( apoElement );
    }
    
    virtual void SetSubElementFront( clsElement* apoElement )
    {
        PutDup( apoElement->GetName() );
        cvChilds.push_front( apoElement );
    }
    
    int GetExtensionStat()
    {
        return ciExtensionStat;
    }
    
    void UpdateExtensionStat( EXTENSTAT aiStat )
    {
        ciExtensionStat = aiStat;
    }
    
    void SetExtension( const char* apcExtension )
    {
        coStrExtension  = apcExtension;
        ciExtensionStat = eExtenPanding;
    }
    
    const char* GetExtension()
    {
        return coStrExtension.c_str();
    }

    iterator Begin()    { return cvChilds.begin(); }
    iterator End()      { return cvChilds.end();   }
    iterator_r RBegin() { return cvChilds.rbegin(); }
    iterator_r REnd()   { return cvChilds.rend();   }

protected:
    void PutDup( const char* apcName )
    {
        if ( csetDup.find( apcName ) == csetDup.end() )
        {
            csetDup.insert( std::set<std::string>::value_type(apcName) );
        }
        else
        {
            throw clsException( clsException::eError, Format("Element %s redefined", apcName) );
        }
    }

protected:
    int                      ciExtensionStat;
    std::string              coStrExtension;
    std::deque<clsElement*>  cvChilds;

    std::set<std::string>    csetDup;
};

#endif
