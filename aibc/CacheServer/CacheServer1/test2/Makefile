include ../common.mk

TARGET = CacheTestV2 MemCacheTest
all: $(TARGET)

CXXFLAGS += $(CACHESERVER_V2_FLAGS)
CXXFLAGS += -I$(HOME)/aibc/lib/ailibEx/include
CXXFLAGS += -I../include/v2/

LIBS += $(CACHESERVER_V2_FLAGS)
LIBS += ../lib/v2/libCacheApi.a
LIBS += $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#CXX = purify -follow-child-processes -max_threads=500 g++

#####################################################################################################
SRCS = $(shell ls *.cpp)
OBJS = $(subst .cpp,.o,$(CACHE_TEST_SRCS))

#####################################################################################################
${OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@

CacheTestV2: CacheTest.o
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(LIBS) -o CacheTestV2;
	@$(CP) CacheTestV2 ../bin/

MemCacheTest: MemCacheTest.o
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(LIBS) -o MemCacheTest;
	@$(CP) MemCacheTest ../bin/

#####################################################################################################
clean:;
	@$(RM) *.o ../bin/CacheTestV2 ../bin/MemCacheTest

check:
	$(LINT) $(CXXFLAGS) $(SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(SRCS)
