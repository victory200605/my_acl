nobase_include_HEADERS = \
cacheserver2/CacheClient.h \
cacheserver2/Utility.h


CACHESERVER2_DEVINCLUDEDIR = $(PACKAGE)-dev-$(VERSION)/include

dist-hook: 
	@$(NORMAL_INSTALL)
	here=`$(am__cd) $(top_builddir) && pwd`; \
	test -z "$$here/$(CACHESERVER2_DEVINCLUDEDIR)" || $(MKDIR_P) "$$here/$(CACHESERVER2_DEVINCLUDEDIR)"; \
	$(am__vpath_adj_setup) \
	list='$(nobase_include_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  $(am__vpath_adj) \
	  $(nobase_includeHEADERS_INSTALL) "$$d$$p" "$$here/$(CACHESERVER2_DEVINCLUDEDIR)/$$f"; \
	done
