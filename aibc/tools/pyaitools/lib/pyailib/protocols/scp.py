from twisted.internet import defer, protocol
from twisted.python import log
from pyailib.protocols import PacketReceiver
from pyailib.utils import *
from pyailib.xmldict import *
import datetime
import hashlib
import re
import random


############################################################################

DATETIME_FMT = "%Y-%m-%dT%H:%M:%SZ"

SCP_RE = re.compile(r'(?P<start_line>.+)\r\n(?P<header>(.+\r\n)*)\r\n(?P<body>.*)', re.M)

LWS = r"(\r\n[\t ]*)"
LWS_RE = re.compile(LWS, re.M)
FIELD_NAME = r'([a-zA-Z][a-zA-Z0-9\-]*)'
BASE_VALUE = r'([^\r\n]*)'
SUB_VALUE = r'("[^\r\n"]*")'
SUB_FIELD = r'(%(FIELD_NAME)s=%(SUB_VALUE)s)' % locals()
FIELD_VALUE = r'((%(SUB_FIELD)s(%(LWS)s%(SUB_FIELD)s)+)|%(BASE_VALUE)s)' % locals()
HEADER = r'^(?P<name>%(FIELD_NAME)s):(?P<value>%(FIELD_VALUE)s)\r\n' % locals()
HEADER_RE = re.compile(HEADER, re.M)

############################################################################
FIELD_CSEQ = "CSeq"
FIELD_SERVICEKEY = "ServiceKey"
FIELD_FROM = "From"
FIELD_TO = "To"
FIELD_TIMESTAMP = "TimeStamp"
FIELD_VIA = "Via"
FIELD_RETRY_AFTER = "Retry-After"
FIELD_AUTHENTICATE = "Authenticate"
FIELD_AUTHORIZATION = "Authorization"
FIELD_LOGOUT = "Logout"
FIELD_CONTENT_LENGTH = "Content-Length"
FIELD_CONTENT_TYPE = "Content-Type"

FIELD2_NONCE = "nonce"
FIELD2_ALGORITHM = "algorithm"
FIELD2_AUTH_DIGEST = "auth-digest"
FIELD2_ACCOUNT = "account"

for k, v in locals().items():
    if k.startswith("FIELD_"):
        locals()[k] = v.upper()

PRI = {
    FIELD_CSEQ: 0,
    FIELD_SERVICEKEY: 1,
    FIELD_FROM: 2,
    FIELD_TO: 3,
    FIELD_TIMESTAMP: 4,
    FIELD_VIA: 5,
    FIELD_RETRY_AFTER: 6,
    FIELD_AUTHENTICATE: 7,
    FIELD_AUTHORIZATION: 8,
    FIELD_LOGOUT: 9,
    FIELD_CONTENT_LENGTH: 10,
    FIELD_CONTENT_TYPE: 11,
}



############################################################################

ECMPP_OK = 200
ECMPP_REGOK = 205
ECMPP_REGERR = 206
ECMPP_LOGOUTOK = 207
ECMPP_LOGOUTERR = 208
ECMPP_NOTEMPBILL = 209
ECMPP_INVALREQ = 400
ECMPP_UNAUTHORIZED = 401
ECMPP_REQENLARGE = 413
ECMPP_MONTERNET_MSG_ERROR = 414
ECMPP_SP_MSG_ERROR = 415
ECMPP_SERVERERR = 500
ECMPP_SERVERBUSY = 501
ECMPP_SCPLINKERR = 502
ECMPP_VERSIONERR = 505
ECMPP_SERVER_TIMEOUT = 506
INVALID_STATUS_ERRNO = - 1 
SERVICE_SUCCESS = 0
SERVICE_NOMUDATA = 1
SERVICE_DBPROCESSFAIL = 2
SERVICE_AUTHFAIL = 3
SERVICE_FAILED = 4
SERVICE_MUDATANOTAVAILABLE = 5
SERVICE_SERVICENOTAVAILABLE = 6
SERVICE_MULEFTMONEYNOTENOUGH = 7
SERVICE_EXCEEDMAXMONTHLYLIMIT = 8
SERVICE_EXCEEDMAXARREARLIMIT = 9
SERVICE_MUHAVEREGTHISSERVICE = 10
SERVICE_MUNOTREGTHISSERVICE = 11
SERVICE_MUEXIST = 12
SERVICE_SUBMIT_SM_FAILED = 13
SERVICE_GW_NOT_EXIST = 14
SERVICE_ALGORITHM_INVALID = 15
SERVICE_INVALID_PARAMETER = 16
SERVICE_DATA_NOTOVER = 17


############################################################################
def formatDatetime(dt):
    """format the Datetime object to scp style timestamp"""
    return dt.strftime(DATETIME_FMT)

