#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AILogSys.h"
#include "AIWndControl.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////
//
#define  WC_DEFAULT_TIMEOUT    (5)
#define  WC_DEFAULT_WINDOWS    (5)

////////////////////////////////////////////////////////////////////
//
void *window_scan_thread(void *avOpt)
{
    clsWindowControl    *lpoMainClass = NULL;
    
    lpoMainClass = (clsWindowControl *)avOpt;
    pthread_detach(pthread_self());
    
    while(!lpoMainClass->ciShutDown)
    {
        lpoMainClass->RemoveTimeout();
        sleep(1);
    }
    
    lpoMainClass->ctThreadID = 0;
    return  (NULL);
}

////////////////////////////////////////////////////////////////////
//
clsWindowControl::clsWindowControl(size_t aiMaxWindow, int aiTimeout)
{
    ciShutDown = 0;
    ctThreadID = 0;
    cpfTimeoutFunc = NULL;

    ciTimeout = (aiTimeout<WC_DEFAULT_TIMEOUT?WC_DEFAULT_TIMEOUT:aiTimeout);
    ciMaxCount = (aiMaxWindow<WC_DEFAULT_WINDOWS?WC_DEFAULT_WINDOWS:aiMaxWindow);
    
    for(int liIt = 0; liIt < WC_MAX_INDEX_SIZE; liIt++)
    {
        ai_init_list_head(&coWndHead[liIt]);
    }
}

clsWindowControl::~clsWindowControl()
{
    if(ctThreadID)
    {
        AIWriteLOG(
            AILIB_LOGFILE,
            AILOG_LEVEL_DEBUG,
            "[%s] Call clsWindowControl::ShutDown()",
             __FUNCTION__
        );
        clsWindowControl::ShutDown();
    }
}

void clsWindowControl::Startup(void)
{
    if(ctThreadID == 0)
    {
        if(pthread_create(&ctThreadID, NULL, window_scan_thread, this) < 0)
        {
            AIWriteLOG(
                AILIB_LOGFILE, 
                AILOG_LEVEL_ERROR,
                "[%s] pthread_create(window_scan_thread) ... fail",
                __FUNCTION__
            );
            ctThreadID = 0;
        }
    }
}

void clsWindowControl::Startup(timeoutCallback apfCallback)
{
    SetCallback(apfCallback);
    clsWindowControl::Startup();
}

void clsWindowControl::ShutDown(void)
{
    LISTHEAD        *lpoHead1 = NULL;
    LISTHEAD        *lpoTemp1 = NULL;
    LISTHEAD        *lpoHead2 = NULL;
    LISTHEAD        *lpoTemp2 = NULL;
    WindowCtl       *lpoWindowCtl = NULL;
    WindowCell      *lpoWindowCell = NULL;
    
    ciShutDown = 1;
    while(ctThreadID)
        {
            sleep(1);
        }
    
    coMutexLock.Lock();

    for(int liIt = 0; liIt < WC_MAX_INDEX_SIZE; liIt++)
    {
        lpoHead1 = &coWndHead[liIt];

        while(!ai_list_is_empty(lpoHead1))
        {
            ai_list_del_head(lpoTemp1, lpoHead1);
            lpoWindowCtl = AI_GET_STRUCT_PTR(lpoTemp1, WindowCtl, coWndChild);
            
            lpoHead2 = &(lpoWindowCtl->coTimeHead);
            while(!ai_list_is_empty(lpoHead2))
            {
                ai_list_del_head(lpoTemp2, lpoHead2);
                lpoWindowCell = AI_GET_STRUCT_PTR(lpoTemp2, WindowCell, coTimeChild);
                
                AIWriteLOG(
                    AILIB_LOGFILE,
                    AILOG_LEVEL_DEBUG,
                    "[%s] Remove %u.%d",
                     __FUNCTION__, 
                    lpoWindowCtl->ciObjectID,
                    lpoWindowCell->ciSeqNo
                );
                
                ai_list_del_any(&(lpoWindowCell->coIndexChild));
                AI_DELETE(lpoWindowCell);
            }
            
            AI_DELETE(lpoWindowCtl);
        }
    }

    coMutexLock.Unlock();
    ciShutDown = 0;
}

void clsWindowControl::SetCallback(timeoutCallback apfCallback)
{
    cpfTimeoutFunc = apfCallback;
}

