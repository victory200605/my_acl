# -*-  coding: UTF-8  -*-
try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO
    
import struct


class CDRError(Exception): pass
class BufferError(CDRError): pass
class InvalidLength(CDRError): pass
class InvalidData(CDRError): pass


BIG_ENDIAN = ">"
LITTLE_ENDIAN = "<"
NETWORK_ENDIAN = "!"
NATIVE_ENDIAN = "="

def BYTES(name, *argv, **kwargs):
    """define a bytes field"""
    return name, "Bytes", argv, kwargs

def INT8(name, *argv, **kwargs):
    """define a 8 bits signed int field"""
    return name, "Int8", argv, kwargs

def INT16(name, *argv, **kwargs):
    """define a 16 bits signed int field"""
    return name, "Int16", argv, kwargs

def INT32(name, *argv, **kwargs):
    """define a 32 bits signed int field"""
    return name, "Int32", argv, kwargs

def INT64(name, *argv, **kwargs):
    """define a 64 bits signed int field"""
    return name, "Int64", argv, kwargs

def INT(name, bits, *argv, **kwargs):
    """define a signed int field, in 8 bits/16 bits/32 bits/64 bits"""
    return name, "Int%d" % bits, argv, kwargs

def UINT8(name, *argv, **kwargs):
    """define a 8 bits unsigned int field"""
    return name, "UInt8", argv, kwargs

def UINT16(name, *argv, **kwargs):
    """define a 16 bits unsigned int field"""
    return name, "UInt16", argv, kwargs

def UINT32(name, *argv, **kwargs):
    """define a 32 bits unsigned int field"""
    return name, "UInt32", argv, kwargs

def UINT64(name, *argv, **kwargs):
    """define a 64 bits unsigned int field"""
    return name, "UInt64", argv, kwargs

def UINT(name, bits, *argv, **kwargs):
    """define a unsigned int field, in 8 bits/16 bits/32 bits/64 bits"""
    return name, "UInt%d" % bits, argv, kwargs

def FLOAT32(name, *argv, **kwargs):
    """define a 32 bits IEEE float point field"""
    return name, "Float32", argv, kwargs

def FLOAT64(name, *argv, **kwargs):
    """define a 64 bits IEEE float point field"""
    return name, "Float64", argv, kwargs

def FLOAT(name, bits, *argv, **kwargs):
    """define a IEEE float point field, in 32 bits/64 bits"""
    return name, "Float%d" % bits, argv, kwargs

def STR(name, *argv, **kwargs):
    """define a string field, see CDRReader.readStr()/CDRWrite.writeStr() for more details"""
    return name, "Str", argv, kwargs

def CSTR(name, *argv, **kwargs):
    """define a c-string field, see CDRReader.readCStr()/CDRWrite.writeCStr() for more details"""
    return name, "CStr", argv, kwargs

def DICT(name, syntax, *argv, **kwargs):
    """define a dict field"""
    return name, dict(), [syntax] + list(argv), kwargs    

def LIST(name, syntax, *argv, **kwargs):
    """define a list field"""
    return name, list(), [syntax] + list(argv), kwargs
    
def LIST_OF(name, length, syntax, *argv, **kwargs):
    """define a list of something(usually a dict) (well, it just looks like [{...}, ...])""" 
    def _YIELD_N():
        for i in xrange(length):
            yield DICT(i, syntax, *argv, **kwargs)    
    return name, list(), [_YIELD_N], {}

def NULL():
    raise StopIteration


class CDRBase(object):
    def __init__(self, stream, endian=""):
        self._stream = stream
        self._endian = endian
    
    def setEndian(self, endian):
        self._endian = endian
        
    def getEndian(self):
        return self._endian

    def getStream(self):
        return self._stream


class CDRReader(CDRBase):    
    def readBytes(self, length):
        data = self.getStream().read(length)
        if len(data) != length:
            raise BufferError
        return data
 
    def unpack(self, fmt):
        size = struct.calcsize(fmt)
        data = self.readBytes(size)
        return struct.unpack(self.getEndian() + fmt, data)
    
    def readInt8(self):
        return self.unpack("b")[0]
    
    def readInt16(self):
        return self.unpack("h")[0]
    
    def readInt32(self):
        return self.unpack("i")[0]
    
    def readInt64(self):
        return self.unpack("q")[0]
    
    def readUInt8(self):
        return self.unpack("B")[0]
    
    def readUInt16(self):
        return self.unpack("H")[0]
    
    def readUInt32(self):
        return self.unpack("I")[0]
    
    def readUInt64(self):
        return self.unpack("Q")[0] 

    def readFloat32(self):
        return self.unpack("f")[0]
    
    def readFloat64(self):
        return self.unpack("d")[0]
    
    def readStr(self, length, lpadding=None, rpadding=None):
        s = self.readBytes(length)
        if lpadding is not None:
            s.lstrip(lpadding)
        if rpadding is not None:
            s.rstrip(rpadding)
        return s

    def readCStr(self, maxLength=None, *limiteds):
        if maxLength:
            assert(maxLength > 0)
            data = self.readBytes(maxLength)
            strLen = data.find("\0")
            if strLen < 0:
                raise InvalidLength
            s = data[:strLen]
            self._stream.seek(strLen + 1 - len(data) , 1)
            return s
        else:
            assert(limiteds)
            lengthLimits.sort()
            s = ""
            for l in lengthLimits:
                s = s + self.readBytes(l - len(s))
                if s.find("\0") == len(s) - 1:
                    return s
            #self._stream.seek(- len(s), 1)
            raise InvalidLength                     
        
    
