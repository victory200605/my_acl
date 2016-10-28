AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../include  -I$(srcdir)/../../apl/include 

AM_LDFLAGS = -L../src/.libs/ -L../../apl/src/.libs -lacl -lapl -lcppunit

noinst_LTLIBRARIES = libtest_dll.la

libtest_dll_la_SOURCES  = test_dll.c

check_PROGRAMS = test_acl_poll          \
                 test_acl_reactor       \
                 test_acl_regex         \
                 test_acl_sstring       \
                 test_acl_datetime      \
                 test_acl_timevalue     \
                 test_acl_timestamp     \
                 test_acl_sockhandle    \
                 test_acl_sockaddr      \
                 test_acl_iohandle      \
                 test_acl_sockstream    \
                 test_acl_sockdgram     \
                 test_acl_sockaccept    \
                 test_acl_sockconnector \
                 test_acl_file          \
                 test_acl_persistentqueue\
                 test_acl_dir           \
                 test_acl_memmap        \
                 test_acl_thread        \
                 test_acl_shlib         \
                 test_acl_getopt        \
                 test_acl_synch         \
                 test_acl_number        \
                 test_acl_any           \
                 test_acl_fixmemorypool \
                 test_acl_memorypool    \
                 test_acl_process       \
                 test_acl_objpool       \
                 test_acl_xml           \
                 test_acl_smartptr      \
                 test_acl_config        \
                 test_acl_memoryblock   \
                 test_acl_memcache      \
                 test_acl_stream        \
                 test_acl_timerqueueadapter \
                 test_acl_filelog       \
                 test_acl_stralgo       \
                 test_acl_memorystream  \
                 test_acl_filestream    \
                 test_acl_codecshex     \
                 test_acl_codecsbase64     \
                 test_acl_codecsbinary  \
                 test_acl_codecsescape  \
                 test_acl_codecsunicode \
                 test_acl_indexdict

check_PROGRAMS += testProExe
testProExe_SOURCES   = test_process_exec.cpp

test_acl_poll_SOURCES           = TestAclPoll.cpp
test_acl_reactor_SOURCES        = TestAclReactor.cpp
test_acl_regex_SOURCES          = TestAclRegex.cpp
test_acl_sstring_SOURCES        = TestAclSString.cpp
test_acl_datetime_SOURCES       = TestAclDateTime.cpp
test_acl_timevalue_SOURCES      = TestAclTimeValue.cpp
test_acl_timestamp_SOURCES      = TestAclTimestamp.cpp
test_acl_sockhandle_SOURCES     = TestAclSockHandle.cpp
test_acl_sockaddr_SOURCES       = TestAclSockAddr.cpp
test_acl_iohandle_SOURCES       = TestAclIOHandle.cpp
test_acl_sockstream_SOURCES     = TestAclSockStream.cpp
test_acl_sockdgram_SOURCES      = TestAclSockDgram.cpp
test_acl_sockaccept_SOURCES     = TestAclSockAcceptor.cpp
test_acl_sockconnector_SOURCES  = TestAclSockConnector.cpp
test_acl_file_SOURCES           = TestAclFile.cpp
test_acl_persistentqueue_SOURCES = TestAclPersistentQueue.cpp
test_acl_dir_SOURCES            = TestAclDir.cpp
test_acl_memmap_SOURCES         = TestAclMemmap.cpp
test_acl_thread_SOURCES         = TestAclThread.cpp
test_acl_shlib_SOURCES          = TestAclShlib.cpp
test_acl_getopt_SOURCES         = TestAclGetOpt.cpp
test_acl_synch_SOURCES          = TestAclSynch.cpp
test_acl_number_SOURCES         = TestAclNumber.cpp
test_acl_any_SOURCES            = TestAclAny.cpp
test_acl_fixmemorypool_SOURCES  = TestAclFixMemoryPool.cpp
test_acl_memorypool_SOURCES     = TestAclMemoryPool.cpp
test_acl_process_SOURCES        = TestAclProcess.cpp
test_acl_objpool_SOURCES        = TestAclObjectPool.cpp
test_acl_xml_SOURCES            = TestAclXml.cpp
test_acl_smartptr_SOURCES       = TestAclSmartPtr.cpp
test_acl_config_SOURCES         = TestAclConfig.cpp
test_acl_memoryblock_SOURCES    = TestAclMemoryBlock.cpp
test_acl_stream_SOURCES         = TestAclStream.cpp
test_acl_memcache_SOURCES       = TestAclMemCache.cpp
test_acl_filelog_SOURCES        = TestAclFileLog.cpp
test_acl_stralgo_SOURCES        = TestAclStrAlgo.cpp
test_acl_memorystream_SOURCES   = TestAclMemoryStream.cpp
test_acl_filestream_SOURCES     = TestAclFileStream.cpp
test_acl_codecshex_SOURCES      = TestAclCodecsHex.cpp
test_acl_codecsbase64_SOURCES   = TestAclCodecsBase64.cpp
test_acl_codecsbinary_SOURCES   = TestAclCodecsBinary.cpp
test_acl_codecsescape_SOURCES   = TestAclCodecsEscape.cpp
test_acl_codecsunicode_SOURCES  = TestAclCodecsUnicode.cpp
test_acl_timerqueueadapter_SOURCES  = TestAclTimerQueueAdapter.cpp
test_acl_indexdict_SOURCES      = TestAclIndexDict.cpp

