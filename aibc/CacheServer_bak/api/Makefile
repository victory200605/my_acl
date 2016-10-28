include $(HOME)/aibc/lib/ailibEx/port/port.mk

TARGET = libCacheApi.a
all: $(TARGET)

CXXFLAGS += -D_AI_ENABLE_NAMESPACE -I$(HOME)/aibc/lib/ailibEx/include

LIBS = $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#####################################################################################################
PROTOCOL_SRCS = $(shell ls ../protocol/*.cpp)
PROTOCOL_OBJS = $(subst .cpp,.o,$(PROTOCOL_SRCS))

#####################################################################################################
MAIN_OBJS   = $(PROTOCOL_OBJS) \
              CacheClient.o \
			  MemCacheClient.o \
			  ../MemCacheServer/AICacheTcp.o \
			  ../MemCacheServer/AICmdLineTcp.o

#####################################################################################################
${MAIN_OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@

${TARGET}: $(MAIN_OBJS)
	@echo "[============= LINK ==============]"
	${AR} $(ARFLAG) ${TARGET} $^;
	@$(CP) ${TARGET} ../lib/

#####################################################################################################
clean:;
	@$(RM) *.o  $(TARGET)
	@find . -name "*.o" -exec $(RM) {} \;

check:
	$(LINT) $(CXXFLAGS) $(MAIN_SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(MAIN_SRCS) $(COMMON_SRCS)

