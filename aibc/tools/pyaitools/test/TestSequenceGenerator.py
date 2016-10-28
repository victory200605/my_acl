import unittest
from pyailib.utils import SequenceGenerator


class TestSequenceGenerator(unittest.TestCase):
    def testSequence(self):
        cases = [
            (0, 100, 1),
            (0, 100, 2),
            (0, 100, 3),
            (0, 100, 9),
            (100, 0, -1),
            (100, 0, -2),
        ]
        
        for min, max, step in cases:
            sg = SequenceGenerator(min, max, step)
            
            for i in xrange(min, max, step):
                self.assertEqual(i, sg.next())
