#include "TestMacro.h"
#include "AIFile.h"

int main()
{
	AIFile loFile;
    char   csBuff[10]; 
	TEST_CASE( "base operator" ){
		ASSERT_EQ( loFile.Open( "tmp", O_CREAT|O_RDWR, 0600 ), 0 );
		ASSERT_EQ( loFile.Write( "123456789", 10 ), 10 );
		ASSERT_EQ( loFile.Write( 0, "123456789", 10 ), 10 );
		ASSERT_EQ( loFile.Read( csBuff, 10 ), 0 );
		ASSERT_EQ( loFile.Read( 0, csBuff, 10 ), 10 );
		ASSERT_EQ_STR( csBuff, "123456789" );
		ASSERT_EQ( loFile.GetSize(), 10 );
		ASSERT_EQ( loFile.Seek( 5 ), 5 );
		ASSERT_EQ( loFile.Read( csBuff, 10 ), 5 );
		ASSERT_EQ_STR( csBuff, "6789" );
		ASSERT_EQ( AIFile::Access( "tmp", R_OK ), 0 );
	}
}
