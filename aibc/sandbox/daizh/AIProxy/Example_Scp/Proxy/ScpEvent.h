
#ifndef __AIPROXY_SCPEVENT_H__
#define __AIPROXY_SCPEVENT_H__

#include "../../AISyncTcp.h"

///////////////////////////////////////// SCP-ActiveEvent //////////////////////////////////////////
class clsScpActiveEvent : public AIChannelEvent
{
public:
    clsScpActiveEvent( int aiTimeInterval );
    
    virtual bool Condition( AIBaseChannel::TState aoState );
    virtual void Action( AIChannelPtr aoChannelPtr );

protected:
    int ciTimeInterval;
    int ciListTimestamp;
};

#endif //__AIPROXY_SCPEVENT_H__
