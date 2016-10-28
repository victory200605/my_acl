# -*- coding: UTF-8 -*-
from twisted.internet import protocol
from pyailib.twisted import smpp
from pyailib.utils.session_manager import SimpleSessionManager

class SmscFactory(protocol.ServerFactory):
    protocol = smpp.SMPPServerSession
    
    def __init__(self):
        self.sessionMgr = SimpleSessionManager(smpp.STATE_TX | smpp.STATE_RX)
        self.forward = None
        
    def setForward(self, factory):
        self.forward = factory
        
