#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "AIGbkUnicode.h"

char gsInBuf[1024*1024];
char gsOutBuf[1024*1024*3];

typedef int ConvFun(char const*, int const, char*, int*); 


void DoConv(ConvFun apfFunc)
{
    int liSize1 = 0;
    int liSize2 = 0;
    int liRetCode;

    liSize1 = read(0, gsInBuf, sizeof(gsInBuf));

    liRetCode = apfFunc(gsInBuf, liSize1, gsOutBuf, &liSize2);

    fprintf(stderr, "convert result=%d, size1=%d, size2=%d\n", liRetCode, liSize1, liSize2);

    if (liSize2 > 0)
    {
        write(1, gsOutBuf, liSize2);
    }
}


void DoConv2()
{
    int liSize1 = 0;
    int liSize2 = 0;
    int liRetCode;

    liSize1 = read(0, gsInBuf, sizeof(gsInBuf));

    memset(gsOutBuf, 0, sizeof(gsOutBuf));
    liRetCode = GBToUTF8(gsInBuf, liSize1, gsOutBuf);

    fprintf(stderr, "convert result=%d, size1=%d, size2=%d\n", liRetCode, liSize1, liSize2);

    if (liSize2 > 0)
    {
        write(1, gsOutBuf, strlen(gsOutBuf));
    }
}


void Usage(char const* apcProg)
{
    fprintf(
        stderr, 
        "USAGE:  %s { -gu | -ug | -bu | -ub | -g8 }\n"
        "  read input data from stdin and write output data to stdout\n"
        " -gu    GBK to UNICODE\n"
        " -ug    UNICODE to GBK\n"
        " -bu    BIG5 to UNICODE\n"
        " -ub    UNICODE to BIG5\n"
        " -g8    GBK to UTF-8\n"
        "",
        apcProg
    );
}


int main(int aiArgc, char** appcArgv)
{
    if (aiArgc < 2)
    {
        Usage(appcArgv[0]);
        exit(0);
    }

    if (strcmp("-gu", appcArgv[1]) == 0)
    {
        DoConv(&GBToUnicode);
    }
    else if (strcmp("-ug", appcArgv[1]) == 0)
    {
        DoConv(&UnicodeToGB);
    }
    else if (strcmp("-bu", appcArgv[1]) == 0)
    {
        DoConv(&BIG5ToUnicode);
    }
    else if (strcmp("-ub", appcArgv[1]) == 0)
    {
        DoConv(&UnicodeToBIG5);
    }
    else if (strcmp("-g8", appcArgv[1]) == 0)
    {
        DoConv2();
    }
}


