include ../../common.mk

TARGET = libCacheClient.a
all: $(TARGET)

CXXFLAGS += $(CACHESERVER_V1_FLAGS)
CXXFLAGS += -I$(HOME)/aibc/lib/ailibEx/include
CXXFLAGS += -I../../include/v1/

#DPLIB = $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#####################################################################################################
MAIN_OBJS   = CacheClient.o 

#####################################################################################################
$(MAIN_OBJS): %.o:./%.cpp
	$(CXX) $(CXXFLAGS) -c ./$(subst .o,.cpp,$@) -o $@

$(TARGET): $(MAIN_OBJS)
	@echo "[============= LINK ==============]"
	$(AR) $(ARFLAG) $(TARGET) $^;
	@$(CP) ${TARGET} ../../lib/v1/

#####################################################################################################
clean:
	@$(RM) *.o  $(TARGET)
	@$(RM) ../../lib/v1/$(TARGET)
	@find . -name "*.o" -exec $(RM) {} \;

check:
	$(LINT) $(CXXFLAGS) $(MAIN_SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(MAIN_SRCS) $(COMMON_SRCS)

