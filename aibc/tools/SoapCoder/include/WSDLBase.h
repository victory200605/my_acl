
#ifndef __AI_BASEWSDL_OBJECT__
#define __AI_BASEWSDL_OBJECT__

#include "AILib.h"
#include "AICrypt.h"
#include "AIMcbXML.h"
#include "AIString.h"
#include "AITypeObject.h"
#include <string.h>

///compatible last version
#if !defined(AIBC)
#   define _AI_OLD_VERSION
#   define AIBC //namespace
#endif

/////////////////////////////////////////////Common///////////////////////////////
inline const char* TailNamespace( const char* apsNameSpace, const char* apsName, char* apsBuff, size_t aiSize )
{
    size_t liNsLen = ::strlen(apsNameSpace);
    
    // copy string
    if ( apsNameSpace != NULL && liNsLen > 0 )
    {
        ::snprintf( apsBuff, aiSize, "%s:%s", apsNameSpace, apsName );
    }
    else
    {
        ::snprintf( apsBuff, aiSize, "%s", apsName );
    }
    
    return apsBuff;
}

inline const char* TailNamespace( const char* apsNameSpace, const char* apsName, AIBC::AIChunkEx& aoChunk )
{
    size_t liNsLen = ::strlen(apsNameSpace);
    size_t liNmLen = ::strlen(apsName);
    
    aoChunk.Resize( liNsLen + liNmLen + 2 );
    
    return TailNamespace( apsNameSpace, apsName, aoChunk.BasePtr(), aoChunk.GetSize() );
}

inline const char* EraseNamespace( const char* apcName )
{
    const char* lpcBaseName = ::strchr( apcName, ':' );
        
    return ( lpcBaseName == NULL ? apcName : lpcBaseName + 1 );
}

//////////////////////////////////////////Soap/////////////////////////////////////////////////
namespace AISOAP
{
    enum BUFFLEN { AI_MAX_XML_LEN = 4096, AI_MAX_XML_HEADER_LEN = 64, AI_MAX_FIELD_LEN = 128 };
    
    enum ERRORCODE
    {
        AI_NO_ERROR              =  0,
        AI_ERROR_PARSE_XML       = -1,
        AI_ERROR_GEN_XML         = -2,
        AI_ERROR_PARSE_HEADER    = -3,
        AI_ERROR_ELEMENT_UNEXIST = -4,
        AI_ERROR_NODE_UNEXIST    = -5,
        AI_ERROR_OUT_OF_RANGE    = -6,
        AI_ERROR_SOAP_ENV        = -7,
        AI_ERROR_SOAP_HEADER     = -8,
        AI_ERROR_SOAP_BODY       = -9
    };
    
    inline AIBC::McbXMLElement* McbFindElementNns( AIBC::McbXMLElement* apoElement, const char* apcName )
    {
        // Soap header
        int liIdx = 0;
        AIBC::McbXMLElement* lpoSubElem = NULL;
        while( ( lpoSubElem = AIBC::McbEnumElements( apoElement, &liIdx ) ) != NULL )
        {
            if ( ::strcmp( EraseNamespace( lpoSubElem->cpcName ), apcName ) == 0 ) break;                
        }
        
        return lpoSubElem;
    }
    
    inline size_t McbFindElementsNns( AIBC::McbXMLElement* apoElement, const char* apcName, AIBC::McbXMLElement **apoElemArray, size_t aiCount )
    {
        // Soap header
        int liIdx = 0;
        size_t liNum = 0;
        AIBC::McbXMLElement* lpoSubElem = NULL;
        while( ( lpoSubElem = AIBC::McbEnumElements( apoElement, &liIdx ) ) != NULL )
        {
            if ( ::strcmp( EraseNamespace( lpoSubElem->cpcName ), apcName ) == 0 )
            {
                if ( liNum < aiCount )
                {
                    apoElemArray[liNum++] = lpoSubElem;
                }
                else
                {
                    break;
                }
            }                
        }
        
        return liNum;
    }
    
