include ../common.mk

TARGET = ./CacheTestV3
all: $(TARGET)

#CXX = purify -follow-child-processes -max_threads=500 g++

CXXFLAGS += $(CACHESERVER_V3_FLAGS)
CXXFLAGS += -I../include/v3/
CXXFLAGS += -I$(AIBC_HOME)/include/

LIBS += $(CACHESERVER_V3_FLAGS)
LIBS += ../lib/v3/libCacheServerApi.a
LIBS += $(AIBC_HOME)/lib/libanf.a
LIBS += $(AIBC_HOME)/lib/libacl.a
LIBS += $(AIBC_HOME)/lib/libapl.a
LIBS += -lrt


#####################################################################################################
SRCS   = CacheTest.cpp
OBJS   = CacheTest.o

#####################################################################################################
${OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@

$(TARGET): $(OBJS)
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
