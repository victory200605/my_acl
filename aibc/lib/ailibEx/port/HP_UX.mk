# HP Unix Parameters

OSLIBS  += -lpthread -lnsl -lrt -lm

OSFLAGS += -pthread -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED=1

SOLINK  = $(CXX) -shared

ILP32 = -milp32

LP64 = -mlp64

