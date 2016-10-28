
class KeyExistError(Exception):
    def __init__(self, key):
        self.key = key
        
    def __str__(self):
        return "The key [%r] exist!"%self.key
