#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "AIString.h"
#include "AIAdvFileQueue.h"

///start namespace
AIBC_NAMESPACE_START

clsAdvFileQueue::clsAdvFileQueue(void)
{
    cpoFlHead = NULL;
    cpfGetQueueSize = NULL;
    AFQ_CLOSE_FILE(ciFileID);

    AI_NEW_ASSERT(cpoFlHead, AdvFileHead);
    AI_NEW_N_ASSERT(cpoPendingHead, LISTHEAD, AFQ_INDX_HASH_SIZE);
    
    ai_init_list_head(&coFreeNodeHead);
    for(int liIt = 0; liIt < AFQ_INFO_HASH_SIZE; liIt++)
    {
        ai_init_list_head(&(coQueueIDHead[liIt]));
        ai_init_list_head(&(coQueueNameHead[liIt]));
    }
    
    for(int liIk = 0; liIk < AFQ_INDX_HASH_SIZE; liIk++)
    {
        ai_init_list_head(&(cpoPendingHead[liIk]));
    }
}

clsAdvFileQueue::~clsAdvFileQueue()
{
    AI_DELETE_N(cpoPendingHead);
    AI_DELETE(cpoFlHead);
}

int clsAdvFileQueue::ShutDown(void)
{
    int             liCount = 0;
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    AdvMemNode      *lpoMemNode = NULL;
    AdvQueueHead    *lpoQueueHead = NULL;
    
    AFQ_CLOSE_FILE(ciFileID);
    
    lpoHead = &coFreeNodeHead;
    while(!ai_list_is_empty(lpoHead))
    {
        ai_list_del_head(lpoTemp, lpoHead);
        lpoMemNode = AI_GET_STRUCT_PTR(lpoTemp, AdvMemNode, coCommonChild);
        AI_DELETE(lpoMemNode);
        liCount += 1;
    }
    
    for(int liIt = 0; liIt < AFQ_INFO_HASH_SIZE; liIt++)
    {
        lpoHead = &(coQueueIDHead[liIt]);
        while(!ai_list_is_empty(lpoHead))
        {
            ai_list_del_head(lpoTemp, lpoHead);
            lpoQueueHead = AI_GET_STRUCT_PTR(lpoTemp, AdvQueueHead, coQIDChild);
            ai_list_del_any(&(lpoQueueHead->coQNameChild));

            while(!ai_list_is_empty(&(lpoQueueHead->coMemNodeHead)))
            {
                ai_list_del_head(lpoTemp, &(lpoQueueHead->coMemNodeHead));
                lpoMemNode = AI_GET_STRUCT_PTR(lpoTemp, AdvMemNode, coCommonChild);
                AI_DELETE(lpoMemNode);
                liCount += 1;
            }
                        
            AI_DELETE(lpoQueueHead);
        }
    }

    for(int liIt = 0; liIt < AFQ_INDX_HASH_SIZE; liIt++)
    {
        lpoHead = &(cpoPendingHead[liIt]);
        while(!ai_list_is_empty(lpoHead))
        {
            ai_list_del_head(lpoTemp, lpoHead);
            lpoMemNode = AI_GET_STRUCT_PTR(lpoTemp, AdvMemNode, coCommonChild);
            AI_DELETE(lpoMemNode);
            liCount += 1;
        }
    }

    return  (liCount);
}

void clsAdvFileQueue::SetCallback(advfqGetQueueSize apfCallback)
{
    cpfGetQueueSize = apfCallback;
}

void clsAdvFileQueue::MultiLevelMkDir(char const* apcFullFileName)
{
    char        lsMkDir[128] = {0};
    char const* lpcPtr = apcFullFileName;

    while((lpcPtr = strchr(lpcPtr, '/')) != NULL)
    {
        if((lpcPtr - apcFullFileName) > 0)
        {
            memcpy(lsMkDir, apcFullFileName, lpcPtr-apcFullFileName);
            lsMkDir[lpcPtr-apcFullFileName] = '\0';
            mkdir(lsMkDir, 0700);
        }

        lpcPtr += 1;
    }
}

AdvQueueHead* clsAdvFileQueue::SearchQueue(int aiQueueID)
{
    LISTHEAD*       lpoTemp = NULL;
    LISTHEAD*       lpoHead = NULL;
    AdvQueueHead*   lpoQueueHead = NULL;
    
    lpoHead = &(coQueueIDHead[ aiQueueID % AFQ_INFO_HASH_SIZE ]);
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoQueueHead = AI_GET_STRUCT_PTR(lpoTemp, AdvQueueHead, coQIDChild);
        
        if(lpoQueueHead->ciQueueID == aiQueueID)
            return  (lpoQueueHead);
    }
    
    return  (NULL);
}