    inline size_t McbFindElementsNns( AIBC::McbXMLElement* apoElement, const char* apcName, vector<AIBC::McbXMLElement*>& aoXmlElements )
    {
        // Soap header
        int liIdx = 0;
        AIBC::McbXMLElement* lpoSubElem = NULL;
        while( ( lpoSubElem = AIBC::McbEnumElements( apoElement, &liIdx ) ) != NULL )
        {
            if ( ::strcmp( EraseNamespace( lpoSubElem->cpcName ), apcName ) == 0 )
            {
                aoXmlElements.push_back(lpoSubElem);
            }                
        }
        
        return aoXmlElements.size();
    }
};

///////////////////////////////////////// string escape ///////////////////////////////////////
class clsStringEscape
{
    struct stEscapeCharacter
    {
        stEscapeCharacter(void)
            : cpcValue(NULL)
            , ciLength(0)
        {
        }
        
        const char* cpcValue;
        size_t ciLength;
    };
    
public:
    clsStringEscape(void)
    {
        this->SetCharacter('<',  "&lt;");
        this->SetCharacter('>',  "&gt;");
        this->SetCharacter('&',  "&amp;");
        this->SetCharacter('\'', "&apos;");
        this->SetCharacter('\"', "&quot;");
    }
    
    int SetCharacter(int aiChr, const char* apcValue)
    {
        if ( aiChr >= 100 || aiChr <= 0 )
        {
            return -1;
        }
        
        this->coEscapeMap[aiChr].cpcValue = apcValue;
        this->coEscapeMap[aiChr].ciLength = strlen(apcValue);
        
        this->coEscapeList.push_back(aiChr);
        
        return 0;
    }
    
    size_t Encode(const char* apcInput, char* apcBuffer)
    {
        size_t liLength = 0;
        while(*apcInput != '\0')
        {
            int liVal = (int)(*apcInput);
            if ( liVal < 100
                && liVal >= 0
                && this->coEscapeMap[liVal].cpcValue != NULL )
            {
                if (apcBuffer != NULL)
                {
                    memcpy(
                        apcBuffer + liLength,
                        this->coEscapeMap[liVal].cpcValue,
                        this->coEscapeMap[liVal].ciLength );
                }
                
                liLength += this->coEscapeMap[liVal].ciLength;
            }
            else
            {
                if (apcBuffer != NULL)
                {
                    apcBuffer[liLength] = *apcInput;
                }
                
                ++liLength;
            }
            
            apcInput++;
        }
        
        if (apcBuffer != NULL)
        {
            apcBuffer[liLength] = '\0';
        }
        
        return liLength;
    }
    
    size_t Decode(const char* apcInput, char* apcBuffer)
    {
        size_t liLength = 0;
        int    liIdx = 0;
        
        while(*apcInput != '\0')
        {
            if ( (liIdx = this->Find(apcInput) ) != -1 )
            {
                apcBuffer[liLength] = (char)liIdx;
                liLength += 1;
                apcInput += this->coEscapeMap[liIdx].ciLength;
            }
            else
            {
                apcBuffer[liLength] = *apcInput;
                liLength += 1;
                apcInput += 1;
            }
        }
        
        apcBuffer[liLength] = '\0';
        
        return liLength;
    }

protected:
    int Find( const char* apcValue )
    {
        for ( size_t liN = 0; liN < this->coEscapeList.size(); liN++ )
        {
            if ( strncmp( 
                this->coEscapeMap[ this->coEscapeList[liN] ].cpcValue,
                apcValue,
                this->coEscapeMap[ this->coEscapeList[liN] ].ciLength ) == 0 )
            {
                return this->coEscapeList[liN];
            }
        }
        
        return -1;
    }

private:
    stEscapeCharacter coEscapeMap[100];
    vector<int> coEscapeList;
};

