#include <stdio.h>

#include "AILib.h"
#include "AITime.h"
#include "AISMPPStru.h"


int main(int argc, char** argv)
{
    char lsBuf[17];
    AITime_t liTime;
    int liRetCode;

    AI_RETURN_IF(-1, argc < 2);

    liTime = AIParseSMPPTime(argv[1]);

    if (liTime < 0)
    {
        printf("error parsing smpp time!\n");
        return -1;
    }

    liRetCode = AIFormatAbsSMPPTime(lsBuf, sizeof(lsBuf), liTime);     

    if (liRetCode < 0)
    {
        printf("error formatting smpp time!\n");
        return -1;
    }

    printf("> %s\n", lsBuf);

    return 0;
}

