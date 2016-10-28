
#include "acl/Config.h"
#include "acl/File.h"
#include "acl/Regex.h"
#include "acl/StrAlgo.h"

ACL_NAMESPACE_START

bool IsEraseChar( char c )
{
    return ( c == ' ' || c == '\t' || c == '\'' || c == '\"' ) ? true : false;
}

CBaseConfig::KeyIterType CBaseConfig::GetValue( char const* apcField, char const* apcKey, const char* apcDefault )
{
    apl_ssize_t liFieldID = -1;
    apl_ssize_t liKeyID   = -1; 
 
    liFieldID = this->FindField(apcField);
    if (liFieldID < 0)
    {
        return KeyIterType(this, liFieldID, liKeyID, apcDefault);
    }
    
    liKeyID = this->FindKey(liFieldID, apcKey);
    if (liKeyID < 0)
    {
        return KeyIterType(this, liFieldID, liKeyID, apcDefault);
    }
    
    return KeyIterType(this, liFieldID, liKeyID, apcDefault);
}

CBaseConfig::KeyIterType CBaseConfig::GetValue( char const* apcField, apl_size_t aiIndex, const char* apcDefault )
{
    apl_ssize_t liFieldID = this->FindField(apcField);
    if (liFieldID < 0)
    {
        return KeyIterType(this, -1, -1, apcDefault);
    }
    
    if ( aiIndex >= this->moFields[liFieldID].moKeys.size() )
    {
        return KeyIterType(this, -1, -1, apcDefault);
    }
    
    return KeyIterType( this, liFieldID, aiIndex, apcDefault );
}

apl_size_t CBaseConfig::GetKeyCount( char const* apcField )
{
    apl_ssize_t liFieldID = this->FindField(apcField);
    if (liFieldID == -1)
    {
        return 0;
    }
    else
    {
        return this->GetKeySize(liFieldID);
    }
}

apl_size_t CBaseConfig::GetFieldCount( char const* apcFieldPattern )
{
    if (apcFieldPattern == NULL)
    {
        return this->moFields.size();
    }
    else
    {
        apl_size_t liSize = 0;
        
        for( apl_ssize_t liN = this->FindField(0, apcFieldPattern); 
            liN >= 0; liN = this->FindField(liN + 1, apcFieldPattern) )
        {
            liSize++;
        }
        
        return liSize;
    }
}

apl_int_t CBaseConfig::SetValue( char const* apcField, char const* apcKey, apl_int_t aiValue, bool abIsCreate )
{
    char lacBuffer[64];
    apl_snprintf( lacBuffer, sizeof(lacBuffer), "%"PRId64, (apl_int64_t)aiValue );
    
    return this->SetValue(apcField, apcKey, lacBuffer, abIsCreate);
}
    
apl_int_t CBaseConfig::SetValue( char const* apcField, char const* apcKey, bool abValue,bool abIsCreate )
{
    char lacBuffer[64];
    apl_snprintf( lacBuffer, sizeof(lacBuffer), "%"PRId32, (apl_int32_t)abValue );
    
    return this->SetValue(apcField, apcKey, lacBuffer, abIsCreate);
}

apl_int_t CBaseConfig::SetValue( char const* apcField, char const* apcKey, double adValue, bool abIsCreate )
{
    char lacBuffer[64];
    apl_snprintf( lacBuffer, sizeof(lacBuffer), "%lf", adValue );
    
    return this->SetValue(apcField, apcKey, lacBuffer, abIsCreate);
}

apl_int_t CBaseConfig::SetValue( char const* apcField, char const* apcKey, char const* apcValue, bool abIsCreate )
{
    apl_ssize_t liFieldID = -1;
    apl_ssize_t liKeyID   = -1; 
    
    liFieldID = this->FindField( apcField );
    if (liFieldID < 0)
    {
        if (abIsCreate)
        {
            liFieldID = this->AddField(apcField);
        }
        else
        {
            return -1;
        }
    }
    
    liKeyID = this->FindKey(liFieldID, apcKey);
    if (liKeyID < 0)
    {
        if (abIsCreate)
        {
            liKeyID = this->AddKey(liFieldID, apcKey, apcValue);
            
            return 0;
        }
        else
        {
            return -1;
        }
    }
    
    this->moFields[liFieldID].moKeys[liKeyID].moValue = apcValue;
    
    return 0;
}

apl_int_t CBaseConfig::AddComment( char const* apcField,  char const* apcKey, char const* apcComment )
{
    apl_ssize_t liFieldID = -1;
    apl_ssize_t liKeyID = -1;
    CommentType loComment;
    
    liFieldID = this->FindField(apcField);
    if (liFieldID < 0)
    {
        return -1;
    }
    
    liKeyID = this->FindKey(liFieldID, apcKey);
    if (liKeyID < 0)
    {
        return -1;
    }
    
    loComment.miFieldID = liFieldID;
    loComment.miKeyID = liKeyID;
    loComment.moComment = apcComment;
    
    this->moComments.push_back(loComment);
    
    return 0;
}
        
