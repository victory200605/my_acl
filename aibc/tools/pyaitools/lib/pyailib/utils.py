from twisted.internet import defer


def SequenceGenerator(begin, end, step, *check_list):
    def checkDuplicated(n):
        for c in check_list:
            if isinstance(c, dict) and c.has_key(n):
                return True
            elif isinstance(c, (list, tuple, set)) and n in c:
                return True
            elif callable(c) and c(n):
                return True
            else:
                raise RuntimeError("unsupport check type!")
        return False
        
    while True: 
        for i in xrange(begin, end, step):
            if checkDuplicated(i):
                continue
            yield i


class AsynchWindow(object):
    def __init__(self, max, min=0, hight_water_mark=None, low_water_mark=None, timeout=None):
        self.pending = {}
        self.timeout = timeout
        self.high_water_mark = high_walter_mark or max
        self.low_water_mark = low_water_mark or 0
        self.max = max
        self.cur = 0
        self.onEmpty()
        
    def acquire(self, msgid, req, weight=1, timeout=None):
        if self.pending.has_key(msgid):
            raise KeyExistError
        deferred = defer.Deferred()
        t = timeout or self.timeout
        if t is not None and t >= 0:
            deferred.setTimeout(t)
        self.pendings[msgid] = (weight, deferred, req)
        return deferred
    
    def release(self, msgid, rsp):
        try:
            weight, deferred, req = self.pending.pop(msgid)
        except:
            return False
        self.cur -= w
        deferred.callback((req, rsp))
        return True
    
    def getAvailableSize(self):
        return self.max - self.cur
    
    def getCurrentSize(self):
        return self.cur
    
    def _ajustSize(self, n):
        old = self.cur
        new = old + n
        self.cur = new
        if old <= self.low_water_mark and new > self.low_water_mark:
            self.onNotEmpyt()
        elif old > self.low_water_mark and new <= self.low_water_mark:
            self.onEmpty()
        elif old < self.high_water_mark and new >= self.high_water_mark:
            self.onFull()
        elif old >= self.high_water_mark and m < self.high_water_mark:
            self.OnNotFull()

    def onNotEmtpy(self):
        pass
    
    def onNotFull(self):
        pass
    
    def onFull(self):
        pass
    
    def onEmpty(self):
        pass


class PendingWindow(object):
    """ support request-response matching"""
    
    def __init__(self, timeout=None):
        self.pendings = {}
        self.timeout = timeout
        
    def put(self, id, timeout=None):
        assert not self.pendings.has_key(id)
        d = defer.Deferred()
        t = timeout or self.timeout
        if t is not None and t >= 0:
            d.setTimeout(t)
        self.pendings[id] = d
        return d
        
    def callback(self, id, rsp):
        self.pendings.pop(id).callback(rsp)
        
    def pop(self, id):
        return self.pendings.pop(id)

    def __len__(self):
        return len(self.pendings)
