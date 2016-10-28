# -*- coding: UTF-8 -*-

def _everyBitsOfInt(i):
    b = 1
    while i:
        yield i & b
        i &= ~b
        b <<= 1
        
class SimpleSessionManager(object):
    def __init__(self, maskBits):
        self._sessions = {}
        self._maskDict = {}
        self._maskBits = maskBits
        for b in _everyBitsOfInt(maskBits):
            self._maskDict[v] = []
            
    def register(self, session, bitMask=1):
        assert(not self._sessions.has_key(session))
        availableBits = bitMask & self._maskBits
        assert(availableBits != 0)
        for b in _everyBitsOfInt(availableBits):
            self._maskDict[b].append(session)
        self._sessions[session] = availableBits
            
    def unregister(self, session):
        availableBits = self._sessions.pop(session)
        for b in _everyBitsOfInt(availableBits):
            try:
                self._maskDict[b].remove(session)
            except:
                pass
    
    def acquire(self, bit=1):
        bl = list(_everyBitsOfInt(bit))
        assert(len(bl) == 1)
        return self._maskDict[bit].pop(0)
    
    def release(self, session, bit=1):
        assert(self._sessions[session] & bit)
        assert(session not in self._maskDict[bit])
        self._maskDict[bit].append(session)
        
