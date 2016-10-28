#include "acl/Regex.h"

ACL_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////////////////////
CRegexMatchResult::CItem::CItem( char const* apcSubStr, apl_size_t aiLen)
    : moSubStr(apcSubStr, aiLen)
{
}

char const* CRegexMatchResult::CItem::GetStr(void)
{
   return this->moSubStr.c_str();
}

apl_size_t CRegexMatchResult::CItem::GetLength(void)
{
   return this->moSubStr.length();
}

CRegexMatchResult::~CRegexMatchResult(void)
{
    this->Reset();
}

CRegexMatchResult::CItem& CRegexMatchResult::operator[] ( apl_size_t aiN )
{
    return *this->moItems[aiN];
}
 
apl_size_t CRegexMatchResult::GetSize(void)
{
    return this->moItems.size();   
}

void CRegexMatchResult::AddMatchResult(char const* apcSubStr, apl_size_t aiLen)
{
    CItem* lpItem = NULL;
    
    ACL_NEW_ASSERT( lpItem, CItem(apcSubStr, aiLen) );

    this->moItems.push_back(lpItem);
}

void CRegexMatchResult::Reset(void)
{
    CItem* lpoItem = NULL;
    
    for( std::vector<CItem*>::iterator loIter = this->moItems.begin();
         loIter != this->moItems.end() ; ++loIter )
    {
        lpoItem = *loIter;
        ACL_DELETE(lpoItem);
    }
    
    this->moItems.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
CRegex::CRegex(void)
    : mbIsCompiled(false)
{
}

CRegex::~CRegex(void)
{
    if (this->mbIsCompiled)
    {
        apl_regfree(&this->moRegex);
    }
}


apl_int_t CRegex::Compile( char const* apcPattern, apl_int_t aiOpt )
{
    apl_int_t liRetCode = 0;
    
    if (this->mbIsCompiled)
    {
        apl_regfree(&this->moRegex);
        this->mbIsCompiled = false;
    }

    if ( ( liRetCode = apl_regcomp(&this->moRegex, apcPattern, aiOpt) ) == 0 )
    {
        this->mbIsCompiled = true;
    }

    return liRetCode == 0 ? 0 : -1;
}



bool CRegex::Match( char const* apcInput, apl_int_t aiOpt )
{
    apl_int_t liRetCode = apl_regexec(&this->moRegex, apcInput, (apl_size_t)0, NULL, aiOpt);
    
    return (liRetCode != APL_REG_NOMATCH && liRetCode == 0) ? true : false;
}

bool CRegex::IsReady(void) const
{
    return this->mbIsCompiled;
}

apl_ssize_t CRegex::Search( char const* apcInput, ResultType* apoMatch, apl_int_t aiOpt )
{
    apl_regmatch_t loResult;
    apl_int_t      liRetCode = 0;
    apl_uint64_t   liOffset = 0;
    
    if (apoMatch == NULL)
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    apoMatch->Reset();
    
    apl_memset(&loResult, 0, sizeof(loResult) );
    liRetCode = apl_regexec(&this->moRegex, apcInput, 1, &loResult, aiOpt);
    
    while (liRetCode == 0 && loResult.rm_so != loResult.rm_eo)
    {
        apoMatch->AddMatchResult(
            apcInput + liOffset + loResult.rm_so, loResult.rm_eo - loResult.rm_so);
        
        liOffset += loResult.rm_eo;
        
        liRetCode = apl_regexec(&this->moRegex, apcInput + liOffset, 1, &loResult, aiOpt | OPT_NOTBOL);
    }

    return liRetCode;
}

ACL_NAMESPACE_END