def parseDatetime(s):
    """parse scp style timestamp to Datetime object"""
    datetime.datetime.strptime(s, DATETIME_FMT)

############################################################################
def formatCompositeHeader(fields):
    """format list of tuple(name, value) to scp composited header"""
    return "\r\n".join(['%s="%s"' % (k, v) for k, v in fields])

def parseCompositeHeader(s):
    """parse the scp composited header to list of tuple(name, value)"""
    return [(k.lower().lstrip(), v.strip('"')) for k, v in [i.split("=", 1) for i in LWS_RE.split(s)[::2]]]

############################################################################
def formatMessage(method, headers, body="", status=None, version="CMPP/1.2"):
    """
    header could be a dict, or list of str, or list of tuple
    """
    if status is None:
        start_line = "%s %s\r\n" % (method, version)
    else:
        start_line = "%s %s %s\r\n" % (method, version, status)
    if not headers:
        header_lines = []
    elif isinstance(headers, dict): 
        header_list = list(headers.items())
        header_list.sort(lambda x, y: PRI.get(x[0], 99) - PRI.get(y[0], 99))
        header_lines = "".join(["%s:%s\r\n" % (k, v) for k, v in header_list])
    elif isinstance(headers, (list, tuple)):
        if isinstance(headers[0], (tuple, list)):
            header_list = headers
            header_lines = "".join(["%s:%s\r\n" % (k, v) for k, v in header_list])
        else: # list of str
            header_lines = "".join(headers)
    return start_line + header_lines + "\r\n" + body


def parseMessage(data):
    r = SCP_RE.match(data).groupdict()
    start_line = r["start_line"]
    header_lines = r["header"]
    body = r["body"]
    
    headers = []
    while header_lines:
        r = HEADER_RE.match(header_lines)
        d = r.groupdict()
        headers.append((d["name"].upper(), d["value"]))
        header_lines = header_lines[r.end():]
                
    return start_line.split(), headers, body


############################################################################
def calcDigest(algorithm, timestamp, nonce, account, password):
     return hashlib.new(
            algorithm,
            ":".join([timestamp, nonce, account, password])
        ).hexdigest()


############################################################################

class ScpReceiver(PacketReceiver):
    CMPP_VERSION = "CMPP/1.2"
    FROM_IP = None
    TO_IP = None    
    recvd = ""

    def connectionMade(self):
        log.msg("SCP connection[", id(self), "] established,", self.transport.getHost(), " <=> ", self.transport.getPeer())
        
    def connectionLost(self, reason):
        log.msg("SCP connection[", id(self), "] closed,", reason.getErrorMessage())

    def onLengthLimitExceeded(self, length):
        log.msg("SCP message too large(%d)" % length)
        self.transport.loseConnection()
        
    def onPacketReceived(self, pkt):
        start_line, headers, body = parseMessage(pkt)
        self.onMessageReceived(start_line, dict(headers), body)
        
    def onMessageReceived(self, startline, headers, body):
        raise NotImplementedError

    def sendMessage(self, method, headers={}, body="", status=None):   
        self.sendPacket(formatMessage(method, headers, body, status=status))