int clsWindowControl::_AttachWindow(unsigned int aiObjectID, unsigned int aiSeqNo)
{
    int             liHashKey;
    LISTHEAD        *lpoHead = NULL;
    LISTHEAD        *lpoTemp = NULL;
    WindowCtl       *lpoWindowCtl = NULL;
    WindowCell      *lpoWindowCell = NULL;
    
    liHashKey = aiObjectID%WC_MAX_INDEX_SIZE;
    lpoHead = &coWndHead[ liHashKey ];
    
    coMutexLock.Lock();
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoWindowCtl = AI_GET_STRUCT_PTR(lpoTemp, WindowCtl, coWndChild);
        
        if(lpoWindowCtl->ciObjectID == aiObjectID)
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_DEBUG,
                "[%s] Search %d ... Ok [%d/%d]",
                __FUNCTION__,
                aiObjectID,
                lpoWindowCtl->ciCurCount, 
                ciMaxCount
            );

            if(lpoWindowCtl->ciCurCount >= ciMaxCount)
            {
                coMutexLock.Unlock();
                return  (WC_ERROR_WINDOWS_FULL);
            }
            
            goto  ADDCELL;
        }
    }
    
    AI_NEW_ASSERT(lpoWindowCtl, WindowCtl);
    memset(lpoWindowCtl, 0, sizeof(WindowCtl));

    lpoWindowCtl->ciObjectID = aiObjectID;
    ai_init_list_head(&(lpoWindowCtl->coTimeHead));
    
    for(int liIt = 0; liIt < WC_MAX_INDEX_SIZE; liIt++)
    {
        ai_init_list_head(&(lpoWindowCtl->coIndexHead[liIt]));
    }

    ai_list_add_tail(&(lpoWindowCtl->coWndChild), lpoHead);

ADDCELL:    
    AI_NEW_ASSERT(lpoWindowCell, WindowCell);
    lpoWindowCell->ciObjectID = aiObjectID;
    lpoWindowCell->ctEnterTime = time(NULL);
    lpoWindowCell->ciSeqNo = aiSeqNo;

    lpoHead = &(lpoWindowCtl->coTimeHead);          
    ai_list_add_tail(&(lpoWindowCell->coTimeChild), lpoHead);
            
    liHashKey = aiSeqNo%WC_MAX_INDEX_SIZE;
    lpoHead = &(lpoWindowCtl->coIndexHead[liHashKey]);
    ai_list_add_tail(&(lpoWindowCell->coIndexChild), lpoHead);

    lpoWindowCtl->ciCurCount += 1;
    coMutexLock.Lock();
    
    return  (0);
}

int clsWindowControl::AttachWindow(void const* avNetID, unsigned int aiSeqNo)
{
    unsigned int        liObjectID;
    
    liObjectID = (unsigned int)avNetID;
    return  _AttachWindow(liObjectID, aiSeqNo);
}

int clsWindowControl::AttachWindow(int aiSocketID, unsigned int aiSeqNo)
{
    unsigned int        liObjectID;
    
    liObjectID = (unsigned int)aiSocketID;
    return  _AttachWindow(liObjectID, aiSeqNo);
}

int clsWindowControl::_DetachWindow(unsigned int aiObjectID, unsigned int aiSeqNo)
{
    int             liHashKey;
    LISTHEAD        *lpoHead = NULL;
    LISTHEAD        *lpoTemp = NULL;
    WindowCtl       *lpoWindowCtl = NULL;
    WindowCell      *lpoWindowCell = NULL;
    
    liHashKey = aiObjectID%WC_MAX_INDEX_SIZE;
    lpoHead = &coWndHead[ liHashKey ];
    
    coMutexLock.Lock();
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoWindowCtl = AI_GET_STRUCT_PTR(lpoTemp, WindowCtl, coWndChild);
        
        if(lpoWindowCtl->ciObjectID == aiObjectID)
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_DEBUG,
                "[%s] Search %d ... Ok",
                __FUNCTION__, 
                aiObjectID
            );

            goto  FINDCELL;
        }
    }
    
    coMutexLock.Unlock();
    return  (WC_ERROR_INVALID_CELL);    

FINDCELL:
    liHashKey = aiSeqNo % WC_MAX_INDEX_SIZE;
    lpoHead = &(lpoWindowCtl->coIndexHead[liHashKey]);
    
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoWindowCell = AI_GET_STRUCT_PTR(lpoTemp, WindowCell, coIndexChild);
        
        if(lpoWindowCell->ciSeqNo == aiSeqNo)
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_DEBUG,
                "[%s] Search [%d] ... Ok", 
                __FUNCTION__, 
                aiSeqNo
            );

            ai_list_del_any(&(lpoWindowCell->coTimeChild));
            ai_list_del_any(&(lpoWindowCell->coIndexChild));
            
            lpoWindowCtl->ciCurCount -= 1;
            coMutexLock.Unlock();

            AI_DELETE(lpoWindowCell);
            return  (0);
        }   
    }
        
    coMutexLock.Unlock();
    return  (WC_ERROR_INVALID_CELL);    
}

int clsWindowControl::DetachWindow(void const* avNetID, unsigned int aiSeqNo)
{
    unsigned int        liObjectID;
    
    liObjectID = (unsigned int)avNetID;
    return  _DetachWindow(liObjectID, aiSeqNo);
}

int clsWindowControl::DetachWindow(int aiSocketID, unsigned int aiSeqNo)
{
    unsigned int        liObjectID;
    
    liObjectID = (unsigned int)aiSocketID;
    return  _DetachWindow(liObjectID, aiSeqNo);
}

