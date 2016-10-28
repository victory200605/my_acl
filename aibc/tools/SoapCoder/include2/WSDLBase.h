
#ifndef AIBC_BASEWSDL_OBJECT_H
#define AIBC_BASEWSDL_OBJECT_H

#include "acl/Xml.h"
#include "acl/Number.h"
#include "acl/MemoryBlock.h"
#include "acl/Codecs.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"
/////////////////////////////////////////////Common///////////////////////////////
namespace {

#define SOAP_RETURN_IF(ret, exp) if(exp) { return ret;}
#define SOAP_GOTO_IF(to, exp) if(exp) { goto to; }
#define SOAP_GOTO_IF_EX(to, exp, do) if(exp) { do; goto to; }

inline const char* TailNamespace( const char* apcNameSpace, const char* apcName, char* apcBuff, apl_size_t auSize )
{
    apl_size_t luNsLen = apl_strlen(apcNameSpace);
    
    // copy string
    if ( apcNameSpace != NULL && luNsLen > 0 )
    {
        apl_snprintf( apcBuff, auSize, "%s:%s", apcNameSpace, apcName );
    }
    else
    {
        apl_snprintf( apcBuff, auSize, "%s", apcName );
    }
    
    return apcBuff;
}

inline const char* TailNamespace( const char* apcNameSpace, const char* apcName, std::string& aoOutput )
{
    apl_size_t luNsLen = apl_strlen(apcNameSpace);
    
    // copy string
    if ( apcNameSpace != NULL && luNsLen > 0 )
    {
        aoOutput = apcNameSpace;
        aoOutput += ":";
        aoOutput += apcName;
    }
    else
    {
        aoOutput = apcNameSpace;
    }

    return aoOutput.c_str();
}

inline const char* EraseNamespace( const char* apcName )
{
    const char* lpcBaseName = apl_strchr( apcName, ':' );
        
    return ( lpcBaseName == NULL ? apcName : lpcBaseName + 1 );
}

}//end namespace {}

//////////////////////////////////////////Soap/////////////////////////////////////////////////
namespace soap
{

enum ErrorState
{
    NO_ERROR              =  0,
    ERROR_PARSE_XML       = -1,
    ERROR_GEN_XML         = -2,
    ERROR_PARSE_HEADER    = -3,
    ERROR_ELEMENT_UNEXIST = -4,
    ERROR_NODE_UNEXIST    = -5,
    ERROR_OUT_OF_RANGE    = -6,
    ERROR_SOAP_ENV        = -7,
    ERROR_SOAP_HEADER     = -8,
    ERROR_SOAP_BODY       = -9
};

//define int&long class type
typedef acl::TNumber<apl_int32_t, acl::CNullLock> CInt;
typedef acl::TNumber<apl_int64_t, acl::CNullLock> CLong;

inline acl::xml::McbXMLElement* McbFindElementNns( acl::xml::McbXMLElement* apoElement, const char* apcName )
{
    // Soap header
    apl_size_t luIdx = 0;
    acl::xml::McbXMLElement* lpoSubElem = NULL;
    while( ( lpoSubElem = acl::xml::McbEnumElements( apoElement, &luIdx ) ) != NULL )
    {
        if ( apl_strcmp( EraseNamespace( lpoSubElem->mpcName ), apcName ) == 0 )
        {
            break;
        }
    }
    
    return lpoSubElem;
}

inline apl_size_t McbFindElementsNns( acl::xml::McbXMLElement* apoElement, const char* apcName, acl::xml::McbXMLElement **apoElemArray, apl_size_t auCount )
{
    // Soap header
    apl_size_t luIdx = 0;
    apl_size_t luNum = 0;
    acl::xml::McbXMLElement* lpoSubElem = NULL;
    while( (lpoSubElem = acl::xml::McbEnumElements(apoElement, &luIdx) ) != NULL )
    {
        if (apl_strcmp(EraseNamespace(lpoSubElem->mpcName ), apcName) == 0 )
        {
            if ( luNum < auCount )
            {
                apoElemArray[luNum++] = lpoSubElem;
            }
            else
            {
                break;
            }
        }                
    }
    
    return luNum;
}

inline apl_size_t McbFindElementsNns( acl::xml::McbXMLElement* apoElement, const char* apcName, std::vector<acl::xml::McbXMLElement*>& aoElements )
{
    // Soap header
    apl_size_t luIdx = 0;
    acl::xml::McbXMLElement* lpoSubElem = NULL;
    
    aoElements.clear();
    
    while( (lpoSubElem = acl::xml::McbEnumElements(apoElement, &luIdx) ) != NULL )
    {
        if (apl_strcmp(EraseNamespace(lpoSubElem->mpcName ), apcName) == 0 )
        {
            aoElements.push_back(lpoSubElem);
        }                
    }
    
    return aoElements.size();
}

//////////////////////////////////////////SoapEnvelope/////////////////////////////////////////
class CSoapEnvelope
{
public:
    // Xml attribute
    struct CAttribute
    {
        CAttribute()
        {
            apl_memset( this, 0, sizeof(CAttribute) );
        }
        
