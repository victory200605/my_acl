# -*- coding: UTF-8 -*-
import struct


class PacketTooBigError(AssertionError):
    """
    Raised when trying to send a string too long for a length prefixed
    protocol.
    """


class AILibAsyncProtocol(protocol.Protocol, basic._PauseableMixin):
    headerFormat = "!III"
    headerLength = struct.calcsize(headerFormat)
    MIN_LENGTH = 0
    MAX_LENGTH = 2**16
    recvd = ""

    def packetReceived(self, seq, status, data):
        """
        Override this.
        """
        raise NotImplementedError

    def dataReceived(self, recd):
        """
        Convert int prefixed strings into calls to stringReceived.
        """
        self.recvd = self.recvd + recd
        while len(self.recvd) >= self.headerLength and not self.paused:
            seq, status, length = struct.unpack(self.headerFormat, self.recvd[:self.headerLength])
            if length > self.MAX_LENGTH or length < self.MIN_LENGTH:
                self.transport.loseConnection()
                return
            if len(self.recvd) < length:
                break
            data = self.recvd[self.headerLength:self.headerLength + length]
            self.recvd = self.recvd[self.headerLength + length:]
            self.packetReceived(seq, status, data)

    def sendPacket(self, seq, status, data):
        """
        Send an pdu to the other end of the connection.
        """
        if len(data) > self.MAX_LENGTH - self.headerLength:
            raise PacketTooBigError("Try to send %s bytes whereas maximum is %s" % (len(data), self.MAX_LENGTH))
        self.transport.write(struct.pack(self.headerFormat, seq, status, len(data)) + data)


class CPAsyncTcpProtocol(protocol.Protocol, basic._PauseableMixin):
    headerFormat = "!IIIH2x"
    headerLength = struct.calcsize(headerFormat)
    MIN_LENGTH = 0
    MAX_LENGTH = 2**16
    MAGIC_HEAD = 20000721
    recvd = ""
    
    def packetReceived(self, seq, status, data):
        """
        Override this.
        """
        raise NotImplementedError

    def dataReceived(self, recd):
        """
        Convert int prefixed strings into calls to stringReceived.
        """
        self.recvd = self.recvd + recd
        while len(self.recvd) >= self.headerLength and not self.paused:
            magic, length, seq, status = struct.unpack(self.headerFormat, self.recvd[:self.headerLength])
            if magic != self.MAGIC_HEAD:
                self.transport.loseConnection()
                return
            if length > self.MAX_LENGTH or length < self.MIN_LENGTH:
                self.transport.loseConnection()
                return
            if len(self.recvd) < length:
                break
            data = self.recvd[self.headerLength:self.headerLength + length]
            self.recvd = self.recvd[self.headerLength + length:]
            self.commandReceived(seq, status, data)

    def sendPacket(self, seq, status, data):
        """
        Send an pdu to the other end of the connection.
        """
        if len(data) > self.MAX_LENGTH - self.headerLength:
            raise PacketTooBigError("Try to send %s bytes whereas maximum is %s" % (len(data), self.MAX_LENGTH))
        self.transport.write(struct.pack(self.headerFormat, self.MAGIC_HEAD, len(data), seq, status) + data)
