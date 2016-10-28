include $(HOME)/aibc/lib/ailibEx/port/port.mk

ifeq ($(CACHESERVER_KEY_LEN), )
    CACHESERVER_KEY_LEN = 48
endif

ifeq ($(CACHESERVER_VALUE_LEN), )
    CACHESERVER_VALUE_LEN = 1536
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

CACHESERVER_V1_FLAGS = $(ILP32)
CACHESERVER_V2_FLAGS = $(ILP32)
CACHESERVER_V3_FLAGS = $(ILP32)

ifeq ($(CACHESERVER_V3_BIT), 64)
    CACHESERVER_V3_FLAGS = $(LP64)
else
    CACHESERVER_V3_BIT = 32
endif

