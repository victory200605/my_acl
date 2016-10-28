include $(HOME)/aibc/lib/ailibEx/port/port.mk

TARGET = ./MemCacheServer
all: $(TARGET)

CXXFLAGS += -D_AI_ENABLE_NAMESPACE -I$(HOME)/aibc/lib/ailibEx/include
CXXFLAGS += -I$(HOME)/aibc/

DPLIB = ../lib/libCacheApi.a
DPLIB += $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#CXX = purify -follow-child-processes -max_threads=500 g++

#####################################################################################################
MEM_CACHE_SRCS = $(shell ls *.cpp)
MEM_CACHE_OBJS = $(subst .cpp,.o,$(MEM_CACHE_SRCS))

#####################################################################################################
MAIN_SRCS   = $(MEM_CACHE_SRCS)
MAIN_OBJS   = $(MEM_CACHE_OBJS)

####################################################################################################
${MAIN_OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@

$(TARGET): $(MAIN_OBJS)
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(DPLIB) $(LIBS) -o $(TARGET);
	@$(CP) $(TARGET) ../bin/

#####################################################################################################
clean:;
	@$(RM) *.o  $(TARGET)
	@find .. -name "*.o" -exec $(RM) {} \;

check:
	$(LINT) $(CXXFLAGS) $(MAIN_SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(MAIN_SRCS) $(COMMON_SRCS)

