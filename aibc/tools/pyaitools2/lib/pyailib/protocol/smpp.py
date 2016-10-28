from twisted.internet import protocol, defer, reactor, task
from twisted.python import log

from struct import *
from time import time

from pyailib.util import *

############################################################################
SMPP_GENERIC_NACK = 0x80000000
SMPP_BIND_RECEIVER_REQ = 0x00000001
SMPP_BIND_RECEIVER_RSP = 0x80000001
SMPP_BIND_TRANSMITTER_REQ = 0x00000002
SMPP_BIND_TRANSMITTER_RSP = 0x80000002
#SMPP_QUERY_SM_REQ = 0x00000003
#SMPP_QUERY_SM_RSP = 0x80000003
SMPP_SUBMIT_SM_REQ = 0x00000004
SMPP_SUBMIT_SM_RSP = 0x80000004
SMPP_DELIVER_SM_REQ = 0x00000005
SMPP_DELIVER_SM_RSP = 0x80000005
SMPP_UNBIND_REQ = 0x00000006
SMPP_UNBIND_RSP = 0x80000006
#SMPP_REPLACE_SM_REQ = 0x00000007
#SMPP_REPLACE_SM_RSP = 0x80000007
#SMPP_CANCEL_SM_REQ = 0x00000008
#SMPP_CANCEL_SM_RSP = 0x80000008
SMPP_BIND_TRANSCEIVER_REQ = 0x00000009
SMPP_BIND_TRANSCEIVER_RSP = 0x80000009
SMPP_OUTBIND_REQ = 0x0000000B
SMPP_ENQUIRE_LINK_REQ = 0x00000015
SMPP_ENQUIRE_LINK_RSP = 0x80000015

############################################################################
SMPP_NAMES = dict([(v, k[5:])
    for k, v in locals().items()
    if k.startswith("SMPP_") and isinstance(v, (int, long))
])

############################################################################
SMPP_STATUS_ROK = 0x00000000
SMPP_STATUS_RINVMSGLEN = 0x00000001
SMPP_STATUS_RINVCMDLEN = 0x00000002
SMPP_STATUS_RINVCMDID = 0x00000003
SMPP_STATUS_RINVBNDSTS = 0x00000004
SMPP_STATUS_RALYBND = 0x00000005
SMPP_STATUS_RINVPRTFLG = 0x00000006
SMPP_STATUS_RINVREGDLVFLG = 0x00000007
SMPP_STATUS_RSYSERR = 0x00000008
SMPP_STATUS_RINVSRCADR = 0x0000000A
SMPP_STATUS_RINVDSTADR = 0x0000000B
SMPP_STATUS_RINVMSGID = 0x0000000C
SMPP_STATUS_RBINDFAIL = 0x0000000D
SMPP_STATUS_RINVPASWD = 0x0000000E
SMPP_STATUS_RINVSYSID = 0x0000000F
SMPP_STATUS_RCANCELFAIL = 0x00000011
SMPP_STATUS_RREPLACEFAIL = 0x00000013
SMPP_STATUS_RMSGQFUL = 0x00000014
SMPP_STATUS_RINVSERTYP = 0x00000015
SMPP_STATUS_RINVNUMDESTS = 0x00000033
SMPP_STATUS_RINVDLNAME = 0x00000034
SMPP_STATUS_RINVDESTFLAG = 0x00000040
SMPP_STATUS_RINVSUBREP = 0x00000042
SMPP_STATUS_RINVESMCLASS = 0x00000043
SMPP_STATUS_RCNTSUBDL = 0x00000044
SMPP_STATUS_RSUBMITFAIL = 0x00000045
SMPP_STATUS_RINVSRCTON = 0x00000048
SMPP_STATUS_RINVSRCNPI = 0x00000049
SMPP_STATUS_RINVDSTTON = 0x00000050
SMPP_STATUS_RINVDSTNPI = 0x00000051
SMPP_STATUS_RINVSYSTYP = 0x00000053
SMPP_STATUS_RINVREPFLAG = 0x00000054
SMPP_STATUS_RINVNUMMSGS = 0x00000055
SMPP_STATUS_RTHROTTLED = 0x00000058
SMPP_STATUS_RINVSCHED = 0x00000061
SMPP_STATUS_RINVEXPIRY = 0x00000062
SMPP_STATUS_RINVDFTMSGID = 0x00000063
SMPP_STATUS_RX_T_APPN = 0x00000064
SMPP_STATUS_RX_P_APPN = 0x00000065
SMPP_STATUS_RX_R_APPN = 0x00000066
SMPP_STATUS_RQUERYFAIL = 0x00000067
SMPP_STATUS_RINVOPTPARSTREAM = 0x000000C0
SMPP_STATUS_ROPTPARNOTALLWD = 0x000000C1
SMPP_STATUS_RINVPARLEN = 0x000000C2
SMPP_STATUS_RMISSINGOPTPARAM = 0x000000C3
SMPP_STATUS_RINVOPTPARAMVAL = 0x000000C4
SMPP_STATUS_RDELIVERYFAILURE = 0x000000FE
SMPP_STATUS_RUNKNOWNERR = 0x000000FF