        CAttribute( const char* apcName, const char* apcValue )
        {
            apl_strncpy(this->macName, apcName, sizeof(this->macName) );
            apl_strncpy(this->macValue, apcValue, sizeof(this->macValue) );
        }

        char macName[APL_NAME_MAX];
        char macValue[APL_NAME_MAX];
    };
    
public:
    typedef std::vector<CAttribute>::iterator iterator;

public:
    CSoapEnvelope()
    {
        this->SetEnvelopeNamespace("soapenv");
    }
    
    void SetEnvelopeNamespace( const char* apcNamespace )
    {
        apl_snprintf(this->macEnvelope, sizeof(this->macEnvelope), "%s:Envelope", apcNamespace );
        apl_snprintf(this->macHeader, sizeof(this->macHeader), "%s:Header", apcNamespace );
        apl_snprintf(this->macBody, sizeof(this->macBody), "%s:Body", apcNamespace );
    }

    void PutEnvelopeAttr( const char* apcName, const char* apcValue )
    {
        if ( !this->ReplaceAttr( this->moEnvelopeAttr, apcName, apcValue) )
        {
            this->moEnvelopeAttr.push_back( CAttribute(apcName, apcValue) );
        }
    }
    
    void PutHeaderAttr( const char* apcName, const char* apcValue )
    {
        if ( !this->ReplaceAttr(this->moHeaderAttr, apcName, apcValue) )
        {
            this->moHeaderAttr.push_back( CAttribute(apcName, apcValue) );
        }
    }
    
    void PutBodyAttr( const char* apcName, const char* apcValue )
    {
        if ( !this->ReplaceAttr(this->moBodyAttr, apcName, apcValue) )
        {
            this->moBodyAttr.push_back( CAttribute(apcName, apcValue) );
        }
    }
    
    ////////////////////////////////////////////////////////////////////
    acl::xml::McbXMLElement* EncodeSoapEnvelope( acl::xml::McbXMLElement* apoCurrElem )
    {
        // Soap header
        acl::xml::McbXMLElement* lpoSubElem = 
            acl::xml::McbAddElement(apoCurrElem, apl_strdup(this->macEnvelope), 1, 1);
        SOAP_RETURN_IF(lpoSubElem, (lpoSubElem == NULL) );
        
        // Attribute
        for( CSoapEnvelope::iterator loIt = this->moEnvelopeAttr.begin(); 
             loIt != this->moEnvelopeAttr.end(); ++loIt)
        {
            acl::xml::McbAddAttribute(
               lpoSubElem, 
               apl_strdup(loIt->macName), 
               apl_strdup(loIt->macValue), 
               this->moEnvelopeAttr.size() );
        }

        return lpoSubElem;
    }

