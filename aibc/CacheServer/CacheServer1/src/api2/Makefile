include ../../common.mk

TARGET = libCacheApi.a
all: $(TARGET)

CXXFLAGS += $(CACHESERVER_V2_FLAGS)
CXXFLAGS += -I$(HOME)/aibc/lib/ailibEx/include
CXXFLAGS += -I../../include/v2/

#DPLIB = $(HOME)/aibc/lib/ailibEx/lib/ailibEx.a

#####################################################################################################
MAIN_SRCS = $(shell ls *.cpp ../common/*.cpp)
MAIN_OBJS = $(subst .cpp,.o,$(MAIN_SRCS))

#####################################################################################################
$(MAIN_OBJS): %.o:./%.cpp
	$(CXX) $(CXXFLAGS) -c ./$(subst .o,.cpp,$@) -o $@

$(TARGET): $(MAIN_OBJS)
	@echo "[============= LINK ==============]"
	$(AR) $(ARFLAG) $(TARGET) $^;
	@$(CP) ${TARGET} ../../lib/v2/

#####################################################################################################
clean:
	@$(RM) *.o  $(TARGET)
	@$(RM) ../../lib/v2/$(TARGET)
	@$(RM) ../common/*.o

check:
	$(LINT) $(CXXFLAGS) $(MAIN_SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(MAIN_SRCS) $(COMMON_SRCS)

