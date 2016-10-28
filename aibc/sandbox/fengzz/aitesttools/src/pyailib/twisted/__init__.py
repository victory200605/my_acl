# -*- coding: UTF-8 -*-
from twisted.internet import protocol
from twisted.protocols import basic
import struct


FOR_TWISTED_VERSION = (8,1,0)


class PDUTooBigError(AssertionError):
    """
    Raised when trying to send a string too long for a length prefixed
    protocol.
    """


class PDUReceiver(protocol.Protocol, basic._PauseableMixin):
    """
    handle CMPP/SMPP like PDU receiving
    """
    MIN_LENGTH = 0
    MAX_LENGTH = 80 * 1024
    recvd = ""

    def pduReceived(self, msg):
        """
        Override this.
        """
        raise NotImplementedError

    def dataReceived(self, recd):
        """
        Convert int prefixed strings into calls to stringReceived.
        """
        self.recvd = self.recvd + recd
        while len(self.recvd) >= self.prefixLength and not self.paused:
            length, = struct.unpack(self.structFormat, self.recvd[:self.prefixLength])
            if length > self.MAX_LENGTH or length < self.MIN_LENGTH:
                # TODO: add log
                self.transport.loseConnection()
                return
            if len(self.recvd) < length:
                break
            pdu = self.recvd[self.prefixLength:length]
            self.recvd = self.recvd[length:]
            self.pduReceived(pdu)

    def sendPDU(self, data):
        """
        Send an pdu to the other end of the connection.
        """
        if len(data) > self.MAX_LENGTH - self.prefixLength:
            raise PDUTooBigError("Try to send %s bytes whereas maximum is %s" % (len(data), self.MAX_LENGTH))
        self.transport.write(struct.pack(self.structFormat, len(data) + self.prefixLength) + data)


class PDU8Receiver(PDUReceiver):
    structFormat = "!B"
    prefixLength = struct.calcsize(structFormat)
    MIN_LENGTH = prefixLength


class PDU16Receiver(PDUReceiver):
    structFormat = "!H"
    prefixLength = struct.calcsize(structFormat)
    MIN_LENGTH = prefixLength


class PDU32Receiver(PDUReceiver):
    structFormat = "!I"
    prefixLength = struct.calcsize(structFormat)
    MIN_LENGTH = prefixLength


