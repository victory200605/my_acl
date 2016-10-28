include common.mk

SUBDIRS = include src test1 test2 tools

all:
	@for a in $(SUBDIRS); do cd $$a; $(MAKE) $@; if [ "$$?" != "0" ]; then exit 1; fi; cd ..; done; exit 0

clean:
	@for a in $(SUBDIRS); do cd $$a; $(MAKE) $@; cd ..; done; exit 0

package:
	@cd ..;tar cvf - GFQ1/include/v1/gfq/*.h \
               GFQ1/include/v1/gfq/*.inc \
               GFQ1/include/v2/gfq/*.h \
               GFQ1/include/v2/gfq/*.inc \
               GFQ1/lib/*.so \
               GFQ1/lib/v1/*.a \
               GFQ1/lib/v2/*.a \
               GFQ1/bin/Master \
               GFQ1/bin/SegmentServer \
               GFQ1/bin/Adm \
               GFQ1/config/*.ini | gzip -c > GFQ1/gfq-$(XPLATFORM)-"V2($(GFQ_V2_BIT))"-D$(GFQ_DATA_LEN)-$(shell date "+%Y%m%d%H%M%S").tar.gz
