from socket import *
from struct import *
from threading import *
import logging
import time

############################################################################
MAX_NAME_LEN = 128
MAX_INT_LEN = 25
MAX_DATA_LEN = 592
IP_ADDR_LEN = 20

############################################################################
M_WSEG_REQ = 0x03
M_WSEG2_REQ = 0x04
M_WSEG_RSP = 0x13

M_RSEG_REQ = 0x05
M_RSEG2_REQ = 0x06
M_RSEG_RSP = 0x14

M_STAT_REQ = 0x07
M_STAT_RSP = 0x15

S_PUT_REQ = 0x41
S_PUT_RSP = 0x51

S_GET_REQ = 0x42
S_GET_RSP = 0x52

############################################################################
class AsyncTcpError(Exception):
    pass 


class GFQProtocolError(Exception):
    def __init__(self, state, *args, **kw):
        Exception.__init__(self, *args, **kw)
        self.state = state

############################################################################
class AsyncTcpConnection(object):
    HDR_FMT = Struct("!iii")
    
    MAX_LEN = 1024

    SOCKET_TIMEOUT = 5

    logger = logging.getLogger("async_tcp_protocol")

    def __init__(self, addr):
        self.seq = 0
        self.in_buf = "" 
        self.socket = socket(AF_INET, SOCK_STREAM, 0)
        self.socket.settimeout(self.SOCKET_TIMEOUT)
        self.socket.connect(addr)

    def newSeq(self):
        seq = self.seq
        self.seq + 1
        if self.seq > 0xFFFFFFFF:
            self.seq = 0
        return seq

    def sendPacket(self, body):
        hdr = self.HDR_FMT.pack(self.newSeq(), 0, len(body))
        pkt = hdr + body
        self.logger.debug("send data: %r", pkt)
        self.socket.sendall(pkt)

    def recvPacket(self):
        self.in_buf = ""
        while len(self.in_buf) < self.HDR_FMT.size:
            data = self.socket.recv(0xFFFF)
            if not data:
                raise AsyncTcpError
            self.logger.debug("recv data: %r", data)
            self.in_buf += data

        seq, result, length = self.HDR_FMT.unpack(self.in_buf[:self.HDR_FMT.size])
        
        self.in_buf = self.in_buf[self.HDR_FMT.size:]
        while len(self.in_buf) < length:
            data = self.socket.recv(0xFFFF)
            if not data:
                raise AsyncTcpError
            self.logger.debug("recv data: %r", data)
            self.in_buf += data

        body = self.in_buf[:length]
        self.in_buf = self.in_buf[length:]
        return body


############################################################################
class GFQConnection(object):
    REQ_HDR_FMT = Struct("!i%(MAX_NAME_LEN)ds"%globals())

    RSP_HDR_FMT = Struct("!ii")

    logger = logging.getLogger("gfq_protocol")

    def __init__(self, addr):
        self.atp = AsyncTcpConnection(addr)
        
    def sendPacket(self, cmd_id, username, body):
        self.logger.debug("send packet: %x, %s, %r", cmd_id, username, body)
        hdr = self.REQ_HDR_FMT.pack(cmd_id, username)
        self.atp.sendPacket(hdr + body)

    def recvPacket(self):
        rsp= self.atp.recvPacket()
        cmd_id, state = self.RSP_HDR_FMT.unpack(rsp[:self.RSP_HDR_FMT.size])
        body = rsp[self.RSP_HDR_FMT.size:]
        self.logger.debug("recv packet: %x, %d, %r", cmd_id, state, body)
        return cmd_id, state, body

    def invoke(self, req_fmt, rsp_fmt, cmd_id, username, *args):
        req = req_fmt.pack(*args)
        self.sendPacket(cmd_id, username, req)

        cmd2_id, state, rsp = self.recvPacket()
        if state:
            raise GFQProtocolError(state)

        return rsp_fmt.unpack(rsp[:rsp_fmt.size])

############################################################################
class SegmentConnection(GFQConnection):
    GET_REQ_FMT = Struct("!i%(MAX_NAME_LEN)ds%(MAX_INT_LEN)dsi"%globals())
    GET_RSP_FMT = Struct("!%(MAX_DATA_LEN)dsi"%globals())

    PUT_REQ_FMT = Struct("!i%(MAX_NAME_LEN)ds%(MAX_INT_LEN)ds%(MAX_DATA_LEN)dsi"%globals())
    PUT_RSP_FMT = Struct("!")

    logger = logging.getLogger("gfq_segment")

    def __init__(self, addr):
        GFQConnection.__init__(self, addr)

    def get(self, username, seg_id, q_name, q_id, delay=0):
        data, size = self.invoke(self.GET_REQ_FMT, self.GET_RSP_FMT, S_GET_REQ, username, seg_id, q_name, q_id, delay)
        return data[:size]

    def put(self, username, seg_id, q_name, q_id, data):
        return self.invoke(self.PUT_REQ_FMT, self.PUT_RSP_FMT, S_PUT_REQ, username, seg_id, q_name, q_id, data, len(data))

############################################################################

