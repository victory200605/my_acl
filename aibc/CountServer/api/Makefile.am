AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../include -I$(AIBC_HOME)/include

lib_LTLIBRARIES = libCountServerApi.la

libCountServerApi_la_SOURCES =   \
CountServerApi.cpp               \
../protocol/CountServerProtocol.cpp

libCountServerApi_la_LIBADD = $(AIBC_HOME)/lib/libacl.la $(AIBC_HOME)/lib/libapl.la

nobase_include_HEADERS = CountServerApi.h

COUNTSERVER_BINLIBDIR = $(PACKAGE)-bin-$(VERSION)/lib
COUNTSERVER_DEVLIBDIR = $(PACKAGE)-dev-$(VERSION)/lib
COUNTSERVER_DEVINCLUDEDIR = $(PACKAGE)-dev-$(VERSION)/include

dist-hook: 
	@$(NORMAL_INSTALL)
	here=`$(am__cd) $(top_builddir) && pwd`; \
	test -z "$$here/$(COUNTSERVER_BINLIBDIR)" || $(MKDIR_P) "$$here/$(COUNTSERVER_BINLIBDIR)"; \
	test -z "$$here/$(COUNTSERVER_DEVLIBDIR)" || $(MKDIR_P) "$$here/$(COUNTSERVER_DEVLIBDIR)"; \
	test -z "$$here/$(COUNTSERVER_DEVINCLUDEDIR)" || $(MKDIR_P) "$$here/$(COUNTSERVER_DEVINCLUDEDIR)"; \
	list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
		f=$(am__strip_dir) \
		$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$$here/$(COUNTSERVER_BINLIBDIR)/$$f"; \
		$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$$here/$(COUNTSERVER_DEVLIBDIR)/$$f"; \
	  else :; fi; \
	done; \
	$(am__vpath_adj_setup) \
	list='$(nobase_include_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  $(am__vpath_adj) \
	  $(nobase_includeHEADERS_INSTALL) "$$d$$p" "$$here/$(COUNTSERVER_DEVINCLUDEDIR)/$$f"; \
	done; \
	sed s/^dependency_libs=.*/dependency_libs=\'-lapl\ -lacl\ -lrt\'/g "$$here/$(COUNTSERVER_DEVLIBDIR)/libCountServerApi.la" | sed s@^libdir=.*@libdir=\'/usr/local/lib/\'@g > "$$here/$(COUNTSERVER_DEVLIBDIR)/libCountServerApi.la_mo"; \
	mv "$$here/$(COUNTSERVER_DEVLIBDIR)/libCountServerApi.la_mo" "$$here/$(COUNTSERVER_DEVLIBDIR)/libCountServerApi.la"; \
	sed s/^dependency_libs=.*/dependency_libs=\'-lapl\ -lacl\ -lrt\'/g "$$here/$(COUNTSERVER_BINLIBDIR)/libCountServerApi.la" | sed s@^libdir=.*@libdir=\'/usr/local/lib/\'@g > "$$here/$(COUNTSERVER_BINLIBDIR)/libCountServerApi.la_mo"; \
	mv "$$here/$(COUNTSERVER_BINLIBDIR)/libCountServerApi.la_mo" "$$here/$(COUNTSERVER_BINLIBDIR)/libCountServerApi.la";
