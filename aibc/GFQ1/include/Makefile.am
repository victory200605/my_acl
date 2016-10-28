nobase_include_HEADERS = \
gfq/GFQueueClient.h \
gfq/GFQProtocol.h \
gfq/Utility.h


GFQ_DEVINCLUDEDIR = $(PACKAGE)-dev-$(VERSION)/include

dist-hook: 
	@$(NORMAL_INSTALL)
	here=`$(am__cd) $(top_builddir) && pwd`; \
	test -z "$$here/$(GFQ_DEVINCLUDEDIR)" || $(MKDIR_P) "$$here/$(GFQ_DEVINCLUDEDIR)"; \
	$(am__vpath_adj_setup) \
	list='$(nobase_include_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  $(am__vpath_adj) \
	  $(nobase_includeHEADERS_INSTALL) "$$d$$p" "$$here/$(GFQ_DEVINCLUDEDIR)/$$f"; \
	done
