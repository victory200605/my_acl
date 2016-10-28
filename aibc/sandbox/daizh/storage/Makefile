include $(HOME)/aibc/lib/ailibEx/port/port.mk

TARGET = writer reader pressure

all: $(TARGET)

CXXFLAGS += -g -Wall -DLDAP_DEPRECATED -I$(HOME)/aibc/lib/apl/include/
CXXFLAGS += -I$(HOME)/aibc/lib/acl/include/
CXXFLAGS += -I$(HOME)/aibc/lib/anf/include/
CXXFLAGS += -I$(HOME)/aibc/lib/build/include/
CXXFLAGS += -I$(HOME)/aibc/lib/build/apl/include/
CXXFLAGS += -I$(HOME)/aibc/lib/build/acl/include/
CXXFLAGS += -I$(HOME)/aibc/lib/build/anf/include/
LIBS += /usr/local/lib/libanf.a
LIBS += /usr/local/lib/libacl.a
LIBS += /usr/local/lib/libapl.a
LIBS += -lrt

#CXX = purify -follow-child-processes -max_threads=500 g++

#####################################################################################################
SRCS = $(shell ls *.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))

####################################################################################################
${OBJS}: %.o:./%.cpp
	${CXX} ${CXXFLAGS} -c ./${subst .o,.cpp,$@} -o $@

writer: SegmentQueue.o Storage.o Page.o BufferPool.o writer.o
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(LIBS) -o writer;

reader: SegmentQueue.o Storage.o Page.o BufferPool.o reader.o
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(LIBS) -o reader;

pressure: SegmentQueue.o Storage.o Page.o BufferPool.o pressure.o
	@echo "[============= LINK ==============]"
	$(LINKER) $^ $(LIBS) -o pressure;

#####################################################################################################
clean:;
	@$(RM) *.o  $(TARGET)
	@find .. -name "*.o" -exec $(RM) {} \;

check:
	$(LINT) $(CXXFLAGS) $(SRCS) 1>check.rst

depend:
	makedepend $(CXXFLAGS) $(SRCS) $(COMMON_SRCS)

