# -*- coding: UTF-8 -*-
from twisted.internet import reactor
from pyailib.twisted.protocol import cmpp
import time
    

class RPCInterface(interface):
    def invokeCommand(self, cmd, seq=None, body=None):
        if not seq is None and seq in self.pendingCommands:
            raise XXXError
        self.sendCommand(cmd, seq, body)
        d = defer.Deferred()
        callID = reactor.callLater(xxxDelay, d.errback, XXXTimeourError)
        def _cancelCall(resultOrFailure):
            callID.cancel()
            return resultOrFailure
        d.setBoth(_cancelCall)
        self.pendingCommands[seq] = d
        return d


class CMPPSession(cmpp.CMPPProtocol):
    def commandReceived(self, cmd, seq, body):
        try:
            pduName = cmpp.PDU_NAMES[cmd]
            handleFunc = getattr(self, "handle_CMPP_%s" % pduName)
            return handlerFunc(seq, body)
        except:
            pass

    def handle_CMPP_CONNECT(self, seq, body):
        self.resetTimer(SESSION_INIT_TIMER)
        self.pendingRequests = {}
        
    def handle_CMPP_ACTIVE_TEST(self, seq, body):
        self.sendCommand(cmpp.PDU_ACTIVE_TEST_RESP, seq)
        
    def handle_CMPP_TERMINATE(self, seq, body):
        self.sendCommand(cmpp.PDU_TERMINATE_RESP, seq)
        self.trasnport.loseConnection()
            
