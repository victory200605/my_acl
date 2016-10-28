from twisted.internet import reactor, protocol, task
from twisted.python import log
import sys, time
from pyailib.protocols.scp import *
from pyailib.xmldict import dictToXmlAttrs

log.startLogging(sys.stderr)

total = 0

body = """<gw-scp><sm-chg-req sp-id="919051" service-id="-XXSQQ" msg-id="83a71cc007d14749" src-gw="002001" smc-addr="13800200500" gw-id="002001"><dst><addr val="13680846275"></addr></dst><chg-info msid="13680846275" rate="1000" type="3" cdrtype="2"></chg-info></sm-chg-req></gw-scp>"""

count = 0

delay = []

class MyScpClientProtocol(ScpClientProtocol):
    def connectionMade(self):
        self.tm = {}
        ScpClientProtocol.connectionMade(self)
        self.login(self.factory.account, self.factory.password)
        self.onSessionBegin()

        
    def onSessionBegin(self, *args, **kw):
        for i in xrange(100):
            self.sendOne()
    
    def sendOne(self, *argv, **kw):
        global count
        count += 1
        header = {"ServiceKey":1, "Content-Length":len(body), "Content-Type":"application/XML"}
        d = self.sendRequest("REQUEST", header, body)
        #d.addCallback(log.msg)
        def _rt(rsp):
            global delay
            delay.append(time.time() - self.tm.pop(rsp[1][FIELD_CSEQ]))
            return rsp
        d.addCallback(_rt)
        d.addCallback(self.sendOne)
        self.tm[header[FIELD_CSEQ]] = time.time()
        
    def connectionLost(self, reason):
        pass

def showSpeed():
    global count, delay
    print "speed:", count
    print "delay:", delay and float(sum(delay))/len(delay) or 0
    delay = []
    count = 0
    
l = task.LoopingCall(showSpeed)
l.start(1.0)

cf = protocol.ClientFactory()
cf.protocol = MyScpClientProtocol
cf.account = "scp"
cf.password = "scp"

#cf.account = "test"
#cf.password = "abcdefg"

for i in xrange(2):
    reactor.connectTCP("10.3.18.2", 9696, cf)
    #reactor.connectTCP("10.3.3.108", 11239, cf)
    #reactor.connectTCP("10.3.18.45", 17911, cf)
reactor.run()