apl_int_t CBaseConfig::AddComment( char const* apcField, char const* apcComment )
{
    apl_ssize_t liFieldID = -1;
    CommentType loComment;
    
    liFieldID = this->FindField(apcField);
    if (liFieldID < 0)
    {
        return -1;
    }
    
    loComment.miFieldID = liFieldID;
    loComment.miKeyID = -1;
    loComment.moComment = apcComment;
    
    this->moComments.push_back(loComment);
    
    return 0;
}
        
apl_ssize_t CBaseConfig::DelComment( char const* apcField, char const* apcKey )
{
    apl_size_t  liErases = 0;
    apl_ssize_t liFieldID = -1;
    apl_ssize_t liKeyID = -1;
    CommentType loComment;
    
    liFieldID = this->FindField(apcField);
    if (liFieldID < 0)
    {
        return -1;
    }
    
    liKeyID = this->FindKey(liFieldID, apcKey);
    if (liKeyID < 0)
    {
        return -1;
    }
    
    for ( std::vector<CommentType>::iterator loIter = this->moComments.begin();
          loIter != this->moComments.end();  )
    {
        if ( loIter->miFieldID == liFieldID
             && loIter->miKeyID == liKeyID )
        {
            loIter = this->moComments.erase(loIter);
            liErases++;
        }
        else
        {
            ++loIter;
        }
    }
    
    return liErases;
}
    
apl_ssize_t CBaseConfig::DelComment( char const* apcField )
{
    apl_size_t  liErases = 0;
    apl_ssize_t liFieldID = -1;
    CommentType loComment;
    
    liFieldID = this->FindField(apcField);
    if (liFieldID < 0)
    {
        return -1;
    }
    
    for ( std::vector<CommentType>::iterator loIter = this->moComments.begin();
          loIter != this->moComments.end();  )
    {
        if ( loIter->miFieldID == liFieldID
             && loIter->miKeyID == -1 )
        {
            loIter = this->moComments.erase(loIter);
            liErases++;
        }
        else
        {
            ++loIter;
        }
    }
    
    return liErases;
}

CBaseConfig::FieldIterType CBaseConfig::Begin( char const* apcFieldPattern )
{
    return CBaseConfig::FieldIterType(
        this,
        ( apcFieldPattern == NULL ? 
            (this->moFields.begin() == this->moFields.end() ? -1 : 0) 
            : this->FindField(0, apcFieldPattern) ),
        apcFieldPattern );
}
    
CBaseConfig::FieldIterType CBaseConfig::End(void)
{
    return CBaseConfig::FieldIterType(this, -1, NULL);
}

CBaseConfig::KeyType& CBaseConfig::GetKey( apl_size_t aiFieldID, apl_size_t aiKeyID )
{
    return this->moFields[aiFieldID].moKeys[aiKeyID];
}

apl_size_t CBaseConfig::GetKeySize( apl_size_t aiFieldID )
{
    return this->moFields[aiFieldID].moKeys.size();
}

CBaseConfig::FieldType& CBaseConfig::GetField( apl_size_t aiFieldID )
{
    return this->moFields[aiFieldID];
}

apl_size_t CBaseConfig::GetFieldSize(void)
{
    return this->moFields.size();
}

CBaseConfig::CommentType& CBaseConfig::GetComment( apl_size_t aiCommentID )
{
    return this->moComments[aiCommentID];
}

apl_size_t CBaseConfig::GetCommentSize(void)
{
    return this->moComments.size();
}

apl_ssize_t CBaseConfig::FindField( const char* apcField )
{
    for( apl_size_t liN = 0; liN < this->moFields.size(); liN++ )
    {
        if ( this->Compare(this->moFields[liN].moName.c_str(), apcField) )
        {
            return liN;
        }
    }
    
    return -1;
}

apl_ssize_t CBaseConfig::FindField( apl_size_t aiPos, char const* apcFieldPattern )
{
    for( apl_size_t liN = aiPos; liN < this->moFields.size(); liN++ )
    {
        if ( this->Match( apcFieldPattern, this->moFields[liN].moName.c_str() ) )
        {
            return liN;
        }
    }
    
    return -1;
}
    
apl_ssize_t CBaseConfig::FindKey( apl_size_t aiFieldID, const char* apcKey )
{
    for( apl_size_t liN = 0; liN < this->moFields[aiFieldID].moKeys.size(); liN++ )
    {
        if ( this->Compare(this->moFields[aiFieldID].moKeys[liN].moKey.c_str(), apcKey) )
        {
            return liN;
        }
    }
    
    return -1;
}
    
