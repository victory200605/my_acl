if AIBC_HOME_FLAG
aibc_include=$(AIBC_HOME)/include
aibc_lib=$(AIBC_HOME)/lib
else
aibc_include=$(srcdir)/../../../3rd/aibc/include
aibc_lib=$(srcdir)/../../../3rd/aibc/lib
endif

AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../include -I$(aibc_include)/

bin_PROGRAMS = 

GFQ_BIN_BINDIR = $(PACKAGE)-bin-$(VERSION)/bin

dist-hook: 
	@$(NORMAL_INSTALL)
	here=`$(am__cd) $(top_builddir) && pwd`; \
	test -z "$$here/$(GFQ_BIN_BINDIR)" || $(MKDIR_P) "$$here/$(GFQ_BIN_BINDIR)"; \
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  p1=`echo $$p|sed 's/$(EXEEXT)$$//'`; \
	  if test -f $$p \
	     || test -f $$p1 \
	  ; then \
	    f=`echo "$$p1" | sed 's,^.*/,,;$(transform);s/$$/$(EXEEXT)/'`; \
		$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(binPROGRAMS_INSTALL) "$$p" "$$here/$(GFQ_BIN_BINDIR)/$$f"; \
	  else :; fi; \
	done;
