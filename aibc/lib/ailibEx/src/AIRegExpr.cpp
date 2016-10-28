
#include <assert.h>

#include "AIRegExpr.h"

///start namespace
AIBC_NAMESPACE_START

void AIMatchResult::AddMatchResult(regoff_t aiBegin, regoff_t aiEnd)
{
    range loRange;

    loRange.ciBegin = aiBegin;
    loRange.ciEnd = aiEnd;

    coResults.push_back(loRange);
}


size_t AIMatchResult::GetMatchCount() const
{
    return coResults.size();
}

regoff_t AIMatchResult::GetBeginOffset(size_t aiNth) const
{
    return coResults[aiNth].ciBegin;
}

regoff_t AIMatchResult::GetEndOffset(size_t aiNth) const
{
    return coResults[aiNth].ciEnd;
}
            
void AIMatchResult::Reset()
{
    coResults.clear();
}


////////////////////////////////////////////////////////////////////////////
AIRegExpr::AIRegExpr()
    :cbIsCompiled(false)
{
}


AIRegExpr::~AIRegExpr()
{
    if (cbIsCompiled)
    {
        regfree(&coRE);
    }
}


int AIRegExpr::Compile(char const* apcPattern, int aiOpt)
{
    int liRetCode;

    if (cbIsCompiled)
    {
        regfree(&coRE);
        cbIsCompiled = false;
    }

    liRetCode = regcomp(&coRE, apcPattern, aiOpt);

    if (0 == liRetCode)
    {
        cbIsCompiled = true;
    }

    return liRetCode == 0 ? 0 : -1;
}


int AIRegExpr::Match(char const* apcStr, int aiOpt)
{
    assert(cbIsCompiled);
    
    return regexec(&coRE, apcStr, (size_t)0, NULL, aiOpt);
}


int AIRegExpr::Find(char const* apcStr, size_t aiMaxMatch, AIMatchResult* apoResult, int aiOpt)
{
    regmatch_t  ltResult;
    regoff_t    liOffset = 0;
    int         liRetCode;
     
    assert(apoResult);  
    assert(cbIsCompiled);

    apoResult->Reset();

    liRetCode = regexec(&coRE, apcStr, 1, &ltResult, aiOpt);
    for (size_t i = 0; i < aiMaxMatch && 0 == liRetCode && ltResult.rm_so != ltResult.rm_eo; ++i)
    {
        apoResult->AddMatchResult(liOffset + ltResult.rm_so, liOffset + ltResult.rm_eo);
        liOffset += ltResult.rm_eo;
        liRetCode = regexec(&coRE, apcStr + liOffset, 1, &ltResult, aiOpt|REG_NOTBOL);
    }

    return liRetCode;
}


int AIRegExpr::FindAll(char const* apcStr, AIMatchResult* apoResult, int aiOpt)
{
    regmatch_t  ltResult;
    regoff_t    liOffset = 0;
    int         liRetCode;
   
    assert(apoResult);  
    assert(cbIsCompiled);
    
    apoResult->Reset();
      
    liRetCode = regexec(&coRE, apcStr, 1, &ltResult, 0);
    while (0 == liRetCode && ltResult.rm_so != ltResult.rm_eo)
    {
        apoResult->AddMatchResult(liOffset + ltResult.rm_so, liOffset + ltResult.rm_eo);
        liOffset += ltResult.rm_eo;
        liRetCode = regexec(&coRE, apcStr + liOffset, 1, &ltResult, aiOpt|REG_NOTBOL);
    }

    return liRetCode;
}


bool AIRegExpr::IsReady() const
{
    return cbIsCompiled;
}

///end namespace
AIBC_NAMESPACE_END
