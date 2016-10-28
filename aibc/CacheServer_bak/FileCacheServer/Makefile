include $(HOME)/aibc/lib/ailibEx/port/port.mk

TARGET = ./FileCacheServer
all: $(TARGET)

CXXFLAGS += -D_AI_ENABLE_NAMESPACE -I$(HOME)/aibc/lib/ailibEx/include

DPLIB = $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#CXX = purify -follow-child-processes -max_threads=500 g++

#####################################################################################################
FILE_CACHE_SRCS = $(shell ls ../FileCache/*.cpp)
FILE_CACHE_OBJS = $(subst .cpp,.o,$(FILE_CACHE_SRCS))

#####################################################################################################
PROTOCOL_SRCS = $(shell ls ../protocol/*.cpp)
PROTOCOL_OBJS = $(subst .cpp,.o,$(PROTOCOL_SRCS))

#####################################################################################################
MAIN_SRCS   = $(FILE_CACHE_SRCS) $(PROTOCOL_SRCS) FileCacheServer.cpp AITask.cpp
MAIN_OBJS   = $(FILE_CACHE_OBJS) $(PROTOCOL_OBJS) FileCacheServer.o AITask.o

####################################################################################################
${MAIN_OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@

$(TARGET): $(MAIN_OBJS) $(DPLIB)
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

