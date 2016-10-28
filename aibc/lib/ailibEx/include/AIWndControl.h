#ifndef __AILIBEX__AIWNDCONTROL_H__
#define __AILIBEX__AIWNDCONTROL_H__

#include "AILib.h"
#include "AIDualLink.h"
#include "AISynch.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////
//
#define  WC_MAX_INDEX_SIZE           (97)

#define  WC_ERROR_WINDOWS_FULL    (-90100)
#define  WC_ERROR_INVALID_CELL    (-90101)
#define  WC_ERROR_INVALID_OBJID   (-90102)

////////////////////////////////////////////////////////////////////
//
typedef void (*timeoutCallback)(unsigned int, unsigned int);

////////////////////////////////////////////////////////////////////
//
typedef struct stWindowCell
{
    unsigned int    ciSeqNo;
    unsigned int    ciObjectID;
    time_t          ctEnterTime;

    LISTHEAD        coTimeChild;    
    LISTHEAD        coIndexChild;
} WindowCell;

typedef struct stWindowCtl
{
    int             ciCurCount;
    unsigned int    ciObjectID;
    
    LISTHEAD        coWndChild;
    LISTHEAD        coTimeHead;
    LISTHEAD        coIndexHead[WC_MAX_INDEX_SIZE];
} WindowCtl;

////////////////////////////////////////////////////////////////////
//
class clsWindowControl
{
private:
    int             ciTimeout;
    int             ciMaxCount;
    int             ciShutDown;
    pthread_t       ctThreadID;
    AIMutexLock     coMutexLock;
    timeoutCallback cpfTimeoutFunc;

private:
    LISTHEAD        coWndHead[WC_MAX_INDEX_SIZE];

public:
    clsWindowControl(size_t aiMaxWindow, int aiTimeout);
    ~clsWindowControl();
    
private:    
    int _AttachWindow(unsigned int aiObjectID, unsigned int aiSeqNo);
    int _DetachWindow(unsigned int aiObjectID, unsigned int aiSeqNo);
    int _RemoveWindow(unsigned int aiObjectID);
    int RemoveTimeout(void);
    
public:
    int AttachWindow(void const* apNetID, unsigned int aiSeqNo);
    int AttachWindow(int aiSocketID, unsigned int aiSeqNo);
    int DetachWindow(void const* apNetID, unsigned int aiSeqNo);
    int DetachWindow(int aiSocketID, unsigned int aiSeqNo);
    int RemoveWindow(void const* apNetID);
    int RemoveWindow(int aiSocketID);

public:
    void SetCallback(timeoutCallback apfCallback);
    void Startup(timeoutCallback apfCallback);
    void ShutDown(void);
    void Startup(void);
    
public:
    friend void *window_scan_thread(void *avOpt);
};

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AIWNDCONTROL_H__