############################################################################
SMPP_ERROR_MESSAGES = {
    SMPP_STATUS_ROK :"No Error",
    SMPP_STATUS_RINVMSGLEN :"Message Length is invalid",
    SMPP_STATUS_RINVCMDLEN :"Command Length is invalid",
    SMPP_STATUS_RINVCMDID :"Invalid Command ID",
    SMPP_STATUS_RINVBNDSTS :"Incorrect BIND Status for given command",
    SMPP_STATUS_RALYBND :"ESME Already in Bound State",
    SMPP_STATUS_RINVPRTFLG :"Invalid Priority Flag",
    SMPP_STATUS_RINVREGDLVFLG :"Invalid Registered Delivery Flag",
    SMPP_STATUS_RSYSERR :"System Error",
    SMPP_STATUS_RINVSRCADR :"Invalid Source Address",
    SMPP_STATUS_RINVDSTADR :"Invalid Dest Addr",
    SMPP_STATUS_RINVMSGID :"Message ID is invalid",
    SMPP_STATUS_RBINDFAIL :"Bind Failed",
    SMPP_STATUS_RINVPASWD :"Invalid Password",
    SMPP_STATUS_RINVSYSID :"Invalid System ID",
    SMPP_STATUS_RCANCELFAIL :"Cancel SM Failed",
    SMPP_STATUS_RREPLACEFAIL :"Replace SM Failed",
    SMPP_STATUS_RMSGQFUL :"Message Queue Full",
    SMPP_STATUS_RINVSERTYP :"Invalid Service Type",
    SMPP_STATUS_RINVNUMDESTS :"Invalid number of destinations",
    SMPP_STATUS_RINVDLNAME :"Invalid Distribution List name",
    SMPP_STATUS_RINVDESTFLAG :"Destination flag is invalid (submit_multi)",
    SMPP_STATUS_RINVSUBREP :"Invalid 'submit with replace' request (i.e. submit_sm with replace_if_present_flag set)",
    SMPP_STATUS_RINVESMCLASS :"Invalid esm_class field data",
    SMPP_STATUS_RCNTSUBDL :"Cannot Submit to Distribution List",
    SMPP_STATUS_RSUBMITFAIL :"submit_sm or submit_multi failed",
    SMPP_STATUS_RINVSRCTON :"Invalid Source address TON",
    SMPP_STATUS_RINVSRCNPI :"Invalid Source address NPI",
    SMPP_STATUS_RINVDSTTON :"Invalid Destination address TON",
    SMPP_STATUS_RINVDSTNPI :"Invalid Destination address NPI",
    SMPP_STATUS_RINVSYSTYP :"Invalid system_type field",
    SMPP_STATUS_RINVREPFLAG :"Invalid replace_if_present flag",
    SMPP_STATUS_RINVNUMMSGS :"Invalid number of messages",
    SMPP_STATUS_RTHROTTLED :"Throttling error (ESME has",
    SMPP_STATUS_RINVSCHED :"Invalid Scheduled Delivery Time",
    SMPP_STATUS_RINVEXPIRY :"Invalid message validity period (Expiry time)",
    SMPP_STATUS_RINVDFTMSGID :"Predefined Message Invalid or Not Found",
    SMPP_STATUS_RX_T_APPN :"ESME Receiver Temporary App Error Code",
    SMPP_STATUS_RX_P_APPN :"ESME Receiver Permanent App Error Code",
    SMPP_STATUS_RX_R_APPN :"ESME Receiver Reject Message Error Code",
    SMPP_STATUS_RQUERYFAIL :"query_sm request failed",
    SMPP_STATUS_RINVOPTPARSTREAM :"Error in the optional part of the PDU Body.",
    SMPP_STATUS_ROPTPARNOTALLWD :"Optional Parameter not allowed",
    SMPP_STATUS_RINVPARLEN :"Invalid Parameter Length.",
    SMPP_STATUS_RMISSINGOPTPARAM :"Expected Optional Parameter missing",
    SMPP_STATUS_RINVOPTPARAMVAL :"Invalid Optional Parameter Value",
    SMPP_STATUS_RDELIVERYFAILURE :"Delivery Failure (used for data_sm_resp)",
    SMPP_STATUS_RUNKNOWNERR :"Unknown Error",
}