int clsAdvFileQueue::GetNextQueueID(void)
{
    int             liQueueID = 999;
    LISTHEAD*       lpoTemp = NULL;
    LISTHEAD*       lpoHead = NULL;
    AdvQueueHead*   lpoQueueHead = NULL;
    
    for(int liIt = 0; liIt < AFQ_INFO_HASH_SIZE; liIt++)
    {
        lpoHead = &(coQueueIDHead[liIt]);
        
        for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
        {
            lpoQueueHead = AI_GET_STRUCT_PTR(lpoTemp, AdvQueueHead, coQIDChild);
            
            if(lpoQueueHead->ciQueueID > liQueueID)
            {
                liQueueID = lpoQueueHead->ciQueueID;
            }
        }
    }

#ifdef  DEBUG
    fprintf(stderr, "%s:  New QueueID = %d\r\n", __FUNCTION__, liQueueID + 1);
#endif  

    return  (liQueueID + 1);
}

int clsAdvFileQueue::WriteQueueInfo(AdvQueueHead* apoQueueHead, int aiArrayIndex)
{
    int             liOffset;
    int             liWriteSize;
    AdvQueueInfo*   lpoQueueInfo = NULL;
    
    lpoQueueInfo = &(cpoFlHead->coQInfo[aiArrayIndex]);

    lpoQueueInfo->ciQueueID = htonl(apoQueueHead->ciQueueID);
    strcpy(lpoQueueInfo->csQueueName, apoQueueHead->csQueueName);

    liWriteSize = (int)sizeof(AdvQueueInfo);
    liOffset = (char*)lpoQueueInfo - (char*)cpoFlHead;

    if(lseek(ciFileID, liOffset, SEEK_SET) == liOffset)
    {
        if(write(ciFileID, lpoQueueInfo, liWriteSize) == liWriteSize)
        {
            lpoQueueInfo->ciQueueID = ntohl(apoQueueHead->ciQueueID);
            return  (0);
        }
    }
    
    memset(lpoQueueInfo, 0, sizeof(AdvQueueInfo));
    return  (AFQ_ERROR_WRITEFAIL);
}

int clsAdvFileQueue::GetEmtpyQueue(void)
{
    AdvQueueInfo*   lpoQueueInfo = NULL;

    for(int liIt = 0; liIt < AFQ_MAX_QUEUECOUNT; liIt++)
    {
        lpoQueueInfo = &(cpoFlHead->coQInfo[liIt]);
        
        if(lpoQueueInfo->ciQueueID <= 0)
        {   
            return  (liIt);
        }
    }
    
    return  (AFQ_ERROR_NOTFOUND);
}

AdvQueueHead* clsAdvFileQueue::SearchQueue(char const* apcQueueName, int aiCreated)
{
    int             liHashKey;
    int             liArrayIndex;
    LISTHEAD*       lpoTemp = NULL;
    LISTHEAD*       lpoHead = NULL;
    AdvQueueHead*   lpoQueueHead = NULL;
    
    liHashKey = GetStringHashKey1(apcQueueName, AFQ_INFO_HASH_SIZE);
    lpoHead = &(coQueueNameHead[ liHashKey ]);
    
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoQueueHead = AI_GET_STRUCT_PTR(lpoTemp, AdvQueueHead, coQNameChild);
        
        if(strcmp(lpoQueueHead->csQueueName, apcQueueName) == 0)
            return  (lpoQueueHead);
    }
    
    if(aiCreated)
    {
        if((liArrayIndex = GetEmtpyQueue()) >= 0)
        {
            AI_NEW_ASSERT(lpoQueueHead, AdvQueueHead);
            memset(lpoQueueHead, 0, sizeof(AdvQueueHead));
            
            lpoQueueHead->ciQueueID = GetNextQueueID();
            strcpy(lpoQueueHead->csQueueName, apcQueueName);
            
            ai_init_list_head(&(lpoQueueHead->coMemNodeHead));
            ai_list_add_tail(&(lpoQueueHead->coQNameChild), lpoHead);
            
            liHashKey = lpoQueueHead->ciQueueID % AFQ_INFO_HASH_SIZE;
            ai_list_add_tail(&(lpoQueueHead->coQIDChild), &(coQueueIDHead[liHashKey]));
            
            if(WriteQueueInfo(lpoQueueHead, liArrayIndex))
            {
#ifdef  DEBUG
                fprintf(stderr, "%s:  WriteQueueInfo(%d) ... Fail\r\n", \
                    __FUNCTION__, liArrayIndex);
#endif
                
                ai_list_del_any(&(lpoQueueHead->coQIDChild));
                ai_list_del_any(&(lpoQueueHead->coQNameChild));
                
                AI_DELETE(lpoQueueHead);
                return  (NULL);
            }
                
#ifdef  DEBUG
            fprintf(stderr, "%s:  WriteQueueInfo(%d) ... Ok\r\n", \
                __FUNCTION__, liArrayIndex);
#endif

            return  (lpoQueueHead);
        }
    }
    
    return  (NULL);
}

