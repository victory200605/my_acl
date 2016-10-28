SUBDIRS = include src test

ACL_BINDIR = $(PACKAGE)-bin-$(VERSION)
ACL_DEVDIR = $(PACKAGE)-dev-$(VERSION)
ACL_MD5GEN = $(top_srcdir)/md5gen.py

dist-hook: 
	-test -d "$(ACL_BINDIR)" && \
	rm -rf $(ACL_BINDIR)/lib/*.a && \
	cp -pR "$(top_srcdir)/ChangeLog" "$(ACL_BINDIR)" && \
	find "$(ACL_BINDIR)" -type f -exec $(ACL_MD5GEN) {} > "$(ACL_BINDIR)/md5sum" \; ;\
	tardir=$(ACL_BINDIR) && $(am__tar) | GZIP=$(GZIP_ENV) gzip -c >$(ACL_BINDIR).tar.gz && \
	rm -rf $(ACL_BINDIR);
	-test -d "$(ACL_DEVDIR)" && \
	cp -pR "$(top_srcdir)/ChangeLog" "$(ACL_DEVDIR)" && \
	find "$(ACL_DEVDIR)" -type f -exec $(ACL_MD5GEN) {} > "$(ACL_DEVDIR)/md5sum" \; ;\
	tardir=$(ACL_DEVDIR) && $(am__tar) | GZIP=$(GZIP_ENV) gzip -c >$(ACL_DEVDIR).tar.gz && \
	rm -rf $(ACL_DEVDIR)
