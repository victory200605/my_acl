#!/usr/bin/env python
import unittest
import sys
import logging, logging.config
from ConfigParser import SafeConfigParser
from CountClient import CountClient

############################################################################
CONF_FILE = "countserver_unittest.ini"

ini = SafeConfigParser()
ini.read([CONF_FILE])

TEST_ADDR = (ini.get("CountServer", "Address"), ini.getint("CountServer", "Port"))
TEST_CONNS = [(opt, ini.getint("ConnectionLimits", opt))  for opt in ini.options("ConnectionLimits")]

############################################################################

class TestOneClient(unittest.TestCase):
    """ test connection control, one client """

    def setUp(self):
        self.client = CountClient(TEST_ADDR)
        self.client.connect()


    def tearDown(self):
        self.client.close()


    def testOneClient(self):
        for name, max_conn in TEST_CONNS:
            state, timestamp = self.client.reportConnection(name, 0, 0)
            self.assertEqual(state, 0, name)
            # 0
            state, timestamp = self.client.reportConnection(name, max_conn + 1, 0)
            self.assertNotEqual(state, 0, name)
            # 0
            state, timestamp = self.client.reportConnection(name, max_conn, 0)
            self.assertEqual(state, 0, name)
            # max_conn
            state, timestamp = self.client.reportConnection(name, -max_conn, max_conn)
            self.assertEqual(state, 0, name)
            # 0
            state, timestamp = self.client.reportConnection(name, -1, 0)
            self.assertNotEqual(state, 0, name)
            # 0
            state, timestamp = self.client.reportConnection(name, 0, max_conn + 1)
            self.assertEqual(state, 0, name)
            # max_conn + 1
            state, timestamp = self.client.reportConnection(name, 1, max_conn + 1)
            self.assertNotEqual(state, 0, name)
            # max_conn + 1
            state, timestamp = self.client.reportConnection(name, -(max_conn + 1), max_conn + 1)
            self.assertEqual(state, 0, name)
            # 0


############################################################################

class TestOneClientReconnect(unittest.TestCase):
    """ test connection control test, one clients, reconnect """

    def setUp(self):
        self.client = CountClient(TEST_ADDR)
        self.client.connect()

    def tearDown(self):
        self.client.close()

    def testOneClientReconnect(self):
        for name, max_conn in TEST_CONNS:
            state, timestamp = self.client.reportConnection(name, 0, 0)
            self.assertEqual(state, 0, name)
            # 0
            state, timestamp = self.client.reportConnection(name, max_conn, 0)
            self.assertEqual(state, 0, name)
            # max_conn
            self.client.close()
            self.client.connect()
            # 0
            state, timestamp = self.client.reportConnection(name, 0, max_conn + 1)
            self.assertEqual(state, 0, name)
            # max_conn + 1
            state, timestamp = self.client.reportConnection(name, 1, max_conn + 1)
            self.assertNotEqual(state, 0, name)
            # max_conn + 1
            state, timestamp = self.client.reportConnection(name, -(max_conn + 1), max_conn + 1)
            self.assertEqual(state, 0, name)
            # 0


############################################################################

class TestTwoClient(unittest.TestCase): 
    def setUp(self):
        self.client1 = CountClient(TEST_ADDR)
        self.client1.connect()
        self.client2 = CountClient(TEST_ADDR)
        self.client2.connect()


    def tearDown(self):
        self.client1.close()
        self.client2.close()


    def testTwoClient(self):
        for name, max_conn in TEST_CONNS:
            state, timestamp = self.client1.reportConnection(name, 0, 0)
            self.assertEqual(state, 0, name)
            # 0
            state, timestamp = self.client1.reportConnection(name, 0, 0)
            self.assertEqual(state, 0, name)
            # 0
            state, timestamp = self.client1.reportConnection(name, max_conn, 0)
            self.assertEqual(state, 0, name)
            # max_conn
            state, timestamp = self.client2.reportConnection(name, max_conn, 0)
            self.assertNotEqual(state, 0, name)
            # max_conn
            state, timestamp = self.client1.reportConnection(name, -max_conn, max_conn)
            self.assertEqual(state, 0, name)
            # 0
            state, timestamp = self.client2.reportConnection(name, max_conn, 0)
            self.assertEqual(state, 0, name)
            # max_conn
            state, timestamp = self.client1.reportConnection(name, 1, 0)
            self.assertNotEqual(state, 0, name)
            # max_conn
            state, timestamp = self.client2.reportConnection(name, -max_conn, max_conn)
            self.assertEqual(state, 0, name)
            # 0
        

############################################################################

if __name__ == "__main__":
    logging.config.fileConfig(CONF_FILE)
    print "server address: %r"%(TEST_ADDR,)
    print "test using: %r"%(TEST_CONNS,)
    unittest.main()

