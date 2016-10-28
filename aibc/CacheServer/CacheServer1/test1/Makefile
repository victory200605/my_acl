include ../common.mk

TARGET = CacheTestV1
all: $(TARGET)

CXXFLAGS += $(CACHESERVER_V1_FLAGS)
CXXFLAGS += -I$(HOME)/aibc/lib/ailibEx/include
CXXFLAGS += -I../include/v1/

LIBS += $(CACHESERVER_V1_FLAGS)
LIBS += ../lib/v1/libCacheClient.a
LIBS += $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#CXX = purify -follow-child-processes -max_threads=500 gcc

#####################################################################################################
MAIN_OBJS   = main.o

#####################################################################################################
${MAIN_OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@}

$(TARGET): $(MAIN_OBJS)
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(LIBS) -o $(TARGET);
	@$(CP) $(TARGET) ../bin/

#####################################################################################################
clean:;
	@$(RM) *.o  $(TARGET) ../bin/$(TARGET)

check:
	$(LINT) $(CXXFLAGS) $(MAIN_SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(MAIN_SRCS) $(COMMON_SRCS)
