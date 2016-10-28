include $(HOME)/aibc/lib/ailibEx/port/port.mk

TARGET = FileCacheTest MemCacheTest
all: $(TARGET)

CXXFLAGS += -D_AI_ENABLE_NAMESPACE -I$(HOME)/aibc/lib/ailibEx/include

DPLIB = ../api/libCacheApi.a
DPLIB += $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#CXX = purify -follow-child-processes -max_threads=500 g++

#####################################################################################################
FILE_CACHE_SRCS = $(shell ls ../FileCache/*.cpp)
FILE_CACHE_OBJS = $(subst .cpp,.o,$(FILE_CACHE_SRCS))

#####################################################################################################
PROTOCOL_SRCS = $(shell ls ../protocol/*.cpp)
PROTOCOL_OBJS = $(subst .cpp,.o,$(PROTOCOL_SRCS))

#####################################################################################################
FILECACHE_MAIN_SRCS   = $(FILE_CACHE_SRCS) $(PROTOCOL_SRCS) FileCacheTest.cpp
FILECACHE_MAIN_OBJS   = $(FILE_CACHE_OBJS) $(PROTOCOL_OBJS) FileCacheTest.o

#####################################################################################################
MEMCACHE_MAIN_SRCS   = MemCacheTest.cpp
MEMCACHE_MAIN_OBJS   = MemCacheTest.o

#####################################################################################################
${FILECACHE_MAIN_OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@
	
${MEMCACHE_MAIN_OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@

FileCacheTest: $(FILECACHE_MAIN_OBJS) $(DPLIB)
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(DPLIB) $(LIBS) -o FileCacheTest;
	@$(CP) FileCacheTest ../bin/

MemCacheTest: $(MEMCACHE_MAIN_OBJS) $(DPLIB)
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(DPLIB) $(LIBS) -o MemCacheTest;
	@$(CP) MemCacheTest ../bin/

#####################################################################################################
clean:;
	@$(RM) *.o  $(TARGET)

check:
	$(LINT) $(CXXFLAGS) $(FILECACHE_MAIN_SRCS) $(MEMCACHE_MAIN_SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(FILECACHE_MAIN_SRCS) $(MEMCACHE_MAIN_SRCS) $(COMMON_SRCS)