int clsWindowControl::_RemoveWindow(unsigned int aiObjectID)
{
    int             liCount;
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    WindowCtl       *lpoWindowCtl = NULL;
    WindowCell      *lpoWindowCell = NULL;

    liCount = 0;

    coMutexLock.Lock();

    for(int liIt = 0; liIt < WC_MAX_INDEX_SIZE; liIt++)
    {
        lpoHead = &coWndHead[liIt];
        
        for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
        {
            lpoWindowCtl = AI_GET_STRUCT_PTR(lpoTemp, WindowCtl, coWndChild);
            
            if(lpoWindowCtl->ciObjectID == aiObjectID)
            {
                AIWriteLOG(
                    AILIB_LOGFILE,
                    AILOG_LEVEL_DEBUG,
                    "[%s] Search %d ... Ok", 
                    __FUNCTION__, 
                    aiObjectID
                );

                ai_list_del_any(&(lpoWindowCtl->coWndChild));
                coMutexLock.Unlock();
                
                goto  DELCELL;
            }
        }
    }
    
    coMutexLock.Unlock();
    return  (WC_ERROR_INVALID_OBJID);

DELCELL:
    lpoHead = &(lpoWindowCtl->coTimeHead);
    while(!ai_list_is_empty(lpoHead))
    {
        ai_list_del_head(lpoTemp, lpoHead);
        lpoWindowCell = AI_GET_STRUCT_PTR(lpoTemp, WindowCell, coTimeChild);
        
        if(cpfTimeoutFunc)
        {
            AIWriteLOG(
                AILIB_LOGFILE, 
                AILOG_LEVEL_DEBUG,
                "[%s] Call cpfTimeoutFunc", 
                __FUNCTION__
            );

            cpfTimeoutFunc(lpoWindowCell->ciObjectID, lpoWindowCell->ciSeqNo);
        }
        
        AI_DELETE(lpoWindowCell);
        liCount += 1;
    }

    AI_DELETE(lpoWindowCtl);
    return  (liCount);
}

int clsWindowControl::RemoveWindow(void const* avNetID)
{
    unsigned int        liObjectID;
    
    liObjectID = (unsigned int)avNetID;
    return  _RemoveWindow(liObjectID);
}

int clsWindowControl::RemoveWindow(int aiSocketID)
{
    unsigned int        liObjectID;
    
    liObjectID = (unsigned int)aiSocketID;
    return  _RemoveWindow(liObjectID);
}
    
int clsWindowControl::RemoveTimeout(void)
{
    int             liCount;
    time_t          ltCurrTime;
    LISTHEAD        loTempHead;
    LISTHEAD        *lpoTemp1 = NULL;
    LISTHEAD        *lpoHead1 = NULL;
    LISTHEAD        *lpoTemp2 = NULL;
    LISTHEAD        *lpoHead2 = NULL;
    WindowCtl       *lpoWindowCtl = NULL;
    WindowCell      *lpoWindowCell = NULL;

    liCount = 0;
    ltCurrTime = time(NULL);
    ai_init_list_head(&loTempHead);
    
    for(int liIt = 0; liIt < WC_MAX_INDEX_SIZE; liIt++)
    {
        lpoHead1 = &coWndHead[liIt];
        coMutexLock.Lock();
        
        for(lpoTemp1 = lpoHead1->cpNext; lpoTemp1 != lpoHead1; lpoTemp1 = lpoTemp1->cpNext)
        {
            lpoWindowCtl = AI_GET_STRUCT_PTR(lpoTemp1, WindowCtl, coWndChild);
            lpoHead2 = &(lpoWindowCtl->coTimeHead);

AGAIN:          
            for(lpoTemp2 = lpoHead2->cpNext; lpoTemp2 != lpoHead2; lpoTemp2 = lpoTemp2->cpNext)
            {
                lpoWindowCell = AI_GET_STRUCT_PTR(lpoTemp2, WindowCell, coTimeChild);

                if((ltCurrTime-lpoWindowCell->ctEnterTime) >= ciTimeout)
                {
                    ai_list_del_any(&(lpoWindowCell->coTimeChild));
                    ai_list_del_any(&(lpoWindowCell->coIndexChild));
                    ai_list_add_tail(&(lpoWindowCell->coTimeChild), &loTempHead);

                    liCount += 1;
                    goto AGAIN;
                }
                
                break;
            }
        }
        
        coMutexLock.Unlock();
    }
    
    while(!ai_list_is_empty(&loTempHead))
    {
        ai_list_del_head(lpoTemp1, &loTempHead);
        lpoWindowCell = AI_GET_STRUCT_PTR(lpoTemp1, WindowCell, coTimeChild);
        
        if(cpfTimeoutFunc)
        {
            AIWriteLOG(
                AILIB_LOGFILE,
                AILOG_LEVEL_DEBUG,
                "[%s] Call cpfTimeoutFunc", 
                __FUNCTION__
            );

            cpfTimeoutFunc(lpoWindowCell->ciObjectID, lpoWindowCell->ciSeqNo);
        }
        
        AI_DELETE(lpoWindowCell);
    }

    return  (liCount);
}

///end namespace
AIBC_NAMESPACE_END
