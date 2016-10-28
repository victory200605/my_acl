AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../../include -I$(AIBC_HOME)/include -I$(MYSQL_HOME)/include/

AM_CXXFLAGS += '-DADL_MYSQL_ENABLE' 
lib_LTLIBRARIES = libadl.la libadl_mysql2.la

#libadl_la_LIBADD = $(AIBC_HOME)/lib/libacl.la $(AIBC_HOME)/lib/libapl.la

libadl_la_LDFLAGS = --enable-thread-safe-client -lmysqlclient_r -L$(MYSQL_HOME)/lib/mysql/

libadl_la_SOURCES = \
						 ../ParamBind.cpp \
						 MySqlParamBind.cpp \
						 ../ResultSet.cpp \
						 MySqlDatabase.cpp \
						 DataBase.cpp \
						 MySqlCursor.cpp \
						 MySqlConn.cpp

#libadl_mysql2_la_LIBADD = $(AIBC_HOME)/lib/libacl.la $(AIBC_HOME)/lib/libapl.la

libadl_mysql2_la_LDFLAGS = --enable-thread-safe-client -lmysqlclient_r -L$(MYSQL_HOME)/lib/mysql/

libadl_mysql2_la_SOURCES = \
						 ../ParamBind.cpp \
						 MySqlParamBind.cpp \
						 ../ResultSet.cpp \
						 MySqlDatabase.cpp \
						 MySqlCursor.cpp \
						 MySqlConn.cpp
install-exec-hook:
	cd $(DESTDIR)$(libdir) && \
	rm -f libadl.la$(EXEEXT) libadl.lai$(EXEEXT) libadl.so$(EXEEXT) libadl.so.0$(EXEEXT) \
	libadl_mysql.so$(EXEEXT) libadl_mysql.so.0$(EXEEXT) && \
	mv -f libadl.a$(EXEEXT) libadl_mysql.a$(EXEEXT) && \
	if test -f libadl.so.0.0.0$(EXEEXT); then \
	  mv -f libadl.so.0.0.0$(EXEEXT) libadl_mysql.so.0.0.0$(EXEEXT) && \
	  $(LN_S) libadl_mysql.so.0.0.0$(EXEEXT) libadl_mysql.so.0$(EXEEXT) && \
	  $(LN_S) libadl_mysql.so.0.0.0$(EXEEXT) libadl_mysql.so$(EXEEXT); \
	fi && \
	if test -f libadl.so.0.0$(EXEEXT); then \
	  mv -f libadl.so.0.0$(EXEEXT) libadl_mysql.so.0.0$(EXEEXT) && \
	  $(LN_S) libadl_mysql.so.0.0$(EXEEXT) libadl_mysql.so.0$(EXEEXT) && \
	  $(LN_S) libadl_mysql.so.0.0$(EXEEXT) libadl_mysql.so$(EXEEXT); \
	fi;
