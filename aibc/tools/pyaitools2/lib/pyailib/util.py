from twisted.internet import task, defer
from random import choice
import string



############################################################################
def getByID(items, key, *defaults):
    for i in items:
        if id(i) == key:
            return i
    if defaults:
        return defaults[0]
    else:
        raise KeyError
    
    
############################################################################
def SequenceGenerator(*argv):
    while True:
        for i in xrange(*argv):
            yield i

############################################################################
class AsynchWindow(object):
    def __init__(self, hwm, def_size=1, timeout=None):
        self.hwm = hwm
        self.def_size = def_size
        self.timeout = timeout
        self.cur_size = 0
        self.map = {}
    
    def getSize(self):
        return self.cur_size
    
    def isFull(self):
        return self.cur_size >= self.hwm
    
    def isEmpty(self):
        return self.cur_size == 0
    
    def push(self, key, size=None, timeout=None):
        if self.isFull():
            raise MemoryError("window if full")
        
        if key in self.map:
            raise KeyError("duplicated key")
        
        size = self.def_size if size is None else size
        timeout = self.timeout if timeout is None else timeout
        d = defer.Deferred()
        self.map[key] = (d, size)
        self.cur_size += size
        
        if timeout is not None:
            d.setTimeout(timeout)

        d.addErrback(lambda f: self.pop(key))
        return d
    
    def pop(self, key):
        d, size = self.map.pop(key)
        self.cur_size -= size     
        assert size in (1, 0)
        return d
    
    def callback(self, key, result):
        d = self.pop(key)
        d.callback(result)
        
    def errback(self, key, f):
        d = self.pop(key)
        d.errback(f)

    def purge(self, reason):
        for key, (d, size) in self.map.items():
            self.cur_size -= size
            d.errback(reason)

############################################################################
def dumpBinary(s, l=16, sep="\n"):
    def dumpHex(s, step=4):
        out = []
        
        for i in xrange(0, len(s), step):
            out.append(" ".join(["%02X" % ord(c) for c in s[i:i + step]]))
        return "-".join(out)
    
    def dumpAscii(s, padding="."):
        out = []
        
        for c in s:
            if c in string.letters or c in string.digits or c in string.punctuation:
                out.append(c)
            else:
                out.append(padding)
        
        return "".join(out)
    
    lines = []
    
    for i in xrange(0, len(s), l):
        f = s[i:i + l]
        h = dumpHex(f)
        a = dumpAscii(f)
        lines.append("%08x: %s %s" % (i, h.ljust(l * 3), a.ljust(l)))
        
    return sep.join(lines)


############################################################################
class RandomWeightedGenerator(object):
    def __init__(self, type=str, unit=0.01):
        self.range = []
        self.type = type
        self.unit = unit
    
    def addMulti(self, data):
        for k, v in data:
            self.add(k, v)
            
    def add(self, what, weight):
        for i in xrange(int(float(weight)/self.unit)):
            self.range.append(self.type(what))
    
    def next(self):
        return choice(self.range) if self.range else self.type() 

############################################################################
class ProtocolManager(object):
    def __init__(self):
        self.tbl = {}
        self.pl = []
    
    def __iter__(self):
        return iter(self.pl)
    
    def register(self, name, proto, *types):
        protos = self.tbl.setdefault(name, {})
        for t in types:
            protos.setdefault(t, []).append(proto)
        self.pl.append(proto)
        
    def unregister(self, name, proto):
        for t, pl in self.tbl.get(name, {}).items():
            pl.remove(proto)
        self.pl.remove(proto)
        
    def get(self, name, type, choice_func=choice):
        pl = self.getAll(name, type)
        return choice_func(pl) if pl else None

    def getAll(self, name, type):
        return self.tbl.get(name, {}).get(type, [])
    
    def getByID(self, pid):
        return getByID(self.pl, pid, None)

############################################################################
class LoopingTask(object):
    def __init__(self, mgr):
        self.mgr = mgr
        self.looping_call = task.LoopingCall(self.run)
        
    def start(self, interval):
        self.looping_call.start(interval)
        if isinstance(self.mgr, list):
            self.mgr.append(self)
        elif isinstance(self.mgr, set):
            self.mgr.add(self)
        else:
            assert False
        self.interval = interval
        print "LoopingTask(%d) START!"%id(self)
        
    def stop(self):
        self.looping_call.stop()
        self.mgr.remove(self)
        print "LoopingTask(%d) STOP!"%id(self)
        
    def run(self):
        pass




                    
                
        

