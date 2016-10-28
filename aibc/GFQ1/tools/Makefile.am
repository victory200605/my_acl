if AIBC_HOME_FLAG
aibc_include=$(AIBC_HOME)/include
aibc_lib=$(AIBC_HOME)/lib
else
aibc_include=$(srcdir)/../../../3rd/aibc/include
aibc_lib=$(srcdir)/../../../3rd/aibc/lib
endif

AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../include -I$(aibc_include)/

bin_PROGRAMS = GFQueueTest

GFQueueTest_LDADD = ../api/.libs/libGFQApi.a $(aibc_lib)/libanf.a $(aibc_lib)/libacl.a $(aibc_lib)/libapl.a

GFQueueTest_SOURCES =               \
GFQueueTest.cpp

