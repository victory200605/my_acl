include $(HOME)/aibc/lib/ailibEx/port/port.mk

TARGET = server

all: $(TARGET)

CXXFLAGS += -g -Wall -I$(AIBC_HOME)/include
LIBS += -lrt -L$(AIBC_HOME)/lib -lapl -lacl -lanf

#CXX = purify -follow-child-processes -max_threads=500 g++

#####################################################################################################
SRCS = $(shell ls *.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))

####################################################################################################
${OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@

${TARGET}: $(OBJS)
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(LIBS) -o $@;

#####################################################################################################
clean:;
	@$(RM) *.o  $(TARGET)
#	@find .. -name "*.o" -exec $(RM) {} \;

check:
	$(LINT) $(CXXFLAGS) $(SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(SRCS) $(COMMON_SRCS)

