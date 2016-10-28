#include <stdlib.h>

#include "AILib.h"
#include "AICrypt.h"
#include "AIString.h"
#include "AIMemAlloc.h"

void GenerateRandomContent(void* apPtr, size_t aiLength)
{
    char* apcEnd = ((char*)apPtr) + aiLength;
    srand(time(NULL)^rand());

    for (char* apcItr = (char*)apPtr; apcItr < apcEnd; ++apcItr)
    {
        *apcItr = rand();
    }
}

void InsertSpace(AIChunk const& apoChunk1, AIChunk& apoChunk2, size_t aiSize)
{
    char* apcPtr1 = (char*)apoChunk1.GetPointer();
    char* apcPtr2 = (char*)apoChunk2.GetPointer();

    for (size_t i = 0; i < aiSize; ++i)
    {
        apcPtr2[i*2] = ' ';
        apcPtr2[i*2 + 1] = apcPtr1[i];
    }
}


int TestBase64(size_t aiTestLength)
{
    size_t      liEncodeSize = BASE64_CALC_ENCODE_SIZE(aiTestLength);
    size_t      liDecodeSize = BASE64_CALC_DECODE_SIZE(liEncodeSize);
    AIChunk     loBin0(aiTestLength);
    AIChunk     loAsc0(liEncodeSize);
    AIChunk     loAsc1(liEncodeSize*2);
    AIChunk     loBin1(liDecodeSize*2);
    ssize_t     liRet;

    GenerateRandomContent(loBin0.GetPointer(), loBin0.GetSize());

    liRet = BASE64Encode(loBin0.GetPointer(), loBin0.GetSize(), loAsc0.GetPointer(), loAsc0.GetSize());
    
    assert(liRet == liEncodeSize - 1);
    
    printf("%u:%s\n", aiTestLength, loAsc0.GetPointer());
   
    InsertSpace(loAsc0, loAsc1, strlen((char*)loAsc0.GetPointer()));

    liRet = BASE64Decode(loAsc1.GetPointer(), strlen((char*)loAsc1.GetPointer()), loBin1.GetPointer(), loBin1.GetSize());

    assert(liRet == aiTestLength);

    assert(memcmp(loBin0.GetPointer(), loBin1.GetPointer(), aiTestLength) == 0);

    return 0;
}

int main(int aiArgc, char** appcArgv)
{
    size_t b = 100;
    size_t e = 100;

    if (aiArgc > 1)
    {
        b = e = StringAtoi(appcArgv[1]);
    }

    if (aiArgc > 2)
    {
        e = StringAtoi(appcArgv[2]);
    }

    for (size_t i = b; i <= e; ++i)
    {
        TestBase64(i);
    }

    return 0;
}

