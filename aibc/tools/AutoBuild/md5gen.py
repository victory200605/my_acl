#!/usr/bin/env python
try:
    import hashlib as md5
except:
    import md5
import sys

def calcmd5(f):
   return md5.md5(f.read()).hexdigest()

for f in sys.argv[1:]:
    print calcmd5(open(f)), f

