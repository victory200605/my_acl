include ../../common.mk

TARGET = ./MemCacheServer
all: $(TARGET)

CXXFLAGS += $(CACHESERVER_V2_FLAGS)
CXXFLAGS += -I$(HOME)/aibc/lib/ailibEx/include
CXXFLAGS += -I../../include/v2/

LIBS += $(CACHESERVER_V2_FLAGS)
LIBS += ../../lib/v2/libCacheApi.a
LIBS += $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#CXX = purify -follow-child-processes -max_threads=500 g++

#####################################################################################################
MEM_CACHE_SRCS = $(shell ls *.cpp ../common/*.cpp)
MEM_CACHE_OBJS = $(subst .cpp,.o,$(MEM_CACHE_SRCS))

#####################################################################################################
MAIN_SRCS   = $(MEM_CACHE_SRCS)
MAIN_OBJS   = $(MEM_CACHE_OBJS)

####################################################################################################
${MAIN_OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@

$(TARGET): $(MAIN_OBJS)
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(LIBS) -o $(TARGET);
	@$(CP) $(TARGET) ../../bin/

#####################################################################################################
clean:;
	@$(RM) *.o  $(TARGET) ../../bin/$(TARGET)
	@find .. -name "*.o" -exec $(RM) {} \;

check:
	$(LINT) $(CXXFLAGS) $(MAIN_SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(MAIN_SRCS) $(COMMON_SRCS)