//////////////////////////////////////////SoapEnvelope/////////////////////////////////////////
class clsSoapEnvelope
{
public:
    // Xml attribute
    struct stAttribute
    {
        stAttribute()
        {
            memset( this, 0, sizeof(stAttribute) );
        }
        stAttribute( const char* apsName, const char* apsValue )
        {
            AIBC::StringCopy( this->ccName, apsName, sizeof(this->ccName) );
            AIBC::StringCopy( this->ccValue, apsValue, sizeof(this->ccValue) );
        }
        char ccName[AISOAP::AI_MAX_FIELD_LEN];
        char ccValue[AISOAP::AI_MAX_FIELD_LEN];
    };
    
public:
    typedef vector<stAttribute>::iterator iterator;

public:
    clsSoapEnvelope()
    {
        this->SetEnvelopeNamespace( "soapenv" );
    }
    
    void SetEnvelopeNamespace( const char* apcNamespace )
    {
        ::snprintf( this->csEnvelope, sizeof(this->csEnvelope), "%s:Envelope", apcNamespace );
        ::snprintf( this->csHeader, sizeof(this->csHeader), "%s:Header", apcNamespace );
        ::snprintf( this->csBody, sizeof(this->csBody), "%s:Body", apcNamespace );
    }

    void PutEnvelopeAttr( const char* apsName, const char* apsValue )
    {
        if ( !this->ReplaceAttr( cvoEnvelopeAttr, apsName, apsValue ) )
        {
            this->cvoEnvelopeAttr.push_back( stAttribute(apsName, apsValue) );
        }
    }
    
    void PutHeaderAttr( const char* apsName, const char* apsValue )
    {
        if ( !this->ReplaceAttr( cvoHeaderAttr, apsName, apsValue ) )
        {
            this->cvoHeaderAttr.push_back( stAttribute(apsName, apsValue) );
        }
    }
    
    void PutBodyAttr( const char* apsName, const char* apsValue )
    {
        if ( !this->ReplaceAttr( cvoBodyAttr, apsName, apsValue ) )
        {
            this->cvoBodyAttr.push_back( stAttribute(apsName, apsValue) );
        }
    }
    
