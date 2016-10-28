# Support platform: SUNOS, LINUX, AIX, HPUX

ifndef AIBC_ROOT
    AIBC_ROOT = $(HOME)/aibc
endif

ifndef AILIBEX_ROOT
    AILIBEX_ROOT = $(AIBC_ROOT)/lib/ailibEx
endif

PLATFORM=$(shell uname -s | tr "abcdefghijklmnopqrstuvwxyz-" "ABCDEFGHIJKLMNOPQRSTUVWXYZ_")

# platform
include $(AILIBEX_ROOT)/port/$(PLATFORM).mk

# commands
CXX=g++
PROC=proc
AR=ar cru
PROCFLAGS=ireclen=1024 oreclen=1024 include=/usr/include include=../include THREAD=YES
LINKER=$(CXX) 
RANLIB=ranlib
RM=/bin/rm -rf
CP=/bin/cp -f
MKDIR=/bin/mkdir -p

# args
LIBS += $(OSLIBS) 
CXXFLAGS += -g -Woverloaded-virtual -Wall -pipe -D$(PLATFORM) $(OSFLAGS) -D_REENTRANT

# default rules
.SUFFIXES: .a .o .cpp .pc

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $*.cpp

.pc.cpp:
	@echo "PROC ..."
	-$(PROC) iname=$*.pc code=cpp oname=$*.cpp $(PROCFLAGS) 

