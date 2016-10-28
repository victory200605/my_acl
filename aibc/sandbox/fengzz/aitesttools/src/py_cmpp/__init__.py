import struct
import logging

# for md5 hash
try:
    from hashlib import md5
except:
    from md5 import md5

# for StringIO
try:
    from cStringIO import StringIO
except:
    from StringIO import StringIO

_PACKET = logging.DEBUG - 1

HEADER_LENGTH = 3 * 4


CONNECT                     = 0x00000001
CONNECT_RESP                = 0x80000001

TERMINATE                   = 0x00000002
TERMINATE_RESP              = 0x80000002

SUBMIT                      = 0x00000004
SUBMIT_RESP                 = 0x80000004

DELIVER                     = 0x00000005
DELIVER_RESP                = 0x80000005

QUERY                       = 0x00000006
QUERY_RESP                  = 0x80000006

CANCEL                      = 0x00000007
CANCEL_RESP                 = 0x80000007

ACTIVE_TEST                 = 0x00000008
ACTIVE_TEST_RESP            = 0x80000008

FWD                         = 0x00000009
FWD_RESP                    = 0x80000009

MT_ROUTE                    = 0x00000010
MT_ROUTE_RESP               = 0x80000010

MO_ROUTE                    = 0x00000011
MO_ROUTE_RESP               = 0x80000011

GET_MT_ROUTE                = 0x00000012
GET_MT_ROUTE_RESP           = 0x80000012

MT_ROUTE_UPDATE             = 0x00000013
MT_ROUTE_UPDATE_RESP        = 0x80000013

MO_ROUTE_UPDATE             = 0x00000014
MO_ROUTE_UPDATE_RESP        = 0x80000014

PUSH_MT_ROUTE_UPDATE        = 0x00000015
PUSH_MT_ROUTE_UPDATE_RESP   = 0x80000015

PUSH_MO_ROUTE_UPDATE        = 0x00000016
PUSH_MO_ROUTE_UPDATE_RESP   = 0x80000016

GET_MO_ROUTE                = 0x00000012
GET_MO_ROUTE_RESP           = 0x80000012



logger = logging.getLogger("py_cmpp")


def _format_hex_text(data):
    return "\n".join(["%08X: "%i +" ".join(["%02X"%ord(b) for b in data[i:i+16]]) for i in xrange(0, len(data), 16)])

def _format_hex_str(data):
    return " ".join(["%02X"%ord(b) for b in data])


   
class CMPP_Object(object):
    def __init__(self):
        self.reset()


    def __str__(self):
        return "\n".join(["%s=%s"%(name, repr(getattr(self, name))) for name, fmt, def_type, rel_field in self.get_define_list()])


    def get_define_list(self):
        return self.__class__.DEFINE_LIST


    def reset(self):
        for name, fmt, def_type, rel_field in self.get_define_list():
            setattr(self, name, def_type())
  
   
    def _encode_value(self, name, fmt, buf):
        val = getattr(self, name)
        data = struct.pack(fmt, val)
        if logger.getEffectiveLevel() <= _PACKET:
            logger.debug("%s._encode_value(), %s: %s", self.__class__.__name__, name, _format_hex_str(data))
        buf.write(data) 
  
   
    def _encode_varstr(self, name, fmt, buf):
        val = getattr(self, name)
        fmt = "%us"%len(val)
        data = struct.pack(fmt, val)
        if logger.getEffectiveLevel() <= _PACKET:
            logger.debug("%s._encode_varstr(), %s: %s", self.__class__.__name__, name, _format_hex_str(data))
        buf.write(data) 

    
    def _encode_strlist(self, name, fmt, buf):
        val = getattr(self, name)
        fmt = fmt * len(val) 
        data = struct.pack(fmt, *val)
        if logger.getEffectiveLevel() <= _PACKET: 
            logger.debug("%s._encode_strlist(), %s: %s", self.__class__.__name__, name, _format_hex_str(data))
        buf.write(data) 


    def encode(self):
        # handle relative fields
        for name, fmt, def_type, rel_field in self.get_define_list():
            if rel_field:
                rel_len = len(getattr(self, name))
                setattr(self, rel_field, rel_len) # auto calc length
                logger.debug("auto set %s to %u", rel_field, rel_len)

        # to encode as string
        buf = StringIO()
        for name, fmt, def_type, rel_field in self.get_define_list():
            if def_type is list:
                self._encode_strlist(name, fmt, buf)

            elif def_type is str and rel_field:
                self._encode_varstr(name, fmt, buf)

            else:
                self._encode_value(name, fmt, buf)
        
        ret = buf.getvalue()
        if logger.getEffectiveLevel() <= _PACKET: 
            logger.debug("%s.encode():\n%s", self.__class__.__name__, _format_hex_text(ret))
        return ret


    def _decode_value(self, name, fmt, buf):
        size = struct.calcsize(fmt)
        data = buf.read(size)
        if logger.getEffectiveLevel() <= _PACKET: 
            logger.debug("%s._decode_value(), %s: %s", self.__class__.__name__, name, _format_hex_str(data))
        val, = struct.unpack(fmt, data)
        if isinstance(val, str):
            val = val.rstrip("\0")
        setattr(self, name, val)



    def _decode_varstr(self, name, rel_len, buf):
        fmt  = "%us"%rel_len 
        size = struct.calcsize(fmt)
        data = buf.read(size)
        if logger.getEffectiveLevel() <= _PACKET: 
            logger.debug("%s._decode_varstr(), %s: %s", self.__class__.__name__, name, _format_hex_str(data))
        val,  = struct.unpack(fmt, data)
        setattr(self, name, val)



    def _decode_strlist(self, name, fmt, rel_len, buf):
        fmt = fmt * rel_len
        size = struct.calcsize(fmt)
        data = buf.read(size)
        if logger.getEffectiveLevel() <= _PACKET: 
            logger.debug("%s._decode_strlist(), %s: %s", self.__class__.__name__, name, _format_hex_str(data))
        val  = struct.unpack(fmt, data)
        setattr(self, name, [v.rstrip("\0") for v in val])



    def decode(self, data):
        if logger.getEffectiveLevel() <= _PACKET: 
            logger.debug("%s.decode():\n%s", self.__class__.__name__, _format_hex_text(data))

        buf = StringIO(data)

        for name, fmt, def_type, rel_field in self.get_define_list():
            if rel_field:
                rel_len = getattr(self, rel_field)

                assert(isinstance(rel_len, (int, long)))
                
                if def_type is list:
                    self._decode_strlist(name, fmt, rel_len, buf)

                elif def_type is str:
                    self._decode_varstr(name, rel_len, buf)

                else:
                    # unsuported
                    assert(false)
            else:
                self._decode_value(name, fmt, buf)



