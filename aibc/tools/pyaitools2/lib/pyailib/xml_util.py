
ETREE_LIST = (
    "xml.etree.cElementTree",
    "cElementTree",
    "lxml.etree",
    "elementtree.ElementTree",
    "xml.etree.ElementTree",
)

def my_import(name):
    mod = __import__(name)
    components = name.split('.')
    for comp in components[1:]:
        mod = getattr(mod, comp)
    return mod

for E in ETREE_LIST:
    try:
        locals()["etree"] = my_import(E)
        break;
    except:
        pass
    
