from twisted.internet import defer, reactor
from twisted.protocols import policies
from pyailib.exceptions import *




class TimeoutMixin(object):
    """Mixin for protocols which wish to timeout connections"""
    
    def __init__(self):
        self.timers = {} 

    def resetTimeout(self, name):
        """Reset the timeout count down"""
        if self.__timeoutCall is not None and self.timeOut is not None:
            self.__timeoutCall.reset(self.timeOut)

    def setTimeout(self, name, period, func=None):
        """Change the timeout period"""
        
        timer, _ = self.timers[name]

        if timer is not None:
            if period is None:
                timer.cancel()
                timer = None
            else:
                timer.reset(period)
        elif period is not None:
            timer = reactor.callLater(period, func or self.handleTimeout)

        return prev

    def __timedOut(self):
        self.__timeoutCall = None
        self.timeoutConnection()

    def timeoutConnection(self):
        """Called when the connection times out.
        Override to define behavior other than dropping the connection.
        """
        self.transport.loseConnection()


