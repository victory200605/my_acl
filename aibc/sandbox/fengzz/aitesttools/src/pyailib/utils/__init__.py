# -*- coding: UTF-8 -*-

def formatHexText(data):
    return "\n".join(["%08X: "%i +" ".join(["%02X"%ord(b) for b in data[i:i+16]]) for i in xrange(0, len(data), 16)])


def formatHexStr(data):
    return " ".join(["%02X"%ord(b) for b in data])


def importModule(moduleName):
    mod = __import__(moduleName)
    components = moduleName.split('.')
    for comp in components[1:]:
        mod = getattr(mod, comp)
    return mod
