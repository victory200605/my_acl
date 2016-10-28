AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../../include -I$(AIBC_HOME)/include -I$(ORACLE_HOME)/rdbms/public -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/include

AM_CXXFLAGS += '-DADL_ORACLE_ENABLE' 
lib_LTLIBRARIES = libadl.la libadl_oracle2.la

#libadl_la_LIBADD = $(AIBC_HOME)/lib/libacl.la $(AIBC_HOME)/lib/libapl.la

libadl_la_LDFLAGS = -lclntsh -L$(ORACLE_HOME)/lib -L$(ORACLE_HOME)/lib32

libadl_la_SOURCES = \
						  ../ParamBind.cpp \
						  OracleParamBind.cpp \
						  ../ResultSet.cpp \
						  OracleDatabase.cpp \
						  DataBase.cpp \
						  OracleConn.cpp \
						  OracleCursor.cpp

#libadl_oracle2_la_LIBADD = $(AIBC_HOME)/lib/libacl.la $(AIBC_HOME)/lib/libapl.la

libadl_oracle2_la_LDFLAGS = -lclntsh -L$(ORACLE_HOME)/lib -L$(ORACLE_HOME)/lib32

libadl_oracle2_la_SOURCES = \
						  ../ParamBind.cpp \
						  OracleParamBind.cpp \
						  ../ResultSet.cpp \
						  OracleDatabase.cpp \
						  OracleConn.cpp \
						  OracleCursor.cpp
install-exec-hook:
	cd $(DESTDIR)$(libdir) && \
	rm -f libadl.la$(EXEEXT) libadl.lai$(EXEEXT) libadl.so$(EXEEXT) libadl.so.0$(EXEEXT) \
	libadl_oracle.so$(EXEEXT) libadl_oracle.so.0$(EXEEXT) && \
	mv -f libadl.a$(EXEEXT) libadl_oracle.a$(EXEEXT) && \
	if test -f libadl.so.0.0.0$(EXEEXT); then \
	  mv -f libadl.so.0.0.0$(EXEEXT) libadl_oracle.so.0.0.0$(EXEEXT) && \
	  $(LN_S) libadl_oracle.so.0.0.0$(EXEEXT) libadl_oracle.so.0$(EXEEXT) && \
	  $(LN_S) libadl_oracle.so.0.0.0$(EXEEXT) libadl_oracle.so$(EXEEXT); \
	fi && \
	if test -f libadl.so.0.0$(EXEEXT); then \
	  mv -f libadl.so.0.0$(EXEEXT) libadl_oracle.so.0.0$(EXEEXT) && \
	  $(LN_S) libadl_oracle.so.0.0$(EXEEXT) libadl_oracle.so.0$(EXEEXT) && \
	  $(LN_S) libadl_oracle.so.0.0$(EXEEXT) libadl_oracle.so$(EXEEXT); \
	fi;
