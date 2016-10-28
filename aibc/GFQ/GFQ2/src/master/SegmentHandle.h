
#ifndef AIBC_GFQ_SEGMENTHANDLE_H
#define AIBC_GFQ_SEGMENTHANDLE_H

#include "gfq2/Utility.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Segment handle for server node segment accessed in manager
 */
class CSegmentHandle
{
public:
    CSegmentHandle(void)
        : muServerID(INVALID_ID)
        , muSegmentID(INVALID_ID)
    {
    }

    CSegmentHandle( apl_size_t auServerID, apl_size_t auSegmentID )
        : muServerID(auServerID)
        , muSegmentID(auSegmentID)
    {
    }

    void Set( apl_size_t auServerID, apl_size_t auSegmentID )
    {
        this->muServerID = auServerID;
        this->muSegmentID = auSegmentID;
    }

    bool operator < ( CSegmentHandle const& aoHandle ) const
    {
        if (this->muServerID < aoHandle.muServerID)
        {
            return true;
        }
        else if (this->muServerID == aoHandle.muServerID
                && this->muSegmentID < aoHandle.muSegmentID )
        {
            return true;
        }

        return false;
    }

    bool operator == ( CSegmentHandle const& aoHandle ) const
    {
        return this->muServerID == aoHandle.muServerID 
            && this->muSegmentID == aoHandle.muSegmentID;
    }

    bool operator != ( CSegmentHandle const& aoHandle ) const
    {
        return !(*this == aoHandle);
    }

    bool IsInvalid(void) const
    {
        return this->muServerID == INVALID_ID || this->muSegmentID == INVALID_ID;
    }

    apl_size_t GetServerNodeID(void) const
    {
        return this->muServerID;
    }

    apl_size_t GetSegmentID(void) const
    {
        return this->muSegmentID;
    }

private:
    apl_size_t muServerID;
    apl_size_t muSegmentID;
};

AIBC_GFQ_NAMESPACE_END

#endif//GFQ_SEGMENTHANDLE_H