AdvMemNode* clsAdvFileQueue::GetFreeNode(void)
{
    LISTHEAD*   lpoTemp = NULL;
    LISTHEAD*   lpoHead = NULL;
    AdvMemNode* lpoMemNode = NULL;
    
    lpoHead = &coFreeNodeHead;
    if(ai_list_del_head(lpoTemp, lpoHead), lpoTemp)
    {
        lpoMemNode = AI_GET_STRUCT_PTR(lpoTemp, AdvMemNode, coCommonChild);
        return  (lpoMemNode);
    }
    
    return  (NULL);
}

int clsAdvFileQueue::RemoveRecord(int aiGlobalID)
{
    int             liOffset;
    int             liTempVal = 0;
    LISTHEAD*       lpoTemp = NULL;
    LISTHEAD*       lpoHead = NULL;
    AdvMemNode*     lpoMemNode = NULL;
    AdvQueueHead*   lpoQueueHead = NULL;
    
    if(aiGlobalID <= 0)
    {
        return  (AFQ_ERROR_INV_PARAM);
    }

    AISmartLock loLock(coMutexLock);
    if(ciFileID < 0)
    {
        return  (AFQ_ERROR_INV_FILE);
    }

    lpoHead = &(cpoPendingHead[aiGlobalID % AFQ_INDX_HASH_SIZE]);
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoMemNode = AI_GET_STRUCT_PTR(lpoTemp, AdvMemNode, coCommonChild);
        
        if(lpoMemNode->ciNodeOffset == aiGlobalID)
        {
            lpoQueueHead = SearchQueue(lpoMemNode->ciQueueID);
            lpoQueueHead->ciNodeCount -= 1;
            
            ai_list_del_any(&(lpoMemNode->coCommonChild));
            ai_list_add_tail(&(lpoMemNode->coCommonChild), &coFreeNodeHead);
            
            liOffset = lpoMemNode->ciNodeOffset;
            if(lseek(ciFileID, liOffset, SEEK_SET) == liOffset)
            {
                if(write(ciFileID, &liTempVal, sizeof(int)) == sizeof(int))
                {
                    return  (0);
                }
            }

#ifdef  DEBUG
            fprintf(stderr, "%s:  Erase Node(%d).ciQueueID ... Fail\r\n", \
                __FUNCTION__, liOffset);
#endif
            
            return  (0);
        }
    }
    
    return  (AFQ_ERROR_NO_RECORD);
}

