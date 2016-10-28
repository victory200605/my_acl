include ../../common.mk

TARGET = libQueueControl.so
all: $(TARGET)

#CXX = purify -follow-child-processes -max_threads=500 g++

CXXFLAGS += $(GFQ_V1_FLAGS)
CXXFLAGS += -I../../include/v1/
CXXFLAGS += -I$(HOME)/aibc/lib/ailibEx/include
CXXFLAGS += -fPIC

LIBS += $(GFQ_V1_FLAGS)
LIBS += $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

LINKLIB = $(OSLIBS) $(LIBS)

#####################################################################################################
SRCS   = QueueControl.cpp
OBJS   = QueueControl.o

#####################################################################################################
${OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@

${TARGET}: $(OBJS)
	@echo "[============= LINK ==============]"
	${SOLINK} $(ARFLAG) -o ${TARGET} $^ $(LINKLIB);
	@$(CP) ${TARGET} ../../lib/

#####################################################################################################
clean:;
	@$(RM) *.o  $(TARGET)
	@$(RM) ../../lib/$(TARGET)
	@find . -name "*.o" -exec $(RM) {} \;

check:
	$(LINT) $(CXXFLAGS) $(MAIN_SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(MAIN_SRCS) $(COMMON_SRCS)

