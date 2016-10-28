# -*- coding: UTF-8 -*-
from twisted.internet import defer
from pyailib.twisted.smpp import *
#from pyailib.twisted.smpp import pdu
from pyailib.utils.session_manager import SimpleSessionManager


class MessageCenterSession(SMPPServerSession):
    systemID = None
    
    @pdu.unpackPDU(pdu.BIND_TRANSMITTER)
    def handle_BIND_TRANSMITTER(self, statue, seq, msg):
        pass
    
    @pdu.unpackPDU(pdu.BIND_RECEIVER)
    def handle_BIND_RECEIVER(self, statue, seq, msg):
        pass    
        
    @pdu.unpackPDU(pdu.BIND_TRANSCEIVER)
    def handle_BIND_TRANSCEIVER(self, status, seq, msg):
        pass
    
    @pdu.unpackPDU(pdu.SUBMIT_SM)
    def handle_SUBMIT_SM(self, status, seq, msg):
        pass


class MessageCenterSessionFactory(protocol.ServerFactory):
    protocol = MessageCenterSession
    
    def __init__(self):
        self.mcsm = MessageCenterSessionManager()
        