    acl::xml::McbXMLElement* EncodeSoapHeader( acl::xml::McbXMLElement* apoCurrElem )
    {
        // Soap header
        acl::xml::McbXMLElement* lpoSubElem = 
            acl::xml::McbAddElement(apoCurrElem, apl_strdup(this->macHeader), 1, 1);
        SOAP_RETURN_IF( lpoSubElem, (lpoSubElem == NULL) );
        
        // Attribute
        for( CSoapEnvelope::iterator loIt = this->moHeaderAttr.begin(); 
             loIt != this->moHeaderAttr.end(); ++loIt)
        {
            acl::xml::McbAddAttribute(
                lpoSubElem,
                apl_strdup(loIt->macName),
                apl_strdup(loIt->macValue), 
                this->moHeaderAttr.size() );
        }

        return lpoSubElem;
    }

    acl::xml::McbXMLElement* EncodeSoapBody( acl::xml::McbXMLElement* apoCurrElem )
    {
        // Soap body
        acl::xml::McbXMLElement* lpoSubElem = 
            acl::xml::McbAddElement(apoCurrElem, apl_strdup(this->macBody), 1, 1);
        SOAP_RETURN_IF( lpoSubElem, (lpoSubElem == NULL) );
        
        // Attribute
        for( CSoapEnvelope::iterator loIt = this->BeginBodyAttr(); 
             loIt != this->EndBodyAttr(); loIt++ )
        {
            acl::xml::McbAddAttribute(
                lpoSubElem,
                apl_strdup(loIt->macName),
                apl_strdup(loIt->macValue),
                this->moBodyAttr.size() );
        }

        return lpoSubElem;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    acl::xml::McbXMLElement* DecodeSoapEnvelope( acl::xml::McbXMLElement* apoCurrElem )
    {
        // Soap Envelope
        acl::xml::McbXMLElement* lpoSubElem = soap::McbFindElementNns(apoCurrElem, "Envelope");
        SOAP_RETURN_IF( lpoSubElem, (lpoSubElem == NULL) );
        
        // Attribute
        apl_size_t luIndex = 0;
        acl::xml::McbXMLAttribute* lpoAttr = NULL;
        while( ( lpoAttr = acl::xml::McbEnumAttributes( lpoSubElem, &luIndex ) ) != NULL )
        {
            this->PutEnvelopeAttr(lpoAttr->mpcName, lpoAttr->mpcValue);
        }

        return lpoSubElem;
    }

    acl::xml::McbXMLElement* DecodeSoapHeader( acl::xml::McbXMLElement* apoCurrElem )
    {
        // Soap header
        acl::xml::McbXMLElement* lpoSubElem = soap::McbFindElementNns( apoCurrElem, "Header" );
        SOAP_RETURN_IF( lpoSubElem, (lpoSubElem == NULL) );
        
        // Attribute
        apl_size_t luIndex = 0;
        acl::xml::McbXMLAttribute* lpoAttr = NULL;
        while( (lpoAttr = acl::xml::McbEnumAttributes(lpoSubElem, &luIndex) ) != NULL )
        {
            this->PutHeaderAttr(lpoAttr->mpcName, lpoAttr->mpcValue);
        }

        return lpoSubElem;
    }

    acl::xml::McbXMLElement* DecodeSoapBody( acl::xml::McbXMLElement* apoCurrElem )
    {
        // Soap Body
        acl::xml::McbXMLElement* lpoSubElem = soap::McbFindElementNns( apoCurrElem, "Body" );
        SOAP_RETURN_IF(lpoSubElem, (lpoSubElem == NULL) );
        
        // Attribute
        apl_size_t luIndex = 0;
        acl::xml::McbXMLAttribute* lpoAttr = NULL;
        while( ( lpoAttr = acl::xml::McbEnumAttributes(lpoSubElem, &luIndex) ) != NULL )
        {
            this->PutBodyAttr( lpoAttr->mpcName, lpoAttr->mpcValue );
        }

        return lpoSubElem;
    }
    
    void Clear()
    {
        this->moEnvelopeAttr.clear();
        this->moHeaderAttr.clear();
        this->moBodyAttr.clear();
    }

    iterator BeginEnvAttr()                                   { return moEnvelopeAttr.begin(); }
    iterator BeginHeaderAttr()                                { return moHeaderAttr.begin(); }
    iterator BeginBodyAttr()                                  { return moBodyAttr.begin(); }
                                                              
    iterator EndEnvAttr()                                     { return moEnvelopeAttr.end(); }
    iterator EndHeaderAttr()                                  { return moHeaderAttr.end(); }
    iterator EndBodyAttr()                                    { return moBodyAttr.end(); }
    
    static const char* GetName( iterator aoIt )               { return aoIt->macName; };
    static const char* GetValue( iterator aoIt )              { return aoIt->macValue; };
    

protected:    
    bool ReplaceAttr( std::vector<CAttribute>& aoList, const char* apcName, const char* apcValue )
    {
        for( apl_size_t liIdx = 0; liIdx < aoList.size(); liIdx++ )
        {
            if (apl_strcmp( aoList[liIdx].macName, apcName ) == 0 )
            {
                apl_strncpy(aoList[liIdx].macValue, apcValue, sizeof(aoList[liIdx].macValue) );
                
                return true;
            }
        }

        return false;
    }

protected:
    char macEnvelope[20];
    char macHeader[20];
    char macBody[20];
    
    std::vector<CAttribute> moEnvelopeAttr;
    std::vector<CAttribute> moHeaderAttr;
    std::vector<CAttribute> moBodyAttr;
};

///////////////////////////////////////WSDLRequest//////////////////////////////////////////
class CWSDLRequest
{   
public:
    CWSDLRequest( CSoapEnvelope* apoSoapEnv )
        : mpoRootElement(NULL)
        , mpoSoapEnvlope(NULL)
        , mpoSoapHeader(NULL)
        , mpoSoapBody(NULL)
        , mpoSoapEnv(apoSoapEnv)
    {
        apl_memset( this->macRequestName, 0, sizeof(this->macRequestName) );
        this->mpcRequestNameNns = this->macRequestName;
    }
    
