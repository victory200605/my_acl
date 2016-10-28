from zope.interface import implements

from twisted.internet import reactor, protocol
from twisted.application import service, strports
from twisted.python import log, usage
from twisted import plugin

from pyailib.protocols import scp, admin, ProtocolManagerMixin
from pyailib.xmldict import *

import random


rsp_body = """
<gw-scp>
<sm-%s-rsp stat="%d"/>
</gw-scp>
"""


class MyScpServerProtocol(scp.ScpServerProtocol):
    def connectionMade(self):
        scp.ScpServerProtocol.connectionMade(self)
        self.factory.registerProtocol(self)
        
    def connectionLost(self, reason):
        self.factory.unregisterProtocol(id(self))
        scp.ScpServerProtocol.connectionLost(self, reason)
        
    def onSessionBegin(self, *argv, **kw):
        log.msg(self.account, "is logined")
        
    def onSessionEnd(self, *argv, **kw):
        log.msg(self.account, "is logouted")
        
    def on_REQUEST(self, headers, body):
        new_headers = self.newHeaders(headers)
        status = self.factory.getStatus()
        rsp = ""
        d = self.factory.getDelay()
        if status is None:
            return
        elif "sm-chg-req" in body:
            if status == scp.ECMPP_OK:
                rsp = rsp_body % ("chg", self.factory.getStat())
            new_headers[scp.FIELD_CONTENT_LENGTH] = len(rsp)
            new_headers[scp.FIELD_CONTENT_TYPE] = "application/XML"
            if d:
                reactor.callLater(d,
                    self.sendMessage,
                    "RESPOND",
                    new_headers,
                    rsp,
                    status,
                )
            else:
                self.sendMessage("RESPOND", new_headers, rsp, status)
        elif "sm-cnfm-req" in body:
            if status == scp.ECMPP_OK:
                rsp = rsp_body % ("cnfm", self.factory.getStat())
            new_headers[scp.FIELD_CONTENT_LENGTH] = len(rsp)
            new_headers[scp.FIELD_CONTENT_TYPE] = "application/XML"
            if d:
                reactor.callLater(d,        
                    self.sendMessage,
                    "RESPOND",
                    new_headers,
                    rsp,
                    status,
                )
            else:
                self.sendMessage("RESPOND", new_headers, rsp, status)
        else:
            self.sendMessage(
                "RESPOND",
                new_headers,
                "",
                scp.ECMPP_INVALREQ,
            )
    
    
class MyScpServerFactory(protocol.Factory, ProtocolManagerMixin):
    protocol = MyScpServerProtocol
    
    def __init__(self, accounts, status, stat, delay=None):
        ProtocolManagerMixin.__init__(self)
        self.status_list = []
        for i in status.split(":"):
            r, w = i.split(",", 1)
            try:
                r = int(r)
            except:
                r = None
            w = int(w)
            for j in xrange(w):
                self.status_list.append(r)
                
        self.stat_list = []
        for i in stat.split(":"):
            r, w = i.split(",", 1)
            try:
                r = int(r)
            except:
                r = None            
            w = int(w)
            for j in xrange(w):
                self.stat_list.append(r)   
                             
        self.account_list = {}
        for a in accounts.split(":"):
            n, p = a.split(",", 1)
            self.account_list[n] = p
            
        if isinstance(delay, (str, unicode)):
            self.delay = float(delay)
        else:
            self.delay = delay

    def getPassword(self, account):
        return self.account_list.get(account)
    
    def getStatus(self):
        return random.choice(self.status_list)

    def getStat(self):
        return random.choice(self.stat_list)
        
    def getDelay(self):
        if self.delay is None:
            return None
        elif isinstance(self.delay, (list, tuple)):
            return random.randint(*self.delay)
        else:
            return self.delay


class Options(usage.Options):
    optParameters = [
        ("scp-port", "p", "9696", "The port number to listen on."),
        ("scp-delay", "d", 0, "Response delay time"),
        ("scp-status", "s", "200,1", "Response status list like 'status_1,weight_1:status_2,weight_2'"),
        ("scp-stat", "t", "0,1", "SCP stat list like 'stat_1,weight_1:stat_2,weight_2'"),
        ("scp-accounts", "a", "scp,scp", "Account list like 'account_1,password_1:account_2,password_2'"),
        ("admin-username", None, "admin", "Username to login admin shell"),
        ("admin-password", None, "secret", "Password to login admin shell"),
        ("admin-port", None, None, "Port to login admin shell"),
    ]


class MyServiceMaker(object):
    implements(service.IServiceMaker, plugin.IPlugin)
    tapname = "scp_server"
    description = "a simple scp server emulator"
    options = Options

    def makeService(self, options):
        my_service = service.MultiService()
        scp_server_factory = MyScpServerFactory(
            options["scp-accounts"],
            options["scp-status"],
            options["scp-stat"],
            options["scp-delay"],
        )
        scp_service = strports.service(
            options["scp-port"],
            scp_server_factory,
        )
        scp_service.setName("scp_server")
        scp_service.setServiceParent(my_service)
        admin_service = admin.makeAdminServer(
            options, 
            {"scp_server_factory":scp_server_factory},
        )
        if admin_service:
            admin_service.setServiceParent(my_service)
        return my_service


serviceMaker = MyServiceMaker()

if __name__ == "__main__":
    from twisted.scripts.twistd import run
    run()