class CDRWriter(CDRBase):
    def writeBytes(self, data, length=None):
        if length is None:
            length = len(data)
        if len(data) != length:
            raise InvalidLength
        wrLen = self._stream.write(data)
        if isinstance(wrLen, (int, long)) and wrLen != len(data):
            raise BufferError
        return wrLen
    
    def pack(self, fmt, *args):
        data = struct.pack(self._endian + fmt, *args)
        return self.writeBytes(data)
    
    def writeInt8(self, i):
        return self.pack("b", i)
    
    def writeInt16(self, i):
        return self.pack("h", i)
    
    def writeInt32(self, i):
        return self.pack("i", i)
    
    def writeInt64(self, i):
        return self.pack("q", i)
    
    def writeUInt8(self, u):
        return self.pack("B", u)
    
    def writeUInt16(self, u):
        return self.pack("H", u)
    
    def writeUInt32(self, u):
        return self.pack("I", u)
    
    def writeUInt64(self, u):
        return self.pack("Q", u)   
    
    def writeFloat32(self, f):
        return self.pack("f", f)
    
    def writeFloat64(self, f):
        return self.pack("d", f)
    
    def writeStr(self, s, length=None, lpadding=None, rpadding=None):
        if length is None:
            length = len(s)
        if length != len(s):
            raise InvalidLength
        if lpadding:
            s = s.ljust(length, lpadding)
        if rpadding:
            s = s.rjust(length, rpadding)
        return self.writeBytes(s)

    def writeCStr(self, s, maxLength=None):
        if s.find("\0") >= 0:
            raise InvalidData
        if maxLength is None:
            maxLength = len(s) + 1
        if len(s) + 1 > maxLength:
            raise InvalidLength
        return self.writeBytes(s + "\0")


class CDRWriteHandler(CDRWriter):
    def __init__(self, obj, stream, endian):
        CDRWriter.__init__(self, stream, endian)
        self._stack = [obj]
        
    def handleParam(self, name, tag, *argv, **kwargs):
        fn = "write" + tag
        func = getattr(self, fn)
        value = self._stack[-1][name]
        func(value, *argv, **kwargs)
        return value
        
    def _beginX(self, name, x):
        obj = self._stack[-1][name]
        assert(isinstance(obj, x))
        self._stack.append(obj)
   
    def _endX(self, name, x):
        obj = self._stack.pop()
        assert(isinstance(obj, x))
        return obj


class CDRReadHandler(CDRReader):
    def __init__(self, obj, stream, endian):
        CDRReader.__init__(self, stream, endian)
        self._stack = [obj]
       
    def handleParam(self, name, tag, *argv, **kwargs):
        fn = "read" + tag
        func = getattr(self, fn)
        value = func(*argv, **kwargs)
        if isinstance(name, str):
            self._stack[-1][name] = value
        else:
            self._stack[-1].append(value)
        return value
        
    def _beginX(self, name, x):
        self._stack.append(x())
   
    def _endX(self, name, x):
        obj = self._stack.pop()
        if isinstance(self._stack[-1], list):
            self._stack[-1].append(obj)
        else:
            self._stack[-1][name] = obj
        assert(isinstance(obj, x))
        return obj


def walk(handler, syntax, *argv, **kwargs):
    try:
        itr = syntax(*argv, **kwargs)
        name, tag, argv, kwargs = itr.next()
        while True:
            if isinstance(tag, str):
                value = handler.handleParam(name, tag, *argv, **kwargs)
            elif isinstance(tag, (list, dict)):
                handler._beginX(name, type(tag))
                walk(handler, *argv, **kwargs)
                value = handler._endX(name, type(tag))
            else:
                raise RuntimeError
            name, tag, argv, kwargs = itr.send(value)
    except StopIteration:
        pass


def toString(obj, endian, syntax, *argv, **kwargs):
    sio = StringIO()
    wh = CDRWriteHandler(obj, sio, endian)
    walk(wh, syntax, *argv, **kwargs)
    return wh.getStream().getvalue()


def fromString(s, endian, syntax, *argv, **kwargs):
    obj = {}
    sio = StringIO(s)
    rh = CDRReadHandler(obj, sio, endian)
    walk(rh, syntax, *argv, **kwargs)
    return obj

   