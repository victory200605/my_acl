# -*- coding: UTF-8 -*-

class Timer(object):
    def __init__(self, timeout, func, *argv, **kwargs):
        self.timeout = timeout
        self.func = func
        self.argv = argv
        self.kwargs = kwargs
        self.timeoutCall = None
        
    def schedule(self):
        self.timeoutCall = reactor.callLater(self.timeout, self.handleTimeout)
    
    def cancel(self):
        if self.timeoutCall is not None:
            self.timeoutCall.cancel()
            self.timeoutCall = None
    
    def reset(self):
        self.cancel()
        self.schedule()
        
    def setTimeout(self, timeout):
        self.timeout = timeout
        
    def setCallback(self, func, *argv, **kwargs):
        self.func = func
        self.argv = argv
        self.kwargs = kwargs
            
    def handleTimeout(self):
        self.func(*self.argv, **self.kwargs)
