import unittest
import logging.config
from ConfigParser import SafeConfigParser
import gfq_api

CONFIG_FILE = "gfq_api_unittest.ini"

logging.config.fileConfig(CONFIG_FILE)
ini = SafeConfigParser()
ini.read([CONFIG_FILE])

class GFQApiTest(unittest.TestCase):

    def setUp(self):
        SECTION = "gfq_api_unittest"

        self.USERNAME = ini.get(SECTION, "username")
        self.IP = ini.get(SECTION, "ip")
        self.PORT = ini.getint(SECTION, "port")
        self.Q_NAME = ini.get(SECTION, "q_name")
        self.N = ini.getint(SECTION, "count")

    def testNormal(self):
        try:
            self.doTest()
        except gfq_api.GFQProtocolError, e:
            print e.state
            raise

    def _testHotshot(self):
        import hotshot, hotshot.stats
        PROF_FILE = "gfq_api_unittest.prof"
        prof = hotshot.Profile(PROF_FILE)
        prof.runcall(self.doTest)
        prof.close()
        stats = hotshot.stats.load(PROF_FILE)
        stats.strip_dirs()
        stats.sort_stats('calls', 'time')
        stats.print_stats(50)


    def doTest(self):
        gfqapi = gfq_api.GFQClient((self.IP, self.PORT), self.USERNAME)
       
        size, cap = gfqapi.stat(self.Q_NAME)

        for i in xrange(size):
            s = gfqapi.get(self.Q_NAME)

        size, cap = gfqapi.stat(self.Q_NAME)
        self.assertEqual(size, 0)

        for i in xrange(self.N):
            s = "test data %d" % i
            gfqapi.put(self.Q_NAME, s)

        size, cap = gfqapi.stat(self.Q_NAME)
        self.assertEqual(size, self.N)

        s = set()
        for i in xrange(self.N):
            d = gfqapi.get(self.Q_NAME)
            s.add(d)

        self.assertEqual(len(s), self.N)

        size, cap = gfqapi.stat(self.Q_NAME)
        self.assertEqual(size, 0)

unittest.main()

