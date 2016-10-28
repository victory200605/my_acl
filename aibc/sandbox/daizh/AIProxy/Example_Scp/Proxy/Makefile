include $(HOME)/aimg/ismg50/lib/ailibEx/port/port.mk

TARGET = ./app
all: $(TARGET)

LIBS += $(HOME)/aimg/ismg50/lib/ailibEx/lib/ailibEx.a

#CXX = purify -follow-child-processes -max_threads=500 g++

#####################################################################################################
SRCS = $(shell ls ../../*.cpp *.cpp)
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
	@$(RM) ../../*.o
	@find .. -name "*.o" -exec $(RM) {} \;

check:
	$(LINT) $(CXXFLAGS) $(SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(SRCS) $(COMMON_SRCS)

