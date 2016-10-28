

#include "../../AISyncTcp.h"
#include "../Proxy/ScpProtocol.h"

using namespace AIPROXY;

class MyChannel : public AIBaseChannel
{
public:
    virtual int Input( const char* apcData, size_t aiSize )
    {
        if ( false )
        {
            AIChunkEx loTmpData(apcData, aiSize);
            loTmpData.Resize(aiSize+1);
            printf( "[Thread=%d][--Recv request success[size=%d]--]\n%s\n", pthread_self(), aiSize, loTmpData.BasePtr() + 4 );
        }
        
        //response
        AIChunkEx loBuffer(0);
        do
        {
            SCPMsgHeader loRequest;
            AIChunkEx    loData( apcData, aiSize );
            if ( loRequest.Decode(loData) != 0 )
            {
                printf( "ERROR: Decode MsgHeader fail\n" );
                break;
            }

            if ( loRequest.GetType() == MSG_TYPE_LOGIN )
            {
                SCPMsgHeader loResponse(MSG_TYPE_LOGIN_RSP);
                loResponse.GeneralHeader()->SetCSeq( loRequest.GeneralHeader()->GetCSeq() );
                loResponse.GeneralHeader()->SetFrom( loRequest.GeneralHeader()->GetTo() );
                loResponse.GeneralHeader()->SetTo( loRequest.GeneralHeader()->GetFrom() );
                loResponse.GeneralHeader()->SetServiceKey( loRequest.GeneralHeader()->GetServiceKey() );
                loResponse.SetStatus( 401 );
                loResponse.Encode( loBuffer );
                break;
            }
            
            if ( loRequest.GetType() == MSG_TYPE_AUTHENTICATION )
            {
                SCPMsgHeader loResponse(MSG_TYPE_AUTHENTICATION_RSP);
                loResponse.GeneralHeader()->SetCSeq( loRequest.GeneralHeader()->GetCSeq() );
                loResponse.GeneralHeader()->SetFrom( loRequest.GeneralHeader()->GetTo() );
                loResponse.GeneralHeader()->SetTo( loRequest.GeneralHeader()->GetFrom() );
                loResponse.GeneralHeader()->SetServiceKey( loRequest.GeneralHeader()->GetServiceKey() );
                loResponse.SetStatus( 205 );
                loResponse.Encode( loBuffer );
                break;
            }
            
            if ( loRequest.GetType() == MSG_TYPE_REQUEST )
            {
                SCPMsgHeader loResponse(MSG_TYPE_RESPOND);
                loResponse.GeneralHeader()->SetCSeq( loRequest.GeneralHeader()->GetCSeq() );
                loResponse.GeneralHeader()->SetFrom( loRequest.GeneralHeader()->GetTo() );
                loResponse.GeneralHeader()->SetTo( loRequest.GeneralHeader()->GetFrom() );
                loResponse.GeneralHeader()->SetServiceKey( loRequest.GeneralHeader()->GetServiceKey() );
                loResponse.SetXmlMsg( loRequest.GetXmlMsg() );
                loResponse.SetStatus( 200 );
                loResponse.Encode( loBuffer );
                break;
            }
            
            if ( loRequest.GetType() == MSG_TYPE_SHAKEHAND )
            {
                SCPMsgHeader loResponse(MSG_TYPE_SHAKEHAND_RSP);
                loResponse.GeneralHeader()->SetCSeq( loRequest.GeneralHeader()->GetCSeq() );
                loResponse.GeneralHeader()->SetFrom( loRequest.GeneralHeader()->GetTo() );
                loResponse.GeneralHeader()->SetTo( loRequest.GeneralHeader()->GetFrom() );
                loResponse.GeneralHeader()->SetServiceKey( loRequest.GeneralHeader()->GetServiceKey() );
                loResponse.Encode( loBuffer );
                break;
            }
            
            printf( "ERROR: invalid request\n" );
            return 0;
        }
        while(false);
        
        this->Send( loBuffer.BasePtr(), loBuffer.GetSize() );
        
        return 0;
    }
    virtual int Exit()
    {
        printf( "INFO : Connection %d close\n", this->GetChannelID() );
        MyChannel* lpoChannel = this;
        delete lpoChannel;
        return 0;
    }
    
    virtual int SendTo( AISocketHandle atHandle, const char* apsData, size_t aiSize )
    {
        return coProtocol.Send( atHandle, apsData, aiSize );
    }
    
    virtual int RecvFrom( AISocketHandle atHandle, AIChunkEx& aoBuffer, int aiTimeout )
    {
        return coProtocol.Recv( atHandle, aoBuffer, aiTimeout );
    }
    
protected:
    clsScpOuterProtocol coProtocol;
};

class MyServer : public AIBaseAcceptor
{
public:
    virtual AIBaseChannel* CreateChannel()
    {
        cpoChannel = new MyChannel;
        printf( "INFO : Accept a new connection %d\n", cpoChannel->GetChannelID() );
        return cpoChannel;
    }
    
    virtual int Accept( AIBaseChannel* apoChannel )
    {
        apoChannel->Ready();
        return 0;
    }
    
    AIBaseChannel* cpoChannel;
};

int main( int argc, char* argv[] )
{
    MyServer  loServer;
    int     liOptChar;
    char    lsIpAddr[20] = {"127.0.0.1"};
    int     liPort = 6501;
    while( (liOptChar = getopt(argc, argv, "s:p:" )) != EOF )
	{
		switch (liOptChar)
		{
			case 's':
			    strcpy( lsIpAddr, optarg );
			    break;
			case 'p':
			    liPort = atoi(optarg);
			    break;
		}
	}
	
    loServer.Startup( lsIpAddr, liPort );
    
    getchar();
    
    return 0;
}
