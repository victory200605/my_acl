
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>

#include "AIRegExpr.h"


int main(int aiArgc, char** appcArgv)
{
    AIRegExpr       loRE;
    AIMatchResult   loResultAll;
    AIMatchResult   loResultN;
    int             liRetCode;

    assert(aiArgc >= 2);

    liRetCode = loRE.Compile(appcArgv[1]);

    assert(0 == liRetCode);

    for (int i = 2; i < aiArgc; ++i)
    {
        char const* const& lpcStr = appcArgv[i];

        liRetCode = loRE.Match(lpcStr);

        printf("[%s]: %d\n", lpcStr, liRetCode);

        if (0 == liRetCode)
        {
            loRE.FindAll(lpcStr, &loResultAll);

            loRE.Find(lpcStr, loResultAll.GetMatchCount(), &loResultN);

            assert(loResultAll.GetMatchCount() == loResultN.GetMatchCount());

            for (size_t n = 0; n < loResultAll.GetMatchCount(); ++n)
            {
                printf(
                    "%u: (%jd, %jd) (%jd, %jd)\n", 
                    n, 
                    (intmax_t)loResultAll.GetBeginOffset(n), 
                    (intmax_t)loResultAll.GetEndOffset(n),
                    (intmax_t)loResultN.GetBeginOffset(n),
                    (intmax_t)loResultN.GetEndOffset(n)
                );

                assert(loResultAll.GetBeginOffset(n) <= loResultAll.GetEndOffset(n));
                assert(loResultN.GetBeginOffset(n) <= loResultN.GetEndOffset(n));
                assert(loResultAll.GetBeginOffset(n) == loResultN.GetBeginOffset(n));
                assert(loResultAll.GetEndOffset(n) == loResultN.GetEndOffset(n));
        
                write(
                    STDOUT_FILENO, 
                    lpcStr + loResultAll.GetBeginOffset(n), 
                    loResultAll.GetEndOffset(n) - loResultAll.GetBeginOffset(n)
                );

                write(STDOUT_FILENO, "\n", 1);
            }
        }
    }
   
    return 0; 
}