TESTS = test_acl_poll          \
        test_acl_reactor       \
        test_acl_regex         \
        test_acl_sstring       \
        test_acl_datetime      \
        test_acl_timevalue     \
        test_acl_timestamp     \
        test_acl_sockhandle    \
        test_acl_sockaddr      \
        test_acl_iohandle      \
        test_acl_sockstream    \
        test_acl_sockdgram     \
        test_acl_sockaccept    \
        test_acl_sockconnector \
        test_acl_file          \
        test_acl_persistentqueue \
        test_acl_dir           \
        test_acl_memmap        \
        test_acl_thread        \
        test_acl_shlib         \
        test_acl_getopt        \
        test_acl_synch         \
        test_acl_number        \
        test_acl_any           \
        test_acl_fixmemorypool \
        test_acl_memorypool    \
        test_acl_process       \
        test_acl_objpool       \
        test_acl_xml           \
        test_acl_smartptr      \
        test_acl_config        \
        test_acl_memoryblock   \
        test_acl_memcache      \
        test_acl_stream        \
        test_acl_timerqueueadapter\
        test_acl_filelog       \
        test_acl_stralgo       \
        test_acl_memorystream  \
        test_acl_filestream    \
        test_acl_codecshex     \
        test_acl_codecsbase64  \
        test_acl_codecsbinary  \
        test_acl_codecsescape  \
        test_acl_codecsunicode \
        test_acl_indexdict

ACL_DEVTESTDIR = $(PACKAGE)-dev-$(VERSION)/test

dist-hook: 
    @$(NORMAL_INSTALL)
    here=`$(am__cd) $(top_builddir) && pwd`; \
    test -z "$$here/$(ACL_DEVTESTDIR)" || $(MKDIR_P) "$$here/$(ACL_DEVTESTDIR)"; \
    srcdirstrip=`echo "$(srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
    topsrcdirstrip=`echo "$(top_srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
    list='$(DIST_SOURCES)'; \
      dist_files=`for file in $$list; do echo $$file; done | \
      sed -e "s|^$$srcdirstrip/||;t" \
          -e "s|^$$topsrcdirstrip/|$(top_builddir)/|;t"`; \
    case $$dist_files in \
      */*) $(MKDIR_P) `echo "$$dist_files" | \
               sed '/\//!d;s|^|$$here/$(ACL_DEVTESTDIR)/|;s,/[^/]*$$,,' | \
               sort -u` ;; \
    esac; \
    for file in $$dist_files; do \
      if test -f $$file || test -d $$file; then d=.; else d=$(srcdir); fi; \
      if test -d $$d/$$file; then \
        dir=`echo "/$$file" | sed -e 's,/[^/]*$$,,'`; \
        if test -d $(srcdir)/$$file && test $$d != $(srcdir); then \
          cp -pR $(srcdir)/$$file $$here/$(ACL_DEVTESTDIR)/$$dir || exit 1; \
        fi; \
        cp -pR $$d/$$file $$here/$(ACL_DEVTESTDIR)/$$dir || exit 1; \
      else \
        test -f $$here/$(ACL_DEVTESTDIR)/$$file \
        || cp -p $$d/$$file $$here/$(ACL_DEVTESTDIR)/$$file \
        || exit 1; \
      fi; \
    done
