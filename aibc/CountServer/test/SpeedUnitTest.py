#!/usr/bin/env python
import unittest
import time
import random
import logging, logging.config
from ConfigParser import SafeConfigParser
from TokenBucket import TokenBucket
from CountClient import CountClient

logger = logging.getLogger()
############################################################################
CONF_FILE = "countserver_unittest.ini"

ini = SafeConfigParser()
ini.read([CONF_FILE])

TEST_ADDR = (ini.get("CountServer", "Address"), ini.getint("CountServer", "Port"))
TEST_SPEEDS = [(opt, ini.getint("SpeedLimits", opt))  for opt in ini.options("SpeedLimits")]

TEST_PERIOD = ini.getfloat("CountServer", "SpeedTestPeriod")
TEST_FACTOR = ini.getfloat("CountServer", "SpeedTestFactor")
ASSERT_DELTA = ini.getfloat("CountServer", "SpeedAssertDelta")


############################################################################

class TestSpeedNicety(unittest.TestCase):
    def setUp(self):
        self.clients = []
        for i in xrange(3):
            client = CountClient(TEST_ADDR)
            client.connect()
            self.clients.append(client)


    def tearDown(self):
        for c in self.clients:
            c.close()


    def doTest(self, clients):
        for name, speed in TEST_SPEEDS:
            tb = TokenBucket(speed, speed * TEST_FACTOR, speed * TEST_FACTOR)
            time.sleep(max(TEST_FACTOR, 1.0) + 0.1)
            t = time.time() + TEST_PERIOD
            total = 0
            rc = 0
            lc = 0
            while time.time() < t:
                c = random.choice(self.clients)
                state, timestamp = c.reportSend(name, 1, 0)
                result = tb.getToken(1, timestamp)
                total += 1
                if state == 0:
                    rc += 1
                if result == 1:
                    lc += 1
            logger.info("finish test for %r, speed=%f", name, speed)

            rr = float(rc) / total
            logger.info("remote success rate: %f", rr)

            lr = float(lc) / total 
            logger.info("local success rate: %f", lr)

            rs = float(rc) / TEST_PERIOD
            logger.info("remote speed: %f", rs)

            ls = float(lc) / TEST_PERIOD 
            logger.info("local speed: %f", ls)

            sd =  1.0 - float(rs) / ls 
            logger.info("speed delta: %f", sd)

            rd =  1.0 - float(rr) / lr 
            logger.info("success rate delta: %f", rd)

            self.assert_(abs(sd) < ASSERT_DELTA, name);


    def testOneClient(self):
        self.doTest(self.clients[:1])


    def testTwoClient(self):
        self.doTest(self.clients[:2])


    def testThreeClient(self):
        self.doTest(self.clients[:3])


############################################################################

if __name__ == "__main__":
    logging.config.fileConfig(CONF_FILE)
    print "server address: %r"%(TEST_ADDR,)
    print "test using: %r"%(TEST_SPEEDS,)
    unittest.main()

