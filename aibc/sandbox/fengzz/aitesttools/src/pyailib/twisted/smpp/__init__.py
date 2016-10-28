# -*- coding: UTF-8 -*-
import struct
from pyailib.twisted import PDU32Receiver
from pyailib.twisted.utils import Timer
from pyailib.utils.sequence import createCycledSequenceGenerator

STATE_CLOSED    = 0x00
STATE_OPEN      = 0x01
STATE_UNBOUND   = 0x02
STATE_OUTBOUND  = 0x03
STATE_BOUND_TX  = 0x04
STATE_BOUND_RX  = 0x08
STATE_BOUND_TRX = STATE_BOUND_TX | STATE_BOUND_RX

# 4.7.5 command_id
PDU_GENERIC_NACK            = 0x80000000
PDU_BIND_RECEIVER           = 0x00000001
PDU_BIND_RECEIVER_RESP      = 0x80000001
PDU_BIND_TRANSMITTER        = 0x00000002
PDU_BIND_TRANSMITTER_RESP   = 0x80000002
PDU_QUERY_SM                = 0x00000003
PDU_QUERY_SM_RESP           = 0x80000003
PDU_SUBMIT_SM               = 0x00000004
PDU_SUBMIT_SM_RESP          = 0x80000004
PDU_DELIVER_SM              = 0x00000005
PDU_DELIVER_SM_RESP         = 0x80000005
PDU_UNBIND                  = 0x00000006
PDU_UNBIND_RESP             = 0x80000006
PDU_REPLACE_SM              = 0x00000007
PDU_REPLACE_SM_RESP         = 0x80000007
PDU_CANCEL_SM               = 0x00000008
PDU_CANCEL_SM_RESP          = 0x80000008
PDU_BIND_TRANSCEIVER        = 0x00000009
PDU_BIND_TRANSCEIVER_RESP   = 0x80000009
PDU_OUTBIND                 = 0x0000000B
PDU_ENQUIRE_LINK            = 0x00000015
PDU_ENQUIRE_LINK_RESP       = 0x80000015
PDU_SUBMIT_MULTI            = 0x00000021
PDU_SUBMIT_MULTI_RESP       = 0x80000021
PDU_ALERT_NOTIFICATION      = 0x00000102
PDU_DATA_SM                 = 0x00000103
PDU_DATA_SM_RESP            = 0x80000103


