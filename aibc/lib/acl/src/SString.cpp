
#include "acl/SString.h"

ACL_NAMESPACE_START

CTokenizer::CTokenizer(void)
    : mbIsSkipBlankField(false)
    , moFields(100, (char*)NULL)
    , muFieldCount(0)
{
}

CTokenizer::CTokenizer( char const* apcDelimiter, bool abIsSkipBlankField )
    : mbIsSkipBlankField(false)
    , moFields(100, (char*)NULL)
    , muFieldCount(0)
{
    this->Delimiter(apcDelimiter, abIsSkipBlankField);
}

CTokenizer::~CTokenizer(void)
{
}

void CTokenizer::Delimiter( char const* apcDelimiter, bool abIsSkipBlankField )
{
    this->moDelimiter = apcDelimiter;

    this->mbIsSkipBlankField = abIsSkipBlankField ;
}

apl_int_t CTokenizer::Preserve( char acStart, char acStop, bool abIsStrip )
{
    CPreserve loPreserve;
    
    loPreserve.mcStart = acStart;
    loPreserve.mcStop = acStop;
    loPreserve.mbIsStrip = abIsStrip;
    
    this->moPreserves.push_back(loPreserve);
    
    return 0;
}

apl_ssize_t CTokenizer::Parse( char* apcInput )
{
    char  lcStop = 0;
    bool  lbIsStrip = false;
    bool  lbIsDualDelimiter = false;
    char* lpcFirst = apcInput;
    char* lpcLast  = lpcFirst;
    
    this->muFieldCount = 0;
    
    while (*lpcLast != '\0')
    {
        if ( this->IsDelimiter(lpcLast) )
        {
            *lpcLast = '\0';
            
            if (lpcLast == lpcFirst)
            {
                if (!this->mbIsSkipBlankField && lbIsDualDelimiter)
                {
                    this->GetToken(this->muFieldCount++, lpcFirst, lpcLast - lpcFirst);
                }
                
                //Skip delimiter
                lpcLast += this->moDelimiter.length();
                lpcFirst += this->moDelimiter.length();
            }
            else
            {
                //Get token
                this->GetToken(this->muFieldCount++, lpcFirst, lpcLast - lpcFirst);
                lpcFirst = lpcLast + this->moDelimiter.length();
                lpcLast = lpcFirst;
            }
            
            lbIsDualDelimiter = true;
            
            continue;
        }
        else if ( this->IsPreserve(*lpcLast, lcStop, lbIsStrip) )
        {
            lbIsDualDelimiter = false;
            
            if (lpcLast != lpcFirst)
            {
                //Get last token
                this->GetToken(this->muFieldCount++, lpcFirst, lpcLast - lpcFirst);
                lpcFirst = lpcLast;
            }
            
            while( *(++lpcLast) != '\0' && *lpcLast != lcStop ) {};
            
            if ( *lpcLast == '\0')
            {
                if (lbIsStrip)
                {
                    *lpcFirst = '\0';
                    ++lpcFirst;
                }
                
                continue;
            }
            
            if (lbIsStrip)
            {
                *lpcFirst = '\0';
                *lpcLast = '\0';
                
                ++lpcFirst;
                
                //Get last token
                this->GetToken(this->muFieldCount++, lpcFirst, lpcLast - lpcFirst - 1);
                lpcFirst = lpcLast + 1;
                lpcLast = lpcFirst;
            }
            else
            {
                //Get last token
                this->GetToken(this->muFieldCount++, lpcFirst, lpcLast - lpcFirst + 1);
                lpcFirst = lpcLast + 1;
                lpcLast = lpcFirst;
            }
            
            continue;
        }
        
        lbIsDualDelimiter = false;
        lpcLast++;
    }
    
    if (lpcLast != lpcFirst)
    {
        //Get last token
        this->GetToken(this->muFieldCount++, lpcFirst, lpcLast - lpcFirst);
    }
    
    return this->muFieldCount;
}

