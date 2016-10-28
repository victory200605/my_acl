include ../../common.mk

TARGET=libCacheServerApi.a
all: $(TARGET)

#CXX := purify -follow-child-processes -max_threads=1000 g++

CXXFLAGS += $(CACHESERVER_V3_FLAGS)
CXXFLAGS += -I../../include/v3/
CXXFLAGS += -I${AIBC_HOME}/include/

#DEPLIBS += ${AIBC_HOME}/lib/libanf.a 
#DEPLIBS += ${AIBC_HOME}/lib/libacl.a 
#DEPLIBS += ${AIBC_HOME}/lib/libapl.a 

OBJS = CacheClient.o CacheClientImpl.o

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -g -o $@

$(TARGET): $(OBJS) 
	@echo "[============= LINK ==============]"
	$(AR) $(ARFLAG) $(TARGET) $^ $(DEPLIBS)
	@$(CP) $(TARGET) ../../lib/v3/

clean:
	@$(RM) *.o  $(TARGET)
	@$(RM) ../../lib/v3/$(TARGET)
	@find . -name "*.o" -exec $(RM) {} \;

depend:
	makedepend -m $(CXXFLAGS) $(SRCS)  

ctags:
	ctags `find . -name "*.h"` `find . -name "*.cpp"` 