############################################################################
class SMPPException(Exception):
    KW = ("command_id", "sequence_number", "mandatory", "optional", "body", "header")
    def __init__(self, command_status, *argv, **kw):
        Exception.__init__(self, *argv)
        self.command_status = command_status
        self.kw = kw
        
    def __str__(self):
        s = ["SMPP Exception: command_status=0x%08X(%s)" % (
            self.command_status,
            SMPP_ERROR_MESSAGES.get(self.command_status, "RESERVED ERROR!"),
        )]
        
        for k in self.KW:
            try:
                v = self.kw[k]
                if isinstance(v, long):
                    s.append("%s=0x%08X" % (k, v))
                else:
                    s.append("%s=%r" % (k, v))
            except:
                pass
        return ",".join(s)

############################################################################
SMPP_RESPONSE_MASK = 0x80000000

def isResponse(command_id):
    return command_id & SMPP_RESPONSE_MASK

def isRequest(command_id):
    return not (command_id & SMPP_RESPONSE_MASK)

def getResponse(command_id):
    assert isRequest(command_id)
    return command_id | SMPP_RESPONSE_MASK

def getRequest(command_id):
    assert isResponse(command_id)
    return command_id & 0x7FFFFFFF


############################################################################
def decodeStr(bytes):
    i = bytes.index("\0")
    return bytes[:i], bytes[i + 1:]

def decodeBytes(bytes, size):
    return bytes[:size], bytes[size:]

def decodeUInt32(bytes):
    return unpack("!I", bytes[:4])[0], bytes[4:]

def decodeUInt16(bytes):
    return unpack("!H", bytes[:2])[0], bytes[2:]

def decodeUInt8(bytes):
    return unpack("!B", bytes[:1])[0], bytes[1:]

def encodeStr(s, out, terminator="\0"):
    out.append(s)
    out.append(terminator)
    
def encodeBytes(bytes, out):
    out.append(bytes)
    
def encodeUInt32(i, out):
    out.append(pack("!I", i))

def encodeUInt16(i, out):
    out.append(pack("!H", i))
    
def encodeUInt8(i, out):
    out.append(pack("!B", i))

############################################################################
def decode_BIND_REQ(bytes):
    msg = {}
    msg["system_id"], bytes = decodeStr(bytes) 
    msg["password"], bytes = decodeStr(bytes) 
    msg["system_type"], bytes = decodeStr(bytes) 
    msg["interface_version"], bytes = decodeUInt8(bytes) 
    msg["addr_ton"], bytes = decodeUInt8(bytes) 
    msg["addr_npi"], bytes = decodeUInt8(bytes) 
    msg["address_range"], bytes = decodeStr(bytes) 
    return msg, bytes