    ////////////////////////////////////////////////////////////////////
    AIBC::McbXMLElement* EncodeSoapEnvelope( AIBC::McbXMLElement* apoCurrElem )
    {
        // Soap header
        AIBC::McbXMLElement* lpoSubElem = AIBC::McbAddElement( apoCurrElem, strdup(this->csEnvelope), 1, 1 );
        AI_RETURN_IF( lpoSubElem, (lpoSubElem == NULL) );
        
        // Attribute
        for( clsSoapEnvelope::iterator loIt = this->BeginEnvAttr(); 
             loIt != this->EndEnvAttr(); loIt++ )
        {
            AIBC::McbAddAttribute( lpoSubElem, strdup( clsSoapEnvelope::GetName(loIt) ), strdup( clsSoapEnvelope::GetValue(loIt) ), 1 );
        }
        return lpoSubElem;
    }
    AIBC::McbXMLElement* EncodeSoapHeader( AIBC::McbXMLElement* apoCurrElem )
    {
        // Soap header
        AIBC::McbXMLElement* lpoSubElem = AIBC::McbAddElement( apoCurrElem, strdup(this->csHeader), 1, 1 );
        AI_RETURN_IF( lpoSubElem, (lpoSubElem == NULL) );
        
        // Attribute
        for( clsSoapEnvelope::iterator loIt = this->BeginHeaderAttr(); 
             loIt != this->EndHeaderAttr(); loIt++ )
        {
            AIBC::McbAddAttribute( lpoSubElem, strdup( clsSoapEnvelope::GetName(loIt) ), strdup( clsSoapEnvelope::GetValue(loIt) ), 1 );
        }
        return lpoSubElem;
    }
    AIBC::McbXMLElement* EncodeSoapBody( AIBC::McbXMLElement* apoCurrElem )
    {
        // Soap body
        AIBC::McbXMLElement* lpoSubElem = AIBC::McbAddElement( apoCurrElem, strdup(this->csBody), 1, 1 );
        AI_RETURN_IF( lpoSubElem, (lpoSubElem == NULL) );
        
        // Attribute
        for( clsSoapEnvelope::iterator loIt = this->BeginBodyAttr(); 
             loIt != this->EndBodyAttr(); loIt++ )
        {
            AIBC::McbAddAttribute( lpoSubElem, strdup( clsSoapEnvelope::GetName(loIt) ), strdup( clsSoapEnvelope::GetValue(loIt) ), 1 );
        }
        return lpoSubElem;
    }
    ////////////////////////////////////////////////////////////////////////////////////
    AIBC::McbXMLElement* DecodeSoapEnvelope( AIBC::McbXMLElement* apoCurrElem )
    {
        // Soap Envelope
        AIBC::McbXMLElement* lpoSubElem = AISOAP::McbFindElementNns( apoCurrElem, "Envelope" );
        AI_RETURN_IF( lpoSubElem, (lpoSubElem == NULL) );
        
        // Attribute
        int liIndex = 0;
        AIBC::McbXMLAttribute* lpoAttr = NULL;
        while( ( lpoAttr = AIBC::McbEnumAttributes( lpoSubElem, &liIndex ) ) != NULL )
        {
            this->PutEnvelopeAttr( lpoAttr->cpcName, lpoAttr->cpcValue );
        }
        return lpoSubElem;
    }
    AIBC::McbXMLElement* DecodeSoapHeader( AIBC::McbXMLElement* apoCurrElem )
    {
        // Soap header
        AIBC::McbXMLElement* lpoSubElem = AISOAP::McbFindElementNns( apoCurrElem, "Header" );
        AI_RETURN_IF( lpoSubElem, (lpoSubElem == NULL) );
        
        // Attribute
        int liIndex = 0;
        AIBC::McbXMLAttribute* lpoAttr = NULL;
        while( ( lpoAttr = AIBC::McbEnumAttributes( lpoSubElem, &liIndex ) ) != NULL )
        {
            this->PutHeaderAttr( lpoAttr->cpcName, lpoAttr->cpcValue );
        }
        return lpoSubElem;
    }
    AIBC::McbXMLElement* DecodeSoapBody( AIBC::McbXMLElement* apoCurrElem )
    {
        // Soap Body
        AIBC::McbXMLElement* lpoSubElem = AISOAP::McbFindElementNns( apoCurrElem, "Body" );
        AI_RETURN_IF( lpoSubElem, (lpoSubElem == NULL) );
        
        // Attribute
        int liIndex = 0;
        AIBC::McbXMLAttribute* lpoAttr = NULL;
        while( ( lpoAttr = AIBC::McbEnumAttributes( lpoSubElem, &liIndex ) ) != NULL )
        {
            this->PutBodyAttr( lpoAttr->cpcName, lpoAttr->cpcValue );
        }
        return lpoSubElem;
    }
    
    void Clear()
    {
        this->cvoEnvelopeAttr.clear();
        this->cvoHeaderAttr.clear();
        this->cvoBodyAttr.clear();
    }

    iterator BeginEnvAttr()                                   { return cvoEnvelopeAttr.begin(); }
    iterator BeginHeaderAttr()                                { return cvoHeaderAttr.begin(); }
    iterator BeginBodyAttr()                                  { return cvoBodyAttr.begin(); }
                                                              
    iterator EndEnvAttr()                                     { return cvoEnvelopeAttr.end(); }
    iterator EndHeaderAttr()                                  { return cvoHeaderAttr.end(); }
    iterator EndBodyAttr()                                    { return cvoBodyAttr.end(); }
    
