nobase_include_HEADERS = \
acl/stl/vector.h         \
acl/stl/string.h         \
acl/stl/map.h            \
acl/stl/set.h            \
acl/stl/algorithm.h      \
acl/stl/deque.h          \
acl/stl/list.h           \
acl/Any.h                \
acl/Config.h             \
acl/DateTime.h           \
acl/Dir.h                \
acl/EventHandler.h       \
acl/File.h               \
acl/FileLog.h            \
acl/FileStream.h         \
acl/FixMemoryPool.h      \
acl/GetOpt.h             \
acl/IndexDict.h          \
acl/IniConfig.h          \
acl/IOHandle.h           \
acl/Log.h                \
acl/McbXml.h             \
acl/MemCache.h           \
acl/MemMap.h             \
acl/MemoryBlock.h        \
acl/MemoryStream.h       \
acl/MemoryPool.h         \
acl/BitArray.h           \
acl/MsgQueue.h           \
acl/PersistentQueue.h    \
acl/Number.h             \
acl/ObjectPool.h         \
acl/Operators.h          \
acl/Poll.h               \
acl/Process.h            \
acl/Reactor.h            \
acl/Regex.h              \
acl/Ring.h               \
acl/SelectType.h         \
acl/Shlib.h              \
acl/smartptr/SharePtr.h  \
acl/smartptr/ShareArray.h\
acl/smartptr/ScopePtr.h  \
acl/smartptr/ScopeArray.h\
acl/SmartPtr.h           \
acl/SockAcceptor.h       \
acl/SockAddr.h           \
acl/SockConnector.h      \
acl/SockDgram.h          \
acl/SockDgramMcast.h     \
acl/SockHandle.h         \
acl/SockStream.h         \
acl/SString.h            \
acl/StrAlgo.h            \
acl/Stream.h             \
acl/Synch.h              \
acl/Thread.h             \
acl/ThreadManager.h      \
acl/ThreadPool.h         \
acl/ThreadTask.h         \
acl/TimerQueue.h         \
acl/TimerQueueAdapter.h  \
acl/Timestamp.h          \
acl/TimeValue.h          \
acl/Utility.h            \
acl/Xml.h                \
acl/XmlConfig.h          \
acl/Codecs.h             \
acl/codecs/Base64.h      \
acl/codecs/Escape.h      \
acl/codecs/Hex.h         \
acl/codecs/Unicode.h     \
acl/codecs/Binary.h      \
acl/Hashs.h              \
acl/HashArray.h          \
acl/Singleton.h          \
acl/PeriodCounter.h      \
aaf/Utility.h            \
aaf/Server.h             \
aaf/LogServer.h          \
aaf/ConfigServer.h       \
aaf/SyncServer.h         \
aaf/TrapServer.h         \
aaf/DaemonServer.h       \
aaf/ControlServer.h       \
aaf/LogService.h          \
aaf/ConfigService.h       \
aaf/TrapService.h         \
aaf/DaemonService.h	      \
aaf/Service.h


ACL_DEVINCLUDEDIR = $(PACKAGE)-dev-$(VERSION)/include

dist-hook: 
	@$(NORMAL_INSTALL)
	here=`$(am__cd) $(top_builddir) && pwd`; \
	test -z "$$here/$(ACL_DEVINCLUDEDIR)" || $(MKDIR_P) "$$here/$(ACL_DEVINCLUDEDIR)"; \
	$(am__vpath_adj_setup) \
	list='$(nobase_include_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  $(am__vpath_adj) \
	  $(nobase_includeHEADERS_INSTALL) "$$d$$p" "$$here/$(ACL_DEVINCLUDEDIR)/$$f"; \
	done