############################################################################
class ScpClientProtocol(ScpReceiver):

    def connectionMade(self):
        ScpReceiver.connectionMade(self)
        self.seq_gen = SequenceGenerator(0, 2 ** 31 - 1, 1)
        self.win = PendingWindow()

    def newHeaders(self):
        return {
            FIELD_CSEQ:         "%d" % self.seq_gen.next(),
            FIELD_FROM:         self.FROM_IP or self.transport.getHost().host,
            FIELD_TO:           self.TO_IP or self.transport.getPeer().host,
            FIELD_TIMESTAMP:    formatDatetime(datetime.datetime.now()),
        }


    def sendRequest(self, method, headers, body=""):
        headers.update(self.newHeaders())
        #new_headers.update(headers)
        if isinstance(body, dict):
            body = dictToXmlAttrs(body)
        self.sendMessage(method, headers, body)
        deferred = self.win.put(headers[FIELD_CSEQ])
        deferred.addErrback(lambda _: self.pop(id))
        deferred.addErrback(lambda f: f.printDetailedTraceback())
        return deferred

    def onMessageReceived(self, start_line, headers, body):
        seq = headers[FIELD_CSEQ]
        self.win.callback(seq, (start_line, headers, body))

    def login(self, account, password):
        """ login session """
        self.account = account
        self.password = password
        d = self.sendRequest("LOGIN", self.newHeaders())
        d.addCallbacks(self.onLoginRsp, self.onSessionFailure)

    def onLoginRsp(self, result):
        start_line, headers, body = result
        method = start_line[0]
        status = start_line[2]
        if method != "LOGIN_RSP":
            self.onSessionFailure("failure")
        if int(status) != ECMPP_UNAUTHORIZED:
            self.onSessionFailure("failure")
        # send authentication
        fields = parseCompositeHeader(headers[FIELD_AUTHENTICATE])
        algorithms = [v  for k, v in fields if k == FIELD2_ALGORITHM]
        nonce = dict(fields)[FIELD2_NONCE]
        auth_digest = calcDigest(algorithms[0], headers[FIELD_TIMESTAMP], nonce, self.account, self.password)

        headers = self.newHeaders()
        headers.update({
            FIELD_AUTHORIZATION: formatCompositeHeader([
                (FIELD2_ACCOUNT, self.account),
                (FIELD2_NONCE, nonce),
                (FIELD2_AUTH_DIGEST, auth_digest),
                (FIELD2_ALGORITHM, algorithms[0]),
            ])
        })
                
        d = self.sendRequest("AUTHENTICATION", headers)
        d.addCallbacks(self.onAuthenticationRsp, self.onSessionFailure)


    def onAuthenticationRsp(self, result):
        start_line, headers, body = result
        method = start_line[0]
        status = start_line[2]
        if method != "AUTHENTICATION_RSP":
            self.onSessionFailure("1")
        elif int(status) != ECMPP_REGOK:
            self.onSessionFailure("2")
        else:
            self.onSessionBegin()


    def logout(self):
        """ logout session """
        d = self.sendRequest(
            "LOGOUT",
            self.newHeaders()
        )
        d.addCallbacks(self.onSessionEnd, self.onSessionFailure)

    def onSessionBegin(self, *argv, **kw):
        pass

    def onSessionEnd(self, *argv, **kw):
        self.transport.loseConnection()

    def onSessionFailure(self, *argv, **kw):
        self.transport.loseConnection() 


############################################################################
class ScpServerProtocol(ScpReceiver):
    def __init__(self):
        self.nonce = False
        self.timestamp = True
        
        
    def connectionMade(self):
        self.FROM_IP = self.transport.getHost().host
        self.TO_IP = self.transport.getPeer().host
        
        
    def newHeaders(self, headers):
        return {
            FIELD_CSEQ:         headers[FIELD_CSEQ],
            FIELD_FROM:         self.FROM_IP or self.transport.getHost().host,
            FIELD_TO:           self.TO_IP or self.transport.getPeer().host,
            FIELD_TIMESTAMP:    formatDatetime(datetime.datetime.now()),
        }
            
    def onMessageReceived(self, start_line, headers, body):
        method = start_line[0]
        func = getattr(self, "on_" + method)
        func(headers, body)

    def on_LOGIN(self, headers, body):
        rsp_headers = self.newHeaders(headers)
        self.nonce = "%032x" % random.randint(0, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFL)
        self.timestamp = rsp_headers[FIELD_TIMESTAMP]
        rsp_headers[FIELD_AUTHENTICATE] = formatCompositeHeader([
            (FIELD2_NONCE, self.nonce),
            (FIELD2_ALGORITHM, "MD5"),
            (FIELD2_ALGORITHM, "SHA"),
        ])
        self.sendMessage("LOGIN_RSP", rsp_headers, status=ECMPP_UNAUTHORIZED)

    def on_AUTHENTICATION(self, headers, body):
        a = dict(parseCompositeHeader(headers[FIELD_AUTHORIZATION]))
        account = a[FIELD2_ACCOUNT]
        password = self.factory.getPassword(account) or ""
        if password is None: 
            status = ECMPP_REGERR
        else:
            self.account = account
            auth_digest = calcDigest(a[FIELD2_ALGORITHM], self.timestamp, self.nonce, account, password).lower()
            if auth_digest == a[FIELD2_AUTH_DIGEST]:
                status = ECMPP_REGOK
            else:
                log.msg("mismatch auth-digest, got[%s] want[%s]" % (auth_digest, a["auth-digest"]))
                status = ECMPP_REGERR
        self.sendMessage("AUTHENTICATION_RSP", self.newHeaders(headers), status=status)
        if status == ECMPP_REGOK:
            self.onSessionBegin()
        else:
            self.onSessionFailure("login failure!")
        
        
    def on_LOGOUT(self, headers, body):
        self.sendMessage("LOGOUT_RSP", self.newHeaders(headers), status=ECMPP_LOGOUTOK)
        self.onSessionEnd()
        
    def on_SHAKEHAND(self, headers, body):
        self.sendMessage("SHAKEHAND_RSP", self.newHeaders(headers), status=ECMPP_OK)

    def onSessionBegin(self, *argv, **kw):
        pass
    
    def onSessionEnd(self, *argv, **kw):
        pass
    
    def onSessionFailure(self, reason):
        pass
        
    def on_REQUEST(self, haders, body):
        pass

