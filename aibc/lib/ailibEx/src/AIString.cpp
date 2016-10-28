#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "AIString.h"

///start namespace
AIBC_NAMESPACE_START

//////////////////////////////////////////////////////////////////////////////////////////
char* StringTrim(char* apcInStr)
{
    char* lpcPtr1 = NULL;
    char* lpcPtr2 = NULL;

    for(lpcPtr1 = apcInStr; (*lpcPtr1) && strchr(" \t\r\n",*lpcPtr1); *lpcPtr1++=0);
    for(lpcPtr2 = apcInStr; (*lpcPtr1) && lpcPtr1>apcInStr; *lpcPtr2=*lpcPtr1++, lpcPtr2++, *lpcPtr2=0);
    for(lpcPtr2 = apcInStr+strlen(apcInStr)-1; (*apcInStr)&&strchr(" \t\r\n",*lpcPtr2); *lpcPtr2--=0);

    return  (apcInStr);
}


char* StringUpper(char* apcInStr)
{
    char* lpcPtr1 = NULL;

    for(lpcPtr1 = apcInStr; (*lpcPtr1); *lpcPtr1 = toupper(*lpcPtr1), lpcPtr1++);
    return  (apcInStr);
}


char* StringLower(char* apcInStr)
{
    char* lpcPtr1 = NULL;

    for(lpcPtr1 = apcInStr; (*lpcPtr1); *lpcPtr1 = tolower(*lpcPtr1), lpcPtr1++);
    return  (apcInStr);
}


int StringAtoi(char const* apcInStr)
{
    int         liOffset = 0;
    char const* lpcPtr1 = NULL;

    for(lpcPtr1 = apcInStr; (*lpcPtr1) && strchr(" \t\r\n",*lpcPtr1); lpcPtr1++);

    lpcPtr1 += (liOffset = strncasecmp(lpcPtr1, "0x", 2)?0:2);
    return  strtoul(lpcPtr1, (char**)NULL, liOffset?16:10);
}


void StringCopy(char* apcDestStr, char const* apcSrcStr, size_t aiSize)
{
    const char  *lpcPtr1 = apcSrcStr;
    char        *lpcPtr2 = apcDestStr;

    *lpcPtr2=0;
    while ((*lpcPtr1) && (lpcPtr2 - apcDestStr) < (ptrdiff_t)(aiSize-1) )
    {
        *lpcPtr2++ = *lpcPtr1++;
        *lpcPtr2 = 0;
    }
}


char const* FindSubstr(char const* apData, size_t aiLen, char const* apcStr)
{
    size_t liSubLen = strlen(apcStr);
    for(char const* p = apData; p < apData + aiLen - liSubLen; ++p)
    {
        if(memcmp(p, apcStr, liSubLen) == 0)
        {
            return p;
        }
    }
    return NULL;
}


//////////////////////////////////////////////////////////////////////////////////////////

int StringIsNumber(char const* apcString)
{
    for(size_t liIt = 0; apcString[liIt] != 0; ++liIt)
    {
        if(isdigit(apcString[liIt]) == 0 && apcString[liIt] != '.' && 
            apcString[liIt] != '-' && apcString[liIt] != '+')
        {
            return  (1);
        }
    }

    return  (0);
}


int StringIsInteger(char const* apcString)
{
    for(size_t liIt = 0; apcString[liIt] != 0; ++liIt)
    {
        if(isdigit(apcString[liIt]) == 0 && apcString[liIt] != '-')
        {
            return  (1);
        }
    }

    return  (0);
}


