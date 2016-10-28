include ../common.mk

TARGET = Adm
all: $(TARGET)

CXXFLAGS += $(CACHESERVER_V1_FLAGS)
CXXFLAGS += -I$(HOME)/aibc/lib/ailibEx/include
CXXFLAGS += -I../include/v1/

LIBS += $(CACHESERVER_V1_FLAGS)
LIBS += $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#####################################################################################################
MAIN_OBJS = \
    ../src/CacheServer/BillCache/BillCache.o \
    ../src/CacheServer/BillCache/CacheIndex.o \
    ../src/CacheServer/BillCache/CacheData.o \
    ../src/CacheServer/BillCache/BitMap.o \
    ../src/common/Protocol.o \
    ../src/api1/CacheClient.o \
    Adm.o

#####################################################################################################
${MAIN_OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@
	
$(TARGET): $(MAIN_OBJS) $(DPLIB)
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(DPLIB) $(LIBS) -o $@;
	@$(CP) $@ ../bin/
	
#####################################################################################################
clean:;
	@$(RM) *.o  $(TARGET) ../bin/$(TARGET)
	@find ../src/CacheServer -name "*.o" -exec $(RM) {} \;
	@find ../src/api1 -name "*.o" -exec $(RM) {} \;

check:
	$(LINT) $(CXXFLAGS) $(MAIN_SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(MAIN_SRCS) $(COMMON_SRCS)

