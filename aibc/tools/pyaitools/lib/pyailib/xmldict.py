try:
    from celementtree import ElementTree as ET
except:
    try:
        from xml.etree import ElementTree as ET
    except:
        from elementtree import ElementTree as ET


def xmlAttrsToDict(xml):
    et = ET.fromstring(xml)
    dic = {}
    def _toDict(prefix, node, dic):
        prefix = "%s/%s"%(prefix, node.tag)
        for k, v in node.items():
            dic["%s#%s"%(prefix, k)] = v
        for child in node.getchildren():
            _toDict(prefix, child, dic)
    _toDict("", et, dic)
    return dic


def dictToXmlAttrs(dic):
    et = ET.Element("root")
    def _add(et, p, a, v):
        for i in p:
            e = et.find(i)
            if e is None:
                et = ET.SubElement(et, i)
            else:
                et = e
        et.attrib[a] = v
        
    for k, v in dic.items():
        n, a = k.split("#")
        p = n.split("/")[1:]
        _add(et, p, a, str(v))
        
    return ET.tostring(et.getchildren()[0])


def dictToXmlAttrs2(dic):
    """ almost the same function of dictToXmlAttrs but faster!"""
    def _sameLevel(a, b):
        l = min(len(a), len(b))
        for i in xrange(l):
            if a[i] != b[i]:
                return i
        return l
    
    ll = dic.items()
    ll.sort()
    last = []
    x = []
    
    for k, v in ll:
        p, attr = k.split("#", 1)
        cur = p.split("/")
        d = _sameLevel(cur, last)
    
        if last and d != len(cur):
            x.append(">")
    
        for i in xrange(len(last) - d):
            x.append("</%s>"%last.pop())
    
        if d < len(cur):
            for i in cur[d:-1]:
                x.append("<%s>"%i)
            x.append("<%s"%cur[-1])
    
        x.append(' %s="%s"'%(attr, v))
        last = cur
    
    if last:
        x.append(">"),
    
    while last:
        x.append("</%s>"%last.pop())
    
    return "".join(x)