apl_ssize_t CBaseConfig::AddField( const char* apcField )
{
    CField loField;
    
    loField.moName = apcField;
    
    stralgo::TrimIf(loField.moName, IsEraseChar);
    
    apl_ssize_t liFieldID = this->FindField(apcField);
    if (liFieldID < 0)
    {
        this->moFields.push_back(loField);
    
        return this->moFields.size() - 1;
    }
    else
    {
        return liFieldID;
    }
    
}
    
apl_ssize_t CBaseConfig::AddKey( apl_size_t aiFieldID, const char* apcKey, const char* apcValue )
{
    CKey loKey;
    
    loKey.moKey = apcKey;
    loKey.moValue = apcValue;
    
    stralgo::TrimIf(loKey.moKey, IsEraseChar);
    stralgo::TrimIf(loKey.moValue, IsEraseChar);
        
    apl_ssize_t liKeyID = this->FindKey(aiFieldID, apcKey);
    if (liKeyID < 0)
    {
        this->moFields[aiFieldID].moKeys.push_back(loKey);
    
        return this->moFields[aiFieldID].moKeys.size() - 1;
    }
    else
    {
        this->moFields[aiFieldID].moKeys[liKeyID].moValue = loKey.moValue;
        return liKeyID;
    }
}

void CBaseConfig::AddComment( apl_ssize_t aiFieldID, apl_ssize_t aiKeyID, const char* apcComment )
{
    CComment loComment;
    
    loComment.miFieldID = aiFieldID;
    loComment.miKeyID = aiKeyID;
    loComment.moComment = apcComment;
    
    this->moComments.push_back(loComment);
}

void CBaseConfig::FixComment( apl_ssize_t aiFieldID, apl_ssize_t aiKeyID )
{
    for ( std::vector<CComment>::reverse_iterator loIter = this->moComments.rbegin();
        loIter != this->moComments.rend(); loIter++ )
    {
        if ( loIter->miFieldID == -1 )
        {
            loIter->miFieldID = aiFieldID;
            loIter->miKeyID = aiKeyID;
        }
        else
        {
            break;
        }
    }
}

