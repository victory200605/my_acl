#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "AIConfig.h"
#include "AIString.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////
AIConfig::AIConfig(void)
{
    ai_init_list_head(&coIniHead);
    memset(csName, 0, sizeof(csName));

    memset(&(coConfigChild), 0, sizeof(coConfigChild));
}

AIConfig::~AIConfig(void)
{
    AIConfig::Clean();
}

/** 
    \brief Get SubString of one string.
    \param apcInStr: one line string.
    \param apcOutStr: sub string.
    \param aiOutSize: the buffer size of apcOutStr .
    \param aiHasMarks: 1 or 0
    \return the pointer of next sub string.
*/
char* AIConfig::GetSubString(char const* apcInStr, char* apcOutStr, size_t aiOutSize, int aiHasMarks)
{
    size_t      liOutOff = 0;
    int         liMarkFlag = 0;
    char        *lpcPtr1 = (char*)apcInStr;

    while(*lpcPtr1 && strchr(" \t",*lpcPtr1))
    {
        lpcPtr1++;
    }

    if(!(*lpcPtr1) || strchr(";#\r\n",*lpcPtr1))
    {
        return  (NULL);
    }

    for(lpcPtr1+=(liMarkFlag=((*lpcPtr1=='"'&&aiHasMarks)?1:0)); (*lpcPtr1)&&(liOutOff<(aiOutSize-1)); lpcPtr1 += 1)
    {
        if((liMarkFlag && (*lpcPtr1) == '"') || (!liMarkFlag && strchr(" \t\r\n", *lpcPtr1)))
        {
            if(liMarkFlag)  lpcPtr1 += 1;
            break;
        }

        if(liMarkFlag && (*lpcPtr1 == '\\' && *(lpcPtr1+1) == '"'))
        {
            *(apcOutStr+liOutOff) = *(lpcPtr1+1);
            liOutOff += 1;
            lpcPtr1 += 1;
            continue;
        }

        *(apcOutStr+liOutOff) = (*lpcPtr1);
        liOutOff += 1;
    }

    while(!strchr(" \t\r\n", *lpcPtr1))
    {
        lpcPtr1 += 1;
    }

    *(apcOutStr+liOutOff) = '\0';
    
    return  (lpcPtr1);
}

void AIConfig::Clean(void)
{
    LISTHEAD    *lpoTemp = NULL;
    LISTHEAD    *lpoHead = &coIniHead;

    while(!ai_list_is_empty(lpoHead))
    {
        CONFIGITEM  *lpoItem = NULL;
        
        ai_list_del_head(lpoTemp, lpoHead);
        lpoItem = AI_GET_STRUCT_PTR(lpoTemp, CONFIGITEM, coIniChild);
        
        if(lpoItem->cpcKey)  free(lpoItem->cpcKey);
        if(lpoItem->cpcValue)  free(lpoItem->cpcValue);
        free(lpoItem);
    }
}