def encode_BIND_REQ(msg):
    out = []
    encodeStr(msg.get("system_id", ""), out)
    encodeStr(msg.get("password", ""), out)
    encodeStr(msg.get("system_type", ""), out)
    encodeUInt8(msg.get("interface_version", 0x34), out)
    encodeUInt8(msg.get("addr_ton", 0), out)
    encodeUInt8(msg.get("addr_npi", 0), out)
    encodeStr(msg.get("address_range", ""), out)
    return "".join(out)

def decode_BIND_RSP(bytes):
    msg = {}
    msg["system_id"], bytes = decodeEx(bytes, decodeStr) 
    return msg, bytes

def encode_BIND_RSP(msg):
    out = []
    encodeStr(msg.get("system_id", ""), out)
    return "".join(out)

def decode_SM_REQ(bytes):
    msg = {}
    msg["service_type"], bytes = decodeStr(bytes)
    msg["source_addr_ton"], bytes = decodeUInt8(bytes)
    msg["source_addr_npi"], bytes = decodeUInt8(bytes)
    msg["source_addr"], bytes = decodeStr(bytes)
    msg["dest_addr_ton"], bytes = decodeUInt8(bytes)
    msg["dest_addr_npi"], bytes = decodeUInt8(bytes)
    msg["destination_addr"], bytes = decodeStr(bytes)
    msg["esm_class"], bytes = decodeUInt8(bytes)
    msg["protocol_id"], bytes = decodeUInt8(bytes)
    msg["priority_flag"], bytes = decodeUInt8(bytes)
    msg["schedule_delivery_time"], bytes = decodeStr(bytes)
    msg["validity_period"], bytes = decodeStr(bytes)
    msg["registered_delivery"], bytes = decodeUInt8(bytes)
    msg["replace_if_present_flag"], bytes = decodeUInt8(bytes)
    msg["data_coding"], bytes = decodeUInt8(bytes)
    msg["sm_default_msg_id"], bytes = decodeUInt8(bytes)
    sm_length, bytes = decodeUInt8(bytes)
    msg["short_message"], bytes = decodeBytes(bytes, sm_length)
    return msg, bytes

def encode_SM_REQ(msg):
    out = []
    encodeStr(msg.get("service_type", ""), out)
    encodeUInt8(msg.get("source_addr_ton", 0), out)
    encodeUInt8(msg.get("source_addr_npi", 0), out)
    encodeStr(msg.get("source_addr", ""), out)
    encodeUInt8(msg.get("dest_addr_ton", 0), out)
    encodeUInt8(msg.get("dest_addr_npi", 0), out)
    encodeStr(msg.get("destination_addr", ""), out)
    encodeUInt8(msg.get("esm_class", 0), out)
    encodeUInt8(msg.get("protocol_id", 0), out)
    encodeUInt8(msg.get("priority_flag", 0), out)
    encodeStr(msg.get("schedule_delivery_time", ""), out)
    encodeStr(msg.get("validity_period", ""), out)
    encodeUInt8(msg.get("registered_delivery", 0), out)
    encodeUInt8(msg.get("replace_if_present_flag", 0), out)
    encodeUInt8(msg.get("data_coding", 0), out)
    encodeUInt8(msg.get("sm_default_msg_id", 0), out)
    encodeUInt8(len(msg.get("short_message", "")), out)
    encodeBytes(msg.get("short_message", ""), out)
    return "".join(out)

def decode_SM_RSP(bytes):
    msg = {}
    msg["message_id"], bytes = decodeStr(bytes)
    return msg, bytes
  
def encode_SM_RSP(msg):
    out = []
    encodeStr(msg.get("message_id", ""), out)
    return "".join(out)

def decode_NULL(bytes):
    return {}, bytes

