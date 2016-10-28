AM_CXXFLAGS  = $(EXTRA_FLAGS) -I$(srcdir)/../include -I$(AIBC_HOME)/include

AM_LDFLAGS = -lapl -lacl -lcppunit -lpthread -L$(AIBC_HOME)/lib

check_PROGRAMS = 
TESTS = 


if ADL_MYSQL_ENABLE
check_PROGRAMS += test_adl_mysql
#test_adl_mysql_LDADD = ../src/mysql/libadl.la
test_adl_mysql_LDADD = ../src/adl/libadl2.la
#test_adl_mysql_LDADD = ../src/mysql/.libs/libadl_mysql2.a -lmysqlclient_r
test_adl_mysql_SOURCES = TestAdlMysql.cpp 
TESTS += test_adl_mysql
endif

if ADL_ORACLE_ENABLE
check_PROGRAMS += test_adl_oracle
#test_adl_oracle_LDADD = ../src/oracle/libadl.la
test_adl_oracle_LDADD = ../src/adl/.libs/libadl2.a
test_adl_oracle_SOURCES = TestAdlOracle.cpp 
TESTS += test_adl_oracle
endif

if ADL_SQLITE_ENABLE
check_PROGRAMS += test_adl_sqlite
test_adl_sqlite_LDADD = ../src/adl/libadl2.la
#test_adl_sqlite_LDADD = ../src/sqlite/.libs/libadl_sqlite.a -lsqlite3
test_adl_sqlite_SOURCES = TestAdlSQLite.cpp 
TESTS += test_adl_sqlite
endif