    ~CWSDLRequest()
    {
        // Recycle Mcb Root Element
        if ( this->mpoRootElement != NULL )
        {
            acl::xml::McbDeleteRoot( this->mpoRootElement );
            this->mpoRootElement = NULL;
        }
    }

    int operator () ( const char* apcXML, apl_int_t* apiLine = NULL, apl_int_t* apiColumn = NULL )
    {
        if ( this->mpoRootElement != NULL )
        {
            // Do again, and clear last one
            this->Clear();
        }
        
        // Parse xml
        this->mpoRootElement = acl::xml::McbParseXML(apcXML, &this->moState);
        if (apiLine != NULL )
        {
            *apiLine = this->moState.miLine;
        }

        if (apiColumn != NULL ) 
        {
            *apiColumn = this->moState.miColumn;
        }

        SOAP_RETURN_IF( soap::ERROR_PARSE_XML, (this->mpoRootElement == NULL) );
        
        // Soap?
        if ( this->mpoSoapEnv == NULL )
        {
            FindRequestName( this->mpoRootElement );
            return soap::NO_ERROR;
        }
        
        // Check soap envelope
        this->mpoSoapEnvlope = this->mpoSoapEnv->DecodeSoapEnvelope( this->mpoRootElement );
        SOAP_RETURN_IF( soap::ERROR_SOAP_ENV, (this->mpoSoapEnvlope == NULL) );
        
        // Check soap header
        this->mpoSoapHeader = this->mpoSoapEnv->DecodeSoapHeader( this->mpoSoapEnvlope );
        SOAP_RETURN_IF( soap::ERROR_SOAP_HEADER, (this->mpoSoapHeader == NULL) );
        
        // Check soap body
        this->mpoSoapBody = this->mpoSoapEnv->DecodeSoapBody( this->mpoSoapEnvlope );
        SOAP_RETURN_IF( soap::ERROR_SOAP_BODY, (this->mpoSoapBody == NULL) );
        
        FindRequestName( this->mpoSoapBody );
        
        return soap::NO_ERROR;
    }
    
