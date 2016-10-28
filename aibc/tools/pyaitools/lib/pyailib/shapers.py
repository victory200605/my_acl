import time

def _align(t, p):
    return t // p * p


class TokenBucketShaper(object):
    """ Usage:

        shaper = TokenBucketShaper(10.0, 10.0)

        while True:
            # xxx
            shaper.ajust(time.time())
            if shaper.accquireN(1):
               # to send
    """

    def __init__(self, rate, bucket_size, precision=1.0, timestamp=None):
        self._rate = rate
        self._precision = precision
        if timestamp is None:
            timestamp = time.time()
        self._timestamp = _align(timestamp, precision)
        self._bucket_size = bucket_size
        self._token_count = 0.0

    def getBucketSize(self):
        return self._bucket_size

    def getTokenCount(self):
        return self._token_count

    def getRate(self):
        return self._rate

    def getPrecision(self):
        return self._precision

    def getTimestamp(self):
        return self._timestamp

    def _calculate(self, t):
        new_timestamp = max(self._timestamp, _align(t, self._precision))
        new_size = min(self._bucket_size, (new_timestamp - self._timestamp) * self._rate + self._token_count)
        return new_size, new_timestamp

    def ajust(self, t=None):
        if t is None:
            t = time.time()
        self._token_count, self._timestamp = self._calculate(t)

    def accquire(self, n):
        old_size = self._token_count
        self._token_count = max(0, old_size - n)
        return min(old_size, n)

    def accquireN(self, n):
        if self._token_count >= n:
            self._token_count -= n
            return n
        else:
            return 0