bool CBaseConfig::Compare( const char* apcLhs, const char* apcRhs )
{
    if ( apl_strcmp(apcLhs, apcRhs) == 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CBaseConfig::Match( char const* apcFieldPattern, char const* apcTest )
{
    CRegex loRegex;
        
    if ( loRegex.Compile(apcFieldPattern) != 0 )
    {
        return false;
    }
    
    return loRegex.Match(apcTest);
}

void CBaseConfig::Clear(void)
{
    this->moComments.clear();
    this->moFields.clear();
}

///////////////////////////////////////////////////////////////////////////////////////
CBaseConfig::CKeyIterator::CKeyIterator(
    CBaseConfig* apoConfig,
    apl_ssize_t aiFieldID,
    apl_ssize_t aiKeyID,
    const char* apcDefault )
    : mpoConfig(apoConfig)
    , miFieldID(aiFieldID)
    , miKeyID(aiKeyID)
    , mpcDefault(apcDefault)
{
}

bool CBaseConfig::CKeyIterator::operator == ( CBaseConfig::CKeyIterator const& aoRhs )
{
    return this->miKeyID == aoRhs.miKeyID;
}
        
bool CBaseConfig::CKeyIterator::operator != ( CBaseConfig::CKeyIterator const& aoRhs )
{
    return !(*this == aoRhs);
}
        
CBaseConfig::CKeyIterator& CBaseConfig::CKeyIterator::operator ++ (void)
{
    if ( !this->IsEmpty() )
    {
        this->miKeyID++;
    
        if ( this->miKeyID >= (apl_ssize_t)this->mpoConfig->GetKeySize(this->miFieldID) )
        {
            this->miKeyID = -1;
        }
    }
    
    return *this;
}

bool CBaseConfig::CKeyIterator::IsEmpty(void)
{
    return this->miKeyID < 0 || this->miFieldID < 0;
}

char const* CBaseConfig::CKeyIterator::GetName(void)
{
    return this->IsEmpty() ? "" : this->mpoConfig->GetKey(this->miFieldID, this->miKeyID).GetKey();
}
        
apl_int_t CBaseConfig::CKeyIterator::ToInt(void)
{
    const char* lpcValue = NULL;
    
    if ( !this->IsEmpty() )
    {
        lpcValue = this->mpoConfig->GetKey(this->miFieldID, this->miKeyID).GetValue();
    }
    else
    {
        lpcValue = this->mpcDefault;
    }
    
    return lpcValue == NULL ? -1 : apl_strtoi32(lpcValue, NULL, 10);
}
        
bool CBaseConfig::CKeyIterator::ToBool(void)
{
    const char* lpcValue = NULL;
    
    if ( !this->IsEmpty() )
    {
        lpcValue = this->mpoConfig->GetKey(this->miFieldID, this->miKeyID).GetValue();
    }
    else
    {
        lpcValue = this->mpcDefault;
    }
    
    return lpcValue == NULL ? false : lpcValue[0] == '1' ? true : false;
}
        
double CBaseConfig::CKeyIterator::ToFloat(void)
{
    const char* lpcValue = NULL;
    
    if ( !this->IsEmpty() )
    {
        lpcValue = this->mpoConfig->GetKey(this->miFieldID, this->miKeyID).GetValue();
    }
    else
    {
        lpcValue = this->mpcDefault;
    }
    
    return lpcValue == NULL ? 0.0 : strtod(lpcValue, NULL);
}

const char* CBaseConfig::CKeyIterator::ToString(void)
{
    const char* lpcValue = NULL;
    
    if ( !this->IsEmpty() )
    {
        lpcValue = this->mpoConfig->GetKey(this->miFieldID, this->miKeyID).GetValue();
    }
    else
    {
        lpcValue = this->mpcDefault;
    }
    
    return lpcValue == NULL ? "" : lpcValue;
}

//////////////////////////////////////////////////////////////////////////////////////////
CBaseConfig::CFieldIterator::CFieldIterator(
    CBaseConfig* apoConfig,
    apl_ssize_t aiFieldID,
    char const* apcFieldPattern )
    : mpoConfig(apoConfig)
    , miFieldID(aiFieldID)
    , mpcFieldPattern(apcFieldPattern)
{
}

bool CBaseConfig::CFieldIterator::operator == ( CBaseConfig::CFieldIterator const& aoRhs )
{
    return this->miFieldID == aoRhs.miFieldID;
}
        
bool CBaseConfig::CFieldIterator::operator != ( CBaseConfig::CFieldIterator const& aoRhs )
{
    return !(*this == aoRhs);
}
        
CBaseConfig::CFieldIterator& CBaseConfig::CFieldIterator::operator ++ (void)
{
    if ( !this->IsEmpty() )
    {
        if ( this->mpcFieldPattern == NULL )
        {
            this->miFieldID++;
        }
        else
        {
            this->miFieldID = this->mpoConfig->FindField(this->miFieldID + 1, this->mpcFieldPattern);
        }
        
        if ( this->miFieldID >= (apl_ssize_t)this->mpoConfig->GetFieldSize() )
        {
            this->miFieldID = -1;
        }
    }
    
    return *this;
}
        
bool CBaseConfig::CFieldIterator::IsEmpty(void)
{
    return this->miFieldID < 0;
}
        
const char* CBaseConfig::CFieldIterator::GetFieldName(void)
{
    return this->IsEmpty() ? "" : this->mpoConfig->GetField(this->miFieldID).GetName();
}

CBaseConfig::KeyIterType CBaseConfig::CFieldIterator::GetValue( const char* apcKey, const char* apcDefault )
{
    apl_ssize_t liKeyID = this->mpoConfig->FindKey(this->miFieldID, apcKey);
    if ( liKeyID < 0 || this->IsEmpty() )
    {
        return KeyIterType(this->mpoConfig, this->miFieldID, -1, apcDefault);
    }
    else
    {
        return KeyIterType(this->mpoConfig, this->miFieldID, liKeyID, apcDefault);
    }
}

CBaseConfig::KeyIterType CBaseConfig::CFieldIterator::GetValue( apl_size_t aiIndex, const char* apcDefault )
{
    if ( this->IsEmpty() )
    {
        return KeyIterType(this->mpoConfig, this->miFieldID, -1, apcDefault);
    }
    else
    {
        return KeyIterType(this->mpoConfig, this->miFieldID, aiIndex, apcDefault);
    }
}

apl_size_t CBaseConfig::CFieldIterator::GetKeyCount(void)
{
    return this->IsEmpty() ? 0 : this->mpoConfig->GetKeySize(this->miFieldID);
}
   
CBaseConfig::KeyIterType CBaseConfig::CFieldIterator::Begin(void)
{
    if ( this->GetKeyCount() == 0 )
    {
        return this->End();
    }
    else
    {
        return KeyIterType(this->mpoConfig, this->miFieldID, 0, NULL);
    }
}
        
CBaseConfig::KeyIterType CBaseConfig::CFieldIterator::End(void)
{
    return KeyIterType(this->mpoConfig, -1, -1, NULL);
}

ACL_NAMESPACE_END
