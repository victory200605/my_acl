include $(HOME)/aibc/lib/ailibEx/port/port.mk

ifeq ($(GFQ_DATA_LEN), )
    GFQ_DATA_LEN = 1536
endif


ifeq ($(PLATFORM), LINUX)
    CXXFLAGS += -DAI_OS_LINUX=1
    XPLATFORM = "`uname -m`-`uname -n`-`uname`"
endif
ifeq ($(PLATFORM), AIX)
    CXXFLAGS += -DAI_OS_AIX=1
    XPLATFORM = "`uname -p`-`uname`-`uname -v`"
endif
ifeq ($(PLATFORM), SUNOS)
    CXXFLAGS += -DAI_OS_SUNOS=1
    XPLATFORM = "`uname -p`-`uname`-`uname -r`"
endif
ifeq ($(PLATFORM), HP_UX)
    CXXFLAGS += -DAI_OS_HPUX=1
    XPLATFORM = "`uname -m`-`uname`-`uname -r`"
endif

GFQ_V1_FLAGS = $(ILP32)
GFQ_V2_FLAGS = $(ILP32)

ifeq ($(GFQ_V2_BIT), 64)
    GFQ_V2_FLAGS = $(LP64)
else
    GFQ_V2_BIT = 32
endif

#statserver for ismg50
ifeq ($(GFQ_ENABLE_STATSERVER), 1)
    CXXFLAGS += -D_ENABLE_TRAP -I$(STATSERVER_HOME)/include
    LIBS += $(STATSERVER_HOME)/lib/statapi.a
endif
