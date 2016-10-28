# Cygwin Parameters

OSLIBS  += -lpthread -ldl -lm

OSFLAGS += -pthread -D_XOPEN_SOURCE=600

SOLINK  = $(CXX) -shared

ILP32 = -m32

LP64 = -m64

