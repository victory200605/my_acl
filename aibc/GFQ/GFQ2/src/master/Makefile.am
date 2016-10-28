if AIBC_HOME_FLAG
aibc_include=$(AIBC_HOME)/include
aibc_lib=$(AIBC_HOME)/lib
else
aibc_include=$(srcdir)/../../3rd/aibc/include
aibc_lib=$(srcdir)/../../3rd/aibc/lib
endif

AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../../include  -I$(aibc_include)/
LDADD = $(aibc_lib)/libanf.a $(aibc_lib)/libacl.a $(aibc_lib)/libapl.a

bin_PROGRAMS = GFQMaster

GFQMaster_SOURCES =         \
../config/ConfigFactory.cpp \
../config/FileConfig.cpp    \
KeepAlived.cpp              \
EventObserver.cpp           \
FIFOQueue.cpp               \
main.cpp                    \
MasterConfig.cpp            \
QueueFactory.cpp            \
QueueManager.cpp            \
QueuePoll.cpp               \
ServerHandler.cpp           \
ServerNode.cpp              \
ServerNodeGroup.cpp         \
ServerNodeManager.cpp       \
ServerNodePriorityList.cpp
