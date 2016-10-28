

SUBDIRS = api FileCacheServer MemCacheServer test tool

TARGETNAME=CacheServer

all:
	@for a in $(SUBDIRS); do cd $$a; make $@; cd ..; done; exit 0

clean:
	@for a in $(SUBDIRS); do cd $$a; make $@; cd ..; done; exit 0

package:
	@-tar cvf - bin/Adm bin/FileCacheServer bin/FileCacheTest bin/MemCacheServer bin/MemCacheTest include/*.h lib/*.a config/*.ini | gzip -c > $(TARGETNAME)-$(shell date "+%Y%m%d%H%M%S").tar.gz
