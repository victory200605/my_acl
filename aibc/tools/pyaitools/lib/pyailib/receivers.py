from twisted.internet.protocol import Protocol
import struct

# CMPP/SCP like packet receiver
class PacketReceiver(Protocol):
    HDR_FMT = "!I"
    HDR_LEN = 4
    MAX_LENGTH = 4096

    def dataReceived(self, data):
        self.recvd += data
        while len(self.recvd) >= self.HDR_LEN:
            length, = struct.unpack(self.HDR_FMT, self.recvd[:self.HDR_LEN])
            if length > self.MAX_LENGTH:
                self.onLengthLimitExceeded(length)
                break
            if len(self.recvd) < length:
                break
            packet = self.recvd[self.HDR_LEN:length]
            self.recvd = self.recvd[length:]
            self.onPacketReceived(packet)

    def onLengthLimitExceeded(self, length):
        self.transport.loseConnection()

    def sendPacket(self, pkt):
        self.transport.writeSequence([struct.pack(self.HDR_FMT, len(pkt) + 4) , pkt])     

    def onPacketReceived(self, pkt):
        raise NotImplementedError 