/** 
    \brief Load all value in the init file.
    \param apcFileName: init file name.
    \return 0:ok -1: open init file failed  -2:malloc faile.
*/
int AIConfig::LoadINI(char const*apcFileName)
{
    FILE        *lpoFp = NULL;
    char        *lpcPtr = NULL;
    char        *lpcPtr2 = NULL;
    char        lsBuf[LINE_MAX] = {0};
    char        lsField[64] = {0};
    char        lsKey[64], lsValue[LINE_MAX];
    CONFIGITEM  *lpoIniItem = NULL;
    AISmartLock loLock(coMutexLock);

    if((lpoFp = fopen(apcFileName, "r")) == NULL)
        return  (-1);

    strcpy(csName, apcFileName);
    AIConfig::Clean();

    while(fgets(lsBuf, sizeof(lsBuf), lpoFp) != NULL)
    {
        for(lpcPtr = lsBuf; (*lpcPtr) && strchr(" \t", *lpcPtr); lpcPtr++);
        if((*lpcPtr) != '[' || *(lpcPtr+1) == ']' || !strchr(lpcPtr,']'))
            continue;

NEW_FIELD: /* Find New Field (length<64) */
        memset(lsField, 0, sizeof(lsField));
        strncpy(lsField, lpcPtr, sizeof(lsField)-1);
        
        lpcPtr = strchr(lsField, ']');
        if(lpcPtr)    *(lpcPtr+1) = '\0';

        while(fgets(lsBuf, sizeof(lsBuf), lpoFp) != NULL)
        {
            for(lpcPtr = lsBuf; (*lpcPtr) && strchr(" \t", *lpcPtr); lpcPtr++);
            if((*lpcPtr) == '[' && *(lpcPtr+1) != ']' && strchr(lpcPtr,']'))
                goto    NEW_FIELD;

#ifndef  NOT_USE_OLD_MODE
            if((lpcPtr2 = strchr(lpcPtr, '=')) == NULL)
                continue;
            *lpcPtr2++ = '\0';
#endif

            if((lpcPtr = GetSubString(lpcPtr, lsKey, sizeof(lsKey), 0)) == NULL)
                continue;

#ifndef  NOT_USE_OLD_MODE
            if((lpcPtr = GetSubString(lpcPtr2, lsValue, sizeof(lsValue), 1)) == NULL)
                strcpy(lsValue, "");
#else
            if((lpcPtr = GetSubString(lpcPtr, lsValue, sizeof(lsValue), 0)) == NULL)
                continue;

            if(strcmp(lsValue, "=") /*|| strlen(lsKey) <= 0*/)
                continue;

            if((lpcPtr = GetSubString(lpcPtr, lsValue, sizeof(lsValue), 1)) == NULL)
                strcpy(lsValue, "");
#endif

            /*
             * NOTE: Store Key=Value into AIConfig.
            */
            if((lpoIniItem = (CONFIGITEM*)malloc(sizeof(CONFIGITEM))) == NULL)
            {
                goto    ERR_EXIT;
            }

            memset(lpoIniItem, 0, sizeof(CONFIGITEM));
            sprintf(lsBuf, "%s.%s", lsField, lsKey);

            if((lpoIniItem->cpcKey = (char*)malloc(strlen(lsBuf)+1)) == NULL)
            {
                goto    ERR_EXIT;
            }

            if(strcpy(lpoIniItem->cpcKey, lsBuf), strlen(lsValue) > 0)
            {
                if((lpoIniItem->cpcValue = (char*)malloc(strlen(lsValue)+1)) == NULL)
                {
                    goto    ERR_EXIT;
                }

                strcpy(lpoIniItem->cpcValue, lsValue);
            }
            
            ai_list_add_tail(&(lpoIniItem->coIniChild), &coIniHead);
            continue;

ERR_EXIT:
            if(lpoIniItem)
            {
                if(lpoIniItem->cpcKey)  free(lpoIniItem->cpcKey);
                free(lpoIniItem);
            }

            fclose(lpoFp);
            return  (-2);
        }

        break;
    }

    fclose(lpoFp);
    return  (0);
}

int AIConfig::ReloadINI()
{
    return this->LoadINI( this->csName );
}

/** 
    \brief Get one int value in the init file.
    \param apcField [IN]: e.g. :FwdGateway.
    \param apcKeyVal [IN]: e.g. :port.
    \param aiDefValue [IN]: the default value of the key,e.g. :5999.
    \return the value.
*/
int AIConfig::GetIniInt(char const* apcField, char const* apcKeyVal, int aiDefValue)
{
    int             liRetCode = 0;
    char            *lpcPtr =  NULL;
    char            lsBuf[256] = {0};
    LISTHEAD        *lpoHead = &coIniHead;
    CONFIGITEM      *lpoIniItem = NULL;
    AISmartLock     loLock(coMutexLock);

    sprintf(lsBuf, "[%s].%s", apcField, apcKeyVal);
    for(LISTHEAD *lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoIniItem = AI_GET_STRUCT_PTR(lpoTemp, CONFIGITEM, coIniChild);

        if(strcmp(lpoIniItem->cpcKey, lsBuf) == 0)
        {
            if(lpoIniItem->cpcValue)
            {
                lpcPtr = lpoIniItem->cpcValue;
                lpcPtr += strncasecmp(lpoIniItem->cpcValue, "0x", 2)?0:2;
                liRetCode = strtoul(lpcPtr, (char**)NULL, (lpcPtr==lpoIniItem->cpcValue)?10:16);
                return  (liRetCode);
            }
        }
    }

    liRetCode = aiDefValue;
    return  (liRetCode);
}

/** 
    \brief Get one string value in the init file.
    \param apcField [IN]: e.g. :FwdGateway.
    \param apcKeyVal [IN]: e.g. :ip.
    \param apcRetValue [OUT]: the value of the key,e.g. :10.3.3.92.
    \param aiSize [IN]: the buffer size of apcRetValue.
    \return the count of character in apcRetValue.
*/
int AIConfig::GetIniString(char const* apcField, char const* apcKeyVal, char* apcRetValue, size_t aiSize)
{
    char            lsBuf[256] = {0};
    LISTHEAD        *lpoHead = &coIniHead;
    CONFIGITEM      *lpoIniItem = NULL;
    AISmartLock     loLock(coMutexLock);

    sprintf(lsBuf, "[%s].%s", apcField, apcKeyVal);
    for(LISTHEAD *lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoIniItem = AI_GET_STRUCT_PTR(lpoTemp, CONFIGITEM, coIniChild);

        if(strcmp(lpoIniItem->cpcKey, lsBuf) == 0)
        {
            if(lpoIniItem->cpcValue)
            {
                memset(apcRetValue, 0, aiSize);
                strncpy(apcRetValue, lpoIniItem->cpcValue, aiSize-1);
                return  strlen(apcRetValue);
            }
        }
    }

    apcRetValue[0] = '\0';
    return  (0);
}

