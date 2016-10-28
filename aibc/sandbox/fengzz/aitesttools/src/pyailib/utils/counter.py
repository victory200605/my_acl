# -*- coding: UTF-8 -*-
import time

class SimpleCounter(object):
    def __init__(self, initValue=0):
        self.initValue = initValue
        self.curValue = initValue
        
    def inc(self, incValue = 1):
        self.curValue += incValue

    def reset(self):
        self.curValue = self.initValue
        
    def getValue(self):
        return self.curValue


class TimedCounter(SimpleCounter):
    def __init__(self, period=1, *argv, **kwargs):
        self.period = period
        self.counterTime = None
        super(TimedCounter, self).__init__(*argv, **kwargs)
        
    def inc(self, incValue=1, curTime=time.time):
        super(TimedCounter, self).inc(incValue)
        if callable(curTime):
            curTime = curTime()
        if self.counterTime is None:
            self.counterTime = curTime
        if (self.counterTime + self.period) < curTime:
            t = self.counterTime
            c = self.getValue()
            self.counterTime += self.period
            self.reset()
            return t, c
