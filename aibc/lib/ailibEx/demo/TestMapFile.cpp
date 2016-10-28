#include "TestMacro.h"
#include "AIMapFile.h"

int main( int argc, char* argv[] )
{
	AIMapFile loMapFile;
	AIFile    loFile;
    char      lsBuff[8192];
	memset( lsBuff, 0, 8192 );
	memset( lsBuff, 'A', 10 );
	memset( lsBuff + 4096, 'B', 10 );

	TEST_CASE( "base operator" )
	{
		ASSERT_EQ( loFile.Open( "tmp", O_CREAT | O_RDWR, 0600 ), 0 );
		ASSERT_EQ( loFile.Write( lsBuff, 8192 ), 8192 );
		ASSERT_EQ( loFile.Sync(), 0 );
		ASSERT_EQ( loMapFile.Map( "tmp", PROT_READ|PROT_WRITE, MAP_SHARED ), 0 );
		ASSERT_EQ( loMapFile.IsMap(), true );
		ASSERT_EQ_STR( (char*)loMapFile.GetAddr(), "AAAAAAAAAA" );
		strcpy( (char*)loMapFile.GetAddr(), "987654321" );
		ASSERT_EQ_STR( (char*)loMapFile.GetAddr(), "987654321" );
		ASSERT_EQ( loMapFile.Sync(), 0 );
		ASSERT_EQ( loFile.Read( 0, lsBuff, 10 ), 10 );
		ASSERT_EQ_STR( lsBuff, "987654321" );
		ASSERT_EQ( loMapFile.Unmap(), 0 );
		ASSERT_EQ( loMapFile.IsMap(), false );
		ASSERT_EQ( loMapFile.Map( loFile.GetHandle(), PROT_READ|PROT_WRITE, MAP_SHARED, 10 ), 0 );
		ASSERT_EQ( loMapFile.IsMap(), true );
		ASSERT_EQ( loMapFile.Map( 
			loFile.GetHandle(), PROT_READ|PROT_WRITE, MAP_PRIVATE, 10, sysconf(_SC_PAGE_SIZE)*1 ), 0 );
		ASSERT_EQ_STR( (char*)loMapFile.GetAddr(), "BBBBBBBBBB" );
		ASSERT_EQ( loMapFile.Unmap(), 0 );
		ASSERT_EQ( loMapFile.Map( loFile.GetHandle(), PROT_READ|PROT_WRITE, MAP_PRIVATE, 10, sysconf(_SC_PAGE_SIZE)*2 ), 
			AIMapFile::AI_ERROR_OUT_OF_RANGE );
	}
}
