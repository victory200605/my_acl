include ../../common.mk

TARGET = libGFQueueApi.a
all: $(TARGET)

#CXX = purify -follow-child-processes -max_threads=500 g++

CXXFLAGS += $(GFQ_V1_FLAGS)
CXXFLAGS += -I../../include/v1
CXXFLAGS += -I$(HOME)/aibc/lib/ailibEx/include

#DEPLIBS += $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#####################################################################################################
OBJS   = GFQueueApi.o \
../common/GFQProtocol.o 

#####################################################################################################
${OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@

${TARGET}: $(OBJS)
	@echo "[============= LINK ==============]"
	${AR} $(ARFLAG) ${TARGET} $^ $(DEPLIBS);
	@$(CP) ${TARGET} ../../lib/v1/

#####################################################################################################
clean:;
	@$(RM) *.o  $(TARGET)
	@$(RM) ../../lib/v1/$(TARGET)
	@find . -name "*.o" -exec $(RM) {} \;

check:
	$(LINT) $(CXXFLAGS) $(MAIN_SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(MAIN_SRCS) $(COMMON_SRCS)

