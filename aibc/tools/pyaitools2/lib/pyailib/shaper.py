
############################################################################
class TokenBucketShaper(object):
    def __init__(self, rate, bucket_size, precision=1.0):
        self.rate = rate
        self.bucket_size = bucket_size
        self.precision = precision
        self.timestamp = 0
        self.available_size = self.bucket_size
    
    def getAvailableSize(self):
        return self.available_size
    
    def getBucketSize(self):
        return self.bucket_size
    
    def getRate(self):
        return self.rate
    
    def getPrecision(self):
        return self.precision
    
    def getTimestamp(self):
        return self.timestamp
    
    def update(self, t=None):
        if t is None:
            t = time()
        i = (t - self.timestamp) // self.precision * self.precision
        if i <= 0:
            return
        else:
            self.available_size = min(
                self.bucket_size, 
                self.available_size + i * self.rate,
            )
    
    def consume(self, n):
        if n > self.available_size:
            raise ValueError("exceed available size!")
        self.available_size -= n
        