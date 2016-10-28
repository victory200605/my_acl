# -*- coding: UTF-8 -*-
import time

def createCycledSequenceGenerator(minValue, maxValue):
    assert(minValue <= maxValue)
    curValue = minValue
    while True:
        if curValue > maxValue:
            curValue = minValue
        yield curValue
        curValue += 1


def createTimedResetSequenceGenerator(minValue, maxValue, resetInterval=1):
    assert(minValue <= maxValue)
    curValue = minValue
    curTime = int(time.time())
    while True:
        now = int(time.time())
        if now > curTime:
            curValue = minValue
            curTime = now
        if curValue > maxValue:
            curValue = minValue
        yield curTime, curValue
        curValue += 1