bool StringMatch(char const* apcInData, char const* apcPattern, bool aiNoCase)
{
    switch(*apcPattern)
    {
        case '\0':      return !(*apcInData);

        case  '*':      return StringMatch(apcInData, apcPattern+1, aiNoCase)  ||
                        ( (*apcInData) && StringMatch(apcInData+1, apcPattern, aiNoCase) );

        case  '?':      return (*apcInData) && StringMatch(apcInData+1, apcPattern+1, aiNoCase);

        default :       return ((*apcPattern==*apcInData) || (aiNoCase && (toupper(*apcPattern)==toupper(*apcInData)))) 
                        && StringMatch(apcInData+1, apcPattern+1, aiNoCase);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

unsigned int GetStringHashKey(char const* apcString, size_t aiMaxHashKey)
{
    unsigned int    liHashKey = 0;
    unsigned int    liCharTotal = 0;

    for(size_t liIt=0; apcString[liIt] != 0; ++liIt)
    {
        liCharTotal = liCharTotal + (unsigned char)apcString[liIt];
    }

    liHashKey = liCharTotal % aiMaxHashKey;
    return  (liHashKey);
}

unsigned int GetStringHashKey1(char const* apcString, size_t aiMaxHashKey)
{
    int                 liChar = 0;
    unsigned int        liHashKey = 0;
    unsigned long       liHashVal = 5381;
    
    for(size_t liIt = 0; apcString[liIt] != 0; ++liIt)
    {
        liChar = (unsigned char)apcString[liIt];
        liHashVal = ((liHashVal << 5) + liHashVal) + liChar; /* liHashVal * 33 + liChar */
    }

    liHashKey = liHashVal % aiMaxHashKey;
    return  (liHashKey);
}

unsigned int GetStringHashKey2(const char *apcString, size_t aiMaxHashKey)
{
    int                 liChar = 0;
    unsigned int        liHashKey = 0;
    unsigned long       liHashVal = 0;
    
    for(size_t liIt = 0; apcString[liIt] != 0; ++liIt)
    {
        liChar = (unsigned char)apcString[liIt];
        liHashVal = liChar + (liHashVal << 6) + (liHashVal << 16) - liHashVal;
    }

    liHashKey = liHashVal % aiMaxHashKey;
    return  (liHashKey);
}

//////////////////////////////////////////////////////////////////////////////////////////

void GetPathName(char const* apcFullName, char* apcPath, size_t aiSize)
{
    char* lpcPtr = (char *)NULL;

    if((lpcPtr = (char*)strrchr(apcFullName, '/')) != NULL)
    {
        size_t liCopySize = aiSize;
        if(((lpcPtr-apcFullName) + 1) < (ptrdiff_t)aiSize)  liCopySize = (lpcPtr-apcFullName) + 2;
        StringCopy(apcPath, apcFullName, liCopySize);
    }
    else
    {
        strcpy(apcPath, "./");
    }
}

void GetFileName(char const* apcFullName, char* apcFile, size_t aiSize)
{
    char* lpcPtr = (char*)NULL;

    if((lpcPtr = (char*)strrchr(apcFullName, '/')) != NULL)
    {
        size_t liCopySize = aiSize;
        if(strlen(lpcPtr+1) < aiSize)  liCopySize = strlen(lpcPtr+1)+1;
        StringCopy(apcFile, lpcPtr+1, liCopySize);
    }
    else
    {
        StringCopy(apcFile, apcFullName, aiSize);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

char* FormatDataToHex(char* apcInBuf, size_t aiBufSize)
{
    size_t liHexSize = 0;
    char*   lpcPtr = NULL;
    char*   lpcBuffer = NULL;
    #define INN_HEX_BUF_MAX_SIZE    (9216)


    if(apcInBuf == NULL || aiBufSize <= 0)
    {
        return  (lpcBuffer);
    }

    AI_NEW_N_ASSERT(lpcBuffer, char, INN_HEX_BUF_MAX_SIZE);
    liHexSize = (aiBufSize >= 1840)?1840:aiBufSize;
    lpcPtr = lpcBuffer;

    for(size_t liIt = 0; liIt < liHexSize;)
    {
        for(size_t liIk = 0; liIk < 16; liIt++, liIk++)
        {
            if(!liIk)
            {
                sprintf(lpcPtr+0, "0x%04zX:  ", liIt);
            }

            if(liIt < liHexSize)
            {
                sprintf(lpcPtr+9+liIk*3, "%02X%c", (unsigned char)apcInBuf[liIt], (liIk==7)?'-':' ');
                if(strchr("%\\\"\'", apcInBuf[liIt]) /* BUG */)
                {
                    sprintf(lpcPtr+9+49+liIk, "%c", '.');
                }
                else
                {
                    sprintf(lpcPtr+9+49+liIk, "%c", isprint(apcInBuf[liIt])?apcInBuf[liIt]:'.');
                }
            }
            else
            {
                sprintf(lpcPtr+9+liIk*3, "%s", "   ");
                sprintf(lpcPtr+9+49+liIk, "%c", ' ');
            }
        }

        *(lpcPtr+9+49-1) = ' ';
        *(lpcPtr+9+49+16) = '\n';
        lpcPtr = lpcPtr + 9 + 49 + 16 + 1; /* 75 Bytes */
    }

    *lpcPtr = '\0';
    /* AI_DELETE() */
    return  (lpcBuffer);
}

//////////////////////////////////////////////////////////////////////////////////////////

char* GetStringColumn(char const* apcString, size_t aiColumn, char acDelimiter, size_t* aiLen)
{
    size_t  liStringLen = 0;
    char    *lpcItem = NULL;
    size_t  liItem, liTempLen;

    *aiLen = 0;
    liItem = 0;
    liTempLen = 0;
    liStringLen = strlen(apcString);
    
    for(size_t liIt = 0; liIt<liStringLen && liItem <= aiColumn; ++liIt)
    {
        if(aiColumn == liItem)
        {
            if(*aiLen == 0)
            {
                // skip left delimiter
                if(apcString[liIt] == acDelimiter)
                {
                    continue;
                }
                lpcItem = (char *)&(apcString[liIt]);
            }
            else
            {
                if(apcString[liIt] == acDelimiter)
                {
                    break;
                }
            }
            *aiLen = *aiLen + 1;
        }
        else
        {
            if(liTempLen == 0)
            {
                if(apcString[liIt] == acDelimiter)
                {
                    continue;
                }
            }
            else
            {
                if(apcString[liIt] == acDelimiter)
                {
                    liItem++;
                    liTempLen = 0;
                    continue;
                }
            }
            liTempLen = liTempLen + 1;
        }
    }

    return  (lpcItem);
}

int ExtractStringColumns(char const* apcString, char acDelimiter,
            char const* apcQuoter, char** apcStringColumn, size_t aiColumnCount)
{
    size_t      liCount = 0;
    ssize_t     liExtractLen;
    size_t      liStartPos;
    size_t      liStringLen;
    const char  *lpcString;

    // char * apcStringColumn[...]
    memset(apcStringColumn, 0, sizeof(char *) * aiColumnCount);

    liStringLen = strlen(apcString);
    if(liStringLen == 0)
        return  (0);

    // no quoter
    if(apcQuoter == NULL || strlen(apcQuoter) == 0)
    {
        lpcString = &(apcString[0]);
        for(liCount=0; liCount<aiColumnCount; ++liCount)
        {
            if(lpcString - &(apcString[0]) >= (ptrdiff_t)liStringLen)
            {
                break;
            }
            if(*lpcString == 0)
            {
                break;
            }

            lpcString = GetStringColumn(lpcString, 0, acDelimiter, (size_t*)&liExtractLen);
            if(lpcString == NULL || liExtractLen == 0)
            {
                break;
            }

            AI_NEW_N_ASSERT(apcStringColumn[liCount], char, liExtractLen + 1);
            StringCopy(apcStringColumn[liCount], lpcString, liExtractLen+1);

            lpcString = lpcString + liExtractLen + 1;
        }
    }
    // has quoter
    else
    {
        liExtractLen = -1;
        liStartPos = 0;
        liCount = 0;

        for(size_t liPos=0; apcString[liPos]!=0 && liCount<aiColumnCount; ++liPos)
        {
            if(apcString[liPos] == apcQuoter[0])
            {
                // mean to start copying characters
                if(liExtractLen == -1)
                {
                    liStartPos = liPos + 1;
                    liExtractLen = 0;
                }
                // mean to stop copying characters
                else
                {
                    AI_NEW_N_ASSERT(apcStringColumn[liCount], char, liExtractLen + 1);
                    StringCopy(apcStringColumn[liCount], &(apcString[liStartPos]), liExtractLen+1);
                    StringTrim(apcStringColumn[liCount]);

                    liCount++;
                    liExtractLen = -1;
                }
            }
            else
            {
                if(liExtractLen >= 0)
                {
                    liExtractLen++;
                }
            }
        }
    }

    return  (liCount);
}

void ReleaseStringColumns(char** apcStringColumn, size_t aiColumnCount)
{
    for(size_t liIt=0; liIt<aiColumnCount; ++liIt)
    {
        AI_DELETE_N(apcStringColumn[liIt]);
    }
}

///end namespace
AIBC_NAMESPACE_END
