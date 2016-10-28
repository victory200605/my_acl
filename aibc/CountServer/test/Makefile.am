AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../include -I$(AIBC_HOME)/include

LDADD = $(AIBC_HOME)/lib/libacl.a
LDADD += $(AIBC_HOME)/lib/libapl.a

bin_PROGRAMS = CountServerTest

CountServerTest_SOURCES =           \
../protocol/CountServerProtocol.cpp \
../api/CountServerApi.cpp           \
main.cpp


COUNTSERVER_BIN_BINDIR = $(PACKAGE)-bin-$(VERSION)/bin

dist-hook: 
	@$(NORMAL_INSTALL)
	here=`$(am__cd) $(top_builddir) && pwd`; \
	test -z "$$here/$(COUNTSERVER_BIN_BINDIR)" || $(MKDIR_P) "$$here/$(COUNTSERVER_BIN_BINDIR)"; \
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  p1=`echo $$p|sed 's/$(EXEEXT)$$//'`; \
	  if test -f $$p \
	     || test -f $$p1 \
	  ; then \
	    f=`echo "$$p1" | sed 's,^.*/,,;$(transform);s/$$/$(EXEEXT)/'`; \
		$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(binPROGRAMS_INSTALL) "$$p" "$$here/$(COUNTSERVER_BIN_BINDIR)/$$f"; \
	  else :; fi; \
	done;
