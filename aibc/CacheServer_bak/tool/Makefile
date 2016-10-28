include $(HOME)/aibc/lib/ailibEx/port/port.mk

TARGET = Adm
all: $(TARGET)

CXXFLAGS += -D_AI_ENABLE_NAMESPACE -I$(HOME)/aibc/lib/ailibEx/include

DPLIB = ../api/libCacheApi.a
DPLIB += $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#####################################################################################################
FILE_CACHE_SRCS = $(shell ls ../FileCache/*.cpp)
FILE_CACHE_OBJS = $(subst .cpp,.o,$(FILE_CACHE_SRCS))

#####################################################################################################
PROTOCOL_SRCS = $(shell ls ../protocol/*.cpp)
PROTOCOL_OBJS = $(subst .cpp,.o,$(PROTOCOL_SRCS))

#####################################################################################################
MAIN_SRCS = $(FILE_CACHE_SRCS) $(PROTOCOL_SRCS) Adm.cpp
MAIN_OBJS = $(FILE_CACHE_OBJS) $(PROTOCOL_OBJS) Adm.o

#####################################################################################################
${MAIN_OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@
	
$(TARGET): $(MAIN_OBJS) $(DPLIB)
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(DPLIB) $(LIBS) -o $@;
	@$(CP) $@ ../bin/
	
#####################################################################################################
clean:;
	@$(RM) *.o  $(TARGET)
	@find . -name "*.o" -exec $(RM) {} \;

check:
	$(LINT) $(CXXFLAGS) $(MAIN_SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(MAIN_SRCS) $(COMMON_SRCS)

