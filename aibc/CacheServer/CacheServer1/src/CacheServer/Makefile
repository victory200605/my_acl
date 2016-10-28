include ../../common.mk

TARGET = ./CacheServer
all: $(TARGET)

CXXFLAGS += $(CACHESERVER_V1_FLAGS)
CXXFLAGS += -I$(HOME)/aibc/lib/ailibEx/include
CXXFLAGS += -I../../include/v1/

LIBS += $(CACHESERVER_V1_FLAGS)
LIBS += $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#CXX = purify -follow-child-processes -max_threads=500 g++

#####################################################################################################
MAIN_SRCS = $(shell ls *.cpp ../common/*.cpp BillCache/*.cpp)
MAIN_OBJS = $(subst .cpp,.o,$(MAIN_SRCS))

####################################################################################################
${MAIN_OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@

$(TARGET): $(MAIN_OBJS) $(DPLIB)
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(LIBS) -o $(TARGET);
	@$(CP) $(TARGET) ../../bin/

#####################################################################################################
clean:;
	@$(RM) *.o ../common/*.o BillCache/*.o $(TARGET)
	@$(RM) ../../bin/$(TARGET)

check:
	$(LINT) $(CXXFLAGS) $(MAIN_SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(MAIN_SRCS) $(COMMON_SRCS)

