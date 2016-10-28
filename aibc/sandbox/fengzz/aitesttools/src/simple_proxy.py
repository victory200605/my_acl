# -*- coding: UTF-8 -*-
from twisted.internet import protocol
from pyailib.twisted.protocol import smpp, cmpp
from pyailib.utils.main import easyMain
from pyailib.utils import formatHexText
from optparse import OptionParser
import logging


optParser = OptionParser(usage="%prog [options]")
optParser.add_option("-p", "--bind-port", dest="bind_port", type="int", metavar="BIND_PORT")
optParser.add_option("-a", "--forward-addr", dest="forward_addr", metavar="FORWARD_ADDR")
optParser.add_option("-r", "--forward-port", dest="forward_port", type="int", metavar="FORWARD_PORT")
optParser.add_option("-o", "--protocol-type", dest="protocol_type", metavar="PROTOCOL_TYPE")

logger = logging.getLogger("proxy")

def logSMPPCommand(fromAddr, toAddr, cmd, status, seq, data):
    fmt = "[%s:%d] >>>> [%s:%d], cmd=0x%08X (%s), status=0x%08X, seq=0x%08X, len=%d"
    if logger.getEffectiveLevel() <= logging.DEBUG:
        fmt += "\n" + formatHexText(data)
    logger.info(fmt, 
        fromAddr.host, fromAddr.port, 
        toAddr.host, toAddr.port, 
        cmd, smpp.PDU_NAMES.get(cmd, "UNKNOWN"), status, seq, len(data),
    )


def logCMPPCommand(fromAddr, toAddr, cmd, seq, data):
    fmt = "[%s:%d] >>>> [%s:%d], cmd=0x%08X (%s), seq=0x%08X, len=%d"
    if logger.getEffectiveLevel() <= logging.DEBUG and data:
        fmt += "\n" + formatHexText(data)
    logger.info(fmt, 
            fromAddr.host, fromAddr.port, 
            toAddr.host, toAddr.port, 
            cmd, cmpp.PDU_NAMES.get(cmd, "UNKNOWN"), seq, len(data),
    )

def getProxyServerFactory(myProtocol, logFunc):
    class MyProxy(myProtocol):
        peer = None
    
        def setPeer(self, peer):
            self.peer = peer
    
        def connectionLost(self, reason):
            if self.peer is not None:
                self.peer.transport.loseConnection()
                self.peer = None
    
        def commandReceived(self, *argv):
            logFunc(self.transport.getPeer(), self.peer.transport.getPeer(), *argv)
            self.peer.sendCommand(*argv)
    
    class MyProxyClient(MyProxy):
        def connectionMade(self):
            addr = self.transport.getPeer()
            logger.info("connected to [%s:%d]", addr.host, addr.port)
            
            self.peer.setPeer(self)
            # We're connected, everybody can read to their hearts content.
            self.peer.transport.resumeProducing()
    
    class MyProxyClientFactory(protocol.ClientFactory):   
        protocol = MyProxyClient
    
        def setServer(self, server):
            self.server = server
    
        def buildProtocol(self, *args, **kw):
            prot = protocol.ClientFactory.buildProtocol(self, *args, **kw)
            prot.setPeer(self.server)
            return prot
    
        def clientConnectionFailed(self, connector, reason):
            self.server.transport.loseConnection()
    
    
    class MyProxyServer(MyProxy):
    
        clientProtocolFactory = MyProxyClientFactory
    
        def connectionMade(self):
            # Don't read anything from the connecting client until we have
            # somewhere to send it to.
            addr = self.transport.getPeer()
            logger.info("accepted from [%s:%d]", addr.host, addr.port)
            self.transport.pauseProducing()
    
            client = self.clientProtocolFactory()
            client.setServer(self)
    
            from twisted.internet import reactor
            reactor.connectTCP(self.factory.host, self.factory.port, client)
    
    
    class MyProxyFactory(protocol.Factory):
        """Factory for port forwarder."""
    
        protocol = MyProxyServer
    
        def __init__(self, host, port):
            self.host = host
            self.port = port
        
    return MyProxyFactory


def simpleProxyMain(optParser):
    opts = optParser.values
    if not opts.bind_port:
        optParser.error("BIND_PORT not found")
    if not opts.forward_addr:
        optParser.error("FORWARD_ADDR not found")
    if not opts.forward_port:
        optParser.error("FORWARD_PORT not found")

    if opts.protocol_type.upper() == "CMPP":
        serverFactoryClass = getProxyServerFactory(cmpp.CMPPProtocol, logCMPPCommand)
    elif opts.protocol_type.upper() == "SMPP":
        serverFactoryClass = getProxyServerFactory(smpp.SMPPProtocol, logSMPPCommand)
    else:
        optParser.error("PROTOCOL_TYPE not known")

    serverFactory = serverFactoryClass(opts.forward_addr, opts.forward_port)
    from twisted.internet import reactor
    reactor.listenTCP(opts.bind_port, serverFactory)
    reactor.run()
    

if __name__ == "__main__":
    easyMain(optParser, simpleProxyMain)
