include ../common.mk

TARGET = ./GFQTestV2
all: $(TARGET)

#CXX = purify -follow-child-processes -max_threads=500 g++

CXXFLAGS += $(GFQ_V2_FLAGS)
CXXFLAGS += -I../include/v2/
CXXFLAGS += -I$(AIBC_HOME)/include/

LIBS += $(GFQ_V2_FLAGS)
LIBS += ../lib/v2/libGFQApi.a
LIBS += $(AIBC_HOME)/lib/libanf.a
LIBS += $(AIBC_HOME)/lib/libacl.a
LIBS += $(AIBC_HOME)/lib/libapl.a
LIBS += -lrt


#####################################################################################################
SRCS   = GFQueueTest.cpp
OBJS   = GFQueueTest.o

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