    static const char* GetName( iterator aoIt )               { return aoIt->ccName; };
    static const char* GetValue( iterator aoIt )              { return aoIt->ccValue; };
    

protected:    
    bool ReplaceAttr( vector<stAttribute>& aoList, const char* apsName, const char* apsValue )
    {
        for( size_t liIdx = 0; liIdx < aoList.size(); liIdx++ )
        {
            if ( ::strcmp( aoList[liIdx].ccName, apsName ) == 0 )
            {
                AIBC::StringCopy( aoList[liIdx].ccValue, apsValue, sizeof(aoList[liIdx].ccValue) );
                return true;
            }
        }
        return false;
    }

protected:
    char csEnvelope[20];
    char csHeader[20];
    char csBody[20];
    
    vector<stAttribute> cvoEnvelopeAttr;
    vector<stAttribute> cvoHeaderAttr;
    vector<stAttribute> cvoBodyAttr;
};

///////////////////////////////////////WSDLRequest//////////////////////////////////////////
class clsWSDLRequest
{   
public:
    clsWSDLRequest( clsSoapEnvelope* apoSoapEnv ) : 
        cpoRootElement(NULL),
        cpoSoapEnvlope(NULL),
        cpoSoapHeader(NULL),
        cpoSoapBody(NULL),
        cpoSoapEnv(apoSoapEnv)
    {
        ::memset( this->ccRequestName, 0, sizeof(this->ccRequestName) );
        this->cpcRequestNameNns = this->ccRequestName;
    }
    
    ~clsWSDLRequest()
    {
        // Recycle Mcb Root Element
        if ( this->cpoRootElement != NULL )
        {
            AIBC::McbDeleteRoot( this->cpoRootElement );
            this->cpoRootElement = NULL;
        }
    }

    int operator () ( const char* apsXML, int* apiLine = NULL, int* apiColumn = NULL )
    {
        if ( this->cpoRootElement != NULL )
        {
            // Do again, and clear last one
            this->Clear();
        }
        
        // Parse xml
        this->cpoRootElement = AIBC::McbParseXML( apsXML, &ciStat );
        if ( apiLine != NULL ) *apiLine     = ciStat.ciLine;
        if ( apiColumn != NULL ) *apiColumn = ciStat.ciColumn;
        AI_RETURN_IF( AISOAP::AI_ERROR_PARSE_XML, (cpoRootElement == NULL) );
        
        // Soap?
        if ( this->cpoSoapEnv == NULL )
        {
            FindRequestName( this->cpoRootElement );
            return AISOAP::AI_NO_ERROR;
        }
        
        // Check soap envelope
        this->cpoSoapEnvlope = this->cpoSoapEnv->DecodeSoapEnvelope( this->cpoRootElement );
        AI_RETURN_IF( AISOAP::AI_ERROR_SOAP_ENV, (cpoSoapEnvlope == NULL) );
        
        // Check soap header
        this->cpoSoapHeader = this->cpoSoapEnv->DecodeSoapHeader( this->cpoSoapEnvlope );
        AI_RETURN_IF( AISOAP::AI_ERROR_SOAP_HEADER, (cpoSoapHeader == NULL) );
        
        // Check soap body
        this->cpoSoapBody = this->cpoSoapEnv->DecodeSoapBody( this->cpoSoapEnvlope );
        AI_RETURN_IF( AISOAP::AI_ERROR_SOAP_BODY, (this->cpoSoapBody == NULL) );
        
        FindRequestName( this->cpoSoapBody );
        
        return AISOAP::AI_NO_ERROR;
    }
    
