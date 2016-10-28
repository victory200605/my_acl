if AIBC_HOME_FLAG
aibc_include=$(AIBC_HOME)/include
aibc_lib=$(AIBC_HOME)/lib
else
aibc_include=$(srcdir)/../../3rd/aibc/include
aibc_lib=$(srcdir)/../../3rd/aibc/lib
endif

AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../../include  -I$(aibc_include)/
LDADD = $(aibc_lib)/libanf.a $(aibc_lib)/libacl.a $(aibc_lib)/libapl.a

Common_SOURCES =     \
../config/ConfigFactory.cpp \
../config/FileConfig.cpp    \
vlr/VLRSegmentQueue.cpp    \
vlr/VLRBufferPool.cpp    \
vlr/VLRPage.cpp    \
vlr/VLRStorage.cpp    \
flr/FLRSegmentQueue.cpp    \
flr/FLRStorage.cpp    \
MasterConnectManager.cpp    \
ClientHandler.cpp           \
ServerConfig.cpp            \
ServerHandler.cpp

bin_PROGRAMS = GFQSegmentServer GFQCreator GFQCleaner GFQRecover GFQDumper

GFQSegmentServer_SOURCES = $(Common_SOURCES) main.cpp

GFQCreator_SOURCES = $(Common_SOURCES) Creator.cpp

GFQCleaner_SOURCES = $(Common_SOURCES) Cleaner.cpp

GFQRecover_SOURCES = $(Common_SOURCES) Recover.cpp

GFQDumper_SOURCES = $(Common_SOURCES) Dumper.cpp