# 4.7.6 command_status, error_status_code    
STATUS_ESME_RSYSERR             = 0x00000008
STATUS_ESME_RINVSRCADR          = 0x0000000A
STATUS_ESME_RINVDSTADR          = 0x0000000B
STATUS_ESME_RINVMSGID           = 0x0000000C
STATUS_ESME_RBINDFAIL           = 0x0000000D
STATUS_ESME_RINVPASWD           = 0x0000000E
STATUS_ESME_RINVSYSID           = 0x0000000F
STATUS_ESME_RCANCELFAIL         = 0x00000011
STATUS_ESME_RREPLACEFAIL        = 0x00000013
STATUS_ESME_RMSGQFUL            = 0x00000014
STATUS_ESME_RINVSERTYP          = 0x00000015
STATUS_ESME_RINVNUMDESTS        = 0x00000033
STATUS_ESME_RINVDLNAME          = 0x00000034
STATUS_ESME_RINVDESTFLAG        = 0x00000040
STATUS_ESME_RINVSUBREP          = 0x00000042
STATUS_ESME_RINVESMCLASS        = 0x00000043
STATUS_ESME_RCNTSUBDL           = 0x00000044
STATUS_ESME_RSUBMITFAIL         = 0x00000045
STATUS_ESME_RINVSRCTON          = 0x00000048
STATUS_ESME_RINVSRCNPI          = 0x00000049
STATUS_ESME_RINVDSTTON          = 0x00000050
STATUS_ESME_RINVDSTNPI          = 0x00000051
STATUS_ESME_RINVSYSTYP          = 0x00000053
STATUS_ESME_RINVREPFLAG         = 0x00000054
STATUS_ESME_RINVNUMMSGS         = 0x00000055
STATUS_ESME_RTHROTTLED          = 0x00000058
STATUS_ESME_RINVSCHED           = 0x00000061
STATUS_ESME_RINVEXPIRY          = 0x00000062
STATUS_ESME_RINVDFTMSGID        = 0x00000063
STATUS_ESME_RX_T_APPN           = 0x00000064
STATUS_ESME_RX_P_APPN           = 0x00000065
STATUS_ESME_RX_R_APPN           = 0x00000066
STATUS_ESME_RQUERYFAIL          = 0x00000067
STATUS_ESME_RINVTLVSTREAM       = 0x000000C0
STATUS_ESME_RTLVNOTALLWD        = 0x000000C1
STATUS_ESME_RINVTLVLEN          = 0x000000C2
STATUS_ESME_RMISSINGTLV         = 0x000000C3
STATUS_ESME_RINVTLVVAL          = 0x000000C4
STATUS_ESME_RDELIVERYFAILURE    = 0x000000FE
STATUS_ESME_RUNKNOWNERR         = 0x000000FF
STATUS_ESME_RSERTYPUNAUTH       = 0x00000100
STATUS_ESME_RPROHIBITED         = 0x00000101
STATUS_ESME_RSERTYPUNAVAIL      = 0x00000102
STATUS_ESME_RSERTYPDENIED       = 0x00000103
STATUS_ESME_RINVDCS             = 0x00000104
STATUS_ESME_RINVSRCADDRSUBUNIT  = 0x00000105
STATUS_ESME_RINVDSTADDRSUBUNIT  = 0x00000106
STATUS_ESME_RINVBCASTFREQINT    = 0x00000107
STATUS_ESME_RINVBCASTALIAS_NAME = 0x00000108
STATUS_ESME_RINVBCASTAREAFMT    = 0x00000109
STATUS_ESME_RINVNUMBCAST_AREAS  = 0x0000010A
STATUS_ESME_RINVBCASTCNTTYPE    = 0x0000010B
STATUS_ESME_RINVBCASTMSGCLASS   = 0x0000010C
STATUS_ESME_RBCASTFAIL          = 0x0000010D
STATUS_ESME_RBCASTQUERYFAIL     = 0x0000010E
STATUS_ESME_RBCASTCANCELFAIL    = 0x0000010F
STATUS_ESME_RINVBCAST_REP       = 0x00000110
STATUS_ESME_RINVBCASTSRVGRP     = 0x00000111
STATUS_ESME_RINVBCASTCHANIND    = 0x00000112

PDU_NAMES = {
    PDU_GENERIC_NACK            : "GENERIC_NACK",
    PDU_BIND_RECEIVER           : "BIND_RECEIVER",
    PDU_BIND_RECEIVER_RESP      : "BIND_RECEIVER_RESP",
    PDU_BIND_TRANSMITTER        : "BIND_TRANSMITTER",
    PDU_BIND_TRANSMITTER_RESP   : "BIND_TRANSMITTER_RESP",
    PDU_QUERY_SM                : "QUERY_SM",
    PDU_QUERY_SM_RESP           : "QUERY_SM_RESP",
    PDU_SUBMIT_SM               : "SUBMIT_SM",
    PDU_SUBMIT_SM_RESP          : "SUBMIT_SM_RESP",
    PDU_DELIVER_SM              : "DELIVER_SM",
    PDU_DELIVER_SM_RESP         : "DELIVER_SM_RESP",
    PDU_UNBIND                  : "UNBIND",
    PDU_UNBIND_RESP             : "UNBIND_RESP",
    PDU_REPLACE_SM              : "REPLACE_SM",
    PDU_REPLACE_SM_RESP         : "REPLACE_SM_RESP",
    PDU_CANCEL_SM               : "CANCEL_SM",
    PDU_CANCEL_SM_RESP          : "CANCEL_SM_RESP",
    PDU_BIND_TRANSCEIVER        : "BIND_TRANSCEIVER",
    PDU_BIND_TRANSCEIVER_RESP   : "BIND_TRANSCEIVER_RESP",
    PDU_OUTBIND                 : "OUTBIND",
    PDU_ENQUIRE_LINK            : "ENQUIRE_LINK",
    PDU_ENQUIRE_LINK_RESP       : "ENQUIRE_LINK_RESP",
    PDU_SUBMIT_MULTI            : "SUBMIT_MULTI",
    PDU_SUBMIT_MULTI_RESP       : "SUBMIT_MULTI_RESP",
    PDU_ALERT_NOTIFICATION      : "ALERT_NOTIFICATION",
    PDU_DATA_SM                 : "DATA_SM",
    PDU_DATA_SM_RESP            : "DATA_SM_RESP",    
}


class SMPPError(Exception):
    def __init__(self, status):
        self.status
        
    def __str__(self):
        return "SMPP error, status = %d" % self.status


