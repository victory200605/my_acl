#ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = ChangeLog

SUBDIRS = include gl src test

APL_BINDIR = $(PACKAGE)-bin-$(VERSION)
APL_DEVDIR = $(PACKAGE)-dev-$(VERSION)
APL_MD5GEN = $(top_srcdir)/md5gen.py

dist-hook: 
	-test -d "$(APL_BINDIR)" && \
	rm -rf $(APL_BINDIR)/lib/*.a && \
	cp -pR "$(top_srcdir)/ChangeLog" "$(APL_BINDIR)" && \
	find "$(APL_BINDIR)" -type f -exec $(APL_MD5GEN) {} > "$(APL_BINDIR)/md5sum" \; ;\
	tardir=$(APL_BINDIR) && $(am__tar) | GZIP=$(GZIP_ENV) gzip -c >$(APL_BINDIR).tar.gz && \
	rm -rf $(APL_BINDIR);
	-test -d "$(APL_DEVDIR)" && \
	cp -pR "$(top_srcdir)/ChangeLog" "$(APL_DEVDIR)" && \
	find "$(APL_DEVDIR)" -type f -exec $(APL_MD5GEN) {} > "$(APL_DEVDIR)/md5sum" \; ;\
	tardir=$(APL_DEVDIR) && $(am__tar) | GZIP=$(GZIP_ENV) gzip -c >$(APL_DEVDIR).tar.gz && \
	rm -rf $(APL_DEVDIR)
