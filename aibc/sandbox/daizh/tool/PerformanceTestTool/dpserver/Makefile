include $(HOME)/aibc/lib/ailibEx/port/port.mk

TARGET = ./server
all: $(TARGET)

LIBS += $(AILIBEX_ROOT)/lib/ailibEx.a
CXXFLAGS += -D_HAS_EVENT_POLL -D_LINUX -I$(AILIBEX_ROOT)/include

#CXX = purify -follow-child-processes -max_threads=500 g++

#####################################################################################################
SRCS = $(shell ls ../*.cpp *.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))

####################################################################################################
${OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@

$(TARGET): $(OBJS)
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(LIBS) -o $(TARGET);

#####################################################################################################
clean:;
	@$(RM) *.o  $(TARGET)
	@find .. -name "*.o" -exec $(RM) {} \;

check:
	$(LINT) $(CXXFLAGS) $(SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(SRCS) $(COMMON_SRCS)
	
package:
	tar -cvf ../server/server ../client/client | gzip -c > ../PerformanceTestTool.tar.gz