size_t AIConfig::GetKeyCount(char const* apcField)
{
    size_t          liCount = 0;
    char            lsBuf[256] = {0};
    LISTHEAD        *lpoHead = &coIniHead;
    CONFIGITEM      *lpoIniItem = NULL;
    AISmartLock     loLock(coMutexLock);

    sprintf(lsBuf, "[%s].", apcField);
    for(LISTHEAD *lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoIniItem = AI_GET_STRUCT_PTR(lpoTemp, CONFIGITEM, coIniChild);

        if(strncmp(lpoIniItem->cpcKey, lsBuf, strlen(lsBuf)) == 0)
        {
            if(lpoIniItem->cpcValue)
            {
                ++liCount;
            }
        }
    }

    return  (liCount);
}

size_t AIConfig::GetFieldCount(char const* apcFieldPattern)
{
    size_t          liCount = 0;
    char            lsBuf[256] = {0};
    LISTHEAD        *lpoHead = &coIniHead;
    CONFIGITEM      *lpoIniItem = NULL;
    AISmartLock     loLock(coMutexLock);

    sprintf(lsBuf, "[%s].", apcFieldPattern);
    for(LISTHEAD *lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoIniItem = AI_GET_STRUCT_PTR(lpoTemp, CONFIGITEM, coIniChild);

        if(StringMatch(lpoIniItem->cpcKey, lsBuf, false))
        {
            if(lpoIniItem->cpcValue)
            {
                ++liCount;
            }
        }
    }

    return  (liCount);
}

int AIConfig::GetFieldByIndex(char const* apcFieldPattern, size_t aiIndex, char* apcField, size_t aiFieldLen)
{
    size_t          liCount = 0;
    char            lsBuf[256] = {0};
    LISTHEAD        *lpoHead = &coIniHead;
    CONFIGITEM      *lpoIniItem = NULL;
    AISmartLock     loLock(coMutexLock);

    sprintf(lsBuf, "[%s].", apcFieldPattern);
    for (LISTHEAD *lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoIniItem = AI_GET_STRUCT_PTR(lpoTemp, CONFIGITEM, coIniChild);

        if(StringMatch(lpoIniItem->cpcKey, lsBuf, false))
        {
            if(lpoIniItem->cpcValue)
            {
                if (aiIndex == liCount)
                {
                    char* lpcEnd = strstr(lpoIniItem->cpcValue, "].");
                    assert(NULL != lpcEnd);
                    assert(lpcEnd >= (lpoIniItem->cpcValue + 1));
                    StringCopy(apcField, lpoIniItem->cpcValue + 1, lpcEnd - (lpoIniItem->cpcValue + 1));
                    return 0;
                }
                ++liCount;
            }
        }
    }

    return  (-1);
}

int AIConfig::GetIniStringByIndex(char const* apcField, size_t aiIndex, char* apcKeyVal, size_t aiKeyLen, char* apcRetValue, size_t aiValLen)
{
    size_t          liCount = 0;
    char            lsBuf[256] = {0};
    LISTHEAD        *lpoHead = &coIniHead;
    CONFIGITEM      *lpoIniItem = NULL;
    AISmartLock     loLock(coMutexLock);

    sprintf(lsBuf, "[%s].", apcField);
    for(LISTHEAD *lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoIniItem = AI_GET_STRUCT_PTR(lpoTemp, CONFIGITEM, coIniChild);

        if(strncmp(lpoIniItem->cpcKey, lsBuf, strlen(lsBuf)) == 0)
        {
            if(lpoIniItem->cpcValue)
            {
                if (aiIndex == liCount)
                {
                    StringCopy(apcKeyVal, lpoIniItem->cpcKey, aiKeyLen);
                    StringCopy(apcRetValue, lpoIniItem->cpcValue, aiValLen);
                    return 0;
                }
                ++liCount;
            }
        }
    }
    return -1;
}

