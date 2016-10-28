nobase_include_HEADERS = apl/ctype.h \
                         apl/dlfcn.h \
                         apl/dir.h \
                         apl/inttypes.h \
                         apl/errno.h \
                         apl/fcntl.h \
                         apl/file.h \
                         apl/fs.h \
                         apl/heap.h \
                         apl/limits.h \
                         apl/io.h \
                         apl/sockaddr.h \
                         apl/mcast.h \
                         apl/mem.h \
                         apl/mman.h \
                         apl/proc.h \
                         apl/signal.h \
                         apl/socket.h \
                         apl/sockopt.h \
                         apl/str.h \
                         apl/synch.h \
                         apl/sys.h \
                         apl/thread.h \
                         apl/time.h \
                         apl/types.h \
                         apl/apl.h \
                         apl/poll.h \
                         apl/regex.h

APL_DEVINCLUDEDIR = $(PACKAGE)-dev-$(VERSION)/include

dist-hook: 
	@$(NORMAL_INSTALL)
	here=`$(am__cd) $(top_builddir) && pwd`; \
	test -z "$$here/$(APL_DEVINCLUDEDIR)" || $(MKDIR_P) "$$here/$(APL_DEVINCLUDEDIR)"; \
	$(am__vpath_adj_setup) \
	list='$(nobase_include_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  $(am__vpath_adj) \
	  $(nobase_includeHEADERS_INSTALL) "$$d$$p" "$$here/$(APL_DEVINCLUDEDIR)/$$f"; \
	done