def encode_NULL(msg):
    return ""

############################################################################
SMPP34_ENCODERS = {
    SMPP_BIND_TRANSMITTER_REQ: encode_BIND_REQ,
    SMPP_BIND_TRANSMITTER_RSP: encode_BIND_RSP,
    SMPP_BIND_TRANSCEIVER_REQ: encode_BIND_REQ,
    SMPP_BIND_TRANSCEIVER_RSP: encode_BIND_RSP,
    SMPP_BIND_RECEIVER_REQ: encode_BIND_REQ,
    SMPP_BIND_RECEIVER_RSP: encode_BIND_RSP,
    SMPP_SUBMIT_SM_REQ: encode_SM_REQ,
    SMPP_SUBMIT_SM_RSP: encode_SM_RSP,
    SMPP_DELIVER_SM_REQ: encode_SM_REQ,
    SMPP_DELIVER_SM_RSP: encode_SM_RSP,
    SMPP_ENQUIRE_LINK_REQ: encode_NULL,
    SMPP_ENQUIRE_LINK_RSP: encode_NULL,
    SMPP_UNBIND_REQ: encode_NULL,
    SMPP_UNBIND_RSP: encode_NULL,
    SMPP_GENERIC_NACK: encode_NULL,
}

SMPP34_DECODERS = {
    SMPP_BIND_TRANSMITTER_REQ: decode_BIND_REQ,
    SMPP_BIND_TRANSMITTER_RSP: decode_BIND_RSP,
    SMPP_BIND_TRANSCEIVER_REQ: decode_BIND_REQ,
    SMPP_BIND_TRANSCEIVER_RSP: decode_BIND_RSP,
    SMPP_BIND_RECEIVER_REQ: decode_BIND_REQ,
    SMPP_BIND_RECEIVER_RSP: decode_BIND_RSP,
    SMPP_SUBMIT_SM_REQ: decode_SM_REQ,
    SMPP_SUBMIT_SM_RSP: decode_SM_RSP,
    SMPP_DELIVER_SM_REQ: decode_SM_REQ,
    SMPP_DELIVER_SM_RSP: decode_SM_RSP,
    SMPP_ENQUIRE_LINK_REQ: decode_NULL,
    SMPP_ENQUIRE_LINK_RSP: decode_NULL,
    SMPP_UNBIND_REQ: decode_NULL,
    SMPP_UNBIND_RSP: decode_NULL,
    SMPP_GENERIC_NACK: decode_NULL,
}


def decodeOptional(bytes):
    try:
        optional = []
        while bytes:
            tag, bytes = decodeUInt16(bytes)
            length, bytes = decodeUInt16(bytes)
            value, bytes = decodeBytes(bytes, length)
            optional.append((tag, value))
        return optional
    except:
        raise SMPPException(SMPP_STATUS_RINVTLVSTREAM, command_id=command_id)

def encodeOptional(optional):
    out = []
    for tag, value in optional:
        encodeUInt16(tag, out)
        encodeUInt16(len(value), out)
        encodeBytes(value)
    return "".join(out)


def decodeMandatory(command_id, bytes):
    try:
        decoder = SMPP34_DECODERS[command_id]
    except:
        raise SMPPException(SMPP_STATUS_RINVCMDID, command_id=command_id, body=bytes)
    
    return decoder(bytes)


def encodeMandatory(command_id, mandatory):
    try:
        encoder = SMPP34_ENCODERS[command_id]
    except:
        raise SMPPException(SMPP_STATUS_RINVCMDID, command_id=command_id)
    return encoder(mandatory)


def decode(command_id, bytes):
    mandatory, bytes = decodeMandatory(command_id, bytes)
    optional = decodeOptional(bytes)
    return mandatory, optional

def encode(command_id, mandatory={}, optional=[]):
    body1 = encodeMandatory(command_id, mandatory)
    body2 = encodeOptional(optional)
    return body1 + body2