int clsAdvFileQueue::AddRecord(char const* apcQueueName, char const* apcData, int aiSize)
{
    int             liTempVal;
    int             liWiteSize;
    int             liFileOffset;
    char            lsQueueName[64];
    char*           lpcBuffer = NULL;
    AdvMemNode*     lpoMemNode = NULL;
    AdvQueueHead*   lpoQueueHead = NULL;
    
    if(apcData == NULL || aiSize <= 0)
    {
        return  (AFQ_ERROR_INV_PARAM);
    }

    if(aiSize > (int)(ntohl(cpoFlHead->ciBlkSize)-sizeof(int)*2))
    {
        return  (AFQ_ERROR_INV_PARAM);
    }
    strcpy(lsQueueName, (apcQueueName?apcQueueName:"DEFAULT"));
                
    AISmartLock loSmartLock(coMutexLock);
    if(ciFileID < 0)
    {
        return  (AFQ_ERROR_INV_FILE);
    }
    
    if((lpoMemNode = GetFreeNode()) == NULL)
    {
        return  (AFQ_ERROR_QUEUE_FULL);
    }
    
    if((lpoQueueHead = SearchQueue(lsQueueName, 1)) == NULL)
    {
        ai_list_add_tail(&(lpoMemNode->coCommonChild), &coFreeNodeHead);
        return  (AFQ_ERROR_QUEUE_FULL);
    }

    if(cpfGetQueueSize)
    {
        if(lpoQueueHead->ciNodeCount >= cpfGetQueueSize(lsQueueName))
        {
            ai_list_add_tail(&(lpoMemNode->coCommonChild), &coFreeNodeHead);
            return  (AFQ_ERROR_QUEUE_FULL);
        }
    }
    
    lpoMemNode->ciDataSize = aiSize;
    lpoMemNode->ciQueueID = lpoQueueHead->ciQueueID;

    liWiteSize = aiSize + (int)(sizeof(int) * 2);
    AI_NEW_N_ASSERT(lpcBuffer, char, liWiteSize);

    liTempVal = htonl(lpoQueueHead->ciQueueID);
    memcpy(lpcBuffer+0, &liTempVal, sizeof(int));

    liTempVal = htonl(lpoMemNode->ciDataSize);
    memcpy(lpcBuffer+sizeof(int), &liTempVal, sizeof(int));

    memcpy(lpcBuffer+sizeof(int)*2, apcData, aiSize);

    liFileOffset = lpoMemNode->ciNodeOffset;
    errno = EINTR;
    
    for(int liIt = 0; (liIt < AFQ_MAX_RETRY_COUNT) && (errno == EINTR); liIt++)
    {
        if(lseek(ciFileID, liFileOffset, SEEK_SET) == liFileOffset)
        {
            if(write(ciFileID, lpcBuffer, liWiteSize) == liWiteSize)
            {
                ai_list_add_tail(&(lpoMemNode->coCommonChild), &(lpoQueueHead->coMemNodeHead));
                lpoQueueHead->ciNodeCount += 1;
    
                AI_DELETE_N(lpcBuffer);
                return  (0);
            }
        }
    }
    
    ai_list_add_tail(&(lpoMemNode->coCommonChild), &coFreeNodeHead);

    AI_DELETE_N(lpcBuffer);
    return  (AFQ_ERROR_WRITEFAIL);
}

int clsAdvFileQueue::GetRecord(char const* apcQueueName, int &aiGlobalID, char* apcData, int &aiSize)
{
    int             liHashKey;
    int             liReadSize;
    int             liFileOffset;
    char            lsQueueName[64];    
    LISTHEAD*       lpoTemp = NULL;
    LISTHEAD*       lpoHead = NULL;
    AdvMemNode*     lpoMemNode = NULL;
    AdvQueueHead*   lpoQueueHead = NULL;
    
    if(apcData == NULL || aiSize <= 0)
    {
        return  (AFQ_ERROR_INV_PARAM);
    }

    strcpy(lsQueueName, (apcQueueName?apcQueueName:"DEFAULT"));

    AISmartLock loSmartLock(coMutexLock);
    if(ciFileID < 0)
    {
        return  (AFQ_ERROR_INV_FILE);
    }

    if((lpoQueueHead = SearchQueue(lsQueueName, 0)) == NULL)
    {
        return  (AFQ_ERROR_NO_RECORD);
    }
    
    lpoHead = &(lpoQueueHead->coMemNodeHead);
    if(ai_list_del_head(lpoTemp, lpoHead), !lpoTemp)
    {
        return  (AFQ_ERROR_NO_RECORD);
    }
    
    lpoMemNode = AI_GET_STRUCT_PTR(lpoTemp, AdvMemNode, coCommonChild);

    liReadSize = ((aiSize<lpoMemNode->ciDataSize)?aiSize:lpoMemNode->ciDataSize);
    liFileOffset = lpoMemNode->ciNodeOffset + (int)(sizeof(int) * 2);
    errno = EINTR;
    
    for(int liIt = 0; (liIt < AFQ_MAX_RETRY_COUNT) && (errno == EINTR); liIt++)
    {
        if(lseek(ciFileID, liFileOffset, SEEK_SET) == liFileOffset)
        {
            if(read(ciFileID, apcData, liReadSize) == liReadSize)
            {
                aiSize = liReadSize;
                aiGlobalID = lpoMemNode->ciNodeOffset;
                liHashKey = aiGlobalID % AFQ_INDX_HASH_SIZE;
                ai_list_add_tail(lpoTemp, &(cpoPendingHead[ liHashKey ]));
    
                return  (0);
            }
        }
    }

    ai_list_add_tail(lpoTemp, &coFreeNodeHead);
    lpoQueueHead->ciNodeCount -= 1;

    return  (AFQ_ERROR_READFAIL);
}

