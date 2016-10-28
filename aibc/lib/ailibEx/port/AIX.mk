# AIX Parameters

OSLIBS  += -lpthread -ldl -lm

OSFLAGS += -pthread -D_REENTRANT -D_XOPEN_SOURCE=600

SOLINK  = $(CXX) -shared

ILP32 = -maix32

LP64 = -maix64

