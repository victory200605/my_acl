
#include "acl/codecs/Escape.h"

ACL_NAMESPACE_START

CEscapeEntityName::CEscapeEntityName(void)
{
} 

CEscapeEntityName::CEscapeEntityName(char const* apcName)
{
    this->mpcName = apcName;
    this->mpuLength = apl_strlen(apcName);
}
    
CEscapeEntityName::CEscapeEntityName(char const* apcName, apl_size_t auLength)
{
    this->mpcName = apcName;
    this->mpuLength = auLength;
}
    
CEscapeEntityName::CEscapeEntityName(CEscapeEntityName const& aoName)
{
    this->mpcName = aoName.mpcName;
    this->mpuLength = aoName.mpuLength;
}
    
bool CEscapeEntityName::operator == (CEscapeEntityName const& aoName) const
{
    return apl_strncmp(this->mpcName, aoName.mpcName, this->mpuLength) == 0;
}

apl_size_t CEscapeReservedEntityNameHash::operator()( CEscapeEntityName const& aoName ) const
{
    if (aoName.mpuLength <= 4)
    {
        return 5;
    }
    
    switch(aoName.mpcName[1])
    {
        case 'l': return 0;
        case 'g': return 1;
        case 'a': return aoName.mpcName[2] == 'm' ? 2 : 3;
        case 'q': return 4;
        default:  return 5;
    }
}

CEscapeReservedCharacters::CEscapeReservedCharacters(void)
{
    this->SetCharacter('<',  "&lt;");
    this->SetCharacter('>',  "&gt;");
    this->SetCharacter('&',  "&amp;");
    this->SetCharacter('\'', "&apos;");
    this->SetCharacter('\"', "&quot;");
}

ACL_NAMESPACE_END