############################################################################
def newReceipt(**kw):
    return "id:%(id)s sub:%(sub)s dlvrd:%(dlvrd)s submit date:%(submit date)s done date:%(done date)s stat:%(stat)s err:%(err)s text:%(text)s" % kw
    
############################################################################
class SMPPProtocol(protocol.Protocol):
    SMPP_HEADER_FMT = "!IIII"
    SMPP_HEADER_LEN = calcsize(SMPP_HEADER_FMT)
    SMPP_LEN_MAX = 1024
    
    in_buf = ""
    
    INACTIVITY_TIMER = 15
    
    last_recv_time = 0
    last_send_time = 0
    
    seq = 1
    
    recv_counter = 0
    send_counter = 0

    session_state = "CLOSED"
    
    LOG_MSG_IO = True
            
    def getState(self):
        return self.session_state
    
    def setState(self, state):
        if state != self.session_state:
            old_state = self.session_state
            self.session_state = state
            self.updateName()
            self.stateChanged(old_state)
    
    def stateChanged(self, old_state):
        pass
        
    def updateName(self):
        host = self.transport.getHost()
        peer = self.transport.getPeer()
        self.name = "%s,%s:%d,%s:%d"%(
            self.session_state,
            host.host,
            host.port,
            peer.host,
            peer.port,
        )
                
    def connectionMade(self):
        self.updateName()
    
    def msg(self, *argv, **kw):
        kw["system"] = self.name
        log.msg(*argv, **kw)
        
    def err(self, *argv, **kw):
        kw["system"] = self.name
        log.err(*argv, **kw)
        
    def newSeq(self):
        s = self.seq
        self.seq += 1
        if self.seq > 0x7FFFFFFF:
            self.seq = 1
        return s
            
    def dataReceived(self, data):
        self.in_buf += data
        buf_len = len(self.in_buf)
        while buf_len >= self.SMPP_HEADER_LEN:
            command_length, command_id, command_status, sequence_number = unpack(
                self.SMPP_HEADER_FMT,
                self.in_buf[:self.SMPP_HEADER_LEN],
            )
            
            if command_length < self.SMPP_HEADER_LEN or command_length > self.SMPP_LEN_MAX:
                self.msg("invalid SMPP command_length(%d)" % length)
                self.transport.loseConnection()
                break
            
            if buf_len >= command_length:
                body = self.in_buf[self.SMPP_HEADER_LEN:command_length]
                self.in_buf = self.in_buf[command_length:]
                self.last_recv_time = reactor.seconds()
                self.recv_counter += 1
                if self.LOG_MSG_IO:
                    self.msg("recv: 0x%08X/0x%08X/0x%08X (len=%d)"%(command_id, command_status, sequence_number, len(body)))
                try:
                    self.packetReceived(command_id, command_status, sequence_number, body)
                except Exception, e:
                    self.msg("exception: %s"%e)
                    self.transport.loseConnection()
            else:
                break
            
            buf_len = len(self.in_buf)

    def packetReceived(self, command_id, command_status, sequence_number, body):
        try:
            name = SMPP_NAMES[command_id]
            func = getattr(self, "handle_%s"%name)            
        except Excpetion, e:
            self.msg("exception: %s"%e)
            raise SMPPException(SMPP_STATUS_RINVCMDID)
        
        func(command_status, sequence_number, body)

    # def packet_*_Received(...)
    # raising exceptions would make the connection lost
    
    def sendPacket(self, command_id, command_status, sequence_number=None, body=""):
        hdr = pack(
            self.SMPP_HEADER_FMT,
            len(body) + self.SMPP_HEADER_LEN,
            command_id,
            command_status,
            self.newSeq() if sequence_number is None else sequence_number,
        )
        self.transport.write(hdr + body)
        self.send_counter += 1
        self.last_send_time = reactor.seconds()
        if self.LOG_MSG_IO:
            self.msg("send: 0x%08X/0x%08X/0x%08X (len=%d)"%(command_id, command_status, sequence_number, len(body)))
        