int AIConfig::GetIniIntByIndex(char const* apcField, size_t aiIndex, char* apcKeyVal, size_t aiKeyLen, int aiValue)
{
    size_t          liCount = 0;
    char            lsBuf[256] = {0};
    char const*     lpcPtr;
    LISTHEAD        *lpoHead = &coIniHead;
    CONFIGITEM      *lpoIniItem = NULL;
    AISmartLock     loLock(coMutexLock);

    sprintf(lsBuf, "[%s].", apcField);
    for(LISTHEAD *lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoIniItem = AI_GET_STRUCT_PTR(lpoTemp, CONFIGITEM, coIniChild);

        if(strncmp(lpoIniItem->cpcKey, lsBuf, strlen(lsBuf)) == 0)
        {
            if(lpoIniItem->cpcValue)
            {
                if (aiIndex == liCount)
                {
                    StringCopy(apcKeyVal, lpoIniItem->cpcKey, aiKeyLen);
                    lpcPtr = lpoIniItem->cpcValue;
                    lpcPtr += strncasecmp(lpoIniItem->cpcValue, "0x", 2)?0:2;
                    int liRetCode = strtoul(lpcPtr, (char**)NULL, (lpcPtr==lpoIniItem->cpcValue)?10:16);
                    return  (liRetCode);
                }
                ++liCount;
            }
        }
    }
    return aiValue;
}

/** 
    \brief ShowAllInitInfo in the init file,use for TEST
    \return 0
*/
int AIConfig::ShowAllInitInfo()
{
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    CONFIGITEM      *lpoItem = NULL;

    lpoHead = &coIniHead;
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoItem = AI_GET_STRUCT_PTR(lpoTemp, CONFIGITEM, coIniChild);
        fprintf(stderr, "%s:%s\n", lpoItem->cpcKey, lpoItem->cpcValue);
    }
    
    return  (0);
}


////////////////////////////////////////////////////////////////////////////////////
static LISTHEAD     goGlobalConfigHead;
static AIMutexLock  goGlobalConfigLock;

void AIInitIniHandler(void) /* MUST after fork() */
{
    ai_init_list_head(&(goGlobalConfigHead));
}

void AICloseIniHandler(void)
{
    AIConfig        *lpoIniConfig = NULL;
    LISTHEAD        *lpoHead = &(goGlobalConfigHead);

    AISmartLock loSmartLock(goGlobalConfigLock);

    while(!ai_list_is_empty(lpoHead))
    {
        LISTHEAD    *lpoTemp = NULL;
        
        ai_list_del_head(lpoTemp, lpoHead);
        lpoIniConfig = AI_GET_STRUCT_PTR(lpoTemp, AIConfig, coConfigChild);
        
        AI_DELETE(lpoIniConfig);
        lpoIniConfig = NULL;
    }
}

int AIReloadAllIniHandler(void)
{
    LISTHEAD        *lpoTemp = NULL;
    AIConfig        *lpoIniConfig = NULL;
    LISTHEAD        *lpoHead = &(goGlobalConfigHead);
    AISmartLock     loLock(goGlobalConfigLock);

    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoIniConfig = AI_GET_STRUCT_PTR(lpoTemp, AIConfig, coConfigChild);
        if ( lpoIniConfig->ReloadINI() != 0 )
        {
            return -1;
        }
    }
    
    return 0;
}

/** 
    \brief Search the AIConfig handle in goGlobalConfigHead or new the handle and added it into goGlobalConfigHead
    \brief you should use AIInitIniHandler() andAIGetIniHandler() before you use AIConfig::LoadINI() .
    \param apcIniFileName [IN]: the name of init file.
    \return the handle of the pointer of AIConfig,then you can use the handle to call GetIniInt() and GetIniString().
*/
AIConfig *AIGetIniHandler(char const* apcIniFileName)
{
    LISTHEAD        *lpoTemp = NULL;
    AIConfig        *lpoIniConfig = NULL;
    LISTHEAD        *lpoHead = &(goGlobalConfigHead);
    AISmartLock     loLock(goGlobalConfigLock);

    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoIniConfig = AI_GET_STRUCT_PTR(lpoTemp, AIConfig, coConfigChild);
        if(strcmp(lpoIniConfig->csName, apcIniFileName) == 0)
        {
            return  (lpoIniConfig);
        }
    }

    AI_NEW_ASSERT(lpoIniConfig, AIConfig);
    if(lpoIniConfig->LoadINI(apcIniFileName))
    {
        AI_DELETE(lpoIniConfig);
        return  (NULL);
    }

    ai_list_add_tail(&(lpoIniConfig->coConfigChild), lpoHead);
    return  (lpoIniConfig);
}

///end namespace
AIBC_NAMESPACE_END