    void Clear()
    {
        // Clear envelope
        if ( this->mpoSoapEnv != NULL )
        {
            this->mpoSoapEnv->Clear();
        }
        
        // Recycle Mcb Root Element
        if ( this->mpoRootElement != NULL )
        {
            acl::xml::McbDeleteRoot( this->mpoRootElement );
            this->mpoRootElement = NULL;
        }
        
        this->mpoRootElement = NULL;
        this->mpoSoapEnvlope = NULL;
        this->mpoSoapHeader  = NULL;
        this->mpoSoapBody    = NULL;
        
        apl_memset( this->macRequestName, 0, sizeof(this->macRequestName) );
        this->mpcRequestNameNns = this->macRequestName;
    }
    
    CSoapEnvelope* GetSoapEnvelope()                                { return mpoSoapEnv; }
    acl::xml::McbXMLElement* GetRootElement()                       { return mpoRootElement; }
    acl::xml::McbXMLElement* GetEnvelope()                          { return mpoSoapEnvlope; }
    acl::xml::McbXMLElement* GetHeader()                            { return mpoSoapHeader; }
    acl::xml::McbXMLElement* GetBody()                              { return mpoSoapBody; }
    
    const acl::xml::McbXMLElement* GetRootElement() const           { return mpoRootElement; }
    const acl::xml::McbXMLElement* GetEnvelope()    const           { return mpoSoapEnvlope; }
    const acl::xml::McbXMLElement* GetHeader()      const           { return mpoSoapHeader; }
    const acl::xml::McbXMLElement* GetBody()        const           { return mpoSoapBody; }
    
    const char* What()const                                     { return macRequestName; }
    const char* WhatNns()const                                  { return mpcRequestNameNns; }

protected:
    void FindRequestName( acl::xml::McbXMLElement* apoElement )
    {
        apl_size_t luIdx = 0;
        acl::xml::McbXMLElement* lpoSubElement = McbEnumElements( apoElement, &luIdx );
        if ( lpoSubElement != NULL )
        {
            apl_strncpy(this->macRequestName, lpoSubElement->mpcName, sizeof(this->macRequestName) );
            this->mpcRequestNameNns = apl_strchr(this->macRequestName, ':');
            if (this->mpcRequestNameNns == NULL)
            {
                this->mpcRequestNameNns = this->macRequestName;
            }
            else
            {
                this->mpcRequestNameNns++;
            }
        }
    }
    
protected:
    acl::xml::McbXMLElement* mpoRootElement;
    acl::xml::McbXMLElement* mpoSoapEnvlope;
    acl::xml::McbXMLElement* mpoSoapHeader;
    acl::xml::McbXMLElement* mpoSoapBody;
    acl::xml::McbXMLResults  moState;
    
    char macRequestName[APL_NAME_MAX];
    const char* mpcRequestNameNns;
    
