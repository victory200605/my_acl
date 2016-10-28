configdir = $(prefix)/config
nobase_config_DATA =


GFQ_BIN_CONFIGDIR = $(PACKAGE)-bin-$(VERSION)/config

dist-hook: 
	@$(NORMAL_INSTALL)
	here=`$(am__cd) $(top_builddir) && pwd`; \
	test -z "$$here/$(GFQ_BIN_CONFIGDIR)" || $(MKDIR_P) "$$here/$(GFQ_BIN_CONFIGDIR)"; \
	$(am__vpath_adj_setup) \
	list='$(nobase_config_DATA)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  $(am__vpath_adj) \
	  $(nobase_configDATA_INSTALL) "$$d$$p" "$$here/$(GFQ_BIN_CONFIGDIR)/$$f"; \
	done;
