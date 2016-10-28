# Sun Solaris Parameters

OSLIBS  += -lpthread -lnsl -lsocket -ldl -lxnet -lposix4 -lm

OSFLAGS += -pthreads -D_POSIX_PTHREAD_SEMANTICS -D_XOPEN_SOURCE=500

SOLINK  = $(CXX) -G

ILP32 = -m32

LP64 = -m64

