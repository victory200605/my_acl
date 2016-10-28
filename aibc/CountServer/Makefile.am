ACLOCAL_AMFLAGS = -I m4
SUBDIRS = api src test include config


COUNTSERVER_BINDIR = $(PACKAGE)-bin-$(VERSION)
COUNTSERVER_DEVDIR = $(PACKAGE)-dev-$(VERSION)
COUNTSERVER_MD5GEN = $(top_srcdir)/md5gen.py

dist-hook: 
	-test -d "$(COUNTSERVER_BINDIR)" && \
	rm -rf $(COUNTSERVER_BINDIR)/lib/*.a && \
	cp -pR "$(top_srcdir)/ChangeLog" "$(COUNTSERVER_BINDIR)" && \
	find "$(COUNTSERVER_BINDIR)" -type f -exec $(COUNTSERVER_MD5GEN) {} > "$(COUNTSERVER_BINDIR)/md5sum" \; ;\
	tardir=$(COUNTSERVER_BINDIR) && $(am__tar) | GZIP=$(GZIP_ENV) gzip -c >$(COUNTSERVER_BINDIR).tar.gz && \
	rm -rf $(COUNTSERVER_BINDIR);
	-test -d "$(COUNTSERVER_DEVDIR)" && \
	cp -pR "$(top_srcdir)/ChangeLog" "$(COUNTSERVER_DEVDIR)" && \
	find "$(COUNTSERVER_DEVDIR)" -type f -exec $(COUNTSERVER_MD5GEN) {} > "$(COUNTSERVER_DEVDIR)/md5sum" \; ;\
	tardir=$(COUNTSERVER_DEVDIR) && $(am__tar) | GZIP=$(GZIP_ENV) gzip -c >$(COUNTSERVER_DEVDIR).tar.gz && \
	rm -rf $(COUNTSERVER_DEVDIR)
