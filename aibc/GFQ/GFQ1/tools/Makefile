include ../common.mk

TARGET = Adm
all: $(TARGET)

#CXX = purify -follow-child-processes -max_threads=500 g++

CXXFLAGS += $(GFQ_V1_FLAGS)
CXXFLAGS += -I../include/v1/
CXXFLAGS += -I$(HOME)/aibc/lib/ailibEx/include

LIBS += $(GFQ_V1_FLAGS)
LIBS += ../lib/v1/libGFQueueApi.a
LIBS += $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#####################################################################################################
OBJS = \
	../src/segmentserver/SegmentStore.o \
	Adm.o

#####################################################################################################
${OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@
	
$(TARGET): $(OBJS)
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(LIBS) -o $@;
	@$(CP) $@ ../bin/
	
#####################################################################################################
clean:;
	@$(RM) *.o  $(TARGET) ../bin/$(TARGET)
	@find . -name "*.o" -exec $(RM) {} \;

check:
	$(LINT) $(CXXFLAGS) $(MAIN_SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(MAIN_SRCS) $(COMMON_SRCS)

