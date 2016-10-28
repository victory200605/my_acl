# -*- coding: UTF-8 -*-
from optparse import OptionParser
import time
import logging

from twisted.application import internet, service
from twisted.internet import protocol
from twisted.python import log

from pyailib.twisted.protocol import cmpp
from pyailib.utils.sequence import *
from pyailib.utils.counter import *
from py_cmpp import v2 as cmpppkt    


parser = OptionParser(usage="%prog [options]")
#parser.add_option("-a", "--bind-addr", dest="addr", metavar="ADDR", default="")
parser.add_option("-p", "--bind-port", dest="port", type="int", metavar="BIND_PORT", )
parser.add_option("-u", "--username", dest="username", metavar="USER_NAME", default="")
parser.add_option("-s", "--shared-secret", dest="shared_secret", metavar="SHARED_SECRET")
parser.add_option("-r", "--response-code", dest="resp_code", type="int", metavar="RESP_CODE", default=0)
parser.add_option("-e", "--mtsr-status", dest="sr_status", metavar="MTSR_STATUS", default="")
parser.add_option("-i", "--ismg-number", dest="ismg_num", metavar="ISMG_NUM", default="000000")
parser.add_option("-t", "--timeout-seconds", dest="timeout", type="int", metavar="TIMEOUT_SEC", default=10)
parser.add_option("-w", "--window-size", dest="win_size", type="int", metavar="WIN_SIZE", default=16)
parser.add_option("-o", "--mo-max", dest="mo_max", type="int", metavar="MO_MAX", default=-1)


class SpeedCounter(TimedCounter):
    def __init__(self, label, *argv, **kwargs):
        super(SpeedCounter, self).__init__(*argv, **kwargs)
        self.label = label
        self.lastCounterTime = None
        self.total = 0
        
    def inc(self, *argv, **kwargs):
        r = super(SpeedCounter, self).inc(*argv, **kwargs)
        if not r is None:
            t, c = r
            if self.lastCounterTime != t:
                self.lastCounterTime = t
                self.total += c
                log.msg("%s speed = %d/s" % (self.label, c))
        return r
        
    def getTotal(self):
        return self.total + self.getValue()


class MyCMPPProtocol(cmpp.CMPPProtocol):
    def connectionMade(self):
        self.username = None
        self.terminateSeq = None
        self.seqGen = createCycledSequenceGenerator(1, 0x7FFFFFFF)
        self.reqWin = {}
        log.msg("new connection")

    def connectionLost(self, reason):
        log.msg("connection is broken, %s" % reason.getErrorMessage(), logLevel=logging.INFO)
    
    def commandReceived(self, cmd, seq, body):
        log.msg("[%s] RECV: command=0x%08X, sequence=0x%08X, len=%d, data=%r" % (self.username, cmd, seq, len(body), body), logLevel=logging.DEBUG)
        handler = getattr(self, "handle_%s" % cmpp.PDU_NAMES.get(cmd, "UNKNOWN"))
        return handler(seq, body)

    def sendCommand(self, cmd, seq, body=""):
        log.msg("[%s] SEND: command=0x%08X, sequence=0x%08X, len=%d, data=%r" % (self.username, cmd, seq, len(body), body), logLevel=logging.DEBUG)
        cmpp.CMPPProtocol.sendCommand(self, cmd, seq, body)
   
    def handle_UNKNOWN(self, cmd, seq, body):
        log.msg("unhandled command [0x%X]" % cmd, logLevel=logging.INFO)
        
    def handle_CONNECT(self, seq, body):
        connect = cmpppkt.Connect()
        connect.decode(body)
        connectResp = cmpppkt.Connect_Resp()
        if (not self.factory.username) or connect.source_addr == self.factory.username:
            connectResp.status = 0
        else:
            connectResp.status = 1
        connectResp.authenticator_ismg = cmpp.calcAuthenticatorISMG(
                connectResp.status,
                connect.authenticator_source, 
                self.factory.sharedSecret,
        )
        connectResp.version = 0x20
        self.sendCommand(cmpp.PDU_CONNECT_RESP, seq, connectResp.encode())
        if connectResp.status == 0:
            self.username = connect.source_addr
            log.msg("[%s] login successful" % self.username, logLevel=logging.INFO)
            self.notifyDeliver()
        else:
            log.msg("[%s] login failed" % connect.source_addr, logLevel=logging.ERROR)
            self.transport.loseConnection()

    def handle_TERMINATE(self, seq, body):
        self.sendCommand(cmpp.PDU_TERMINATE_RESP, seq)
        self.transport.loseConnection()
        
    def handle_ACTIVE_TEST(self, seq, body):
        self.sendCommand(cmpp.PDU_ACTIVE_TEST_RESP, seq, "0")
        self.notifyDeliver()
        
    def handle_TERMINATE_RESP(self, seq, body):
        if self.terminateSeq == seq:
            self.transport.loseConnection()
            
    def handle_SUBMIT(self, seq, body):
        self.factory.mtCounter.inc()
        submitReq = cmpppkt.Submit()
        submitReq.decode(body)
        
        submitResp = cmpppkt.Submit_Resp()
        submitResp.msg_id = self.factory.msgIDGen.next() 
        submitResp.status = self.factory.respCode
        self.sendCommand(cmpp.PDU_SUBMIT_RESP, seq, submitResp.encode())
        # add status report
        if submitResp.result == 0 and self.factory.srStatus:
            mtsr = cmpppkt.MT_StatReport()
            mtsr.msg_id = submitResp.msg_id
            mtsr.stat = self.factory.srStatus
        
            sr = cmpppkt.Deliver();
            sr.msg_src = submitReq.msg_src
            sr.dest_id = submitReq.src_id  
            sr.registered_delivery = 1

            for t in submitReq.dest_terminal_ids:
                mtsr.dest_terminal_id = t
                sr.src_terminal_id = t
                sr.msg_content = mtsr.encode()
                
                self.deliver(sr.encode())
                self.factory.srCounter.inc()
            
        self.notifyDeliver()
        
    
    def handle_DELIVER_RESP(self, seq, body):
        try:
            self.reqWin.pop(seq)
        except:
            pass
        self.notifyDeliver()
    
    def notifyDeliver(self):
        #for k, v in self.reqWin.items():
        #    if time.time() - v > self.factory.timeout:
        #        self.reqWin.pop(k)

        while len(self.reqWin) < self.factory.winSize and (self.factory.moMax < 0 or self.factory.moCounter.getTotal() < self.factory.moMax):
            self.deliver(self.factory.deliverGen.next())
        
    def deliver(self, body):
        self.factory.moCounter.inc()
        seq = self.seqGen.next()
        self.sendCommand(cmpp.PDU_DELIVER, seq, body)
        now = time.time()
        self.reqWin[seq] = now
        
    def terminate(self):
        self.terminateSeq = self.seq.next()
        self.sendCommand(cmpp.PDU_TERMINATE, self.terminateSeq)        
        
