if AIBC_HOME_FLAG
aibc_include=$(AIBC_HOME)/include
aibc_lib=$(AIBC_HOME)/lib
else
aibc_include=$(srcdir)/../../../3rd/aibc/include
aibc_lib=$(srcdir)/../../../3rd/aibc/lib
endif

AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../include -I$(aibc_include)/

lib_LTLIBRARIES = libCacheServer2Api.la

libCacheServer2Api_la_SOURCES =  CacheClient.cpp

nobase_include_HEADERS =

CACHESERVER2_BINLIBDIR = $(PACKAGE)-bin-$(VERSION)/lib
CACHESERVER2_DEVLIBDIR = $(PACKAGE)-dev-$(VERSION)/lib
CACHESERVER2_DEVINCLUDEDIR = $(PACKAGE)-dev-$(VERSION)/include

dist-hook: 
	@$(NORMAL_INSTALL)
	here=`$(am__cd) $(top_builddir) && pwd`; \
	test -z "$$here/$(CACHESERVER2_BINLIBDIR)" || $(MKDIR_P) "$$here/$(CACHESERVER2_BINLIBDIR)"; \
	test -z "$$here/$(CACHESERVER2_DEVLIBDIR)" || $(MKDIR_P) "$$here/$(CACHESERVER2_DEVLIBDIR)"; \
	test -z "$$here/$(CACHESERVER2_DEVINCLUDEDIR)" || $(MKDIR_P) "$$here/$(CACHESERVER2_DEVINCLUDEDIR)"; \
	list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
		f=$(am__strip_dir) \
		$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$$here/$(CACHESERVER2_BINLIBDIR)/$$f"; \
		$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$$here/$(CACHESERVER2_DEVLIBDIR)/$$f"; \
	  else :; fi; \
	done; \
	$(am__vpath_adj_setup) \
	list='$(nobase_include_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  $(am__vpath_adj) \
	  $(nobase_includeHEADERS_INSTALL) "$$d$$p" "$$here/$(CACHESERVER2_DEVINCLUDEDIR)/$$f"; \
	done; \
	sed s/^dependency_libs=.*/dependency_libs=\'-lapl\ -lacl\ -lrt\'/g "$$here/$(CACHESERVER2_DEVLIBDIR)/libCountServerApi.la" | sed s@^libdir=.*@libdir=\'/usr/local/lib/\'@g > "$$here/$(CACHESERVER2_DEVLIBDIR)/libCountServerApi.la_mo"; \
	mv "$$here/$(CACHESERVER2_DEVLIBDIR)/libCountServerApi.la_mo" "$$here/$(CACHESERVER2_DEVLIBDIR)/libCountServerApi.la"; \
	sed s/^dependency_libs=.*/dependency_libs=\'-lapl\ -lacl\ -lrt\'/g "$$here/$(CACHESERVER2_BINLIBDIR)/libCountServerApi.la" | sed s@^libdir=.*@libdir=\'/usr/local/lib/\'@g > "$$here/$(CACHESERVER2_BINLIBDIR)/libCountServerApi.la_mo"; \
	mv "$$here/$(CACHESERVER2_BINLIBDIR)/libCountServerApi.la_mo" "$$here/$(CACHESERVER2_BINLIBDIR)/libCountServerApi.la";
