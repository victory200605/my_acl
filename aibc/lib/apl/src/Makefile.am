AM_CFLAGS = $(EXTRA_FLAGS) -I$(srcdir)/../include  -I$(top_builddir)/apl/gl -I$(top_builddir)/include 

lib_LTLIBRARIES = libapl.la

libapl_la_LIBADD = ../gl/libgnu.la

libapl_la_SOURCES = \
    inttypes.c \
    ctype.c \
    dir.c \
    dlfcn.c \
    errno.c \
    fcntl.c \
    file.c \
    fs.c \
    heap.c \
    io.c \
    mcast.c \
    mem.c \
    mman.c \
    poll.c \
    proc.c \
    regex.c \
    signal.c \
    sockaddr.c \
    socket.c \
    sockopt.c \
    str.c \
    synch.c \
    thread.c \
    time.c 


APL_BINLIBDIR = $(PACKAGE)-bin-$(VERSION)/lib
APL_DEVLIBDIR = $(PACKAGE)-dev-$(VERSION)/lib

dist-hook: 
	@$(NORMAL_INSTALL)
	here=`$(am__cd) $(top_builddir) && pwd`; \
	test -z "$$here/$(APL_BINLIBDIR)" || $(MKDIR_P) "$$here/$(APL_BINLIBDIR)"; \
	test -z "$$here/$(APL_DEVLIBDIR)" || $(MKDIR_P) "$$here/$(APL_DEVLIBDIR)"; \
	list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    f=$(am__strip_dir) \
		$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$$here/$(APL_BINLIBDIR)/$$f"; \
		$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$$here/$(APL_DEVLIBDIR)/$$f"; \
	  else :; fi; \
	done
