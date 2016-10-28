#ifndef __AILIBEX__AILINKRELATION_H__
#define __AILIBEX__AILINKRELATION_H__

#include <time.h>

#include "AIDualLink.h"
#include "AISynch.h"

///start namespace
AIBC_NAMESPACE_START

//////////////////////////////////////////////////////////////////////////
//
typedef struct tagSEQCONNOBJ
{
    void*           cpoConnObj;
    time_t          ctEnterTime;
    unsigned int    ciSequenceNo;
    unsigned int    ciOldSequence;
    
    LISTHEAD        coSeqConnChild;
    LISTHEAD        coListChild;
} SEQCONNOBJ;

//////////////////////////////////////////////////////////////////////////
//
class clsLinkRelation
{
private:
    size_t      ciHashSize;
    size_t      ciCurSize;
    LISTHEAD*   cpoConnHead;
    LISTHEAD    coListHead;
    AIMutexLock coMutexLock;
    
public:
    clsLinkRelation(size_t aiHashSize = 512);
    ~clsLinkRelation();
    
public:
    size_t Remove(void const* apoConnObj);
    size_t RemoveAll(time_t aiTimeout=0);

public:
    int Get(unsigned int aiSequenceNo, bool abToRemove);
    int Put(unsigned int aiSequenceNo, void *apoConnObj, unsigned int aiOld);
    int Get(unsigned int aiSequenceNo, void *&apoConnObj, unsigned int &aiOld, bool abToRemove=true);
    int GetTimeout(unsigned int& aiSequenceNo, void *&apoConnObj, unsigned int &aiOld, time_t aiTimeout=0);

    bool IsEmpty();

    size_t GetSize();
};

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AILINKRELATION_H__