int clsAdvFileQueue::PutRecord(int aiGlobalID)
{
    LISTHEAD*       lpoTemp = NULL;
    LISTHEAD*       lpoHead = NULL;
    AdvMemNode*     lpoMemNode = NULL;
    AdvQueueHead*   lpoQueueHead = NULL;
    
    if(aiGlobalID <= 0)
        return  (AFQ_ERROR_INV_PARAM);

    AISmartLock loSmartLock(coMutexLock);
    if(ciFileID < 0)
    {
        return  (AFQ_ERROR_INV_FILE);
    }

    lpoHead = &(cpoPendingHead[aiGlobalID % AFQ_INDX_HASH_SIZE]);
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoMemNode = AI_GET_STRUCT_PTR(lpoTemp, AdvMemNode, coCommonChild);
        
        if(lpoMemNode->ciNodeOffset == aiGlobalID)
        {
            lpoQueueHead = SearchQueue(lpoMemNode->ciQueueID);
            lpoHead = &(lpoQueueHead->coMemNodeHead);

            ai_list_del_any(&(lpoMemNode->coCommonChild));
            ai_list_add_tail(&(lpoMemNode->coCommonChild), lpoHead);
            
            return  (0);
        }
    }
    
    return  (AFQ_ERROR_NO_RECORD);
}

int clsAdvFileQueue::LoadRecord(AdvFileHead* apoFileHead)
{
    int             liReadSize;
    int             liCount = 0;
    int             liOffset = 0;
    int             liBlkSize = 0;
    int             liBlkCount = 0;
    LISTHEAD*       lpoHead = NULL;
    AdvMemNode*     lpoMemNode = NULL;
    AdvQueueHead*   lpoQueueHead = NULL;

    liBlkSize = ntohl(apoFileHead->ciBlkSize);    
    liBlkCount = ntohl(apoFileHead->ciBlkCount);
    
    for(int liIt = 0; liIt < liBlkCount; liIt++)
    {
        liOffset = sizeof(AdvFileHead) + (liIt * liBlkSize);
        
        if(lseek(ciFileID, liOffset, SEEK_SET) == liOffset)
        {
            liReadSize = (int)(sizeof(int) * 2);
            AI_NEW_ASSERT(lpoMemNode, AdvMemNode);
            
            if(read(ciFileID, lpoMemNode, liReadSize) == liReadSize)
            {
                lpoMemNode->ciNodeOffset = liOffset;
                lpoMemNode->ciQueueID = ntohl(lpoMemNode->ciQueueID);
                lpoMemNode->ciDataSize = ntohl(lpoMemNode->ciDataSize);

                if(lpoMemNode->ciQueueID > 0)
                {               
                    if((lpoQueueHead = SearchQueue(lpoMemNode->ciQueueID)))
                    {
                        lpoQueueHead->ciNodeCount += 1;
                        lpoHead = &(lpoQueueHead->coMemNodeHead);
                        ai_list_add_tail(&(lpoMemNode->coCommonChild), lpoHead);
                        
                        liCount += 1;
                        continue;
                    }
                }

                ai_list_add_tail(&(lpoMemNode->coCommonChild), &coFreeNodeHead);
                continue;
            }
            
            AI_DELETE(lpoMemNode);
            break;
        }
        
        break;
    }
    
    return  (liCount);
}

int clsAdvFileQueue::LoadQueueInfo(AdvFileHead* apoFileHead)
{
    int             liHashKey;
    int             liCount = 0;
    AdvQueueHead*   lpoQueueHead= NULL;
    AdvQueueInfo*   lpoQueueInfo = NULL;
    
    for(int liIt = 0; liIt < AFQ_MAX_QUEUECOUNT; liIt++)
    {
        lpoQueueInfo = &(apoFileHead->coQInfo[liIt]);
        if(ntohl(lpoQueueInfo->ciQueueID) > 0 && lpoQueueInfo->csQueueName[0])
        {
            liCount += 1;
            
            AI_NEW_ASSERT(lpoQueueHead, AdvQueueHead);
            memset(lpoQueueHead, 0, sizeof(AdvQueueHead));
            
            ai_init_list_head(&(lpoQueueHead->coMemNodeHead));
            lpoQueueHead->ciQueueID = ntohl(lpoQueueInfo->ciQueueID);
            strcpy(lpoQueueHead->csQueueName, lpoQueueInfo->csQueueName);
            
            liHashKey = GetStringHashKey1(lpoQueueHead->csQueueName, AFQ_INFO_HASH_SIZE);
            ai_list_add_tail(&(lpoQueueHead->coQNameChild), &(coQueueNameHead[liHashKey]));
            
            liHashKey = lpoQueueHead->ciQueueID % AFQ_INFO_HASH_SIZE;
            ai_list_add_tail(&(lpoQueueHead->coQIDChild), &(coQueueIDHead[liHashKey]));
        }
    }
    
    return  (liCount);
}