    void Clear()
    {
        // Clear envelope
        if ( this->cpoSoapEnv != NULL )
        {
            this->cpoSoapEnv->Clear();
        }
        
        // Recycle Mcb Root Element
        if ( this->cpoRootElement != NULL )
        {
            AIBC::McbDeleteRoot( this->cpoRootElement );
            this->cpoRootElement = NULL;
        }
        
        this->cpoRootElement = NULL;
        this->cpoSoapEnvlope = NULL;
        this->cpoSoapHeader  = NULL;
        this->cpoSoapBody    = NULL;
        
        ::memset( this->ccRequestName, 0, sizeof(this->ccRequestName) );
        this->cpcRequestNameNns = this->ccRequestName;
    }
    
    clsSoapEnvelope* GetSoapEnvelope()                          { return cpoSoapEnv; }
    AIBC::McbXMLElement* GetRootElement()                       { return cpoRootElement; }
    AIBC::McbXMLElement* GetEnvelope()                          { return cpoSoapEnvlope; }
    AIBC::McbXMLElement* GetHeader()                            { return cpoSoapHeader; }
    AIBC::McbXMLElement* GetBody()                              { return cpoSoapBody; }
    
    const AIBC::McbXMLElement* GetRootElement() const           { return cpoRootElement; }
    const AIBC::McbXMLElement* GetEnvelope()    const           { return cpoSoapEnvlope; }
    const AIBC::McbXMLElement* GetHeader()      const           { return cpoSoapHeader; }
    const AIBC::McbXMLElement* GetBody()        const           { return cpoSoapBody; }
    
    const char* What()const                                     { return ccRequestName; }
    const char* WhatNns()const                                  { return cpcRequestNameNns; }

protected:
    void FindRequestName( AIBC::McbXMLElement* apoElement )
    {
        int liIdx = 0;
        AIBC::McbXMLElement* lpoSubElement = McbEnumElements( apoElement, &liIdx );
        if ( lpoSubElement != NULL )
        {
            AIBC::StringCopy( this->ccRequestName, lpoSubElement->cpcName, sizeof(this->ccRequestName) );
            this->cpcRequestNameNns = ::strchr( this->ccRequestName, ':' );
            if ( this->cpcRequestNameNns == NULL )
            {
                this->cpcRequestNameNns = this->ccRequestName;
            }
            else
            {
                this->cpcRequestNameNns++;
            }
        }
    }
    
protected:
    AIBC::McbXMLElement* cpoRootElement;
    AIBC::McbXMLElement* cpoSoapEnvlope;
    AIBC::McbXMLElement* cpoSoapHeader;
    AIBC::McbXMLElement* cpoSoapBody;
    AIBC::McbXMLResults  ciStat;
    
    char ccRequestName[AISOAP::AI_MAX_FIELD_LEN];
    const char* cpcRequestNameNns;
    
    clsSoapEnvelope* cpoSoapEnv;
};

