#ifndef __AILIBEX__AIPACKETFILTER_H__
#define __AILIBEX__AIPACKETFILTER_H__

#include "AIChunkEx.h"

///start namespace
AIBC_NAMESPACE_START

struct _FilterData;
class AISmartDataPtr
{
public:
    AISmartDataPtr( _FilterData* apoData );
    AISmartDataPtr( AISmartDataPtr& aoRhs );
    ~AISmartDataPtr();
    
    bool operator == ( void* apvRhs );
    
    const char* GetData();
    const char* GetType();
    size_t      GetSize();
    
    operator _FilterData* ();
    
protected:
    _FilterData* cpoData;
};

void    AIPacketFilterUpdate(char const* apcExpr);
int     AIPacketFilterFlushPending(int aiSock);
AISmartDataPtr AIPacketFilterGetData(void);

void _AIPacketFilterAcquireRLock();
void _AIPacketFilterReleaseRLock();

bool _AIPacketFilterTestFieldS(char const* apcField, char const* apcValue);
bool _AIPacketFilterTestFieldI(char const* apcField, intmax_t aiValue);

void _AIPacketFilterAddPacket(char const* apcType, void const* apData, size_t aiLen);



#define AI_PT_BEGIN()  \
    { \
        _AIPacketFilterAcquireRLock(); \
        if (


#define AI_PT_FIELD_STR(field, value)  \
            (_AIPacketFilterTestFieldS(field, value)) &&


#define AI_PT_FIELD_INT(field, value)  \
            (_AIPacketFilterTestFieldI(field, value)) &&


#define AI_PT_END(type, data, len) \
            true) \
        { \
            _AIPacketFilterAddPacket(type, data, len); \
        } \
        _AIPacketFilterReleaseRLock(); \
    }        

#define AI_PT_END2(type, data1, len1, data2, len2) \
            true) \
        { \
            AIChunkEx   loChunk(len1 + len2); \
            memcpy(loChunk.GetPointer(), data1, len1); \
            memcpy(loChunk.GetPointer() + len1, data2, len2); \
            _AIPacketFilterAddPacket(type, loChunk.GetPointer(), loChunk.GetSize()); \
        } \
        _AIPacketFilterReleaseRLock(); \
    }        

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AIPACKETFILTER_H__