    CSoapEnvelope* mpoSoapEnv;
};

////////////////////////////////////////////////////////////////////////////////////////
class CBaseWSDL
{
public:
    enum //Escape character
    {
        DO_ESCAPE = 1,
        DONT_ESCAPE = 0
    };
    
public:
    CBaseWSDL(CSoapEnvelope* apoEnv = NULL, apl_int_t aiOpt = DONT_ESCAPE, apl_size_t auBufferSize = 4096 ) 
        : mpoSoapEnvelope(apoEnv)
        , moResult(auBufferSize)
        , miOpt(aiOpt)
    {
        apl_memset( this->macCurrElementName, 0, sizeof(this->macCurrElementName) );
        apl_strncpy( this->macXmlHeader, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", sizeof(this->macXmlHeader) );
    }
    
    virtual ~CBaseWSDL()
    {
    }
    
    // operation
    virtual const char* Encode(void) = 0;
    virtual apl_int_t   Decode( CWSDLRequest& apoRequest ) = 0;
    virtual apl_int_t   Decode( const char* apcXMLString )   = 0;

    const char* GetCurrElement()
    {
        return this->macCurrElementName;
    }
    
protected:
    acl::xml::McbXMLElement* EncodeXmlHeader( acl::xml::McbXMLElement* apoCurrElem )
    {
        return apoCurrElem;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////
    acl::xml::McbXMLElement* DecodeXmlHeader( acl::xml::McbXMLElement* apoCurrElem )
    {
        return apoCurrElem;
    }
    
    CSoapEnvelope* GetSoapEnvelope()
    {
        return this->mpoSoapEnvelope;
    }
    
    void SetCurrElement( const char* apcName )
    {
        apl_strncpy( this->macCurrElementName, apcName, sizeof(this->macCurrElementName) );
    }
    
    int CreateXMLString( acl::xml::McbXMLElement *apoHead )
    {
        apl_int_t liFormat = 0;
        apl_int_t liLength = 0;
        apl_int_t liHeadLen = apl_strlen(this->macXmlHeader);
        
        SOAP_RETURN_IF(soap::ERROR_GEN_XML, apoHead == NULL);
        
        liLength = acl::xml::McbCreateXMLStringR(apoHead, NULL, liFormat);
        SOAP_RETURN_IF( soap::ERROR_GEN_XML, liLength <= 0 );
        
        this->moResult.Resize( liHeadLen + liLength + 1 );
        apl_memcpy( this->moResult.GetWritePtr(), this->macXmlHeader, liHeadLen );
    
        acl::xml::McbCreateXMLStringR(apoHead, this->moResult.GetWritePtr() + liHeadLen, liFormat );

        this->moResult.GetWritePtr()[liHeadLen + liLength] = '\0';

        return soap::NO_ERROR;
    }
    
    char* ResultPointer() { return moResult.GetReadPtr(); }

public:
    ///////////////////////////////////// Assign function ///////////////////////////////////////
    void Assign( char* apcL, const char* apcR )
    {
        const char* lpcR = ( apcR == NULL ? "" : apcL );
        
        if (this->miOpt == DO_ESCAPE)
        {
            this->moEscapeDecoder.Final(lpcR);

            strcpy(apcL, this->moEscapeDecoder.GetOutput() );
        }
        else
        {
            strcpy(apcL, lpcR);
        }
    }
    
    void Assign( std::string& aoL, const char* apcR )
    {
        const char* lpcR = (apcR == NULL ? "" : apcR);
        
        if (this->miOpt == DO_ESCAPE)
        {
            this->moEscapeDecoder.Final(lpcR);
            aoL = this->moEscapeDecoder.GetOutput();
        }
        else
        {
            aoL = lpcR;
        }
    }
    
    void Assign( acl::CMemoryBlock& aoL, const char* apcR )
    {
        const char* lpcR  = ( apcR == NULL ? "" : apcR );
        
        if (this->moBase64Decoder.Final(lpcR) == 0)
        {
            aoL.Reset();
            aoL.Resize(this->moBase64Decoder.GetLength() );
            apl_memcpy(aoL.GetWritePtr(), this->moBase64Decoder.GetOutput(), this->moBase64Decoder.GetLength() );
            aoL.SetWritePtr(this->moBase64Decoder.GetLength() );
        }
    }
    
    void Assign( soap::CInt& aoL, const char* apcR )
    {
        if ( apcR != NULL )
        {
            aoL = apl_strtoi32(apcR, NULL, 10);
        }
    }
    
    void Assign( soap::CLong& aoL, const char* apcR )
    {
        if ( apcR != NULL )
        {
            aoL = apl_strtoi64(apcR, NULL, 10);
        }
    }
    
    void Assign( bool& aiL, const char* apcR )
    {
        if ( apcR != NULL )
        {
            aiL = ( apcR[0] == '1' ? true : false );
        }
    }

    ///////////////////////////////// Other type convert to string ////////////////////////////////////
    char* ToString( const char* apcR )
    {
        char*       lpcValue = NULL;
        const char* lpcR     = (apcR == NULL? "" : apcR);
        
        if (this->miOpt == DO_ESCAPE)
        {
            this->moEscapeEncoder.Final(lpcR);
            ACL_MALLOC_ASSERT(lpcValue, char, this->moEscapeEncoder.GetLength() + 1);
            apl_strncpy(lpcValue, this->moEscapeEncoder.GetOutput(), this->moEscapeEncoder.GetLength() + 1);
        }
        else
        {
            lpcValue = apl_strdup(apcR);
        }

        return lpcValue;
    }
    
    char* ToString( const char* apcNameSpace, const char* apcR )
    {
        char*       lpcValue  = NULL;
        const char* lpcR      = (apcR == NULL? "" : apcR);
        apl_size_t  luNeedLen = apl_strlen(apcNameSpace) + apl_strlen(lpcR) + 2;
        
        ACL_MALLOC_ASSERT(lpcValue, char, luNeedLen);
         
        TailNamespace(apcNameSpace, lpcR, lpcValue, luNeedLen);
         
        return lpcValue;
    }
    
    char* ToString( const char* apcNameSpace, std::string& aoR )
    {     
        return this->ToString( apcNameSpace, aoR.c_str() );
    }
    
    char* ToString( const std::string& aoR )
    {
        return this->ToString(aoR.c_str() );
    }
    
    char* ToString( const soap::CInt& aoR )
    {
        char* lpcValue = NULL;
        
        ACL_MALLOC_ASSERT(lpcValue, char, 20);
        
        if ( aoR.IsEmpty() )
        {
            apl_memset(lpcValue, 0, 20);
        }
        else
        {
            apl_snprintf( lpcValue, 20, "%"APL_PRId32, (apl_int_t)aoR );
        }
        
        return lpcValue;
    }
    
    char* ToString( const soap::CLong& aoR )
    {
        char* lpcValue = NULL;
        
        ACL_MALLOC_ASSERT(lpcValue, char, 30);
        
        if ( aoR.IsEmpty() )
        {
            apl_memset(lpcValue, 0, 30);
        }
        else
        {
            apl_snprintf(lpcValue, 30, "%"APL_PRId64, (apl_int64_t)aoR);
        }

        return lpcValue;
    }
    
    char* ToString( bool abR )
    {
        char* lpcValue = NULL;
        
        ACL_MALLOC_ASSERT( lpcValue, char, 2 );
        
        lpcValue[0] = ( abR ? '1' : '0');
        lpcValue[1] = '\0';
        
        return lpcValue;
    }
    
    char* ToString( const acl::CMemoryBlock& aoR )
    {
        if (aoR.GetLength() > 0)
        {
            this->moBase64Encoder.Final(aoR.GetReadPtr(), aoR.GetLength() );
        
            return apl_strdup(this->moBase64Encoder.GetOutput() );
        }
        else
        {
            return apl_strdup("");
        }
    }

protected:
    CSoapEnvelope* mpoSoapEnvelope;
    
    char macXmlHeader[256];
        
    acl::CMemoryBlock moResult;
        
    char macCurrElementName[APL_NAME_MAX];
    
    acl::CXmlEscapeEncoder moEscapeEncoder;

    acl::CXmlEscapeDecoder moEscapeDecoder;
    
    acl::CBase64Encoder moBase64Encoder;

    acl::CBase64Decoder moBase64Decoder;

    apl_int_t miOpt;
};


}

#endif //AIBC_SOAP_WSDLBASE_H
