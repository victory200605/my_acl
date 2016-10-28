import unittest
from pyailib.xmldict import *

xml_1 = """<tag_1>
    <tag_2 attr_2_1="abcdefg" attr_2_2="1234567">
    </tag_2>
</tag_1>"""

dict_1 = {
    "/tag_1/tag_2#attr_2_1": "abcdefg",
    "/tag_1/tag_2#attr_2_2": "1234567",
}

cases = [(xml_1, dict_1)]

class TestXmlDict(unittest.TestCase):
    def testXmlDict(self):
        for x, d in cases:
            d2 = xmlToDictAttrs(x)
            self.assertEqual(d, d2)
            
            x2 = dictToXmlAttrs(d)
            d3 = xmlToDictAttrs(x2)
            self.assertEquals(d, d3)

    
 