class SMPPProtocol(PDU32Receiver):
    headerFormat = "!III"
    headerLength = struct.calcsize(headerFormat)
    MIN_LENGTH = PDU32Receiver.prefixLength + headerLength
    
    def commandReceived(self, cmd, status, seq, data):
        """
        Override this.
        """
        
    def pduReceived(self, pdu):
        cmd, status, seq = struct.unpack(self.headerFormat, pdu[:self.headerLength])
        body = pdu[self.headerLength:]
        self.commandReceived(cmd, status, seq, body)
        
    def sendCommand(self, cmd, status, seq, data):
        self.sendPDU(struct.pack(self.headerFormat, cmd, status, seq) + data)


def _wrapCallback(func, *argv, **kwargs):
    def _wrapper(_):
        func(*argv, **kwargs)
        return _    
    return _wrapper


def allowStates(*states):
    def _wrapping(func):
        def _wrapper(self, *argv, **kwargs):
            assert self.state in states
            func(self, *argv, **kwargs)
        return _wrapper
    return _wrapping


class SMPPSession(SMPPProtocol):
    state = STATE_CLOSED

    def unhandleRequestReceived(self, cmd, status, seq, data):
        """
        Override this
        """
        
    def unhandledResponseReceived(self, cmd, status, seq, data):
        """
        Override this.
        """
    
    def connectionMade(self):
        self.pendingRequests = {}
        self.seq = createCycledSequenceGenerator(1, 0x7FFFFFFF)
        self.sessionInitTimer = Timer(self.factory.sessionInitTimeout, self.transport.loseConnection)
        self.enquireLinkTimer = Timer(self.factory.enquireLinkTimeout, self.enquireLink)
        self.inactivityTimer = Timer(self.factory.inactivityTimeouut, self.transport.loseConnection)
        self.switchState(STATE_OPEN)
        
    def connectionLost(self, reason):
        for seq, d in self.pendingRequests.items():
            defer.timeout(d) 
        self.switchState(STATE_CLOSED)
            
    def commandReceived(self, cmd, status, seq, data):
        if cmd & 0x80000000:
            self.handleResponse(cmd, status, seq, data)
        else:
            self.handleRequest(cmd, status, seq, data)
        self.switchState(self.state) # reset timers
                
    def handleRequest(self, cmd, status, seq, data):
        try:
            name = PDU_NAMES[cmd]
            func = getattr(self, "handle_%s" % name)
        except:
            self.unhandledRequestReceived(cmd, status, seq, data)
        else:
            func(status, seq, data)   
    
    def handleResponse(self, cmd, status, seq, data):
        reqDeferred = pendingRequest.get(seq, None)
        if reqDeferred:
            reqDeferred.callback((cmd, status, seq, data))
        else:
            self.unhandledResponseReceived(cmd, status, seq, data)
            
    def sendCommand(self, cmd, status=0, seq=None, data=""):
        if seq is None:
            seq = self.seq.next()
        return SMPPProtocol.sendCommand(self, cmd, status, seq, data)
    
    def invokeCommand(self, cmd, status=0, seq=None, data=""):
        assert((cmd & 0x80000000) == 0)
        self.sendCommand(cmd, status, seq, body)
        d = defer.Deferred()
        callID = reactor.callLater(self.responseTime, d.errback, SMPPError(-1))
        def _cancelTimeout(result):
            if not callID.canceled:
                callID.cancel()
            return result
        d.addBoth(_cancelTimeout)
        return d

    def cancelCommand(self, seq):
        d = self.pendingRequests.pop(seq)
        d.errback(SMPPError(-2))
    
    def switchState(self, newState):
        if newState in (STATE_OPEN, STATE_OUTBOUND):
            self.sessionInitTimer.reset()
        else:
            self.sessionInitTimer.cancel()
        
        if newState != STATE_CLOSED:
            self.enquireLinkTimer.reset()
        else:
            self.enquireLinkTimer.cancel()
        
        if newState in (STATE_BOUND_TX, STATE_BOUND_RX, STATE_BOUND_TRX):
            self.inactivityTimer.reset()
        else:
            self.inactivityTimer.cancel()
        self.state = newState
    
    def enquireLink(self, data="", status=0, seq=None):
        return self.invokeCommand(PDU_ENQUIRE_LINK, 0)
    
    @allowStates(STATE_BOUND_TX, STATE_BOUND_RX, STATE_BOUND_TRX)    
    def unbind(self, data="", status=0, seq=None):
        return self.invokeCommand(PDU_UNBIND, status, seq, data).addCallback(
                _wrapCallback(self.switchState, STATE_UNBOUND)
        )
        
    def handle_ENQUIRE_LINK(self, status, seq, data):
        self.sendCommand(PDU_ENQUIRE_LINK, 0, seq)
    
    @allowStates(STATE_BOUND_TX, STATE_BOUND_RX, STATE_BOUND_TRX)
    def handle_UNBIND(self, status, seq, data):
        self.sencCommand(PDU_UNBIND_RESP, 0, seq)
        self.transport.loseConnection()


