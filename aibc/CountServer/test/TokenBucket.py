import time

SECOND = 1000000000

class TokenBucket(object):
    def __init__(self, rate, max_size=None, init=0):
        self._rate = rate
        if max_size:
            self._max_size = max_size
        else:
            self._max_size = rate
        assert (self._max_size >= 0)
        self._cur_size = init
        self._last_time = 0
    
    
    def _newTokens(self, cur_time):
        inc_size = int(float(cur_time - self._last_time) / SECOND * self._rate)
        new_size = min(self._cur_size + inc_size, self._max_size)
        return new_size


    def getToken(self, n, cur_time):
        new_size = self._newTokens(cur_time)
        if new_size < n:
            return 0
        else:
            self._last_time = cur_time
            self._cur_size = new_size - n
            return n


if __name__ == "__main__":
    RATE = 20.0
    MAX = 20
    INTERVAL = 1.0 / MAX / 2

    a = TokenBucketShaper(RATE, MAX)
    t = time.time()
    for i in xrange(MAX*2):
        time.sleep(INTERVAL)
        print "%2.2f %d" % (time.time() - t, a.getToken(time.time(), 1))