def ismgsimMain(optParser, deliverGeneratorFunc):
    opts = optParser.values
    
    if not opts.port:
        optParser.error("no PORT found")

    if not opts.ismg_num or len(opts.ismg_num) != 6:
        optParser.error("no ISMG_NO or invalid")
        

    factory = protocol.ServerFactory()
    factory.protocol = MyCMPPProtocol
    factory.username = opts.username
    factory.sharedSecret = opts.shared_secret
    factory.ismgNum = opts.ismg_num
    factory.msgIDGen = cmpp.createMessageIDGenerator(opts.ismg_num)
    factory.deliverGen = deliverGeneratorFunc(opts.ismg_num)
    factory.timeout = opts.timeout
    factory.winSize = opts.win_size
    factory.respCode = opts.resp_code
    factory.srStatus = opts.sr_status
    factory.moMax = opts.mo_max
    factory.mtCounter = SpeedCounter("MT")
    factory.moCounter = SpeedCounter("MO")
    factory.srCounter = SpeedCounter("SR")
    
    #reactor.suggestThreadPoolSize(20)
    from twisted.internet import reactor
    reactor.listenTCP(opts.port, factory)
    reactor.run()


def createDeliverGenerator(ismgNum):
    msgIDGen = cmpp.createMessageIDGenerator(ismgNum)
    d = cmpppkt.Deliver()
    
    # 固定值的字段在这里设置
    # d.msg_id = 
    d.dest_id = "10601111"
    d.service_id = "test1"
    d.tp_pid = 0
    d.tp_udhi = 0
    d.msg_fmt = 0
    # d.src_terminal_id =
    d.registered_delivery = 0
    # d.msg_length = # do not set this
    # d.msg_content =
      
    i = 0
    while True:
        d.msg_id = msgIDGen.next()
        
        # 每次循环都需要改变的字段在这里设置
        d.src_terminal_id = "1350123%04d" % (i % 1000)
        d.msg_content = "abcdefg_%04d" % (i % 1000) 
        
        i += 1
        yield d.encode()


opts = optParser.values

if not opts.port:
    optParser.error("no PORT found")

if not opts.ismg_num or len(opts.ismg_num) != 6:
    optParser.error("no ISMG_NO or invalid")
        

factory = protocol.ServerFactory()
factory.protocol = MyCMPPProtocol
factory.username = opts.username
factory.sharedSecret = opts.shared_secret
factory.ismgNum = opts.ismg_num
factory.msgIDGen = cmpp.createMessageIDGenerator(opts.ismg_num)
factory.deliverGen = deliverGeneratorFunc(opts.ismg_num)
factory.timeout = opts.timeout
factory.winSize = opts.win_size
factory.respCode = opts.resp_code
factory.srStatus = opts.sr_status
factory.moMax = opts.mo_max
factory.mtCounter = SpeedCounter("MT")
factory.moCounter = SpeedCounter("MO")
factory.srCounter = SpeedCounter("SR")
    
#reactor.suggestThreadPoolSize(20)
#from twisted.internet import reactor
#reactor.listenTCP(opts.port, factory)
#reactor.run()

application = service.Application("ISMG_ICP")
ismgICPService = internet.TCPServer(opts.port, factory)

