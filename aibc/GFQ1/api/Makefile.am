if AIBC_HOME_FLAG
aibc_include=$(AIBC_HOME)/include
aibc_lib=$(AIBC_HOME)/lib
else
aibc_include=$(srcdir)/../../../3rd/aibc/include
aibc_lib=$(srcdir)/../../../3rd/aibc/lib
endif

AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../include -I$(aibc_include)/

lib_LTLIBRARIES = libGFQApi.la

libGFQApi_la_SOURCES =  GFQProtocol.cpp GFQueueClient.cpp

nobase_include_HEADERS =

GFQ_BINLIBDIR = $(PACKAGE)-bin-$(VERSION)/lib
GFQ_DEVLIBDIR = $(PACKAGE)-dev-$(VERSION)/lib
GFQ_DEVINCLUDEDIR = $(PACKAGE)-dev-$(VERSION)/include

dist-hook: 
	@$(NORMAL_INSTALL)
	here=`$(am__cd) $(top_builddir) && pwd`; \
	test -z "$$here/$(GFQ_BINLIBDIR)" || $(MKDIR_P) "$$here/$(GFQ_BINLIBDIR)"; \
	test -z "$$here/$(GFQ_DEVLIBDIR)" || $(MKDIR_P) "$$here/$(GFQ_DEVLIBDIR)"; \
	test -z "$$here/$(GFQ_DEVINCLUDEDIR)" || $(MKDIR_P) "$$here/$(GFQ_DEVINCLUDEDIR)"; \
	list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
		f=$(am__strip_dir) \
		$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$$here/$(GFQ_BINLIBDIR)/$$f"; \
		$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$$here/$(GFQ_DEVLIBDIR)/$$f"; \
	  else :; fi; \
	done; \
	$(am__vpath_adj_setup) \
	list='$(nobase_include_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  $(am__vpath_adj) \
	  $(nobase_includeHEADERS_INSTALL) "$$d$$p" "$$here/$(GFQ_DEVINCLUDEDIR)/$$f"; \
	done; \
	sed s/^dependency_libs=.*/dependency_libs=\'-lapl\ -lacl\ -lrt\'/g "$$here/$(GFQ_DEVLIBDIR)/libCountServerApi.la" | sed s@^libdir=.*@libdir=\'/usr/local/lib/\'@g > "$$here/$(GFQ_DEVLIBDIR)/libCountServerApi.la_mo"; \
	mv "$$here/$(GFQ_DEVLIBDIR)/libCountServerApi.la_mo" "$$here/$(GFQ_DEVLIBDIR)/libCountServerApi.la"; \
	sed s/^dependency_libs=.*/dependency_libs=\'-lapl\ -lacl\ -lrt\'/g "$$here/$(GFQ_BINLIBDIR)/libCountServerApi.la" | sed s@^libdir=.*@libdir=\'/usr/local/lib/\'@g > "$$here/$(GFQ_BINLIBDIR)/libCountServerApi.la_mo"; \
	mv "$$here/$(GFQ_BINLIBDIR)/libCountServerApi.la_mo" "$$here/$(GFQ_BINLIBDIR)/libCountServerApi.la";
