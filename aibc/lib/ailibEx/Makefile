include ./port/port.mk

# for gprof
#CXX = purify -follow-child-processes -max_threads=3000 g++
#CXXFLAGS += -pg

CXXFLAGS += $(ILP32) -I./include
#CXXFLAGS += -D_AI_ENABLE_NAMESPACE

TARGET_STATIC = lib/ailibEx.a
SRCS = $(shell ls src/*.cpp)
OBJS = $(subst src/,tmp/,$(subst .cpp,.o,$(SRCS)))

DEMO_TARGETS = $(subst demo/,bin/,$(subst .cpp,,$(shell ls demo/*.cpp))) 

all:	$(TARGET_STATIC) 

demo:   $(DEMO_TARGETS)

$(TARGET_STATIC): $(OBJS)
	$(RM) $(TARGET_STATIC)
	$(AR) $(TARGET_STATIC) $(OBJS)
	$(RANLIB) $(TARGET_STATIC)

tmp/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

bin/%: demo/%.cpp $(TARGET_STATIC)
	$(RM) $@
	$(CXX) $(CXXFLAGS) $(LIBS)  $<  $(TARGET_STATIC)  -o $@

clean:
	$(RM) $(OBJS) $(TARGET_STATIC)

depend:
	makedepend -m $(CXXFLAGS) $(SRCS)  

ctags:
	ctags `find . -name "*.h"` `find . -name "*.cpp"` 

package: $(TARGET_STATIC)
	tar cvf - port/*.mk include/*.h include/stl/*.h lib/*.a | gzip -c > ailibEx-$(PLATFORM)-$(shell date "+%Y%m%d%H%M%S").tar.gz
