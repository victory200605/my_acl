#ifndef  __AILIBEX__AIADVFILEQUEUE_H__
#define  __AILIBEX__AIADVFILEQUEUE_H__
#include "AILib.h"
#include "AIDualLink.h"
#include "AISynch.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////
//
#define  AFQ_ERROR_NO_RECORD        (-90400)
#define  AFQ_ERROR_QUEUE_FULL       (-90401)
#define  AFQ_ERROR_INV_PARAM        (-90410)
#define  AFQ_ERROR_INV_FILE         (-90411)
#define  AFQ_ERROR_FSEEKFAIL        (-90412)
#define  AFQ_ERROR_WRITEFAIL        (-90413)
#define  AFQ_ERROR_READFAIL         (-90414)
#define  AFQ_ERROR_OPENFILE         (-90415)
#define  AFQ_ERROR_MAGICNO          (-90416)
#define  AFQ_ERROR_NOTFOUND         (-90417)

////////////////////////////////////////////////////////////////////
//
#define  AFQ_MAX_RETRY_COUNT           (16)
#define  AFQ_MAX_QUEUECOUNT           (128)
#define  AFQ_INFO_HASH_SIZE            (97)
#define  AFQ_INDX_HASH_SIZE          (1777)
#define  AFQ_FLQMAGIC_NUMBER     (0x123888)

////////////////////////////////////////////////////////////////////
//
typedef int (*advfqGetQueueSize)(const char*);

#define  AFQ_CLOSE_FILE(file) \
    { close(file); file = -1; }

////////////////////////////////////////////////////////////////////
//
typedef struct stAdvQueueInfo
{
    int             ciQueueID;
    char            csQueueName[64];
} AdvQueueInfo;

typedef struct stAdvFileHead
{
    int             ciMagic;
    int             ciBlkSize;
    int             ciBlkCount;
    AdvQueueInfo    coQInfo[AFQ_MAX_QUEUECOUNT];
} AdvFileHead;

typedef struct stAdvMemNode
{
    int             ciQueueID;
    int             ciDataSize;
    
    int             ciNodeOffset;
    LISTHEAD        coCommonChild;
} AdvMemNode;

typedef struct stAdvQueueHead
{
    int             ciQueueID;
    int             ciNodeCount;
    char            csQueueName[64];
    
    LISTHEAD        coQIDChild;
    LISTHEAD        coQNameChild;
    LISTHEAD        coMemNodeHead;
} AdvQueueHead;

////////////////////////////////////////////////////////////////////
//
class clsAdvFileQueue
{
private:
    int             ciFileID;
    AdvFileHead*    cpoFlHead;
    AIMutexLock     coMutexLock;
    advfqGetQueueSize  cpfGetQueueSize;
        
private:
    LISTHEAD    coFreeNodeHead;
    LISTHEAD*   cpoPendingHead;
    LISTHEAD    coQueueIDHead[AFQ_INFO_HASH_SIZE];
    LISTHEAD    coQueueNameHead[AFQ_INFO_HASH_SIZE];

public:
    clsAdvFileQueue(void);
    ~clsAdvFileQueue();

private:
    int GetEmtpyQueue(void);
    int GetNextQueueID(void);
    void MultiLevelMkDir(char const* apcFullFileName);
    int WriteQueueInfo(AdvQueueHead* apoQueueHead, int aiArrayIndex);
    int LoadQueueInfo(AdvFileHead* apoFileHead);
    int LoadRecord(AdvFileHead* apoFileHead);

private:
    AdvMemNode *GetFreeNode(void);
    AdvQueueHead *SearchQueue(int aiQueueID);
    AdvQueueHead *SearchQueue(char const*apcQueueName, int aiCreated);
    
public:
    int ShutDown(void);
    int RemoveRecord(int aiGlobalID);
    int AddRecord(char const* apcQueueName, char const* apcData, int aiSize);
    int GetRecord(char const* apcQueueName, int& aiGlobalID, char* apcData, int &aiSize);
    int PutRecord(int aiGlobalID);
    int LoadFileQueue(char const* apcFileName, int aiBlkSize, int aiBlkCount);  
    void SetCallback(advfqGetQueueSize apfCallback);  
};

///end namespace
AIBC_NAMESPACE_END

#endif   // __AILIBEX__AIADVFILEQUEUE_H__

