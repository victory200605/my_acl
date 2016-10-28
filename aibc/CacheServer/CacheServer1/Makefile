include common.mk

SUBDIRS = include src test1 test2 test3 tools

all:
	@for a in $(SUBDIRS); do cd $$a; $(MAKE) $@; if [ "$$?" != "0" ]; then exit 1; fi; cd ..; done; exit 0

clean:
	@for a in $(SUBDIRS); do cd $$a; $(MAKE) $@; cd ..; done; exit 0

package:
	@cd ..;tar cvf - CacheServer1/include/v1/cacheserver/*.h \
               CacheServer1/include/v1/cacheserver/*.inc \
               CacheServer1/include/v2/cacheserver/*.h \
               CacheServer1/include/v2/cacheserver/*.inc \
               CacheServer1/include/v3/cacheserver/*.h \
               CacheServer1/include/v3/cacheserver/*.inc \
               CacheServer1/lib/v1/*.a \
               CacheServer1/lib/v2/*.a \
               CacheServer1/lib/v3/*.a \
               CacheServer1/bin/CacheServer \
               CacheServer1/bin/MemCacheServer \
               CacheServer1/bin/CacheTestV1 \
               CacheServer1/bin/CacheTestV2 \
               CacheServer1/bin/CacheTestV3 \
               CacheServer1/bin/Adm \
               CacheServer1/config/*.ini | \
               gzip -c > CacheServer1/cacheserver-$(XPLATFORM)-"V3($(CACHESERVER_V3_BIT))"-K$(CACHESERVER_KEY_LEN)-V$(CACHESERVER_VALUE_LEN)-$(shell date "+%Y%m%d%H%M%S").tar.gz