////////////////////////////////////////////////////////////////////////////////////////
class clsBaseWSDL
{
public:
    enum //Escape character
    {
        DO_ESCAPE = 1,
        DONT_ESCAPE = 0
    };
    
public:
    clsBaseWSDL( clsSoapEnvelope* apoEnv = NULL, int aiOpt = DONT_ESCAPE ) 
        : cpoSoapEnvelope(apoEnv)
        , coResult(AISOAP::AI_MAX_XML_LEN)
        , ciOpt(aiOpt)
    {
        ::memset( this->ccCurrElementName, 0, sizeof(this->ccCurrElementName) );
        AIBC::StringCopy( this->ccXmlHeader, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", sizeof(this->ccXmlHeader) );
    }
    
    virtual ~clsBaseWSDL()
    {
    }
    
    // operation
    virtual const char* EncodeXMLString() = 0;
    virtual int         DecodeXMLString( clsWSDLRequest& apoRequest ) = 0;
    virtual int         DecodeXMLString( const char* apsXMLString )   = 0;

    const char* GetCurrElement()
    {
        return this->ccCurrElementName;
    }
    
protected:
    AIBC::McbXMLElement* EncodeXmlHeader( AIBC::McbXMLElement* apoCurrElem )
    {
        return apoCurrElem;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////
    AIBC::McbXMLElement* DecodeXmlHeader( AIBC::McbXMLElement* apoCurrElem )
    {
        return apoCurrElem;
    }
    
    clsSoapEnvelope* GetSoapEnvelope()
    {
        return this->cpoSoapEnvelope;
    }
    
    void SetCurrElement( const char* apsName )
    {
        AIBC::StringCopy( this->ccCurrElementName, apsName, sizeof(this->ccCurrElementName) );
    }
    
    int CreateXMLString( AIBC::McbXMLElement *apoHead )
    {
        int liFormat = 0;
        int liLength = 0;
        int liHeadLen = ::strlen(this->ccXmlHeader);
    
        this->coResult.BasePtr()[0] = '\0';
        
        AI_RETURN_IF( AISOAP::AI_ERROR_GEN_XML, apoHead == NULL );
        
        liLength = AIBC::McbCreateXMLStringR( apoHead, NULL, liFormat );
        AI_RETURN_IF( AISOAP::AI_ERROR_GEN_XML, liLength <= 0 );
        
        this->coResult.Resize( liHeadLen + liLength + 1 );
        ::memcpy( this->coResult.BasePtr(), this->ccXmlHeader, liHeadLen );
    
        AIBC::McbCreateXMLStringR(apoHead, this->coResult.BasePtr() + liHeadLen, liFormat );
        
        return AISOAP::AI_NO_ERROR;
    }
    
    char* ResultPointer() { return coResult.BasePtr(); }

public:
    ///////////////////////////////////// Assign function ///////////////////////////////////////
    void Assign( char* apsL, const char* apsR )
    {
        const char* lpsR = ( apsR == NULL ? "" : apsL );
        
        if (this->ciOpt == DO_ESCAPE)
        {
            this->coEscape.Decode(lpsR, apsL);
        }
        else
        {
            ::strcpy( apsL, lpsR );
        }
    }
    
    void Assign( string& aosL, const char* apsR )
    {
        const char* lpsR = ( apsR == NULL ? "" : apsR );
        
        if (this->ciOpt == DO_ESCAPE)
        {
            AIBC::AIChunkEx loBuffer( strlen(lpsR) + 1 );
            this->coEscape.Decode(lpsR, loBuffer.BasePtr() );
            aosL = loBuffer.BasePtr();
        }
        else
        {
            aosL = lpsR;
        }
    }
    
    void Assign( AIBC::AIChunkEx& aoL, const char* apsR )
    {
        const char* lpsR  = ( apsR == NULL ? "" : apsR );
        size_t      liLen = ::strlen(lpsR);
        
        aoL.Resize( liLen );
        size_t liSize = AIBC::BASE64Decode( lpsR, liLen, aoL.BasePtr(), aoL.GetSize() );
        
        aoL.Resize(liSize);
    }
    
    void Assign( AIBC::AIInt& aiL, const char* apsR )
    {
        if ( apsR != NULL )
        {
            aiL = AIBC::StringAtoi(apsR);
        }
    }
    
    #if !defined(_AI_OLD_VERSION)
    void Assign( AIBC::AILong& aiL, const char* apsR )
    {
        if ( apsR != NULL )
        {
            aiL = ::atoll(apsR);
        }
    }
    #endif
    
    void Assign( bool& aiL, const char* apsR )
    {
        if ( apsR != NULL )
        {
            aiL = ( apsR[0] == '1' ? true : false );
        }
    }

    ///////////////////////////////// Other type convert to string ////////////////////////////////////
    char* ToString( const char* apsR )
    {
        char*       lpsValue = NULL;
        const char* lpsR     = (apsR == NULL? "" : apsR);
        size_t      liLen    = 0;
        
        if (this->ciOpt == DO_ESCAPE)
        {
            liLen = this->coEscape.Encode(lpsR, NULL);
            AI_MALLOC_ASSERT( lpsValue, char, liLen + 1 );
            this->coEscape.Encode(lpsR, lpsValue);
        }
        else
        {
            liLen = ::strlen(lpsR);
            AI_MALLOC_ASSERT( lpsValue, char, liLen + 1 );
            ::memcpy(lpsValue, apsR, liLen + 1);
        }

        return lpsValue;
    }
    
    char* ToString( const char* apsNameSpace, const char* apsR )
    {
        char*       lpsValue  = NULL;
        const char* lpsR      = (apsR == NULL? "" : apsR);
        size_t      liNeedLen = strlen(apsNameSpace) + strlen(lpsR) + 2;
        
        AI_MALLOC_ASSERT( lpsValue, char, liNeedLen );
         
        TailNamespace( apsNameSpace, lpsR, lpsValue, liNeedLen );
         
        return lpsValue;
    }
    
    char* ToString( const char* apsNameSpace, string& aosR )
    {     
        return ToString( apsNameSpace, aosR.c_str() );
    }
    
    char* ToString( const string& aosR )
    {
        char*  lpsValue = NULL;
        
        if (this->ciOpt == DO_ESCAPE)
        {
            size_t liLen = this->coEscape.Encode(aosR.c_str(), NULL);
            AI_MALLOC_ASSERT( lpsValue, char, liLen + 1 );
            this->coEscape.Encode(aosR.c_str(), lpsValue);
        }
        else
        {
            AI_MALLOC_ASSERT( lpsValue, char, aosR.size() + 1 );
            ::memcpy( lpsValue, aosR.c_str(), aosR.size() + 1 );
        }
         
        return lpsValue;
    }
    
    char* ToString( const AIBC::AIInt& aiR )
    {
        char* lpsValue = NULL;
        
        AI_MALLOC_ASSERT( lpsValue, char, 20 );
        
        if ( aiR == AIBC::AIInt::nil )
            ::memset( lpsValue, 0, 20 );
        else
            ::snprintf( lpsValue, 20, "%" PRId32 "", (int32_t)aiR );
        
        return lpsValue;
    }
    
    #if !defined(_AI_OLD_VERSION)
    char* ToString( const AIBC::AILong& aiR )
    {
        char* lpsValue = NULL;
        
        AI_MALLOC_ASSERT( lpsValue, char, 30 );
        
        if ( aiR == AIBC::AILong::nil )
            ::memset( lpsValue, 0, 30 );
        else
            ::snprintf( lpsValue, 30, "%" PRId64 "", (int64_t)aiR );
        
        return lpsValue;
    }
    #endif
    
    char* ToString( bool abR )
    {
        char* lpsValue = NULL;
        
        AI_MALLOC_ASSERT( lpsValue, char, 2 );
        
        lpsValue[0] = ( abR ? '1' : '0');
        lpsValue[1] = '\0';
        
        return (char*)(lpsValue);
    }
    
    char* ToString( const AIBC::AIChunkEx& aoR )
    {
        char* lpsValue = NULL;
        AI_MALLOC_ASSERT( lpsValue, char, BASE64_CALC_ENCODE_SIZE(aoR.GetSize()) );
        
        AIBC::BASE64Encode( aoR.BasePtr(), aoR.GetSize(), lpsValue, BASE64_CALC_ENCODE_SIZE(aoR.GetSize()) );
        
        return (char*)(lpsValue);
    }

protected:
    clsSoapEnvelope* cpoSoapEnvelope;
    
    char ccXmlHeader[AISOAP::AI_MAX_XML_HEADER_LEN];
        
    AIBC::AIChunkEx coResult;
        
    char ccCurrElementName[AISOAP::AI_MAX_FIELD_LEN];
    
    clsStringEscape coEscape;
    
    int ciOpt;
};

#define AI_GOTO_IF_EX( to, exp, do ) if(exp) { do; goto to; }

#endif //__AI_BASEWSDL_OBJECT__
