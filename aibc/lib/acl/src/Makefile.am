AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../include -I$(top_srcdir)/apl/include 

#AM_LDFLAGS = -lapl

lib_LTLIBRARIES = libacl.la

libacl_la_LIBADD = $(top_builddir)/apl/src/.libs/libapl.la

libacl_la_SOURCES =   \
Config.cpp            \
DateTime.cpp          \
Dir.cpp               \
File.cpp              \
FileStream.cpp        \
FileLog.cpp           \
FixMemoryPool.cpp     \
GetOpt.cpp            \
IndexDict.cpp         \
IniConfig.cpp         \
IOHandle.cpp          \
Log.cpp               \
McbXml.cpp            \
MemMap.cpp            \
MemoryBlock.cpp       \
MemoryPool.cpp        \
MemoryStream.cpp      \
BitArray.cpp          \
Poll.cpp              \
Process.cpp           \
Reactor.cpp           \
Regex.cpp             \
Shlib.cpp             \
SockAcceptor.cpp      \
SockAddr.cpp          \
SockConnector.cpp     \
SockDgram.cpp         \
SockDgramMcast.cpp    \
SockHandle.cpp        \
SockStream.cpp        \
SString.cpp           \
StrAlgo.cpp           \
Stream.cpp            \
Synch.cpp             \
Thread.cpp            \
ThreadManager.cpp     \
ThreadPool.cpp        \
ThreadTask.cpp        \
TimerQueue.cpp        \
TimerQueueAdapter.cpp \
Timestamp.cpp         \
TimeValue.cpp         \
Xml.cpp               \
XmlConfig.cpp         \
Codecs.cpp            \
codecs/Base64.cpp     \
codecs/Escape.cpp     \
codecs/Hex.cpp        \
codecs/Binary.cpp     \
codecs/Unicode.cpp    \
Hashs.cpp             \
PeriodCounter.cpp     \
aaf/ConfigServer.cpp  \
aaf/LogServer.cpp     \
aaf/SyncServer.cpp    \
aaf/TrapServer.cpp    \
aaf/DaemonServer.cpp  \
aaf/ControlServer.cpp  \
aaf/LogService.cpp     \
aaf/TrapService.cpp    \
aaf/DaemonService.cpp  \
aaf/ConfigService.cpp


ACL_BINLIBDIR = $(PACKAGE)-bin-$(VERSION)/lib
ACL_DEVLIBDIR = $(PACKAGE)-dev-$(VERSION)/lib

dist-hook: 
	@$(NORMAL_INSTALL)
	here=`$(am__cd) $(top_builddir) && pwd`; \
	test -z "$$here/$(ACL_BINLIBDIR)" || $(MKDIR_P) "$$here/$(ACL_BINLIBDIR)"; \
	test -z "$$here/$(ACL_DEVLIBDIR)" || $(MKDIR_P) "$$here/$(ACL_DEVLIBDIR)"; \
	list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    f=$(am__strip_dir) \
		$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$$here/$(ACL_BINLIBDIR)/$$f"; \
		$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$$here/$(ACL_DEVLIBDIR)/$$f"; \
	  else :; fi; \
	done; \
	sed s/^dependency_libs=.*/dependency_libs=\'-lapl\ -lrt\'/g "$$here/$(ACL_DEVLIBDIR)/libacl.la" | sed s@^libdir=.*@libdir=\'/usr/local/lib/\'@g > "$$here/$(ACL_DEVLIBDIR)/libacl.la_mo"; \
	mv "$$here/$(ACL_DEVLIBDIR)/libacl.la_mo" "$$here/$(ACL_DEVLIBDIR)/libacl.la"; \
	sed s/^dependency_libs=.*/dependency_libs=\'-lapl\ -lrt\'/g "$$here/$(ACL_BINLIBDIR)/libacl.la" | sed s@^libdir=.*@libdir=\'/usr/local/lib/\'@g > "$$here/$(ACL_BINLIBDIR)/libacl.la_mo"; \
	mv "$$here/$(ACL_BINLIBDIR)/libacl.la_mo" "$$here/$(ACL_BINLIBDIR)/libacl.la";