class MasterConnection(GFQConnection):
    SEG_REQ_FMT = Struct("!%(MAX_NAME_LEN)ds"%globals())
    SEG2_REQ_FMT = Struct("!%(MAX_NAME_LEN)dsiii"%globals())
    
    SEG_RSP_FMT = Struct("!ii%(MAX_INT_LEN)ds%(IP_ADDR_LEN)dsi"%globals())
    
    STAT_REQ_FMT = Struct("!%(MAX_NAME_LEN)ds"%globals())
    STAT_RSP_FMT = Struct("!ii"%globals())
    
    logger = logging.getLogger("gfq_master") 

    def __init__(self, addr):
        GFQConnection.__init__(self, addr)

    def getRSeg(self, username, q_name):
        return self.invoke(self.SEG_REQ_FMT, self.SEG_RSP_FMT, M_RSEG_REQ, username, q_name)

    def getRSeg2(self, username, q_name, seg_id, mod_id, state):
        return self.invoke(self.SEG2_REQ_FMT, self.SEG_RSP_FMT, M_RSEG2_REQ, username, q_name, seg_id, mod_id, state)

    def getWSeg(self, username, q_name):
        return self.invoke(self.SEG_REQ_FMT, self.SEG_RSP_FMT, M_WSEG_REQ, username, q_name)

    def getWSeg2(self, username, q_name, seg_id, mod_id, state):
        return self.invoke(self.SEG2_REQ_FMT, self.SEG_RSP_FMT, M_WSEG2_REQ, username, q_name, seg_id, mod_id, state)

    def getStat(self, username, q_name):
        return self.invoke(self.STAT_REQ_FMT, self.STAT_RSP_FMT, M_STAT_REQ, username, q_name)

############################################################################
class ConnectionPool(object):
    def __init__(self, cls, addr, max_conn=1):
        self.cls = cls
        self.addr = addr
        self.max_conn = max_conn
        self.cur_conn = 0
        self.cond = Condition()
        self.pool = []

    def acquire(self):
        self.cond.acquire()
        try:
            while self.cur_conn >= self.max_conn and self.pool == []:
                self.cond.wait()

            if self.pool:
                return self.pool.pop(0)
            else:
                new_obj = self.cls(self.addr)
                self.cur_conn += 1
                return new_obj
        finally:
            self.cond.release()

    def release(self, obj):
        self.cond.acquire()
        try:
            self.pool.append(obj) 
            self.cond.notify()
        finally:
            self.cond.release()

    def destroy(self, obj):
        self.cond.acquire()
        try:
            self.cur_conn -= 1
            self.cond.notify()
        finally:
            self.cond.release()

    def call(self, func, *args, **kw):
        conn = self.acquire()
        try:
            r = func(conn, *args, **kw)
        except GFQProtocolError:
            self.release(conn)
            raise
        except:
            self.destroy(conn)
            raise

        self.release(conn)
        return r

############################################################################
class GFQClient(object):
    MAX_CONN_OF_MASTER = 1
    MAX_CONN_PER_SEGMENT = 5

    def __init__(self, addr, username):
        self.username = username
        self.m_pool = ConnectionPool(MasterConnection, addr, self.MAX_CONN_OF_MASTER)
        self.s_pools = {}
        self.rseg_cache = {}
        self.wseg_cache = {}

    def _getSegmentPool(self, addr):
        try:
            return self.s_pools[addr]
        except:
            return self.s_pools.setdefault(addr, ConnectionPool(SegmentConnection, addr, self.MAX_CONN_PER_SEGMENT)) 

    def _do2Pharse(self, cache, m_func, m_func2, s_func, q_name, *args):
        try:
            seg_id, mod_id, q_id, ip, port = cache[q_name]
        except:
            seg_id, mod_id, q_id, ip, port = cache[q_name] = self.m_pool.call(m_func, self.username, q_name) 

        addr = (ip.rstrip("\x00"), port)

        s_pool = self._getSegmentPool(addr)
        try:
            return s_pool.call(s_func, self.username, seg_id, q_name, q_id, *args) 
        except GFQProtocolError, e:
            errcode = e.state

        seg_id, mod_id, q_id, ip, port = cache[q_name] = self.m_pool.call(
            m_func2, 
            self.username, 
            q_name, 
            seg_id, 
            mod_id, 
            errcode)

        addr = (ip.rstrip("\x00"), port)
        s_pool = self._getSegmentPool(addr)
        return s_pool.call(s_func, self.username, seg_id, q_name, q_id, *args)
            
    def get(self, q_name, delay=0):
        return self._do2Pharse(
            self.rseg_cache, 
            MasterConnection.getRSeg, 
            MasterConnection.getRSeg2, 
            SegmentConnection.get, 
            q_name, delay)

    def put(self, q_name, data):
        return self._do2Pharse(
            self.wseg_cache, 
            MasterConnection.getWSeg, 
            MasterConnection.getWSeg2, 
            SegmentConnection.put, 
            q_name, data)
 
    def stat(self, q_name):
        return self.m_pool.call(MasterConnection.getStat, self.username, q_name)

############################################################################
