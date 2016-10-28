AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../include -I$(top_srcdir)/apl/include -I$(top_srcdir)/acl/include

#LIBADD = -lacl -lapl

lib_LTLIBRARIES = libanf.la

libanf_la_LIBADD = $(top_builddir)/apl/src/.libs/libapl.la $(top_builddir)/acl/src/.libs/libacl.la 

libanf_la_SOURCES =                \
Exception.cpp                      \
IoAcceptor.cpp                     \
IoConnector.cpp                    \
IoSockAcceptor.cpp                 \
IoSockConnector.cpp                \
IoFilterChain.cpp                  \
IoFuture.cpp                       \
IoPollingProcessor.cpp             \
IoSimpleProcessorPool.cpp          \
IoService.cpp                      \
IoSession.cpp                      \
IoSessionAdapter.cpp               \
IoConfiguration.cpp                \
IoFilter.cpp                       \
WakeablePoll.cpp                   \
future/AcceptorOperationFuture.cpp \
future/WriteFuture.cpp             \
future/ConnectFuture.cpp           \
filter/ScpMessageFilter.cpp        \
filter/HttpMessageFilter.cpp       \
filter/SMPPMessageFilter.cpp       \
filter/CMPP20MessageFilter.cpp     \
filter/CMPP30MessageFilter.cpp     \
filter/SMMCMessageFilter.cpp       \
filter/SimpleAsyncMessageFilter.cpp \
filter/BencodingMessageFilter.cpp  \
filter/CmdLineMessageFilter.cpp    \
protocol/ScpMessage.cpp            \
protocol/HttpMessage.cpp           \
protocol/SMPPMessage.cpp           \
protocol/CMPPMessage.cpp           \
protocol/SMMCMessage.cpp           \
protocol/CmdLineMessage.cpp        \
protocol/SimpleAsyncMessage.cpp    \
protocol/mm4/MM4SMTP.cpp           \
protocol/mm4/MM4HTTP.cpp           \
util/MessageTable.cpp              \
util/ConnectManager.cpp            \
util/SessionManager.cpp
