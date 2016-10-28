#include <assert.h>
#include <string.h>
#include "AILib.h"
#include "AICMPPStru.h"

#define TEST_NUM_OK_1 "13812341234"
#define TEST_NUM_OK_2 "1"
#define TEST_NUM_OK_3 "1381234123412341"
#define TEST_NUM_FAIL_1 "13812341234123412341"
#define TEST_NUM_FAIL_2 ""

void test_num(char const* apcTestNum, bool abResult)
{
    char lsBuf[256];
    int liRetCode;

    strcpy(lsBuf, apcTestNum);
    liRetCode = Remove86(lsBuf);
    assert((0 == liRetCode) == abResult); 
}

void test_with_prefixes(char const** appcTestNums, size_t aiLen, bool abResult)
{
    for (size_t i = 0; i < aiLen; ++i)
    {
        test_num(appcTestNums[i], abResult);
    }
}

static char const* OK_NUMBERS[] = {
    "8613812341234", 
    "008613812341234", 
    "+8613812341234", 
    "+13812341234",
    "1345",
    "12345678901234567890",
    "1"
};

static char const* ERR_NUMBERS[] = {
    "123456789012345678901", 
    "+", 
    "-", 
    "86123456789012345678901",
    ""
};


int main()
{
    test_with_prefixes(OK_NUMBERS, AI_ARRAY_SIZE(OK_NUMBERS), true);

    test_with_prefixes(ERR_NUMBERS, AI_ARRAY_SIZE(ERR_NUMBERS), false);

    return 0;
}

