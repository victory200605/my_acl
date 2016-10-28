import unittest
from twisted.internet import reactor, defer
from pyailib.utils import PendingWindow


class TestPendingWindow(unittest.TestCase):
    def testCallback(self):
        rrm = PendingWindow()
        key = 1
        data = "good!"
        d = rrm.put(key)
        send = "good!"
        
        def _cb(r, want):
            self.assertEqual(r, want)
            return r
        
        d.addCallback(_cb, want=data)
        rrm.callback(key, data)

    def testCancelPendingRequest(self):
        rrm = PendingWindow()
        key = 10
        d = rrm.put(key)
        rrm.pop(key)
        self.assertRaises(Exception, rrm.callback, key, None)
        
    def testNotTimeoutRequest(self):
        rrm = PendingWindow()
        key = 30
        t = 2.0
        msg = "abcdefg"
        d = rrm.put(key, t)
        
        def _stop(a=None):
            try:
                reactor.stop()
            except:
                pass
            return a
        
        d.addCallbacks(_stop, _stop)
        rrm.callback(key, msg)

        self.assertEqual(msg, d.result)


    def testTimeoutRequest(self):
        rrm = PendingWindow()
        key = 20
        t = 2.0
        msg = "abcdefg"
        d = rrm.put(key, t)
        
        def _stop(a=None):
            reactor.stop()
            return a
        
        d.addCallbacks(_stop, _stop)

        reactor.callLater(t + 1, rrm.callback, key, msg)
        reactor.run()    
        
        self.assertNotEqual(msg, d.result)
        self.assertRaises(defer.TimeoutError, d.result.raiseException)
        