int clsAdvFileQueue::LoadFileQueue(char const* apcFileName, int aiBlkSize, int aiBlkCount)
{
    int     liBlkSize;
    int     liBlkCount;
    int     liReadSize;
    int     liWriteSize;
    char*   lpcBuffer = NULL;
    
    if(apcFileName == NULL || aiBlkSize <= 0 || aiBlkCount <= 0)
        return  (AFQ_ERROR_INV_PARAM);
    
    liBlkSize = aiBlkSize + (int)(sizeof(int)*2);
    liBlkSize = (liBlkSize + 7) & (~7);

    liBlkCount = (aiBlkCount + 7) & (~7);
    clsAdvFileQueue::MultiLevelMkDir(apcFileName);

    if((ciFileID = open(apcFileName, (O_CREAT|O_EXCL|O_RDWR), 0600)) < 0)
    {
        if((ciFileID = open(apcFileName, (O_CREAT|O_RDWR), 0600)) < 0)
        {
            return  (AFQ_ERROR_OPENFILE);
        }

        lseek(ciFileID, 0, SEEK_SET);
        liReadSize = (int)sizeof(AdvFileHead);
        
        if(read(ciFileID, cpoFlHead, liReadSize) != liReadSize)
        {
            AFQ_CLOSE_FILE(ciFileID);
            return  (AFQ_ERROR_READFAIL);
        }

        if(cpoFlHead->ciMagic != (int)htonl(AFQ_FLQMAGIC_NUMBER))
        {
            AFQ_CLOSE_FILE(ciFileID);
            return  (AFQ_ERROR_MAGICNO);
        }
        
        if(cpoFlHead->ciBlkSize != (int)htonl(liBlkSize))
        {
            AFQ_CLOSE_FILE(ciFileID);
            return  (AFQ_ERROR_MAGICNO);
        }
        
        if(cpoFlHead->ciBlkCount != (int)htonl(liBlkCount))
        {
            AFQ_CLOSE_FILE(ciFileID);
            return  (AFQ_ERROR_MAGICNO);
        }
        
        LoadQueueInfo(cpoFlHead);
        LoadRecord(cpoFlHead);
        return  (0);
    }

    memset(cpoFlHead, 0, sizeof(AdvFileHead));
    cpoFlHead->ciMagic = htonl(AFQ_FLQMAGIC_NUMBER);
    cpoFlHead->ciBlkCount = htonl(liBlkCount);
    cpoFlHead->ciBlkSize = htonl(liBlkSize);
    
    lseek(ciFileID, 0, SEEK_SET);
    liWriteSize = (int)sizeof(AdvFileHead);
    
    if(write(ciFileID, cpoFlHead, liWriteSize) != liWriteSize)
    {
        AFQ_CLOSE_FILE(ciFileID);
        remove(apcFileName);

        return  (AFQ_ERROR_WRITEFAIL);
    }
    
    liWriteSize = (liBlkSize * 8);
    AI_NEW_N_ASSERT(lpcBuffer, char, liWriteSize);
    memset(lpcBuffer, 0, liWriteSize);
    
    for(int liIt = 0; liIt < liBlkCount; liIt += 8)
    {
        if(write(ciFileID, lpcBuffer, liWriteSize) != liWriteSize)
        {
            AFQ_CLOSE_FILE(ciFileID);
            remove(apcFileName);

            AI_DELETE_N(lpcBuffer);
            return  (AFQ_ERROR_WRITEFAIL);
        }
    }
    
    fsync(ciFileID);
    AI_DELETE_N(lpcBuffer);
    LoadRecord(cpoFlHead);

    return  (0);
}

///end namespace
AIBC_NAMESPACE_END