bool CTokenizer::IsDelimiter( char const* apcInput ) const
{
    if (this->moDelimiter.length() > 1)
    {
        return apl_strncmp( apcInput, this->moDelimiter.c_str(), this->moDelimiter.length() ) == 0 ? true : false;
    }
    else
    {
        return apcInput[0] == this->moDelimiter[0] ? true : false;
    }
}
    
bool CTokenizer::IsPreserve( char acStart, char& acStop, bool& abIsStrip ) const
{
    for ( apl_size_t liN = 0; liN < this->moPreserves.size(); liN++ )
    {
        if (this->moPreserves[liN].mcStart == acStart)
        {
            acStop = this->moPreserves[liN].mcStop;
            abIsStrip = this->moPreserves[liN].mbIsStrip;
            return true;
        }
    }
    
    return false;
}

void CTokenizer::GetToken(apl_size_t aiFieldID, char* apcFirst, apl_size_t aiLen)
{
    if ( aiFieldID >= this->moFields.size() )
    {
        this->moFields.resize(aiFieldID * 2);
    }
    
    this->moFields[aiFieldID] = apcFirst;
}


apl_size_t CTokenizer::GetSize() const
{
    return this->muFieldCount; //this->moFields.size();
}

char const* CTokenizer::GetField( apl_size_t aiN )
{
    return aiN < this->moFields.size() ? this->moFields[aiN] : NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////
CSpliter::CSpliter(void)
    : mbIsSkipBlankField(false)
    , moFields(100)
    , muFieldCount(0)
{
}

CSpliter::CSpliter( char const* apcDelimiter, bool abIsSkipBlankField )
    : mbIsSkipBlankField(false)
    , moFields(100)
    , muFieldCount(0)
{
    this->Delimiter(apcDelimiter, abIsSkipBlankField);
}

CSpliter::~CSpliter(void)
{
}

void CSpliter::Delimiter( char const* apcDelimiter, bool abIsSkipBlankField )
{
    this->moDelimiter = apcDelimiter;

    this->mbIsSkipBlankField = abIsSkipBlankField ;
}

apl_int_t CSpliter::Preserve( char acStart, char acStop, bool abIsStrip )
{
    CPreserve loPreserve;
    
    loPreserve.mcStart = acStart;
    loPreserve.mcStop = acStop;
    loPreserve.mbIsStrip = abIsStrip;
    
    this->moPreserves.push_back(loPreserve);
    
    return 0;
}

apl_ssize_t CSpliter::Parse( char const* apcInput )
{
    char        lcStop = 0;
    bool        lbIsStrip = false;
    bool        lbIsDualDelimiter = false;
    char const* lpcFirst = apcInput;
    char const* lpcLast  = lpcFirst;
    
    this->muFieldCount = 0;
    
    while (*lpcLast != '\0')
    {
        if ( this->IsDelimiter(lpcLast) )
        {
            if (lpcLast == lpcFirst)
            {
                if (!this->mbIsSkipBlankField && lbIsDualDelimiter)
                {
                    this->GetToken(this->muFieldCount++, lpcFirst, lpcLast - lpcFirst);
                }
                
                //Skip delimiter
                lpcLast += this->moDelimiter.length();
                lpcFirst += this->moDelimiter.length();
            }
            else
            {
                //Get token
                this->GetToken(this->muFieldCount++, lpcFirst, lpcLast - lpcFirst);
                lpcFirst = lpcLast + this->moDelimiter.length();
                lpcLast = lpcFirst;
            }
            
            lbIsDualDelimiter = true;
            
            continue;
        }
        else if ( this->IsPreserve(*lpcLast, lcStop, lbIsStrip) )
        {
            lbIsDualDelimiter = false;
            
            if (lpcLast != lpcFirst)
            {
                //Get last token
                this->GetToken(this->muFieldCount++, lpcFirst, lpcLast - lpcFirst);
                lpcFirst = lpcLast;
            }
            
            while( *(++lpcLast) != '\0' && *lpcLast != lcStop ) {};
            
            if ( *lpcLast == '\0')
            {
                if (lbIsStrip)
                {
                    ++lpcFirst;
                }
                
                continue;
            }
            
            if (lbIsStrip)
            {
                ++lpcFirst;
                
                //Get last token
                this->GetToken(this->muFieldCount++, lpcFirst, lpcLast - lpcFirst);
                lpcFirst = lpcLast + 1;
                lpcLast = lpcFirst;
            }
            else
            {
                //Get last token
                this->GetToken(this->muFieldCount++, lpcFirst, lpcLast - lpcFirst + 1);
                lpcFirst = lpcLast + 1;
                lpcLast = lpcFirst;
            }
            
            continue;
        }
        
        lbIsDualDelimiter = false;
        lpcLast++;
    }
    
    if (lpcLast != lpcFirst)
    {
        //Get last token
        this->GetToken(this->muFieldCount++, lpcFirst, lpcLast - lpcFirst);
    }
    
    return this->muFieldCount;
}

apl_ssize_t CSpliter::Parse( std::string const& aoStr )
{
    return this->Parse( aoStr.c_str() );
}

bool CSpliter::IsDelimiter( char const* apcInput ) const
{
    if (this->moDelimiter.length() > 1)
    {
        return apl_strncmp( apcInput, this->moDelimiter.c_str(), this->moDelimiter.length() ) == 0 ? true : false;
    }
    else
    {
        return apcInput[0] == this->moDelimiter[0] ? true : false;
    }
}
    
bool CSpliter::IsPreserve( char acStart, char& acStop, bool& abIsStrip ) const
{
    for ( apl_size_t liN = 0; liN < this->moPreserves.size(); liN++ )
    {
        if (this->moPreserves[liN].mcStart == acStart)
        {
            acStop = this->moPreserves[liN].mcStop;
            abIsStrip = this->moPreserves[liN].mbIsStrip;
            return true;
        }
    }
    
    return false;
}

void CSpliter::GetToken( apl_size_t aiFieldID, char const* apcFirst, apl_size_t aiLen )
{
    if ( aiFieldID >= this->moFields.size() )
    {
        this->moFields.resize(aiFieldID * 2);
    }
    
    this->moFields[aiFieldID].assign(apcFirst, aiLen);
}


apl_size_t CSpliter::GetSize() const
{
    return this->muFieldCount;
}

char const* CSpliter::GetField( apl_size_t aiN )
{
    return aiN < this->moFields.size() ? this->moFields[aiN].c_str() : NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////
CFixSpliter::CFixSpliter( apl_size_t* apiFixWidth, apl_size_t aiFieldCnt )
    : moFieldWidths( apiFixWidth, apiFixWidth + aiFieldCnt )
    , muTotalLen(0)
    , moFields(aiFieldCnt)
{
    for ( apl_size_t liN = 0; liN < aiFieldCnt; liN++ )
    {
        this->muTotalLen += apiFixWidth[liN];
    }
}

CFixSpliter::~CFixSpliter(void)
{
}

apl_ssize_t CFixSpliter::Parse( char const* apcInput )
{
    return this->Parse( apcInput, apl_strlen(apcInput) );
}

apl_ssize_t CFixSpliter::Parse( std::string const& aoInput )
{
    return this->Parse( aoInput.c_str(), aoInput.length() );
}

apl_ssize_t CFixSpliter::Parse( char const* apcInput, apl_size_t aiLen )
{
    apl_size_t liCurrIndex = 0;
    
    if (this->muTotalLen > aiLen)
    {
        return -1;
    }
    
    for ( apl_size_t liN = 0; liN < this->moFieldWidths.size(); liN++ )
    {
        this->moFields[liN].assign(apcInput + liCurrIndex, this->moFieldWidths[liN]);
        liCurrIndex += this->moFieldWidths[liN];
    }
    
    return this->moFields.size();
}


apl_size_t CFixSpliter::GetSize() const
{
    return this->moFields.size();
}

char const* CFixSpliter::GetField( apl_size_t aiN )
{
    return aiN < this->moFields.size() ? this->moFields[aiN].c_str() : NULL;
}

ACL_NAMESPACE_END