class Header(CMPP_Object):
    DEFINE_LIST = (
        ("total_length",    "!L",    int,   None),
        ("command_id",      "!L",    int,   None),
        ("sequence_id",     "!L",    int,   None),
    )



class Message_ID(object):
    def __init__(self, msg_id=None):
        self.time = (0,0,0,0,0) 
        self.ismg_no = 0
        self.msg_seq = 0
        
        if msg_id:
            assert(isinstance(msg_id, (int, long)))
            self.decode(msg_id)


    def __str__(self):
        return "%02u%02u%02u%02u%02u:%u:%u"%(list(self.time) + [self.ismg_no, self.msg_seq])


    def encode(self):
        msg_id = 0
        msg_id |= (self.time[0] & 0xF) << 60
        msg_id |= (self.time[1] & 0x1F) << 55
        msg_id |= (self.time[2] & 0x1F) << 50
        msg_id |= (self.time[3] & 0x3F) << 44
        msg_id |= (self.time[4] & 0x3F) << 38
        msg_id |= (int(self.ismg_no) & 0x1FFFFF) << 16
        msg_id |= self.msg_seq & 0xFFFF

        return msg_id


    def decode(self, msg_id):
        assert(isinstance(msg_id, (int, long)))

        self.time = (
            (msg_id >> 60) & 0xF,
            (msg_id >> 55) & 0x1F,
            (msg_id >> 50) & 0x1F,
            (msg_id >> 44) & 0x3F,
            (msg_id >> 38) & 0x3F
        )
        self.ismg_no = (msg_id >> 16) & 0x1FFFFF
        self.msg_seq = msg_id & 0xFFFF


    def encode_bill_msg_id(self):
        return "%02u%02u%02u%02u%02u"%self.time + "%05u%05u"%(self.ismg_no%100000, self.msg_seq%0xFFFF)


    def decode_bill_msg_id(self, bill_msg_id):
        self.time = tuple([int(bill_msg_id[i*2:i*2+2]) for i in xrange(5)])
        self.ismg_no = int(bill_msg_id[10:15])
        self.msg_seq = int(bill_msg_id[15:20])



def calc_authenticator_source(source_addr, share_secret, timestamp):
    assert(isinstance(source_addr, str))
    assert(isinstance(share_secret, str))
    assert(isinstance(timestamp, (int, long)))
    ts = "%010u"%(timestamp % 10000000000)
    assert(len(ts) == 10)

    m = md5()
    m.update(source_addr)
    m.update('\0' * 9)
    m.update(share_secret)
    m.update(ts)

    return m.digest()



def calc_authenticator_ismg(status, authenticator_source, share_secret):
    m = md5()
    m.update("%u"%status)
    m.update(authenticator_source)
    m.update(share_secret)

    return m.digest()


def get_command_class(command_id, pkt_mod):
    for n in dir(pkt_mod):
        o = getattr(pkt_mod, n)
        if hasattr(o, "COMMAND_ID"):
            if command_id == getattr(o, "COMMAND_ID"):
                return o
    return None

