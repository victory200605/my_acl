//=============================================================================
/**
 * \file    Types.h
 * \brief
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: Types.h,v 1.3 2010/07/17 13:49:11 daizh Exp $
 *
 * History
 * 2008.01.27 first release by DZH
 */
//=============================================================================
#ifndef __AI_TYPES_H__
#define __AI_TYPES_H__

#include "Singleton.h"
#include "Element.h"
#include "Exception.h"
#include <algorithm>
#include <vector>
#include <map>

struct clsElementLess
{
    bool operator () ( const clsElement* apoLeft, const clsElement* apoRight ) const
    {
        return apoLeft->GetDepth() > apoRight->GetDepth();
    }
};

class clsTypes : public Singleton<clsTypes>
{
public:
    typedef std::vector<clsElement*>::iterator iterator;
        
public:
	virtual ~clsTypes()
	{
		for ( iterator loIt = this->Begin(); loIt != this->End(); loIt++ )
		{
			delete (*loIt);
		}
	}

    void AddElement( const char* apsName, clsElement* apoElement )
    {
		if (cmTypes.find(apsName) != cmTypes.end() )
		{
			throw clsException( clsException::eError, Format("[Element]:Element name %s redefined.", apsName) ); 
		}

        cmTypes[apsName] = apoElement;
        cvTypes.push_back(apoElement);
    }
    
    clsElement* Find( const char* apsName )
    {
        std::map<std::string, clsElement*>::iterator loIt;
        
        if ( ( loIt = cmTypes.find(apsName) ) != cmTypes.end() )
        {
            return loIt->second;
        }
        
        return NULL;
    }
    
    void ProcessExtendComplexType()
    {
        for( iterator loIt = cvTypes.begin(); loIt != cvTypes.end(); loIt++ )
        {
            if ( (*loIt)->GetType() == clsElement::eComplex )
            {
                this->ExtendComplexType( *loIt );
            }
        }
    }
    
    void ProcessTypeLink( )
    {
        for( iterator loIt = cvTypes.begin(); loIt != cvTypes.end(); loIt++ )
        {
            if ( (*loIt)->GetType() == clsElement::eElement )
            {
                this->LinkUndefinedElement( *loIt );
			}
            else if ( (*loIt)->GetType() == clsElement::eComplex )
            {
                this->LinkComplexElement( *loIt );
            }
        }
    }
    
    void ProcessForwardDeclare( )
    {
        for( iterator loIt = cvTypes.begin(); loIt != cvTypes.end(); loIt++ )
        {
            CalculateDeclareDepth( *loIt, (*loIt)->GetDepth() );
        }
        
        std::sort( cvTypes.begin(), cvTypes.end(), clsElementLess() );
    }
    iterator Begin()
    {
        return cvTypes.begin();
    }
    iterator End()
    {
        return cvTypes.end();
    }

protected:
    void ExtendComplexType( clsElement* lpoElement )
    {
        clsComplexElement* lpoComplex = (clsComplexElement*)(lpoElement);
        if ( lpoComplex->GetExtensionStat() == clsComplexElement::eExtenPanding )
        {
            clsElement* lpoParentElement = this->Find( lpoComplex->GetExtension() );
            if ( lpoParentElement == NULL )
            {
                throw clsException( clsException::eError, Format("[types]:%s undefined.", lpoElement->GetTypeName()) );
            }
            if ( lpoParentElement->GetType() == clsElement::eComplex )
            {
                clsComplexElement* lpoParentComplex = (clsComplexElement*)(lpoParentElement);
                this->ExtendComplexType( lpoParentElement );
                for ( clsComplexElement::iterator_r loIter = lpoParentComplex->RBegin();
                      loIter != lpoParentComplex->REnd(); loIter++ )
                {
                    lpoComplex->SetSubElementFront( *loIter );
                }
                lpoComplex->UpdateExtensionStat( clsComplexElement::eExtenCompleted );
            }
        }
    }
    
    void LinkUndefinedElement( clsElement* lpoElement )
    {
        if ( lpoElement->GetType() == clsElement::eElement )
        {
            clsElement* lpoParentElement = this->Find( lpoElement->GetTypeName() );
            if ( lpoParentElement == NULL )
            {
                throw clsException( clsException::eError, Format("[types]:%s undefined.", lpoElement->GetTypeName()) );
            }
            
            if ( lpoParentElement->GetType() == clsElement::eNature )
            {
                lpoElement->SetType( clsElement::eNature );
                lpoElement->SetTypeName( lpoParentElement->GetTypeName() );
            }
            lpoElement->SetTypeElement( lpoParentElement );
		}
    }
    void LinkComplexElement( clsElement* lpoElement )
    {
        clsComplexElement* lpoComplex = (clsComplexElement*)lpoElement;
        
        for ( clsComplexElement::iterator loIt = lpoComplex->Begin(); loIt != lpoComplex->End(); loIt++ )
        {
            this->LinkUndefinedElement( *loIt );
        }
    }
    void CalculateDeclareDepth( clsElement* lpoElement, int aiDepth )
    {
        if ( lpoElement->GetType() == clsElement::eElement )
        {
            clsElement* lpoParentElement = lpoElement->GetTypeElement( );
            if ( lpoParentElement->GetDepth() <= aiDepth )
            {
                lpoParentElement->SetDepth( aiDepth + 1 );
                this->CalculateDeclareDepth( lpoParentElement, aiDepth + 1 );
            }
		}
        else if ( lpoElement->GetType() == clsElement::eComplex )
        {
            clsComplexElement* lpoComplex = (clsComplexElement*)lpoElement;
            for ( clsComplexElement::iterator loIt = lpoComplex->Begin(); loIt != lpoComplex->End(); loIt++ )
            {
                this->CalculateDeclareDepth( *loIt, aiDepth );
            }
        }
    }
    
protected:
    std::map<std::string, clsElement*> cmTypes;
    std::vector<clsElement*>           cvTypes;
};

#endif
