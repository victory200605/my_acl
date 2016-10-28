LDADD = ../src/libapl.la  -lcppunit -lpthread

AM_CXXFLAGS = $(EXTRA_FLAGS) -I$(srcdir)/../include

AM_COLOR_TESTS = always

AUTOMAKE_OPTIONS = foreign


lib_LTLIBRARIES = libtest_dll.la

libtest_dll_la_SOURCES  = test_dll.c


check_PROGRAMS = test_apl_sys \
                 test_apl_inttypes \
                 test_apl_limits \
                 test_apl_errno \
                 test_apl_time \
                 test_apl_dir \
                 test_apl_fcntl \
                 test_apl_str \
                 test_apl_file \
                 test_apl_fs \
                 test_apl_heap \
                 test_apl_io \
                 test_apl_mem \
                 test_apl_mcast \
                 test_apl_mman \
                 test_apl_proc \
                 test_apl_regex \
                 test_apl_sockaddr \
                 test_apl_sockopt \
                 test_apl_thread \
                 test_apl_dlfcn \
                 test_apl_synch \
                 test_apl_signal \
                 test_apl_socket \
                 test_apl_poll 

test_apl_sys_SOURCES        = TestAplSys.cpp
test_apl_inttypes_SOURCES   = TestAplIntTypes.cpp
test_apl_limits_SOURCES     = TestAplLimits.cpp
test_apl_errno_SOURCES      = TestAplErrno.cpp
test_apl_time_SOURCES       = TestAplTime.cpp
test_apl_dir_SOURCES        = TestAplDir.cpp 
test_apl_fcntl_SOURCES      = TestAplFcntl.cpp
test_apl_str_SOURCES        = TestAplStr.cpp
test_apl_file_SOURCES       = TestAplFile.cpp
test_apl_fs_SOURCES         = TestAplFs.cpp
test_apl_heap_SOURCES       = TestAplHeap.cpp
test_apl_io_SOURCES         = TestAplIo.cpp
test_apl_mem_SOURCES        = TestAplMem.cpp
test_apl_mcast_SOURCES      = TestAplMcast.cpp
test_apl_mman_SOURCES       = TestAplMman.cpp
test_apl_dlfcn_SOURCES      = TestAplDlfcn.cpp
test_apl_signal_SOURCES     = TestAplSignal.cpp
test_apl_proc_SOURCES       = TestAplProc.cpp
test_apl_thread_SOURCES     = TestAplThread.cpp
test_apl_synch_SOURCES      = TestAplSynch.cpp
test_apl_sockaddr_SOURCES   = TestAplSockAddr.cpp
test_apl_sockopt_SOURCES    = TestAplSockOpt.cpp
test_apl_socket_SOURCES     = TestAplSocket.cpp
test_apl_poll_SOURCES       = TestAplPoll.cpp
test_apl_regex_SOURCES      = TestAplRegex.cpp


TESTS = test_apl_sys \
        test_apl_inttypes \
        test_apl_limits \
        test_apl_errno \
        test_apl_time \
        test_apl_str \
        test_apl_mem \
        test_apl_mcast \
        test_apl_heap \
        test_apl_mman \
        test_apl_regex \
        test_apl_dlfcn \
        test_apl_dir \
        test_apl_fs \
        test_apl_fcntl \
        test_apl_file \
        test_apl_io \
        test_apl_proc \
	test_apl_regex \
        test_apl_thread \
        test_apl_synch \
        test_apl_sockaddr \
        test_apl_sockopt \
        test_apl_socket \
        test_apl_poll

APL_DEVTESTDIR = $(PACKAGE)-dev-$(VERSION)/test

dist-hook: 
	@$(NORMAL_INSTALL)
	here=`$(am__cd) $(top_builddir) && pwd`; \
	test -z "$$here/$(APL_DEVTESTDIR)" || $(MKDIR_P) "$$here/$(APL_DEVTESTDIR)"; \
	srcdirstrip=`echo "$(srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
    topsrcdirstrip=`echo "$(top_srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
    list='$(DIST_SOURCES)'; \
      dist_files=`for file in $$list; do echo $$file; done | \
      sed -e "s|^$$srcdirstrip/||;t" \
          -e "s|^$$topsrcdirstrip/|$(top_builddir)/|;t"`; \
    case $$dist_files in \
      */*) $(MKDIR_P) `echo "$$dist_files" | \
               sed '/\//!d;s|^|$$here/$(APL_DEVTESTDIR)/|;s,/[^/]*$$,,' | \
               sort -u` ;; \
    esac; \
    for file in $$dist_files; do \
      if test -f $$file || test -d $$file; then d=.; else d=$(srcdir); fi; \
      if test -d $$d/$$file; then \
        dir=`echo "/$$file" | sed -e 's,/[^/]*$$,,'`; \
        if test -d $(srcdir)/$$file && test $$d != $(srcdir); then \
          cp -pR $(srcdir)/$$file $$here/$(APL_DEVTESTDIR)/$$dir || exit 1; \
        fi; \
        cp -pR $$d/$$file $$here/$(APL_DEVTESTDIR)/$$dir || exit 1; \
      else \
        test -f $$here/$(APL_DEVTESTDIR)/$$file \
        || cp -p $$d/$$file $$here/$(APL_DEVTESTDIR)/$$file \
        || exit 1; \
      fi; \
    done