############################################################################
class SMPPSession(SMPPProtocol):
    INACTIVITY_TIMER = 15
    ENQUIRE_LINK_TIMER = 5
    RESPONSE_TIMER = 10
    MAX_WINDOW_SIZE = 32
    TIMER_PRECISION = 1.0
    
    established_time = None
    
    def connectionMade(self):
        SMPPProtocol.connectionMade(self)
        self.established_time = reactor.seconds()
        self.window = AsynchWindow(
            self.MAX_WINDOW_SIZE,
            1,
            self.RESPONSE_TIMER,
        )
        self.last_recv_time = reactor.seconds()
        self.last_send_time = reactor.seconds()
        self.timer = task.LoopingCall(self.timerTriggered)
        self.timer.start(1.0)
        self.setState("OPEN")
        
    def connectionLost(self, reason):
        self.timer.stop()
        self.window.purge(reason)
    
    def isWindowFull(self):
        return self.window.isFull()
    
    def timerTriggered(self):
        t = reactor.seconds() - self.last_recv_time
        if t >= self.INACTIVITY_TIMER:
            self.msg("inactivity for %f seconds" % t)
            self.transport.loseConnection()
        else:
            t = reactor.seconds() - self.last_send_time
            if t >= self.ENQUIRE_LINK_TIMER:
                self.msg("enquire link after %f seconds idle" % t)
                self.sendRequest(
                    SMPP_ENQUIRE_LINK_REQ,
                    size=0,
                )
                
    def packetReceived(self, command_id, command_status, sequence_number, body):
        if isResponse(command_id):
            try:              
                self.window.callback(sequence_number, (command_status, body))
            except KeyError:
                try:
                    SMPPProtocol.packetReceived(self, command_id, command_status, sequence_number, body)
                except:
                    pass                
            except Exception, e:
                self.err()
        else:
            try:
                SMPPProtocol.packetReceived(self, command_id, command_status, sequence_number, body)
            except SMPPException, e:
                self.err()
                self.sendResponse(SMPP_GENERIC_NACK, e.command_status, sequence_number)
            except:
                self.err()
                self.sendResponse(SMPP_GENERIC_NACK, SMPP_STATUS_RSYSERR, sequence_number)
    
    def handle_ENQUIRE_LINK_REQ(self, command_status, sequence_number, body):
        self.sendResponse(SMPP_ENQUIRE_LINK_RSP, 0, sequence_number)   
        
    def sendResponse(self, command_id, command_status=0, sequence_number=None, mandatory={}, optional=[]):
        assert isResponse(command_id)
        self.sendPacket(
            command_id,
            command_status,
            sequence_number,
            encode(command_id, mandatory, optional),
        )
        return sequence_number
        
    def sendRequest(self, command_id, command_status=0, sequence_number=None, mandatory={}, optional=[], size=1, timeout=None):
        assert isRequest(command_id)
        
        if self.window.isFull():
            raise SMPPException(SMPP_STATUS_RTHROTTLED)
        
        if sequence_number is None:
            sequence_number = self.newSeq()
            
        self.sendPacket(
            command_id,
            command_status,
            sequence_number,
            encode(command_id, mandatory, optional),
        )
        
        return self.window.push(sequence_number, size, timeout), sequence_number

############################################################################
def guard(*states):
    def _guard(func):
        def _wrapper(self, *argv, **kw):
            if self.getState() not in states:
                raise SMPPException(SMPP_STATUS_RINVBNDSTS)
            return func(self, *argv, **kw)
        _wrapper.__name__ = func.__name__
        _wrapper.__doc__ = func.__doc__
        return _wrapper
    return _guard

############################################################################
def decodePacket(command_id):
    def _decodePacket(func):
        def _wrapper(self, command_status, sequence_number, body):
            mandatory, optional = decode(command_id, body)
            return func(self, command_status, sequence_number, mandatory, optional)
        _wrapper.__name__ = func.__name__
        _wrapper.__doc__ = func.__doc__
        return _wrapper
    return _decodePacket
        