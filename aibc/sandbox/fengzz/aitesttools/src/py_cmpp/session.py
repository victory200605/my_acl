import time
import logging

from py_cmpp import *

logger = logging.getLogger("py_cmpp.session")



class Session(object):
    def __init__(self, sock, cmpp_module):
        self.__pkt_mod = cmpp_module
        self.__sock = sock 
        self.__seq = 0


    def get_socket(self):
        return self.__sock


    def get_sequence(self):
        seq = self.__seq
        self.__seq = (self.__seq + 1) & 0x7FFFFFFF
        return seq


    def _recv_n(self, n):
        buf = []
        while n > 0:
            data = self.__sock.recv(n)
            assert(data)
            buf.append(data)
            n -= len(data)
        return "".join(buf)


    def send_packet(self, command_id, pdu, sequence_id=None):
        """return header that sent"""

        hdr = self.__pkt_mod.Header()
        hdr.total_length    = self.__pkt_mod.HEADER_LENGTH + len(pdu)
        hdr.command_id      = command_id

        if sequence_id is None:
            hdr.sequence_id = self.get_sequence()
        else:
            hdr.sequence_id = sequence_id

        pkt = hdr.encode() + pdu

        self.__sock.sendall(pkt)
        return hdr


    def recv_packet(self):
        """return header and pdu data"""

        hdr = Header()
        hdr.decode(self._recv_n(self.__pkt_mod.HEADER_LENGTH))
        pdu_len = hdr.total_length - self.__pkt_mod.HEADER_LENGTH

        assert(pdu_len >= 0)

        return hdr, pdu_len != 0 and self._recv_n(pdu_len) or ""


    def send_command(self, command, sequence_id=None):
        """return header"""
        return self.send_packet(command.COMMAND_ID, command.encode(), sequence_id=sequence_id)


    def recv_command(self):
        """return header and decoded command pdu"""
        hdr, pdu = self.recv_packet()
        cls = get_command_class(hdr.command_id, self.__pkt_mod)
        assert(cls)
        cmd = cls()
        cmd.decode(pdu)
        return hdr, cmd


    def login_gateway(self, username, password, version=None):
        conn = self.__pkt_mod.Connect()
        conn.source_addr            = username
        conn.version                = version or self.__pkt_mod.VERSION
        conn.timestamp              = int(time.strftime("%m%d%H%M%S"))
        conn.authenticator_source   = calc_authenticator_source(conn.source_addr, password, conn.timestamp)

        hdr_req         = self.send_command(conn)
        hdr_rsp, rsp    = self.recv_command()
        
        assert(hdr_req.sequence_id == hdr_rsp.sequence_id)
        assert(hdr_rsp.command_id == CONNECT_RESP)

        cai = calc_authenticator_ismg(rsp.status, conn.authenticator_source, password)

        assert(cai == rsp.authenticator_ismg)

        return rsp.status

