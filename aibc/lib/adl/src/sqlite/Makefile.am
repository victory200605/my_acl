AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../../include -I$(AIBC_HOME)/include -I$(SQLITE_HOME)/include/

AM_CXXFLAGS += '-DADL_SQLITE_ENABLE' 
lib_LTLIBRARIES = libadl_sqlite.la

#libadl_sqlite_la_LIBADD = $(AIBC_HOME)/lib/libacl.la $(AIBC_HOME)/lib/libapl.la

libadl_sqlite_la_LDFLAGS = -lsqlite3 -L$(SQLITE_HOME)/lib/

libadl_sqlite_la_SOURCES = \
SQLiteDatabase.cpp \
SQLiteConnection.cpp \
SQLiteStatement.cpp

