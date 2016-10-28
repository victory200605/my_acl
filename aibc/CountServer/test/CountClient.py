#!/usr/bin/env python
import socket
import struct
import logging


############################################################################

NAME_LEN = 64

HDR_FMT = "!iii"
HDR_LEN = struct.calcsize(HDR_FMT)

REQ_FMT = "!i64sii"
REQ_LEN = struct.calcsize(REQ_FMT)

RSP_FMT = "!iiq"
RSP_LEN = struct.calcsize(RSP_FMT)

CONN_REQ = 0x01
CONN_RSP = 0x11
SEND_REQ = 0x02
SEND_RSP = 0x12

############################################################################

class CountClient(object):
    def __init__(self, addr, timeout=5.0):
        self.addr = addr
        self.timeout = timeout
        self.sock = None
        self.msgid = 0
        self.rbuf = ""
        self.logger = logging.getLogger(self.__class__.__name__)
        self.logger.info("server address: %r", self.addr)


    def connect(self):
        assert(self.sock is None)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
        self.sock.settimeout(self.timeout)
        self.sock.connect(self.addr)
        self.logger.info("connected to server")


    def close(self):
        assert(isinstance(self.sock, socket.socket))
        self.sock.close()
        self.sock = None
        self.logger.info("connection close")


    def getMsgId(self):
        self.msgid += 1
        return self.msgid 
   

    def sendCommand(self, cmd, name, delta, last):
        msgid = self.getMsgId()
        result = 0
        name = name[:NAME_LEN].ljust(NAME_LEN, '\0')
        req = struct.pack(REQ_FMT, cmd, name, delta, last)
        hdr = struct.pack(HDR_FMT, msgid, result, len(req)) 
        pkt = hdr + req
        self.sock.sendall(pkt)
        self.logger.debug("send command: (0x%X, 0x%X, (0x%X, %r, %d, %d))", msgid, result, cmd, name, delta, last) 
        self.logger.debug("send data: %r", pkt)
        return msgid, result


    def recvCommand(self):
        while True:
            data = self.sock.recv(HDR_LEN + RSP_LEN)
            self.logger.debug("socket recv: %r", data)
            assert data
            self.rbuf += data
            if len(self.rbuf) >= HDR_LEN + RSP_LEN:
                hdr = self.rbuf[:HDR_LEN]
                msgid, result, pkt_len = struct.unpack(HDR_FMT, hdr)
                assert(pkt_len == RSP_LEN)
                rsp = self.rbuf[HDR_LEN:HDR_LEN + RSP_LEN]
                cmd, state, timestamp = struct.unpack(RSP_FMT, rsp)
                self.rbuf = self.rbuf[HDR_LEN + RSP_LEN:]
                self.logger.debug("recv data: %r", hdr+rsp)
                self.logger.debug("recv command: (0x%X, 0x%X, (0x%X, %d, %d))", msgid, result, cmd, state, timestamp)
                return msgid, result, cmd, state, timestamp

    
    def sendRequest(self, req_type, rsp_type, name, delta, last):
        req_msgid, req_result = self.sendCommand(req_type, name, delta, last)
        rsp_msgid, rsp_result, rsp_cmd, rsp_state, rsp_timestamp = self.recvCommand()

        assert(req_msgid == rsp_msgid)
        assert(req_result == 0)
        assert(rsp_result == 0)
        assert(rsp_cmd == rsp_type)

        return rsp_state, rsp_timestamp


    def reportConnection(self, name, delta, last_num):
        return self.sendRequest(CONN_REQ, CONN_RSP, name, delta, last_num)


    def reportSend(self, name, delta, last_num=0):
        return self.sendRequest(SEND_REQ, SEND_RSP, name, delta, last_num)


############################################################################

