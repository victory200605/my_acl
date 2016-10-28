# -*- coding: UTF-8 -*-
from pyailib.twisted import PDU32Receiver
from pyailib.utils.sequence import createTimedResetSequenceGenerator
import struct
import time
try:
    from hashlib import md5
except ImportError:
    from md5 import md5


PDU_CONNECT                     = 0x00000001
PDU_CONNECT_RESP                = 0x80000001
PDU_TERMINATE                   = 0x00000002
PDU_TERMINATE_RESP              = 0x80000002
PDU_SUBMIT                      = 0x00000004
PDU_SUBMIT_RESP                 = 0x80000004
PDU_DELIVER                     = 0x00000005
PDU_DELIVER_RESP                = 0x80000005
PDU_QUERY                       = 0x00000006
PDU_QUERY_RESP                  = 0x80000006
PDU_CANCEL                      = 0x00000007
PDU_CANCEL_RESP                 = 0x80000007
PDU_ACTIVE_TEST                 = 0x00000008
PDU_ACTIVE_TEST_RESP            = 0x80000008
PDU_FWD                         = 0x00000009
PDU_FWD_RESP                    = 0x80000009
PDU_MT_ROUTE                    = 0x00000010
PDU_MT_ROUTE_RESP               = 0x80000010
PDU_MO_ROUTE                    = 0x00000011
PDU_MO_ROUTE_RESP               = 0x80000011
PDU_GET_MT_ROUTE                = 0x00000012
PDU_GET_MT_ROUTE_RESP           = 0x80000012
PDU_MT_ROUTE_UPDATE             = 0x00000013
PDU_MT_ROUTE_UPDATE_RESP        = 0x80000013
PDU_MO_ROUTE_UPDATE             = 0x00000014
PDU_MO_ROUTE_UPDATE_RESP        = 0x80000014
PDU_PUSH_MT_ROUTE_UPDATE        = 0x00000015
PDU_PUSH_MT_ROUTE_UPDATE_RESP   = 0x80000015
PDU_PUSH_MO_ROUTE_UPDATE        = 0x00000016
PDU_PUSH_MO_ROUTE_UPDATE_RESP   = 0x80000016
PDU_GET_MO_ROUTE                = 0x00000017
PDU_GET_MO_ROUTE_RESP           = 0x80000017


PDU_NAMES = {
    PDU_CONNECT                     : "CONNECT",
    PDU_CONNECT_RESP                : "CONNECT_RESP",
    PDU_TERMINATE                   : "TERMINATE",
    PDU_TERMINATE_RESP              : "TERMINATE_RESP",
    PDU_SUBMIT                      : "SUBMIT",
    PDU_SUBMIT_RESP                 : "SUBMIT_RESP",
    PDU_DELIVER                     : "DELIVER",
    PDU_DELIVER_RESP                : "DELIVER_RESP",
    PDU_QUERY                       : "QUERY",
    PDU_QUERY_RESP                  : "QUERY_RESP",
    PDU_CANCEL                      : "CANCEL",
    PDU_CANCEL_RESP                 : "CANCEL_RESP",
    PDU_ACTIVE_TEST                 : "ACTIVE_TEST",
    PDU_ACTIVE_TEST_RESP            : "ACTIVE_TEST_RESP",
    PDU_FWD                         : "FWD",
    PDU_FWD_RESP                    : "FWD_RESP",
    PDU_MT_ROUTE                    : "MT_ROUTE",
    PDU_MT_ROUTE_RESP               : "MT_ROUTE_RESP",
    PDU_MO_ROUTE                    : "MO_ROUTE",
    PDU_MO_ROUTE_RESP               : "MO_ROUTE_RESP",
    PDU_GET_MT_ROUTE                : "GET_MT_ROUTE",
    PDU_GET_MT_ROUTE_RESP           : "GET_MT_ROUTE_RESP",
    PDU_MT_ROUTE_UPDATE             : "MT_ROUTE_UPDATE",
    PDU_MT_ROUTE_UPDATE_RESP        : "MT_ROUTE_UPDATE_RESP",
    PDU_MO_ROUTE_UPDATE             : "MO_ROUTE_UPDATE",
    PDU_MO_ROUTE_UPDATE_RESP        : "MO_ROUTE_UPDATE_RESP",
    PDU_PUSH_MT_ROUTE_UPDATE        : "PUSH_MT_ROUTE_UPDATE",
    PDU_PUSH_MT_ROUTE_UPDATE_RESP   : "PUSH_MT_ROUTE_UPDATE_RESP",
    PDU_PUSH_MO_ROUTE_UPDATE        : "PUSH_MO_ROUTE_UPDATE",
    PDU_PUSH_MO_ROUTE_UPDATE_RESP   : "PUSH_MO_ROUTE_UPDATE_RESP",
    PDU_GET_MO_ROUTE                : "GET_MO_ROUTE",
    PDU_GET_MO_ROUTE_RESP           : "GET_MO_ROUTE_RESP",
}


def calcAuthenticatorSource(srcAddr, sharedSecret, timeStamp):
    m = md5()
    m.update(srcAddr)
    m.update('\0' * 9)
    m.update(sharedSecret)
    m.update("%010u" % (timeStamp % 10000000000))
    return m.digest()


def calcAuthenticatorISMG(connStatus, authenticatorSource, sharedSecret):
    m = md5()
    m.update("%u" % connStatus)
    m.update(authenticatorSource)
    m.update(sharedSecret)
    return m.digest()


def buildMessageID(timeTuple, ismgNum, seqNum):
    msgID = 0
    msgID |= (timeTuple[0] & 0xF) << 60 
    msgID |= (timeTuple[1] & 0x1F) << 55 
    msgID |= (timeTuple[2] & 0x1F) << 50
    msgID |= (timeTuple[3] & 0x3F) << 44
    msgID |= (timeTuple[4] & 0x3F) << 38
    msgID |= (ismgNum & 0x1FFFFF) << 16 
    msgID |= seqNum & 0xFFFF
    return msgID


def parseMessageID(msgID):
    timeTuple = (
        (msgID >> 60) & 0xF,
        (msgID >> 55) & 0x1F,
        (msgID >> 50) & 0x1F,
        (msgID >> 44) & 0x3F,
        (msgID >> 38) & 0x3F,
    )
    ismgNum = (msgID >> 16) & 0x1FFFFF
    seqNum = msgID & 0xFFFF
    
    return timeTuple, ismgNum, seqNum


def createMessageIDGenerator(ismgNum):
    ismgNum = int(ismgNum)
    seqGen = createTimedResetSequenceGenerator(0, 0xFFFF)
    while True:
        timeStamp, seqNum = seqGen.next()
        timeTuple = time.localtime(timeStamp)[1:6]
        yield buildMessageID(timeTuple, ismgNum, seqNum)

        
class CMPPProtocol(PDU32Receiver):
    headerFormat = "!II"
    headerLength = struct.calcsize(headerFormat)
    MIN_LENGTH = PDU32Receiver.prefixLength + headerLength
    
    def commandReceived(self, cmd, seq, data):
        """
        Override this.
        """
        raise NotImplementedError       
    
    def pduReceived(self, pdu):
        cmd, seq = struct.unpack(self.headerFormat, pdu[:self.headerLength])
        data = pdu[self.headerLength:]
        self.commandReceived(cmd, seq, data)
        
    def sendCommand(self, cmd, seq, data=""):
        self.sendPDU(struct.pack(self.headerFormat, cmd, seq) + data)
        
