#ifndef __AILIBEX__AIREGEXPR_H__
#define __AILIBEX__AIREGEXPR_H__

#include "AILib.h"
#include <regex.h>
#include "stl/vector.h"

///start namespace
AIBC_NAMESPACE_START

using namespace AI_STD;

class AIMatchResult
{
public:
    void AddMatchResult(regoff_t aiBegin, regoff_t aiEnd);
   
    size_t GetMatchCount() const;
     
    regoff_t GetBeginOffset(size_t aiNth) const;
    regoff_t GetEndOffset(size_t aiNth) const;

    void Reset();

private:
    struct range
    {
        regoff_t    ciBegin;
        regoff_t    ciEnd;
    };
    
    vector<range> coResults;  
};


class AIRegExpr
{
public:
    AIRegExpr();
    ~AIRegExpr();

    int Compile(char const* apcPattern, int aiOpt = REG_EXTENDED);

    int Match(char const* apcStr, int aiOpt = 0);
  
    int Find(char const* apcStr, size_t aiMaxResult, AIMatchResult* apMatch, int aiOpt = 0);

    int FindAll(char const* apcStr, AIMatchResult* apMatch, int aiOpt = 0);

    bool IsReady() const;

private:
    bool        cbIsCompiled;
    regex_t     coRE;
};

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AIREGEXPR_H__