class SMPPClientSession(SMPPSession):
    @allowStates(STATE_BOUND_OPEN, STATE_BOUND_OUTBIND)
    def bindTransmitter(self, data, status=0, seq=None):
        return self.invokeCommand(PDU_BIND_TRANSMITTER, status, seq, data).addCallbacks(
                _wrapCallback(self.switchState, STATE_BOUND_TX)
        )

    @allowStates(STATE_BOUND_OPEN, STATE_BOUND_OUTBIND)
    def bindReceiver(self, data, status=0, seq=None):
        return self.invokeCommand(PDU_BIND_RECEIVER, status, seq, data).addCallbacks(
                _wrapCallback(self.switchState, STATE_BOUND_RX)
        )

    @allowStates(STATE_BOUND_OPEN, STATE_BOUND_OUTBIND)
    def bindTransceiver(self, data, status=0, seq=None):
        return self.invokeCommand(PDU_BIND_TRANSCEIVER, status, seq, data).addCallbacks(
                _wrapCallback(self.switchState, STATE_BOUND_TRX)
        )
    
    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def submitSM(self, data, status=0, seq=None):
        return self.invokeCommand(PDU_SUBMIT_SM, status, seq, data)

    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def submitMulti(self, data, status=0, seq=None):
        return self.invokeCommand(PDU_SUBMIT_MULTI, status, seq, data)
        
    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def querySM(self, data, status=0, seq=None):
        return self.invokeCommand(PDU_QUREY_SM, status, seq, data)
    
    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def replaceSM(self, data, status=0, seq=None):
        return self.invokeCommand(PDU_REPLACE_SM, status, seq, data)
    
    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def cancelSM(self, data, status=0, seq=None):
        return self.invokeCommand(PDU_CANCEL_SM, status, seq, data)
    
    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def broadcastSM(self, data, status=0, seq=None):
        return self.invokeCommand(PDU_BROADCAST_SM, status, seq, data)
    
    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def qureyBroadcastSM(self, data, status=0, seq=None):
        return self.invokeCommand(PDU_QUERY_BROADCAST_SM, status, seq, data)
        
    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def cancelBroadcastSM(self, data, status=0, seq=None):
        return self.invokeCommand(PDU_CANCEL_BROADCAST_SM, status, seq, data)

    @allowStates(STATE_BOUND_RX, STATE_BOUND_TRX)
    def handle_DELIVER_SM(self, status, seq, data):
        raise NotImplementError
        
    @allowStates(STATE_OPEN)
    def handle_OUTBIND(self, status, seq, data):
        raise NotImplementError
    

class SMPPServerSession(SMPPSession):
    @allowStates(STATE_OPEN)
    def outbind(self, data, status=0, seq=None):
        self.sendCommand(PDU_OUTBIND)
        self.switchState(STATE_OUTBOUND)

    @allowStates(STATE_BOUND_RX, STATE_BOUND_TRX)
    def deilverSM(self, data, status=0, seq=None):
        return self.invokeCommand(PDU_DELIVER_SM, data=data)
    
    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def handle_SUMBIT_SM(self, statue, seq, data):
        raise NotImplementError
    
    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def handle_QUERY_SM(self, statue, seq, data):
        raise NotImplementError

    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def handle_REPLACE_SM(self, statue, seq, data):
        raise NotImplementError

    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def handle_CANCEL_SM(self, statue, seq, data):
        raise NotImplementError
    
    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def handle_BROADCAST_SM(self, statue, seq, data):
        raise NotImplementError
    
    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def handle_QUERY_BROADCAST_SM(self, statue, seq, data):
        raise NotImplementError
    
    @allowStates(STATE_BOUND_TX, STATE_BOUND_TRX)
    def handle_CANCEL_BROADCAST_SM(self, statue, seq, data):
        raise NotImplementError
    