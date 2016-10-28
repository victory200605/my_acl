include ../../common.mk

TARGET=libGFQApi.a
all: $(TARGET)

#CXX := purify -follow-child-processes -max_threads=1000 g++

CXXFLAGS += $(GFQ_V2_FLAGS)
CXXFLAGS += -I../../include/v2/
CXXFLAGS += -I${AIBC_HOME}/include/

#DEPLIBS += ${AIBC_HOME}/lib/libanf.a 
#DEPLIBS += ${AIBC_HOME}/lib/libacl.a 
#DEPLIBS += ${AIBC_HOME}/lib/libapl.a 

OBJS = \
GFQueueClientImpl.o \
GFQueueClient.o \
GFQProtocol.o

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -g -o $@

$(TARGET): $(OBJS) 
	@echo "[============= LINK ==============]"
	$(AR) $(ARFLAG) $(TARGET) $^ $(DEPLIBS)
	@$(CP) $(TARGET) ../../lib/v2/

clean:
	@$(RM) *.o  $(TARGET)
	@$(RM) ../../lib/v2/$(TARGET)
	@find . -name "*.o" -exec $(RM) {} \;

depend:
	makedepend -m $(CXXFLAGS) $(SRCS)  

ctags:
	ctags `find . -name "*.h"` `find . -name "*.cpp"` 

