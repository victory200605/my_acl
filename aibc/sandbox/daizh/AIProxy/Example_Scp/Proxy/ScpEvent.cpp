
#include "ScpProtocol.h"
#include "ScpServer.h"
#include "ScpEvent.h"

clsScpActiveEvent::clsScpActiveEvent( int aiTimeInterval ) :
    ciTimeInterval(aiTimeInterval),
    ciListTimestamp(time(NULL))
{
}

bool clsScpActiveEvent::Condition( AIBaseChannel::TState aoState )
{
    return ( time(NULL) - this->ciListTimestamp > this->ciTimeInterval ) ? true : false;
}

void clsScpActiveEvent::Action( AIChannelPtr aoChannelPtr )
{
    clsScpOuterServer* lpoServer = AIProxy::GetOuterServer<clsScpOuterServer*>();
        
    this->ciListTimestamp = time(NULL);
    
    lpoServer->DoHandshake( aoChannelPtr );
}
